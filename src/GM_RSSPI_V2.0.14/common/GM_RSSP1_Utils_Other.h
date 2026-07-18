/**
 * Copyright(C) 2012,CASCO Company 保留所有权利。( All rights reserved. )
 * 
 * @file:GM_RSSP1_Utils_Other.h
 * @brief:
 * @auther:Hao Liming
 * @create date:2010年10月30日
 * @modify by:wtt
 * @Modify info:2012年12月6日
 */


#ifndef GM_RSSP1_UTILS_OTHER_H
#define GM_RSSP1_UTILS_OTHER_H

#include "GM_RSSP1_Utils_Base.h"

#ifdef __cplusplus
extern "C"
{
#endif /**< __cplusplus */

    /**
    * @brief GM_RSSP1_atoi
    *
    * 字符串转换成整数的封装，可转换的数字范围[-2147483648，2147483647]
    * @param[in] pBuf 要转换的字符串
    * @param[in] bufSize 要转换的字符串长度
    * @param[out] pVal 转换后的值
    * @return GM_RSSP1_BOOL 转换是否成功
    */
    GM_RSSP1_BOOL GM_RSSP1_atoi(const char* pBuf, GM_RSSP1_UINT32 bufSize, int* pVal);

	/**
    * @brief GM_RSSP1_atoui
    *
    * 字符串转换成整数的封装，可转换的数字范围[0，4294967295]
    * @param[in] pBuf 要转换的字符串
    * @param[in] bufSize 要转换的字符串长度
    * @param[out] pVal 转换后的值
    * @return GM_RSSP1_BOOL 转换是否成功
    */
    GM_RSSP1_BOOL GM_RSSP1_atoui(const char* pBuf, GM_RSSP1_UINT32 bufSize, unsigned int* pVal);

    /**
    * @brief GM_RSSP1_Log_Msg
    *
    * 封装了logMsg.
    * @param[in] fmt
    * @param[in] arg1
    * @param[in] arg2
    * @param[in] arg3
    * @param[in] arg4
    * @param[in] arg5
    * @param[in] arg6
    */
	#ifndef NO_SYS
    void GM_RSSP1_Log_Msg(int level, const char* fmt, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
	#endif
	
	/**
    * @brief
	*
	* 应用错误回调函数.
	* @param[in] errNo
	* @param[in] fmt
	* @param[in] arg1
	* @param[in] arg2
	* @param[in] arg3
	* @param[in] arg4
	* @param[in] arg5
	* @param[in] arg6
	*/
    typedef void (*GM_RSSP1_LOG_CALL_BACK_FUN)(unsigned long errNo, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);


	/**
    * @brief GM_RSSP1_Log_Err
    *
    * GM内部错误回调函数.
	* @param[in] errNo
    * @param[in] fmt
    * @param[in] arg1
    * @param[in] arg2
	* @param[in] arg3
	* @param[in] arg4
	* @param[in] arg5
	* @param[in] arg6
	*/
	void GM_RSSP1_Log_Err(unsigned long errNo, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);

			/**
    * @brief GM_FSFB2_Log_Usr_Callback_Init
    *
    * GM_FSFB2错误回调函数初始化
	* @param[in] Log_err_fun
	*/
	GM_RSSP1_BOOL GM_RSSP1_Log_Usr_Callback_Init(GM_RSSP1_LOG_CALL_BACK_FUN log_err_fun);


	    /**
    * @brief GM_RSSP1_Code_Is_Big_Endian
    *
    * 判断本机是主机字节序还是网络字节序.
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Code_Is_Big_Endian(void);

    /**
    * @brief GM_RSSP1_Code_Big_Little_Endian16
    *
    * 大小端之间的转换.
    * @param[in] x
    * @return GM_RSSP1_UINT16
    */
    GM_RSSP1_UINT16 GM_RSSP1_Code_Big_Little_Endian16(GM_RSSP1_UINT16 x);

    /**
    * @brief GM_RSSP1_Code_Big_Little_Endian32
    *
    * 大小端之间的转换.
    * @param[in] x
    * @return GM_RSSP1_UINT32
    */
    GM_RSSP1_UINT32 GM_RSSP1_Code_Big_Little_Endian32(GM_RSSP1_UINT32 x);


	
    extern GM_RSSP1_LOG_CALL_BACK_FUN GM_RSSP1_Log_Usr;

#ifdef __cplusplus
}

#endif /**< __cplusplus */

#endif /**< _GM_UTILS_BASE_H */

