#ifndef GM_RSSP1_SFM_INTERFACE_H
#define GM_RSSP1_SFM_INTERFACE_H


#include "GM_RSSP1_Utils.h"
#include "GM_RSSP1_SFM.h"

#ifdef __cplusplus
extern "C"
{
#endif
    /**
    * @brief GM_RSSP1_SFM_Interface_Proc_Send
    *该函数接口用于SFM层发送处理过程函数（先接收，后发送流程）
    * @return
    */
	void GM_RSSP1_SFM_Interface_Proc_Send(void);

    /**
    * @brief GM_RSSP1_SFM_Interface_Remove
    *该函数接口用于SFM层接收处理过程函数（先接收，后发送流程）
    * @return
    */
	void GM_RSSP1_SFM_Interface_Proc_Recv(void);

    /**
    * @brief GM_RSSP1_SFM_Interface_Remove
    *该函数接口用于移除对应远端节点的本地SFM的静态配置相关信息
    * @param[in] Dst_Addr 远端地址
    * @return
    */
	void GM_RSSP1_SFM_Interface_Remove(GM_RSSP1_UINT16 Dst_Addr);
#ifdef __cplusplus
}
#endif

#endif