/**
* @file GM_RSSP1_LFSR.h
* @brief 采用小端编码
* @author Hao Liming
* @date 2010-7-29 15:28:03
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>haoliming@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/

#ifndef GM_RSSP1_LFSR_H
#define GM_RSSP1_LFSR_H

#include "GM_RSSP1_Utils_Base.h"

#ifdef __cplusplus
extern "C"
{
#endif /**< __cplusplus */

    /**
    * @brief FSFB2通道1 PD运算用到的左移LFSR表0x10FC22F87
    *
    */
    extern GM_RSSP1_UINT32 GM_RSSP1_LFSR_LEFT_CHN1_TABLE[256U];

    /**
    * @brief FSFB2通道1 PD运算用到的右移LFSR表0x10FC22F87
    *
    */
    extern GM_RSSP1_UINT32 GM_RSSP1_LFSR_RIGHT_CHN1_TABLE[256U];

    /**
    * @brief FSFB2通道2 PD运算用到的左移LFSR表0x1C3E887E1
    *
    */
    extern GM_RSSP1_UINT32 GM_RSSP1_LFSR_LEFT_CHN2_TABLE[256U];

    /**
    * @brief FSFB2通道2 PD运算用到的右移LFSR表0x1C3E887E1
    *
    */
    extern GM_RSSP1_UINT32 GM_RSSP1_LFSR_RIGHT_CHN2_TABLE[256U];

    typedef struct
    {
        GM_RSSP1_UINT32 reg;
        const GM_RSSP1_UINT32 *leftTable;
        const GM_RSSP1_UINT32 *rightTable;
    }GM_RSSP1_PACKED GM_RSSP1_LFSR;

    /**
    * @brief GM_RSSP1_LFSR_Read
    *
    * 获取运算结果.
    * @param[in] This LFSR运算中间数据结构
    * @param[out] pData 寄存器中的值
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_LFSR_Read(GM_RSSP1_LFSR* This, GM_RSSP1_UINT32* pData);

    /**
    * @brief GM_RSSP1_LFSR_Init
    *
    * 初始化LFSR运算中间数据结构.
    * @param[in] This LFSR运算中间数据结构
    * @param[in] pLeftTable 左移运算表
    * @param[in] pRightTable 右移运算表
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_LFSR_Init(GM_RSSP1_LFSR* This, const GM_RSSP1_UINT32* pLeftTable, const GM_RSSP1_UINT32* pRightTable);

    /**
    * @brief GM_RSSP1_LFSR_Load
    *
    * 加载进行LFSR运算的数据.
    * @param[in] This LFSR运算中间数据结构
    * @param[in] data 运算数据
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_LFSR_Load(GM_RSSP1_LFSR* This, GM_RSSP1_UINT32 data);

    /**
    * @brief GM_RSSP1_LFSR_Add
    *
    * 进行Add运算.
    * @param[in] This LFSR运算中间数据结构
    * @param[in] data 运算数据
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_LFSR_Add(GM_RSSP1_LFSR* This, GM_RSSP1_UINT32 data);

    /**
    * @brief GM_RSSP1_LFSR_Post
    *
    * 进行Post运算.
    * @param[in] This LFSR运算中间数据结构
    * @param[in] data 运算数据
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_LFSR_Post(GM_RSSP1_LFSR* This, GM_RSSP1_UINT32 data);

#ifdef __cplusplus
}

#endif /**< __cplusplus */

#endif /**< _GM_RSSP1_LFSR_H */



