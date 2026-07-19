/**
* @file GM_RSSP1_Utils.h
* @brief 提供了FSFB协议中使用的公共数据
* @author JiangHongjun
* @date 2010-2-7 16:27:30
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>jianghongjun@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/
#ifndef _GM_RSSP1_UTILS_H
#define _GM_RSSP1_UTILS_H

#include "GM_RSSP1_Utils_Other.h"
#include "GM_RSSP1_Memory.h"
#include "GM_RSSP1_CRC.h"
#include "GM_RSSP1_LFSR.h"
#include "GM_RSSP1_Mutex.h"
#include "GM_RSSP1_File.h"
#include "GM_RSSP1_VSN.h"


#ifdef WIN32
#include <winsock2.h>
#include "process.h"
#pragma comment(lib, "Ws2_32.lib")
#elif defined REWORKS_CERTV2
#elif defined NO_SYS
#else
#include <netinet/in.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	#define GM_RSSP1_OK 1
	#define GM_RSSP1_ERROR (-1)
	#define GM_RSSP1_RETURN_RESERVED  (-2)



	/** 常用数据大小限制定义 */
#ifndef GM_RSSP1_CHECK_CHN_NUM
	#define GM_RSSP1_CHECK_CHN_NUM          (2U)       /**< FSFB双通道校验 */
#endif

#ifndef GM_RSSP1_MAX_USER_DAT_LEN
	#define GM_RSSP1_MAX_USER_DAT_LEN       (480U)     /**< 为传输信息 */
#endif

#ifndef GM_RSSP1_MAX_SAFETY_CONNECTION_NUM
	#define GM_RSSP1_MAX_SAFETY_CONNECTION_NUM      (128U)   /**< 最多支持的FSFB安全连接数 */
#endif

#ifndef GM_RSSP1_MAX_ONLINE_CONNECTION_NUM
	#define GM_RSSP1_MAX_ONLINE_CONNECTION_NUM      GM_RSSP1_MAX_SAFETY_CONNECTION_NUM   /**< 最多支持的FSFB安全连接数 */
#endif

#ifndef GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE
	#define GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE       (20U)    /**< 为PREC_SINIT[]和POST_RXDATA[]数组分配的最大容量。实际使用时以各通道配置的deltaTime为依据 */
#endif

#ifndef GM_RSSP1_MAX_LIFETIME
	#define GM_RSSP1_MAX_LIFETIME					(20U)  /*发送SSE等待SSR回复的最大本地周期间隔*/
#endif

#ifndef GM_RSSP1_MAX_DELAYTIME
	#define GM_RSSP1_MAX_DELAYTIME					(20U)  /*本地接收相邻数据包的最大周期间隔*/
#endif

#ifndef GM_RSSP1_MAX_TOLERATE_CYCLE
	#define GM_RSSP1_MAX_TOLERATE_CYCLE             (20U)  /**< FSFB宽恕配置最大不能超过10秒。*/
#endif

#ifndef GM_RSSP1_MAX_PKT_NUM_PER_CYCLE
	#define GM_RSSP1_MAX_PKT_NUM_PER_CYCLE          (5U)     /**< 每周期可能产生的每种数据包最大数量。包括RSD、SSE、SSR */
#endif

	/*    #define GM_RSSP1_MAX_USER_DAT_LEN       478  */  /**< 安全层可接受的来自应用层的最大数据长度RSSP-I p10. 必须为偶数字节 */
	#define GM_RSSP1_MAX_SAFETY_DAT_LEN		(GM_RSSP1_MAX_USER_DAT_LEN+22U)     /**< 通信层可接受的来自安全层的最大数据长度RSSP-I p10. */
	#define GM_RSSP1_MAX_SND_DAT_LEN		GM_RSSP1_MAX_SAFETY_DAT_LEN
	#define GM_RSSP1_CFM_MAX_CHANNEL_NUM	GM_RSSP1_CHECK_CHN_NUM      /**< 底层冗余通道数最大值 */
	
	/*fulintai*/
	#define GM_RSSP1_MAX_LINK_CHANNEL		GM_RSSP1_CFM_MAX_CHANNEL_NUM

	#define GM_RSSP1_MAX_PKT_TOTAL_NUM		(GM_RSSP1_MAX_PKT_NUM_PER_CYCLE*3U)
    	/*VSN_Get 回调函数类型定义added by huang, 20151219*/
	typedef void(*GM_RSSP1_VSN_GET_CALLBACK_FUN)(GM_RSSP1_UINT32* pVSN0, GM_RSSP1_UINT32* pVSN1, GM_RSSP1_UINT32* pVSN2);

#ifdef __cplusplus
}
#endif

#endif

