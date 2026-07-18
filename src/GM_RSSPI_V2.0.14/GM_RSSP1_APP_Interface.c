#include "GM_RSSP1_APP_Interface.h"
#ifdef SFM_Part
	#include "GM_RSSP1_Syn.h"
#endif

GM_RSSP_GET_ABAS_FUN GM_RSSP_Resource_Get_ABAS = NULL;

GM_RSSP1_VSN_GET_CALLBACK_FUN GM_RSSP1_VSN_Get_Callback_Fun = NULL;	/*added by huang 20151219*/

GM_RSSP1_BOOL RSSP1_Initialized = GM_RSSP1_FALSE;
GM_RSSP1_UINT16 g_Max_ConnctNum = 0U;
GM_RSSP1_UINT32 g_Cfg_IsFixNode = 0U;
extern RSSP1_MQ_Inter_struct g_SFM_MQ;
GM_RSSP1_UINT8* g_pCrcmAddr = NULL;
GM_RSSP1_UINT16 g_CrcmIndex = 0xFFFFU;


extern GM_RSSP1_UINT16 g_P1_LogLevel;
#ifdef CFM_Part
extern RSSP1_MQ_Inter_struct g_CFM_MQ;

extern RSSP1_MQ_LINK_struct g_Link_MQ;
#endif

GM_RSSP1_BOOL GM_RSSP1_Send_App_Dat(GM_RSSP1_UINT8 *pDat , GM_RSSP1_UINT32 dat_len);

#ifdef GM_RSSP1_SYSCKW_FROM_USER
extern GM_RSSP1_UINT32 App_SYSCKW[2];
#endif

/* CR: 11036019*/
GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Init_Addr(GM_RSSP_GET_ABAS_FUN fun, GM_RSSP1_UINT8* SFM_Addr, GM_RSSP1_UINT8* CFM_Addr, GM_RSSP1_VSN_GET_CALLBACK_FUN VSN_Get_Fun,GM_RSSP1_BOOL isbinbig)
{
	/*初始化离线链表*/
	GM_RSSP1_InitSeqArray(OffLine_Object, GM_RSSP1_MAX_SAFETY_CONNECTION_NUM);
#ifdef SFM_Part

	GM_RSSP1_memset(&g_SFM_MQ, 0, sizeof(g_SFM_MQ));

	if (GM_RSSP1_FALSE == GM_RSSP1_SFM_Init_Addr(SFM_Addr,isbinbig))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_Init_Addr_SFM_FAIL, 0, 0, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"RSSP-I SFM init FAIL!\n", 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		GM_RSSP1_Log_Msg(4,"RSSP-I SFM init OK.\n", 0, 0, 0, 0, 0, 0);
	}
#endif

#ifdef CFM_Part

	GM_RSSP1_memset(&g_CFM_MQ, 0, sizeof(g_CFM_MQ));
	GM_RSSP1_memset(&g_Link_MQ, 0, sizeof(g_Link_MQ));

	if (GM_RSSP1_FALSE == GM_RSSP1_CFM_Init_Addr(CFM_Addr,isbinbig))
	{
		GM_RSSP1_Log_Msg(1,"RSSP-I CFM init FAIL!\n", 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		GM_RSSP1_Log_Msg(4,"RSSP-I CFM init OK.\n", 0, 0, 0, 0, 0, 0);
	}
#endif

	

	GM_RSSP_Resource_Get_ABAS = (GM_RSSP_GET_ABAS_FUN)fun;

	/* added by huang 20151219*/
	if ( VSN_Get_Fun != NULL)
	{
		GM_RSSP1_VSN_Get_Callback_Fun = VSN_Get_Fun;
	}
	else
	{
		GM_RSSP1_VSN_Get_Callback_Fun = GM_RSSP1_VSN_Get;
	}
	/*the end 20151219*/

	RSSP1_Initialized = GM_RSSP1_TRUE;

	return GM_RSSP1_TRUE;
}


/* CR: 11036019*/
GM_RSSP1_BOOL GM_RSSP1_APP_SetCrcmAddr(GM_RSSP1_UINT8* baseAddr)
{
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;

	if (NULL != baseAddr) 
	{
		g_pCrcmAddr = baseAddr;
		g_CrcmIndex = 0U;
		rt = GM_RSSP1_TRUE;
	}

	return rt;
}


#ifdef FILE_SYSTEM
GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Init(GM_RSSP_GET_ABAS_FUN fun, char* path, GM_RSSP1_BOOL is_path, GM_RSSP1_VSN_GET_CALLBACK_FUN VSN_Get_Fun)
{
	GM_RSSP1_INT32 file_len = -1;
	GM_RSSP1_INT8 *pIniFileContent = NULL; /**< 存储Ini文件内容, 动态分配, 使用完之后释放 */
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;

	if(NULL == path)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_Init_Param_Point_Error, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	
	if (GM_RSSP1_TRUE == is_path)
	{
		/* 为读文件分配内存 */
		file_len = GM_RSSP1_Get_File_Size(path);

		if (0>=file_len)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_Init_FILE_Size_Error, 0, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}

		pIniFileContent = (GM_RSSP1_INT8 *)GM_RSSP1_malloc((size_t)file_len + (size_t)1);

		if (NULL == pIniFileContent)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_Init_Memory_Error, 0, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}

		GM_RSSP1_memset((void*)pIniFileContent, 0U, ((size_t)file_len + (size_t)1));

		/** @brief 初始化ini文件, 即读取其中的内容 */

		rt = GM_RSSP1_Ini_File_Load(path, (char*)pIniFileContent, (int)file_len + 1);

		if (GM_RSSP1_FALSE == rt)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_Init_FILE_Load_Error, 0, 0, 0, 0, 0, 0);
			GM_RSSP1_free((void*)pIniFileContent);
			pIniFileContent = NULL;
			return GM_RSSP1_FALSE;
		}
	}
	else
	{
		pIniFileContent = (GM_RSSP1_INT8*) path;
	}

	/*初始化离线链表*/
	GM_RSSP1_InitSeqArray(OffLine_Object, GM_RSSP1_MAX_SAFETY_CONNECTION_NUM);
#ifdef SFM_Part

	GM_RSSP1_memset(&g_SFM_MQ, 0, sizeof(g_SFM_MQ));

	if (GM_RSSP1_FALSE == GM_RSSP1_SFM_Init(pIniFileContent))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_Init_SFM_Init_Error, 0, 0, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"RSSP-I SFM init FAIL!\n", 0, 0, 0, 0, 0, 0);
		if (GM_RSSP1_TRUE == is_path)
		{
			GM_RSSP1_free((void*)pIniFileContent);
			pIniFileContent = NULL;
		}
		return GM_RSSP1_FALSE;
	}
	else
	{
		GM_RSSP1_Log_Msg(4,"RSSP-I SFM init OK.\n", 0, 0, 0, 0, 0, 0);
	}
#endif

#ifdef CFM_Part

	GM_RSSP1_memset(&g_CFM_MQ, 0, sizeof(g_CFM_MQ));
	GM_RSSP1_memset(&g_Link_MQ, 0, sizeof(g_Link_MQ));

	if (GM_RSSP1_FALSE == GM_RSSP1_CFM_Init(pIniFileContent))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_Init_CFM_Init_Error, 0, 0, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"RSSP-I CFM init FAIL!\n", 0, 0, 0, 0, 0, 0);
		if (GM_RSSP1_TRUE == is_path)
		{
			GM_RSSP1_free((void*)pIniFileContent);
			pIniFileContent = NULL;
		}
		return GM_RSSP1_FALSE;
	}
	else
	{
		GM_RSSP1_Log_Msg(4,"RSSP-I CFM init OK.\n", 0, 0, 0, 0, 0, 0);
	}
#endif

	if (GM_RSSP1_TRUE == is_path)
	{
		GM_RSSP1_free((void*)pIniFileContent);
		pIniFileContent = NULL;
	}

	GM_RSSP_Resource_Get_ABAS = (GM_RSSP_GET_ABAS_FUN)fun;

	/* added by huang 20151219*/
	if ( VSN_Get_Fun != NULL)
	{
		GM_RSSP1_VSN_Get_Callback_Fun = VSN_Get_Fun;
	}
	else
	{
		GM_RSSP1_VSN_Get_Callback_Fun = GM_RSSP1_VSN_Get;
	}
	/*the end 20151219*/

	RSSP1_Initialized = GM_RSSP1_TRUE;

	return GM_RSSP1_TRUE;
}
#endif

GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Get_Syn_AB_AS_Info(Local_Sys_AB_enum *p_sys_a_b , GM_RSSP1_BOOL *p_bactive)
{
    GM_RSSP1_UINT16 t_usIPS_ROLE = 0U;
	t_usIPS_ROLE = GM_RSSP_Resource_Get_ABAS();

    if (((GM_RSSP1_UINT16)ABAS_Type_A_AS == t_usIPS_ROLE) || ((GM_RSSP1_UINT16)ABAS_Type_A_AA == t_usIPS_ROLE) ||
            ((GM_RSSP1_UINT16)ABAS_Type_B_AS == t_usIPS_ROLE) || ((GM_RSSP1_UINT16)ABAS_Type_B_AA == t_usIPS_ROLE))
    {

        if (((GM_RSSP1_UINT16)ABAS_Type_A_AS == t_usIPS_ROLE) || ((GM_RSSP1_UINT16)ABAS_Type_A_AA == t_usIPS_ROLE))
        {
            *p_sys_a_b = GM_RSSP1_Local_Sys_A;
        }
        else
        {
            *p_sys_a_b = GM_RSSP1_Local_Sys_B;
        }

        *p_bactive = GM_RSSP1_TRUE;
    }
    else if (((GM_RSSP1_UINT16)ABAS_Type_A_SA == t_usIPS_ROLE) || ((GM_RSSP1_UINT16)ABAS_Type_A_SS == t_usIPS_ROLE) ||
             ((GM_RSSP1_UINT16)ABAS_Type_B_SA == t_usIPS_ROLE) || ((GM_RSSP1_UINT16)ABAS_Type_B_SS == t_usIPS_ROLE))
    {
        if (((GM_RSSP1_UINT16)ABAS_Type_A_SA == t_usIPS_ROLE) || ((GM_RSSP1_UINT16)ABAS_Type_A_SS == t_usIPS_ROLE))
        {
            *p_sys_a_b = GM_RSSP1_Local_Sys_A;
        }
        else
        {
            *p_sys_a_b = GM_RSSP1_Local_Sys_B;
        }

        *p_bactive = GM_RSSP1_FALSE;
    }
    else
    {
        return GM_RSSP1_FALSE;
    }

    return GM_RSSP1_TRUE;
}


GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Is_Local_ACTIVE(void)
{
    Local_Sys_AB_enum sys_a_b = GM_RSSP1_Local_Sys_UNLOWN;
    GM_RSSP1_BOOL b_active = GM_RSSP1_FALSE;

    if (GM_RSSP1_TRUE == GM_RSSP1_APP_Interface_Get_Syn_AB_AS_Info(&sys_a_b , &b_active))
    {
        return b_active;
    }
    else
    {
        /* 异常，视为备机 */
        return GM_RSSP1_FALSE;
    }

}
#ifdef FILE_SYSTEM
GM_RSSP1_BOOL GM_RSSP1_APP_Interface_ReLoad(char* path, GM_RSSP1_BOOL is_path)
{
	GM_RSSP1_INT32 file_len = -1;
	GM_RSSP1_INT8 *pIniFileContent = NULL; /**< 存储Ini文件内容, 动态分配, 使用完之后释放 */
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;

	if(NULL == path)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_ReLoad_Param_Point_Error, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_TRUE == is_path)
	{
		/* 为读文件分配内存 */
		file_len = GM_RSSP1_Get_File_Size(path);

		if (0>=file_len)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_ReLoad_FILE_Size_Error, 0, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}

		pIniFileContent = (GM_RSSP1_INT8 *)GM_RSSP1_malloc((size_t)file_len + (size_t)1);

		if (NULL == pIniFileContent)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_ReLoad_Memory_Error, 0, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}

		GM_RSSP1_memset((void*)pIniFileContent, 0U, ((size_t)file_len + (size_t)1));

		/** @brief 初始化ini文件, 即读取其中的内容 */

		rt = GM_RSSP1_Ini_File_Load(path, (char*)pIniFileContent, (int)file_len + 1);

		if (GM_RSSP1_FALSE == rt)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_ReLoad_FILE_Load_Error, 0, 0, 0, 0, 0, 0);
			GM_RSSP1_free((void*)pIniFileContent);
			pIniFileContent = NULL;
			return GM_RSSP1_FALSE;
		}
	}
	else
	{
		pIniFileContent = (GM_RSSP1_INT8*) path;
	}

#ifdef SFM_Part
	if (GM_RSSP1_FALSE == GM_RSSP1_SFM_Init(pIniFileContent))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_ReLoad_SFM_Init_Error, 0, 0, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"RSSP-I SFM Reload FAIL!\n", 0, 0, 0, 0, 0, 0);
		if (GM_RSSP1_TRUE == is_path)
		{
			GM_RSSP1_free((void*)pIniFileContent);
			pIniFileContent = NULL;
		}
		return GM_RSSP1_FALSE;
	}
	else
	{
		GM_RSSP1_Log_Msg(4,"RSSP-I SFM Reload OK.\n", 0, 0, 0, 0, 0, 0);
	}
#endif

#ifdef CFM_Part
	if (GM_RSSP1_FALSE == GM_RSSP1_CFM_Init(pIniFileContent))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_ReLoad_CFM_Init_Error, 0, 0, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"RSSP-I CFM Reload FAIL!\n", 0, 0, 0, 0, 0, 0);
		if (GM_RSSP1_TRUE == is_path)
		{
			GM_RSSP1_free((void*)pIniFileContent);
			pIniFileContent = NULL;
		}
		return GM_RSSP1_FALSE;
	}
	else
	{
		GM_RSSP1_Log_Msg(4,"RSSP-I CFM Reload OK.\n", 0, 0, 0, 0, 0, 0);
	}
#endif

	if (GM_RSSP1_TRUE == is_path)
	{
		GM_RSSP1_free((void*)pIniFileContent);
		pIniFileContent = NULL;
	}
	return GM_RSSP1_TRUE;
}
#endif

GM_RSSP1_BOOL GM_RSSP1_APP_Interface_ReLoad_Addr(GM_RSSP1_UINT8* SFM_Addr, GM_RSSP1_UINT8* CFM_Addr,GM_RSSP1_BOOL isbinbig)
{
#ifdef SFM_Part
	if (GM_RSSP1_FALSE == GM_RSSP1_SFM_Init_Addr(SFM_Addr,isbinbig))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_Init_Addr_SFM_FAIL, 0, 0, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"RSSP-I SFM Reload FAIL!\n", 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		GM_RSSP1_Log_Msg(4,"RSSP-I SFM Reload OK.\n", 0, 0, 0, 0, 0, 0);
	}
#endif

#ifdef CFM_Part
	if (GM_RSSP1_FALSE == GM_RSSP1_CFM_Init_Addr(CFM_Addr, isbinbig))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_Init_Addr_CFM_FAIL, 0, 0, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"RSSP-I CFM Reload FAIL!\n", 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		GM_RSSP1_Log_Msg(4,"RSSP-I CFM Reload OK.\n", 0, 0, 0, 0, 0, 0);
	}
#endif

	return GM_RSSP1_TRUE;
}

void GM_RSSP1_APP_Interface_Remove(GM_RSSP1_Layer_Type_enum partType, GM_RSSP1_UINT32 input_1, GM_RSSP1_UINT32 input_2)
{
	switch (partType)
	{
#ifdef SFM_Part
	case Layer_Type_SFM:/*删除SFM的配置*/
		GM_RSSP1_SFM_Interface_Remove((GM_RSSP1_UINT16)input_1);
		break;
#endif

#ifdef CFM_Part
	case Layer_Type_CFM:/*删除CFM的配置*/

		if (input_2 == 0U)
		{
			GM_RSSP1_CFM_Interface_RemoveWithIndex((GM_RSSP1_UINT16)input_1);
		}
		else
		{
			GM_RSSP1_CFM_Interface_RemoveWithAddr(input_1, input_2);
		}
		break;
#endif

	default:
		/*do nothing*/
		break;
	}
}


#ifdef SFM_Part
/**
* @brief 按队列结构重组的用户数据写入内部队列
* @param[in] p_dat		待写入数据的内存首地址
* @param[in] dat_len	数据的长度
* @return GM_RSSP1_BOOL	操作结果(True:写入协议队列成功;False:失败,有可能长度过长,地址未索引到或其它随机错误)
*/
GM_RSSP1_BOOL GM_RSSP1_Send_App_Dat(GM_RSSP1_UINT8 *pDat , GM_RSSP1_UINT32 dat_len)
{
	GM_RSSP1_SACEPID_struct dest_sacepid = {0};
	Local_Sys_AB_enum sys_a_b = GM_RSSP1_Local_Sys_UNLOWN;
	GM_RSSP1_BOOL b_active = GM_RSSP1_TRUE;
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	GM_RSSP1_UINT16 crc16 = 0U;
	GM_RSSP1_UINT16 crc16_in = 0U;
	GM_RSSP1_UINT16 con_index = 0xFFFFU;
	GM_RSSP1_UINT16 HashIndex = 0xFFFFU;

	if ((NULL == pDat) || (dat_len>(GM_RSSP1_MAX_SFM_DAT_LEN+4U)))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Send_App_Dat_Num_Param_Error, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	dest_sacepid.source_addr    = ((GM_RSSP1_UINT16)(*pDat) << 8U) + ((GM_RSSP1_UINT16)*(pDat + 1U));
	dest_sacepid.dest_addr      = (GM_RSSP1_UINT16)((GM_RSSP1_UINT32)*(pDat + 2U) << 8U) + ((GM_RSSP1_UINT16)*(pDat + 3U));

	if (GM_RSSP1_TRUE == GM_RSSP1_APP_Interface_Get_Syn_AB_AS_Info(&sys_a_b , &b_active))
	{

		rt = GM_RSSP1_User_Send_Dat(&dest_sacepid, (pDat+4U), (dat_len-4U), sys_a_b, b_active, GM_RSSP1_Get_SFM_Object());
		return rt;
	}
	else
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Send_App_Dat_Num_Result_No_Match, dest_sacepid.source_addr, dest_sacepid.dest_addr, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
}

GM_RSSP1_BOOL GM_RSSP1_APP_Clear_Msg_Queue(void)
{
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_object_struct *pSFM_FSFB = NULL;
	register GM_RSSP1_SFM_connection_struct *pSFM_FSFB_con = NULL;

	/*clear FSFB SFM queue*/
	pSFM_FSFB = GM_RSSP1_Get_SFM_Object();

	rt = FSFB_Msg_Queue_Clear(&(g_SFM_MQ.SND_MQ));
	if(GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Clear_Msg_Queue_Result_No_Match, 0xF0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	rt = FSFB_Msg_Queue_Clear(&(g_SFM_MQ.RCV_MQ));   
	if(GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Clear_Msg_Queue_Result_No_Match, 0xF1, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	/*every connection Q*/
	GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 1U);
	for(l_index = 0U; l_index< pSFM_FSFB->connection_nums; l_index++)
	{
		con_index = GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 0U);
		/*if (0xFFFFU != con_index)*/
		if(con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
		{
			if(GM_RSSP1_TRUE  == pSFM_FSFB->connection[con_index].b_enable_FSFB_on_chn)
			{
				pSFM_FSFB_con = &(pSFM_FSFB->connection[con_index]);
				rt =  FSFB_Msg_Queue_Clear(&(pSFM_FSFB_con->L2U_dat_Q));
				if(GM_RSSP1_FALSE == rt)
				{
					GM_RSSP1_Log_Usr(GM_RSSP1_APP_Clear_Msg_Queue_Result_No_Match, con_index, 0, 0, 0, 0, 0);
					return GM_RSSP1_FALSE;
				}
			}
		}
	}
	return GM_RSSP1_TRUE;
}


GM_RSSP1_INT8 GM_RSSP1_APP_Interface_Rcv_App_Dat(GM_RSSP1_UINT8* buf, GM_RSSP1_UINT32* Src, GM_RSSP1_UINT32* len, GM_RSSP1_INT32* count)
{
    GM_RSSP1_INT32 i = 0;
    GM_RSSP1_SFM_L2U_pri_struct fsfb_pri;
    GM_RSSP1_UINT8 dat[GM_MAX_QUEUE_ELE_LEN];   /* 预留，防止VLE端对SDM打包P1状态时，增加包头长度后超过GM_MAX_QUEUE_ELE_LEN */
    GM_RSSP1_UINT8 tmp_state_dat[GM_MAX_QUEUE_ELE_LEN];
    GM_RSSP1_UINT8 warning_dat[GM_MAX_QUEUE_ELE_LEN];
    GM_RSSP1_INT32 warning_dat_len = 0;
    GM_RSSP1_INT32 dat_len = 0;
    GM_RSSP1_INT32 state_len = 0;
    GM_RSSP1_INT32 p1_pkt_sum= 0;
    GM_RSSP1_INT8 rt = -1;

    if ((buf == NULL)||(Src == NULL)||(len == NULL)||(count == NULL))
    {
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_Rcv_App_Dat_Param_Point_Error, (int)buf, (int)Src, (int)len, 0, 0, 0);
        return -2;
    }

    GM_RSSP1_memset((void*)dat , 0U, sizeof(dat));
    GM_RSSP1_memset((void*)warning_dat, 0U, sizeof(warning_dat));
    warning_dat[0U]  = INTERFACE_TYPE_RSSP1;
    warning_dat[1U]  = INTERFACE_DAT_TYPE_RSSP1_WARNING;
    warning_dat_len = 0;
    *len = 0U;

    *count = 0;
    if (GM_RSSP1_TRUE == FSFB_Msg_Queue_Get(&(g_SFM_MQ.RCV_MQ) , &fsfb_pri))
    {
        *count = (GM_RSSP1_INT32)FSFB_Get_Queue_Count(&(g_SFM_MQ.RCV_MQ));
        switch (fsfb_pri.type)
        {

        case GM_RSSP1_SFM_Data_Ind:/**本地接收的安全数据**/
            if (fsfb_pri.as.data_indi.bytes_count + 6U > (GM_RSSP1_UINT16)sizeof(dat))
            {
                GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_Rcv_App_Dat_MSG_Size_Error, fsfb_pri.as.data_indi.bytes_count, 0, 0, 0, 0, 0);
                return -2;
            }

            if (GM_RSSP1_SYN_DEST_ADDR != fsfb_pri.SaCEPID.dest_addr)
            {
                /* dat from remote devices.report to VLE */
                dat[0]   = INTERFACE_TYPE_RSSP1;
                dat[1]   = INTERFACE_DAT_TYPE_RSSP1_DAT;
                dat[2]   = (GM_RSSP1_UINT8)((fsfb_pri.SaCEPID.source_addr & 0xff00U)>> 8U);
                dat[3]   = (GM_RSSP1_UINT8)(fsfb_pri.SaCEPID.source_addr & 0x00ffU) ;
                dat[4]   = (GM_RSSP1_UINT8)((fsfb_pri.SaCEPID.dest_addr & 0xff00U)>> 8U);
                dat[5]   = (GM_RSSP1_UINT8)(fsfb_pri.SaCEPID.dest_addr & 0x00ffU) ;
                GM_RSSP1_memcpy((void*)(dat + 6U), (const void*)fsfb_pri.as.data_indi.byte , (size_t)fsfb_pri.as.data_indi.bytes_count);
#ifdef InterfaceData_With_CRC
                GM_RSSP1_memcpy((void*)(dat + 6U + fsfb_pri.as.data_indi.bytes_count), (const void*)fsfb_pri.as.data_indi.CRC, (size_t)(sizeof(GM_RSSP1_UINT32)*2U));
                *len = 6U + fsfb_pri.as.data_indi.bytes_count+sizeof(GM_RSSP1_UINT32)*2U;
#else
                *len = 6U + fsfb_pri.as.data_indi.bytes_count;
#endif
                *Src = fsfb_pri.SaCEPID.dest_addr;
                GM_RSSP1_memcpy((void*)buf, (const void*)dat, (size_t)*len);
            }
            else
            {
                /* syn dat from peer
                GM_RSSP1_Syn_Proc_Rcvd_Syn_Dat(fsfb_pri.as.data_indi.byte , fsfb_pri.as.data_indi.bytes_count , pFSFB_SFM);*/
            }
            rt = 1;
            break;

        case GM_RSSP1_SFM_Active_Data_Ind:/**当前备系从主系通过0xFFFF通道接收的安全数据**/
            /* dat from remote devices.report to VLE */
            if (GM_RSSP1_TRUE== GM_RSSP1_APP_Interface_Is_Local_ACTIVE())
            {
                    return 0;
            }
            dat[0U]   = INTERFACE_TYPE_RSSP1;
            dat[1U]   = INTERFACE_DAT_TYPE_RSSP1_DAT;
            dat[2U]   = (GM_RSSP1_UINT8)((fsfb_pri.SaCEPID.source_addr & 0xff00U) >> 8U);
            dat[3U]   = (GM_RSSP1_UINT8)(fsfb_pri.SaCEPID.source_addr & 0x00ffU) ;
            dat[4U]   = (GM_RSSP1_UINT8)((fsfb_pri.SaCEPID.dest_addr & 0xff00U) >> 8U);
            dat[5U]   = (GM_RSSP1_UINT8)(fsfb_pri.SaCEPID.dest_addr & 0x00ffU) ;
            GM_RSSP1_memcpy((void*)(dat + 6U) , (const void*)fsfb_pri.as.data_indi.byte , (size_t)fsfb_pri.as.data_indi.bytes_count);

            *len = 6U + fsfb_pri.as.data_indi.bytes_count;	
            *Src = fsfb_pri.SaCEPID.dest_addr;
            GM_RSSP1_memcpy((void*)buf, (const void*)dat, ((size_t)6 + (size_t)fsfb_pri.as.data_indi.bytes_count));					
			rt = 1;
			break;

        case GM_RSSP1_SFM_Connection_State_Indi:/**状态包,包含SFM层和CFM的通道状态**/

            state_len = 0;
            for (i = 0 ; i < (GM_RSSP1_INT32)fsfb_pri.as.connection_state.connection_num ; ++i)
            {
                tmp_state_dat[state_len] = (GM_RSSP1_UINT8)((fsfb_pri.as.connection_state.connection_states[i].index & 0xff00U) >> 8U);
				state_len++;
                tmp_state_dat[state_len] = (GM_RSSP1_UINT8)(fsfb_pri.as.connection_state.connection_states[i].index & 0xffU);
				state_len++;
                tmp_state_dat[state_len] = (GM_RSSP1_UINT8)((fsfb_pri.as.connection_state.connection_states[i].sacepid.dest_addr & 0xff00U) >> 8U);
				state_len++;
                tmp_state_dat[state_len] = (GM_RSSP1_UINT8)((fsfb_pri.as.connection_state.connection_states[i].sacepid.dest_addr & 0xffU));
				state_len++;

                tmp_state_dat[state_len] = (GM_RSSP1_UINT8)((fsfb_pri.as.connection_state.connection_states[i].state & 0xff000000U) >> 24U);
				state_len++;
                tmp_state_dat[state_len] = (GM_RSSP1_UINT8)((fsfb_pri.as.connection_state.connection_states[i].state & 0x00ff0000U) >> 16U);
				state_len++;
                tmp_state_dat[state_len] = (GM_RSSP1_UINT8)((fsfb_pri.as.connection_state.connection_states[i].state & 0x0000ff00U) >> 8U);
				state_len++;
                tmp_state_dat[state_len] = (GM_RSSP1_UINT8)((fsfb_pri.as.connection_state.connection_states[i].state & 0x000000ffU));
				state_len++;

                if((state_len + 6 + 8> (GM_RSSP1_INT32)sizeof(dat)) || (i == (GM_RSSP1_INT32)(fsfb_pri.as.connection_state.connection_num - 1U)))
                {
                    dat_len = 0;
                    dat[dat_len] = INTERFACE_TYPE_RSSP1;
					dat_len++;
                    dat[dat_len] = INTERFACE_DAT_TYPE_RSSP1_COM_STATE;
					dat_len++;
                    dat[dat_len] = (GM_RSSP1_UINT8)((((GM_RSSP1_UINT32)state_len/8U) & 0xff000000U)>>24U);
					dat_len++;
                    dat[dat_len] = (GM_RSSP1_UINT8)((((GM_RSSP1_UINT32)state_len/8U) & 0x00ff0000U)>>16U);
					dat_len++;
                    dat[dat_len] = (GM_RSSP1_UINT8)((((GM_RSSP1_UINT32)state_len/8U) & 0x0000ff00U)>>8U);
					dat_len++;
                    dat[dat_len] = (GM_RSSP1_UINT8)(((GM_RSSP1_UINT32)state_len/8U) & 0x000000ffU);
					dat_len++;
                    GM_RSSP1_memcpy((void*)(dat + dat_len) , (const void*)tmp_state_dat , (size_t)state_len);
                    dat_len += state_len;

                    *Src = 0x11U;
                    *len = (GM_RSSP1_UINT32)dat_len;
                    GM_RSSP1_memcpy((void*)buf, (const void*)dat, (size_t)dat_len);		

                    state_len = 0;
                    GM_RSSP1_memset((void*)tmp_state_dat , 0U , sizeof(tmp_state_dat));
                }
            }
            rt = 1;
            break;

        case GM_RSSP1_SFM_Warning_Indi:
/*             处理报警信息。收集本周期内产生的所有报警信息，集中发送给vle 
            if(warning_dat_len + 2 + 2 <= sizeof(warning_dat))
            {
                warning_dat[1 + warning_dat_len] = fsfb_pri.as.warning.index;
                warning_dat[1 + warning_dat_len + 1] = fsfb_pri.as.warning.type;
                warning_dat_len += 2;
            }
*/			rt = 0;
			break;


        default:
            GM_RSSP1_Log_Msg(2,"RSSP-I USER recvd unknow SFM indi:0x%x.Qcnt:%d.ID:(0x%X,0x%X).\n", (int)(fsfb_pri.type), *(int*)count, (int)(fsfb_pri.SaCEPID.source_addr), (int)(fsfb_pri.SaCEPID.dest_addr), 0, 0);
			rt = 0;
			break;
        }
    }

    *count = (GM_RSSP1_INT32)FSFB_Get_Queue_Count(&(g_SFM_MQ.RCV_MQ));

    return rt;
}

GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Send_App_Dat(GM_RSSP1_UINT8* pData, GM_RSSP1_INT16 len, GM_RSSP1_UINT16 dest_addr)
{
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_object_struct *pFSFB_SFM = NULL;
	GM_RSSP1_UINT8 dat[GM_MAX_QUEUE_ELE_LEN];   /* 预留，防止VLE端对SDM打包P1状态时，增加包头长度后超过GM_MAX_QUEUE_ELE_LEN */
	GM_RSSP1_UINT16 i = 0U;
	GM_RSSP1_UINT16 l_source_addr = 0U;
	GM_RSSP1_UINT16 l_index = 0xFFFFU;
	GM_RSSP1_UINT16 Hashindex = 0xFFFFU;
	GM_RSSP1_INT16 dat_len = 0;

	if ((len<0)||(len>(GM_RSSP1_INT16)GM_RSSP1_MAX_USER_DAT_LEN))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_Send_App_Dat_Param_Val_Error, len, GM_RSSP1_MAX_USER_DAT_LEN, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	pFSFB_SFM = GM_RSSP1_Get_SFM_Object();

	GM_RSSP1_memset((void*)dat, 0U, (size_t)GM_MAX_QUEUE_ELE_LEN);
	
	rt = GM_RSSP1_Hash_Search_CnfDst((GM_RSSP1_UINT32)dest_addr, &l_index, &Hashindex);
	if (GM_RSSP1_TRUE == rt)
	{

		l_source_addr = pFSFB_SFM->connection[l_index].source_addr;
		dat[0U]   = (GM_RSSP1_UINT8)((l_source_addr & 0xff00U) >> 8U);
		dat[1U]   = ((GM_RSSP1_UINT8)l_source_addr & 0x00ffU) ;
		dat[2U]   = (GM_RSSP1_UINT8)((dest_addr & 0xff00U) >> 8U);
		dat[3U]   = ((GM_RSSP1_UINT8)dest_addr & 0x00ffU) ;
		GM_RSSP1_memcpy((void*)(dat + 4U), (const void*)pData , (size_t)len);
		dat_len = 4 + len;
		rt = GM_RSSP1_Send_App_Dat(dat , (GM_RSSP1_UINT32)dat_len);

	}

	return rt;
}

/*added by huang 20151201, destAddr 可以从GM_RSSP1_APP_Interface_Rcv_App_Dat()输出的形参Src得到*/
GM_RSSP1_INT32 GM_RSSP1_APP_Interface_GetPreciTime(GM_RSSP1_UINT16 destAddr)
{
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_UINT16 connIndex = 0xFFFFU;
	GM_RSSP1_UINT16 HashIndex = 0xFFFFU;
	GM_RSSP1_SFM_object_struct *pFSFB_SFM = NULL;

	pFSFB_SFM = GM_RSSP1_Get_SFM_Object();

	bRt = GM_RSSP1_Hash_Search_CnfDst((GM_RSSP1_UINT32)destAddr, &connIndex, &HashIndex);
	if (GM_RSSP1_TRUE == bRt)
	{
		return pFSFB_SFM->connection[connIndex].PreciTime;
	}
	
	return -1;
}

GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Disconnect(GM_RSSP1_UINT16 destAddr)
{
	GM_RSSP1_SFM_object_struct *pSFM = NULL;
	GM_RSSP1_SFM_connection_struct *pSFM_conn = NULL;
	GM_RSSP1_UINT8 ConNum = 8U;
	GM_RSSP1_UINT16 i = 0U;
	GM_RSSP1_UINT16 connIndex = 0xFFFFU;
	GM_RSSP1_UINT16 HashIndex = 0xFFFFU;
	GM_RSSP1_UINT16 indexArray[8U] = {0U};
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;

	pSFM = GM_RSSP1_Get_SFM_Object();

	if (NULL != pSFM)
	{
		GM_RSSP1_memset((void*)&indexArray, 0xFFU, sizeof(indexArray));

		rt = GM_RSSP1_Hash_CnfDst_ResArray((GM_RSSP1_UINT32)destAddr, indexArray, &ConNum);
		if (GM_RSSP1_TRUE == rt)
		{
			for (i=0U; i<(ConNum/2U); ++i)
			{
				connIndex = indexArray[i*2U];
				HashIndex = indexArray[i*2U + 1U];

				pSFM_conn = &(pSFM->connection[connIndex]);
				pSFM_conn->state = GM_RSSP1_Layer_State_Free;
				GM_RSSP1_Hash_Insert_Online(HashIndex, GM_RSSP1_FALSE);
				/* reset tolerate vars */
				pSFM_conn->b_rcvd_new_dat_in_cycle = GM_RSSP1_FALSE;
				GM_RSSP1_memset((void*)&(pSFM_conn->last_valid_dat_pri) , 0U, sizeof(pSFM_conn->last_valid_dat_pri));
				pSFM_conn->b_torlerant_dat_valid = GM_RSSP1_FALSE;
			}
			rt = GM_RSSP1_TRUE;
		}
	}

	return rt;
}

GM_RSSP1_BOOL GM_RSSP1_APP_Get_OnlineArray(GM_RSSP1_UINT16* OnlineArray, GM_RSSP1_UINT16 Len)/*CR:GM00004827 get SFM active index*/
{
	GM_RSSP1_UINT16 i = 0U;
	GM_RSSP1_UINT16 onlineNum = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;

	if (NULL != OnlineArray)
	{
		onlineNum = GM_RSSP1_GetSeqAmount(OnLine_Object);
		GM_RSSP1_GetSeqIndexPositive(OnLine_Object, 1U);

		if ((onlineNum+1U)*2U <= Len)
		{
			for (i=0U; i<onlineNum; ++i)
			{
				con_index = GM_RSSP1_GetSeqIndexPositive(OnLine_Object, 0U);
				if (0xFFFF != con_index)
				{
					*(OnlineArray+i+1U) = con_index;
					bRt = GM_RSSP1_TRUE;
				}
				else
				{
					bRt = GM_RSSP1_FALSE;
					break;
				}
			}
			*OnlineArray = i;
		}
	}

	return bRt;
}

#ifdef OFFLINE_TOOL_SUPPORT
GM_RSSP1_BOOL GM_RSSP1_APP_Interface_CalcCNF_Init(char* path, GM_RSSP1_BOOL is_path,GM_RSSP1_BOOL isbinbig)
{
	GM_RSSP1_INT32 file_len = -1;
	GM_RSSP1_INT8 *pIniFileContent = NULL; /**< 存储Ini文件内容, 动态分配, 使用完之后释放 */
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	GM_RSSP1_UINT32 len = 0U;
	GM_RSSP1_UINT8 i = 0U;
	GM_RSSP1_UINT8 j = 0U;
	GM_RSSP1_UINT8 k = 0U;
	GM_RSSP1_SFM_object_struct *pFSFB_SFM = GM_RSSP1_Get_SFM_Object();
	GM_RSSP1_BOOL isbig = GM_RSSP1_Code_Is_Big_Endian();


	if ((NULL == path)||(NULL == pFSFB_SFM)||(GM_RSSP1_FALSE == RSSP1_Initialized))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_CalcCNF_Init_Param_Point_Error, path, pFSFB_SFM, RSSP1_Initialized, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_TRUE == is_path)
	{
		/* 为读文件分配内存 */
		file_len = GM_RSSP1_Get_File_Size(path);

		if (0>=file_len)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_CalcCNF_Init_FILE_Size_Error, 0, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}

		pIniFileContent = (char *)GM_RSSP1_malloc((size_t)(file_len + 1U));

		if (NULL == pIniFileContent)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_CalcCNF_Init_Memory_Error, 0, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}

		GM_RSSP1_memset((void*)pIniFileContent, 0U, (size_t)(file_len + 1U));


		rt = GM_RSSP1_Bin_File_Load(path, pIniFileContent, (GM_RSSP1_UINT32)(file_len + 1U));

		if (GM_RSSP1_FALSE == rt)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_CalcCNF_Init_FILE_Load_Error, 0, 0, 0, 0, 0, 0);
			GM_RSSP1_free((void*)pIniFileContent);
			pIniFileContent = NULL;
			return GM_RSSP1_FALSE;
		}
	}
	else
	{
		pIniFileContent = (GM_RSSP1_INT8*) path;
		file_len = *(GM_RSSP1_INT32*)pIniFileContent;
		pIniFileContent += sizeof(GM_RSSP1_INT32);
	}

	for (i=0U; i<GM_RSSP1_CHECK_CHN_NUM; ++i)
	{
		for (j=0U; j<pFSFB_SFM->connection_nums; ++j)
		{
			len += sizeof(GM_RSSP1_UINT32)* (pFSFB_SFM->connection[j].deltaTime * 3U + pFSFB_SFM->connection[j].torlerate_cycle_cfg - 1U);
			if ((GM_RSSP1_INT32)len>file_len)
			{
				GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_CalcCNF_Init_FILE_Size_Error, len, file_len, 0, 0, 0, 0);
				return GM_RSSP1_FALSE;
			}
			else
 			{
				GM_RSSP1_memcpy((void*)&(pFSFB_SFM->connection[j].fsfb_chn[i].CON_1), (const void*)pIniFileContent, (size_t)(sizeof(GM_RSSP1_UINT32)*pFSFB_SFM->connection[j].deltaTime));
				pIniFileContent += sizeof(GM_RSSP1_UINT32)*pFSFB_SFM->connection[j].deltaTime;

				GM_RSSP1_memcpy((void*)&(pFSFB_SFM->connection[j].fsfb_chn[i].CON_2), (const void*)pIniFileContent, (size_t)(sizeof(GM_RSSP1_UINT32)*pFSFB_SFM->connection[j].deltaTime));
				pIniFileContent += sizeof(GM_RSSP1_UINT32)*pFSFB_SFM->connection[j].deltaTime;

				GM_RSSP1_memcpy((void*)&(pFSFB_SFM->connection[j].fsfb_chn[i].CON_3), (const void*)pIniFileContent, (size_t)(sizeof(GM_RSSP1_UINT32)*pFSFB_SFM->connection[j].deltaTime));
				pIniFileContent += sizeof(GM_RSSP1_UINT32)*pFSFB_SFM->connection[j].deltaTime;

				GM_RSSP1_memcpy((void*)&(pFSFB_SFM->connection[j].fsfb_chn[i].CON_4), (const void*)pIniFileContent, (size_t)(sizeof(GM_RSSP1_UINT32)* (pFSFB_SFM->connection[j].torlerate_cycle_cfg - 1U)));
				pIniFileContent += sizeof(GM_RSSP1_UINT32)*(pFSFB_SFM->connection[j].torlerate_cycle_cfg-1U);
			}
		}
	}
	if (isbig != isbinbig)
	{
		for (i=0U; i<GM_RSSP1_CHECK_CHN_NUM; ++i)
		{
			for (j=0U; j<pFSFB_SFM->connection_nums; ++j)
			{
				for (k=0U; k<pFSFB_SFM->connection[j].deltaTime; ++k)
				{
					pFSFB_SFM->connection[j].fsfb_chn[i].CON_1[k] = GM_RSSP1_Code_Big_Little_Endian32(pFSFB_SFM->connection[j].fsfb_chn[i].CON_1[k]);
					pFSFB_SFM->connection[j].fsfb_chn[i].CON_2[k] = GM_RSSP1_Code_Big_Little_Endian32(pFSFB_SFM->connection[j].fsfb_chn[i].CON_2[k]);
					pFSFB_SFM->connection[j].fsfb_chn[i].CON_3[k] = GM_RSSP1_Code_Big_Little_Endian32(pFSFB_SFM->connection[j].fsfb_chn[i].CON_3[k]);					
				}
				for (k=0U; k<pFSFB_SFM->connection[j].torlerate_cycle_cfg; ++k)
				{
					pFSFB_SFM->connection[j].fsfb_chn[i].CON_4[k] = GM_RSSP1_Code_Big_Little_Endian32(pFSFB_SFM->connection[j].fsfb_chn[i].CON_4[k]);					
				}
			}
		}
	}
	return GM_RSSP1_TRUE;
}

#endif

#ifdef GM_RSSP1_SYSCKW_FROM_USER
void GM_RSSP1_APP_Interface_Get_SysCKW(GM_RSSP1_UINT32 sysckw_1, GM_RSSP1_UINT32 sysckw_2)
{
	App_SYSCKW[0] = sysckw_1;
	App_SYSCKW[1] = sysckw_2;
}
#endif

/*added by huang 20151219*/
void GM_RSSP1_APP_Interface_VSN_Init(void)
{
    GM_RSSP1_VSN_Init();
}

void GM_RSSP1_APP_Interface_VSN_Update(void)
{
    GM_RSSP1_VSN_Update();
}
#endif

GM_RSSP1_BOOL GM_RSSP1_APP_Set_Active_WithOnline(GM_RSSP1_UINT16* OnlineArray)/*CR:GM00004827 Set active index*/
{
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;

	if (NULL != OnlineArray)
	{
		bRt = GM_RSSP1_Active_AddByArry(OnlineArray);
	}

	return bRt;
}

void GM_RSSP1_Set_LogLevel(GM_RSSP1_UINT16 lev)
{
	g_P1_LogLevel =  lev;
}

void GM_RSSP1_APP_Interface_RxPrc(void)
{
	GM_RSSP1_SeqDele_Record_Structure* pRec = NULL;
	
#ifdef GM_RSSP1_SAVING_MODE
	GM_RSSP1_SFM_Update_Time();
	pRec = GM_RSSP1_SeqDeleteRecord_Get();/**当前周期记录的状态变化的链接**/
	if (NULL == pRec)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_APP_Interface_RxPrc_Point_Match_Error, 0, 0, 0, 0, 0, 0);
	}
	pRec->RecordNum = 0U;
#endif

#ifdef CFM_Part
	GM_RSSP1_CFM_Interface_Proc_Recv();
#endif

#ifdef SFM_Part
	GM_RSSP1_SFM_Interface_Proc_Recv();
#endif
}

void GM_RSSP1_APP_Interface_TxPrc(void)
{
#ifdef SFM_Part
	GM_RSSP1_SFM_Interface_Proc_Send();
#endif

#ifdef CFM_Part
	GM_RSSP1_CFM_Interface_Proc_Send();
#endif
}