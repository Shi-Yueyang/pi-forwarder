#include "GM_RSSP1_SFM_Init.h"
#ifdef SFM_Part
extern RSSP1_MQ_Inter_struct g_SFM_MQ;

#ifndef CFM_Part
extern RSSP1_MQ_Inter_struct g_CFM_MQ;
#endif
extern GM_RSSP1_UINT16 g_Max_ConnctNum;
extern GM_RSSP1_UINT32 g_Cfg_IsFixNode;
GM_RSSP1_SFM_InitBinMode_enum g_InitMode = GM_RSSP1_SFM_Bin_Stand;
GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Cfg_SFM_Addr(GM_RSSP1_UINT8 *pFileContent , GM_RSSP1_comm_cfg_struct *pCfg, GM_RSSP1_BOOL isbinbig)
{
	GM_RSSP1_UINT8 **rp_file = &pFileContent;
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_INT32 file_len = -1;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_object_struct *pSFM = NULL;

	pSFM = GM_RSSP1_Get_SFM_Object();

	if ((NULL == pSFM) || (NULL == pFileContent) || (NULL == pCfg))
	{
		return bRt;
	}
	
	bRt = GM_RSSP1_Init_Comm_Global_SFM_Addr(pCfg, pFileContent, rp_file, isbinbig);
	/* 顺序不可变动。初始化各连接时需要从SFM cfg中获取本地连接通道共享的一些配置 */
	if (GM_RSSP1_TRUE == bRt)
	{
		GM_RSSP1_InitSeqArray(OnLine_Object, g_Max_ConnctNum);   /* CR: 11036019*/
		for (l_index=0U; l_index<pCfg->SFM_cfg.connection_nums; ++l_index)
		{
			bRt = GM_RSSP1_Init_Comm_Connection_SFM_Addr(pCfg, pFileContent, rp_file, isbinbig);
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

/* CR: 11036019*/
GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Global_SFM_Addr(GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_UINT8 *pFileContent, GM_RSSP1_UINT8 **r_ptr, GM_RSSP1_BOOL isbinbig)	/*CR:GM00003071添加识别bin文件大小端的参数*/
{
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	GM_RSSP1_UINT32 valueInt = 0U;	
	GM_RSSP1_BOOL isbig = GM_RSSP1_Code_Is_Big_Endian();
	GM_RSSP1_UINT8 i =0U;
	GM_RSSP1_UINT16 con_num = 0U;
	GM_RSSP1_INT8 conn_index = 0;
	GM_RSSP1_UINT8 refresh_mode = 0U;
	GM_RSSP1_UINT32 mem_size = 0U;

	if ((NULL == pCfg) || (NULL == pFileContent))
	{
		return GM_RSSP1_FALSE;
	}

	/* RSSP1_GLOBAL 全局配置 */
	valueInt = *(GM_RSSP1_UINT32*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT32);
	pCfg->SFM_cfg.lobalInfoStruct.main_cycle = (GM_RSSP1_UINT32)valueInt;

	valueInt = *(GM_RSSP1_UINT16*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT16);
	pCfg->SFM_cfg.connection_nums = (GM_RSSP1_UINT16)valueInt;
	if (isbig != isbinbig)
	{
		pCfg->SFM_cfg.connection_nums = GM_RSSP1_Code_Big_Little_Endian16(pCfg->SFM_cfg.connection_nums);
	}

	if (pCfg->SFM_cfg.connection_nums > (GM_RSSP1_INT32)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
	{
		return GM_RSSP1_FALSE;
	}

	valueInt = *(GM_RSSP1_UINT16*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT16);
	pCfg->SFM_cfg.lobalInfoStruct.source_addr = (GM_RSSP1_UINT16)valueInt;

	for (i=0U; i<GM_RSSP1_CHECK_CHN_NUM; ++i)
	{
		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
		pCfg->SFM_cfg.lobalInfoStruct.local_sid[i] = valueInt;

		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
		pCfg->SFM_cfg.lobalInfoStruct.local_sinit[i] = valueInt;

		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
		pCfg->SFM_cfg.lobalInfoStruct.local_dataVer[i] = valueInt;

		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
        pCfg->SFM_cfg.lobalInfoStruct.local_sys_chk[i] = ~valueInt;
	}

	valueInt = *(GM_RSSP1_UINT32*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT32);
	pCfg->SFM_cfg.U2L_pri_Q_size = valueInt;

	valueInt = *(GM_RSSP1_UINT32*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT32);
	pCfg->SFM_cfg.L2U_pri_Q_size = valueInt;


	/**> 增加对Max_ConnectNum可选字段的读入 @63343 */
	valueInt = *(GM_RSSP1_UINT32*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT32);
	
	if (isbig != isbinbig)
	{
		valueInt= GM_RSSP1_Code_Big_Little_Endian32(valueInt);
	}

	if (valueInt != 0U)
	{

		if (valueInt > GM_RSSP1_MAX_ONLINE_CONNECTION_NUM)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_SFM_Max_Num_Error, rt, (int)valueInt, (int)GM_RSSP1_MAX_ONLINE_CONNECTION_NUM, pCfg->SFM_cfg.connection_nums, 0, 0);
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



	/**> 增加对CFM_U2L_Q_Size_per_connection可选字段的读入 @63343 */
	valueInt = *(GM_RSSP1_UINT32*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT32);
#ifndef CFM_Part
	pCfg->SFM_cfg.U2L_pri_Q_size_CFM = valueInt;
#endif
	
	/**> 增加对L2U_pri_Q_size_CFM的读入 @63343 */
	valueInt = *(GM_RSSP1_UINT32*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT32);
#ifndef CFM_Part
	pCfg->SFM_cfg.L2U_pri_Q_size_CFM = valueInt;
#endif


	if (isbig != isbinbig)/*若系统大小端 与 bin文件大小端不一致*/
	{
		pCfg->SFM_cfg.lobalInfoStruct.main_cycle = GM_RSSP1_Code_Big_Little_Endian32((GM_RSSP1_UINT32)(pCfg->SFM_cfg.lobalInfoStruct.main_cycle));

		pCfg->SFM_cfg.lobalInfoStruct.source_addr = GM_RSSP1_Code_Big_Little_Endian16(pCfg->SFM_cfg.lobalInfoStruct.source_addr);

		for (i=0U;i<GM_RSSP1_CHECK_CHN_NUM;i++)
		{
			pCfg->SFM_cfg.lobalInfoStruct.local_sid[i] = GM_RSSP1_Code_Big_Little_Endian32(pCfg->SFM_cfg.lobalInfoStruct.local_sid[i]);
			pCfg->SFM_cfg.lobalInfoStruct.local_sinit[i] = GM_RSSP1_Code_Big_Little_Endian32(pCfg->SFM_cfg.lobalInfoStruct.local_sinit[i]);
			pCfg->SFM_cfg.lobalInfoStruct.local_dataVer[i] = GM_RSSP1_Code_Big_Little_Endian32(pCfg->SFM_cfg.lobalInfoStruct.local_dataVer[i]);
			pCfg->SFM_cfg.lobalInfoStruct.local_sys_chk[i] = GM_RSSP1_Code_Big_Little_Endian32(pCfg->SFM_cfg.lobalInfoStruct.local_sys_chk[i]);
		}

		pCfg->SFM_cfg.U2L_pri_Q_size = GM_RSSP1_Code_Big_Little_Endian32(pCfg->SFM_cfg.U2L_pri_Q_size);
		pCfg->SFM_cfg.L2U_pri_Q_size = GM_RSSP1_Code_Big_Little_Endian32(pCfg->SFM_cfg.L2U_pri_Q_size);
#ifndef CFM_Part
		pCfg->SFM_cfg.U2L_pri_Q_size_CFM =  GM_RSSP1_Code_Big_Little_Endian32(pCfg->SFM_cfg.U2L_pri_Q_size_CFM);
		pCfg->SFM_cfg.L2U_pri_Q_size_CFM =  GM_RSSP1_Code_Big_Little_Endian32(pCfg->SFM_cfg.L2U_pri_Q_size_CFM);
#endif
	}


	*r_ptr = pFileContent;
	return GM_RSSP1_TRUE;
}

/* CR: 11036019*/
GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Connection_SFM_Addr(GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_UINT8 *pFileContent, GM_RSSP1_UINT8 **r_ptr, GM_RSSP1_BOOL isbinbig)
{
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	GM_RSSP1_UINT32 valueInt = 0U;
	GM_RSSP1_INT32 chn_index = 0;
	GM_RSSP1_INT32 fsfb_chn_index = 0;
	GM_RSSP1_BOOL isbig = GM_RSSP1_Code_Is_Big_Endian();
	GM_RSSP1_UINT8 i =0U;
	GM_RSSP1_SFM_connection_cfg_struct *pSFM_conn_cfg = NULL;

	if ((NULL == pCfg) || (NULL == pFileContent))
	{
		return GM_RSSP1_FALSE;
	}

	pSFM_conn_cfg = &(pCfg->SFM_cfg.connection_cfg);

	valueInt = *(GM_RSSP1_UINT32*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT32);
	pSFM_conn_cfg->fsfb_comm_cycle = (GM_RSSP1_UINT32)valueInt;

	/**> 增加main_cycle可选字段 @63343*/  /* CR:GM00007617 */
	valueInt = *(GM_RSSP1_UINT32*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT32);
	pSFM_conn_cfg->main_cycle = (GM_RSSP1_UINT32)valueInt;  


	/**> 增加source_addr可选字段 @63343*/   /* CR:GM00007617 */
	valueInt = *(GM_RSSP1_UINT16*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT16);
	pSFM_conn_cfg->source_addr = (GM_RSSP1_UINT16)valueInt;


	valueInt = *(GM_RSSP1_UINT16*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT16);
	pSFM_conn_cfg->dest_addr = (GM_RSSP1_UINT16)valueInt;

	/* init FSFB param */
	valueInt = *(GM_RSSP1_UINT16*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT16);
	pSFM_conn_cfg->deltaTime = (GM_RSSP1_UINT16)valueInt;

	valueInt = *(GM_RSSP1_UINT16*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT16);
	pSFM_conn_cfg->DelayTime = (GM_RSSP1_UINT16)valueInt;

	valueInt = *(GM_RSSP1_UINT16*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT16);
	pSFM_conn_cfg->lifeTime = (GM_RSSP1_UINT16)valueInt;

	valueInt = *(GM_RSSP1_UINT16*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT16);
	pSFM_conn_cfg->torlerate_cycle_cfg = (GM_RSSP1_UINT16)valueInt;

	valueInt = *(GM_RSSP1_UINT8*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT8);
	pSFM_conn_cfg->num_data_ver = (GM_RSSP1_UINT8)valueInt;

	for (i=0U; i<GM_RSSP1_CHECK_CHN_NUM; i++)
	{
		/**> 增加local_sys_chk可选字段 @63343*/
		if (GM_RSSP1_SFM_Bin_MultLoc == g_InitMode)
		{
			valueInt = *(GM_RSSP1_UINT32*)pFileContent;
			pFileContent += sizeof(GM_RSSP1_UINT32);
			pSFM_conn_cfg->fsfb_chn_cfg[i].local_sys_chk = valueInt;
		}	
		else
		{
			valueInt = *(GM_RSSP1_UINT32*)pFileContent;
			pFileContent += sizeof(GM_RSSP1_UINT32);
			pSFM_conn_cfg->fsfb_chn_cfg[i].local_sys_chk = ~valueInt;
		}

		/**> 增加local_sid可选字段 @63343*/
		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
		pSFM_conn_cfg->fsfb_chn_cfg[i].local_sid = valueInt;

		/**> 增加local_sinit可选字段 @63343*/
		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
		pSFM_conn_cfg->fsfb_chn_cfg[i].local_sinit = valueInt;

		
		/**> 增加local_dataVer可选字段 @63343*/
		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
		pSFM_conn_cfg->fsfb_chn_cfg[i].local_dataVer = valueInt;



		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
		pSFM_conn_cfg->fsfb_chn_cfg[i].remote_sid = valueInt;

		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
		pSFM_conn_cfg->fsfb_chn_cfg[i].remote_sinit = valueInt;

		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
		pSFM_conn_cfg->fsfb_chn_cfg[i].remote_dataVer = valueInt;
	}

	

	/* 对等设备各个UDP通道均使用独立的FSFB。此时UDP通道数必须为1 */
	valueInt = *(GM_RSSP1_BOOL*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_BOOL);
	if (isbig != isbinbig)
	{
		valueInt = GM_RSSP1_Code_Big_Little_Endian32(valueInt);
	}
	pSFM_conn_cfg->b_enable_FSFB_on_chn = (GM_RSSP1_BOOL)valueInt;

	if (GM_RSSP1_TRUE == pSFM_conn_cfg->b_enable_FSFB_on_chn)
	{
		/* init UDP chn apply FSFB ID */
		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
		if (isbig != isbinbig)/*CR3071*/
		{
			valueInt = GM_RSSP1_Code_Big_Little_Endian32(valueInt);
		}
		pSFM_conn_cfg->Chn_apply_FSFB_ID = (GM_RSSP1_UINT32)valueInt;

		/* init msg queue in the SFM connection */
		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
		if (isbig != isbinbig)/*CR3071*/
		{
			valueInt = GM_RSSP1_Code_Big_Little_Endian32(valueInt);
		}
		
		pSFM_conn_cfg->L2U_dat_Q_size = (GM_RSSP1_UINT32)valueInt;
		pSFM_conn_cfg->L2U_dat_Q_offset = (GM_RSSP1_Pointer)GM_RSSP1_malloc((size_t)valueInt*sizeof(GM_RSSP1_SFM_L2U_pri_struct));
		if (0U == pSFM_conn_cfg->L2U_dat_Q_offset)
		{
			return GM_RSSP1_FALSE;
		}
		GM_RSSP1_memset((void*)pSFM_conn_cfg->L2U_dat_Q_offset, 0U, (size_t)valueInt*sizeof(GM_RSSP1_SFM_L2U_pri_struct));
	}
	else
	{
		pFileContent += sizeof(GM_RSSP1_UINT32)*2U;
	}

	/**> 增加remote_dev_AB可选字段 @63343*/
	valueInt = *(GM_RSSP1_UINT16*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT16);
	if (isbig != isbinbig)
	{
		valueInt = GM_RSSP1_Code_Big_Little_Endian16((GM_RSSP1_UINT16)valueInt);
	}
	if(GM_RSSP1_TRUE == (GM_RSSP1_BOOL)valueInt)
	{
		pSFM_conn_cfg->remote_dev_AB = (GM_RSSP1_UINT16)GM_RSSP1_Remote_Dev_A;		
	}
	else if(GM_RSSP1_FALSE == (GM_RSSP1_BOOL)valueInt)
	{
		pSFM_conn_cfg->remote_dev_AB = (GM_RSSP1_UINT16)GM_RSSP1_Remote_Dev_B;	
	}
	else
	{
		pSFM_conn_cfg->remote_dev_AB = (GM_RSSP1_UINT16)GM_RSSP1_Remote_Dev_Unknow;	 /**CR 3071 flt**/
	}




	/**> 增加IsFixNode可选字段 @63343*/
	valueInt = *(GM_RSSP1_UINT32*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT32);
	if (isbig != isbinbig)
	{
		valueInt = GM_RSSP1_Code_Big_Little_Endian32((GM_RSSP1_UINT32)valueInt);
	}
	pSFM_conn_cfg->IsFixNode = valueInt;

	if(valueInt == GM_RSSP1_TRUE )
	{
		g_Cfg_IsFixNode = g_Cfg_IsFixNode + 1;
	}

	if(g_Cfg_IsFixNode > g_Max_ConnctNum)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_SFM_IsFixNum_Error, rt, (int)valueInt, (int)g_Max_ConnctNum, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}


	if (isbig != isbinbig)
	{
		pSFM_conn_cfg->fsfb_comm_cycle = GM_RSSP1_Code_Big_Little_Endian32(pSFM_conn_cfg->fsfb_comm_cycle);
		pSFM_conn_cfg->main_cycle = GM_RSSP1_Code_Big_Little_Endian32(pSFM_conn_cfg->main_cycle); /* CR:GM00007617 */
		pSFM_conn_cfg->source_addr = GM_RSSP1_Code_Big_Little_Endian16(pSFM_conn_cfg->source_addr); /* CR:GM00007617 */
		pSFM_conn_cfg->dest_addr = GM_RSSP1_Code_Big_Little_Endian16(pSFM_conn_cfg->dest_addr);
		pSFM_conn_cfg->deltaTime = GM_RSSP1_Code_Big_Little_Endian16(pSFM_conn_cfg->deltaTime);
		pSFM_conn_cfg->DelayTime = GM_RSSP1_Code_Big_Little_Endian16(pSFM_conn_cfg->DelayTime);
		pSFM_conn_cfg->lifeTime = GM_RSSP1_Code_Big_Little_Endian16(pSFM_conn_cfg->lifeTime);
		pSFM_conn_cfg->torlerate_cycle_cfg = GM_RSSP1_Code_Big_Little_Endian16(pSFM_conn_cfg->torlerate_cycle_cfg);
		for (i=0U; i<2U; ++i)
		{
			pSFM_conn_cfg->fsfb_chn_cfg[i].local_sid       = GM_RSSP1_Code_Big_Little_Endian32(pSFM_conn_cfg->fsfb_chn_cfg[i].local_sid);
			pSFM_conn_cfg->fsfb_chn_cfg[i].local_sinit     = GM_RSSP1_Code_Big_Little_Endian32(pSFM_conn_cfg->fsfb_chn_cfg[i].local_sinit);
			pSFM_conn_cfg->fsfb_chn_cfg[i].local_dataVer   = GM_RSSP1_Code_Big_Little_Endian32(pSFM_conn_cfg->fsfb_chn_cfg[i].local_dataVer);
			pSFM_conn_cfg->fsfb_chn_cfg[i].local_sys_chk   = GM_RSSP1_Code_Big_Little_Endian32(pSFM_conn_cfg->fsfb_chn_cfg[i].local_sys_chk);

			pSFM_conn_cfg->fsfb_chn_cfg[i].remote_sid = GM_RSSP1_Code_Big_Little_Endian32(pSFM_conn_cfg->fsfb_chn_cfg[i].remote_sid);
			pSFM_conn_cfg->fsfb_chn_cfg[i].remote_sinit = GM_RSSP1_Code_Big_Little_Endian32(pSFM_conn_cfg->fsfb_chn_cfg[i].remote_sinit);
			pSFM_conn_cfg->fsfb_chn_cfg[i].remote_dataVer = GM_RSSP1_Code_Big_Little_Endian32(pSFM_conn_cfg->fsfb_chn_cfg[i].remote_dataVer);
		}
		
		
	}

	*r_ptr = pFileContent;

	return GM_RSSP1_TRUE;
}

GM_RSSP1_BOOL GM_RSSP1_SFM_Init_Addr(GM_RSSP1_UINT8 *addr, GM_RSSP1_BOOL isbinbig)	/*CR:GM00003071添加识别bin文件大小端的参数isbinbig*/
{
	GM_RSSP1_comm_cfg_struct fsfb_cfg ;
	GM_RSSP1_SFM_object_struct *pSFM = NULL;
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	static GM_RSSP1_BOOL is_boot = GM_RSSP1_FALSE;

	pSFM = GM_RSSP1_Get_SFM_Object();

	if ((NULL != addr) || (NULL != pSFM))
	{
		if (is_boot == GM_RSSP1_FALSE)/**由于配置数据允许增删操作,只有在第一次加载是进行数据复位包括Hash表**/
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

		rt = GM_RSSP1_Init_Comm_Cfg_SFM_Addr(addr, &fsfb_cfg, isbinbig);  /* CR:GM00007614*/
		if (GM_RSSP1_TRUE == rt )
		{
			if (is_boot == GM_RSSP1_FALSE)/**由于配置数据允许增删操作,只有在第一次进行队列空间申请**/
			{
				rt = GM_RSSP1_SFM_MQ_Init(&fsfb_cfg , pSFM);  /* CR:GM00007614*/
				if (GM_RSSP1_TRUE == rt )
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

void GM_RSSP1_SFM_Init_Addr_ModeSlt(GM_RSSP1_SFM_InitBinMode_enum initMode)
{
	g_InitMode = initMode;
}
#endif
