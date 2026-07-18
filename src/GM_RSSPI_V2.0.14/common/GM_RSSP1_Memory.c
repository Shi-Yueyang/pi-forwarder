/**
* @file GM_RSSP1_Memory.c
* @brief
* @author Hao Liming
* @date 2010-9-30 13:33:19
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>haoliming@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/
#include "GM_RSSP1_Memory.h"
#include "GM_RSSP1_Utils.h"

#if !defined (REWORKS_CERTV2) && !defined (NO_SYS_MEMORY)
GM_RSSP1_UINT8 *g_pMemBase_P1 = NULL;/*指向内存首地址*/
GM_RSSP1_UINT64 g_MemSize_P1 = 0U;/*内存总大小*/


void* GM_RSSP1_malloc(size_t size)
{
    void* rt = NULL;

	/*增加对size=0时候，返回null */
	if (size != (size_t)0)
	{
	#ifndef GM_RSSP1_MEMORY_WITH_STATIC /*CR4609: Add new macro GM_RSSP1_MEMORY_WITH_STATIC with memory address&size from upper*/
		rt = malloc(size);
	#else
		rt = GM_RSSP1_Simple_Malloc(size);
	#endif
	}

    return rt;
}
/* CR: 13800904 */
void GM_RSSP1_memcpy(void* destination, const void* source, size_t size)
{
	GM_RSSP1_UINT8* pDestAddr = NULL;
	GM_RSSP1_UINT8* pSourceAddr = NULL;

	if ((NULL == destination) || (NULL == source) || (size == (size_t)0))
	{
		return;
	}
#ifndef GM_RSSP1_MEMORY_WITH_STATIC
	memcpy(destination, source, size);
#else
	pDestAddr = (GM_RSSP1_UINT8*)destination;
	pSourceAddr = (GM_RSSP1_UINT8*)source;

	if ((pDestAddr <= pSourceAddr) || (pDestAddr > (pSourceAddr + size)))
	{
		while (size--)
		{
			*pDestAddr = *pSourceAddr;
			pDestAddr = pDestAddr + 1U;
			pSourceAddr = pSourceAddr + 1U;
		}
	}
	else/*有内存重叠的情况 */
	{
		pDestAddr = pDestAddr + size - 1U;
		pSourceAddr = pSourceAddr + size - 1U;
		while (size--)
		{
			*pDestAddr = *pSourceAddr;
			pDestAddr = pDestAddr - 1U;
			pSourceAddr = pSourceAddr - 1U;
		}
	}

#endif
    return;
}

/* CR: 13800904 */
void GM_RSSP1_memset(void* destination, GM_RSSP1_UINT8 value, size_t size)
{
	size_t i = 0U; /* defect: 14027787*/
	GM_RSSP1_UINT8* pDestAddr = NULL;

    if ((NULL == destination) || (size == (size_t)0))
    {
		GM_RSSP1_Log_Usr(GM_RSSP1_Memory_Init_PAR_ERR, 0, 0, 0, 0, 0, 0);
        return;
    }
#ifndef GM_RSSP1_MEMORY_WITH_STATIC
    memset(destination, (int)value, size);
#else
	pDestAddr = (GM_RSSP1_UINT8*)destination;

	while (i < size)
	{
		*(pDestAddr + i) = value;
		i++;
	}

#endif
	return;
}

void GM_RSSP1_free(void* p)
{
#ifndef GM_RSSP1_MEMORY_WITH_STATIC
    if (NULL != p)
    {
        free(p);
    }
#endif
    return;
}

/*CR4609: Add new macro GM_RSSP1_MEMORY_WITH_STATIC with memory address&size from upper*/
GM_RSSP1_BOOL GM_RSSP1_Memory_Init(void *pStartAddr, GM_RSSP1_UINT64 size)
{
	if ((pStartAddr == NULL)||(size == 0U))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Memory_Init_PAR_ERR, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	if ((g_pMemBase_P1 != NULL)&&(g_MemSize_P1 != 0U))
	{
	 /*has already initialized
	 不允许重复调用，返回false提示用户二次设置错误。
	 随机失效再次调用该函数 系统失效 固有错误 调用该函数两次 返回false让系统据此进入导向安全的操作
	 */
		GM_RSSP1_Log_Usr(GM_RSSP1_Memory_Init_AGAIN_ERR, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	g_pMemBase_P1 = (GM_RSSP1_UINT8*)pStartAddr;
	g_MemSize_P1 = size;

	return GM_RSSP1_TRUE;
}

void* GM_RSSP1_Simple_Malloc(size_t size)
{

	static GM_RSSP1_UINT64 g_MemUsedSize_P1 = 0U;/*已使用大小*/
	void* p = NULL;
	GM_RSSP1_UINT64 tempSize = (GM_RSSP1_UINT64)size;
	GM_RSSP1_UINT8 tmpVal = 0U;	
#ifdef OS_64
	GM_RSSP1_UINT8 alignSize = 8U;
#else
	GM_RSSP1_UINT8 alignSize = 4U;
#endif

	if ((tempSize == 0U)||(g_pMemBase_P1 == NULL))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Simple_Malloc_PAR_ERR, 0, 0, 0, 0, 0, 0);
		return NULL;
	}

	/*4字节或8字节对齐，提高访问效率*/
	tmpVal = (GM_RSSP1_Pointer)(g_pMemBase_P1 + g_MemUsedSize_P1) % alignSize;

	if (tmpVal != 0U)
	{
		g_MemUsedSize_P1 += (GM_RSSP1_UINT64)(alignSize - tmpVal); /*@63343 若未对齐则需要调整对齐*/
	}
	
	if (tempSize > (g_MemSize_P1 - g_MemUsedSize_P1))/*等于时，刚好分配完;用减法，防止溢出*/
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Simple_Malloc_SIZE_ERR, 0, 0, 0, 0, 0, 0);
		return NULL; /*剩余空间不足*/
	}

	/*g_pMemBase_P1需定义为GM_UINT8*,才能通过‘+数字’，来前进*/

	/* @63343 指针 + n = 指针地址 + n * sizeof（指针指向的数据类型）
	* 那么如果定义未char*，则g_pMemBase_P1 + n 就每次都会移动 n*1字节
	* 即+1就移动一个字节，符合按数字偏移的设定
	*/
	p = (void*)(g_pMemBase_P1 + g_MemUsedSize_P1); /*@63343 移动到剩余空间*/
	g_MemUsedSize_P1 += tempSize; /*@63343 更新已使用空间的大小*/

	return p;
}
#endif
