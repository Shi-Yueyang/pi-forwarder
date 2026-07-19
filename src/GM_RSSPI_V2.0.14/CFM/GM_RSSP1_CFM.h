/**
* @file GM_RSSP1_CFM.h
* @brief CFM模块结构定义、相关接口、处理函数定义
* @author JiangHongjun
* @date 2010-2-7 16:21:23
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>jianghongjun@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/


#ifndef GM_RSSP1_CFM_H_
#define GM_RSSP1_CFM_H_

#include "GM_RSSP1_Lib_Def.h"
#include "GM_RSSP1_Msg_Queue.h"


#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum
	{
		GM_RSSP1_CFM_Get_SFM_Req_AVAILABLE           = 3300, /* pL2U parameters has been filled with a Pri. */
		GM_RSSP1_CFM_Get_SFM_Req_UNAVAILABLE         = 3301, /* No  Pri available. pU2L is meaningless. */
		GM_RSSP1_CFM_Get_SFM_Req_INVALID_OBJ         = 3302, /* The object pointed by pFSFB_CFM is invalid (NULL). */
		GM_RSSP1_CFM_Get_SFM_Req_INVALID_L2U         = 3303, /* The object pointed by pL2U is invalid (NULL). */
		GM_RSSP1_CFM_Get_SFM_Req_LIB_NOT_OPERATIONAL = 3304, /* The library is not in operational state (FSFB_CFM_Library_State_Operational). */
		GM_RSSP1_CFM_Get_SFM_Req_UNPROTECTED_OBJ     = 3305  /* The object pointed by pSFM has an invalid protection. */
	} GM_RSSP1_CFM_Get_SFM_Req_Result_Enum;


	typedef struct
	{
		GM_RSSP1_UINT8 pktType;
		GM_RSSP1_UINT16 nextIndex;
		GM_RSSP1_UINT32 pktTC;
	}RSSP1_RCV_PKT_struct;

	typedef struct
	{
		GM_RSSP1_UINT16 num;
		GM_RSSP1_UINT16 MinTC_Index; /*亦可作为 start_index*/
		GM_RSSP1_UINT16 MaxTC_Index;
		GM_RSSP1_UINT16 End_Index;
		RSSP1_RCV_PKT_struct pktInfo[GM_RSSP1_MAX_PKT_TOTAL_NUM];
		GM_RSSP1_com_input_pri_struct Allpkt[GM_RSSP1_MAX_PKT_TOTAL_NUM]; 
	}RSSP1_RCV_LINK_struct;

	typedef struct
	{
		GM_RSSP1_Msg_Queue_struct SND_MQ;
		GM_RSSP1_Msg_Queue_struct RCV_MQ[GM_RSSP1_MAX_ONLINE_CONNECTION_NUM][GM_RSSP1_MAX_LINK_CHANNEL];
	}RSSP1_MQ_LINK_struct;
	

	/** 一个UDP通道的结构定义 */

	typedef struct
	{
		GM_RSSP1_BOOL                       b_connected;
        GM_RSSP1_UINT32                     loc_ip;
        GM_RSSP1_UINT32                     rem_ip;
        GM_RSSP1_UINT32                     loc_port;
        GM_RSSP1_UINT32                     rem_port;
		GM_RSSP1_UINT32                     recv_Q_size;
		GM_RSSP1_Pointer					recv_Q_offset;
	}GM_RSSP1_PACKED GM_RSSP1_CFM_chn_mng_struct;

	typedef struct
	{
		GM_RSSP1_UINT16						index;
		GM_RSSP1_connection_state_enum      state;                  /**< 连接状态 */
		GM_RSSP1_SACEPID_struct				SaCEPID;                /**< 安全连接ID */
		GM_RSSP1_BOOL                       b_enable_CRSCD_pack;    /**< 使用通号设计院TCC组包方案.多包合一包、一包拆多包 */
		GM_RSSP1_CFM_Com_Type_Enum          com_type;               /**< 底层冗余通信方式  */   	    
		GM_RSSP1_UINT16                     chn_num;                            /**< 冗余通道数，不大于GM_RSSP1_CFM_MAX_CHANNEL_NUM */
		GM_RSSP1_UINT32						recv_Q_size;
		GM_RSSP1_CFM_chn_mng_struct			com_chn[GM_RSSP1_CFM_MAX_CHANNEL_NUM];  /**< 所有冗余通道 */
	}GM_RSSP1_PACKED GM_RSSP1_CFM_connection_struct;

	/** 通信层模块定义 */

	typedef struct
	{
		GM_RSSP1_library_state_enum					lib_state;
		GM_RSSP1_UINT16								connection_nums;
		GM_RSSP1_UINT32								all0_size;
		GM_RSSP1_Pointer							SendQ_offset;
		GM_RSSP1_Pointer							U2L_pri_Q_offset;   /**< 共需要为队列分配的字节数 = U2L_pri_Q_size * sizeof(GM_RSSP1_SFM_U2L_pri_struct) */
		GM_RSSP1_Pointer							L2U_pri_Q_offset;   /**< 共需要为队列分配的字节数 = L2U_pri_Q_size * sizeof(GM_RSSP1_SFM_L2U_pri_struct) */
		GM_RSSP1_CFM_connection_struct				connection[GM_RSSP1_MAX_SAFETY_CONNECTION_NUM];
	} GM_RSSP1_PACKED GM_RSSP1_CFM_object_struct;


	/************************************************************************/
	/* 接口                                         */
	/************************************************************************/

	/**
	* @brief 获取静态CFM对象，以指针形式返回。
	*
	* Detailed description.
	* @return GM_RSSP1_CFM_object_struct*
	*/
	GM_RSSP1_CFM_object_struct *GM_RSSP1_Get_CFM_Object(void);

	/**
	* @brief 获取CFM模块状态
	*
	* Detailed description.
	* @param[in] pCFM
	* @return GM_RSSP1_library_state_enum
	*/
	GM_RSSP1_library_state_enum GM_RSSP1_CFM_Get_Library_Status(GM_RSSP1_CFM_object_struct *pCFM);

	/**
	* @brief 判断两个SACEPID是否相等
	*
	* Detailed description.
	* @param[in] pSaCEPID1
	* @param[in] pSaCEPID2
	* @return GM_RSSP1_BOOL
	*/
	GM_RSSP1_BOOL GM_RSSP1_CFM_Is_SaCEPID_Equal(GM_RSSP1_SACEPID_struct *pSaCEPID1 , GM_RSSP1_SACEPID_struct *pSaCEPID2);

	/**
	* @brief 根据SACEPID获取一个安全连接
	*
	* Detailed description.
	* @param[in] pSaCEPID
	* @param[in] pSFM
	* @return GM_RSSP1_CFM_connection_struct*
	*/
	GM_RSSP1_CFM_connection_struct *GM_RSSP1_CFM_Get_Connection_BY_SaCEPID(GM_RSSP1_SACEPID_struct *pSaCEPID);

	/**
	* @brief  CFM处理来自SFM的消息
	*
	* Detailed description.
	* @param[in] pCFM
	*/
	void GM_RSSP1_CFM_Proc_SFM_Req(void);

	/**
	* @brief CFM发送来自SFM模块的安全数据包
	*
	* 主要有SSE、SSE、SSR三种数据包。
	* @param[in] pDat
	* @param[in] dat_len
	* @param[in] pCFM_con
	* @param[in] pCFM
	* @return GM_RSSP1_BOOL
	*/
	GM_RSSP1_BOOL GM_RSSP1_CFM_Send_Dat(GM_RSSP1_UINT8 *pDat , GM_RSSP1_UINT16 dat_len , GM_RSSP1_UINT16 Inx);

	/**
	* @brief CFM发送来自上层的数据包
	*
	* @param[in] psacepid
	* @param[in] pDat
	* @param[in] len
	* @param[in] pCFM
	* @return GM_RSSP1_BOOL
	*/
	GM_RSSP1_BOOL GM_RSSP1_CFM_User_Send_Dat(GM_RSSP1_SACEPID_struct *psacepid , GM_RSSP1_UINT8 *pDat , GM_RSSP1_UINT16 len);

	GM_RSSP1_BOOL GM_RSSP1_CFM_LatestUnderPack(GM_RSSP1_com_input_pri_struct* rcvd_dat_pri);

	GM_RSSP1_BOOL GM_RSSP1_CFM_LatestAlone(GM_RSSP1_com_input_pri_struct* rcvd_dat_pri);

	GM_RSSP1_BOOL GM_RSSP1_CFM_Proc_Con_DatWithLatest(GM_RSSP1_UINT16 index, GM_RSSP1_BOOL bLock);

	GM_RSSP1_BOOL GM_RSSP1_CFM_ReSeqUnderPack(GM_RSSP1_com_input_pri_struct* rcvd_dat_pri);

	GM_RSSP1_BOOL GM_RSSP1_CFM_ReSeqAlone(GM_RSSP1_com_input_pri_struct* rcvd_dat_pri);

	GM_RSSP1_BOOL GM_RSSP1_CFM_Proc_Con_DatWithReSeq(GM_RSSP1_UINT16 index, GM_RSSP1_BOOL bLock);

	/**
	* @brief CFM处理一个队列的外部消息
	*
	* @param[in] pCFM	CFM内部数据集合对象
	* @param[in] index	处理对应通信队列索引
	* @return GM_RSSP1_BOOL
	*/
	GM_RSSP1_BOOL GM_RSSP1_CFM_Proc_Con_Rcvd_Dat(GM_RSSP1_UINT16 index);

	/**
	* @brief CFM模块处理所有接收数据包
	*
	* Detailed description.
	* @param[in] pCFM	CFM内部数据集合对象
	* @return
	*/
	void GM_RSSP1_CFM_Proc_Recvd_Dat(GM_RSSP1_CFM_object_struct * pCFM);

	/**
	* @brief CFM提交通信状态包给SFM
	*
	* Detailed description.
	* @param[in] pCFM	CFM内部数据集合对象
	* @return
	*/
	void GM_RSSP1_CFM_Report_com_State_To_SFM(GM_RSSP1_CFM_object_struct *pCFM);

	/**
	* @brief CFM第二种接收模式下数据包Msg链表的插入函数
	*
	* Detailed description.
	* @param[in] rcvd_TC	Msg内TC值
	* @param[in] MsgType	Msg类型
	* @param[in] pkt_len	数据包长度
	* @param[in] data_pri	数据包内容指针
	* @return
	*/
	void GM_RSSP1_CFM_PktLink_Insert(GM_RSSP1_UINT32 rcvd_TC, GM_RSSP1_UINT8 MsgType, GM_RSSP1_UINT16 pkt_len, GM_RSSP1_UINT8* data_pri);
#ifdef CFM_Stanby_Answer
	/**
	* @brief CFM按活动通道,回复全0数据
	*
	* Detailed description.
	* @return GM_RSSP1_BOOL
	*/
	GM_RSSP1_BOOL GM_RSSP1_Standby_Info(void);
#endif
#ifdef __cplusplus
}
#endif

#endif
