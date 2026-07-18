/**
* @file GM_RSSP1_File.h
* @brief 读取配置文件
* -'='之后的值不能为空char
* -支持';'注释前缀
* -不支持跨行
* @author Hao Liming
* @date 2009-9-18 16:22:00
* @version 0.3
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>haoliming@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>将代码修改成只初始化一次, 添加了读取布尔值、十六进制的方法, 修正了key前缀相同读取错误的bug.</pre>
*/

#ifndef GM_RSSP1_FILE_H
#define GM_RSSP1_FILE_H

#include "GM_RSSP1_Utils_Other.h"
#include "GM_RSSP1_Memory.h"
#ifdef FILE_SYSTEM
#include <ctype.h>

#ifdef __cplusplus
extern "C"
{
#endif /**< __cplusplus */

    /**
    * @brief GM_RSSP1_Ini_File_Load
    *
    * 加载配置文件到fileContent中.
    * @param[in] filePathAndName 配置文件路径
    * @param[out] fileContent 配置文件的内容
    * @param[in] fileContent 存放配置文件内容的大小
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Ini_File_Load(const char* filePathAndName, char fileContent[], int fileContentSize);


    /**
    * @brief GM_RSSP1_Ini_File_Read_String
    *
    * 在section中读取键名key的值到value中.
    * @param[in] section
    * @param[in] key
    * @param[out] value
    * @param[in] size
    * @param[in] iniFileContent
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Ini_File_Read_String(const char *section, const char *key, char value[], int size, GM_RSSP1_INT8 *iniFileContent);

    /**
    * @brief GM_RSSP1_Ini_File_Read_Int
    *
    * 在section中读取键名key的值到value中.
    * @param[in] section
    * @param[in] key
    * @param[out] value
    * @param[in] iniFileContent
    * @return GM_RSSP1_BOOL
    */
	
    GM_RSSP1_BOOL GM_RSSP1_Ini_File_Read_Int(const char *section, const char *key, int* value, GM_RSSP1_INT8 *iniFileContent);

    /**
    * @brief GM_RSSP1_Ini_File_Read_BOOL
    *
    * 在section中读取键名key的值到value中.
    * @param[in] section
    * @param[in] key
    * @param[out] value
    * @param[in] iniFileContent
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Ini_File_Read_BOOL(const char *section, const char *key, GM_RSSP1_BOOL* value, GM_RSSP1_INT8 *iniFileContent);

    /**
    * @brief GM_RSSP1_Get_File_Size
    *
    * 根据文件名获得文件大小.
    * @param[in] filePathAndName
    * @return int
    */
    /*sjz: 在Windows非嵌入式操作系统和Vxworks嵌入式操作系统下，获取的文件大小是相同的，即回车在这两个系统中，均是"\n" */
    long GM_RSSP1_Get_File_Size(const char *filePathAndName);

	    /**
    * @brief 从二进制文件读取配置文件.
    * @param[in] fileName 文件名称.
    * @param[out] pBuf 存放读到的二进制数据的缓冲区.
    * @param[in] bufSize 缓冲区大小
    * @return GM_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Bin_File_Load(const char* fileName, GM_RSSP1_UINT8* pBuf, GM_RSSP1_UINT32 bufSize);



#ifdef __cplusplus
}

#endif /**< __cplusplus */
#endif
#endif /**< _GM_INI_FILE_H */

