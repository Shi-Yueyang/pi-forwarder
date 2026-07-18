/**
* @file FSFB_lib_init.h
* @brief 初始化FSFB模块
* @author JiangHongjun
* @date 2010-2-7 16:23:35
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>jianghongjun@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/


#ifndef GM_RSSP1_CFM_Init_H_
#define GM_RSSP1_CFM_Init_H_

#include "GM_RSSP1_Lib_Def.h"
#include "GM_RSSP1_MQ_Interface.h"
#include "GM_RSSP1_CFM.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
    * @brief 文本数据初始化一个FSFB通信模块配置变量
    * Detailed description.
    * @param[in] init_file_path
    * @param[in] pCfg
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Cfg_CFM(GM_RSSP1_INT8 *pIniFileContent , GM_RSSP1_comm_cfg_struct *pCfg);

    /**
    * @brief 文本数据初始化FSFB通信的全局配置
    * Detailed description.
    * @param[in] pCfg
    * @param[in] pIniFileContent
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Global_CFM(GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_INT8 *pIniFileContent);

    /**
    * @brief 文本数据初始化FSFB通信每一个连接的配置
    * Detailed description.
	* @param[in] conn_index
	* @param[in] pCfg
    * @param[in] pIniFileContent
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Connection_CFM(GM_RSSP1_UINT16 conn_index, GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_INT8 *pIniFileContent);


    /**
    * @brief 文本数据初始化FSFB的安全、通信模块。
    * Detailed description.
    * @param[in] file_path
    * @param[in] p_cfg
    * @param[in] pSFM
    * @param[in] pCFM
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_CFM_Init(GM_RSSP1_INT8 *file);

	/**
    * @brief  加载CFM模块数据
    * Detailed description.
	* @param[in] conIndex
	* @param[in] pcfg
    * @param[in] pCFM
    * @return GM_RSSP1_BOOL GM_RSSP1_CFM_Init_Lib
    */
	GM_RSSP1_BOOL GM_RSSP1_CFM_Init_Lib(GM_RSSP1_UINT16 conIndex, GM_RSSP1_comm_cfg_struct *pcfg, GM_RSSP1_CFM_object_struct *pCFM);

    /**
    * @brief 二进制数据初始化一个FSFB通信模块配置变量
    * Detailed description.
    * @param[in] init_file_path
    * @param[in] pCfg
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Cfg_CFM_Addr(GM_RSSP1_UINT8 *pFileContent , GM_RSSP1_comm_cfg_struct *pCfg,GM_RSSP1_BOOL isbinbig);

    /**
    * @brief 二进制数据初始化FSFB通信的全局配置
    * Detailed description.
    * @param[in] pCfg
    * @param[in] pIniFileContent
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Global_CFM_Addr(GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_UINT8 *pFileContent, GM_RSSP1_UINT8 **r_ptr,GM_RSSP1_BOOL isbinbig);

    /**
    * @brief 二进制数据初始化FSFB通信每一个连接的配置
    * Detailed description.
	* @param[in] pCfg
    * @param[in] pIniFileContent
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Connection_CFM_Addr(GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_UINT8 *pFileContent,GM_RSSP1_UINT8 **r_ptr,GM_RSSP1_BOOL isbinbig);


    /**
    * @brief 二进制数据初始化FSFB的安全、通信模块。
    * Detailed description.
    * @param[in] addr
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_CFM_Init_Addr(GM_RSSP1_UINT8 *addr,GM_RSSP1_BOOL isbinbig);

    /**
    * @brief 初始化CFM的所有消息队列,但必须注意消息队列只进行一次
    * Detailed description.
    * @param[in] p_cfg
    * @param[in] pCFM
    * @return GM_RSSP1_BOOL
    */
	GM_RSSP1_BOOL GM_RSSP1_CFM_MQ_Init(GM_RSSP1_comm_cfg_struct *pcfg, GM_RSSP1_CFM_object_struct *pCFM);

    /**
    * @brief 初始化FSFB的安全、通信模块。
    * Detailed description.
    * @param[in] file_path
    * @param[in] p_cfg
    * @param[in] pSFM
    * @param[in] pCFM
    * @return GM_RSSP1_BOOL
    */
	GM_RSSP1_BOOL GM_RSSP1_CFM_Delete(GM_RSSP1_UINT16 con_index, GM_RSSP1_CFM_object_struct *pCFM);
#ifdef __cplusplus
}
#endif /* extern "C" */


#endif
