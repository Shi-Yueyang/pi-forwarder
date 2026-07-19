#ifndef GM_RSSP1_SYN_H
#define GM_RSSP1_SYN_H

#include "GM_RSSP1_Utils.h"
#include "GM_RSSP1_Lib_Def.h"
#ifdef SFM_Part
#include "GM_RSSP1_SFM.h"
#endif
#ifdef CFM_Part
#include "GM_RSSP1_CFM.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	/** 约定主备同步通道的ID.其他正常fsfb连接的sacepid组合不可与此重复! */
#define GM_RSSP1_SYN_DEST_ADDR   ((GM_RSSP1_UINT16)0xffffU)

#define    INTERFACE_TYPE_SYN     0x55U							  /* 系间DVCOM同步接口相关 */
#define    INTERFACE_DAT_TYPE_SYN_RSSP1_STATUS_DAT    0x43U        /* 系间同步的RSSP1安全连接状态数据 */
#define    INTERFACE_DAT_TYPE_SYN_RSSP1_ACTIVE_DAT    0x49U        /* 系间同步的RSSP1主系数据*/
	typedef struct
	{
		GM_RSSP1_UINT16 index;
		GM_RSSP1_Remote_Dev_AS_State_enum remote_dev_AS_state;
		GM_RSSP1_SACEPID_struct sacepid;
		/* FSFB 连接中TIME信息 */
		GM_RSSP1_UINT32     time_TC;
		GM_RSSP1_UINT32  time_TS[GM_RSSP1_CHECK_CHN_NUM];
		GM_RSSP1_UINT32  time_oldTS[GM_RSSP1_CHECK_CHN_NUM];

		/* FSFB 连接中ENV信息 */
		#ifdef SFM_Part		
		GM_RSSP1_CHN_ENV        env[GM_RSSP1_CHECK_CHN_NUM];	/*jcf:env[]仅与SFM相关，分离两层时需加入条件编译，以防编译出错*/
		#endif
		#ifndef GM_RSSP1_SYN_NO_DATA
		/*PreciTime信息,added by huang 20151215*/
		GM_RSSP1_INT32 PreciTime;			 /* 计算精确的消息有效时间*/
		GM_RSSP1_INT32 SSE_SSR_Delay;
		GM_RSSP1_INT32 TempDiffTime;      /* CR GM6950，历史累计偏移时间，单位:ms */
		GM_RSSP1_UINT32 SINIT_TremCycle;       /* 更新SINITM时的远程节点的RSD的TC */
		GM_RSSP1_UINT32 RxSSRTrem;            /* 接收到的SSR消息的TC，也是Remote_TC */
		GM_RSSP1_UINT32 RxSSRTloc;
		GM_RSSP1_UINT32 TcycleLoc;      /* 本地节点的周期，单位：毫秒 */
		GM_RSSP1_UINT32 TcycleRem;      /* 远程节点的周期，单位：毫秒 */
		#endif
	} GM_RSSP1_PACKED GM_RSSP1_Syn_Info_struct;

	typedef struct
	{
		GM_RSSP1_Syn_Info_struct base_info;
		GM_RSSP1_connection_state_enum state;
		/*GM_RSSP1_UINT8 state_check_counter1;
		GM_RSSP1_UINT8 state_check_counter2;*/
		#ifndef GM_RSSP1_SYN_NO_DATA
		GM_RSSP1_BOOL b_rcvd_new_dat_in_cycle;
		GM_RSSP1_BOOL b_torlerant_dat_valid;
		GM_RSSP1_UINT8 torlerate_cycle_count1;
#ifdef RSSP1_TolerateWithCycle
		GM_RSSP1_INT8 torlerated_cycle_count2;
#endif
		GM_RSSP1_UINT16 dat_count;
		#endif
	}GM_RSSP1_PACKED GM_RSSP1_Syn_Extern_Info_struct;
	/*
	*@63343
	*@brief: VPI项目---主系通过内部发送同步状态给备系
	*/
	GM_RSSP1_BOOL GM_RSSP1_Syn_Send_Fsfb_State_To_Peer(void);
	void GM_RSSP1_Syn_Proc_Rcvd_Syn_Dat(GM_RSSP1_UINT8 *pDat , GM_RSSP1_INT32 dat_len);

	/*
	*@63343
	*@brief: VPI项目---主系转发给备系应用数据
	*@pdata[Out]: 数据地址
	*@len[Out]: 数据长度
	*/
	void GM_RSSP1_Syn_Send_App_To_Peer(GM_RSSP1_UINT8 pdate[], GM_RSSP1_UINT16 len);
	GM_RSSP1_BOOL GM_RSSP1_Syn_Data_Set(GM_RSSP1_UINT8* pdata, GM_RSSP1_UINT32 dataSize);

	/*
	*@63343
	*@brief: 在线节点的获取同步数据函数
	*@pdata[Out]: 数据地址
	*@len[Out]: 数据长度
	*@return: 是否获取成功
	*/
	GM_RSSP1_BOOL GM_RSSP1_Syn_OnLineData_Get(GM_RSSP1_Pointer* pdata, GM_RSSP1_UINT32* len);

	/*
	*@63343
	*@brief: 获取要同步的所有节点的数据地址和大小
	*@pdata[Out]: 数据地址
	*@len[Out]: 数据长度
	*@return: 是否获取成功
	*/
	GM_RSSP1_BOOL GM_RSSP1_Syn_AllData_Get(GM_RSSP1_Pointer* pdata, GM_RSSP1_UINT32* len);

	/*
	*@63343
	*@brief: 单个节点的获取同步数据函数
	*@pdata[Out]: 数据地址
	*@len[Out]: 数据长度
	*@con_index[In]: 节点在配置中的索引（十进制）
	*@dest_addr[In]: 节点配置的远端目的地址（十六进制）
	*@return: 是否获取成功
	*/
	GM_RSSP1_BOOL GM_RSSP1_Syn_Data_OneNode_Get(GM_RSSP1_UINT8* pdata,GM_RSSP1_UINT32* plen,GM_RSSP1_UINT16 con_index,GM_RSSP1_UINT16 dest_addr);
	
	/*
	*@brief: 单个节点的设置同步数据函数
	*@pdata[Out]: 数据地址
	*@len[Out]: 数据长度
	*/
	GM_RSSP1_BOOL GM_RSSP1_Syn_Data_OneNode_Set(GM_RSSP1_UINT8* pdata,GM_RSSP1_UINT32* plen);

#ifdef __cplusplus
}
#endif

#endif
