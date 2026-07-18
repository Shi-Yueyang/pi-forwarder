#ifndef GM_RSSP1_MQ_INTERFACE_H
#define GM_RSSP1_MQ_INTERFACE_H

#include "GM_RSSP1_Msg_Queue.h"
#include "GM_RSSP1_Utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct
	{
		GM_RSSP1_Msg_Queue_struct RCV_MQ;
		GM_RSSP1_Msg_Queue_struct SND_MQ;
	}GM_RSSP1_RCV_SND_MQ_struct;

	typedef enum
	{
		RSSP1_S2C_SND = 0x01,	/**数据从SFM到CFM,即发送到外部的数据**/
		RSSP1_C2S_RCV = 0x02	/**数据从CFM到SFM,即从外部接收的数据**/
	}GM_RSSP1_MQ_Type_Enum;


	typedef GM_RSSP1_RCV_SND_MQ_struct RSSP1_MQ_Inter_struct;

	/**
	* @brief 当SFM与CFM分拆后,将消息写入交互数据队列的接口
	* @param[in] type		指定队列(SFM给CFM的消息 异或 CFM给SFM的消息)
	* @param[out] pMsg		待写入的数据缓存区
	* @return GM_RSSP1_BOOL	队列操作结果(True:写入列成功;False:失败)
	*/
	GM_RSSP1_BOOL GM_RSSP1_MessageQueue_Inter_Set(GM_RSSP1_MQ_Type_Enum QType, GM_RSSP1_UINT8* pMsg);

	/**
	* @brief 当SFM与CFM分拆后,获取交互数据队列中消息的接口
	* @param[in] type		待发送数据的内存首地址
	* @param[in] pMsg		待发送数据的长度
	* @param[in] len		待发送数据的远端地址
	* @return GM_RSSP1_BOOL	队列操作结果(True:写入列成功;False:失败)
	*/
	GM_RSSP1_INT8 GM_RSSP1_MessageQueue_Inter_Get(GM_RSSP1_MQ_Type_Enum QType, GM_RSSP1_UINT8* pMsg, GM_RSSP1_UINT32* len);


#ifdef __cplusplus
}
#endif /* extern "C" */

#endif