#ifndef GM_RSSP1_APP_INTERFACE_H
#define GM_RSSP1_APP_INTERFACE_H

#include "GM_RSSP1_Define.h"

#ifdef SFM_Part
#include "GM_RSSP1_SFM_Init.h"
#include "GM_RSSP1_SFM_Interface.h"
#endif
#ifdef CFM_Part
#include "GM_RSSP1_CFM_Init.h"
#include "GM_RSSP1_CFM_Interface.h"
#endif

#include "GM_RSSP1_MQ_Interface.h"

#ifdef __cplusplus
extern "C"
{
#endif

	#define    INTERFACE_TYPE_RSSP1         0x11U        /* RSSP-I接口相关 */
	#define    INTERFACE_DAT_TYPE_RSSP1_DAT         0x11U        /* RSSP1应用数据 */
	#define    INTERFACE_DAT_TYPE_RSSP1_COM_STATE   0x13U        /* RSSP1连接状态 */
	#define    INTERFACE_DAT_TYPE_RSSP1_WARNING     0x15U        /* RSSP1报警信息 */


	typedef enum
	{
		ABAS_Type_A_AS = 0x1111U, /**< 自己为A机主机, Peer为备机 */
		ABAS_Type_A_SA = 0x2222U, /**< 自己为A机备机, Peer为主机 */
		ABAS_Type_A_AA = 0x3333U, /**< 自己为A机主机, Peer为主机 */
		ABAS_Type_A_SS = 0x4444U, /**< 自己为A机备机, Peer为备机 */
		ABAS_Type_B_AS = 0x5555U, /**< 自己为B机主机, Peer为备机 */
		ABAS_Type_B_SA = 0x6666U, /**< 自己为B机备机, Peer为主机 */
		ABAS_Type_B_AA = 0x7777U, /**< 自己为B机主机, Peer为主机 */
		ABAS_Type_B_SS = 0x8888U, /**< 自己为B机备机, Peer为备机 */
		ASAS_Type_Unknow = 0x9999U
	} GM_VLE_ABAS_Type_enum;


	typedef enum
	{
		Layer_Type_SFM = 0x11U,
		Layer_Type_CFM = 0x55U,
		Layer_Type_Unknow = 0xAAU
	} GM_RSSP1_Layer_Type_enum;

	typedef GM_RSSP1_UINT16(*GM_RSSP_GET_ABAS_FUN)(void); /**< 获取应用主备状态 */
	
#ifdef PF2oo3
	#define RSSP1_MAX_MESSAGE_SIZE         1460U

	typedef struct
	{
		GM_RSSP1_UINT16 PeripheralNumber; /*peripheral number identifier*/
		GM_RSSP1_UINT16 MsgSize; /*the message size*/
		GM_RSSP1_UINT8  Message[RSSP1_MAX_MESSAGE_SIZE]; /*the message size. Array of byte. Max size defined by MAXMSGSIZE*/
	}RSSP_Message_t;

#endif

	/**
    * @brief 处理RSSP-I模块的初始化,bin初始化接口
    * Detailed description.
	* @param[in] fun			获取系统主备状态的功能函数
	* @param[in] SFM_Addr		SFM配置信息的首地址
    * @param[in] CFM_Addr		CFM配置信息的首地址
    * @param[in] VSN_Get_Fun	获取系统时间的功能函数
	* @return GM_RSSP1_BOOL		初始化结果
    */
	GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Init_Addr(GM_RSSP_GET_ABAS_FUN fun, GM_RSSP1_UINT8* SFM_Addr, GM_RSSP1_UINT8* CFM_Addr, GM_RSSP1_VSN_GET_CALLBACK_FUN VSN_Get_Fun,GM_RSSP1_BOOL isbinbig);
	
	/**
    * @brief 处理RSSP-I模块的初始化,文本初始化接口
    * Detailed description.
    * @param[in] fun			获取系统主备状态的功能函数
    * @param[in] path			文件路径或已加载文件内存的首地址
    * @param[in] is_path		传入指针参数path是否是路径
    * @param[in] VSN_Get_Fun	获取系统时间的功能函数
	* @return GM_RSSP1_BOOL		初始化结果
    */
    GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Init(GM_RSSP_GET_ABAS_FUN fun, char* path, GM_RSSP1_BOOL is_path, GM_RSSP1_VSN_GET_CALLBACK_FUN VSN_Get_Fun);

    /**
    * @brief 获取设备信息
    * Detailed description.
    * @param[out] p_sys_a_b	A/B机信息
    * @param[out] p_bactive	主备信息,True为主
	* @return GM_RSSP1_BOOL		获取操作结果
    */
    GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Get_Syn_AB_AS_Info(Local_Sys_AB_enum *p_sys_a_b , GM_RSSP1_BOOL *p_bactive);
    /**
    * @brief 获取主备机信息
    * Detailed description.
	* @return GM_RSSP1_BOOL 主备机状态(True为主,False为备)
    */
    GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Is_Local_ACTIVE(void);

	/**
	* @brief 清空消息队列.
	* @return GM_RSSP1_BOOL		获取操作结果
	*/
	GM_RSSP1_BOOL GM_RSSP1_APP_Clear_Msg_Queue(void);

	/**
	* @brief 安全层与应用层接口,提供接收数据
	* @param[out] buf	缓存消息内容
	* @param[out] Src	当前消息的来源地址
	* @param[out] len	当前消息包长度
	* @param[out] count	剩余消息数
	* @return GM_RSSP1_INT8		获取操作结果(1,获取到安全消息;0,获取到无关消息;-1,未获取到消息)
	*/
	GM_RSSP1_INT8 GM_RSSP1_APP_Interface_Rcv_App_Dat(GM_RSSP1_UINT8* buf, GM_RSSP1_UINT32* Src, GM_RSSP1_UINT32* len, GM_RSSP1_INT32* count);

	/**
	* @brief 安全层与应用层接口,用于发送用户数据
	* @param[in] p_dat		待发送数据的内存首地址
	* @param[in] len		待发送数据的长度
	* @param[in] dest_addr	待发送数据的远端地址
	* @return GM_RSSP1_BOOL	发送结果(True:写入协议队列成功;False:失败,有可能长度过长,地址未索引到或其它随机错误)
	*/
	GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Send_App_Dat(GM_RSSP1_UINT8* pData, GM_RSSP1_INT16 len, GM_RSSP1_UINT16 dest_addr);
	
	/**
	* @brief 安全层与应用层接口,获得PreciTime(对应数据接收间隔和发送间隔差,亦网络传输偏差)
	* @param[in] destAddr	对应的地址安全链接
	* @return GM_RSSP1_BOOL	发送结果(True:写入协议队列成功;False:失败,有可能长度过长,地址未索引到或其它随机错误)
	*/
	GM_RSSP1_INT32 GM_RSSP1_APP_Interface_GetPreciTime(GM_RSSP1_UINT16 destAddr);

#ifdef OFFLINE_TOOL_SUPPORT
	/**
	* @brief 初始化离线工具后的BIN文件
	* @param[in] path		文件路径或已加载文件内存的首地址
	* @param[in] is_path	传入指针参数path是否是路径
	* @param[in] isbinbig   BIN文件是否以大端形式存储
	* @return GM_RSSP1_BOOL	初始化结果
	*/
	GM_RSSP1_BOOL GM_RSSP1_APP_Interface_CalcCNF_Init(char* path, GM_RSSP1_BOOL is_path,GM_RSSP1_BOOL isbinbig);

#endif

#ifdef GM_RSSP1_SYSCKW_FROM_USER
	/**
	* @brief 获取应用提供的校核字,参与校验信息生成
	* @param[in] sysckw_1	系统校核字1^Ts1
	* @param[in] sysckw_2	系统校核字2^Ts2
	* @return
	*/
	void GM_RSSP1_APP_Interface_Get_SysCKW(GM_RSSP1_UINT32 sysckw_1, GM_RSSP1_UINT32 sysckw_2);
#endif

#ifdef PF2oo3
	/**
	* @brief 
	* CFM安全层与通信层接口,提供接收数据
	* @param[in] p_dat
	* @param[in] dat_len
	*/
	GM_RSSP1_BOOL GM_RSSP1_2oo3_Conn_Send_Dat(RSSP_Message_t* msg, GM_RSSP1_UINT8 index, GM_RSSP1_UINT8 chn_index);

	/**
	* @brief 
	* CFM安全层与通信层接口,发送用户数据
	* @param[in] p_dat
	* @param[in] dat_len
	*/
	GM_RSSP1_BOOL GM_RSSP1_2oo3_Conn_Recv_Dat(RSSP_Message_t* msg, GM_RSSP1_UINT8 index, GM_RSSP1_UINT8 chn_index);
#endif

	/*若使用RSSP1的VSN，应用调用的初始化和更新VSN的接口, added by huang 20151219*/
	/**
	* @brief RSSP-I的时间戳初始化
	* @return
	*/
	void GM_RSSP1_APP_Interface_VSN_Init(void);

	/**
	* @brief RSSP-I的时间戳更新
	* @return
	*/
	void GM_RSSP1_APP_Interface_VSN_Update(void);

	/**
	* @brief 提供中断安全链接的接口
	* @param[in] destAddr	目标链接对应的地址对象
	* @return GM_RSSP1_BOOL	操作结果(True:中断成功;False:中断失败)
	*/
	GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Disconnect(GM_RSSP1_UINT16 destAddr);

	/**
	* @brief 处理RSSP-I模块的加载,文本初始化接口(重复加载接口)
	* @param[in] path		文件路径或已加载文件内存的首地址
	* @param[in] is_path	传入指针参数path是否是路径
	* @return GM_RSSP1_BOOL	初始化结果
	*/
	GM_RSSP1_BOOL GM_RSSP1_APP_Interface_ReLoad(char* path, GM_RSSP1_BOOL is_path);

	/**
	* @brief 删除RSSP-I离线数据配置
	* @param[in] partType	删除对应层所在的配置
	* @param[in] input_1	当Type为SFM层,则为远端地址;当Type为CFM层,input_2为0,则为内部索引号.否则为目标dst_IP
	* @param[in] input_2	当Type为CFM层,input_2非0,则dst_port
	* @return GM_RSSP1_BOOL	初始化结果
	*/
	void GM_RSSP1_APP_Interface_Remove(GM_RSSP1_Layer_Type_enum partType, GM_RSSP1_UINT32 input_1, GM_RSSP1_UINT32 input_2);

	/**
	* @brief 获取所有在线节点的个数和索引
	* @param[out] OnlineArray	缓存个数和队列的内存地址
	* @param[in] Len	缓存地址的有效长度
	* @return GM_RSSP1_BOOL	获取结果
	*/
	GM_RSSP1_BOOL GM_RSSP1_APP_Get_OnlineArray(GM_RSSP1_UINT16* OnlineArray, GM_RSSP1_UINT16 Len);


	/**
	* @brief 将指定节点索引更新到活动队列中
	* @param[out] OnlineArray	缓存个数和队列的内存地址
	* @return GM_RSSP1_BOOL	更新结果
	*/
	GM_RSSP1_BOOL GM_RSSP1_APP_Set_Active_WithOnline(GM_RSSP1_UINT16* OnlineArray);

	GM_RSSP1_BOOL GM_RSSP1_APP_Interface_ReLoad_Addr(GM_RSSP1_UINT8* SFM_Addr, GM_RSSP1_UINT8* CFM_Addr,GM_RSSP1_BOOL isbinbig);

	void GM_RSSP1_Set_LogLevel(GM_RSSP1_UINT16 lev);

	void GM_RSSP1_APP_Interface_RxPrc(void);

	void GM_RSSP1_APP_Interface_TxPrc(void);

	GM_RSSP1_BOOL GM_RSSP1_APP_SetCrcmAddr(GM_RSSP1_UINT8* baseAddr);
#ifdef __cplusplus
}

#endif

#endif /* _CILK_APP2_H_ */
