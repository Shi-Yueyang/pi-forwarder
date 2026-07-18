#include "GM_RSSP1_SFM.h"
#include "GM_RSSP1_PROC_Int.h"
#include "GM_RSSP1_Msg_Define.h"
#include "GM_RSSP1_Syn.h"
#include "GM_RSSP1_MQ_Interface.h"

#ifdef SFM_Part
extern GM_RSSP1_VSN_GET_CALLBACK_FUN GM_RSSP1_VSN_Get_Callback_Fun;	/*added by huang 201511219*/

static GM_RSSP1_SFM_object_struct g_SFM_obj;

extern RSSP1_MQ_Inter_struct g_SFM_MQ;

extern RSSP1_MQ_Inter_struct g_CFM_MQ;

extern GM_RSSP1_UINT8* g_pCrcmAddr;

#ifdef GM_RSSP1_SYSCKW_FROM_USER
GM_RSSP1_UINT32 App_SYSCKW[2U] = {0U};
#endif

extern GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Is_Local_ACTIVE(void);


GM_RSSP1_BOOL SYS_Is_Local_Sys_Active(void)
{
	/* TODO:增加接口 */
	if(GM_RSSP1_TRUE == GM_RSSP1_APP_Interface_Is_Local_ACTIVE())
	{
		return GM_RSSP1_TRUE;
	}
	else
	{
		return GM_RSSP1_FALSE;
	}
}

GM_RSSP1_SFM_object_struct *GM_RSSP1_Get_SFM_Object(void)
{
	return &g_SFM_obj;
}

GM_RSSP1_BOOL GM_RSSP1_SFM_Ini_FSFB_Offline_Const(GM_RSSP1_SFM_connection_struct *pSFM_conn , GM_RSSP1_SFM_object_struct *pSFM)
{
	GM_RSSP1_INT32 rt = GM_RSSP1_ERROR;
	GM_RSSP1_INT32 chn_index = 0;

	if ((NULL == pSFM_conn) || (NULL == pSFM))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Ini_FSFB_Offline_Const_Param_Point_Error, (int)pSFM_conn, (int)pSFM, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	for (chn_index = 0 ; chn_index < (GM_RSSP1_INT32)GM_RSSP1_CHECK_CHN_NUM ; ++chn_index)
	{
		rt = GM_RSSP1_PREC_SINT_CAL_Int(
			pSFM_conn->fsfb_chn[chn_index].chn_cfg.remote_sinit ,
			pSFM_conn->fsfb_chn[chn_index].chn_cfg.remote_sid ,
			pSFM_conn->fsfb_chn[chn_index].PREC_SINIT ,
			g_Polyomia[chn_index].pLFSR_left_table,
			g_Polyomia[chn_index].pLFSR_right_table,
			(GM_RSSP1_INT32)pSFM_conn->deltaTime
			);

		if (GM_RSSP1_ERROR == rt)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Ini_FSFB_Offline_Const_SINT_Error, (int)pSFM_conn->index, (int)chn_index, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}

		rt = GM_RSSP1_PREC_FIRSTSINIT_CAL(
			pSFM_conn->fsfb_chn[chn_index].chn_cfg.remote_sinit,
			pSFM_conn->fsfb_chn[chn_index].chn_cfg.local_sid,
			pSFM_conn->fsfb_chn[chn_index].chn_cfg.remote_dataVer,
			&(pSFM_conn->fsfb_chn[chn_index].PREC_FIRSTSINIT),
			g_Polyomia[chn_index].pLFSR_left_table,
			g_Polyomia[chn_index].pLFSR_right_table
			);

		if (GM_RSSP1_ERROR == rt)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Ini_FSFB_Offline_Const_FIRSTSINIT_Error, (int)pSFM_conn->index, (int)chn_index, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}

		rt = GM_RSSP1_POST_RXDATA_Int(

			pSFM_conn->fsfb_chn[chn_index].chn_cfg.remote_sinit,
			pSFM_conn->fsfb_chn[chn_index].chn_cfg.remote_sid,
			pSFM_conn->fsfb_chn[chn_index].POST_RXDATA,
			g_Polyomia[chn_index].pLFSR_left_table,
			g_Polyomia[chn_index].pLFSR_right_table,
			(GM_RSSP1_INT32)pSFM_conn->deltaTime
			);

		if (GM_RSSP1_ERROR == rt)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Ini_FSFB_Offline_Const_RXDATA_Error, (int)pSFM_conn->index, (int)chn_index, 0, 0, 0, 0);
			return GM_RSSP1_FALSE;
		}
	}

	return GM_RSSP1_TRUE;
}

void GM_RSSP1_SFM_Update_Time(void)
{
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_UINT16 chn_index = 0U;
	GM_RSSP1_BOOL b_UpdateTC = GM_RSSP1_FALSE;
	GM_RSSP1_UINT32 i =0U;	/*注意i的位数，循环TC的下标必须具备和TC一样的32位*/
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_connection_struct *pSFM_con = NULL;
	GM_RSSP1_UINT16 indexArray[8U];
	GM_RSSP1_UINT8 ConNum = 8U;
	GM_RSSP1_UINT16 search_index = 0U;
	GM_RSSP1_UINT16 j = 0U;
	GM_RSSP1_TIME l_time;
	GM_RSSP1_SFM_connection_struct *pSFM_conn_x = NULL;

	if (GM_RSSP1_Lib_State_Operational != g_SFM_obj.lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Update_Time_Param_Result_No_Match, (int)g_SFM_obj.lib_state, 0, 0, 0, 0, 0);
		return;
	}

	/* update ENV TC,TS */
	GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 1U);/**重置正序查询索引**/
	for (l_index = 0U ; l_index < g_SFM_obj.connection_nums ; ++l_index)/**按静态配置链表顺序,获取index**/
	{
		ConNum = 8U; /*flt CR5738*/
		con_index = GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 0U);
		/*if (0xFFFFU != con_index)*/
		if(con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
		{
			pSFM_con = &(g_SFM_obj.connection[con_index]);
#ifdef GM_RSSP1_UpdateTC_Assign
			b_UpdateTC = GM_RSSP1_UpdateTC_Assign((GM_RSSP1_UINT16)pSFM_con->TcycleLoc);/*CR5649 通过回调，返回不同周期对应的待更新TC*/

			GM_RSSP1_memcpy((void*)&l_time, (const void*)&(pSFM_con->time), sizeof(GM_RSSP1_TIME));

		
			if((GM_RSSP1_TRUE == b_UpdateTC) && (pSFM_con->env[0U].TC == pSFM_con->time.TC))
			{
				GM_RSSP1_TIME_STEP(&l_time);

				/*更新相同dest_addr冗余连接的time*/
				bRt = GM_RSSP1_Hash_CnfDst_ResArray((GM_RSSP1_UINT32)pSFM_con->dest_addr, indexArray, &ConNum);
				if ((GM_RSSP1_TRUE == bRt) && (ConNum > 0U)){
					for (j=0U; j<(GM_RSSP1_UINT16)(ConNum/2U); ++j)
					{
						search_index = indexArray[j*2U];
						pSFM_conn_x = &(g_SFM_obj.connection[search_index]);
						GM_RSSP1_memcpy((void*)&(pSFM_conn_x->time), (const void*)&l_time, sizeof(GM_RSSP1_TIME));
					}
				}
			}

			/*若本周期更新了time，则更新相应的数据*/
			if (pSFM_con->env[0U].TC != pSFM_con->time.TC)
			{
				for (chn_index = 0U ; chn_index < GM_RSSP1_CHECK_CHN_NUM ; ++chn_index)
				{
					pSFM_con->env[chn_index].TC     = pSFM_con->time.TC;
					pSFM_con->env[chn_index].TS     = pSFM_con->time.TS[chn_index];
				}
				pSFM_con->b_rcvd_new_dat_in_cycle = GM_RSSP1_FALSE;	/**重置发送RSD标识符,每周期只能发一包**/
				pSFM_con->b_RSD_Already_Sent = GM_RSSP1_FALSE;       /* RVS 10922253 */
			}

#else
			GM_RSSP1_TIME_STEP(&(pSFM_con->time));
			for (chn_index = 0U; chn_index < GM_RSSP1_CHECK_CHN_NUM ; ++chn_index)
			{
				pSFM_con->env[chn_index].TC     = pSFM_con->time.TC;
				pSFM_con->env[chn_index].TS     = pSFM_con->time.TS[chn_index];
			}
			pSFM_con->b_rcvd_new_dat_in_cycle = GM_RSSP1_FALSE;	/**重置发送RSD标识符,每周期只能发一包**/
			pSFM_con->b_RSD_Already_Sent = GM_RSSP1_FALSE;        /* RVS 10922253 */
#endif
		}
	}

	return;
}

GM_RSSP1_library_state_enum GM_RSSP1_SFM_Get_Library_Status(GM_RSSP1_SFM_object_struct *pSFM)
{
	if (NULL == pSFM)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Get_Library_Status_Point_Error, (int)pSFM, 0, 0, 0, 0, 0);
		return GM_RSSP1_Lib_State_Failure;
	}

	return pSFM->lib_state;
}

GM_RSSP1_SFM_connection_struct *GM_RSSP1_SFM_Get_Connection_BY_SaCEPID(GM_RSSP1_SACEPID_struct *pSaCEPID , GM_RSSP1_SFM_object_struct *pSFM)
{
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_TRUE;
	GM_RSSP1_SFM_connection_struct *pSFM_conn = NULL;

	if ((NULL == pSFM) || (NULL == pSaCEPID))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Get_Connection_BY_SaCEPID_Point_Error, (int)pSFM, 0, 0, 0, 0, 0);
		return NULL;
	}

	if (GM_RSSP1_Lib_State_Operational != pSFM->lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Get_Connection_BY_SaCEPID_Result_No_Match, (int)pSFM->lib_state, 0, 0, 0, 0, 0);
		return NULL;
	}

	GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 1U);/**重置正序查询索引**/
	for (l_index = 0U ; l_index < pSFM->connection_nums ; ++l_index)/**按静态配置链表顺序,获取index**/
	{
		con_index = GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 0U);
		/*if (0xFFFFU != con_index)*/
		if(con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
		{
			pSFM_conn = &(pSFM->connection[con_index]);

			if (GM_RSSP1_TRUE == GM_RSSP1_SFM_Is_SaCEPID_Equal(pSaCEPID , &(pSFM_conn->SaCEPID)))
			{
				return pSFM_conn;
			}
		}
	}

	return NULL;
}

GM_RSSP1_BOOL GM_RSSP1_SFM_Is_SaCEPID_Equal(GM_RSSP1_SACEPID_struct *pSaCEPID1 , GM_RSSP1_SACEPID_struct *pSaCEPID2)
{
	if ((NULL == pSaCEPID1) || (NULL == pSaCEPID2))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Is_SaCEPID_Equal_Point_Error, (int)pSaCEPID1, (int)pSaCEPID2, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	if ((pSaCEPID1->source_addr == pSaCEPID2->source_addr) && (pSaCEPID1->dest_addr == pSaCEPID2->dest_addr))
	{
		return GM_RSSP1_TRUE;
	}
	else
	{
		return GM_RSSP1_FALSE;
	}
}


GM_RSSP1_User_Put_Pri_To_SFM_Result_enum GM_RSSP1_User_Put_Pri_To_SFM(GM_RSSP1_SFM_U2L_pri_struct *pU2L , GM_RSSP1_SFM_object_struct *pSFM)
{
	GM_RSSP1_Write_Q_Return_Enum write_rt = Q_WRITE_FAIL;

	if ((NULL == pSFM) || (pSFM->lib_state != GM_RSSP1_Lib_State_Operational))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_User_Put_Pri_To_SFM_Result_No_Match, (int)pSFM, 0, 0, 0, 0, 0);
		return GM_RSSP1_User_Put_Pri_LIB_NOT_OPERATIONAL;
	}

	if (NULL == pU2L)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_User_Put_Pri_To_SFM_Point_Error, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_User_Put_Pri_INVALID_U2L;
	}

	if (GM_RSSP1_Pri_Type_SFM_U2L_Unknow == pU2L->type)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_User_Put_Pri_To_SFM_Type_Unknow, (int)pU2L->type, 0, 0, 0, 0, 0);
		return GM_RSSP1_User_Put_Pri_INVALID_REQ_TYPE;
	}

	write_rt = FSFB_Msg_Queue_Write(&(g_SFM_MQ.SND_MQ) , pU2L);

	if (Q_WRITE_FAIL == write_rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_User_Put_Pri_To_SFM_WRITE_FAIL, 0, 0, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"UsrData Put To SFM Fail!\n ", 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_User_Put_Pri_LIB_NOT_OPERATIONAL;
	}
	else if (Q_WRITE_OK_FULL == write_rt)
	{
		/* 成功，但队列已满 */
		GM_RSSP1_Log_Usr(GM_RSSP1_User_Put_Pri_To_SFM_WRITE_OK_FULL, 0, 0, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"UsrData Put To SFM FULL!\n ", 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_User_Put_Pri_MQ_FULL;
	}
	else
	{
		/* 成功 */
		return GM_RSSP1_User_Put_Pri_OK;
	}
}

GM_RSSP1_User_Get_Pri_From_SFM_Result_enum GM_RSSP1_User_Get_Pri_From_SFM(GM_RSSP1_SFM_L2U_pri_struct *pL2U , GM_RSSP1_SFM_object_struct *pSFM)
{
	if (NULL == pL2U)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_User_Get_Pri_From_SFM_Point_Error, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_User_Get_Pri_INVALID_L2U;
	}

	if ((NULL == pSFM) || (GM_RSSP1_Lib_State_Operational != pSFM->lib_state))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_User_Get_Pri_From_SFM_Result_No_Match, (int)pSFM, 0, 0, 0, 0, 0);
		return GM_RSSP1_User_Get_Pri_LIB_NOT_OPERATIONAL;
	}

	if (GM_RSSP1_TRUE == FSFB_Msg_Queue_Get(&(g_SFM_MQ.RCV_MQ/*pSFM->L2U_pri_Q*/), pL2U))
	{
		return GM_RSSP1_User_Get_Pri_OK;
	}
	else
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_User_Get_Pri_From_SFM_Get_Pri_Fail, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_User_Get_Pri_LIB_NOT_OPERATIONAL;
	}
}

GM_RSSP1_BOOL GM_RSSP1_User_Send_Dat(GM_RSSP1_SACEPID_struct *pSaCEPID , GM_RSSP1_UINT8 *pDat , GM_RSSP1_UINT32 dat_len , Local_Sys_AB_enum sysAORB , GM_RSSP1_BOOL bSysActive, GM_RSSP1_SFM_object_struct *pSFM)
{
	GM_RSSP1_SFM_U2L_pri_struct pri;
	GM_RSSP1_User_Put_Pri_To_SFM_Result_enum put_pri_rt;

	if ((NULL == pSaCEPID) || (NULL == pDat) || (NULL == pSFM))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_User_Send_Dat_Point_Error, (int)pSaCEPID, (int)pDat, (int)pSFM, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_Lib_State_Operational != pSFM->lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_User_Send_Dat_Result_No_Match, (int)pSFM->lib_state, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	if ((dat_len > GM_RSSP1_MAX_SFM_DAT_LEN) || (dat_len <= 0U))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_User_Send_Dat_Data_Error, (int)dat_len, (int)GM_RSSP1_MAX_USER_DAT_LEN, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"SFM User dat dat_len=%d exceed GM_RSSP1_MAX_USER_DAT_LEN,or dat src is null.\n", (int)dat_len, (int)GM_RSSP1_MAX_USER_DAT_LEN, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	pri.type = GM_RSSP1_Pri_Type_SFM_Data_req;

	pri.SaCEPID = *(pSaCEPID);

	if (GM_RSSP1_Local_Sys_A == sysAORB)
	{
		pri.data_req.sys_A_or_B = RSSP1_RSD_FRAME_TYPE_FROM_A;
	}
	else if (GM_RSSP1_Local_Sys_B == sysAORB)
	{
		pri.data_req.sys_A_or_B = RSSP1_RSD_FRAME_TYPE_FROM_B;
	}
	else
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_User_Send_Dat_SYS_Unknow, (int)sysAORB, 0, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"Synstem AorB:%d error!.\n", (int)sysAORB, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_TRUE == bSysActive)
	{
		pri.data_req.active_or_standby = RSSP1_RSD_PROTOCOL_TYPE_ACTIVE;
	}
	else
	{
		pri.data_req.active_or_standby = RSSP1_RSD_PROTOCOL_TYPE_STANDBY;
	}

	pri.data_req.bytes_count = (GM_RSSP1_UINT16)dat_len;

	GM_RSSP1_memcpy((void*)pri.data_req.byte , (const void*)pDat , (size_t)dat_len);

#ifdef GM_RSSP1_SAVING_MODE
	
	GM_RSSP1_SFM_Process_TxData(&pri);
	return GM_RSSP1_TRUE;
#else

	/* 防护SFM的U2L队列，防止proc_send的通过while(1)从队列取数据的操作被打断，又放入新数据，在内存刷新时被刷掉 */
	put_pri_rt = GM_RSSP1_User_Put_Pri_To_SFM(&pri, pSFM);

	if ((GM_RSSP1_User_Put_Pri_OK == put_pri_rt) || (GM_RSSP1_User_Put_Pri_MQ_FULL == put_pri_rt))
	{
		return GM_RSSP1_TRUE;
	}
	else
	{
		return GM_RSSP1_FALSE;
	}


#endif
}

GM_RSSP1_BOOL GM_RSSP1_SFM_Send_RSD(GM_RSSP1_SFM_U2L_pri_struct *pDat_pri , GM_RSSP1_SFM_connection_struct *pSFM_conn)
{
	GM_RSSP1_CFM_U2L_pri_struct cfm_pri; /* RSD包最终以U2L_pri的形式传递至CFM */
	GM_RSSP1_SFM_Put_Pri_To_CFM_Result_enum  put_pri_rt = GM_RSSP1_SFM_Put_Pri_LIB_NOT_OPERATIONAL;
	GM_RSSP1_INT32 rt = GM_RSSP1_ERROR;
	GM_RSSP1_UINT32 crc32[GM_RSSP1_CHECK_CHN_NUM];
	GM_RSSP1_UINT32 crcm[GM_RSSP1_CHECK_CHN_NUM];
	GM_RSSP1_UINT16 priLen = 0U;
	GM_RSSP1_INT32 chn_index = 0;
	GM_RSSP1_INT32 chnnum = 2;
	GM_RSSP1_BOOL ret = GM_RSSP1_FALSE;
	GM_RSSP1_UINT8* CrcmRes = g_pCrcmAddr;

#ifdef GM_RSSP1_SYSCKW_FROM_USER
	GM_RSSP1_UINT32 vsn0 =0U;
	GM_RSSP1_UINT32 vsn[2U] ={0U};
#endif

	if ((NULL == pDat_pri) || (NULL == pSFM_conn))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Send_RSD_Point_Error, (int)pDat_pri, (int)pSFM_conn, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_Lib_State_Operational != g_SFM_obj.lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Send_RSD_Result_No_Match, (int)g_SFM_obj.lib_state, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	GM_RSSP1_memset((void*)&cfm_pri, 0U, sizeof(GM_RSSP1_CFM_U2L_pri_struct));


	cfm_pri.type = GM_RSSP1_CFM_Data_req;

	cfm_pri.index = pSFM_conn->index;

	cfm_pri.data_req.type = GM_RSSP1_MSG_TYPE_RSD;
	cfm_pri.data_req.TC = pSFM_conn->env[0U].TC;
	cfm_pri.data_req.bytes_count = 0U;
	priLen = pDat_pri->data_req.bytes_count;
#ifdef InterfaceData_With_CRC
	pDat_pri->data_req.bytes_count = pDat_pri->data_req.bytes_count - sizeof(GM_RSSP1_UINT32)*2;/**减去用户数据最末尾的两个CRC32的长度才是实际数据长度**/
#endif


#if defined(GM_RSSP1_ENABLE_CH1) && defined(GM_RSSP1_ENABLE_CH2)
	chnnum = 2;
#elif defined(GM_RSSP1_ENABLE_CH1)
	chnnum = 1;
#else
	chn_index = 1;
#endif

	for (chn_index = 0 ; chn_index < chnnum ; ++chn_index)
	{
#ifdef InterfaceData_With_CRC
		crc32[chn_index] = *(GM_RSSP1_UINT32*)(pDat_pri->data_req.byte + pDat_pri->data_req.bytes_count + (chn_index * sizeof(GM_RSSP1_UINT32)));/**使用随数据传下来的CRC32**/
#else
		GM_RSSP1_CRC_Calculate_CRC32(pDat_pri->data_req.byte, (GM_RSSP1_UINT32)pDat_pri->data_req.bytes_count, &(crc32[chn_index]), g_Polyomia[chn_index].pCRC32_table);
#endif

#ifdef GM_RSSP1_SYSCKW_FROM_USER
		GM_RSSP1_VSN_Get_Callback_Fun(&vsn0,&vsn[0U],&vsn[1U]);
		crcm[chn_index] = crc32[chn_index] ^ App_SYSCKW[chn_index] ^ (pSFM_conn->fsfb_chn[chn_index].chn_cfg.local_sid) ^ (pSFM_conn->env[chn_index].TS) ^vsn[chn_index];
#else
		crcm[chn_index] = crc32[chn_index] ^(~(pSFM_conn->fsfb_chn[chn_index].chn_cfg.local_sys_chk)) ^(pSFM_conn->fsfb_chn[chn_index].chn_cfg.local_sid) ^(pSFM_conn->env[chn_index].TS) ;
#endif
	}
#ifdef GM_RSSP1_ENABLE_CH1

	GM_RSSP1_Get_CRCM(&crcm[1u]);


	rt  = GM_RSSP1_RSD_BUILD_Int(
		cfm_pri.data_req.byte,
		&(cfm_pri.data_req.bytes_count) ,
		pDat_pri->data_req.byte,
		pDat_pri->data_req.bytes_count ,
		(GM_RSSP1_UINT8)(pDat_pri->data_req.active_or_standby),
		(GM_RSSP1_UINT8)(pDat_pri->data_req.sys_A_or_B),
		pDat_pri->SaCEPID.source_addr ,   /* 地址使用VLE应用层传下的信息，防止匹配错误 jianghongjun 20110825 safety enhancement */
		pDat_pri->SaCEPID.dest_addr ,
		pSFM_conn->env[0U].TC ,  /* env双通道中TC值始终相同，默认取第一个即可 */
		crcm[0U],
		crcm[1U],
		g_Polyomia[0U].pCRC16_table,
		CrcmRes
		); /* CR: 11036019*/


	if (GM_RSSP1_ERROR == rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Send_RSD_Build_Error, (int)cfm_pri.data_req.bytes_count, (int)GM_RSSP1_MAX_SAFETY_DAT_LEN, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"GM_RSSP1_RSD_BUILD  fail! Inx %d, DstAddr %x Size: %d - %d\n",(int)pSFM_conn->index, (int)pSFM_conn->dest_addr,(int)cfm_pri.data_req.bytes_count,(int)GM_RSSP1_MAX_SAFETY_DAT_LEN,0,0);
		return GM_RSSP1_FALSE;
	}

#ifdef OFFLINE_TOOL_SUPPORT
	pDat_pri->data_req.bytes_count = pDat_pri->data_req.bytes_count + sizeof(GM_RSSP1_UINT32)*2;/*Twin CRC32 长度*/
#endif

#if defined(GM_RSSP1_SAVING_MODE) && !defined(GM_RSSP1_ENABLE_CRSCD_PACK)
	ret = GM_RSSP1_CFM_Send_Dat(cfm_pri.data_req.byte , cfm_pri.data_req.bytes_count, cfm_pri.index);
#else
	/* 将数据包交给CFM模块发送 */
	put_pri_rt = GM_RSSP1_SFM_Put_Pri_To_CFM(&cfm_pri);

	pDat_pri->data_req.bytes_count = priLen;/**配置中存在相同ID的远端设备,因此该pDat_pri可能会被重复使用,需将长度信息还原**/
	if ((GM_RSSP1_SFM_Put_Pri_OK == put_pri_rt) || (GM_RSSP1_SFM_Put_Pri_MQ_FULL == put_pri_rt))
	{
		
		GM_RSSP1_Log_Msg(4,"SFM ID:0x%x --------> send RSD.%d bytes.\n",(int)pSFM_conn->SaCEPID.dest_addr, (int)cfm_pri.data_req.bytes_count,0,0,0,0);
		/*FSFB_Output_Data_As_HexString(cfm_pri.as.data_req.byte , cfm_pri.as.data_req.bytes_count);
		*/
		return GM_RSSP1_TRUE;
	}
	else
	{
		return GM_RSSP1_FALSE;
	}
#endif
#else
	GM_RSSP1_Support_CRCM(crcm[1]);
	return GM_RSSP1_TRUE;
#endif
}

GM_RSSP1_BOOL GM_RSSP1_SFM_Send_SSE(GM_RSSP1_SFM_connection_struct *pSFM_conn)
{
	GM_RSSP1_CFM_U2L_pri_struct cfm_pri;                    /* SSE包最终以U2L_pri的形式传递至CFM */
	GM_RSSP1_SFM_Put_Pri_To_CFM_Result_enum  put_pri_rt = GM_RSSP1_SFM_Put_Pri_LIB_NOT_OPERATIONAL;
	GM_RSSP1_INT32 rt = GM_RSSP1_ERROR;
	GM_RSSP1_BOOL ret = GM_RSSP1_FALSE;

	if (NULL == pSFM_conn)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Send_SSE_Point_Error, (int)pSFM_conn, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_Lib_State_Operational != g_SFM_obj.lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Send_SSE_Result_No_Match, (int)g_SFM_obj.lib_state, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	GM_RSSP1_memset((void*)&cfm_pri, 0U, sizeof(GM_RSSP1_CFM_U2L_pri_struct));

	cfm_pri.type = GM_RSSP1_CFM_Data_req;
	cfm_pri.index = pSFM_conn->index;
	cfm_pri.data_req.type = GM_RSSP1_MSG_TYPE_SSE;
	cfm_pri.data_req.TC = pSFM_conn->env[0].TC;

	rt  = GM_RSSP1_SSE_BUILD_Int(
		cfm_pri.data_req.byte ,
		&(cfm_pri.data_req.bytes_count) ,
		pSFM_conn->source_addr ,
		pSFM_conn->dest_addr ,
		pSFM_conn->env[0U].TC ,  /* env双通道中TC值始终相同，默认取第一个即可 */
		pSFM_conn->fsfb_chn[0U].chn_cfg.local_sid , pSFM_conn->env[0U].TS,
		pSFM_conn->fsfb_chn[1U].chn_cfg.local_sid , pSFM_conn->env[1U].TS,
		&(pSFM_conn->env[0U].sseTC) , &(pSFM_conn->env[1U].sseTC),
		&(pSFM_conn->env[0U].sseTS) , &(pSFM_conn->env[1U].sseTS),
		&(pSFM_conn->env[0U].bSendSSE) , &(pSFM_conn->env[1U].bSendSSE) ,
		pSFM_conn->lifeTime ,
		g_Polyomia[0U].pCRC16_table
		);

	if (GM_RSSP1_ERROR == rt)
	{
		/*
		GM_RSSP1_SFM_Report_Warning_To_User(pSFM_conn , pSFM , GM_RSSP1_SFM_WARNING_TYPE_SSE_BUILD_ERR);
		
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Send_SSE_Build_Error, pSFM_conn->index, 0, 0, 0, 0, 0);*/
		GM_RSSP1_Log_Msg(2,"GM_RSSP1_SSE_BUILD  fail! Inx %d, DstAddr %x Size: %d - %d\n",(int)pSFM_conn->index, (int)pSFM_conn->dest_addr,(int)cfm_pri.data_req.bytes_count,(int)GM_RSSP1_MAX_SAFETY_DAT_LEN,0,0);

		return GM_RSSP1_FALSE;
	}

	GM_RSSP1_SetPreciTime(pSFM_conn);/*CR GM6950  清除历史信息*/
#if defined(GM_RSSP1_SAVING_MODE) && !defined(GM_RSSP1_ENABLE_CRSCD_PACK)
	ret = GM_RSSP1_CFM_Send_Dat(cfm_pri.data_req.byte , cfm_pri.data_req.bytes_count, cfm_pri.index);
#else
	/* 将数据包交给CFM模块发送 */
	put_pri_rt = GM_RSSP1_SFM_Put_Pri_To_CFM(&cfm_pri);

	if ((GM_RSSP1_SFM_Put_Pri_OK == put_pri_rt) || (GM_RSSP1_SFM_Put_Pri_MQ_FULL == put_pri_rt))
	{
		GM_RSSP1_Log_Msg(4,"SFM ID:0x%X ----> send SSE.%d bytes.(%d.%d),index:%d.\n", (int)pSFM_conn->SaCEPID.dest_addr , (int)cfm_pri.data_req.bytes_count, (int)(pSFM_conn->env[0U].local_TC), (int)(pSFM_conn->env[0U].remote_TC), (int)pSFM_conn->index, 0);
		return GM_RSSP1_TRUE;
	}
	else
	{
		return GM_RSSP1_FALSE;
	}
#endif
}

GM_RSSP1_BOOL GM_RSSP1_SFM_Send_SSR(GM_RSSP1_UINT32 rcvd_SEQENQ_1 , GM_RSSP1_UINT32 rcvd_SEQENQ_2 , GM_RSSP1_UINT32 rcvd_TC , GM_RSSP1_SFM_connection_struct *pSFM_conn)
{
	GM_RSSP1_CFM_U2L_pri_struct cfm_pri;                    /* RSD包最终以U2L_pri的形式传递至CFM */
	GM_RSSP1_SFM_Put_Pri_To_CFM_Result_enum  put_pri_rt = GM_RSSP1_SFM_Put_Pri_LIB_NOT_OPERATIONAL;
	GM_RSSP1_INT32 rt = GM_RSSP1_ERROR;
	GM_RSSP1_BOOL ret = GM_RSSP1_FALSE;

	if (NULL == pSFM_conn)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Send_SSR_Point_Error, (int)pSFM_conn, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_Lib_State_Operational != g_SFM_obj.lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Send_SSR_Result_No_Match, g_SFM_obj.lib_state, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	GM_RSSP1_memset((void*)&cfm_pri, 0U, sizeof(GM_RSSP1_CFM_U2L_pri_struct));

	cfm_pri.type = GM_RSSP1_CFM_Data_req;

	cfm_pri.index = pSFM_conn->index;

	cfm_pri.data_req.type = GM_RSSP1_MSG_TYPE_SSR;

	cfm_pri.data_req.TC = pSFM_conn->env[0U].TC;

	rt  = GM_RSSP1_SSR_BUILD_Int(
		cfm_pri.data_req.byte ,
		&(cfm_pri.data_req.bytes_count) ,
		pSFM_conn->source_addr ,
		pSFM_conn->dest_addr ,
		rcvd_SEQENQ_1 ,
		rcvd_SEQENQ_2 ,
		pSFM_conn->fsfb_chn[0U].chn_cfg.local_sid ,
		pSFM_conn->fsfb_chn[1U].chn_cfg.local_sid ,
		pSFM_conn->fsfb_chn[0U].chn_cfg.local_dataVer , /* 设计院。 @NOTE:注意!build SSR使用的是本地还是对方的dataver! */
		pSFM_conn->fsfb_chn[1U].chn_cfg.local_dataVer ,
		/*
		pSFM_conn->fsfb_chn[0].chn_cfg.remote_dataVer , / * @NOTE:注意!build SSR使用的是本地还是对方的dataver! * /
		pSFM_conn->fsfb_chn[1].chn_cfg.remote_dataVer ,
		*/
		pSFM_conn->env[0U].TS,
		pSFM_conn->env[1U].TS,
		rcvd_TC,
		pSFM_conn->env[0U].TC ,
		0x01U,/* RSSP1:预留固定值*/
		g_Polyomia[0U].pCRC16_table
		);

	if (GM_RSSP1_ERROR == rt)
	{
		GM_RSSP1_Log_Msg(2,"GM_RSSP1_SSR_BUILD  fail! Inx %d, DstAddr %x Size: %d - %d\n",(int)pSFM_conn->index, (int)pSFM_conn->dest_addr,(int)cfm_pri.data_req.bytes_count,(int)GM_RSSP1_MAX_SAFETY_DAT_LEN,0,0);
		/*GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Send_SSR_Build_Error, 0, 0, 0, 0, 0, 0);*/
		return GM_RSSP1_FALSE;
	}
#if defined(GM_RSSP1_SAVING_MODE) && !defined(GM_RSSP1_ENABLE_CRSCD_PACK)
	ret = GM_RSSP1_CFM_Send_Dat(cfm_pri.data_req.byte , cfm_pri.data_req.bytes_count, cfm_pri.index);
#else
	/* 将数据包交给CFM模块发送 */
	put_pri_rt = GM_RSSP1_SFM_Put_Pri_To_CFM(&cfm_pri);

	if ((GM_RSSP1_SFM_Put_Pri_OK == put_pri_rt) || (GM_RSSP1_SFM_Put_Pri_MQ_FULL == put_pri_rt))
	{
		GM_RSSP1_Log_Msg(4,"Inx:%d SFM ID:0x%X ------> send SSR.%d bytes.(%d.%d).\n", (int)pSFM_conn->index, (int)pSFM_conn->SaCEPID.dest_addr , (int)cfm_pri.data_req.bytes_count, (int)(pSFM_conn->env[0U].local_TC), (int)(pSFM_conn->env[0U].remote_TC), 0);
		pSFM_conn->b_RSD_Already_Sent = GM_RSSP1_TRUE;/**在发送SSR的周期,停发本周期内的RSD**/
		return GM_RSSP1_TRUE;
	}
	else
	{
		return GM_RSSP1_FALSE;
	}
#endif
}

void GM_RSSP1_SFM_Proc_Rcvd_SSE(GM_RSSP1_SFM_connection_struct *pSFM_conn , GM_RSSP1_UINT8 pDatByte[], GM_RSSP1_UINT16 ByteCount)
{
	GM_RSSP1_UINT16 crc16 = 0U;
	GM_RSSP1_UINT16 rcvd_crc16 = 0U;
	GM_RSSP1_UINT32 rcvd_TC = 0U;
	GM_RSSP1_UINT32 SEQENQ1 = 0U;
	GM_RSSP1_UINT32 SEQENQ2 = 0U;
	GM_RSSP1_UINT16 rcvd_src_addr = 0U;
	GM_RSSP1_UINT16 rcvd_dest_addr = 0U;
	GM_RSSP1_UINT16 con_index = 0xFFFFU;
	GM_RSSP1_UINT16 ValidIndex = 0xFFFFU;
	GM_RSSP1_UINT16 FreeIndex = 0xFFFFU;
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;

	if ((NULL == pSFM_conn) || (NULL == pDatByte))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_SSE_Point_Error, (int)pSFM_conn, (int)pDatByte, 0, 0, 0, 0);
		return;
	}

	if (GM_RSSP1_Lib_State_Operational != g_SFM_obj.lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_SSE_Result_No_Match, (int)g_SFM_obj.lib_state, 0, 0, 0, 0, 0);
		return;
	}

	if (ByteCount != RSSP1_SSE_FRAME_LEN_WITH_CRC)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_SSE_Byte_Loss, (int)pSFM_conn->SaCEPID.dest_addr , (int)ByteCount , (int)RSSP1_SSE_FRAME_LEN_WITH_CRC, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"SFM ID: 0x%X SSE rcvd frame length err! rcvd:%d , RSSP1_SSE_FRAME_LEN_WITH_CRC:%d.\n", (int)pSFM_conn->SaCEPID.dest_addr , (int)ByteCount, (int)RSSP1_SSE_FRAME_LEN_WITH_CRC, 0, 0, 0);
		return;
	}

	/* 记录收到SSE事件，证明对方校验失败
	GM_RSSP1_SFM_Report_Warning_To_User(pSFM_conn , pSFM , GM_RSSP1_SFM_WARNING_TYPE_RECV_SSE); */

	rcvd_src_addr = (((GM_RSSP1_UINT16)(pDatByte[3U]) << 8U) + (GM_RSSP1_UINT16)(pDatByte[2U]));
	rcvd_dest_addr = (((GM_RSSP1_UINT16)(pDatByte[5U]) << 8U) + (GM_RSSP1_UINT16)(pDatByte[4U]));

	if ((rcvd_src_addr != pSFM_conn->dest_addr) || (rcvd_dest_addr != pSFM_conn->source_addr))
	{
		/* 收到来源/目的 */
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_SSE_Addr_Error, (int)pSFM_conn->index, (int)pSFM_conn->source_addr, (int)rcvd_dest_addr, (int)pSFM_conn->dest_addr, (int)rcvd_src_addr, 0);
		GM_RSSP1_Log_Msg(1,"Inx %d SFM ID:0x%X, rcv SSE src/dest err!(from:0x%X,to:0x%X) err! cfg:(from:0x%X,to:0x%X).\n", (int)pSFM_conn->index, (int)pSFM_conn->SaCEPID.dest_addr, (int)rcvd_src_addr, (int)rcvd_dest_addr, (int)pSFM_conn->dest_addr, (int)pSFM_conn->source_addr);
		return;
	}

#ifndef CFM_Part/*如果SFM和CFM分拆运行,则需重新检查CRC16*/
	GM_RSSP1_CRC_Calculate_CRC16(pDatByte, ByteCount, &crc16, g_Polyomia[0U].pCRC16_table);
	rcvd_crc16 = ((pDatByte[ByteCount-1U]) << 8U) + (pDatByte[ByteCount-2U]);

	if (crc16 != 0U)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_SSE_CRC16_Error, (int)pSFM_conn->index, (int)rcvd_src_addr, (int)rcvd_dest_addr, (int)rcvd_crc16, (int)ByteCount, 0);
		/*GM_RSSP1_SFM_Report_Warning_To_User(pSFM_conn , pSFM , GM_RSSP1_SFM_WARNING_TYPE_RCVD_SSE_CRC_ERROR);*/
		GM_RSSP1_Log_Msg(1,"Inx %d SSE Head Src %x Dst %x with CRC16 %x err!\n", (int)pSFM_conn->index, (int)rcvd_src_addr, (int)rcvd_dest_addr, (int)rcvd_crc16, 0, 0);
		return;
	}
	else
#endif
	{
		if (GM_RSSP1_FALSE == SYS_Is_Local_Sys_Active())/**备系不允许发送SSR**/
		{
			return;
		}
	}

	/*增加在线数据管理*/
	rt = GM_RSSP1_Hash_Search_DstAndIndex((GM_RSSP1_UINT32)rcvd_src_addr, pSFM_conn->index, &ValidIndex, &FreeIndex);/**寻找hash索引值**/
	if (GM_RSSP1_TRUE == rt)
	{
		rt = GM_RSSP1_Hash_Insert_Online(ValidIndex, GM_RSSP1_TRUE);/**根据hash索引值,将该通道归入online**/
		if (GM_RSSP1_TRUE == rt)
		{
			rcvd_TC = ((GM_RSSP1_UINT32)(pDatByte[6U]) + ((GM_RSSP1_UINT32)(pDatByte[7U]) << 8U) + ((GM_RSSP1_UINT32)(pDatByte[8U]) << 16U) + ((GM_RSSP1_UINT32)(pDatByte[9U]) << 24U));

			SEQENQ1 = ((GM_RSSP1_UINT32)(pDatByte[10U]) + ((GM_RSSP1_UINT32)(pDatByte[11U]) << 8U) + ((GM_RSSP1_UINT32)(pDatByte[12U]) << 16U) + ((GM_RSSP1_UINT32)(pDatByte[13U]) << 24U));
			SEQENQ2 = ((GM_RSSP1_UINT32)(pDatByte[14U]) + ((GM_RSSP1_UINT32)(pDatByte[15U]) << 8U) + ((GM_RSSP1_UINT32)(pDatByte[16U]) << 16U) + ((GM_RSSP1_UINT32)(pDatByte[17U]) << 24U));

			GM_RSSP1_Log_Msg(4,"Rcving SSE: RcvTC = %d\n", (int)rcvd_TC, 0, 0, 0, 0, 0);

			GM_RSSP1_SFM_Send_SSR(SEQENQ1 , SEQENQ2 , rcvd_TC , pSFM_conn);
		}
	}
	return;
}

void GM_RSSP1_SFM_Proc_Rcvd_SSR(GM_RSSP1_SFM_connection_struct *pSFM_conn , GM_RSSP1_UINT8 pDatByte[], GM_RSSP1_UINT16 ByteCount)
{
	GM_RSSP1_UINT16 crc16 = 0U;
	GM_RSSP1_UINT16 rcvd_crc16 = 0U;
	GM_RSSP1_UINT32 SEQINI[GM_RSSP1_CHECK_CHN_NUM] = {0U};
	GM_RSSP1_UINT32 rcvd_TC_R = 0U;
	GM_RSSP1_UINT32 rcvd_TC_E = 0U;
	GM_RSSP1_INT32 chn_index = 0;
	GM_RSSP1_BOOL bchn_set_fail = GM_RSSP1_FALSE;
	GM_RSSP1_UINT8 rcvd_dataver = 0U;
	GM_RSSP1_UINT16 rcvd_src_addr = 0U;
	GM_RSSP1_UINT16 rcvd_dest_addr = 0U;
	GM_RSSP1_UINT16 con_index = 0xFFFFU;
	GM_RSSP1_UINT16 HashIndex = 0xFFFFU;
	GM_RSSP1_UINT16 ValidIndex = 0xFFFFU;
	GM_RSSP1_UINT16 FreeIndex = 0xFFFFU;
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;


	if ((NULL == pSFM_conn) || (NULL == pDatByte))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_SSR_Point_Error, (int)pSFM_conn, (int)pDatByte, 0, 0, 0, 0);
		return;
	}

	if (GM_RSSP1_Lib_State_Operational != g_SFM_obj.lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_SSR_Result_No_Match, (int)g_SFM_obj.lib_state, 0, 0, 0, 0, 0);
		return;
	}

	if (ByteCount != RSSP1_SSR_FRAME_LEN_WITH_CRC)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_SSR_Byte_Loss, pSFM_conn->index, (int)ByteCount, (int)RSSP1_SSR_FRAME_LEN_WITH_CRC, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"Inx %d SFM ID: 0x%X SSR rcvd frame length err! rcvd size:%d - %d.\n", (int)pSFM_conn->index, (int)pSFM_conn->SaCEPID.dest_addr, (int)ByteCount , (int)RSSP1_SSR_FRAME_LEN_WITH_CRC , 0, 0);
		return;
	}

	/* 记录收到SSR事件
	GM_RSSP1_SFM_Report_Warning_To_User(pSFM_conn , pSFM , GM_RSSP1_SFM_WARNING_TYPE_RECV_SSR); */

	rcvd_src_addr = ((GM_RSSP1_UINT16)(pDatByte[3U]) << 8U) + (GM_RSSP1_UINT16)pDatByte[2U];
	rcvd_dest_addr = ((GM_RSSP1_UINT16)(pDatByte[5U]) << 8U) + (GM_RSSP1_UINT16)pDatByte[4U];
	if ((rcvd_src_addr != pSFM_conn->dest_addr) || (rcvd_dest_addr != pSFM_conn->source_addr))/**地址检查**/
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_SSR_Addr_Error, (int)pSFM_conn->dest_addr, (int)rcvd_src_addr, (int)pSFM_conn->source_addr, (int)rcvd_dest_addr, 0, 0);
		/* 收到来源/目的 */
		GM_RSSP1_Log_Msg(1,"SFM ID:0x%X,index:%d, rcv SSR src/dest err!(from:0x%X,to:0x%X) err! cfg:(from:0x%X,to:0x%X).\n", (int)pSFM_conn->SaCEPID.dest_addr, (int)pSFM_conn->index, (int)rcvd_src_addr, (int)rcvd_dest_addr, (int)pSFM_conn->dest_addr, (int)pSFM_conn->source_addr);
		return;
	}

#ifndef CFM_Part/*如果SFM和CFM分拆运行,则需重新检查CRC16*/
	GM_RSSP1_CRC_Calculate_CRC16(pDatByte, ByteCount - 2U, &crc16, g_Polyomia[0U].pCRC16_table);

	rcvd_crc16 = ((pDatByte[ByteCount-1U]) << 8U) + (pDatByte[ByteCount-2U]);

	if (crc16 != rcvd_crc16)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_SSR_CRC16_Error, (int)pSFM_conn->index, (int)rcvd_src_addr, (int)rcvd_dest_addr, (int)rcvd_crc16, 0, 0);
		/*GM_RSSP1_SFM_Report_Warning_To_User(pSFM_conn , pSFM , GM_RSSP1_SFM_WARNING_TYPE_RCVD_SSR_CRC_ERROR);*/
		GM_RSSP1_Log_Msg(1,"Inx %d SSR Head Src %x Dst %x with CRC16 %x err!\n", (int)pSFM_conn->index, (int)rcvd_src_addr, (int)rcvd_dest_addr, (int)rcvd_crc16, 0, 0);
		return;
	}
#endif

	/*增加在线数据管理*/
	rt = GM_RSSP1_Hash_Search_DstAndIndex((GM_RSSP1_UINT32)rcvd_src_addr, pSFM_conn->index, &ValidIndex, &FreeIndex);/**寻找hash索引值**/
	if (GM_RSSP1_TRUE == rt)
	{
		rt = GM_RSSP1_Hash_Insert_Online(ValidIndex, GM_RSSP1_TRUE);/**根据hash索引值,将该通道归入online**/
		if (GM_RSSP1_TRUE == rt)
		{
			rcvd_TC_R  = ((GM_RSSP1_UINT32)(pDatByte[6U]) + ((GM_RSSP1_UINT32)(pDatByte[7U]) << 8U) + ((GM_RSSP1_UINT32)(pDatByte[8U]) << 16U) + ((GM_RSSP1_UINT32)(pDatByte[9U]) << 24U));

			rcvd_TC_E  = ((GM_RSSP1_UINT32)(pDatByte[10U]) + ((GM_RSSP1_UINT32)(pDatByte[11U]) << 8U) + ((GM_RSSP1_UINT32)(pDatByte[12U]) << 16U) + ((GM_RSSP1_UINT32)(pDatByte[13U]) << 24U));

			SEQINI[0U]   = ((GM_RSSP1_UINT32)(pDatByte[14U]) + ((GM_RSSP1_UINT32)(pDatByte[15U]) << 8U) + ((GM_RSSP1_UINT32)(pDatByte[16U]) << 16U) + ((GM_RSSP1_UINT32)(pDatByte[17U]) << 24U));
			SEQINI[1U]   = ((GM_RSSP1_UINT32)(pDatByte[18U]) + ((GM_RSSP1_UINT32)(pDatByte[19U]) << 8U) + ((GM_RSSP1_UINT32)(pDatByte[20U]) << 16U) + ((GM_RSSP1_UINT32)(pDatByte[21U]) << 24U));

			/*增加对SSR中数据版本(预留固定值0x01)的检查 jianghongjun TSRS00000125 */
			rcvd_dataver = pDatByte[22U];
			if (rcvd_dataver != 0x01U)
			{
				GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_SSR_DataVer_Error, (int)pSFM_conn->index, (int)rcvd_dataver, 0, 0, 0, 0);
				GM_RSSP1_Log_Msg(1,"Inx %d SFM ID:0x%X SSR dataver %d err!\n", (int)pSFM_conn->index, (int)rcvd_dataver, 0, 0, 0, 0);
				return;
			}

			for (chn_index = 0 ; chn_index < (GM_RSSP1_INT32)GM_RSSP1_CHECK_CHN_NUM ; ++chn_index)
			{
				if (GM_RSSP1_ERROR == GM_RSSP1_SSR_CHN_SET(
					g_Polyomia[chn_index].pLFSR_left_table,
					g_Polyomia[chn_index].pLFSR_right_table,
					SEQINI[chn_index],
					pSFM_conn->fsfb_chn[chn_index].PREC_FIRSTSINIT,
					pSFM_conn->env[chn_index].sseTC,
					pSFM_conn->env[chn_index].sseTS,
					&(pSFM_conn->env[chn_index].dynamicKey),
					pSFM_conn->env[chn_index].TC,
					&(pSFM_conn->env[chn_index].local_TC),
					&(pSFM_conn->env[chn_index].remote_TC),
					rcvd_TC_E,
					rcvd_TC_R,
					&(pSFM_conn->env[chn_index].bSendSSE),
					(GM_RSSP1_UINT32)pSFM_conn->lifeTime
					))
				{
					bchn_set_fail = GM_RSSP1_TRUE;
				}
			}
			if (GM_RSSP1_TRUE == bchn_set_fail)
			{
				GM_RSSP1_Log_Msg(2,"Inx %d, Chn %d SSR channel set err  TC_R %x, TC_E %x!\n", (int)pSFM_conn->index, (int)chn_index, (int)pSFM_conn->SaCEPID.dest_addr, (int)rcvd_TC_R, (int)rcvd_TC_E, 0);
			}
			else
			{
				pSFM_conn->RxSSRTrem = rcvd_TC_R;	/*计算preciTime用，added by huang 20151125*/
				pSFM_conn->RxSSRTloc = pSFM_conn->env[0U].TC;
				pSFM_conn->state = GM_RSSP1_Layer_State_Connected;
				GM_RSSP1_SetPreciTime(pSFM_conn);/*CR GM6950  更新SSE_SSR_Delay*/
			}
		}
	}
}

void GM_RSSP1_SFM_Proc_Rcvd_RSD(GM_RSSP1_SFM_connection_struct *pSFM_conn , GM_RSSP1_UINT8 pDatByte[], GM_RSSP1_UINT16 ByteCount)
{
	GM_RSSP1_UINT16 crc16 = 0U;
	GM_RSSP1_UINT16 rcvd_crc16 = 0U;
	GM_RSSP1_UINT32 rcvd_crc32[GM_RSSP1_CHECK_CHN_NUM] = {0U};
	GM_RSSP1_UINT32 rcvd_crcm[GM_RSSP1_CHECK_CHN_NUM] = {0U};
	GM_RSSP1_UINT32 rcvd_TC = 0U;
	GM_RSSP1_INT32 chn_index = 0;
	GM_RSSP1_UINT32 rcvd_app_dat_len = 0U;    /* 根据接收到的数据长度 - RSSP1协议规定的其他域长度得到的应用数据大小 */
	GM_RSSP1_UINT32 app_dat_len_in_pkt = 0U;    /* 数据包中SDS域指明的应用数据大小 */
	GM_RSSP1_INT32 rt = GM_RSSP1_ERROR;
	GM_RSSP1_SFM_L2U_pri_struct* pTmpData = NULL;
	GM_RSSP1_SFM_L2U_pri_struct dat_pri;
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;
	GM_RSSP1_UINT16 rcvd_src_addr = 0U;
	GM_RSSP1_UINT16 rcvd_dest_addr = 0U;
	GM_RSSP1_UINT32 CalCRC[GM_RSSP1_CHECK_CHN_NUM] = {0U};
	GM_RSSP1_UINT16 con_index = 0xFFFFU;
	GM_RSSP1_UINT16 HashIndex = 0xFFFFU;
	GM_RSSP1_UINT16 ValidIndex = 0xFFFFU;
	GM_RSSP1_UINT16 FreeIndex = 0xFFFFU;
	GM_RSSP1_BOOL ret = GM_RSSP1_FALSE;

	if ((NULL == pSFM_conn) || (NULL == pDatByte))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_RSD_Point_Error, (int)pSFM_conn, (int)pDatByte, 0, 0, 0, 0);
		return;
	}

	if (GM_RSSP1_Lib_State_Operational != g_SFM_obj.lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_RSD_Result_No_Match, (int)g_SFM_obj.lib_state, 0, 0, 0, 0, 0);
		return;
	}

	if (ByteCount > GM_RSSP1_MAX_SAFETY_DAT_LEN)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_RSD_Byte_Error, (int)pSFM_conn->index, (int)pSFM_conn->SaCEPID.dest_addr, (int)ByteCount, (int)GM_RSSP1_MAX_SAFETY_DAT_LEN, 0, 0);
		GM_RSSP1_Log_Msg(1,"Inx %d SFM ID: 0x%X RSD rcvd frame length %d exceed GM_RSSP1_MAX_SAFETY_DAT_LEN: %d.dat discarded!\n", (int)pSFM_conn->index, (int)pSFM_conn->SaCEPID.dest_addr, (int)ByteCount, (int)GM_RSSP1_MAX_SAFETY_DAT_LEN, 0, 0);
		return;
	}

	GM_RSSP1_memset((void*)&dat_pri, 0xFFU, sizeof(dat_pri));

	dat_pri.type = GM_RSSP1_SFM_L2U_unknow;

	rcvd_src_addr = ((GM_RSSP1_UINT16)(pDatByte[3U]) << 8U) + (GM_RSSP1_UINT16)pDatByte[2U];
	rcvd_dest_addr = ((GM_RSSP1_UINT16)(pDatByte[5U]) << 8U) + (GM_RSSP1_UINT16)pDatByte[4U];

	if ((GM_RSSP1_SYN_DEST_ADDR != pSFM_conn->dest_addr) && (GM_RSSP1_SYN_DEST_ADDR != rcvd_dest_addr))
	{
		/* 主备同步通道外的其他通道，检验来源/目的地址的合法性 */
		if ((rcvd_src_addr != pSFM_conn->dest_addr) || (rcvd_dest_addr != pSFM_conn->source_addr))
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_RSD_Addr_Error, (int)pSFM_conn->index, (int)rcvd_src_addr, (int)rcvd_dest_addr, (int)pSFM_conn->dest_addr, (int)pSFM_conn->source_addr, 0);
			/* 收到来源/目的 */
			GM_RSSP1_Log_Msg(1,"Inx %d rcv dat src/dest err!(from:0x%X,to:0x%X) err! cfg:(from:0x%X,to:0x%X).\n", (int)pSFM_conn->index, (int)rcvd_src_addr, (int)rcvd_dest_addr, (int)pSFM_conn->dest_addr, (int)pSFM_conn->source_addr, 0);
			return;
		}
	}

	/* 接收到对方主和备机发送的消息，都进行验证，以保证维持fsfb连接 */
	if ((RSSP1_RSD_PROTOCOL_TYPE_ACTIVE == pDatByte[0U]) || (RSSP1_RSD_PROTOCOL_TYPE_STANDBY == pDatByte[0U]))
	{
		if (pSFM_conn->remote_dev_AB != GM_RSSP1_Remote_Dev_Unknow)		/*jcf:CR5649 若User配置了该字段，才进行远端AB系的检查*/
		{
			if((GM_RSSP1_UINT32)pSFM_conn->remote_dev_AB != (GM_RSSP1_UINT32)pDatByte[1U])
			{
				GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_RSD_Remote_AB_Error, (int)pSFM_conn->index, (int)pDatByte[1], (int)pSFM_conn->remote_dev_AB, 0, 0, 0);
				GM_RSSP1_Log_Msg(1,"Inx %d: remote device AB err! (Msg:0x%X) (cfg:0x%X).\n", (int)pSFM_conn->index, (int)pDatByte[1], (int)pSFM_conn->remote_dev_AB,0,0,0);
				return;
			}
		}
		
		if (RSSP1_RSD_PROTOCOL_TYPE_ACTIVE == pDatByte[0U])
		{
			pSFM_conn->remote_dev_AS_state = GM_RSSP1_Remote_Dev_Active;
		}
		else
		{
			pSFM_conn->remote_dev_AS_state = GM_RSSP1_Remote_DEV_Standby;
		}

#ifndef CFM_Part/*如果SFM和CFM分拆运行,则需重新检查CRC16*/
		GM_RSSP1_CRC_Calculate_CRC16(pDatByte, ByteCount - 2U, &crc16, g_Polyomia[0U].pCRC16_table);
		rcvd_crc16 = ((pDatByte[ByteCount-1U]) << 8U) + (pDatByte[ByteCount-2U]);
		if (crc16 != rcvd_crc16)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_RSD_CRC16_Error, 0, 0, 0, 0, 0, 0);
			/*GM_RSSP1_SFM_Report_Warning_To_User(pSFM_conn , pSFM , GM_RSSP1_SFM_WARNING_TYPE_RCVD_RSD_CRC_ERROR);*/
			GM_RSSP1_Log_Msg(1,"Inx %d RSD Head Src %x Dst %x with CRC16 %x err!\n", (int)pSFM_conn->index, (int)rcvd_src_addr, (int)rcvd_dest_addr, (int)rcvd_crc16, 0, 0);
			return;
		}
		else/* crc check OK */
		{
		   ;/*nothing*/
		}
#endif
		{

			/**CR:GM00004753 Dele the local standby protocol**/
			if (RSSP1_RSD_PROTOCOL_TYPE_STANDBY == pDatByte[0U]) /**备系数据不能用于安全逻辑,因此直接不检查.同时由于远端为备系(不能发送SSR),安全链接是不可能建立**/
			{
				pSFM_conn->state = GM_RSSP1_Layer_State_Connected;
				/*CR:GM00005082 备系不做检查，但要更新必备的数据，否则连接断开，上报用户时会有Unkonwn未知态
				pSFM_conn->state_check_counter2 = (GM_RSSP1_INT8)pSFM_conn->keepIdle;	
				pSFM_conn->state_check_counter1 = 0;*/
				return;
			}
		}

		/*增加在线数据管理*/
		ret = GM_RSSP1_Hash_Search_DstAndIndex((GM_RSSP1_UINT32)rcvd_src_addr, pSFM_conn->index, &ValidIndex, &FreeIndex);/**寻找hash索引值**/
		if (GM_RSSP1_TRUE == ret)
		{
			ret = GM_RSSP1_Hash_Insert_Online(ValidIndex, GM_RSSP1_TRUE);/**根据hash索引值,将该通道归入online**/
			if (GM_RSSP1_TRUE == ret)
			{
				if (pSFM_conn->state == GM_RSSP1_Layer_State_Connected)
				{
					GM_RSSP1_Log_Msg(4,"SFM ID %d: state connected.\n", (int)pSFM_conn->index, 0, 0, 0, 0, 0);

					rcvd_app_dat_len = (GM_RSSP1_UINT32)ByteCount - 22U;
					app_dat_len_in_pkt = (GM_RSSP1_UINT32)(pDatByte[10U]) + ((GM_RSSP1_UINT32)(pDatByte[11U]) << 8U) - (GM_RSSP1_UINT32)8U ;
					if (rcvd_app_dat_len != app_dat_len_in_pkt)
					{
						GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_RSD_Byte_Error, (int)pSFM_conn->index, (int)rcvd_app_dat_len, (int)app_dat_len_in_pkt, 0, 0, 0);
						GM_RSSP1_Log_Msg(1,"SFM Inx 0x%d rcvd RSD frame length err! exp-size: %d , actual: %d.\n", (int)pSFM_conn->index, (int)app_dat_len_in_pkt, (int)rcvd_app_dat_len, 0, 0, 0);
						return;
					}

					/* 校验前 , 计算RSD内应用数据的明文checksum，保存到dat_pri，防止校验过程中改变应用数据 */
					rcvd_TC = ((GM_RSSP1_UINT32)(pDatByte[6U]) + ((GM_RSSP1_UINT32)(pDatByte[7U]) << 8U) + ((GM_RSSP1_UINT32)(pDatByte[8U]) << 16U) + ((GM_RSSP1_UINT32)(pDatByte[9U]) << 24U));
					rcvd_crcm[0U] = ((GM_RSSP1_UINT32)(pDatByte[12U]) + ((GM_RSSP1_UINT32)(pDatByte[13U]) << 8U) + ((GM_RSSP1_UINT32)(pDatByte[14U]) << 16U) + ((GM_RSSP1_UINT32)(pDatByte[15U]) << 24U));
					rcvd_crcm[1U] = ((GM_RSSP1_UINT32)(pDatByte[16U]) + ((GM_RSSP1_UINT32)(pDatByte[17U]) << 8U) + ((GM_RSSP1_UINT32)(pDatByte[18U]) << 16U) + ((GM_RSSP1_UINT32)(pDatByte[19U]) << 24U));

					for (chn_index = 0 ; chn_index < (GM_RSSP1_INT32)GM_RSSP1_CHECK_CHN_NUM ; ++chn_index)
					{
						GM_RSSP1_CRC_Calculate_CRC32(pDatByte + 20U, rcvd_app_dat_len, &(rcvd_crc32[chn_index]), g_Polyomia[chn_index].pCRC32_table);
						rt = GM_RSSP1_CRCM_CHN_CHECK_Int(
							rcvd_crcm[chn_index],
							rcvd_crc32[chn_index],
							(~(pSFM_conn->fsfb_chn[chn_index].chn_cfg.local_sys_chk)),
							g_Polyomia[chn_index].pLFSR_left_table,
							g_Polyomia[chn_index].pLFSR_right_table,
							rcvd_TC,
							pSFM_conn->env[chn_index].TC,
							&(pSFM_conn->env[chn_index].local_TC),
							&(pSFM_conn->env[chn_index].remote_TC),
							&(pSFM_conn->env[chn_index].dynamicKey),
							(GM_RSSP1_UINT32 *)(pSFM_conn->fsfb_chn[chn_index].PREC_SINIT),
							(GM_RSSP1_UINT32 *)(pSFM_conn->fsfb_chn[chn_index].POST_RXDATA),
							pSFM_conn->deltaTime,
							pSFM_conn->DelayTime,
							&(CalCRC[chn_index])
#ifdef OFFLINE_TOOL_SUPPORT
							,
							&(pSFM_conn->env[chn_index].dynLocKey),
							(GM_RSSP1_UINT32 *)(pSFM_conn->fsfb_chn[chn_index].CON_1),
							(GM_RSSP1_UINT32 *)(pSFM_conn->fsfb_chn[chn_index].CON_2),
							(GM_RSSP1_UINT32 *)(pSFM_conn->fsfb_chn[chn_index].CON_3),
							chn_index
#endif
							);

						if (GM_RSSP1_ERROR == rt)
						{
							/*GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_RSD_Check_Error, 0, 0, 0, 0, 0, 0);
							GM_RSSP1_SFM_Report_Warning_To_User(pSFM_conn , pSFM , GM_RSSP1_SFM_WARNING_TYPE_FSFB_CRCM_CHECK_ERR);*/
							break;
						}
						else if(GM_RSSP1_RETURN_RESERVED == rt)
						{
							return;
						}
						else
						{
							; /**do nothing**/
						}
					}
				}

				if (GM_RSSP1_OK != rt)
				{
					/* reset tolerate vars */
					pSFM_conn->b_rcvd_new_dat_in_cycle = GM_RSSP1_FALSE;
					GM_RSSP1_memset((void*)&(pSFM_conn->last_valid_dat_pri) , 0U, sizeof(pSFM_conn->last_valid_dat_pri));
					pSFM_conn->b_torlerant_dat_valid = GM_RSSP1_FALSE;
					pSFM_conn->state = GM_RSSP1_Layer_State_Connecting;
					/* FSFB check fail,send SSE */
					GM_RSSP1_SFM_Send_SSE(pSFM_conn);
				}
				else
				{
					pSFM_conn->torlerate_cycle_count1 = 0U;
#ifdef RSSP1_TolerateWithCycle
					pSFM_conn->torlerated_cycle_count2 = -1;	/*CR5649 为支持异步通信节点，采用新的计数规则，state不再使用，宽恕拆分为已宽恕和宽恕差*/
#endif
					pSFM_conn->SINIT_TremCycle = rcvd_TC;	/*计算preciTime用，added by huang 20151125*/
					dat_pri.type = GM_RSSP1_SFM_Data_Ind;

					/* 地址使用外部收到的信息，防止匹配错误 jianghongjun 20110825 safety enhancement */
					dat_pri.SaCEPID.source_addr = rcvd_dest_addr;
					dat_pri.SaCEPID.dest_addr = rcvd_src_addr;

					pSFM_conn->b_rcvd_new_dat_in_cycle = GM_RSSP1_TRUE;/*RVS:9968420 reset the flag before SetPreciTime*/
					
					GM_RSSP1_SetPreciTime(pSFM_conn);

#ifdef GM_RSSP1_RxMsgWithTime/*将2个时间信息放入数据前8位*/
					dat_pri.as.data_indi.bytes_count = (GM_RSSP1_UINT16)rcvd_app_dat_len+8U;
					GM_RSSP1_memcpy((void*)dat_pri.as.data_indi.byte, (const void*)&(pSFM_conn->PreciTime), (size_t)4U);
					GM_RSSP1_memcpy((void*)(dat_pri.as.data_indi.byte+4U), (const void*)&(pSFM_conn->SSE_SSR_Delay), (size_t)4U);
					GM_RSSP1_memcpy((void*)(dat_pri.as.data_indi.byte+8U),(const void*)(pDatByte + 20U), (size_t)rcvd_app_dat_len);
#else
					dat_pri.as.data_indi.bytes_count = (GM_RSSP1_UINT16)rcvd_app_dat_len;
					GM_RSSP1_memcpy((void*)dat_pri.as.data_indi.byte, (const void*)(pDatByte + 20U), (size_t)rcvd_app_dat_len);
#endif
					GM_RSSP1_memcpy((void*)dat_pri.as.data_indi.CRC, (const void*)CalCRC, sizeof(GM_RSSP1_UINT32)* 2U);


					if (GM_RSSP1_TRUE == pSFM_conn->b_enable_FSFB_on_chn)
					{
						/* 直接储存到本通道内的队列 */
						write_q_rt = FSFB_Msg_Queue_Write(&(pSFM_conn->L2U_dat_Q) , (void*)&dat_pri);
					}
					else
					{
						/* 存储到SFM的队列 */
						write_q_rt = FSFB_Msg_Queue_Write(&(g_SFM_MQ.RCV_MQ) , (void*)&dat_pri);
					}

					if (Q_WRITE_OK_FULL == write_q_rt)
					{
						GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_RSD_Write_Full, 0, 0, 0, 0, 0, 0);
						GM_RSSP1_Log_Msg(1,"Inx %d: SFM object/SFM_connection L2U_pri_Q full!Type:%d.\n", (int)pSFM_conn->index, (int)pSFM_conn->b_enable_FSFB_on_chn, 0, 0, 0, 0);
					}
					else if (Q_WRITE_FAIL == write_q_rt)
					{
						GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Rcvd_RSD_Write_Error, 0, 0, 0, 0, 0, 0);
						GM_RSSP1_Log_Msg(1,"Inx %d: SFM object/SFM_connection L2U_pri_Q Error! MQ Type %d.\n", (int)pSFM_conn->index, (int)pSFM_conn->b_enable_FSFB_on_chn, 0, 0, 0, 0);
						g_SFM_obj.lib_state = GM_RSSP1_Lib_State_Failure;
					}
					else
					{
						; /**do nothing**/
					}

					if (pSFM_conn->torlerate_cycle_cfg != 0u)
					{
						GM_RSSP1_memcpy((void*)&pSFM_conn->last_valid_dat_pri, (const void*)&dat_pri, sizeof(GM_RSSP1_SFM_L2U_pri_struct));
						pSFM_conn->b_torlerant_dat_valid = GM_RSSP1_TRUE;
					}
				}
			}
		}

	}
	else
	{
		pSFM_conn->remote_dev_AS_state = GM_RSSP1_Remote_DEV_Unknow;
		/* unkown Active standby flag */
		GM_RSSP1_Log_Msg(2,"Inx %d remote dev AS state flag:0x%X err!\n", (int)pSFM_conn->index, (int)pDatByte[0U], 0, 0, 0, 0);
		/* reset tolerate vars */
		pSFM_conn->b_rcvd_new_dat_in_cycle = GM_RSSP1_FALSE;
		GM_RSSP1_memset((void*)&(pSFM_conn->last_valid_dat_pri) , 0U, sizeof(GM_RSSP1_SFM_L2U_pri_struct));
		pSFM_conn->b_torlerant_dat_valid = GM_RSSP1_FALSE;
	}

	return;
}

void GM_RSSP1_SFM_Reset_Connection_RSD_Sent_Flag(void)
{
	GM_RSSP1_UINT16 onlineNum = 0U;
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_UINT16 ValidIndex = 0xFFFFU;
	GM_RSSP1_UINT16 FreeIndex = 0xFFFFU;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_connection_struct *pSFM_conn = NULL;

	onlineNum = GM_RSSP1_GetSeqAmount(OnLine_Object);
	GM_RSSP1_GetSeqIndexReverse(OnLine_Object, 1U);/**重置反序查询索引**/
	for (l_index = 0U; l_index < onlineNum; ++l_index)/**按online配置链表顺序,获取index**/
	{
		con_index = GM_RSSP1_GetSeqIndexReverse(OnLine_Object, 0U);
		/*if (0xFFFFU!= con_index)*/
		if(con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
		{
			pSFM_conn = &(g_SFM_obj.connection[con_index]);

			if ((GM_RSSP1_Layer_State_Free == pSFM_conn->state) && (GM_RSSP1_TRUE != (GM_RSSP1_BOOL)pSFM_conn->IsFixed)) /*未建立连接,释放所占在线通道*/
			{
				bRt = GM_RSSP1_Hash_Search_DstAndIndex((GM_RSSP1_UINT32)pSFM_conn->dest_addr, pSFM_conn->index, &ValidIndex, &FreeIndex);
				bRt = GM_RSSP1_Hash_Insert_Online(ValidIndex, GM_RSSP1_FALSE);
			}
		}
	}

	return;
}

void GM_RSSP1_SFM_Process_TxData(GM_RSSP1_SFM_U2L_pri_struct *pri)
{
	GM_RSSP1_SFM_connection_struct *pSFM_conn = NULL;
	GM_RSSP1_INT32 rt = GM_RSSP1_ERROR;
	GM_RSSP1_UINT8 ConNum = 8U;
	GM_RSSP1_UINT16 i = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_UINT16 indexArray[8U] = {0U};
	GM_RSSP1_UINT16 HashIndex = 0xFFFFU;

	if (NULL == pri)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Process_TxData_Param_Point_Error, 0, 0, 0, 0, 0, 0);
	}
	else
	{
		GM_RSSP1_memset((void*)indexArray, 0xFFU, sizeof(indexArray));

		rt = GM_RSSP1_Hash_CnfDst_ResArray((GM_RSSP1_UINT32)pri->SaCEPID.dest_addr, indexArray, &ConNum);/**遍历hash表,将对应地址的所有静态index都一次查询出来.注意项目最多只能配4组相同地址信息**/
		if (GM_RSSP1_TRUE == rt)
		{
			for (i=0U; i<(ConNum/2U); ++i)
			{
				con_index = indexArray[i*2U];
				HashIndex = indexArray[i*2U + 1U];

				pSFM_conn = &(g_SFM_obj.connection[con_index]);

				if (GM_RSSP1_FALSE == pSFM_conn->b_RSD_Already_Sent)/*RSD每周期只允许发送一包*/
				{
					rt = GM_RSSP1_Hash_Insert_Online(HashIndex, GM_RSSP1_TRUE);/**根据hash索引值,将该通道归入online**/
					if (GM_RSSP1_TRUE == rt)
					{
						rt = GM_RSSP1_SFM_Send_RSD(pri , pSFM_conn);
						if (GM_RSSP1_TRUE == rt)
						{
							pSFM_conn->b_RSD_Already_Sent = GM_RSSP1_TRUE;
						}
						else
						{
							if (GM_RSSP1_TRUE != (GM_RSSP1_BOOL)pSFM_conn->IsFixed)
							{
								GM_RSSP1_Hash_Insert_Online(HashIndex, GM_RSSP1_FALSE);/**根据hash索引值,将该online通道释放**/
							}
						}
					}
				}
			}
		}
	}
}


void GM_RSSP1_SFM_Process_User_Req(void)
{
	GM_RSSP1_SFM_U2L_pri_struct pri;

	while (GM_RSSP1_TRUE == FSFB_Msg_Queue_Get(&(g_SFM_MQ.SND_MQ) , &pri))
	{
		switch (pri.type)
		{
		case GM_RSSP1_Pri_Type_SFM_Data_req:
			GM_RSSP1_SFM_Process_TxData(&pri);
			break;

		default:
			/*do nothing*/
			break;
		}
	}

	return ;
}

void GM_RSSP1_SFM_Report_Connection_State_To_User(GM_RSSP1_CFM_L2U_pri_struct *pIndi_pri)
{
	GM_RSSP1_UINT16 i = 0U;
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_UINT16 l_cnf_index = 0U;
	GM_RSSP1_UINT16 onlineNum = 0U;
	GM_RSSP1_UINT16 curindex = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_connection_struct *pSFM_con = NULL;
	GM_RSSP1_SFM_L2U_pri_struct pri;
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;
	GM_RSSP1_SeqDele_Record_Structure *pDele_record = NULL;
	if (NULL == pIndi_pri)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Report_Connection_State_To_User_Point_Error, (int)pIndi_pri, 0, 0, 0, 0, 0);
		return;
	}

	if (GM_RSSP1_Lib_State_Operational != g_SFM_obj.lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Report_Connection_State_To_User_Result_No_Match, (int)g_SFM_obj.lib_state, 0, 0, 0, 0, 0);
		return;
	}

	pri.type = GM_RSSP1_SFM_Connection_State_Indi;

	onlineNum = GM_RSSP1_GetSeqAmount(OnLine_Object);
	GM_RSSP1_GetSeqIndexPositive(OnLine_Object, 1U);/**重置正序查询索引**/
	for (l_index = 0U; l_index < onlineNum ; ++l_index)/**按online配置链表顺序,获取index**/
	{
		con_index = GM_RSSP1_GetSeqIndexPositive(OnLine_Object, 0U);
		/*if (0xFFFFU != con_index)*/
		if(con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
		{
			pSFM_con = &(g_SFM_obj.connection[con_index]);

			/* fsfb state:  4 bits */
			if (GM_RSSP1_Layer_State_Connected == pSFM_con->state)/**只有SFM链接成功,才上报状态**/
			{
				pri.as.connection_state.connection_states[curindex].index = con_index;
				pri.as.connection_state.connection_states[curindex].sacepid = pSFM_con->SaCEPID;
				pri.as.connection_state.connection_states[curindex].state |= GM_RSSP1_REPORT_STATE_TO_APP_FSFB_OK << 28U;


				/* remote active-standby state:  4 bits */
				if (GM_RSSP1_Remote_Dev_Active == pSFM_con->remote_dev_AS_state)
				{
					pri.as.connection_state.connection_states[curindex].state |= GM_RSSP1_REPORT_STATE_TO_APP_REMOTE_ACTIVE << 24U;
				}
				else if (GM_RSSP1_Remote_DEV_Standby == pSFM_con->remote_dev_AS_state)
				{
					pri.as.connection_state.connection_states[curindex].state |= GM_RSSP1_REPORT_STATE_TO_APP_REMOTE_STANDBY << 24U;
				}
				else
				{
					pri.as.connection_state.connection_states[curindex].state |= GM_RSSP1_REPORT_STATE_TO_APP_REMOTE_UNKOWN << 24U;
				}
				curindex++;
			}
		}
	}
	pri.as.connection_state.connection_num = curindex;

	if (pIndi_pri->index > GM_RSSP1_MAX_ONLINE_CONNECTION_NUM )
	{
		GM_RSSP1_Log_Msg(2,"The number inside the States Report is invalid cur:%d MAX:%d!\n", (int)(pIndi_pri->index), (int)GM_RSSP1_MAX_ONLINE_CONNECTION_NUM, 0, 0, 0, 0);
		pIndi_pri->index = GM_RSSP1_MAX_ONLINE_CONNECTION_NUM;
	}

	/* UDP chn nums:    4 bits */
	for (i=0U; i<pIndi_pri->index; ++i) /*在CFM，pIndi_pri->index被赋值了CFM的接收到UDP的总通道数*/
	{
		bRt = GM_RSSP1_FALSE;
		for (l_index = 0U; l_index < pri.as.connection_state.connection_num ; ++l_index) /*如果SFM层已连接，同时UDP状态活动则加入SFM的索引*/
		{
			if (pri.as.connection_state.connection_states[l_index].index == pIndi_pri->as.channel_state_indi.GM_RSSP1_CFM_Channel_states[i].index)
			{
				pri.as.connection_state.connection_states[l_index].state |= ((GM_RSSP1_UINT32)(pIndi_pri->as.channel_state_indi.GM_RSSP1_CFM_Channel_states[i].chn_num) << 20U);
				/* UDP chn state:    8 bits */
				pri.as.connection_state.connection_states[l_index].state |= ((GM_RSSP1_UINT32)(pIndi_pri->as.channel_state_indi.GM_RSSP1_CFM_Channel_states[i].channel_state) << 12U);
				/* reserved:    12 bits: */	

				bRt = GM_RSSP1_TRUE;
				break;
			}
		}

		if ((GM_RSSP1_FALSE == bRt) && (curindex<GM_RSSP1_MAX_ONLINE_CONNECTION_NUM)) /*如果SFM链接未建立，但有UDP数据接收则根据状态包结构需新构建。但此时SFM层状态为未知*/
		{
			con_index = pIndi_pri->as.channel_state_indi.GM_RSSP1_CFM_Channel_states[i].index;
			pSFM_con = &(g_SFM_obj.connection[con_index]);

			pri.as.connection_state.connection_states[curindex].index = con_index;
			pri.as.connection_state.connection_states[curindex].sacepid = pSFM_con->SaCEPID;
			pri.as.connection_state.connection_states[curindex].state |= ((GM_RSSP1_UINT32)GM_RSSP1_REPORT_STATE_TO_APP_REMOTE_UNKOWN << 24U);
			pri.as.connection_state.connection_states[curindex].state |= ((GM_RSSP1_UINT32)(pIndi_pri->as.channel_state_indi.GM_RSSP1_CFM_Channel_states[i].chn_num) << 20U);
			/* UDP chn state:    8 bits */
			pri.as.connection_state.connection_states[curindex].state |= ((GM_RSSP1_UINT32)(pIndi_pri->as.channel_state_indi.GM_RSSP1_CFM_Channel_states[i].channel_state) << 12U);
			/* reserved:    12 bits: */	

			curindex++;
		}
	}
	pri.as.connection_state.connection_num = curindex;

	/*jcf:添加对当前周期断开连接Node的状态上报*/
	pDele_record = GM_RSSP1_SeqDeleteRecord_Get();
	for(l_index = 0U; l_index < pDele_record->RecordNum ; ++l_index)/**获取已记录的连接状态变化的index**/
	{
		l_cnf_index = pDele_record->RecordIndex[l_index];
		pSFM_con = &(g_SFM_obj.connection[l_cnf_index]);
		if( (curindex<GM_RSSP1_MAX_ONLINE_CONNECTION_NUM) && (GM_RSSP1_Layer_State_Connected != pSFM_con->state) )	/*当前周期 断开的连接*/
		{
			pri.as.connection_state.connection_states[curindex].index = l_cnf_index;
			pri.as.connection_state.connection_states[curindex].sacepid = pSFM_con->SaCEPID;
			pri.as.connection_state.connection_states[curindex].state |= (GM_RSSP1_UINT32)(GM_RSSP1_REPORT_STATE_TO_APP_FSFB_FAIL << 28U);
			curindex++;
		}
	}
	pri.as.connection_state.connection_num = curindex;	/*jcf:此处赋值不代表connection_num的增加，而是为了APP_Interface_Rcv_App_Dat()中能够处理此断连包*/

	write_q_rt = FSFB_Msg_Queue_Write(&(g_SFM_MQ.RCV_MQ) , &pri);

	if (Q_WRITE_OK_FULL == write_q_rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Report_Connection_State_To_User_Write_Full, 0, 0, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"SFM L2U_pri_Q full , report connection state fail!\n", 0, 0, 0, 0, 0, 0);
	}
	else if (Q_WRITE_FAIL == write_q_rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Report_Connection_State_To_User_Write_Fail, 0, 0, 0, 0, 0, 0);
		g_SFM_obj.lib_state = GM_RSSP1_Lib_State_Failure;
	}
	else
	{
		; /**do nothing**/
	}

	return;
}

void GM_RSSP1_SFM_Process_RxData(GM_RSSP1_CFM_L2U_type_enum InsType, GM_RSSP1_UINT16 Inx, GM_RSSP1_UINT8 msgInd[], GM_RSSP1_UINT16 msgLen)
{
	GM_RSSP1_SFM_connection_struct *pSFM_conn = NULL;
	
	if (Inx < GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)/*CR5746*/
	{
		pSFM_conn = &(g_SFM_obj.connection[Inx]);
		switch (InsType)
		{
		case GM_RSSP1_CFM_Data_Ind:
			if (GM_RSSP1_SYN_DEST_ADDR == pSFM_conn->SaCEPID.dest_addr)
			{
				/* 本通道id为同步通道,则从此处将数据拦截，直接作为同步数据处理，而不再送向上层 */
				GM_RSSP1_Syn_Proc_Rcvd_Syn_Dat(msgInd, (GM_RSSP1_INT32)msgLen);
			}
			else
			{
				if (msgInd[1U] == RSSP1_SSE_FRAME_TYPE)
				{
					GM_RSSP1_Log_Msg(4,"SFM ID:0x%X <---- rcvd SSE.%d bytes.(%d.%d),index:%d.\n", (int)pSFM_conn->SaCEPID.dest_addr , (int)msgLen, (int)(pSFM_conn->env[0U].local_TC), (int)(pSFM_conn->env[0U].remote_TC), (int)pSFM_conn->index, 0);
					GM_RSSP1_SFM_Proc_Rcvd_SSE(pSFM_conn , msgInd,  msgLen);
				}
				else if (msgInd[1U] == RSSP1_SSR_FRAME_TYPE)
				{
					GM_RSSP1_Log_Msg(4,"SFM ID:0x%X <------ rcvd SSR.%d bytes.(%d.%d),index:%d.\n", (int)pSFM_conn->SaCEPID.dest_addr , (int)msgLen, (int)(pSFM_conn->env[0U].local_TC), (int)(pSFM_conn->env[0U].remote_TC), (int)pSFM_conn->index, 0);
					GM_RSSP1_SFM_Proc_Rcvd_SSR(pSFM_conn , msgInd, msgLen);
				}
				else if ((msgInd[1U] == RSSP1_RSD_FRAME_TYPE_FROM_A) || (msgInd[1U] == RSSP1_RSD_FRAME_TYPE_FROM_B))
				{
					GM_RSSP1_Log_Msg(4,"SFM ID:from 0x%X <-------- rcvd RSD.%d bytes.(%d.%d),index:%d.\n", (int)pSFM_conn->SaCEPID.dest_addr , (int)msgLen, (int)(pSFM_conn->env[0U].local_TC), (int)(pSFM_conn->env[0U].remote_TC), (int)pSFM_conn->index, 0);
					GM_RSSP1_SFM_Proc_Rcvd_RSD(pSFM_conn , msgInd, msgLen);
				}
				else
				{
					GM_RSSP1_Log_Msg(2,"SFM ID:0x%X <---- rcvd unknow type frame. %d bytes.\n", (int)pSFM_conn->SaCEPID.dest_addr , (int)msgLen, 0, 0, 0, 0);
				}
			}
			break;

		case GM_RSSP1_CFM_Indi_com_State:
			/**在数据宽恕中,影响状态变化,需在状态包上传前进行处理.保证提供的数据与对应状态一致**/
			GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate();
			/* collect CFM udp channel state , report state to USER with FSFB state. */
			GM_RSSP1_SFM_Report_Connection_State_To_User((GM_RSSP1_CFM_L2U_pri_struct*)(msgInd-sizeof(GM_RSSP1_CFM_L2U_type_enum)-sizeof(GM_RSSP1_UINT16)-sizeof(GM_RSSP1_UINT16)));/*特殊处理,由于借口形参修改。 传入的地址需偏移地址*/
			break;

		default:
			/*do nothing*/
			break;
		}
	}
}
void GM_RSSP1_SFM_Process_CFM_Ind(GM_RSSP1_SFM_object_struct *pSFM)
{
	GM_RSSP1_INT32 con_index = 0;
	GM_RSSP1_UINT16 Num = 0U;
	GM_RSSP1_CFM_L2U_pri_struct cfm_ind;
	GM_RSSP1_SeqDele_Record_Structure* pRec = NULL;
	pRec = GM_RSSP1_SeqDeleteRecord_Get();/**当前周期记录的状态变化的链接**/

	if (NULL == pRec)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Process_CFM_Ind_Result_No_Match, 0, 0, 0, 0, 0, 0);
		return ;
	}

#ifndef GM_RSSP1_SAVING_MODE
	pRec->RecordNum = 0U;
#endif
	GM_RSSP1_memset(&cfm_ind, 0U, sizeof(GM_RSSP1_CFM_L2U_pri_struct));
	while (GM_RSSP1_SFM_Get_Pri_OK == GM_RSSP1_SFM_Get_Pri_From_CFM(&cfm_ind))
	{
		GM_RSSP1_SFM_Process_RxData(cfm_ind.type, cfm_ind.index, (GM_RSSP1_UINT8*)&(cfm_ind.as.data_indi.byte), cfm_ind.as.data_indi.bytes_count);
	}
}

void GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate(void)
{
	GM_RSSP1_SFM_connection_struct *pSFM_con = NULL;
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;
	GM_RSSP1_UINT16 onlineNum = 0U;
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_UINT16 ValidIndex = 0xFFFFU;
	GM_RSSP1_UINT16 FreeIndex = 0xFFFFU;
	GM_RSSP1_BOOL ret = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_L2U_pri_struct *pTmp = NULL;
#ifdef OFFLINE_TOOL_SUPPORT
	GM_RSSP1_SFM_L2U_pri_struct pri = {0U};
	GM_RSSP1_UINT32 CRCM[GM_RSSP1_CHECK_CHN_NUM] = {0U};
	GM_RSSP1_UINT32 VSN0 = 0U;
	GM_RSSP1_UINT32 VSN1 = 0U;
	GM_RSSP1_UINT32 VSN2 = 0U;
	GM_RSSP1_LFSR reg = {0};
	GM_RSSP1_UINT8 i = 0U;
	GM_RSSP1_UINT8 j = 0U;
#endif

	if (GM_RSSP1_Lib_State_Operational != g_SFM_obj.lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate_Result_No_Match, 0, 0, 0, 0, 0, 0);
		return;
	}
	/*jcf:删除重复宽恕的报警信息，改为若已宽恕直接return*/
	if(GM_RSSP1_TRUE == g_SFM_obj.Is_torlerate_process)
	{
		return;
	}
	else
	{
		g_SFM_obj.Is_torlerate_process = GM_RSSP1_TRUE;
	}

	GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 1U);
	for (l_index = 0U; l_index < g_SFM_obj.connection_nums; ++l_index)
	{
		con_index = GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 0U);
		/*if (0xFFFFU != con_index)*/
		if(con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
		{
			pSFM_con = &(g_SFM_obj.connection[con_index]);

			if (GM_RSSP1_Layer_State_Connected == pSFM_con->state)
			{
				/*CR5649 若当前节点持续未收到数据 超过了配置的周期计数区间，则断开连接*/
				if ( pSFM_con->env[0U].TC > (GM_RSSP1_UINT32)(pSFM_con->keepIdle) + pSFM_con->env[0U].local_TC )
				{
					if (GM_RSSP1_TRUE != (GM_RSSP1_BOOL)pSFM_con->IsFixed)
					{
						ret = GM_RSSP1_Hash_Search_DstAndIndex((GM_RSSP1_UINT32)pSFM_con->dest_addr, con_index, &ValidIndex, &FreeIndex);
						ret = GM_RSSP1_Hash_Insert_Online(ValidIndex, GM_RSSP1_FALSE);
					}

					/*pSFM_con->state_check_counter1 = 0;
					pSFM_con->state_check_counter2 = (GM_RSSP1_INT8)pSFM_con->keepIdle;*/
					pSFM_con->b_rcvd_new_dat_in_cycle = GM_RSSP1_FALSE;
					pSFM_con->state = GM_RSSP1_Layer_State_Free;
					/* start:CR:GM00002529, GM162, fulintai,2013-06-09, reset the AS status, otherwise it would conflict inside when remote switch AS*/
					pSFM_con->remote_dev_AS_state = GM_RSSP1_Remote_DEV_Unknow;
					/* end CR:GM00002529*/
				}
			}
			else if (GM_RSSP1_Layer_State_Connecting == pSFM_con->state)/*缓解频繁抢夺未分配资源的情况，稳定通道一段时间*/
			{
				if (GM_RSSP1_TRUE != (GM_RSSP1_BOOL)pSFM_con->IsFixed)
				{
					if (pSFM_con->env[0U].TC > (GM_RSSP1_UINT32)(pSFM_con->lifeTime) + pSFM_con->env[0U].sseTC)
					{
						ret = GM_RSSP1_Hash_Search_DstAndIndex((GM_RSSP1_UINT32)pSFM_con->dest_addr, con_index, &ValidIndex, &FreeIndex);
						ret = GM_RSSP1_Hash_Insert_Online(ValidIndex, GM_RSSP1_FALSE);

						pSFM_con->state = GM_RSSP1_Layer_State_Free;
						pSFM_con->remote_dev_AS_state = GM_RSSP1_Remote_DEV_Unknow;
					}
				}
			}
			else
			{
				;/*do nothing*/
			}

			/* 若本周期未收到有效数据，开始进行宽恕检查 proc tolerate cycle */
			if (pSFM_con->b_rcvd_new_dat_in_cycle == GM_RSSP1_TRUE)
			{
				pSFM_con->b_rcvd_new_dat_in_cycle = GM_RSSP1_FALSE;
			}
			else if (GM_RSSP1_TRUE == pSFM_con->b_torlerant_dat_valid)
			{
				/*CR5649 宽恕计数 = 本节点TC与上一次收到有效包TC的差值*/
				pSFM_con->torlerate_cycle_count1 = (GM_RSSP1_UINT8)(pSFM_con->env[0U].TC - pSFM_con->env[0U].local_TC);	/*若宽恕，此差值应：已宽恕周期数< 差值 < 配置的宽恕周期*/

				if ((GM_RSSP1_Layer_State_Connected == pSFM_con->state)
#ifdef RSSP1_TolerateWithCycle
					&& (pSFM_con->torlerate_cycle_count1 > (pSFM_con->torlerated_cycle_count2) )	/*宽恕计数应大于已宽恕的个数，防止节点周期不同于主周期时的重复宽恕*/
#endif
					&& (pSFM_con->torlerate_cycle_count1 < pSFM_con->torlerate_cycle_cfg)/*jcf:包含当前收到数据的周期：若配置3，T周期收到数据，T+1周期开始未接收到数据，则宽恕为T+1，T+2*/
					)
				{
#ifdef RSSP1_TolerateWithCycle
					pSFM_con->torlerated_cycle_count2++;/*已宽恕周期数++，当节点周期与主周期不同时，仅宽恕节点周期对应的次数*/
#endif
					GM_RSSP1_SetPreciTime(pSFM_con);
#ifdef OFFLINE_TOOL_SUPPORT/**该模式下,宽恕数据内的CRC内所带的Ts_l需更新成当前周期**/
					GM_RSSP1_memset((void*)(&pri), 0U, sizeof(GM_RSSP1_SFM_L2U_pri_struct));
					GM_RSSP1_memcpy((void*)(&pri), (const void*)(&(pSFM_con->last_valid_dat_pri)), sizeof(GM_RSSP1_SFM_L2U_pri_struct));
					GM_RSSP1_VSN_Get_Callback_Fun(&VSN0, &VSN1, &VSN2);

					for (i=0U; i<GM_RSSP1_CHECK_CHN_NUM; ++i)
					{
						GM_RSSP1_LFSR_Init(&reg, g_Polyomia[i].pLFSR_left_table, g_Polyomia[i].pLFSR_right_table);
						GM_RSSP1_LFSR_Load(&reg, pSFM_con->env[i].dynLocKey);
						for (j=0U; j<pSFM_con->torlerate_cycle_count1; ++j)
						{
							GM_RSSP1_LFSR_Add(&reg, 0U);						
						}
						if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&reg , &CRCM[i]))
						{
							return;
						}
						pri.as.data_indi.CRC[i] ^= pSFM_con->env[i].dynLocKey ^ pSFM_con->fsfb_chn[i].CON_4[pSFM_con->torlerate_cycle_count1-1] ^ CRCM[i];
					}

					pTmp = &pri;
#else
#ifdef GM_RSSP1_RxMsgWithTime
					GM_RSSP1_memcpy((void*)pSFM_con->last_valid_dat_pri.as.data_indi.byte, (const void*)&(pSFM_con->PreciTime), (size_t)4U);
					GM_RSSP1_memcpy((void*)(pSFM_con->last_valid_dat_pri.as.data_indi.byte+4U), (const void*)&(pSFM_con->SSE_SSR_Delay), (size_t)4U);
#endif
					pTmp = &pSFM_con->last_valid_dat_pri;

#endif
					/* rcv nothing this cycle , but FSFB still connected , tolerate */
					if (GM_RSSP1_TRUE == pSFM_con->b_enable_FSFB_on_chn)
					{
						/* 直接储存到本通道内的队列 */
						write_q_rt = FSFB_Msg_Queue_Write(&(pSFM_con->L2U_dat_Q) , (void*)pTmp);
					}
					else
					{
						/* 存储到SFM的队列 */
						write_q_rt = FSFB_Msg_Queue_Write(&(g_SFM_MQ.RCV_MQ) , (void*)pTmp);
					}
					if (Q_WRITE_OK_FULL == write_q_rt)
					{
						GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate_Write_Full, (int)pSFM_con->index, 0, 0, 0, 0, 0);
						GM_RSSP1_Log_Msg(1,"Inx %d: SFM object/SFM_connection L2U_pri_Q full! Type %d.\n", (int)pSFM_con->index, (int)pSFM_con->b_enable_FSFB_on_chn, 0, 0, 0, 0);
					}
					else if (Q_WRITE_FAIL == write_q_rt)
					{
						GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate_Write_Fail, (int)pSFM_con->index, 0, 0, 0, 0, 0);
						g_SFM_obj.lib_state = GM_RSSP1_Lib_State_Failure;
					}
					else
					{
						; /**do nothing**/
					}
				}
				else
				{
					pSFM_con->b_torlerant_dat_valid = GM_RSSP1_FALSE;
					pSFM_con->torlerate_cycle_count1 = 0U;
#ifdef RSSP1_TolerateWithCycle
					pSFM_con->torlerated_cycle_count2 = 0U;/*已宽恕周期数++，当节点周期与主周期不同时，仅宽恕节点周期对应的次数*/
#endif
					GM_RSSP1_memset((void*)&(pSFM_con->last_valid_dat_pri) , 0U, sizeof(GM_RSSP1_SFM_L2U_pri_struct));
				}
			}
			else
			{
				;/*do nothing*/
			}
		}
	}
}

void GM_RSSP1_SFM_Dispatch_Dat_Indi(GM_RSSP1_SFM_object_struct *pSFM)
{
	GM_RSSP1_UINT8 ConNum = 8U;	/*jcf：修正无法取值的问题*/
	GM_RSSP1_UINT16 i = 0U;
	GM_RSSP1_UINT16 j = 0U;
	GM_RSSP1_UINT16 onlineNum = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_UINT16 other_index = 0U;
	GM_RSSP1_UINT16 indexArray[8] = {0U};
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_connection_struct *pSFM_con = NULL;
	GM_RSSP1_SFM_connection_struct *pSFM_other_con = NULL;
	GM_RSSP1_SFM_connection_struct *pSFM_best_con = NULL;
	GM_RSSP1_SFM_L2U_pri_struct dat_pri;
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;
	GM_RSSP1_BOOL b_got_best_pri = GM_RSSP1_FALSE;

	if ((NULL == pSFM)||(GM_RSSP1_Lib_State_Operational != pSFM->lib_state))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Dispatch_Dat_Indi_Result_No_Match, (int)pSFM, 0, 0, 0, 0, 0);
		return;
	}

	onlineNum = GM_RSSP1_GetSeqAmount(OnLine_Object);
	GM_RSSP1_GetSeqIndexPositive(OnLine_Object, 1U);/**重置正序查询索引**/
	for (i = 0U; i < onlineNum; ++i)/**按online配置链表顺序,获取index**/
	{
		con_index = GM_RSSP1_GetSeqIndexPositive(OnLine_Object, 0U);
		/*if (0xFFFFU != con_index)*/
		if(con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
		{
			pSFM_best_con = NULL;
			b_got_best_pri = GM_RSSP1_FALSE;

			pSFM_con = &(pSFM->connection[con_index]);

			if (GM_RSSP1_TRUE == pSFM_con->b_enable_FSFB_on_chn)
			{
				pSFM_best_con = pSFM_con;

				bRt = GM_RSSP1_Hash_CnfDst_ResArray((GM_RSSP1_UINT32)pSFM_con->dest_addr, indexArray, &ConNum);/**遍历hash表,将对应地址的所有静态index都一次查询出来.注意项目最多只能配4组相同地址信息**/

				if ((GM_RSSP1_TRUE == bRt) && (ConNum > 2U))
				{
					for (j=0U; j<(ConNum/2U); ++j)
					{
						other_index = indexArray[j*2U];

						if (other_index != con_index)
						{
							pSFM_other_con = &(pSFM->connection[other_index]);

							/**最优数据判定条件为是否当前周期收到最新有效数据,如果获取到最优数据则清空其它通道**/
							if ((GM_RSSP1_TRUE != pSFM_con->b_rcvd_new_dat_in_cycle) && (GM_RSSP1_TRUE == pSFM_other_con->b_enable_FSFB_on_chn) && (pSFM_other_con->Chn_apply_FSFB_ID == pSFM_con->Chn_apply_FSFB_ID) && (GM_RSSP1_TRUE == pSFM_other_con->b_rcvd_new_dat_in_cycle))
							{
								pSFM_best_con = pSFM_other_con;
								FSFB_Msg_Queue_Clear(&(pSFM_con->L2U_dat_Q));
								pSFM_con->b_rcvd_new_dat_in_cycle = GM_RSSP1_FALSE;
							}
							else
							{
								FSFB_Msg_Queue_Clear(&(pSFM_other_con->L2U_dat_Q));
								pSFM_other_con->b_rcvd_new_dat_in_cycle = GM_RSSP1_FALSE;
							}
						}
					}
				}

				/* 提取最优的数据 */
				while (GM_RSSP1_TRUE == FSFB_Msg_Queue_Get(&(pSFM_best_con->L2U_dat_Q) , &dat_pri))
				{
					write_q_rt = FSFB_Msg_Queue_Write(&(g_SFM_MQ.RCV_MQ/*pSFM->L2U_pri_Q*/) , &dat_pri);

					if (Q_WRITE_OK_FULL == write_q_rt)
					{
						GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Dispatch_Dat_Indi_Write_Full, 0, 0, 0, 0, 0, 0);
						pSFM->lib_state = GM_RSSP1_Lib_State_Operational;
						GM_RSSP1_Log_Msg(1,"SFM object L2U_pri_Q full!\n", 0, 0, 0, 0, 0, 0);
					}
					else if (Q_WRITE_FAIL == write_q_rt)
					{
						GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Dispatch_Dat_Indi_Write_Fail, 0, 0, 0, 0, 0, 0);
						pSFM->lib_state = GM_RSSP1_Lib_State_Failure;
						break;
					}
					else
					{
						; /**do nothing**/
					}
				}
			}
		}
	}

	pSFM->Is_torlerate_process = GM_RSSP1_FALSE;
}

GM_RSSP1_SFM_Get_Pri_From_CFM_Result_enum GM_RSSP1_SFM_Get_Pri_From_CFM(GM_RSSP1_CFM_L2U_pri_struct *pL2U)
{
	if (NULL == pL2U)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Get_Pri_From_CFM_Point_Error, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_SFM_Get_Pri_INVALID_L2U;
	}

	if (GM_RSSP1_TRUE == FSFB_Msg_Queue_Get(&(g_CFM_MQ.RCV_MQ), pL2U))
	{
		return GM_RSSP1_SFM_Get_Pri_OK;
	}
	else
	{
		return GM_RSSP1_SFM_Get_Pri_FAIL;
	}

}

GM_RSSP1_SFM_Put_Pri_To_CFM_Result_enum GM_RSSP1_SFM_Put_Pri_To_CFM(GM_RSSP1_CFM_U2L_pri_struct *pU2L)
{
	GM_RSSP1_Write_Q_Return_Enum write_rt = Q_WRITE_FAIL;

	if (NULL == pU2L)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Put_Pri_To_CFM_Result_Point_Error, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_SFM_Put_Pri_INVALID_U2L;
	}

	if (GM_RSSP1_CFM_U2L_Unknow == pU2L->type)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Put_Pri_To_CFM_Type_Error, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_SFM_Put_Pri_INVALID_REQ_TYPE;
	}


	write_rt = FSFB_Msg_Queue_Write(&(g_CFM_MQ.SND_MQ) , pU2L);

	if (Q_WRITE_FAIL == write_rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Put_Pri_To_CFM_Write_Fail, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_SFM_Put_Pri_LIB_NOT_OPERATIONAL;
	}
	else if (Q_WRITE_OK_FULL == write_rt)
	{
		/* 成功，但队列已满 */
		GM_RSSP1_Log_Usr(GM_RSSP1_SFM_Put_Pri_To_CFM_Write_Full, 0, 0, 0, 0, 0, 0);
		GM_RSSP1_Log_Msg(1,"CFM U2L_pri_Q FULL!\n ", 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_SFM_Put_Pri_MQ_FULL;
	}
	else
	{
		/* 成功 */
		return GM_RSSP1_SFM_Put_Pri_OK;
	}


}

void GM_RSSP1_SFM_Syn_Con(GM_RSSP1_SFM_object_struct *pSFM)
{
	GM_RSSP1_UINT8 ConNum = 0U;
	GM_RSSP1_UINT16 i = 0U;
	GM_RSSP1_UINT16 j = 0U;
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_UINT16 HashIndex = 0xFFFFU;
	GM_RSSP1_UINT16 ValidConIndex = 0xFFFFU;
	GM_RSSP1_UINT16 ValidIndex = 0xFFFFU;
	GM_RSSP1_UINT16 FreeIndex = 0xFFFFU;
	GM_RSSP1_UINT16 indexArray[8U] = {0U};
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;

	GM_RSSP1_SFM_connection_struct *pSFM_conn = NULL;
	GM_RSSP1_SFM_connection_struct *pSFM_conn_x = NULL;
	if (NULL == pSFM) 
	{
		return;
	}
	if (GM_RSSP1_Lib_State_Operational != pSFM->lib_state)
	{
		return;
	}

	GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 1U);/**重置正序查询索引**/
	for (i = 0U; i < pSFM->connection_nums ; ++i)/**按静态配置链表顺序,获取index**/
	{
		ConNum = 8U;	/*jcf：修正无法取值的问题*/
		GM_RSSP1_memset((void*)indexArray, 0xFFU, sizeof(indexArray));

		con_index = GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 0U);
		/*if (0xFFFF != con_index)*/
		if(con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
		{
			pSFM_conn = &(pSFM->connection[con_index]);
			
			bRt = GM_RSSP1_Hash_Search_DstAndIndex((GM_RSSP1_UINT32)pSFM_conn->dest_addr, con_index, &ValidConIndex, &FreeIndex);/**CR 5764 寻找hash索引值**/
			if (GM_RSSP1_TRUE == bRt)
			{
				bRt = GM_RSSP1_Hash_CnfDst_ResArray((GM_RSSP1_UINT32)pSFM_conn->dest_addr, indexArray, &ConNum);/**遍历hash表,将对应地址的所有静态index都一次查询出来.注意项目最多只能配4组相同地址信息**/
			}

			if ((GM_RSSP1_TRUE == bRt) && (ConNum > 2U))
			{
				for (j=0U; j<(ConNum/2U); ++j)
				{
					l_index = indexArray[j*2U];
					HashIndex = indexArray[j*2U + 1U];

					if (l_index != con_index)
					{
						/**同步原则:以远端主系,接收消息最新为准**/
						pSFM_conn_x = &(pSFM->connection[l_index]);
						if((GM_RSSP1_Remote_Dev_Active== pSFM_conn->remote_dev_AS_state)&&(GM_RSSP1_Remote_Dev_Active!= pSFM_conn_x->remote_dev_AS_state))
						{
							GM_RSSP1_memcpy((void*)pSFM_conn_x->env, (const void*)pSFM_conn->env, sizeof(GM_RSSP1_CHN_ENV)*GM_RSSP1_CHECK_CHN_NUM);
							pSFM_conn_x->state  = pSFM_conn->state;
							/**CR 6950  内部同步下，需要将基准以及历史信息一并同步**/
							pSFM_conn_x->RxSSRTrem  = pSFM_conn->RxSSRTrem;
							pSFM_conn_x->RxSSRTloc  = pSFM_conn->RxSSRTloc;
							pSFM_conn_x->TempDiffTime  = pSFM_conn->TempDiffTime;
							pSFM_conn_x->SSE_SSR_Delay  = pSFM_conn->SSE_SSR_Delay;
							/**end CR**/

						}
						else if((GM_RSSP1_Remote_Dev_Active== pSFM_conn_x->remote_dev_AS_state)&&(GM_RSSP1_Remote_Dev_Active!= pSFM_conn->remote_dev_AS_state))
						{
							GM_RSSP1_memcpy((void*)pSFM_conn->env, (const void*)pSFM_conn_x->env, sizeof(GM_RSSP1_CHN_ENV)*GM_RSSP1_CHECK_CHN_NUM);
							pSFM_conn->state  = pSFM_conn_x->state;
							/**CR 6950  内部同步下，需要将基准以及历史信息一并同步**/
							pSFM_conn->RxSSRTrem  = pSFM_conn_x->RxSSRTrem;
							pSFM_conn->RxSSRTloc  = pSFM_conn_x->RxSSRTloc;
							pSFM_conn->TempDiffTime  = pSFM_conn_x->TempDiffTime;
							pSFM_conn->SSE_SSR_Delay  = pSFM_conn_x->SSE_SSR_Delay;
							/**end CR**/

						}
						else if((GM_RSSP1_Remote_Dev_Active== pSFM_conn_x->remote_dev_AS_state)&&(GM_RSSP1_Remote_Dev_Active== pSFM_conn->remote_dev_AS_state))
						{
							if(pSFM_conn->env[0U].local_TC >= pSFM_conn_x->env[0U].local_TC)
							{
								GM_RSSP1_memcpy((void*)pSFM_conn_x->env, (const void*)pSFM_conn->env, sizeof(GM_RSSP1_CHN_ENV)*GM_RSSP1_CHECK_CHN_NUM);
								pSFM_conn_x->state  = pSFM_conn->state;
								/**CR 6950  内部同步下，需要将基准以及历史信息一并同步**/
								pSFM_conn_x->RxSSRTrem  = pSFM_conn->RxSSRTrem;
								pSFM_conn_x->RxSSRTloc  = pSFM_conn->RxSSRTloc;
								pSFM_conn_x->TempDiffTime  = pSFM_conn->TempDiffTime;
								pSFM_conn_x->SSE_SSR_Delay  = pSFM_conn->SSE_SSR_Delay;
								/**end CR**/

								pSFM_conn_x->remote_dev_AS_state = GM_RSSP1_Remote_DEV_Standby;
								pSFM_conn->remote_dev_AS_state = GM_RSSP1_Remote_Dev_Active;

								if (GM_RSSP1_TRUE == pSFM_conn_x->b_enable_FSFB_on_chn)
								{
									FSFB_Msg_Queue_Clear(&(pSFM_conn_x->L2U_dat_Q));
								}
								pSFM_conn_x->b_torlerant_dat_valid = GM_RSSP1_FALSE;
							}
							else
							{
								GM_RSSP1_memcpy((void*)pSFM_conn->env, (const void*)pSFM_conn_x->env, sizeof(GM_RSSP1_CHN_ENV)*GM_RSSP1_CHECK_CHN_NUM);
								pSFM_conn->state  = pSFM_conn_x->state;
								/**CR 6950  内部同步下，需要将基准以及历史信息一并同步**/
								pSFM_conn->RxSSRTrem  = pSFM_conn_x->RxSSRTrem;
								pSFM_conn->RxSSRTloc  = pSFM_conn_x->RxSSRTloc;
								pSFM_conn->TempDiffTime  = pSFM_conn_x->TempDiffTime;
								pSFM_conn->SSE_SSR_Delay  = pSFM_conn_x->SSE_SSR_Delay;
								/**end CR**/
								pSFM_conn->remote_dev_AS_state = GM_RSSP1_Remote_DEV_Standby;
								pSFM_conn_x->remote_dev_AS_state = GM_RSSP1_Remote_Dev_Active;

								if (GM_RSSP1_TRUE == pSFM_conn->b_enable_FSFB_on_chn)
								{
									FSFB_Msg_Queue_Clear(&(pSFM_conn->L2U_dat_Q));
								}
								pSFM_conn->b_torlerant_dat_valid = GM_RSSP1_FALSE;
							}
						}
						else
						{
							; /*do nothing*/
						}

						/*jcf：CR5594 根据可能被更新后的状态（若有），重新进行con_index的Hash插入，重复插入时若状态一致则会忽略插入*/
						/*flt  CR5764*/
						if( GM_RSSP1_Layer_State_Connected == pSFM_conn_x->state )
						{
							GM_RSSP1_Hash_Insert_Online(HashIndex, GM_RSSP1_TRUE);
						}
						else if(( GM_RSSP1_Layer_State_Free == pSFM_conn_x->state) && (GM_RSSP1_TRUE != (GM_RSSP1_BOOL)pSFM_conn_x->IsFixed))
						{
							GM_RSSP1_Hash_Insert_Online(HashIndex, GM_RSSP1_FALSE);
						}
						else
						{
							; /*nothing*/
						}

						if( GM_RSSP1_Layer_State_Connected == pSFM_conn->state )
						{
							GM_RSSP1_Hash_Insert_Online(ValidConIndex, GM_RSSP1_TRUE);
						}
						else if(( GM_RSSP1_Layer_State_Free == pSFM_conn->state) && (GM_RSSP1_TRUE != (GM_RSSP1_BOOL)pSFM_conn->IsFixed))
						{
							GM_RSSP1_Hash_Insert_Online(ValidConIndex, GM_RSSP1_FALSE);
						}
						else
						{
							; /*nothing*/
						}
					}
				}
			}
		}
	}
}

/*CR:3894，6950 update prectime calculation*/
void GM_RSSP1_SetPreciTime(GM_RSSP1_SFM_connection_struct *pSFM_conn)
{
	GM_RSSP1_UINT16 conn_index = 0U;
	GM_RSSP1_INT32 tmpCalc = 0U;
	GM_RSSP1_UINT32 TremCycle_Diff = 0U;
	GM_RSSP1_UINT32 TlocCycle_Diff = 0U;
	GM_RSSP1_UINT32 l_TcycleRem = 0U;
	GM_RSSP1_UINT32 l_TcycleLoc = 0U;

	if (NULL != pSFM_conn) 
	{
		if ((pSFM_conn->state != GM_RSSP1_Layer_State_Connected) || (pSFM_conn->index >= GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)) /*发送SSE时初始化各项参数*/
		{
			pSFM_conn->PreciTime = -1;
			pSFM_conn->SSE_SSR_Delay = -1;
			pSFM_conn->TempDiffTime = 0;
			pSFM_conn->RxSSRTrem = 0U;
			pSFM_conn->RxSSRTloc = 0U;
		}
		else if ((pSFM_conn->SSE_SSR_Delay == -1) && (pSFM_conn->time.TC == pSFM_conn->RxSSRTloc)) /*当接收到SSR数据包时候，其SSE_SSR_Delay即可计算。且该值在中断之前一直保持有效*/
		{
			pSFM_conn->SSE_SSR_Delay = pSFM_conn->RxSSRTloc - pSFM_conn->env[0].sseTC;
			pSFM_conn->PreciTime = -1;
			pSFM_conn->TempDiffTime = pSFM_conn->TcycleRem + pSFM_conn->TcycleLoc;
		}
		else
		{
			if (pSFM_conn->b_rcvd_new_dat_in_cycle == GM_RSSP1_TRUE)	/*本周期收到RSD消息*/
			{
				TremCycle_Diff = pSFM_conn->SINIT_TremCycle - pSFM_conn->RxSSRTrem;
				TlocCycle_Diff = pSFM_conn->time.TC - pSFM_conn->RxSSRTloc;
				l_TcycleRem = pSFM_conn->TcycleRem;
				l_TcycleLoc = pSFM_conn->TcycleLoc;

				/*每当接收新数据以后，将基准偏移值改为最新数据包*/
				pSFM_conn->RxSSRTrem = pSFM_conn->SINIT_TremCycle;
				pSFM_conn->RxSSRTloc = pSFM_conn->time.TC;

				/**计算更新值为当前偏移+累计偏移的有符号位运算**/
				pSFM_conn->TempDiffTime = pSFM_conn->TempDiffTime + (GM_RSSP1_INT32)(TlocCycle_Diff * l_TcycleLoc) - (GM_RSSP1_INT32)(TremCycle_Diff * l_TcycleRem);

				tmpCalc = pSFM_conn->TempDiffTime + l_TcycleLoc + l_TcycleRem; /*RVS:10113556  calc time recorrect to keep consistency with SwRS */

				if (tmpCalc > 0)
				{
					if (tmpCalc <= (GM_RSSP1_INT32)(0x7FFFFFFFU-l_TcycleLoc))
					{
						pSFM_conn->PreciTime = (tmpCalc + (GM_RSSP1_INT32)(l_TcycleLoc - 1)) / (GM_RSSP1_INT32)l_TcycleLoc;
					}
					else
					{
						pSFM_conn->PreciTime = -1;
					}
				}
				else
				{
					pSFM_conn->PreciTime = 0;
				}	
			}
			else
			{
				pSFM_conn->PreciTime++;
			}
		}
	}
}
#endif
