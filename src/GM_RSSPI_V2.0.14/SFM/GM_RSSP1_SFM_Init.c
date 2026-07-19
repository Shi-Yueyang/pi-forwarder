
#include "GM_RSSP1_SFM_Init.h"

#ifdef SFM_Part
extern RSSP1_MQ_Inter_struct g_SFM_MQ;

#ifndef CFM_Part
extern RSSP1_MQ_Inter_struct g_CFM_MQ;
#endif

extern GM_RSSP1_UINT32 g_Cfg_IsFixNode;
extern GM_RSSP1_UINT16 g_Max_ConnctNum;
#ifdef FILE_SYSTEM
GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Cfg_SFM(GM_RSSP1_INT8 *pIniFileContent , GM_RSSP1_comm_cfg_struct *pCfg)
{
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_object_struct *pSFM = NULL;

	pSFM = GM_RSSP1_Get_SFM_Object();

	if ((NULL == pSFM) || (NULL == pIniFileContent) || (NULL == pCfg))
	{
		return bRt;
	}

	bRt = GM_RSSP1_Init_Comm_Global_SFM(pCfg, pIniFileContent);

	/* 顺序不可变动。初始化各连接时需要从SFM cfg中获取本地连接通道共享的一些配置 */
	if (GM_RSSP1_TRUE == bRt)
	{
		/*初始化在线链表*/
		GM_RSSP1_InitSeqArray(OnLine_Object, g_Max_ConnctNum);

		for (l_index=0U; l_index<pCfg->SFM_cfg.connection_nums; ++l_index)
		{
			bRt = GM_RSSP1_Init_Comm_Connection_SFM(l_index, pCfg, pIniFileContent); /*单个节点load+init*/
			if (GM_RSSP1_TRUE == bRt)
			{
				bRt = GM_RSSP1_SFM_Init_Lib(pCfg, pSFM);
			}

			if (GM_RSSP1_FALSE == bRt)
			{
				break;
			}
		}
	}

	return bRt;
}

GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Global_SFM(GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_INT8 *pIniFileContent)
{
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	GM_RSSP1_INT32 valueInt = -1;
	GM_RSSP1_INT8 valueString[256] = {0};
	GM_RSSP1_UINT32 mem_size = 0U;

	if ((NULL == pCfg) || (NULL == pIniFileContent))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Point_Error, (int)pCfg, (int)pIniFileContent, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	/*added by huang 20151201*/
	rt = GM_RSSP1_Ini_File_Read_Int("RSSP1_GLOBAL", "main_cycle", (int*)&valueInt, pIniFileContent);
	if (GM_RSSP1_FALSE == rt)
	{
		return GM_RSSP1_FALSE;
	}
	else
	{
		pCfg->SFM_cfg.lobalInfoStruct.main_cycle = (GM_RSSP1_UINT32)valueInt;
	}
	/*the end 20151201*/		

	if (g_Max_ConnctNum == 0U)
	{
		rt = GM_RSSP1_Ini_File_Read_Int("RSSP1_GLOBAL", "Max_ConnectNum", (int*)&valueInt, pIniFileContent);

		if (GM_RSSP1_TRUE == rt)
		{
			if ((valueInt < 0)||(valueInt > GM_RSSP1_MAX_ONLINE_CONNECTION_NUM))
			{
				GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Max_Num_Error, rt, (int)valueInt, (int)GM_RSSP1_MAX_ONLINE_CONNECTION_NUM, 0, 0, 0);
				return GM_RSSP1_FALSE;
			}
			else
			{
				g_Max_ConnctNum = (GM_RSSP1_UINT16)valueInt;
			}
		}
		else
		{
			g_Max_ConnctNum = GM_RSSP1_MAX_ONLINE_CONNECTION_NUM;
		}
	}

	rt = GM_RSSP1_Ini_File_Read_Int("RSSP1_GLOBAL", "connection_num", (int*)&valueInt, pIniFileContent);

	if ((GM_RSSP1_FALSE == rt) || (valueInt < 0) || (valueInt > (GM_RSSP1_INT32)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Connec_Num_Error, rt, (int)valueInt, (int)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pCfg->SFM_cfg.connection_nums = (GM_RSSP1_UINT16)valueInt;
	}

	rt = GM_RSSP1_Ini_File_Read_String("RSSP1_GLOBAL", "source_addr", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		pCfg->SFM_cfg.lobalInfoStruct.source_addr = 0U;
		/*
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Src_Error, rt, (int)valueInt, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
		*/
	}
	else
	{
		pCfg->SFM_cfg.lobalInfoStruct.source_addr = (GM_RSSP1_UINT16)(strtoul(valueString , NULL , 16));
	}

	rt = GM_RSSP1_Ini_File_Read_String("RSSP1_GLOBAL", "local_sid_A",  (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		pCfg->SFM_cfg.lobalInfoStruct.local_sid[0] = 0U;
		/*
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_Sid_Error, rt, 1, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
		*/
	}
	else
	{
		pCfg->SFM_cfg.lobalInfoStruct.local_sid[0] = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}

	rt = GM_RSSP1_Ini_File_Read_String("RSSP1_GLOBAL", "local_sinit_A", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		pCfg->SFM_cfg.lobalInfoStruct.local_sinit[0] = 0U;
		/*
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_Sinit_Error, rt, 1, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
		*/
	}
	else
	{
		pCfg->SFM_cfg.lobalInfoStruct.local_sinit[0] = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}

	rt = GM_RSSP1_Ini_File_Read_String("RSSP1_GLOBAL", "local_dataVer_A", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		pCfg->SFM_cfg.lobalInfoStruct.local_dataVer[0] = 0U;
		/*
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_DataVer_Error, rt, 1, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
		*/
	}
	else
	{
		pCfg->SFM_cfg.lobalInfoStruct.local_dataVer[0] = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}

	rt = GM_RSSP1_Ini_File_Read_String("RSSP1_GLOBAL", "local_sys_chk_A", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_SysChk_Error, rt, 1, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
        pCfg->SFM_cfg.lobalInfoStruct.local_sys_chk[0] = (~((GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16))));
	}

	rt = GM_RSSP1_Ini_File_Read_String("RSSP1_GLOBAL", "local_sid_B", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		pCfg->SFM_cfg.lobalInfoStruct.local_sid[1] = 0U;
		/*
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_Sid_Error, rt, 2, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;*/
	}
	else
	{
		pCfg->SFM_cfg.lobalInfoStruct.local_sid[1] = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}

	rt = GM_RSSP1_Ini_File_Read_String("RSSP1_GLOBAL", "local_sinit_B", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		pCfg->SFM_cfg.lobalInfoStruct.local_sinit[1] = 0U;
		/*
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_Sinit_Error, rt, 2, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
		*/
	}
	else
	{
		pCfg->SFM_cfg.lobalInfoStruct.local_sinit[1] = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}

	rt = GM_RSSP1_Ini_File_Read_String("RSSP1_GLOBAL", "local_dataVer_B", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		pCfg->SFM_cfg.lobalInfoStruct.local_dataVer[1] = 0U;
		/*
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_DataVer_Error, rt, 2, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
		*/
	}
	else
	{
		pCfg->SFM_cfg.lobalInfoStruct.local_dataVer[1] = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}

	rt = GM_RSSP1_Ini_File_Read_String("RSSP1_GLOBAL", "local_sys_chk_B", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_SysChk_Error, rt, 2, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
        pCfg->SFM_cfg.lobalInfoStruct.local_sys_chk[1] = (~((GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16))));
	}

	rt = GM_RSSP1_Ini_File_Read_Int("RSSP1_GLOBAL", "SFM_U2L_Q_Size_per_connection", (int*)&valueInt, pIniFileContent);

	if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_U2L_Q_Size_Error, rt, valueInt, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pCfg->SFM_cfg.U2L_pri_Q_size = (GM_RSSP1_UINT32)valueInt;
	}

	rt = GM_RSSP1_Ini_File_Read_Int("RSSP1_GLOBAL", "SFM_L2U_Q_Size_per_connection", (int*)&valueInt, pIniFileContent);

	if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_L2U_Q_Size_Error, rt, valueInt, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pCfg->SFM_cfg.L2U_pri_Q_size = (GM_RSSP1_UINT32)valueInt;
	}

	#ifndef CFM_Part
	rt = GM_RSSP1_Ini_File_Read_Int("RSSP1_GLOBAL", "CFM_U2L_Q_Size_per_connection", (int*)&valueInt, pIniFileContent);

	if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_CFM_U2L_Q_Size_Error, rt, (int)valueInt, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pCfg->SFM_cfg.U2L_pri_Q_size_CFM = (GM_RSSP1_UINT32)valueInt;
	}

	rt = GM_RSSP1_Ini_File_Read_Int("RSSP1_GLOBAL", "CFM_L2U_Q_Size_per_connection", (int*)&valueInt, pIniFileContent);

	if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_CFM_L2U_Q_Size_Error, rt, (int)valueInt, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pCfg->SFM_cfg.L2U_pri_Q_size_CFM = (GM_RSSP1_UINT32)valueInt;
	}
	#endif

	return GM_RSSP1_TRUE;
}

GM_RSSP1_BOOL GM_RSSP1_Init_ConUnderPack( GM_RSSP1_SFM_connection_cfg_struct* pSFM_conn_cfg, GM_RSSP1_INT8* section_key, GM_RSSP1_INT8 * pIniFileContent) 
{
	GM_RSSP1_INT32 valueInt = 0;
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;

	if (NULL != pSFM_conn_cfg)/*CR 5885*/
	{
		if (GM_RSSP1_TRUE == pSFM_conn_cfg->b_enable_FSFB_on_chn)
		{
			/* init UDP chn apply FSFB ID */
			rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "Chn_apply_FSFB_ID", (int*)&valueInt, pIniFileContent);

			if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
			{
				GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_FSFB_ID_Error, rt, (int)valueInt, 0, 0, 0, 0);
				return GM_RSSP1_FALSE;
			}
			else
			{
				pSFM_conn_cfg->Chn_apply_FSFB_ID = (GM_RSSP1_UINT32)valueInt;
			}

			/* init msg queue in the SFM connection */
			rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "con_L2U_Q_size", (int*)&valueInt, pIniFileContent);

			if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
			{
				GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_L2U_Q_Size_Error, rt, (int)valueInt, 0, 0, 0, 0);
				return GM_RSSP1_FALSE;
			}
			else
			{
				pSFM_conn_cfg->L2U_dat_Q_size = (GM_RSSP1_UINT32)valueInt;
				pSFM_conn_cfg->L2U_dat_Q_offset = (GM_RSSP1_Pointer)GM_RSSP1_malloc((size_t)valueInt*sizeof(GM_RSSP1_SFM_L2U_pri_struct));  /* CR: 11036019*/

				if (0U == pSFM_conn_cfg->L2U_dat_Q_offset)
				{
					GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_Malloc_Error, 0, 0, 0, 0, 0, 0);
					return GM_RSSP1_FALSE;
				}
				GM_RSSP1_memset((void*)pSFM_conn_cfg->L2U_dat_Q_offset, 0U, ((size_t)valueInt*sizeof(GM_RSSP1_SFM_L2U_pri_struct)));
			}
		}
		else
		{
			rt = GM_RSSP1_TRUE;
		}
	}

	return rt;
}

GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Connection_SFM(GM_RSSP1_UINT16 conn_index,GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_INT8 *pIniFileContent)
{
	GM_RSSP1_INT8 valueString[256] = {0};
	GM_RSSP1_INT8 section_key[256] = {0};   /* 连接通道键名 */
	GM_RSSP1_INT8 sub_section_key[256] = {0};   /* UDP通道键名 */
	GM_RSSP1_BOOL valueBool = GM_RSSP1_FALSE;
	GM_RSSP1_INT32 valueInt = -1;
	GM_RSSP1_INT32 key_len = 0;
	GM_RSSP1_INT32 sub_key_len = 0;
	GM_RSSP1_INT32 chn_index = 0;
	GM_RSSP1_INT32 fsfb_chn_index = 0;
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_connection_cfg_struct *pSFM_conn_cfg = NULL;

	if ((NULL == pCfg) || (NULL == pIniFileContent))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_Point_Error, (int)pCfg, (int)pIniFileContent, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	/* 各个连接通道配置 */
	key_len = (GM_RSSP1_INT32)(sizeof("CON_") - 1U);

	GM_RSSP1_memcpy((void*)section_key, "CON_", ((size_t)key_len + (size_t)1));


	pSFM_conn_cfg = &(pCfg->SFM_cfg.connection_cfg);
	GM_RSSP1_memset(pSFM_conn_cfg, 0U, sizeof(GM_RSSP1_SFM_connection_cfg_struct));

	GM_RSSP1_memset((void*)(section_key + key_len), 0U, (size_t)(sizeof(section_key) - key_len));
	sprintf(section_key + key_len, "%d", conn_index);

	pSFM_conn_cfg->main_cycle = pCfg->SFM_cfg.lobalInfoStruct.main_cycle;

	pSFM_conn_cfg->source_addr = pCfg->SFM_cfg.lobalInfoStruct.source_addr;
	for (fsfb_chn_index = 0 ; fsfb_chn_index < (GM_RSSP1_INT32)GM_RSSP1_CHECK_CHN_NUM ; ++fsfb_chn_index)
	{
		pSFM_conn_cfg->fsfb_chn_cfg[fsfb_chn_index].local_sid       = pCfg->SFM_cfg.lobalInfoStruct.local_sid[fsfb_chn_index];
		pSFM_conn_cfg->fsfb_chn_cfg[fsfb_chn_index].local_sinit     = pCfg->SFM_cfg.lobalInfoStruct.local_sinit[fsfb_chn_index];
		pSFM_conn_cfg->fsfb_chn_cfg[fsfb_chn_index].local_dataVer   = pCfg->SFM_cfg.lobalInfoStruct.local_dataVer[fsfb_chn_index];
		pSFM_conn_cfg->fsfb_chn_cfg[fsfb_chn_index].local_sys_chk   = pCfg->SFM_cfg.lobalInfoStruct.local_sys_chk[fsfb_chn_index];
	}

	/*added by huang 20151201*/
	rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "FSFB_comm_cycle", (int*)&valueInt, pIniFileContent);
	if (GM_RSSP1_FALSE == rt)
	{
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->fsfb_comm_cycle = (GM_RSSP1_UINT32)valueInt;
	}
	/*added by huang 20151201*/

	/*CR5649：jcf 获取异步节点的周期数，若用户配置则替换为节点本地周期，未配置则忽略*/
	rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "local_node_cycle", (int*)&valueInt, pIniFileContent);
	if (GM_RSSP1_TRUE == rt)
	{
		pSFM_conn_cfg->main_cycle = (GM_RSSP1_UINT32)valueInt;	/*若当前节点有异步周期，则将节点主周期替换为异步周期*/
	}
	else
	{
		GM_RSSP1_Log_Msg(4,"Warning:[Con_%d] User not config local node cycle\n", (int)conn_index, 0, 0, 0, 0, 0);
	}

	/*新增源节点不同地址*/
	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "source_addr", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_TRUE == rt)
	{
		pSFM_conn_cfg->source_addr = (GM_RSSP1_UINT16)(strtoul(valueString , NULL , 16));
	}
	else if (0U == pSFM_conn_cfg->source_addr)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Src_Error, rt, (int)valueInt, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		;/*do nothing*/
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "local_sid_A",  (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_TRUE == rt)
	{
		pSFM_conn_cfg->fsfb_chn_cfg[0].local_sid = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}
	else if (0U == pCfg->SFM_cfg.lobalInfoStruct.local_sid[0])
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_Sid_Error, rt, 1, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		;/*do nothing*/
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "local_sinit_A", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_TRUE == rt)
	{
		pSFM_conn_cfg->fsfb_chn_cfg[0].local_sinit = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}
	else if (0U == pCfg->SFM_cfg.lobalInfoStruct.local_sinit[0])
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_Sinit_Error, rt, 1, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		;/*do nothing*/
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "local_dataVer_A", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_TRUE == rt)
	{
		pSFM_conn_cfg->fsfb_chn_cfg[0].local_dataVer = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}
	else if (0U == pCfg->SFM_cfg.lobalInfoStruct.local_dataVer[0])
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_DataVer_Error, rt, 1, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		;/*do nothing*/
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "local_sys_chk_A", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_TRUE == rt)
	{
		pSFM_conn_cfg->fsfb_chn_cfg[0].local_sys_chk = (~((GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16))));
	}
	else if (0U == pCfg->SFM_cfg.lobalInfoStruct.local_sys_chk[0])
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_SysChk_Error, rt, 1, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		;/*do nothing*/
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "local_sid_B", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_TRUE == rt)
	{
		pSFM_conn_cfg->fsfb_chn_cfg[1].local_sid = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}
	else if (0U == pCfg->SFM_cfg.lobalInfoStruct.local_sid[1])
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_Sid_Error, rt, 2, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		;/*do nothing*/
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "local_sinit_B", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_TRUE == rt)
	{
		pSFM_conn_cfg->fsfb_chn_cfg[1].local_sinit = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}
	else if (0U == pCfg->SFM_cfg.lobalInfoStruct.local_sinit[1])
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_Sinit_Error, rt, 2, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		;/*do nothing*/
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "local_dataVer_B", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_TRUE == rt)
	{
		pSFM_conn_cfg->fsfb_chn_cfg[1].local_dataVer = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}
	else if (0U == pCfg->SFM_cfg.lobalInfoStruct.local_dataVer[1])
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_DataVer_Error, rt, 2, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		;/*do nothing*/
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "local_sys_chk_B", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_TRUE == rt)
	{
		pSFM_conn_cfg->fsfb_chn_cfg[1].local_sys_chk = (~((GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16))));
	}
	else if (0U == pCfg->SFM_cfg.lobalInfoStruct.local_sys_chk[1])
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Loc_SysChk_Error, rt, 2, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		;/*do nothing*/
	}
	/****/

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "dest_addr", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_Des_Error, (int)conn_index, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->dest_addr = (GM_RSSP1_UINT16)(strtoul(valueString , NULL , 16));
	}

	/* init FSFB param */
	rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "deltaTime", (int*)&valueInt, pIniFileContent);

	if ((GM_RSSP1_FALSE == rt) || (valueInt < 0) || (valueInt > (GM_RSSP1_INT32)GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_Delta_Error, (int)conn_index, rt, (int)valueInt, GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->deltaTime = (GM_RSSP1_UINT16)valueInt;
	}

	/*GM00003077:添加对主要配置文件参数的防护*/
	rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "DelayTime", (int*)&valueInt, pIniFileContent);

	if ((GM_RSSP1_FALSE == rt) || (valueInt < 0) || (valueInt > (GM_RSSP1_INT32)GM_RSSP1_MAX_DELAYTIME ))	/*jcf：添加防护*/
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_Loss_Error, (int)conn_index, rt, (int)valueInt, (int)GM_RSSP1_MAX_DELAYTIME, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->DelayTime = (GM_RSSP1_UINT16)valueInt;
	}

	rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "lifeTime", (int*)&valueInt, pIniFileContent);

	if ((GM_RSSP1_FALSE == rt) || (valueInt < 0) || (valueInt > (GM_RSSP1_INT32)GM_RSSP1_MAX_LIFETIME ))	/*jcf：添加防护*/
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_LifeTime_Error, (int)conn_index, rt, (int)valueInt, (int)GM_RSSP1_MAX_LIFETIME, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->lifeTime = (GM_RSSP1_UINT16)valueInt;
	}

	rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "torlerate_cycle", (int*)&valueInt, pIniFileContent);

	if ((GM_RSSP1_FALSE == rt) || (valueInt < 0) || (valueInt > (GM_RSSP1_INT32)GM_RSSP1_MAX_TOLERATE_CYCLE))	/*jcf：添加防护*/
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_TorCyc_Error, (int)conn_index, rt, (int)valueInt, (int)GM_RSSP1_MAX_TOLERATE_CYCLE, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->torlerate_cycle_cfg = (GM_RSSP1_UINT16)valueInt;
	}

	rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "num_data_ver", (int*)&valueInt, pIniFileContent);

	if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_DataVer_Error, (int)conn_index, rt, (int)valueInt, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->num_data_ver = (GM_RSSP1_UINT8)valueInt;
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "remote_sid_A", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_Rem_Sid_Error, (int)conn_index, 1, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->fsfb_chn_cfg[0U].remote_sid = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "remote_sinit_A", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_Rem_Sinit_Error, (int)conn_index, 1, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->fsfb_chn_cfg[0U].remote_sinit = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "remote_dataVer_A", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_Rem_DateVer_Error, (int)conn_index, 1, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->fsfb_chn_cfg[0U].remote_dataVer = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "remote_sid_B", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_Rem_Sid_Error, (int)conn_index, 2, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->fsfb_chn_cfg[1U].remote_sid = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "remote_sinit_B", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_Rem_Sinit_Error, (int)conn_index, 2, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->fsfb_chn_cfg[1U].remote_sinit = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "remote_dataVer_B", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_Rem_DateVer_Error, (int)conn_index, 2, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->fsfb_chn_cfg[1U].remote_dataVer = (GM_RSSP1_UINT32)(strtoul(valueString , NULL , 16));
	}

	/*jcf:添加对远端设备AB系标识的配置*/
	rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "remote_dev_is_A", (int*)&valueInt, pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Msg(2,"Warning:[Con_%d] User not config remote device tobe A or B\n", (int)conn_index, 0, 0, 0, 0, 0);
		pSFM_conn_cfg->remote_dev_AB = (GM_RSSP1_UINT16)GM_RSSP1_Remote_Dev_Unknow;		/*若用户未配置，不返回，默认为未知态*/
	}
	else
	{
		if(GM_RSSP1_TRUE == (GM_RSSP1_BOOL)valueInt)
		{
			pSFM_conn_cfg->remote_dev_AB = (GM_RSSP1_UINT16)GM_RSSP1_Remote_Dev_A;		
		}else
		{
			pSFM_conn_cfg->remote_dev_AB = (GM_RSSP1_UINT16)GM_RSSP1_Remote_Dev_B;	
		}
	}

	/* 对等设备各个UDP通道均使用独立的FSFB。此时UDP通道数必须为1 */
	rt = GM_RSSP1_Ini_File_Read_BOOL((const char *)section_key , "enable_UDP_chn_FSFB" , &valueBool, pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_EnableUDP_Error, (int)conn_index, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pSFM_conn_cfg->b_enable_FSFB_on_chn = valueBool;
	}


	/*对设备节点设置特殊标志位，如果新版本*/
	rt = GM_RSSP1_Ini_File_Read_BOOL((const char *)section_key , "Is_Fix_Node" , &valueBool, pIniFileContent);

	if (GM_RSSP1_TRUE == rt)
	{
		pSFM_conn_cfg->IsFixNode = (GM_RSSP1_UINT32)valueBool;
		if(pSFM_conn_cfg->IsFixNode == 1U)
		{
			g_Cfg_IsFixNode = g_Cfg_IsFixNode + 1;
		}


		if(g_Cfg_IsFixNode > g_Max_ConnctNum)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_IsFixNum_Error, rt, (int)valueInt, (int)g_Max_ConnctNum, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
	}
	else
	{
		;/*非安全数据，且考虑配置兼容性不做报警处理*/
	}

	rt = GM_RSSP1_Init_ConUnderPack(pSFM_conn_cfg, section_key, pIniFileContent);

	return rt;
}

GM_RSSP1_BOOL GM_RSSP1_SFM_Init(GM_RSSP1_INT8 *file)
{
	GM_RSSP1_comm_cfg_struct fsfb_cfg = {0U};
	GM_RSSP1_SFM_object_struct *pSFM = NULL;
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	static GM_RSSP1_BOOL is_boot = GM_RSSP1_FALSE;

	pSFM = GM_RSSP1_Get_SFM_Object();

	if ((NULL != file) || (NULL != pSFM))
	{
		if (is_boot == GM_RSSP1_FALSE)
		{
			rt = GM_RSSP1_Hash_SFM_Init();
			if (GM_RSSP1_FALSE == rt)
			{
				return rt;
			}
			GM_RSSP1_memset((void*)pSFM, 0U, sizeof(GM_RSSP1_SFM_object_struct));
			pSFM->lib_state = GM_RSSP1_Lib_State_Unknown;
			pSFM->Is_torlerate_process = GM_RSSP1_FALSE;
		}

		GM_RSSP1_memset((void*)&fsfb_cfg, 0U, sizeof(GM_RSSP1_comm_cfg_struct));
		rt = GM_RSSP1_Init_Comm_Cfg_SFM(file, &fsfb_cfg); /* CR:GM00007614*/
		if (GM_RSSP1_TRUE == rt)
		{
			if (is_boot == GM_RSSP1_FALSE)
			{
				rt = GM_RSSP1_SFM_MQ_Init(&fsfb_cfg , pSFM); /* CR:GM00007614*/
				if (GM_RSSP1_TRUE == rt)
				{
					is_boot = GM_RSSP1_TRUE;
					rt = GM_RSSP1_TRUE;
				}
			}
			else
			{
				rt = GM_RSSP1_TRUE;
			}
		}
	}

	return rt;
}
#endif

GM_RSSP1_BOOL GM_RSSP1_SFM_MQ_Init(GM_RSSP1_comm_cfg_struct *pFSFB_comm_cfg, GM_RSSP1_SFM_object_struct *pSFM)
{
	GM_RSSP1_UINT32 len = 0U;

	if ((NULL == pFSFB_comm_cfg) || (NULL == pSFM))
	{
		return GM_RSSP1_FALSE;
	}
#ifndef GM_RSSP1_SAVING_MODE
	len = (GM_RSSP1_UINT32)(pFSFB_comm_cfg->SFM_cfg.U2L_pri_Q_size * sizeof(GM_RSSP1_SFM_U2L_pri_struct));
	pSFM->U2L_pri_Q_offset = (GM_RSSP1_Pointer)GM_RSSP1_malloc((size_t)len);  /* CR: 11036019*/
	
	if (0U == pSFM->U2L_pri_Q_offset)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_MQ_Init_U2L_Memory_Fail, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	GM_RSSP1_memset((void*)pSFM->U2L_pri_Q_offset, 0U, (size_t)len);

	if (GM_RSSP1_FALSE == FSFB_Msg_Queue_Init(&(g_SFM_MQ.SND_MQ), pFSFB_comm_cfg->SFM_cfg.U2L_pri_Q_size , pSFM->U2L_pri_Q_offset , FSFB_Q_TYPE_SFM_U2L_PRI))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_MQ_Init_U2L_MQ_Fail, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
#endif
	len = (GM_RSSP1_UINT32)(pFSFB_comm_cfg->SFM_cfg.L2U_pri_Q_size * sizeof(GM_RSSP1_SFM_L2U_pri_struct));
	pSFM->L2U_pri_Q_offset  = (GM_RSSP1_Pointer)GM_RSSP1_malloc((size_t)len);  /* CR: 11036019*/

	if (0U == pSFM->L2U_pri_Q_offset)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_MQ_Init_L2U_Memory_Fail, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	GM_RSSP1_memset((void*)pSFM->L2U_pri_Q_offset, 0U, (size_t)len);

	if (GM_RSSP1_FALSE == FSFB_Msg_Queue_Init(&(g_SFM_MQ.RCV_MQ), pFSFB_comm_cfg->SFM_cfg.L2U_pri_Q_size , pSFM->L2U_pri_Q_offset , FSFB_Q_TYPE_SFM_L2U_PRI))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_MQ_Init_L2U_MQ_Fail, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
#ifndef CFM_Part
	len = (GM_RSSP1_UINT32)(pFSFB_comm_cfg->SFM_cfg.U2L_pri_Q_size_CFM * sizeof(GM_RSSP1_CFM_U2L_pri_struct));
	pSFM->U2L_pri_Q_offset_CFM = (GM_RSSP1_Pointer)GM_RSSP1_malloc((size_t)len);  /* CR: 11036019*/

	if (NULL == pSFM->U2L_pri_Q_offset_CFM)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_MQ_Init_CFM_U2L_Memory_Fail, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	GM_RSSP1_memset((void*)pSFM->U2L_pri_Q_offset_CFM, 0U, (size_t)len);

	if (GM_RSSP1_FALSE == FSFB_Msg_Queue_Init(&(g_CFM_MQ.SND_MQ), pFSFB_comm_cfg->SFM_cfg.U2L_pri_Q_size_CFM, pSFM->U2L_pri_Q_offset_CFM , FSFB_Q_TYPE_CFM_U2L_PRI))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_MQ_Init_CFM_U2L_MQ_Fail, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	len = (GM_RSSP1_UINT32)(pFSFB_comm_cfg->SFM_cfg.L2U_pri_Q_size_CFM * sizeof(GM_RSSP1_CFM_L2U_pri_struct));
	pSFM->L2U_pri_Q_offset_CFM = (GM_RSSP1_Pointer)GM_RSSP1_malloc((size_t)len);  /* CR: 11036019*/
	if (NULL == pSFM->L2U_pri_Q_offset_CFM)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_MQ_Init_CFM_L2U_Memory_Fail, 2, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	GM_RSSP1_memset((void*)pSFM->L2U_pri_Q_offset_CFM, 0U, (size_t)len);

	if (GM_RSSP1_FALSE == FSFB_Msg_Queue_Init(&(g_CFM_MQ.RCV_MQ), pFSFB_comm_cfg->SFM_cfg.L2U_pri_Q_size_CFM, pSFM->L2U_pri_Q_offset_CFM, FSFB_Q_TYPE_CFM_L2U_PRI))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_MQ_Init_CFM_L2U_MQ_Fail, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
#endif
	pSFM->lib_state = GM_RSSP1_Lib_State_Operational;

	return GM_RSSP1_TRUE;
}


GM_RSSP1_BOOL GM_RSSP1_SFM_Init_Lib(GM_RSSP1_comm_cfg_struct *pFSFB_comm_cfg, GM_RSSP1_SFM_object_struct *pSFM)
{
	GM_RSSP1_UINT16 conIndex = 0U;
	GM_RSSP1_UINT16 conn_num = 0U;
	GM_RSSP1_UINT16 Temp16U = 0U;
	GM_RSSP1_SFM_connection_struct *pSFM_conn = NULL;
	GM_RSSP1_SFM_connection_cfg_struct *pConn_cfg = NULL;
	GM_RSSP1_INT32 chn_index = 0;
	GM_RSSP1_INT32 rt = GM_RSSP1_ERROR;
	GM_RSSP1_BOOL ret = GM_RSSP1_FALSE;

	if ((NULL == pFSFB_comm_cfg) || (NULL == pSFM))
	{
		return GM_RSSP1_FALSE;
	}

	conn_num = pSFM->connection_nums + 1U;
	if (conn_num>GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
	{
		return GM_RSSP1_FALSE;
	}

	conIndex = GM_RSSP1_InitSeqInsert(OffLine_Object);
	pSFM_conn = &(pSFM->connection[conIndex]);
	pConn_cfg = &(pFSFB_comm_cfg->SFM_cfg.connection_cfg);

	pSFM_conn->index                = conIndex;
	pSFM_conn->state                = GM_RSSP1_Layer_State_Unknow;
	pSFM_conn->source_addr          = pConn_cfg->source_addr;
	pSFM_conn->dest_addr            = pConn_cfg->dest_addr;
	pSFM_conn->SaCEPID.source_addr  = pConn_cfg->source_addr;
	pSFM_conn->SaCEPID.dest_addr    = pConn_cfg->dest_addr;
	pSFM_conn->deltaTime            = pConn_cfg->deltaTime;
	pSFM_conn->lifeTime             = pConn_cfg->lifeTime;
	pSFM_conn->DelayTime            = pConn_cfg->DelayTime;
	pSFM_conn->IsFixed				= pConn_cfg->IsFixNode;

	if(pSFM_conn->deltaTime > GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE)
	{
		return GM_RSSP1_FALSE;
	}

	if(pSFM_conn->lifeTime > GM_RSSP1_MAX_TOLERATE_CYCLE)
	{
		return GM_RSSP1_FALSE;
	}

	pSFM_conn->num_data_ver         = pConn_cfg->num_data_ver;
	pSFM_conn->remote_dev_AB		=  (GM_RSSP1_Remote_Dev_AB_enum)pConn_cfg->remote_dev_AB;	/*jcf:要么为0x80或0x81，要么为未知态0xff，其余值均认为错误*/

	for (chn_index = 0 ; chn_index < (GM_RSSP1_INT32)GM_RSSP1_CHECK_CHN_NUM ; ++chn_index)
	{
		pSFM_conn->fsfb_chn[chn_index].chn_cfg  = pConn_cfg->fsfb_chn_cfg[chn_index];
	}
	if (GM_RSSP1_FALSE == GM_RSSP1_SFM_Ini_FSFB_Offline_Const(pSFM_conn, pSFM))
	{
		return GM_RSSP1_FALSE;
	}
	pSFM_conn->b_RSD_Already_Sent = GM_RSSP1_FALSE;
	pSFM_conn->b_enable_FSFB_on_chn = pConn_cfg->b_enable_FSFB_on_chn;
	if (GM_RSSP1_TRUE == pSFM_conn->b_enable_FSFB_on_chn)
	{
		pSFM_conn->Chn_apply_FSFB_ID = pConn_cfg->Chn_apply_FSFB_ID;
		if (GM_RSSP1_FALSE == FSFB_Msg_Queue_Init(&(pSFM_conn->L2U_dat_Q) , pConn_cfg->L2U_dat_Q_size , pConn_cfg->L2U_dat_Q_offset , FSFB_Q_TYPE_SFM_L2U_PRI))
		{
			return GM_RSSP1_FALSE;
		}
	}
	pSFM_conn->torlerate_cycle_cfg = pConn_cfg->torlerate_cycle_cfg;
	if(pSFM_conn->torlerate_cycle_cfg > GM_RSSP1_MAX_TOLERATE_CYCLE)
	{
		return GM_RSSP1_FALSE;
	}
	GM_RSSP1_memset((void*)&(pSFM_conn->last_valid_dat_pri), 0U, sizeof(GM_RSSP1_SFM_L2U_pri_struct));
	pSFM_conn->b_rcvd_new_dat_in_cycle = GM_RSSP1_FALSE;
	pSFM_conn->b_torlerant_dat_valid   = GM_RSSP1_FALSE;
	rt = GM_RSSP1_TIME_Init(
		&(pSFM_conn->time) ,
		g_Polyomia[0U].pLFSR_left_table,  /* SFM中使用的LFSR表相同，使用第0个通道的LFSR表即可 */
		g_Polyomia[1U].pLFSR_left_table,
		pSFM_conn->fsfb_chn[0U].chn_cfg.remote_sid,
		pSFM_conn->fsfb_chn[1U].chn_cfg.remote_sid
		);
	if (GM_RSSP1_ERROR == rt)
	{
		return GM_RSSP1_FALSE;
	}

	/*added by huang 20151201*/
	pSFM_conn->PreciTime = -1;
	pSFM_conn->SSE_SSR_Delay = -1;
	pSFM_conn->SINIT_TremCycle = 0U;
	pSFM_conn->RxSSRTrem = 0U;
	pSFM_conn->RxSSRTloc = 0U;
	pSFM_conn->TcycleLoc = pConn_cfg->main_cycle;	/*jcf修改，节点异步周期覆盖*/
	pSFM_conn->TcycleRem = pConn_cfg->fsfb_comm_cycle;
	/*the end 20151201*/

	if ((pSFM_conn->DelayTime == 0U) && (pSFM_conn->TcycleLoc != 0U))
	{
		pSFM_conn->keepIdle = (GM_RSSP1_UINT16)(((pSFM_conn->deltaTime * pSFM_conn->TcycleRem)+pSFM_conn->TcycleLoc-1U)/pSFM_conn->TcycleLoc);
	}
	else
	{
		pSFM_conn->keepIdle = pSFM_conn->DelayTime;
	}

	pSFM_conn->state                = GM_RSSP1_Layer_State_Free;
	pSFM_conn->remote_dev_AS_state  = GM_RSSP1_Remote_DEV_Unknow;
	/*pSFM_conn->state_check_counter1  = 0;
	pSFM_conn->state_check_counter2  = (GM_RSSP1_INT8)pSFM_conn->keepIdle;*/

	for (chn_index = 0 ; chn_index < (GM_RSSP1_INT32)GM_RSSP1_CHECK_CHN_NUM ; ++chn_index)
	{
		pSFM_conn->env[chn_index].TC           = pSFM_conn->time.TC;
		pSFM_conn->env[chn_index].TS           = pSFM_conn->time.TS[chn_index];
		pSFM_conn->env[chn_index].local_TC     = 0U;
		pSFM_conn->env[chn_index].remote_TC    = 0U;
		pSFM_conn->env[chn_index].sseTS        = 0U;
		pSFM_conn->env[chn_index].sseTC        = 0U;
		pSFM_conn->env[chn_index].dynamicKey   = 0U;
		pSFM_conn->env[chn_index].bSendSSE     = 0U;
	}

	Temp16U = GM_RSSP1_Hash_Fill_SFM_Index((GM_RSSP1_UINT32)pSFM_conn->dest_addr, conIndex);/**将dest_add作为关键值索引,并将dest_add和静态数据索引插入Hash表内**/
	if (0xFFFFU == Temp16U)
	{
		return GM_RSSP1_FALSE;
	}
	else
	{
		if (GM_RSSP1_TRUE == (GM_RSSP1_BOOL)pSFM_conn->IsFixed)
		{
			ret = GM_RSSP1_Hash_Insert_Online(Temp16U, GM_RSSP1_TRUE);
		}
		else
		{
			ret = GM_RSSP1_TRUE;
		}
	}

	pSFM->connection_nums = conn_num;

	return ret;
}


GM_RSSP1_BOOL GM_RSSP1_SFM_Delete(GM_RSSP1_UINT16 con_index, GM_RSSP1_SFM_object_struct *pSFM)
{
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;

	if (NULL == pSFM)
	{
		return bRt;
	}

	if ((con_index < GM_RSSP1_MAX_ONLINE_CONNECTION_NUM) && (pSFM->connection_nums > 0U))
	{
		if (GM_RSSP1_TRUE == pSFM->connection[con_index].b_enable_FSFB_on_chn)
		{
			GM_RSSP1_free((void*)(pSFM->connection[con_index].L2U_dat_Q.Q_offset));
		}

		GM_RSSP1_memset((void*)(&pSFM->connection[con_index]), (GM_RSSP1_UINT8)0U, sizeof(GM_RSSP1_SFM_connection_struct));
		pSFM->connection_nums --;
		bRt = GM_RSSP1_TRUE;
	}

	return bRt;
}
#endif
