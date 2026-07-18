/**
* @file GM_RSSP1_CRC.h
* @brief
* @author Hao Liming CRC封装,不太完整,之后再进行封装
* @date 2009-12-22 15:35:29
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>haoliming@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/
#ifndef GM_RSSP1_CRC_H
#define GM_RSSP1_CRC_H

#include "GM_RSSP1_Utils_Base.h"

#ifdef __cplusplus
extern "C"
{
#endif /**< __cplusplus */

    /** @brief 多项式0x11021的正序CRC表 */
    extern const GM_RSSP1_UINT16 GM_RSSP1_CRC16_Table_0x11021_MSB[256U];

    /** @brief 多项式0x11021的逆序CRC表(0x18408) */
    extern const GM_RSSP1_UINT16 GM_RSSP1_CRC16_Table_0x11021_LSB[256U];

    /** @brief 多项式0x10811的正序CRC表 */
    extern const GM_RSSP1_UINT16 GM_RSSP1_CRC16_Table_0x10811_MSB[256U];

    /** @brief 多项式0x10811的逆序CRC表, FSFB2中会用到该CRC表 */
    extern GM_RSSP1_UINT16 GM_RSSP1_CRC16_Table_0x10811_LSB[256U];

    /** @brief FSFB2通道1用到的CRC表0x1100D4E63 */
    extern GM_RSSP1_UINT32 GM_RSSP1_CRC32_Channel1_Table[256U];

    /** @brief FSFB2通道2用到的CRC表0x18CE56011 */
    extern GM_RSSP1_UINT32 GM_RSSP1_CRC32_Channel2_Table[256U];
	
    /** @brief 多项式0x04C11DB7的CRC32颠倒表 */
    extern const GM_RSSP1_UINT32 GM_RSSP1_CRC32_Table_0x04C11DB7_L[256U];

    /**
    * @brief
    *
    * 根据种子和顺序决定CRC校验码
    * @param[in] pBuffer 被校验的数据
    * @param[in] bufferLen 被校验的数据长度(字节)
    * @param[in] pCRC 返回CRC校验码
    * @param[in] pCRCTable CRC表
    * @return GM_BOOL GM_CRC_Calculate_CRC16
    */
    GM_RSSP1_BOOL GM_RSSP1_CRC_Calculate_CRC16(const GM_RSSP1_UINT8 pBuffer[], GM_RSSP1_UINT32 bufferLen, GM_RSSP1_UINT16* pCRC, const GM_RSSP1_UINT16 pCRCTable[]);

    /**
    * @brief GM_RSSP1_CRC_Calculate_CRC32
    *
    * Detailed description.
    * @param[in] pBuffer 被校验的数据
    * @param[in] bufferLen 被校验的数据长度(字节)
    * @param[in] pCRC 返回CRC校验码
    * @param[in] pCRCTable CRC表
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_CRC_Calculate_CRC32(const GM_RSSP1_UINT8 pBuffer[], GM_RSSP1_UINT32 bufferLen, GM_RSSP1_UINT32* pCRC, const GM_RSSP1_UINT32 pCRCTable[]);


#ifdef __cplusplus
}

#endif /**< __cplusplus */

#endif /**< _GM_CRC_H */

