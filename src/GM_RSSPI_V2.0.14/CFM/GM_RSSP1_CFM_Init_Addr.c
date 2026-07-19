
#include "GM_RSSP1_CFM_Init.h"
#include "GM_RSSP1_CFM_Interface.h"

#ifdef CFM_Part
extern RSSP1_MQ_Inter_struct g_CFM_MQ;
extern RSSP1_MQ_LINK_struct g_Link_MQ;
extern GM_RSSP1_Local_Con_struct g_loc_info;
extern GM_RSSP1_UINT16 g_Max_ConnctNum;

/*CR:GM00003071 添加标记bin文件大小端的参数*/
GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Cfg_CFM_Addr(GM_RSSP1_UINT8 *pFileContent, GM_RSSP1_comm_cfg_struct *pCfg,GM_RSSP1_BOOL isbinbig)
{
	GM_RSSP1_UINT8 **rp_file = &pFileContent;
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 arryIndex = 0U;
	GM_RSSP1_UINT16 indexArry[GM_RSSP1_MAX_SAFETY_CONNECTION_NUM] = {0U};
    GM_RSSP1_INT32 file_len = -1;
    GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_CFM_object_struct *pCFM = NULL;

	pCFM = GM_RSSP1_Get_CFM_Object();
    if ((NULL == pCFM) || (NULL == pFileContent) || (NULL == pCfg))
    {
        return bRt;
    }

    /* 顺序不可变动。初始化各连接时需要从SFM cfg中获取本地连接通道共享的一些配置 */
	bRt = GM_RSSP1_Init_Comm_Global_CFM_Addr(pCfg, pFileContent, rp_file, isbinbig);
	if (GM_RSSP1_TRUE == bRt)
	{
		GM_RSSP1_memset((void*)indexArry, 0xFFU,sizeof(indexArry));

#ifdef SFM_Part
		GM_RSSP1_GetSeqIndexReverse(OffLine_Object, 1U);
		for (l_index=0U; l_index<pCfg->CFM_cfg.connection_nums; ++l_index)/**当SFM与CFM一起时,SFM已在离线索引链表添加,CFM需索引SFM所在链表地址**/
		{
			/**把index预取**/
			arryIndex = pCfg->CFM_cfg.connection_nums-l_index-1U;
			indexArry[arryIndex] = GM_RSSP1_GetSeqIndexReverse(OffLine_Object, 0U);
		}
#else
		/*初始化在线链表*/
		GM_RSSP1_InitSeqArray(OnLine_Object, g_Max_ConnctNum);  /* CR: 11036019*/
#endif
		GM_RSSP1_Hash_CFM_ActInit((GM_RSSP1_UINT32)g_Max_ConnctNum);   /* CR: 11036019*/


		for (l_index=0U; l_index<pCfg->CFM_cfg.connection_nums; ++l_index)
		{
			bRt = GM_RSSP1_Init_Comm_Connection_CFM_Addr(pCfg, pFileContent, rp_file, isbinbig);
			if (GM_RSSP1_TRUE == bRt)
			{
				bRt = GM_RSSP1_CFM_Init_Lib(indexArry[l_index],pCfg, pCFM);
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
GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Global_CFM_Addr(GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_UINT8 *pFileContent, GM_RSSP1_UINT8 **r_ptr,GM_RSSP1_BOOL isbinbig)
{
    GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
    GM_RSSP1_UINT32 valueInt = 0U;
	GM_RSSP1_BOOL isbig = GM_RSSP1_Code_Is_Big_Endian();

    if ((NULL == pCfg) || (NULL == pFileContent))
    {
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_CFM_Param_Point_Error, 0, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    /* RSSP1_GLOBAL 全局配置 */
	valueInt = *(GM_RSSP1_UINT16*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT16);
	if (isbig != isbinbig)
	{
		valueInt = GM_RSSP1_Code_Big_Little_Endian16((GM_RSSP1_UINT16)valueInt);
	}
    if (valueInt > GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
    {
        return GM_RSSP1_FALSE;
    }
    else
    {
        pCfg->CFM_cfg.connection_nums = (GM_RSSP1_UINT16)valueInt;
    }

	valueInt = *(GM_RSSP1_UINT16*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT16);
	if (isbig != isbinbig)
	{
		valueInt = GM_RSSP1_Code_Big_Little_Endian16((GM_RSSP1_UINT16)valueInt);
	}
	pCfg->CFM_cfg.source_addr = (GM_RSSP1_UINT16)valueInt;

	valueInt = *(GM_RSSP1_UINT32*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT32);
	if (isbig != isbinbig)
	{
		valueInt = GM_RSSP1_Code_Big_Little_Endian32(valueInt);
	}
	pCfg->CFM_cfg.U2L_pri_Q_size    = (GM_RSSP1_UINT32)valueInt;
	
	valueInt = *(GM_RSSP1_UINT32*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT32);
	if (isbig != isbinbig)
	{
		valueInt = GM_RSSP1_Code_Big_Little_Endian32(valueInt);
	}
    pCfg->CFM_cfg.L2U_pri_Q_size = (GM_RSSP1_UINT32)valueInt;

	valueInt = *(GM_RSSP1_UINT32*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT32);
	if (isbig != isbinbig)
	{
		valueInt = GM_RSSP1_Code_Big_Little_Endian32(valueInt);
	}
	if ((valueInt <= 0) || (valueInt > (GM_RSSP1_INT32)GM_RSSP1_MAX_USER_DAT_LEN))	/*CR:GM00004940 修正为最大用户数据，MAX_SAFETY_DAT_LEN -22 = MAX_USER_DAT_LEN*/
	{
		pCfg->CFM_cfg.all0_size = 0xFFFFFFFFU;
	}
	else
	{
		pCfg->CFM_cfg.all0_size = (GM_RSSP1_UINT32)valueInt;
	}


	/**> 增加g_Max_ConnctNum 可选字段 @63343*/
	valueInt = *(GM_RSSP1_UINT32*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT32);

	if (isbig != isbinbig)
	{
		valueInt = GM_RSSP1_Code_Big_Little_Endian32(valueInt);
	}
	if (valueInt != 0U)
	{
		if (valueInt > GM_RSSP1_MAX_ONLINE_CONNECTION_NUM)
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


	*r_ptr = pFileContent;
    return GM_RSSP1_TRUE;
}

/* CR: 11036019*/
GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Connection_CFM_Addr(GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_UINT8 *pFileContent,GM_RSSP1_UINT8 **r_ptr,GM_RSSP1_BOOL isbinbig)
{
    GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
    GM_RSSP1_UINT32 valueInt = 0xFFFFFFFFU;
    GM_RSSP1_INT32 chn_index = 0;
	GM_RSSP1_BOOL isbig = GM_RSSP1_Code_Is_Big_Endian();
    GM_RSSP1_CFM_connection_cfg_struct *pCFM_conn_cfg = NULL;

    if ((NULL == pCfg) || (NULL == pFileContent))
    {
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_CFM_Param_Point_Error, (int)pCfg, (int)pFileContent, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
    }

    /* 各个连接通道配置 */
	pCFM_conn_cfg = &(pCfg->CFM_cfg.connection_cfg);

	valueInt = *(GM_RSSP1_UINT16*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT16);
	pCFM_conn_cfg->dest_addr = (GM_RSSP1_UINT16)valueInt;

	valueInt = *(GM_RSSP1_BOOL*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_BOOL);
	pCFM_conn_cfg->b_enable_CRSCD_pack = (GM_RSSP1_BOOL)valueInt;

	/* init com type */
	valueInt = *(GM_RSSP1_UINT16*)pFileContent;
	pFileContent += sizeof(GM_RSSP1_UINT16);
	if (isbig != isbinbig)
	{
		valueInt = GM_RSSP1_Code_Big_Little_Endian16((GM_RSSP1_UINT16)valueInt);
	}
	if(0U == valueInt)	/*com_type=0 不受大小端影响*/
	{
		pCFM_conn_cfg->com_type = GM_RSSP1_CFM_COMM_TYPE_UDP;
	}
	else if(1U == valueInt)
	{
		pCFM_conn_cfg->com_type = GM_RSSP1_CFM_COMM_TYPE_SCOM;
	}
	else
	{
		pCFM_conn_cfg->com_type = GM_RSSP1_CFM_COMM_TYPE_UNKNOW;
	}

	if(GM_RSSP1_CFM_COMM_TYPE_UDP == pCFM_conn_cfg->com_type)
	{
		/* init UDP channel param */
		valueInt = *(GM_RSSP1_UINT16*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT16);
		if (isbig != isbinbig)
		{
			valueInt = GM_RSSP1_Code_Big_Little_Endian16((GM_RSSP1_UINT16)valueInt);
		}
		if (valueInt > GM_RSSP1_CFM_MAX_CHANNEL_NUM)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_CFM_UDP_Num_Fail, valueInt, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
		else
		{
			pCFM_conn_cfg->chn_num = (GM_RSSP1_UINT16)valueInt;
		}

		/* init UDP msg queues */
		for (chn_index = 0 ; chn_index < (GM_RSSP1_INT32)pCFM_conn_cfg->chn_num ; ++chn_index)
		{
			valueInt = *(GM_RSSP1_UINT32*)pFileContent;
			pFileContent += sizeof(GM_RSSP1_UINT32);
			pCFM_conn_cfg->UDP_chn_cfg[chn_index].loc_ip = valueInt;

			valueInt = *(GM_RSSP1_UINT32*)pFileContent;
			pFileContent += sizeof(GM_RSSP1_UINT32);
			pCFM_conn_cfg->UDP_chn_cfg[chn_index].loc_port= valueInt;

			valueInt = *(GM_RSSP1_UINT32*)pFileContent;
			pFileContent += sizeof(GM_RSSP1_UINT32);
			pCFM_conn_cfg->UDP_chn_cfg[chn_index].rem_ip = valueInt;

			valueInt = *(GM_RSSP1_UINT32*)pFileContent;
			pFileContent += sizeof(GM_RSSP1_UINT32);
			pCFM_conn_cfg->UDP_chn_cfg[chn_index].rem_port= valueInt;
		}

		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
		if (isbig != isbinbig)
		{
			valueInt = GM_RSSP1_Code_Big_Little_Endian32(valueInt);
		}

		pCFM_conn_cfg->recv_Q_size = valueInt;


		valueInt = *(GM_RSSP1_UINT32*)pFileContent;
		pFileContent += sizeof(GM_RSSP1_UINT32);
		if (isbig != isbinbig)
		{
			valueInt = GM_RSSP1_Code_Big_Little_Endian32(valueInt);
		}

		pCfg->CFM_cfg.SendQ_Total += valueInt * pCFM_conn_cfg->chn_num;

		if (isbig != isbinbig)
		{
			pCFM_conn_cfg->dest_addr = GM_RSSP1_Code_Big_Little_Endian16(pCFM_conn_cfg->dest_addr);
			pCFM_conn_cfg->b_enable_CRSCD_pack = (GM_RSSP1_BOOL)GM_RSSP1_Code_Big_Little_Endian32((GM_RSSP1_UINT32)pCFM_conn_cfg->b_enable_CRSCD_pack);

			/* init UDP msg queues */
			for (chn_index = 0 ; chn_index < (GM_RSSP1_INT32)pCFM_conn_cfg->chn_num ; ++chn_index)
			{
				pCFM_conn_cfg->UDP_chn_cfg[chn_index].loc_ip = GM_RSSP1_Code_Big_Little_Endian32(pCFM_conn_cfg->UDP_chn_cfg[chn_index].loc_ip);
				pCFM_conn_cfg->UDP_chn_cfg[chn_index].loc_port= GM_RSSP1_Code_Big_Little_Endian32(pCFM_conn_cfg->UDP_chn_cfg[chn_index].loc_port);
				pCFM_conn_cfg->UDP_chn_cfg[chn_index].rem_ip = GM_RSSP1_Code_Big_Little_Endian32(pCFM_conn_cfg->UDP_chn_cfg[chn_index].rem_ip);
				pCFM_conn_cfg->UDP_chn_cfg[chn_index].rem_port= GM_RSSP1_Code_Big_Little_Endian32(pCFM_conn_cfg->UDP_chn_cfg[chn_index].rem_port);
			}
		}
	}

	*r_ptr = pFileContent;

    return GM_RSSP1_TRUE;
}

GM_RSSP1_BOOL GM_RSSP1_CFM_Init_Addr(GM_RSSP1_UINT8 *addr,GM_RSSP1_BOOL isbinbig)	/*jcf：添加bin文件大小端识别参数isbinbig*/
{
	GM_RSSP1_CFM_object_struct *pCFM = NULL;
	GM_RSSP1_comm_cfg_struct fsfb_cfg;
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	static GM_RSSP1_BOOL is_boot = GM_RSSP1_FALSE;

	pCFM = GM_RSSP1_Get_CFM_Object();

	if ((NULL != addr)||(pCFM != NULL))
	{
		if (is_boot == GM_RSSP1_FALSE)/**由于配置数据允许增删操作,只有在第一次加载是进行数据复位包括Hash表**/
		{
			GM_RSSP1_Hash_CFM_Init();
			GM_RSSP1_memset((void*)pCFM, 0U, sizeof(GM_RSSP1_CFM_object_struct));
			pCFM->lib_state = GM_RSSP1_Lib_State_Unknown;
		}

		GM_RSSP1_memset((void*)&fsfb_cfg, 0U, sizeof(GM_RSSP1_comm_cfg_struct));
		rt = GM_RSSP1_Init_Comm_Cfg_CFM_Addr(addr, &fsfb_cfg,isbinbig);
		if (GM_RSSP1_TRUE == rt )
		{
			rt = GM_RSSP1_CFM_MQ_Init(&fsfb_cfg , pCFM);
			if (GM_RSSP1_TRUE == rt)
			{
				is_boot = GM_RSSP1_TRUE;
				rt = GM_RSSP1_TRUE;
			}
		}
	}

	return rt;
}
#endif
