#include "GM_RSSP1_SFM_Interface.h"

#ifdef SFM_Part
#include "GM_RSSP1_SFM_Init.h"

#ifdef CFM_Part
#include "GM_RSSP1_CFM_Init.h"
#endif

/*接收过程 先CFM，再SFM*/

void GM_RSSP1_SFM_Interface_Proc_Recv(void)
{
	GM_RSSP1_SFM_object_struct *pSFM = NULL;
	pSFM = GM_RSSP1_Get_SFM_Object();

	if ((NULL == pSFM))
	{
		return;
	}

	if (GM_RSSP1_Lib_State_Operational != pSFM->lib_state)
	{
		return;
	}

#ifndef GM_RSSP1_SAVING_MODE
	GM_RSSP1_SFM_Update_Time();
#endif

	GM_RSSP1_SFM_Process_CFM_Ind(pSFM);/*该函数直接将从CFM队列处理接收消息，拆分中间队列*/

	/* jianghongjun 20100618 调整GM_RSSP1_SFM_Dispatch_Dat_Indi与GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate调用顺序，应该先宽恕，后分发各通道数据。 */
	GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate();

	GM_RSSP1_SFM_Syn_Con(pSFM);
	
	GM_RSSP1_SFM_Dispatch_Dat_Indi(pSFM);
}

/*发送过程 先SFM，再CFM*/

void GM_RSSP1_SFM_Interface_Proc_Send(void)
{
	GM_RSSP1_SFM_object_struct *pSFM = NULL;

	pSFM = GM_RSSP1_Get_SFM_Object();

	if (NULL == pSFM)
	{
		return;
	}

	if (GM_RSSP1_Lib_State_Operational != pSFM->lib_state)
	{
		return;
	}

#ifndef GM_RSSP1_SAVING_MODE
	GM_RSSP1_SFM_Process_User_Req();
#endif

	GM_RSSP1_SFM_Reset_Connection_RSD_Sent_Flag();

#ifdef CFM_Part
	GM_RSSP1_Reset_Active();/*重置当前周期的活动通道信息*/
#endif
	return;
}

void GM_RSSP1_SFM_Interface_Remove(GM_RSSP1_UINT16 Dst_Addr)
{
	GM_RSSP1_UINT8 i = 0U;
	GM_RSSP1_UINT8 counter = 0U;
	GM_RSSP1_UINT16 CnfArray[4U] = {0U};
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_object_struct *pSFM = NULL;
#ifdef CFM_Part
	GM_RSSP1_CFM_object_struct *pCFM = NULL;
#endif

	GM_RSSP1_memset((void*)CnfArray, 0xFFU, sizeof(CnfArray));

	pSFM = GM_RSSP1_Get_SFM_Object();
	if (NULL != pSFM)
	{
		bRt = GM_RSSP1_Hash_SFM_Dele((GM_RSSP1_UINT32)Dst_Addr, &counter, CnfArray);/*删除SFM层哈希表内的对应索引信息*/
		for(i=0u; i<counter; ++i)
		{
			GM_RSSP1_SFM_Delete(CnfArray[i], pSFM);/*删除SFM层中的相关节点配置*/
#ifdef CFM_Part
			pCFM = GM_RSSP1_Get_CFM_Object();
			if (NULL != pCFM)
			{
				GM_RSSP1_CFM_Delete(CnfArray[i], pCFM);/*删除CFM层中的相关节点配置*/
			}
#endif
			GM_RSSP1_SeqDelete(OffLine_Object, CnfArray[i]);/*删除链表（所有节点）中的对应索引信息*/
			GM_RSSP1_SeqDelete(OnLine_Object, CnfArray[i]);/*删除链表（活动节点）中的对应索引信息*/
		}
	}
}
#endif
