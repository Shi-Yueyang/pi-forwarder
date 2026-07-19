/**
* @file GM_RSSP1_Memory.h
* @brief
* @author Hao Liming
* @date 2010-9-30 13:33:06
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>haoliming@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/
#ifndef GM_RSSP1_MEMORY_H
#define GM_RSSP1_MEMORY_H

#include "GM_RSSP1_Utils_Base.h"


#ifdef WIN32
#include <malloc.h>
#endif


#if !defined (REWORKS_CERTV2) && !defined (NO_SYS_MEMORY)
#ifdef __cplusplus
extern "C"
{
#endif /**< __cplusplus */

    /**
    * @brief
    *
    * 封装了malloc.
	* GM_RSSP1_MEMORY_WITH_STATIC时调用GM_RSSP1_Simple_Malloc
	*
    * @param[in] size
    * @return void* GM_malloc
    */
    void* GM_RSSP1_malloc(size_t size);

    /**
    * @brief
    *
    * 封装了memcpy.
    * @param[in] destination
    * @param[in] source
    * @param[in] dim
    * @return void GM_memcpy
    */
    void GM_RSSP1_memcpy(void* destination, const void* source, size_t size);

    /**
    * @brief
    *
    * 封装了memset.
    * @param[in] destination
    * @param[in] value
    * @param[in] dim
    * @return void GM_memset
    */
    void GM_RSSP1_memset(void* destination, GM_RSSP1_UINT8 value, size_t size);

    /**
    * @brief GM_free
    *
    * 释放指针所指向的内存,是对free的封装.
	* define GM_RSSP1_MEMORY_WITH_STATIC时不free
    * @param[in] p 指针
    */
    void GM_RSSP1_free(void* p);

	/** @63343
    * @brief 自定义分配内存函数，define GM_RSSP1_MEMORY_WITH_STATIC（用户自定义内存空间）时使用
    *
    * @param[in] size：需要分配内存空间的大小
	* @return p：返回分配的内存指针
    */
	void* GM_RSSP1_Simple_Malloc(size_t size);

	/** @63343
    * @brief 内存初始化，define GM_RSSP1_MEMORY_WITH_STATIC（用户自定义内存空间）时使用
    *
	* @param[in] pStartAddr：内存起始地址
    * @param[in] size：用户提供的总的可分配的内存空间大小
    */
	GM_RSSP1_BOOL GM_RSSP1_Memory_Init(void *pStartAddr, GM_RSSP1_UINT64 size);
#ifdef __cplusplus
}

#endif /**< __cplusplus */
#endif
#endif /**< _GM_MALLOCY_H */

