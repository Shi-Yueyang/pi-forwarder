#ifndef GM_RSSP1_CFM_INTERFACE_H
#define GM_RSSP1_CFM_INTERFACE_H

#include "GM_RSSP1_Utils.h"
#include "GM_RSSP1_CFM_Init.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct
	{
        GM_RSSP1_UINT32                     loc_ip;
        GM_RSSP1_UINT32                     loc_port;
	}GM_RSSP1_PACKED GM_RSSP1_Local_Addr_struct;

	typedef struct
	{
	  GM_RSSP1_UINT16                     chn_num; 
	  GM_RSSP1_Local_Addr_struct  locl_addr[GM_RSSP1_CFM_MAX_CHANNEL_NUM];
	}GM_RSSP1_PACKED GM_RSSP1_Local_CHN_struct;

	typedef struct
	{
	  GM_RSSP1_UINT16                     connection_num; 
	  GM_RSSP1_Local_CHN_struct  connection[GM_RSSP1_MAX_SAFETY_CONNECTION_NUM];
	}GM_RSSP1_PACKED GM_RSSP1_Local_Con_struct;

    /**
    * @brief 该函数接口用于CFM层发送处理过程函数（先接收，后发送流程）
    * @return
    */
	void GM_RSSP1_CFM_Interface_Proc_Send(void);

	/**
    * @brief 该函数接口用于CFM层接收处理过程函数（先接收，后发送流程）
    * @return
    */
	void GM_RSSP1_CFM_Interface_Proc_Recv(void);

    /**
    * @brief 该函数接口用于根据con_index, red_index获取对应链路IP+Port
	* @param[out] r_port	目的port
	* @param[out] r_ip		目的IP
	* @param[in] con_index	通道索引
	* @param[in] red_index	冗余网索引
    * @return GM_RSSP1_BOOL
    */
	GM_RSSP1_BOOL GM_RSSP1_Get_Address(GM_RSSP1_UINT32* r_port, GM_RSSP1_UINT32* r_ip, GM_RSSP1_UINT16 con_index, GM_RSSP1_UINT8 red_index);

    /**
    * @brief 该函数接口用于根据IP,Port获取对应软件内的con_index和red_index
	* @param[out] r_port	源port
	* @param[out] r_ip		源IP
	* @param[in] con_index	通道索引
	* @param[in] red_index	冗余网索引
    * @return GM_RSSP1_BOOL
    */
	GM_RSSP1_BOOL GM_RSSP1_Get_ConIndex(GM_RSSP1_UINT32 r_port, GM_RSSP1_UINT32 r_ip, GM_RSSP1_UINT16* con_index, GM_RSSP1_UINT8* red_index);

    /**
    * @brief 循环调用该函数接口用于读取待发送消息,
	* @param[out] p_dat		待发送数据
	* @param[out] len		发送长度
	* @param[out] r_port	源port
	* @param[out] r_ip		源IP
    * @return GM_RSSP1_INT8	-1:非常规性故障; 0:消息读空; 1:读取到有效数据
    */
	GM_RSSP1_INT8 GM_RSSP1_Write_SFM_Interface_ergodic(GM_RSSP1_UINT8* p_dat, GM_RSSP1_UINT16* len, GM_RSSP1_UINT32* r_port, GM_RSSP1_UINT32* r_ip);

    /**
    * @brief 该函数接口清空消息队列
    * @return GM_RSSP1_BOOL
    */
	GM_RSSP1_BOOL RSSP1_CFM_Clear_Msg_Queue(void);

    /**
    * @brief 循环调用该函数接口用于读取待发送消息,
	* @param[out] p_dat		待发送数据
	* @param[out] len		发送长度
	* @param[out] r_port	目的port
	* @param[out] r_ip		目的IP
	* @param[out] index		通道索引
	* @param[out] chn_index	冗余网索引
    * @return GM_RSSP1_BOOL True表示存在有效数据
    */
	GM_RSSP1_BOOL GM_RSSP1_SND_com_Interface(GM_RSSP1_UINT8* p_dat, GM_RSSP1_UINT16* len, GM_RSSP1_UINT32 *r_ip, GM_RSSP1_UINT32 *r_port, GM_RSSP1_UINT16 *index, GM_RSSP1_UINT8 *chn_index);
	GM_RSSP1_BOOL GM_RSSP1_SND_com_with_crcm_Interface(GM_RSSP1_UINT8* p_dat, GM_RSSP1_UINT16* len, GM_RSSP1_UINT32 *r_ip, GM_RSSP1_UINT32 *r_port, GM_RSSP1_UINT16 *index, GM_RSSP1_UINT8 *chn_index, GM_RSSP1_UINT16 *crcm_index);

	/**
    * @brief 循环调用该函数接口用于接收网络消息,
	* @param[in] p_dat		待接收数据
	* @param[in] recv_len	数据长度
	* @param[in] r_port	源port
	* @param[in] r_ip		源IP
	* @param[in] index		通道索引
	* @param[in] chn_index	冗余网索引
	* @param[in] mode		模式定义 1:不使用形参index和chn_index,软件内部根据IP+Port索引
    * @return GM_RSSP1_BOOL True表示数据写入成功
    */
	GM_RSSP1_BOOL GM_RSSP1_RCV_com_Interface(GM_RSSP1_UINT8* p_dat, GM_RSSP1_INT16 recv_len, GM_RSSP1_UINT32 r_ip, GM_RSSP1_UINT32 r_port, GM_RSSP1_UINT16 index, GM_RSSP1_UINT8 chn_index, GM_RSSP1_UINT8 mode);

#ifdef CFM_Stanby_Answer
	GM_RSSP1_INT8 GM_RSSP1_Write_StandbyInfo_Interface_ergodic(GM_RSSP1_UINT8* p_dat, GM_RSSP1_UINT16* len, GM_RSSP1_UINT16* con_index, GM_RSSP1_UINT8* reduan_index);
#endif
	/**
    * @brief 根据配置Index删除CFM对应配置节点信息
	* @param[in] CnfIndex	配置节点对应索引
    * @return
    */
	void GM_RSSP1_CFM_Interface_RemoveWithIndex(GM_RSSP1_UINT16 CnfIndex);

	/**
	* @brief 根据port+ip删除CFM对应配置节点信息
	* @param[in] Dst_ip		远端ip
	* @param[in] Dst_port	远端port
    * @return
    */
	void GM_RSSP1_CFM_Interface_RemoveWithAddr(GM_RSSP1_UINT32 Dst_ip, GM_RSSP1_UINT32 Dst_port);

	/**
    * @brief 重置CFM中所有的活动通道
    * @return
    */
	void GM_RSSP1_Clean_Link_Status(void);

	/**
    * @brief 设置CFM接收消息冗余数据处理模式
	* @param[in] method		模式定义 0:将冗余通道数据都进行过滤(包括CRC16&包序),chn从0开始遍历
	*							   非0:将冗余通道数据过滤重复包并重排序
    * @return
    */
	void GM_RSSP1_Set_ChkPKT_Seq_Method(GM_RSSP1_UINT8 method);

	/**
    * @brief 获取当前CFM层内所有的本地ip+port信息
    * @return GM_RSSP1_Local_Con_struct* 所有ip+port的地址结构对象
    */
	GM_RSSP1_Local_Con_struct* GM_RSSP1_Get_LocInfo_Obj(void);

#ifdef __cplusplus
}
#endif

#endif
