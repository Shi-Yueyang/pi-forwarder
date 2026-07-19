/**
* @file GM_RSSP1_VSN.h
* @brief VSN算法实现,AlgorithmA和AlgorithmB两种算法均在此文件中实现
* @author Hao Liming
* @date 2010-4-4 11:11:55
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>haoliming@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/
#ifndef GM_RSSP1_VSN_H
#define GM_RSSP1_VSN_H

#include "GM_RSSP1_Utils_Base.h"
#include "GM_RSSP1_LFSR.h"
#include "GM_RSSP1_Memory.h"
#include "GM_RSSP1_Utils_Other.h"

#ifdef __cplusplus
extern "C"
{
#endif /**< __cplusplus */

#define GM_RSSP1_VSN_M01 (0x12340234U) /**< VSN1的掩码  */
#define GM_RSSP1_VSN_M02 (0x43211328U) /**< VSN2的掩码 */

#define GM_RSSP1_VSN_SEED1 (0x0B7A7124U) /**< VSN1的初始值*/
#define GM_RSSP1_VSN_SEED2 (0x1C4269F4U) /**< VSN2的初始值*/

    /**
    * @brief GM_RSSP1_VSN_Init
    *
    * 初始化VSN.  注意：正常情况下VSN只能初始化一次，第二次调用初始化函数，会使VSN的值被重置为初始态。
    */
    GM_RSSP1_BOOL GM_RSSP1_VSN_Init(void);

    /**
    * @brief GM_RSSP1_VSN_Update
    *
    * 每个周期更新一次VSN.
    */
    void GM_RSSP1_VSN_Update(void);

    /**
    * @brief GM_VSN_Get
    *
    * 获得当前周期的VSN.
    * @param[out] pVSN0
    * @param[out] pVSN1
    * @param[out] pVSN2
    */
    void GM_RSSP1_VSN_Get(GM_RSSP1_UINT32* pVSN0, GM_RSSP1_UINT32* pVSN1, GM_RSSP1_UINT32* pVSN2);

    /**
    * @brief GM_RSSP1_VSN_Set
    *
    * 设置当前周期的VSN.
    * @param[in] VSN0
    * @param[in] VSN1
    * @param[in] VSN2
    */
    void GM_RSSP1_VSN_Set(GM_RSSP1_UINT32 VSN0, GM_RSSP1_UINT32 VSN1, GM_RSSP1_UINT32 VSN2);

#ifdef __cplusplus
}

#endif /**< __cplusplus */

#endif /**< _GM_VSN_H */

