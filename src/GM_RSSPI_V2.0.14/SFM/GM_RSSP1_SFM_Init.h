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


#ifndef GM_RSSP1_SFM_INIT_H_
#define GM_RSSP1_SFM_INIT_H_

#include "GM_RSSP1_SFM.h"
#include "GM_RSSP1_Lib_Def.h"
#include "GM_RSSP1_MQ_Interface.h"
#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum
	{
		GM_RSSP1_SFM_Bin_Stand				   = 0x1U,
		GM_RSSP1_SFM_Bin_MultLoc			   = 0x6U,
	} GM_RSSP1_SFM_InitBinMode_enum;
    /**
    * @brief 初始化一个FSFB通信模块配置变量
    *
    * Detailed description.
    * @param[in] init_file_path
    * @param[in] pCfg
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Cfg_SFM(GM_RSSP1_INT8 *pIniFileContent, GM_RSSP1_comm_cfg_struct *pCfg);

    /**
    * @brief 初始化FSFB通信的全局配置
    *
    * Detailed description.
    * @param[in] pCfg
    * @param[in] pIniFileContent
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Global_SFM(GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_INT8 *pIniFileContent);

	GM_RSSP1_BOOL GM_RSSP1_Init_ConUnderPack( GM_RSSP1_SFM_connection_cfg_struct* pSFM_conn_cfg, GM_RSSP1_INT8* section_key, GM_RSSP1_INT8 * pIniFileContent);

    /**
    * @brief 初始化FSFB通信每一个连接的配置
    *
    * Detailed description.
	* @param[in] conn_index
	* @param[in] pCfg
    * @param[in] pIniFileContent
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Connection_SFM(GM_RSSP1_UINT16 conn_index, GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_INT8 *pIniFileContent);

	/**
    * @brief  初始化SFM模块
    *
    * Detailed description.
    * @param[in] pcfg
    * @param[in] pSFM
    * @return GM_RSSP1_BOOL GM_RSSP1_SFM_Init_Lib
    */
    GM_RSSP1_BOOL GM_RSSP1_SFM_Init_Lib(GM_RSSP1_comm_cfg_struct *pFSFB_comm_cfg, GM_RSSP1_SFM_object_struct *pSFM);

	GM_RSSP1_BOOL GM_RSSP1_SFM_Init(GM_RSSP1_INT8 *file);

	GM_RSSP1_BOOL GM_RSSP1_SFM_Init_Addr(GM_RSSP1_UINT8 *addr,GM_RSSP1_BOOL isbinbig);

	    /**
    * @brief 初始化一个FSFB通信模块配置变量
    *
    * Detailed description.
    * @param[in] init_file_path
    * @param[in] pCfg
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Cfg_SFM_Addr(GM_RSSP1_UINT8 *pFileContent , GM_RSSP1_comm_cfg_struct *pCfg,GM_RSSP1_BOOL isbinbig);

    /**
    * @brief 初始化FSFB通信的全局配置
    *
    * Detailed description.
    * @param[in] pCfg
    * @param[in] pIniFileContent
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Global_SFM_Addr(GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_UINT8 *pFileContent, GM_RSSP1_UINT8 **r_ptr,GM_RSSP1_BOOL isbinbig);

    /**
    * @brief 初始化FSFB通信每一个连接的配置
    *
    * Detailed description.
	* @param[in] pCfg
    * @param[in] pIniFileContent
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Connection_SFM_Addr(GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_UINT8 *pFileContent, GM_RSSP1_UINT8 **r_ptr,GM_RSSP1_BOOL isbinbig);
	
	GM_RSSP1_BOOL GM_RSSP1_SFM_MQ_Init(GM_RSSP1_comm_cfg_struct *pFSFB_comm_cfg, GM_RSSP1_SFM_object_struct *pSFM);

	GM_RSSP1_BOOL GM_RSSP1_SFM_Delete(GM_RSSP1_UINT16 con_index, GM_RSSP1_SFM_object_struct *pSFM);
#ifdef __cplusplus
}
#endif /* extern "C" */


#endif
