/**
* @file GM_RSSP1_SFM.h
* @brief SFM模块结构定义、相关接口、处理函数定义
* @author JiangHongjun
* @date 2010-2-7 16:17:44
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>jianghongjun@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/

#ifndef GM_RSSP1_SFM_H_
#define GM_RSSP1_SFM_H_

#include "GM_RSSP1_Time.h"

#include "GM_RSSP1_Lib_Def.h"
#include "GM_RSSP1_Msg_Queue.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct _GM_RSSP1_CHN_ENV_
	{
		GM_RSSP1_UINT32   TC;
		GM_RSSP1_UINT32   TS;
		GM_RSSP1_UINT32   local_TC;       /*最近收到数据包时自己计数器的数值*/
		GM_RSSP1_UINT32   remote_TC;      /*最近收到数据包时数据包中计数器数值*/
		GM_RSSP1_UINT32   sseTS;          /*发送SSE时自己的TS数值*/
		GM_RSSP1_UINT32   sseTC;          /*发送SSE时自己的TC数值*/
		GM_RSSP1_UINT32   dynamicKey;     /*sint+(sid^T(n))*/
#ifdef OFFLINE_TOOL_SUPPORT
		GM_RSSP1_UINT32	dynLocKey;		/*本地宽恕数据值*/
#endif
		GM_RSSP1_UINT8   bSendSSE;       /*是否发送SSE 0x00表示未发送sse, 0xff表示发送了sse但未收到ssr*/
	} GM_RSSP1_PACKED GM_RSSP1_CHN_ENV;



    /** 应用层向SFM注入服务请求原语的结果 */
    typedef enum
    {
        GM_RSSP1_User_Put_Pri_OK                    = 10100, /*< 成功           */
        GM_RSSP1_User_Put_Pri_MQ_FULL               = 10112, /*< 成功，原语请求队列已满，有数据覆盖  */

        GM_RSSP1_User_Put_Pri_INVALID_U2L           = 10102, /*< pU2L为NULL, 或原语内容不正确 */
        GM_RSSP1_User_Put_Pri_LIB_NOT_OPERATIONAL   = 10104, /*< SFM未初始化    */
        GM_RSSP1_User_Put_Pri_INVALID_REQ_TYPE      = 10105, /*< pU2L->Type未知.*/
        GM_RSSP1_User_Put_Pri_INVALID_CONNECTION_ID = 10108  /*< id非法         */
    } GM_RSSP1_User_Put_Pri_To_SFM_Result_enum;

    /** 应用层获取SFM状态事件、数据报告原语的结果 */
    typedef enum
    {
        GM_RSSP1_User_Get_Pri_OK                    = 10200, /*< 成功 */
        GM_RSSP1_User_Get_Pri_UNAVAILABLE           = 10210, /*< 原语无效(类型未知, 数据长度超长等..) */
        GM_RSSP1_User_Get_Pri_INVALID_L2U           = 10230, /*< pL2U 为NULL. */
        GM_RSSP1_User_Get_Pri_LIB_NOT_OPERATIONAL   = 10240  /*< SFM未初始化. */
    } GM_RSSP1_User_Get_Pri_From_SFM_Result_enum;

    /** 一个FSFB通道结构定义 */

    typedef struct
    {
        GM_RSSP1_SFM_chn_cfg_struct        chn_cfg;
#ifdef OFFLINE_TOOL_SUPPORT
        GM_RSSP1_UINT32                      CON_1[GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE];
        GM_RSSP1_UINT32                      CON_2[GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE];
        GM_RSSP1_UINT32                      CON_3[GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE];
        GM_RSSP1_UINT32                      CON_4[GM_RSSP1_MAX_TOLERATE_CYCLE];
#endif
        GM_RSSP1_UINT32                      PREC_SINIT[GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE];
        GM_RSSP1_UINT32                      POST_RXDATA[GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE];
        GM_RSSP1_UINT32                      PREC_FIRSTSINIT;
    } GM_RSSP1_PACKED GM_RSSP1_SFM_fsfb_chn_struct;

    /** 一个FSFB安全连接结构定义 */

    typedef struct
    {
        GM_RSSP1_UINT16							index;
		GM_RSSP1_UINT32                         IsFixed;      /* 当前是否为固定节点（不会释放链接）added by flt 20210426*/
        GM_RSSP1_connection_state_enum			state;                  /**< 连接状态。判断通断的依据：FSFB失步/lifetime时间内仍没有获得校验成功的数据 */
        GM_RSSP1_Remote_Dev_AS_State_enum		remote_dev_AS_state;    /**< 如果收到RSD，根据RSD判断此FSFB连接对应的远程设备是主机还是备机 */
		GM_RSSP1_Remote_Dev_AB_enum				remote_dev_AB;			/**< 远端设备AB系标识，用户可提前配置，并根据收到的RSD判断是否一致 */
        /*GM_RSSP1_INT8							state_check_counter1;*/    /**< 连接检查周期计数 */
        /*GM_RSSP1_INT8							state_check_counter2;*/    /**< 连接检查周期计数.多计数器增加相异性 jianghongjun 20110825 safety enhancement*/

        GM_RSSP1_SACEPID_struct					SaCEPID;                /**< 安全连接ID */
        GM_RSSP1_UINT16							source_addr;            /**< 本地设备源地址。默认采用设备编号 */
        GM_RSSP1_UINT16							dest_addr;              /**< 目的地址。默认采用设备编号 */
		GM_RSSP1_UINT16							deltaTime;              /**< 默认规定 RSD 帧的可同步容忍的最大时序偏差周期数.例如为 2 秒（若系统周期为250ms，则周期数为0x08），超时后需启动请求同步校时机制 */
		GM_RSSP1_UINT16							DelayTime;             /**< 本地接收 RSD 帧的可同步容忍的最大时序偏差周期数.例如为 2 秒（若系统周期为250ms，则周期数为0x08），超时后需启动请求同步校时机制 */
        GM_RSSP1_UINT16							lifeTime;               /**< 网络连接的有效等待时间 */
        GM_RSSP1_UINT8							num_data_ver;           /**< 安全通信时序校验SSR数据的版本号.RSSP1 spe 2.2.2.1.3.3 */
        GM_RSSP1_UINT16							keepIdle;               /**< 连接状态维护数据 */

        GM_RSSP1_TIME							time;                               /**< 时序相关变量管理，如TC等，由主任务周期驱动 */
        GM_RSSP1_SFM_fsfb_chn_struct			fsfb_chn[GM_RSSP1_CHECK_CHN_NUM];       /**< FSFB通道参数，包括从文件直接读入的参数以及经过计算后得到的参数 */
        GM_RSSP1_CHN_ENV						env[GM_RSSP1_CHECK_CHN_NUM];            /**< 通道计算用相关变量 */

        GM_RSSP1_BOOL							b_RSD_Already_Sent;               /**< 本周期已经发送过rsd,不能发超过一包rsd */

        GM_RSSP1_BOOL							b_enable_FSFB_on_chn;   /**< 与对等设备各个UDP通道均使用独立的FSFB。启用后，上层取数据的方式有所不同：属于同一个设备的各UDP通道的FSFB连接，CTCS-ID必须相同!上层提取到一个连接的数据后，将根据清空其他相同的连接中的L2U_dat_Q */
        GM_RSSP1_UINT32							Chn_apply_FSFB_ID;      /**< 启用UDP通道使用独立的FSFB时，此ID相同的各FSFB连接其实属于同一个应用层，可合并，用于上层提取数据。 */
        GM_RSSP1_Msg_Queue_struct				L2U_dat_Q;                  /**< 本安全连接内收到的已解析的数据包.用于单个UDP也使用独立FSFB校验的情况 */
									
        GM_RSSP1_UINT16							torlerate_cycle_cfg;        /**< 宽恕周期：N个主周期内未收到RSD数据，但FSFB连接尚未断开，则将上一次有效数据送往上层。 */
        GM_RSSP1_UINT8							torlerate_cycle_count1;      /**< 宽恕计数。 */
#ifdef RSSP1_TolerateWithCycle
        GM_RSSP1_INT8							torlerated_cycle_count2;     /**< 已宽恕计数。jcf 当节点周期不同于主周期时，需做宽恕次数的判断*/
#endif
        GM_RSSP1_SFM_L2U_pri_struct				last_valid_dat_pri;         /**< 上一次有效的数据包，用于宽恕 */
        GM_RSSP1_BOOL							b_rcvd_new_dat_in_cycle;    /**< 本周期新收到了数据 */
        GM_RSSP1_BOOL							b_torlerant_dat_valid;      /**< 宽恕的数据是否合法 */
        /*added by huang 20151125*/
        GM_RSSP1_INT32							PreciTime;		/* 计算精确的消息有效时间*/
		GM_RSSP1_INT32							SSE_SSR_Delay;
        GM_RSSP1_UINT32							SINIT_TremCycle;       /* 更新SINITM时的远程节点的RSD的TC */
        GM_RSSP1_UINT32							RxSSRTrem;            /* 初值为接收到的SSR消息的TC，也是Remote_TC. 在偏移计算后更新为已计算的远端时间。*/
		GM_RSSP1_UINT32							RxSSRTloc;
        GM_RSSP1_UINT32							TcycleLoc;      /* 本地节点的周期，单位：毫秒 */
        GM_RSSP1_UINT32							TcycleRem;      /* 远程节点的周期，单位：毫秒 */
		GM_RSSP1_INT32							TempDiffTime;      /* CR GM6950，历史累计偏移时间，单位:ms */
        /*the end 20151125*/
    } GM_RSSP1_PACKED GM_RSSP1_SFM_connection_struct;

    /** 安全层模块定义 */

    typedef struct
    {
        GM_RSSP1_library_state_enum         lib_state;
		GM_RSSP1_BOOL						Is_torlerate_process;

		GM_RSSP1_Pointer								U2L_pri_Q_offset;
		GM_RSSP1_Pointer								L2U_pri_Q_offset;

#ifndef CFM_Part
		GM_RSSP1_Pointer								U2L_pri_Q_offset_CFM;
		GM_RSSP1_Pointer								L2U_pri_Q_offset_CFM;
#endif
		GM_RSSP1_UINT16                     connection_nums;
		GM_RSSP1_UINT16                     Max_OnlineNum;
        GM_RSSP1_SFM_connection_struct      connection[GM_RSSP1_MAX_SAFETY_CONNECTION_NUM];
    } GM_RSSP1_PACKED GM_RSSP1_SFM_object_struct;
	
    /************************************************************************/
    /* 接口                                         */
    /************************************************************************/

    /**
    * @brief 获取本机状态:是否是主机
    *
    * Detailed description.
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL SYS_Is_Local_Sys_Active(void);

    /**
    * @brief 获取SFM层内所有通道的安全信息
    *
    * Detailed description.
    * @return GM_RSSP1_SFM_object_struct*
    */
    GM_RSSP1_SFM_object_struct *GM_RSSP1_Get_SFM_Object(void);


    /**
    * @brief  初始化FSFB校验通道的离线运算参数,用于安全码的生成以及更新
    * Detailed description.
    * @param[in] pSFM_conn
    * @param[in] pSFM
    * @return GM_RSSP1_BOOL GM_RSSP1_SFM_Ini_FSFB_Offline_Const
    */
    GM_RSSP1_BOOL GM_RSSP1_SFM_Ini_FSFB_Offline_Const(GM_RSSP1_SFM_connection_struct *pSFM_conn , GM_RSSP1_SFM_object_struct *pSFM);

    /**
    * @brief FSFB校验周期步进
    *
    * Detailed description.
    * @return GM_RSSP1_BOOL
    */
    void GM_RSSP1_SFM_Update_Time(void);

    /**
    * @brief 获取SFM模块的状态
    *
    * Detailed description.
    * @param[in] pSFM
    * @return GM_RSSP1_library_state_enum
    */
    GM_RSSP1_library_state_enum GM_RSSP1_SFM_Get_Library_Status(GM_RSSP1_SFM_object_struct *pSFM);

    /**
    * @brief 根据SACEPID获取一个安全连接
    *
    * Detailed description.
    * @param[in] pSaCEPID
    * @param[in] pSFM
    * @return GM_RSSP1_SFM_connection_struct*
    */
    GM_RSSP1_SFM_connection_struct *GM_RSSP1_SFM_Get_Connection_BY_SaCEPID(GM_RSSP1_SACEPID_struct *pSaCEPID , GM_RSSP1_SFM_object_struct *pSFM);

    /**
    * @brief 判断两个SACEPID是否相等
    *
    * Detailed description.
    * @param[in] pSaCEPID1
    * @param[in] pSaCEPID2
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_SFM_Is_SaCEPID_Equal(GM_RSSP1_SACEPID_struct *pSaCEPID1 , GM_RSSP1_SACEPID_struct *pSaCEPID2);

    /**
    * @brief 应用层向SFM模块放入请求原语
    *
    * Detailed description.
    * @param[in] pU2L
    * @param[in] pSFM
    * @return GM_RSSP1_User_Put_Pri_To_SFM_Result_enum
    */
    GM_RSSP1_User_Put_Pri_To_SFM_Result_enum GM_RSSP1_User_Put_Pri_To_SFM(GM_RSSP1_SFM_U2L_pri_struct *pU2L , GM_RSSP1_SFM_object_struct *pSFM);

    /**
    * @brief 应用层从SFM模块获取原语
    *
    * Detailed description.
    * @param[in] pL2U
    * @param[in] pSFM
    * @return GM_RSSP1_User_Get_Pri_From_SFM_Result_enum
    */
    GM_RSSP1_User_Get_Pri_From_SFM_Result_enum GM_RSSP1_User_Get_Pri_From_SFM(GM_RSSP1_SFM_L2U_pri_struct *pL2U , GM_RSSP1_SFM_object_struct *pSFM);

    /**
    * @brief 应用层发送数据
    *
    * 通过SACEPID标识数据接收方.
    * @param[in] pSaCEPID
    * @param[in] pDat
    * @param[in] dat_len
    * @param[in] sysAORB
    * @param[in] bSysActive
    * @param[in] pMD5
    * @param[in] pSFM
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_User_Send_Dat(GM_RSSP1_SACEPID_struct *pSaCEPID , GM_RSSP1_UINT8 *pDat , GM_RSSP1_UINT32 dat_len , Local_Sys_AB_enum sysAORB , GM_RSSP1_BOOL bSysActive, GM_RSSP1_SFM_object_struct *pSFM);

    /**
    * @brief SFM发送RSD数据包
    *
    * Detailed description.
    * @param[in] pDat_pri
    * @param[in] pSFM_conn
    * @param[in] pSFM
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_SFM_Send_RSD(GM_RSSP1_SFM_U2L_pri_struct *pDat_pri , GM_RSSP1_SFM_connection_struct *pSFM_conn);

    /**
    * @brief SFM发送SSE数据包
    *
    * Detailed description.
    * @param[in] pSFM_conn
    * @param[in] pSFM
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_SFM_Send_SSE(GM_RSSP1_SFM_connection_struct *pSFM_conn);

    /**
    * @brief SFM发送SSR数据包
    *
    * Detailed description.
    * @param[in] rcvd_SEQENQ_1
    * @param[in] rcvd_SEQENQ_2
    * @param[in] rcvd_TC
    * @param[in] pSFM_conn
    * @param[in] pSFM
    * @param[in] pCFM
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_SFM_Send_SSR(GM_RSSP1_UINT32 rcvd_SEQENQ_1 , GM_RSSP1_UINT32 rcvd_SEQENQ_2 , GM_RSSP1_UINT32 rcvd_TC , GM_RSSP1_SFM_connection_struct *pSFM_conn);

	/**
    * @brief 重置SFM层所有安全链接的发送RSD状态,同时将SFM连接未成功建立的活动节点释放所占用的在线通道
    *
    * Detailed description.
    * @param[in] pSFM
    * @return
    */
    void GM_RSSP1_SFM_Reset_Connection_RSD_Sent_Flag(void);

    /**
    * @brief SFM处理应用消息,并将数据打包为RSD消息发送
    *
    * Detailed description.
    * @param[in] pSFM
	* @return
    */
    void GM_RSSP1_SFM_Process_User_Req(void);

    /**
    * @brief SFM向应用层报告安全连接状态(该状态只提供连接正常或外部数据接收的相关活动节点)
    * @param[in] pSFM
    * @param[in] pIndi_pri
	* @return
    */
    void GM_RSSP1_SFM_Report_Connection_State_To_User(GM_RSSP1_CFM_L2U_pri_struct *pIndi_pri);
    
    /**
    * @brief SFM处理CFM收到的消息,对消息进行解包并进行校验或时序对齐逻辑
    *
    * Detailed description.
    * @param[in] pSFM
	* @return
   */
    void GM_RSSP1_SFM_Process_CFM_Ind(GM_RSSP1_SFM_object_struct *pSFM);

    /**
    * @brief SFM监视各个安全连接的状态，并决定是否要进行宽恕。
    *
    * 通过各个连接可配置的时间值，在此时间内未收到校验正确的数据包，即视为断开.
    * @param[in] pSFM
	* @return
    */
    void GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate(void);

    /**
    * @brief SFM处理各个安全连接通道内的数据队列
    *
    * 因为应用层只关心具体的设备，不关系本地设备与远程设备通过几个安全连接通道相连。
    * 启用b_enable_FSFB_on_chn选项UDP通道的FSFB连接，CFM将数据先送到相应FSFB连接中的L2U队列，Dispatch后才送到SFM中的L2U队列
    * 其他正常的FSFB连接，CFM处理完的数据将直接送到SFM的L2U队列.
    * @param[in] pSFM
	* @return
    */
    void GM_RSSP1_SFM_Dispatch_Dat_Indi(GM_RSSP1_SFM_object_struct *pSFM);

    /**
    * @brief SFM处理接收到的SSE包
    *
    * Detailed description.
    * @param[in] pSFM_conn
    * @param[in] pDat_indi
    * @param[in] pSFM
	* @return
    */
    void GM_RSSP1_SFM_Proc_Rcvd_SSE(GM_RSSP1_SFM_connection_struct *pSFM_conn , GM_RSSP1_UINT8 pDatByte[], GM_RSSP1_UINT16 ByteCount);

    /**
    * @brief SFM处理接收到的SSR包
    *
    * Detailed description.
    * @param[in] pSFM_conn
    * @param[in] pDat_indi
    * @param[in] pSFM
	* @return
    */
    void GM_RSSP1_SFM_Proc_Rcvd_SSR(GM_RSSP1_SFM_connection_struct *pSFM_conn , GM_RSSP1_UINT8 pDatByte[], GM_RSSP1_UINT16 ByteCount);

    /**
    * @brief SFM处理接收到的RSD包
    *
    * Detailed description.
    * @param[in] pSFM_conn
    * @param[in] pDat_indi
    * @param[in] pSFM
    * @param[in] pCFM
	* @return
   */
    void GM_RSSP1_SFM_Proc_Rcvd_RSD(GM_RSSP1_SFM_connection_struct *pSFM_conn , GM_RSSP1_UINT8 pDatByte[], GM_RSSP1_UINT16 ByteCount);

    /**
    * @brief SFM周期检查关键配置，防止越界等导致关键安全配置被修改。jianghongjun 20110825 safety enhancement
    *
    * Detailed description.
    * @param[in] pSFM
    
    void GM_RSSP1_Check_Vital_Cfg(GM_RSSP1_SFM_object_struct *pSFM);*/

	/**
	* @brief SFM从CFM队列获取消息事件
	*
	* 此接口由CFM向SFM提供
	* @param[in] pL2U
	* @return GM_RSSP1_SFM_Get_Pri_From_CFM_Result_enum
	*/
	GM_RSSP1_SFM_Get_Pri_From_CFM_Result_enum GM_RSSP1_SFM_Get_Pri_From_CFM(GM_RSSP1_CFM_L2U_pri_struct *pL2U);

	/**
	* @brief SFM向CFM队列写入消息事件
	*
	* 此接口由SFM向CFM推入
	* @param[in] pU2L
	* @param[in] pSFM
	* @return GM_RSSP1_SFM_Put_Pri_To_CFM_Result_enum
	*/
	GM_RSSP1_SFM_Put_Pri_To_CFM_Result_enum GM_RSSP1_SFM_Put_Pri_To_CFM(GM_RSSP1_CFM_U2L_pri_struct *pU2L);

	/**
	* @brief 将SFM中具有相同ID的安全链接进行内部同步,以最新活动节点覆盖其它通道相关信息
	* @param[in] pSFM
	* @return
	*/
	void GM_RSSP1_SFM_Syn_Con(GM_RSSP1_SFM_object_struct *pSFM);

	/**
	* @brief 计算网络延时（SSE_SSR_Delay）和消息的网络时差
	* @param[in] pSFM_conn
	* @return
	*/
	void GM_RSSP1_SetPreciTime(GM_RSSP1_SFM_connection_struct *pSFM_conn);

	void GM_RSSP1_SFM_Process_TxData(GM_RSSP1_SFM_U2L_pri_struct *pri);

	void GM_RSSP1_SFM_Process_RxData(GM_RSSP1_CFM_L2U_type_enum InsType, GM_RSSP1_UINT16 Inx, GM_RSSP1_UINT8 msgInd[], GM_RSSP1_UINT16 msgLen);
#ifdef __cplusplus
}
#endif

#endif
