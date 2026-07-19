
#include "GM_RSSP1_CFM_Init.h"
#include "GM_RSSP1_CFM_Interface.h"

#ifdef CFM_Part

extern RSSP1_MQ_Inter_struct g_CFM_MQ;
extern RSSP1_MQ_LINK_struct g_Link_MQ;

extern GM_RSSP1_BOOL g_MaxPerform;

#ifdef	CFM_Stanby_Answer
extern GM_RSSP1_Msg_Queue_struct g_CFM_Debug_MQ;
#endif
extern GM_RSSP1_Local_Con_struct g_loc_info;

extern GM_RSSP1_UINT16 g_Max_ConnctNum;


GM_RSSP1_Pointer Debug_Q_offset = 0U;


#ifdef FILE_SYSTEM
GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Cfg_CFM(GM_RSSP1_INT8 *pIniFileContent , GM_RSSP1_comm_cfg_struct *pCfg)
{
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 arryIndex = 0U;
	GM_RSSP1_UINT16 indexArry[GM_RSSP1_MAX_SAFETY_CONNECTION_NUM] = {0U};
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_CFM_object_struct *pCFM = NULL;

	pCFM = GM_RSSP1_Get_CFM_Object();

    if ((NULL == pCFM) || (NULL == pIniFileContent) || (NULL == pCfg))
    {
        return bRt;
    }

    /* 顺序不可变动。初始化各连接时需要从SFM cfg中获取本地连接通道共享的一些配置 */
	bRt = GM_RSSP1_Init_Comm_Global_CFM(pCfg, pIniFileContent);

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
		GM_RSSP1_InitSeqArray(OnLine_Object, g_Max_ConnctNum);
#endif
		GM_RSSP1_Hash_CFM_ActInit((GM_RSSP1_UINT32)g_Max_ConnctNum);

		for (l_index=0U; l_index<pCfg->CFM_cfg.connection_nums; ++l_index)
		{
			bRt = GM_RSSP1_Init_Comm_Connection_CFM(l_index, pCfg, pIniFileContent);
			if (GM_RSSP1_TRUE == bRt)
			{
				bRt = GM_RSSP1_CFM_Init_Lib(indexArry[l_index], pCfg, pCFM);
			}

			if (GM_RSSP1_FALSE == bRt)
			{
				break;
			}
		}
    }

    return bRt;
}

GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Global_CFM(GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_INT8 *pIniFileContent)
{
    GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
    GM_RSSP1_INT32 valueInt = -1;
    char valueString[256U] = {'\0'};

    if ((NULL == pCfg) || (NULL == pIniFileContent))
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_CFM_Param_Point_Error, (int)pCfg, (int)pIniFileContent, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    /* RSSP1_GLOBAL 全局配置 */
    rt = GM_RSSP1_Ini_File_Read_Int("RSSP1_GLOBAL", "connection_num", (int*)&valueInt, pIniFileContent);

    if ((GM_RSSP1_FALSE == rt) || (valueInt < 0) || (valueInt > (GM_RSSP1_INT32)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM))
    {
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_CFM_Connect_Num_Fail, rt, valueInt, GM_RSSP1_MAX_SAFETY_CONNECTION_NUM, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }
    else
    {
        pCfg->CFM_cfg.connection_nums = (GM_RSSP1_UINT16)valueInt;
    }

	if (g_Max_ConnctNum == 0U)
	{
		rt = GM_RSSP1_Ini_File_Read_Int("RSSP1_GLOBAL", "Max_ConnectNum", (int*)&valueInt, pIniFileContent);

		if (GM_RSSP1_TRUE == rt)
		{
			if ((valueInt < 0)||(valueInt > GM_RSSP1_MAX_ONLINE_CONNECTION_NUM))
			{
				GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_CFM_Max_Num_Fail, valueInt, GM_RSSP1_MAX_ONLINE_CONNECTION_NUM, 0, 0, 0, 0);
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

    rt = GM_RSSP1_Ini_File_Read_String("RSSP1_GLOBAL", "source_addr", valueString , (int)sizeof(valueString), pIniFileContent);

    if (GM_RSSP1_FALSE == rt)
    {
		 pCfg->CFM_cfg.source_addr = 0U;    /*CR:GM00007615*/
		/*GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_CFM_Src_Addr_Fail, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;*/
    }
    else
    {
        pCfg->CFM_cfg.source_addr = (GM_RSSP1_UINT16)(strtoul(valueString , NULL , 16));
    }

    rt = GM_RSSP1_Ini_File_Read_Int("RSSP1_GLOBAL", "CFM_U2L_Q_Size_per_connection", (int*)&valueInt, pIniFileContent);

    if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_CFM_U2L_Q_Fail, 0, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }
    else
    {
        pCfg->CFM_cfg.U2L_pri_Q_size    = (GM_RSSP1_UINT32)valueInt;
    }

    rt = GM_RSSP1_Ini_File_Read_Int("RSSP1_GLOBAL", "CFM_L2U_Q_Size_per_connection", (int*)&valueInt, pIniFileContent);

    if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_CFM_L2U_Q_Fail, 0, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }
    else
    {
        pCfg->CFM_cfg.L2U_pri_Q_size    = (GM_RSSP1_UINT32)valueInt;
    }

    /* start:CR:GM00002526, GM162, fulintai,2015-06-09, add configuration for all 0 message size in standby's CFM*/
    rt = GM_RSSP1_Ini_File_Read_Int("RSSP1_GLOBAL", "UsrData_All0_Size", (int*)&valueInt, pIniFileContent);

    if ((GM_RSSP1_FALSE == rt) || (valueInt <= 0) || (valueInt > (GM_RSSP1_INT32)GM_RSSP1_MAX_USER_DAT_LEN))	/*CR:GM00004940 修正为最大用户数据，MAX_SAFETY_DAT_LEN -22 = MAX_USER_DAT_LEN*/
    {
        pCfg->CFM_cfg.all0_size = 0xFFFFFFFFU;
    }
    else
    {
        pCfg->CFM_cfg.all0_size = (GM_RSSP1_UINT32)valueInt;
    }
    /* end:CR:GM00002526*/
    return GM_RSSP1_TRUE;
}

GM_RSSP1_BOOL GM_RSSP1_Init_Comm_Connection_CFM(GM_RSSP1_UINT16 conn_index, GM_RSSP1_comm_cfg_struct *pCfg , GM_RSSP1_INT8 *pIniFileContent)
{
    GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
    GM_RSSP1_INT32 valueInt = -1;
    GM_RSSP1_INT8 valueString[256] = {0};
    GM_RSSP1_BOOL valueBool = GM_RSSP1_FALSE;
    GM_RSSP1_INT8 section_key[256] = {0};   /* 连接通道键名 */
    GM_RSSP1_INT8 sub_section_key[256] = {0};   /* UDP通道键名 */
    GM_RSSP1_INT32 key_len = 0;
    GM_RSSP1_INT32 sub_key_len = 0;
    GM_RSSP1_INT32 chn_index = 0;
    GM_RSSP1_CFM_connection_cfg_struct *pCFM_conn_cfg = NULL;

    if ((NULL == pCfg) || (NULL == pIniFileContent))
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_CFM_Param_Point_Error, (int)pCfg, (int)pIniFileContent, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    /* 各个连接通道配置 */
    key_len = (GM_RSSP1_INT32)(sizeof("CON_") - 1);

    GM_RSSP1_memcpy((void*)section_key, "CON_", ((size_t)key_len + (size_t)1));

	pCFM_conn_cfg = &(pCfg->CFM_cfg.connection_cfg);

	GM_RSSP1_memset((void*)(section_key + key_len), 0U, (size_t)(sizeof(section_key) - key_len));
	sprintf(section_key + key_len, "%d", conn_index);

	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "dest_addr", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_CFM_Des_Addr_Fail, conn_index, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pCFM_conn_cfg->dest_addr = (GM_RSSP1_UINT16)(strtoul(valueString , NULL , 16));
	}

	rt = GM_RSSP1_Ini_File_Read_BOOL((const char *)section_key , "enable_CRSCD_pack" , &valueBool, pIniFileContent);

	if (GM_RSSP1_FALSE == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_CFM_Enable_Pack_Fail, conn_index, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pCFM_conn_cfg->b_enable_CRSCD_pack = valueBool;
	}


	/* CR:GM00007615 */
	rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, "source_addr", (char *)valueString , (int)sizeof(valueString), pIniFileContent);

	if (GM_RSSP1_TRUE == rt)
	{
		pCfg->CFM_cfg.source_addr = (GM_RSSP1_UINT16)(strtoul(valueString , NULL , 16));
	}
	else if (0U == pCfg->CFM_cfg.source_addr)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Global_CFM_Src_Error, rt, (int)valueInt, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		;/*do nothing*/
	}


	/* init com type */
	rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "com_type", (int*)&valueInt, pIniFileContent);

	if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_CFM_Com_Type_Fail, conn_index, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		if(0 == valueInt)
		{
			pCFM_conn_cfg->com_type = GM_RSSP1_CFM_COMM_TYPE_UDP;
		}
		else if(1 == valueInt)
		{
			pCFM_conn_cfg->com_type = GM_RSSP1_CFM_COMM_TYPE_SCOM;
		}
		else
		{
			pCFM_conn_cfg->com_type = GM_RSSP1_CFM_COMM_TYPE_UNKNOW;
		}
	}

	if(GM_RSSP1_CFM_COMM_TYPE_UDP == pCFM_conn_cfg->com_type)
	{
		/* init UDP channel param */
		rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "UDP_channel_num", (int*)&valueInt, pIniFileContent);

		if ((GM_RSSP1_FALSE == rt) || (valueInt < 0) || (valueInt > (GM_RSSP1_INT32)GM_RSSP1_CFM_MAX_CHANNEL_NUM))
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_CFM_UDP_Num_Fail, conn_index, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
		else
		{
			pCFM_conn_cfg->chn_num = (GM_RSSP1_UINT16)valueInt;
		}
	}

	/* init UDP msg queues */
	for (chn_index = 0 ; chn_index < (GM_RSSP1_INT32)pCFM_conn_cfg->chn_num ; ++chn_index)
	{

		sub_key_len = (GM_RSSP1_INT32)(sizeof("local_ip_") - 1U);
		GM_RSSP1_memcpy((void*)sub_section_key, "local_ip_", ((size_t)sub_key_len + (size_t)1));
		GM_RSSP1_memset((void*)(sub_section_key + sub_key_len), 0U, 2U);
		sprintf(sub_section_key + sub_key_len, "%d", chn_index);
		rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, (const char *)sub_section_key, (char *)valueString, (int)sizeof(valueString), pIniFileContent);

		if (GM_RSSP1_FALSE == rt)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_CFM_Loc_IP_Fail, conn_index, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
		else
		{
			pCFM_conn_cfg->UDP_chn_cfg[chn_index].loc_ip = inet_addr(valueString);
		}

		sub_key_len = (GM_RSSP1_INT32)(sizeof("local_port_") - 1U);

		GM_RSSP1_memcpy((void*)sub_section_key, "local_port_", ((size_t)sub_key_len + (size_t)1));
		GM_RSSP1_memset((void*)(sub_section_key + sub_key_len), 0U, 2U);
		sprintf(sub_section_key + sub_key_len, "%d", chn_index);
		rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, (const char *)sub_section_key, (int*)&valueInt, pIniFileContent);

		if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_CFM_Loc_Port_Fail, conn_index, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
		else
		{
			pCFM_conn_cfg->UDP_chn_cfg[chn_index].loc_port= (GM_RSSP1_UINT32)valueInt;
		}

		sub_key_len = (GM_RSSP1_INT32)(sizeof("remote_ip_") - 1U);
		GM_RSSP1_memcpy((void*)sub_section_key, "remote_ip_", ((size_t)sub_key_len + (size_t)1));
		GM_RSSP1_memset((void*)(sub_section_key + sub_key_len), 0U, 2U);
		sprintf(sub_section_key + sub_key_len, "%d", chn_index);
		rt = GM_RSSP1_Ini_File_Read_String((const char *)section_key, (const char *)sub_section_key,  (char *)valueString, (int)sizeof(valueString), pIniFileContent);

		if (GM_RSSP1_FALSE == rt)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_CFM_Rem_IP_Fail, conn_index, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
		else
		{
			pCFM_conn_cfg->UDP_chn_cfg[chn_index].rem_ip = inet_addr(valueString);
		}

		sub_key_len = (GM_RSSP1_INT32)(sizeof("remote_port_") - 1U);

		GM_RSSP1_memcpy((void*)sub_section_key, "remote_port_", ((size_t)sub_key_len + (size_t)1));
		GM_RSSP1_memset((void*)(sub_section_key + sub_key_len), 0U, 2U);
		sprintf(sub_section_key + sub_key_len, "%d", chn_index);
		rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, (const char *)sub_section_key, (int*)&valueInt, pIniFileContent);

		if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_CFM_Rem_Port_Fail, conn_index, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
		else
		{
			pCFM_conn_cfg->UDP_chn_cfg[chn_index].rem_port= (GM_RSSP1_UINT32)valueInt;
		}
	}

	rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "UDP_recv_MQ_size", (int*)&valueInt, pIniFileContent);

	if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_CFM_Rcv_Size_Fail, conn_index, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pCFM_conn_cfg->recv_Q_size = (GM_RSSP1_UINT32)valueInt;
	}

	rt = GM_RSSP1_Ini_File_Read_Int((const char *)section_key, "UDP_send_MQ_size", (int*)&valueInt, pIniFileContent);

	if ((GM_RSSP1_FALSE == rt) || (valueInt < 0))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Init_Comm_Connection_CFM_Snd_Size_Fail, conn_index, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else
	{
		pCfg->CFM_cfg.SendQ_Total += (GM_RSSP1_UINT32)valueInt * pCFM_conn_cfg->chn_num;
	}

    return GM_RSSP1_TRUE;
}

GM_RSSP1_BOOL GM_RSSP1_CFM_Init(GM_RSSP1_INT8 *file)
{
	GM_RSSP1_CFM_object_struct *pCFM = NULL;
	GM_RSSP1_comm_cfg_struct fsfb_cfg = {0U};
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	static GM_RSSP1_BOOL is_boot = GM_RSSP1_FALSE;

	pCFM = GM_RSSP1_Get_CFM_Object();

	/* defect: 14027787*/
	if ((NULL != file) && (pCFM != NULL)) 
	{
		if (is_boot == GM_RSSP1_FALSE)
		{
			GM_RSSP1_Hash_CFM_Init();
			GM_RSSP1_memset((void*)pCFM, 0U, sizeof(GM_RSSP1_CFM_object_struct));
			pCFM->lib_state = GM_RSSP1_Lib_State_Unknown;
		}

		GM_RSSP1_memset((void*)&fsfb_cfg, 0U, sizeof(GM_RSSP1_comm_cfg_struct));

		rt = GM_RSSP1_Init_Comm_Cfg_CFM(file, &fsfb_cfg);  /* CR:GM00007614*/
		if (GM_RSSP1_TRUE == rt)
		{
			rt = GM_RSSP1_CFM_MQ_Init(&fsfb_cfg , pCFM);  /* CR:GM00007614*/
			if (GM_RSSP1_TRUE == rt )
			{
				is_boot = GM_RSSP1_TRUE;
				rt = GM_RSSP1_TRUE;
			}

		}
	}

	return rt;
}
#endif

/* CR: 11036019*/
GM_RSSP1_BOOL GM_RSSP1_CFM_MQ_Init(GM_RSSP1_comm_cfg_struct *pcfg, GM_RSSP1_CFM_object_struct *pCFM)
{
	GM_RSSP1_UINT32 l_index = 0U;
	GM_RSSP1_UINT32 l_sub_index = 0U;
	GM_RSSP1_UINT32 conn_num = 0U;
	GM_RSSP1_UINT32 len = 0U;
	GM_RSSP1_INT32 counter = 0;
	GM_RSSP1_CFM_connection_cfg_struct *pConn_cfg = NULL;
	GM_RSSP1_CFM_connection_struct *pCFM_con = NULL;
	if ((NULL == pcfg) || (NULL == pCFM))
	{
		return GM_RSSP1_FALSE;
	}

	conn_num = (GM_RSSP1_UINT32)pCFM->connection_nums + (GM_RSSP1_UINT32)pcfg->CFM_cfg.connection_nums;

	/* init LINK msg Receive queue */
	for (l_index = (GM_RSSP1_UINT32)(pCFM->connection_nums) ; l_index < conn_num ; ++l_index)
	{
		if (l_index >= g_Max_ConnctNum)
		{
			break;
		}

		pCFM_con = &(pCFM->connection[l_index]); /*defect:13801928*/
		for (l_sub_index = 0U; l_sub_index < (GM_RSSP1_UINT32)pCFM_con->chn_num; ++l_sub_index)
		{
			if ((GM_RSSP1_Pointer)NULL == g_Link_MQ.RCV_MQ[l_index][l_sub_index].Q_offset)
			{
				len = (GM_RSSP1_UINT32)(pCFM_con->recv_Q_size * sizeof(GM_RSSP1_com_input_pri_struct));
				pCFM_con->com_chn[l_sub_index].recv_Q_offset =(GM_RSSP1_Pointer)GM_RSSP1_malloc((size_t)len);
				if ((GM_RSSP1_Pointer)NULL == pCFM_con->com_chn[l_sub_index].recv_Q_offset)
				{
					GM_RSSP1_Log_Usr(GM_RSSP1_CFM_MQ_Init_Link_RCV_Memory_Fail, l_index, l_sub_index, 0, 0, 0, 0);
					return GM_RSSP1_FALSE;
				}
				else
				{
					GM_RSSP1_memset((void *)pCFM_con->com_chn[l_sub_index].recv_Q_offset, (GM_RSSP1_UINT8)0U, (size_t)len);
				}

				if (GM_RSSP1_FALSE == FSFB_Msg_Queue_Init(
					&(g_Link_MQ.RCV_MQ[l_index][l_sub_index]),
					pCFM_con->recv_Q_size,
					(GM_RSSP1_Pointer)(pCFM_con->com_chn[l_sub_index].recv_Q_offset),
					FSFB_Q_TYPE_RCVD_PKT)
					)
				{
					GM_RSSP1_Log_Usr(GM_RSSP1_CFM_MQ_Init_Link_RCV_MQ_Fail, l_index, l_sub_index, 0, 0, 0, 0);
					return GM_RSSP1_FALSE;
				}
			}
		}
		counter++;
	}

	/* init LINK msg Sending queue */
	if ((GM_RSSP1_Pointer)NULL == (GM_RSSP1_Pointer)pCFM->SendQ_offset)
	{
		len = (GM_RSSP1_UINT32)(pcfg->CFM_cfg.SendQ_Total * sizeof(GM_RSSP1_com_out_pri_struct));
		pCFM->SendQ_offset = (GM_RSSP1_Pointer)GM_RSSP1_malloc((size_t)len);

		if ((GM_RSSP1_Pointer)NULL == (GM_RSSP1_Pointer)pCFM->SendQ_offset)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_CFM_MQ_Init_Link_SND_Memory_Fail, 0, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
		GM_RSSP1_memset((void*)pCFM->SendQ_offset, 0U, (size_t)len);

		if (GM_RSSP1_FALSE == FSFB_Msg_Queue_Init(
			&(g_Link_MQ.SND_MQ),
			pcfg->CFM_cfg.SendQ_Total,
			pCFM->SendQ_offset,
			FSFB_Q_TYPE_SENT_PKT)
			)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_CFM_MQ_Init_Link_SND_MQ_Fail, 0, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
	}


#ifndef GM_RSSP1_SAVING_MODE
	/* init CFM msg queue */
	if (NULL == (void*)(pCFM->U2L_pri_Q_offset))
	{
		len = (GM_RSSP1_UINT32)(pcfg->CFM_cfg.U2L_pri_Q_size * sizeof(GM_RSSP1_CFM_U2L_pri_struct));
		pCFM->U2L_pri_Q_offset = (GM_RSSP1_Pointer)GM_RSSP1_malloc((size_t)len);

		if ((GM_RSSP1_Pointer)NULL == (GM_RSSP1_Pointer)pCFM->U2L_pri_Q_offset)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_CFM_MQ_Init_U2L_Memory_Fail, 0, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
		GM_RSSP1_memset((void*)pCFM->U2L_pri_Q_offset, 0U, (size_t)len);

		if (GM_RSSP1_FALSE == FSFB_Msg_Queue_Init(&(g_CFM_MQ.SND_MQ), pcfg->CFM_cfg.U2L_pri_Q_size , pCFM->U2L_pri_Q_offset , FSFB_Q_TYPE_CFM_U2L_PRI))
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_CFM_MQ_Init_U2L_MQ_Fail, 1, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
	}

	if (NULL == (void*)(pCFM->L2U_pri_Q_offset))
	{
		len = (GM_RSSP1_UINT32)(pcfg->CFM_cfg.L2U_pri_Q_size * sizeof(GM_RSSP1_CFM_L2U_pri_struct));
		pCFM->L2U_pri_Q_offset  = (GM_RSSP1_Pointer)GM_RSSP1_malloc((size_t)len);

		if ((GM_RSSP1_Pointer)NULL == (GM_RSSP1_Pointer)pCFM->L2U_pri_Q_offset)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_CFM_MQ_Init_L2U_Memory_Fail, 0, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
		GM_RSSP1_memset((void*)pCFM->L2U_pri_Q_offset, 0U, (size_t)len);

		if (GM_RSSP1_FALSE == FSFB_Msg_Queue_Init(&(g_CFM_MQ.RCV_MQ), pcfg->CFM_cfg.L2U_pri_Q_size , pCFM->L2U_pri_Q_offset , FSFB_Q_TYPE_CFM_L2U_PRI))
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_CFM_MQ_Init_L2U_MQ_Fail, 2, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
	}
#endif
	/* start:CR:GM00002526, GM162, fulintai,2015-06-09, new MQ init for standby's all 0 message */
#ifdef	CFM_Stanby_Answer
	if (g_CFM_Debug_MQ.Q_size == 0U)
	{
		len = 2U * conn_num;
		Debug_Q_offset = (GM_RSSP1_Pointer)GM_RSSP1_malloc(len * sizeof(GM_RSSP1_CFM_U2L_pri_struct));

		if (NULL == pCFM->U2L_pri_Q_offset)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_CFM_MQ_Init_StbyAnsw_Memory_Fail, 0, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
		if (GM_RSSP1_FALSE == FSFB_Msg_Queue_Init(&g_CFM_Debug_MQ, len , Debug_Q_offset , FSFB_Q_TYPE_CFM_U2L_PRI))
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_CFM_MQ_Init_StbyAnsw_MQ_Fail, 0, 0, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
	}
#endif
	/* end */
	pCFM->connection_nums = (GM_RSSP1_UINT16)conn_num;
	pCFM->lib_state = GM_RSSP1_Lib_State_Operational;
	return GM_RSSP1_TRUE;
}

GM_RSSP1_BOOL GM_RSSP1_CFM_Init_Lib(GM_RSSP1_UINT16 conIndex, GM_RSSP1_comm_cfg_struct *pcfg, GM_RSSP1_CFM_object_struct *pCFM)
{
	GM_RSSP1_INT32 chn_index = 0;
	GM_RSSP1_INT32 conn_num = 0;
	GM_RSSP1_CFM_connection_struct *pCFM_con = NULL;
	GM_RSSP1_CFM_connection_cfg_struct *pConn_cfg = NULL;

	if ((NULL == pcfg) || (NULL == pCFM))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Init_Lib_Param_Point_Error, (int)pcfg, (int)pCFM, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	conn_num = (GM_RSSP1_INT32)pCFM->connection_nums + 1;
	if (conn_num>(GM_RSSP1_INT32)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
	{
		return GM_RSSP1_FALSE;
	}

	/* start:CR:GM00002526, GM162, fulintai,2015-06-09, add configuration for all 0 message size in standby's CFM*/
	pCFM->all0_size       = pcfg->CFM_cfg.all0_size;
	/* end:CR:GM00002526 */
	/* init connections */
#ifndef SFM_Part
	conIndex = GM_RSSP1_InitSeqInsert(OffLine_Object);
#endif

	if (0xFFFF != conIndex)
	{
		g_loc_info.connection_num += 1U;

		pConn_cfg = &(pcfg->CFM_cfg.connection_cfg);
		pCFM_con = &(pCFM->connection[conIndex]);

		pCFM_con->index = conIndex;
		pCFM_con->state = GM_RSSP1_Layer_State_Free;
		pCFM_con->b_enable_CRSCD_pack = pConn_cfg->b_enable_CRSCD_pack;
		pCFM_con->SaCEPID.source_addr = pcfg->CFM_cfg.source_addr;
		pCFM_con->SaCEPID.dest_addr = pConn_cfg->dest_addr;
		pCFM_con->com_type = pConn_cfg->com_type;
		pCFM_con->chn_num = pConn_cfg->chn_num;
		pCFM_con->recv_Q_size = pConn_cfg->recv_Q_size;

		g_loc_info.connection[conIndex].chn_num = pConn_cfg->chn_num;

		for (chn_index=0; chn_index<(GM_RSSP1_INT32)pCFM_con->chn_num; ++chn_index)
		{
			pCFM_con->com_chn[chn_index].loc_ip = pConn_cfg->UDP_chn_cfg[chn_index].loc_ip;
			pCFM_con->com_chn[chn_index].rem_ip = pConn_cfg->UDP_chn_cfg[chn_index].rem_ip;
			pCFM_con->com_chn[chn_index].loc_port = pConn_cfg->UDP_chn_cfg[chn_index].loc_port;
			pCFM_con->com_chn[chn_index].rem_port = pConn_cfg->UDP_chn_cfg[chn_index].rem_port;
			pCFM_con->com_chn[chn_index].b_connected = GM_RSSP1_FALSE;

			g_loc_info.connection[conIndex].locl_addr[chn_index].loc_ip = pConn_cfg->UDP_chn_cfg[chn_index].loc_ip;
			g_loc_info.connection[conIndex].locl_addr[chn_index].loc_port = pConn_cfg->UDP_chn_cfg[chn_index].loc_port;

			GM_RSSP1_Hash_Fill_CFM_Index(pCFM_con->com_chn[chn_index].rem_ip, pCFM_con->com_chn[chn_index].rem_port, pCFM_con->index, (GM_RSSP1_UINT8)chn_index);
		}
	}

	return GM_RSSP1_TRUE;
}

GM_RSSP1_BOOL GM_RSSP1_CFM_Delete(GM_RSSP1_UINT16 con_index, GM_RSSP1_CFM_object_struct *pCFM)
{
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;

	if ((NULL == pCFM))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Delete_Param_Point_Error, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	if ((con_index < GM_RSSP1_MAX_SAFETY_CONNECTION_NUM) && (pCFM->connection_nums > 0U))
	{
		GM_RSSP1_memset((void *)(&pCFM->connection[con_index]), (GM_RSSP1_UINT8)0U, sizeof(GM_RSSP1_CFM_connection_struct));
		pCFM->connection_nums --;
		bRt = GM_RSSP1_TRUE;
	}

	return bRt;
}
#endif
