
#include "GM_RSSP1_CFM.h"
#include "GM_RSSP1_MQ_Interface.h"
#include "GM_RSSP1_Syn.h"

#ifdef CFM_Part

extern GM_RSSP1_VSN_GET_CALLBACK_FUN GM_RSSP1_VSN_Get_Callback_Fun;	/*added by huang 201511219*/

static GM_RSSP1_CFM_object_struct g_CFM_obj;

RSSP1_MQ_LINK_struct g_Link_MQ;

GM_RSSP1_UINT8 MQ_SEQ_METHOD = 0U;

extern RSSP1_MQ_Inter_struct g_CFM_MQ;
RSSP1_RCV_LINK_struct RecordPkt;
GM_RSSP1_UINT32 RecordTC[3U] = {0U};

GM_RSSP1_CFM_State_Index_struct g_CFM_Status;
#ifdef	CFM_Stanby_Answer
extern GM_RSSP1_Msg_Queue_struct g_CFM_Debug_MQ;
extern GM_RSSP1_BOOL GM_RSSP1_APP_Interface_Get_Syn_AB_AS_Info(Local_Sys_AB_enum *p_sys_a_b , GM_RSSP1_BOOL *p_bactive);
#endif

GM_RSSP1_CFM_object_struct *GM_RSSP1_Get_CFM_Object(void)
{
	return &g_CFM_obj;
}

GM_RSSP1_library_state_enum GM_RSSP1_CFM_Get_Library_Status(GM_RSSP1_CFM_object_struct *pCFM)
{
	if (NULL == pCFM)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Get_Library_Status_Param_Point_Error, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_Lib_State_Failure;
	}

	return pCFM->lib_state;
}

GM_RSSP1_BOOL GM_RSSP1_CFM_Is_SaCEPID_Equal(GM_RSSP1_SACEPID_struct *pSaCEPID1 , GM_RSSP1_SACEPID_struct *pSaCEPID2)
{
	if ((NULL == pSaCEPID1) || (NULL == pSaCEPID2))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Is_SaCEPID_Equal_Param_Point_Error, (int)pSaCEPID1, (int)pSaCEPID2, 0, 0, 0, 0);
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


GM_RSSP1_CFM_connection_struct *GM_RSSP1_CFM_Get_Connection_BY_SaCEPID(GM_RSSP1_SACEPID_struct *pSaCEPID)
{
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_CFM_connection_struct *pCFM_conn   = NULL;

	if (NULL == pSaCEPID)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Get_Connection_BY_SaCEPID_Param_Point_Error, (int)pSaCEPID, 0, 0, 0, 0, 0);
		return NULL;
	}

	if (GM_RSSP1_Lib_State_Operational != g_CFM_obj.lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Get_Connection_BY_SaCEPID_State_Fail, (int)g_CFM_obj.lib_state, 0, 0, 0, 0, 0);
		return NULL;
	}

	GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 1U);

	for (l_index = 0U; l_index < g_CFM_obj.connection_nums ; ++l_index)
	{
		con_index = GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 0U);
		/*if (0xFFFF != con_index)*/
		if(con_index <	(GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
		{
			pCFM_conn = &(g_CFM_obj.connection[con_index]);

			if (GM_RSSP1_TRUE == GM_RSSP1_CFM_Is_SaCEPID_Equal(pSaCEPID , &(pCFM_conn->SaCEPID)))
			{
				return pCFM_conn;
			}
		}
	}

	return NULL;
}


void GM_RSSP1_CFM_Proc_SFM_Req(void)
{
	GM_RSSP1_CFM_connection_struct *pCFM_con = NULL;
	GM_RSSP1_CFM_U2L_pri_struct pri;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
#ifdef GM_RSSP1_ENABLE_CRSCD_PACK
	GM_RSSP1_UINT16 rsd_pri_num[GM_RSSP1_MAX_ONLINE_CONNECTION_NUM] = {0};
	GM_RSSP1_UINT16 sse_pri_num[GM_RSSP1_MAX_ONLINE_CONNECTION_NUM] = {0};
	GM_RSSP1_UINT16 ssr_pri_num[GM_RSSP1_MAX_ONLINE_CONNECTION_NUM] = {0};
	GM_RSSP1_CFM_U2L_pri_struct RSD_pri[GM_RSSP1_MAX_ONLINE_CONNECTION_NUM][GM_RSSP1_MAX_PKT_NUM_PER_CYCLE] = {0};
	GM_RSSP1_CFM_U2L_Dat_SSE_pri_struct SSE_pri[GM_RSSP1_MAX_ONLINE_CONNECTION_NUM][GM_RSSP1_MAX_PKT_NUM_PER_CYCLE] = {0};
	GM_RSSP1_CFM_U2L_Dat_SSR_pri_struct SSR_pri[GM_RSSP1_MAX_ONLINE_CONNECTION_NUM][GM_RSSP1_MAX_PKT_NUM_PER_CYCLE] = {0};
	GM_RSSP1_UINT8 combined_pkt_buf[GM_RSSP1_MAX_ONLINE_CONNECTION_NUM][GM_RSSP1_MAX_PKT_NUM_PER_CYCLE * GM_RSSP1_MAX_SAFETY_DAT_LEN] = {0};
	GM_RSSP1_UINT16 combined_pkt_len[GM_RSSP1_MAX_ONLINE_CONNECTION_NUM] = {0};
	GM_RSSP1_UINT16 pri_index = 0U;
	GM_RSSP1_UINT16 index = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_UINT8 frame_type = 0U;
#endif

	if (GM_RSSP1_Lib_State_Operational != g_CFM_obj.lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_SFM_Req_State_Fail, (int)g_CFM_obj.lib_state, 0, 0, 0, 0, 0);
		return;
	}

	while (GM_RSSP1_TRUE == FSFB_Msg_Queue_Get(&(g_CFM_MQ.SND_MQ) , &pri))
	{
		if (GM_RSSP1_CFM_U2L_Unknow != pri.type)
		{
			pCFM_con = &(g_CFM_obj.connection[pri.index]);

			if (GM_RSSP1_TRUE == pCFM_con->b_enable_CRSCD_pack)
			{
				/* 设计院方案：多包组合发送。RSD一直放在最前面，SSE、SSR紧接着RSD后面.SSE和SSR顺序没有要求
				* 此处，只对CRSCD连接进行FSFB小数据包的提取和暂存，暂不发送  */
#ifdef GM_RSSP1_ENABLE_CRSCD_PACK
				switch (pri.type)
				{

				case GM_RSSP1_CFM_Data_req:
					frame_type = pri.data_req.byte[1];

					if ((RSSP1_RSD_FRAME_TYPE_FROM_A == frame_type) || (RSSP1_RSD_FRAME_TYPE_FROM_B == frame_type))
					{
						RSD_pri[pCFM_con->index][rsd_pri_num[pCFM_con->index]] = pri;
						++rsd_pri_num[pCFM_con->index];
					}
					else if (RSSP1_SSE_FRAME_TYPE == frame_type)
					{
						SSE_pri[pCFM_con->index][sse_pri_num[pCFM_con->index]].type         = pri.type;
						SSE_pri[pCFM_con->index][sse_pri_num[pCFM_con->index]].TC           = pri.data_req.TC;
						SSE_pri[pCFM_con->index][sse_pri_num[pCFM_con->index]].bytes_count  = pri.data_req.bytes_count;

						if (RSSP1_SSE_FRAME_LEN_WITH_CRC == pri.data_req.bytes_count)
						{
							GM_RSSP1_memcpy((void*)SSE_pri[pCFM_con->index][sse_pri_num[pCFM_con->index]].byte , (const void*)pri.data_req.byte , (size_t)pri.data_req.bytes_count);
							++sse_pri_num[pCFM_con->index];
						}
						else
						{
							continue;
						}
					}
					else if (RSSP1_SSR_FRAME_TYPE == frame_type)
					{
						SSR_pri[pCFM_con->index][ssr_pri_num[pCFM_con->index]].type         = pri.type;
						SSR_pri[pCFM_con->index][ssr_pri_num[pCFM_con->index]].TC           = pri.data_req.TC;
						SSR_pri[pCFM_con->index][ssr_pri_num[pCFM_con->index]].bytes_count  = pri.data_req.bytes_count;

						if (RSSP1_SSR_FRAME_LEN_WITH_CRC == pri.data_req.bytes_count)
						{
							GM_RSSP1_memcpy((void*)SSR_pri[pCFM_con->index][ssr_pri_num[pCFM_con->index]].byte , (const void*)pri.data_req.byte , (size_t)pri.data_req.bytes_count);
							++ssr_pri_num[pCFM_con->index];
						}
						else
						{
							continue;
						}
					}
					else
					{
					}

					break;

				default:
					break;
				}
#endif
			}
			else
			{
				/* 标准单包发送 */
				switch (pri.type)
				{
				case GM_RSSP1_CFM_Data_req:
					GM_RSSP1_CFM_Send_Dat(pri.data_req.byte , pri.data_req.bytes_count, pri.index);
					break;			
				default:
					/*do nothing*/
					break;
				}
			}
		}
	}
#ifdef GM_RSSP1_ENABLE_CRSCD_PACK
	/* send combined pkts */
	GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 1U);
	for (index = 0U ; index < g_CFM_obj.connection_nums ; ++index)
	{
		con_index = GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 0U);
		if (0xFFFF != con_index)
		{
			pCFM_con = &(g_CFM_obj.connection[con_index]);

			if (GM_RSSP1_TRUE == pCFM_con->b_enable_CRSCD_pack)
			{
				combined_pkt_len[con_index] = 0U;

				/* RSD在最前面 */

				for (pri_index = 0U ; pri_index < rsd_pri_num[con_index] ; ++pri_index)
				{
					GM_RSSP1_memcpy((void*)(combined_pkt_buf[con_index] + combined_pkt_len[con_index]), (const void*)RSD_pri[con_index][pri_index].data_req.byte , (size_t)RSD_pri[con_index][pri_index].data_req.bytes_count);
					combined_pkt_len[con_index] += RSD_pri[con_index][pri_index].data_req.bytes_count;
				}

				/* SSE */
				for (pri_index = 0U ; pri_index < sse_pri_num[con_index] ; ++pri_index)
				{
					GM_RSSP1_memcpy((void*)(combined_pkt_buf[con_index] + combined_pkt_len[con_index]), (const void*)SSE_pri[con_index][pri_index].byte , (size_t)SSE_pri[con_index][pri_index].bytes_count);
					combined_pkt_len[con_index]  += SSE_pri[con_index][pri_index].bytes_count;
				}

				/* SSR */
				for (pri_index = 0U ; pri_index < ssr_pri_num[con_index] ; ++pri_index)
				{
					GM_RSSP1_memcpy((void*)(combined_pkt_buf[con_index] + combined_pkt_len[con_index]), (const void*)SSR_pri[con_index][pri_index].byte , (size_t)SSR_pri[con_index][pri_index].bytes_count);
					combined_pkt_len[con_index]  += SSR_pri[con_index][pri_index].bytes_count;
				}

				if (combined_pkt_len[con_index] > 0U)
				{
					GM_RSSP1_CFM_Send_Dat(combined_pkt_buf[con_index] , combined_pkt_len[con_index], con_index);
				}
			}
		}
	}
#endif
	return;
}

GM_RSSP1_BOOL GM_RSSP1_CFM_Send_Dat(GM_RSSP1_UINT8 *pDat , GM_RSSP1_UINT16 dat_len , GM_RSSP1_UINT16 Inx)	/*CR:GM00004889 删除多余形参index*/
{
	GM_RSSP1_INT32 chn_index = 0 ;
	GM_RSSP1_com_out_pri_struct send_pri ;
	GM_RSSP1_Write_Q_Return_Enum q_write_rt = Q_WRITE_FAIL;
	GM_RSSP1_CFM_connection_struct *pCFM_con = NULL;

	GM_RSSP1_memset((void*)&send_pri, 0U, sizeof(GM_RSSP1_com_out_pri_struct));

	if ((dat_len > GM_RSSP1_MAX_SND_DAT_LEN) || (NULL == pDat) || (Inx >=GM_RSSP1_MAX_SAFETY_CONNECTION_NUM))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Send_Dat_Param_Val_Error, dat_len, (int)pDat, (int)Inx, (int)GM_RSSP1_MAX_SND_DAT_LEN, (int)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM, 0);
		return GM_RSSP1_FALSE;
	}

	pCFM_con = &(g_CFM_obj.connection[Inx]);

	if (GM_RSSP1_Lib_State_Operational != g_CFM_obj.lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Send_Dat_State_Fail, g_CFM_obj.lib_state, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	
	GM_RSSP1_memcpy((void*)send_pri.byte , (const void*)pDat , (size_t)dat_len);

	send_pri.bytes_count = dat_len;

	for (chn_index = 0 ; chn_index < (GM_RSSP1_INT32)pCFM_con->chn_num ; ++chn_index)
	{
		send_pri.index = pCFM_con->index;
		send_pri.subindex = (GM_RSSP1_UINT8)chn_index;
		send_pri.r_ip = pCFM_con->com_chn[chn_index].rem_ip;
		send_pri.r_port = pCFM_con->com_chn[chn_index].rem_port;

		q_write_rt = FSFB_Msg_Queue_Write(&(g_Link_MQ.SND_MQ) , (void*)&send_pri);

		if (Q_WRITE_OK_FULL == q_write_rt)
		{
			GM_RSSP1_Log_Msg(2,"Link Queue full!\n", (int)pCFM_con->index, (int)chn_index, 0, 0, 0, 0);
		}
		else if (Q_WRITE_FAIL == q_write_rt)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Send_Dat_Write_Fail, pCFM_con->index, chn_index, 0, 0, 0, 0);
			g_CFM_obj.lib_state = GM_RSSP1_Lib_State_Failure;
			return GM_RSSP1_FALSE;
		}
		else
		{
			; /*do nothing*/
		}



	}

	return GM_RSSP1_TRUE;
}

GM_RSSP1_BOOL GM_RSSP1_CFM_User_Send_Dat(GM_RSSP1_SACEPID_struct *psacepid , GM_RSSP1_UINT8 *pDat , GM_RSSP1_UINT16 len)
{
	GM_RSSP1_CFM_U2L_pri_struct cfm_pri;
	GM_RSSP1_CFM_connection_struct *pCFM_con = NULL;
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;
	GM_RSSP1_BOOL ret = GM_RSSP1_FALSE;

	if ((NULL == psacepid) || (NULL == pDat) || (len > sizeof(cfm_pri.data_req.byte)))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_User_Send_Dat_Param_Val_Error, (int)psacepid, (int)pDat, (int)len, (int)sizeof(cfm_pri.data_req.byte), 0, 0);
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_Lib_State_Operational != g_CFM_obj.lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_User_Send_Dat_State_Fail, g_CFM_obj.lib_state, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	pCFM_con = GM_RSSP1_CFM_Get_Connection_BY_SaCEPID(psacepid);
	if (NULL == pCFM_con)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_User_Send_Dat_Search_Fail, psacepid->dest_addr, psacepid->source_addr, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	cfm_pri.type             = GM_RSSP1_CFM_Data_req;
	cfm_pri.index            = pCFM_con->index;
	cfm_pri.data_req.type    = GM_RSSP1_MSG_TYPE_RSD;
	cfm_pri.data_req.TC      = 0U;

	GM_RSSP1_memcpy((void*)cfm_pri.data_req.byte , (const void*)pDat , (size_t)len);

	cfm_pri.data_req.bytes_count = len;


#if defined(GM_RSSP1_SAVING_MODE)
	ret = GM_RSSP1_CFM_Send_Dat(cfm_pri.data_req.byte , cfm_pri.data_req.bytes_count, cfm_pri.index);
#else
	write_q_rt = FSFB_Msg_Queue_Write(&(g_CFM_MQ.SND_MQ) , &cfm_pri);
	if (Q_WRITE_FAIL == write_q_rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_User_Send_Dat_Write_Fail, pCFM_con->index, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	else if (Q_WRITE_OK_FULL == write_q_rt)
	{
		/* 成功，但队列已满 */
		GM_RSSP1_Log_Msg(2,"CFM U2L_pri_Q FULL!\n ", (int)pCFM_con->index, 0, 0, 0, 0, 0);
	}
	else
	{
		; /*do nothing*/
	}
	return GM_RSSP1_TRUE;
#endif
}

GM_RSSP1_BOOL GM_RSSP1_CFM_LatestUnderPack(GM_RSSP1_com_input_pri_struct* rcvd_dat_pri) 
{
	GM_RSSP1_UINT8 MsgType = 0xFFU; 
	GM_RSSP1_UINT16 crc16  = 0U;
	GM_RSSP1_INT32 pkt_len = 0;
	GM_RSSP1_UINT32 i = 0U;
	GM_RSSP1_UINT32 rcvd_TC = 0U;
	GM_RSSP1_INT32 rsd_user_dat_len = 0;
	GM_RSSP1_CFM_L2U_pri_struct L2U_pri;
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;
	GM_RSSP1_BOOL breakFlag = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;

	if (rcvd_dat_pri != NULL)
	{
		for (i = 0U; i < rcvd_dat_pri->bytes_count;i++)
		{
			if ((RSSP1_RSD_FRAME_TYPE_FROM_A == rcvd_dat_pri->byte[i + 1U]) || (RSSP1_RSD_FRAME_TYPE_FROM_B == rcvd_dat_pri->byte[i + 1U]))
			{
				/* find a RSD */
				MsgType = 0U;
				rsd_user_dat_len = (GM_RSSP1_INT32)(((GM_RSSP1_UINT16)rcvd_dat_pri->byte[i + 10U]) + ((GM_RSSP1_UINT16)(rcvd_dat_pri->byte[i + 11U]) << 8U) - 8U);
				pkt_len = (GM_RSSP1_INT32)RSSP1_RSD_FRAME_LEN_WITHOUT_USERDAT + rsd_user_dat_len;
			}
			else if (RSSP1_SSE_FRAME_TYPE == rcvd_dat_pri->byte[i + 1U])
			{
				/* find a SSE */
				MsgType = 1U;
				pkt_len = (GM_RSSP1_INT32)RSSP1_SSE_FRAME_LEN_WITH_CRC;
			}
			else if (RSSP1_SSR_FRAME_TYPE == rcvd_dat_pri->byte[i + 1U])
			{
				/* find a SSR */
				MsgType = 2U;
				pkt_len = (GM_RSSP1_INT32)RSSP1_SSR_FRAME_LEN_WITH_CRC;
			}
			else
			{
				/* unknown pkt */
				GM_RSSP1_Log_Msg(2,"CFM recvd unknown pkt,type:%d.\n", (int)rcvd_dat_pri->byte[i + 1U], 0, 0, 0, 0, 0);
				continue;
			}

			if (pkt_len > 0) /* defect: 11515538 */
			{
				rcvd_TC = (GM_RSSP1_UINT32)(rcvd_dat_pri->byte[i+6U]) + ((GM_RSSP1_UINT32)(rcvd_dat_pri->byte[i+7U]) << 8U) + ((GM_RSSP1_UINT32)(rcvd_dat_pri->byte[i+8U]) << 16U) + ((GM_RSSP1_UINT32)(rcvd_dat_pri->byte[i+9U]) << 24U);

				if (rcvd_TC > RecordTC[MsgType])
				{
#ifdef SFM_Part
					GM_RSSP1_CRC_Calculate_CRC16(rcvd_dat_pri->byte + i, (GM_RSSP1_UINT32)pkt_len, &crc16, g_Polyomia[0U].pCRC16_table);
#endif
					if (crc16 == 0U)
					{
						RecordTC[MsgType] = rcvd_TC;
#ifdef  GM_RSSP1_SAVING_MODE
						GM_RSSP1_SFM_Process_RxData(GM_RSSP1_CFM_Data_Ind, rcvd_dat_pri->index, rcvd_dat_pri->byte + i, pkt_len);
#else
						/* 每提取一包数据就向上报告一次 */
						L2U_pri.index   = rcvd_dat_pri->index;
						L2U_pri.type    = GM_RSSP1_CFM_Data_Ind;
						L2U_pri.as.data_indi.bytes_count    = (GM_RSSP1_UINT16)pkt_len;
						GM_RSSP1_memcpy((void*)L2U_pri.as.data_indi.byte ,  (const void*)(rcvd_dat_pri->byte + i) , (size_t)pkt_len);

						write_q_rt = FSFB_Msg_Queue_Write(&(g_CFM_MQ.RCV_MQ), (void*)(&L2U_pri));

						if (Q_WRITE_OK_FULL == write_q_rt)
						{
							GM_RSSP1_Log_Msg(2,"CFM L2U_pri_Q full(CRSCD)!\n", (int)rcvd_dat_pri->index, 0, 0, 0, 0, 0);
							GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Full, rcvd_dat_pri->index, 0, 0, 0, 0, 0);
						}
						else if (Q_WRITE_FAIL == write_q_rt)
						{
							GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Fail, rcvd_dat_pri->index, 0, 0, 0, 0, 0);
							return GM_RSSP1_FALSE;
						}
						else
						{
							; /*do nothing*/
						}
#endif
						rcvd_TC = 0U;
						i += (GM_RSSP1_UINT32)pkt_len;
						i -= 1U;	/*CR:GM00005252 与for循环中的i++抵消，保证顺序*/
						pkt_len = 0;
						bRt = GM_RSSP1_TRUE;
					}
				}
				else
				{
					GM_RSSP1_Log_Msg(2,"TC is invalid, rcvd_TC=%d not bigger than RecordTC=%d\n", (int)rcvd_TC, (int)RecordTC[MsgType], 0, 0, 0, 0);
					breakFlag = GM_RSSP1_TRUE;	/*CR:GM00005252 若时效检查失败则直接退出，防止因后续的不可控遍历导致内存越界*/
				}
			}
			else
			{
				breakFlag = GM_RSSP1_TRUE;
			}

			if(GM_RSSP1_TRUE == breakFlag)
			{
				break;
			}
		}

		/*由于同步不在组合包中,单独判断*/
		if (GM_RSSP1_FALSE == bRt)
		{
			/*CR:GM00004936、5777 添加对同步包的识别*/
			if((INTERFACE_TYPE_SYN == rcvd_dat_pri->byte[0])&&(
				((0xFFU == rcvd_dat_pri->byte[4U])&&(0xFFU == rcvd_dat_pri->byte[5U])&&(INTERFACE_DAT_TYPE_SYN_RSSP1_STATUS_DAT == rcvd_dat_pri->byte[1U])) 
				|| (INTERFACE_DAT_TYPE_SYN_RSSP1_ACTIVE_DAT == rcvd_dat_pri->byte[1U])))
			{
#ifdef  GM_RSSP1_SAVING_MODE
				GM_RSSP1_SFM_Process_RxData(GM_RSSP1_CFM_Data_Ind, rcvd_dat_pri->index, rcvd_dat_pri->byte, rcvd_dat_pri->bytes_count);
				bRt = GM_RSSP1_TRUE;
#else
				/* find a Syn */
				L2U_pri.index   = rcvd_dat_pri->index;
				L2U_pri.type    = GM_RSSP1_CFM_Data_Ind;
				L2U_pri.as.data_indi.bytes_count    = rcvd_dat_pri->bytes_count;
				GM_RSSP1_memcpy((void*)L2U_pri.as.data_indi.byte , (const void*)rcvd_dat_pri->byte , (size_t)rcvd_dat_pri->bytes_count);

				write_q_rt = FSFB_Msg_Queue_Write(&(g_CFM_MQ.RCV_MQ) , &L2U_pri);
				if (Q_WRITE_OK_FULL == write_q_rt)
				{
					GM_RSSP1_Log_Msg(2,"CFM L2U_pri_Q full(CRSCD)!\n", (int)rcvd_dat_pri->index, 0, 0, 0, 0, 0);
					GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Full, rcvd_dat_pri->index, 0, 0, 0, 0, 0);
					bRt = GM_RSSP1_TRUE;
				}
				else if (Q_WRITE_FAIL == write_q_rt)
				{
					GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Fail, rcvd_dat_pri->index, 0, 0, 0, 0, 0);
				}
				else
				{
					bRt = GM_RSSP1_TRUE; /*do nothing*/
				}
#endif
			}
		}
	}

	return bRt;
}






GM_RSSP1_BOOL GM_RSSP1_CFM_LatestAlone(GM_RSSP1_com_input_pri_struct* rcvd_dat_pri) 
{
	GM_RSSP1_UINT8 MsgType = 0xFFU;
	GM_RSSP1_UINT16 crc16  = 0U;
	GM_RSSP1_UINT32 rcvd_TC = 0U;
	GM_RSSP1_CFM_L2U_pri_struct L2U_pri;
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;
	
	if (rcvd_dat_pri != NULL)
	{
		if ((RSSP1_RSD_FRAME_TYPE_FROM_A == rcvd_dat_pri->byte[1U]) || (RSSP1_RSD_FRAME_TYPE_FROM_B == rcvd_dat_pri->byte[1U]))
		{
			/* find a RSD */
			MsgType = 0U;
		}
		else if (RSSP1_SSE_FRAME_TYPE == rcvd_dat_pri->byte[1U])
		{
			/* find a SSE */
			MsgType = 1U;
		}
		else if (RSSP1_SSR_FRAME_TYPE == rcvd_dat_pri->byte[1U])
		{
			/* find a SSR */
			MsgType = 2U;
		}
		/*CR:GM00004936、5777 添加对同步包的识别*/
		else if((INTERFACE_TYPE_SYN == rcvd_dat_pri->byte[0U])&& (
			((0xFFU == rcvd_dat_pri->byte[4U])&&(0xFFU == rcvd_dat_pri->byte[5U])&&(INTERFACE_DAT_TYPE_SYN_RSSP1_STATUS_DAT == rcvd_dat_pri->byte[1U])) 
			|| (INTERFACE_DAT_TYPE_SYN_RSSP1_ACTIVE_DAT == rcvd_dat_pri->byte[1U])))
		{
#ifdef  GM_RSSP1_SAVING_MODE
			GM_RSSP1_SFM_Process_RxData(GM_RSSP1_CFM_Data_Ind, rcvd_dat_pri->index, rcvd_dat_pri->byte, rcvd_dat_pri->bytes_count);

#else
			/* find a Syn */
			L2U_pri.index   = rcvd_dat_pri->index;
			L2U_pri.type    = GM_RSSP1_CFM_Data_Ind;
			L2U_pri.as.data_indi.bytes_count    = rcvd_dat_pri->bytes_count;
			GM_RSSP1_memcpy((void*)L2U_pri.as.data_indi.byte , (const void*)rcvd_dat_pri->byte , (size_t)rcvd_dat_pri->bytes_count);
			write_q_rt = FSFB_Msg_Queue_Write(&(g_CFM_MQ.RCV_MQ) , &L2U_pri);
			if (Q_WRITE_OK_FULL == write_q_rt)
			{
				GM_RSSP1_Log_Msg(2,"CFM L2U_pri_Q full(CRSCD)!\n", (int)rcvd_dat_pri->index, 0, 0, 0, 0, 0);
				GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Full, rcvd_dat_pri->index, 0, 0, 0, 0, 0);
			}
			else if (Q_WRITE_FAIL == write_q_rt)
			{
				GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Fail, rcvd_dat_pri->index, 0, 0, 0, 0, 0);
				return GM_RSSP1_FALSE;
			}
			else
			{
				; /*do nothing*/
			}
#endif
			return GM_RSSP1_TRUE;
		}
		else
		{
			/* unknown pkt */
			GM_RSSP1_Log_Msg(2,"CFM ID:0x%X UDP socket %d recvd unknown pkt,type:%d.\n", (int)rcvd_dat_pri->byte[1U], 0, 0, 0, 0, 0);	/*CR：GM00004935 单包下无i，将byte[i+1]改为byte[1]*/
			return GM_RSSP1_FALSE;
		}

		/* TODO: 如何挑选最合适包待定 */
		if (rcvd_dat_pri->bytes_count > GM_RSSP1_MAX_SAFETY_DAT_LEN)
		{
			GM_RSSP1_Log_Msg(2,"CFM ID:0x%X UDP socket %d recvd %d bytes,exceed GM_RSSP1_MAX_SAFETY_DAT_LEN: %d.\n", (int)rcvd_dat_pri->bytes_count, (int)GM_RSSP1_MAX_SAFETY_DAT_LEN, 0, 0, 0, 0);
		}
		else
		{
			rcvd_TC = (GM_RSSP1_UINT32)(rcvd_dat_pri->byte[6U]) + ((GM_RSSP1_UINT32)(rcvd_dat_pri->byte[7U]) << 8U) + ((GM_RSSP1_UINT32)(rcvd_dat_pri->byte[8U]) << 16U) + ((GM_RSSP1_UINT32)(rcvd_dat_pri->byte[9U]) << 24U);

			if (rcvd_TC > RecordTC[MsgType])/* defect: 11515538 */
			{
				/* TODO:CRC检验 */
#ifdef SFM_Part
				GM_RSSP1_CRC_Calculate_CRC16(rcvd_dat_pri->byte, (GM_RSSP1_UINT32)rcvd_dat_pri->bytes_count, &crc16, g_Polyomia[0U].pCRC16_table);
#endif
				if (crc16 == 0U)
				{
#ifdef  GM_RSSP1_SAVING_MODE
					GM_RSSP1_SFM_Process_RxData(GM_RSSP1_CFM_Data_Ind, rcvd_dat_pri->index, rcvd_dat_pri->byte, rcvd_dat_pri->bytes_count);

#else
					RecordTC[MsgType] = rcvd_TC;
					L2U_pri.index   = rcvd_dat_pri->index;
					L2U_pri.type    = GM_RSSP1_CFM_Data_Ind;
					L2U_pri.as.data_indi.bytes_count    = rcvd_dat_pri->bytes_count;
					GM_RSSP1_memcpy((void*)L2U_pri.as.data_indi.byte , (const void*)rcvd_dat_pri->byte , (size_t)rcvd_dat_pri->bytes_count);
					write_q_rt = FSFB_Msg_Queue_Write(&(g_CFM_MQ.RCV_MQ) , &L2U_pri);

					if (Q_WRITE_OK_FULL == write_q_rt)
					{
						GM_RSSP1_Log_Msg(2,"CFM L2U_pri_Q full!\n", (int)rcvd_dat_pri->index, 0, 0, 0, 0, 0);
						GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Full, rcvd_dat_pri->index, 0, 0, 0, 0, 0);
					}
					else if (Q_WRITE_FAIL == write_q_rt)
					{
						GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Fail, rcvd_dat_pri->index, 0, 0, 0, 0, 0);
						return GM_RSSP1_FALSE;
					}
					else
					{
						; /*do nothing*/
					}
#endif
				}
			}
		}

		return GM_RSSP1_TRUE;
	}
	else
	{
		return GM_RSSP1_FALSE;
	}
}







GM_RSSP1_BOOL GM_RSSP1_CFM_Proc_Con_DatWithLatest(GM_RSSP1_UINT16 index, GM_RSSP1_BOOL bLock) 
{
	GM_RSSP1_INT32 chn_index = 0;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_com_input_pri_struct rcvd_dat_pri;
	GM_RSSP1_CFM_connection_struct *pCFM_con = NULL;

	bRt = GM_RSSP1_TRUE;

	GM_RSSP1_memset((void*)&RecordTC[0], 0U, sizeof(RecordTC));

	for (chn_index = 0 ; chn_index < (GM_RSSP1_INT32)GM_RSSP1_MAX_LINK_CHANNEL ; ++chn_index)
	{
		while (GM_RSSP1_TRUE == FSFB_Msg_Queue_Get(&(g_Link_MQ.RCV_MQ[index][chn_index]), &rcvd_dat_pri))
		{
			if (rcvd_dat_pri.index < GM_RSSP1_MAX_SAFETY_CONNECTION_NUM )
			{
				pCFM_con = &(g_CFM_obj.connection[rcvd_dat_pri.index]);

				if (rcvd_dat_pri.bytes_count > GM_RSSP1_MAX_SAFETY_DAT_LEN)
				{
					GM_RSSP1_Log_Msg(2,"CFM ID:0x%X UDP socket %d recvd %d bytes,exceed GM_RSSP1_MAX_SAFETY_DAT_LEN: %d.\n", (int)pCFM_con->SaCEPID.source_addr , (int)chn_index , (int)rcvd_dat_pri.bytes_count, (int)GM_RSSP1_MAX_SAFETY_DAT_LEN, 0, 0);
					continue;
				}

				if (GM_RSSP1_TRUE ==  bLock)
				{
					g_CFM_Status.IndexArry[g_CFM_Status.netNum] = rcvd_dat_pri.index;
					g_CFM_Status.netNum ++;
					bLock = GM_RSSP1_FALSE;
				}

				pCFM_con->com_chn[chn_index].b_connected = GM_RSSP1_TRUE;
				/* CRSCD方案，收到的一个UDP数据包可能是多个FSFB包合并而成 */
				if (GM_RSSP1_TRUE == pCFM_con->b_enable_CRSCD_pack)
				{
					bRt = GM_RSSP1_CFM_LatestUnderPack(&rcvd_dat_pri);
				}
				else/* 每个完整的UDP数据包只是单个FSFB包 */
				{
					bRt = GM_RSSP1_CFM_LatestAlone(&rcvd_dat_pri);
				}
			}
		}
	}

	return GM_RSSP1_TRUE;
}








GM_RSSP1_BOOL GM_RSSP1_CFM_ReSeqUnderPack(GM_RSSP1_com_input_pri_struct* rcvd_dat_pri) 
{
	GM_RSSP1_UINT8 MsgType = 0U;
	GM_RSSP1_UINT16 i = 0U;
	GM_RSSP1_UINT16 crc16 = 0U;
	GM_RSSP1_INT32 pkt_len = 0;
	GM_RSSP1_UINT32 rcvd_TC = 0U;
	GM_RSSP1_INT32 rsd_user_dat_len  = 0;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_CFM_L2U_pri_struct L2U_pri;
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;

	if (rcvd_dat_pri != NULL)
	{
		for (i = 0U; i < rcvd_dat_pri->bytes_count;i++)
		{
			if ((RSSP1_RSD_FRAME_TYPE_FROM_A == rcvd_dat_pri->byte[i + 1U]) || (RSSP1_RSD_FRAME_TYPE_FROM_B == rcvd_dat_pri->byte[i + 1U]))
			{
				/* find a RSD */
				MsgType = 0U;
				rsd_user_dat_len = (GM_RSSP1_INT32)(((GM_RSSP1_UINT16)rcvd_dat_pri->byte[i + 10U]) + ((GM_RSSP1_UINT16)(rcvd_dat_pri->byte[i + 11U]) << 8U) - 8U);
				pkt_len = (GM_RSSP1_INT32)RSSP1_RSD_FRAME_LEN_WITHOUT_USERDAT + rsd_user_dat_len;
			}
			else if (RSSP1_SSE_FRAME_TYPE == rcvd_dat_pri->byte[i + 1U])
			{
				/* find a SSE */
				MsgType = 1U;
				pkt_len = (GM_RSSP1_INT32)RSSP1_SSE_FRAME_LEN_WITH_CRC;
			}
			else if (RSSP1_SSR_FRAME_TYPE == rcvd_dat_pri->byte[i + 1U])
			{
				/* find a SSR */
				MsgType = 2U;
				pkt_len = (GM_RSSP1_INT32)RSSP1_SSR_FRAME_LEN_WITH_CRC;
			}
			else
			{
				/* unknown pkt */
				GM_RSSP1_Log_Msg(2,"CFM recvd unknown pkt,type:%d.\n", (int)rcvd_dat_pri->byte[i + 1U], 0, 0, 0, 0, 0);
				continue;
			}

			if (pkt_len > 0)
			{
				rcvd_TC = (GM_RSSP1_UINT32)(rcvd_dat_pri->byte[i+6U]) + ((GM_RSSP1_UINT32)(rcvd_dat_pri->byte[i+7U]) << 8U) + ((GM_RSSP1_UINT32)(rcvd_dat_pri->byte[i+8U]) << 16U) + ((GM_RSSP1_UINT32)(rcvd_dat_pri->byte[i+9U]) << 24U);
#ifdef SFM_Part
				GM_RSSP1_CRC_Calculate_CRC16(rcvd_dat_pri->byte + i, (GM_RSSP1_UINT32)pkt_len, &crc16, g_Polyomia[0U].pCRC16_table);
#endif
				if (crc16 == 0U)
				{
					GM_RSSP1_CFM_PktLink_Insert(rcvd_TC,MsgType,(GM_RSSP1_UINT16)pkt_len,rcvd_dat_pri->byte + i);

					rcvd_TC = 0U;
					i += (GM_RSSP1_UINT16)pkt_len;
					i -= 1U;	/*jcf:与for循环中的i++抵消，保证顺序*/
					pkt_len = 0;
					bRt = GM_RSSP1_TRUE;
				}
			}
			else
			{
				break;
			}
		}
		/**由于同步不在组合包中,单独判断**/
		if (GM_RSSP1_FALSE == bRt)
		{
			bRt = GM_RSSP1_TRUE;
			/*CR:GM00004936、5777 添加对同步包的识别*/
			if((INTERFACE_TYPE_SYN == rcvd_dat_pri->byte[0])&&(
				((0xFFU == rcvd_dat_pri->byte[4U])&&(0xFFU == rcvd_dat_pri->byte[5U])&&(INTERFACE_DAT_TYPE_SYN_RSSP1_STATUS_DAT == rcvd_dat_pri->byte[1U])) 
				|| (INTERFACE_DAT_TYPE_SYN_RSSP1_ACTIVE_DAT == rcvd_dat_pri->byte[1U])))
			{
#ifdef  GM_RSSP1_SAVING_MODE
				GM_RSSP1_SFM_Process_RxData(GM_RSSP1_CFM_Data_Ind, rcvd_dat_pri->index, rcvd_dat_pri->byte, rcvd_dat_pri->bytes_count);

#else				/* find a Syn */
				L2U_pri.index   = rcvd_dat_pri->index;
				L2U_pri.type    = GM_RSSP1_CFM_Data_Ind;
				L2U_pri.as.data_indi.bytes_count    = rcvd_dat_pri->bytes_count;
				GM_RSSP1_memcpy((void*)L2U_pri.as.data_indi.byte , (const void*)rcvd_dat_pri->byte , (size_t)rcvd_dat_pri->bytes_count);
				write_q_rt = FSFB_Msg_Queue_Write(&(g_CFM_MQ.RCV_MQ) , &L2U_pri);
				if (Q_WRITE_OK_FULL == write_q_rt)
				{
					GM_RSSP1_Log_Msg(2,"CFM L2U_pri_Q full(CRSCD)!\n", (int)rcvd_dat_pri->index, 0, 0, 0, 0, 0);
					GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Full, rcvd_dat_pri->index, 0, 0, 0, 0, 0);
				}
				else if (Q_WRITE_FAIL == write_q_rt)
				{
					GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Fail, rcvd_dat_pri->index, 0, 0, 0, 0, 0);
					bRt = GM_RSSP1_FALSE;
				}
				else
				{
					; /*do nothing*/
				}
#endif
			}
		}
	}

	return bRt;
}

GM_RSSP1_BOOL GM_RSSP1_CFM_ReSeqAlone(GM_RSSP1_com_input_pri_struct* rcvd_dat_pri) 
{
	GM_RSSP1_UINT8 MsgType = 0xFFU;
	GM_RSSP1_UINT16 crc16 = 0U;
	GM_RSSP1_UINT32 rcvd_TC = 0U;
	GM_RSSP1_CFM_L2U_pri_struct L2U_pri;
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;

	if (rcvd_dat_pri != NULL)
	{

		if ((RSSP1_RSD_FRAME_TYPE_FROM_A == rcvd_dat_pri->byte[1U]) || (RSSP1_RSD_FRAME_TYPE_FROM_B == rcvd_dat_pri->byte[1U]))
		{
			/* find a RSD */
			MsgType = 0U;
		}
		else if (RSSP1_SSE_FRAME_TYPE == rcvd_dat_pri->byte[1U])
		{
			/* find a SSE */
			MsgType = 1U;
		}
		else if (RSSP1_SSR_FRAME_TYPE == rcvd_dat_pri->byte[1U])
		{
			/* find a SSR */
			MsgType = 2U;
		}
		/*CR:GM00004936、5777 添加对同步包的识别*/
		else if((INTERFACE_TYPE_SYN == rcvd_dat_pri->byte[0U])&&(
			((0xFFU == rcvd_dat_pri->byte[4U])&&(0xFFU == rcvd_dat_pri->byte[5U]) &&(INTERFACE_DAT_TYPE_SYN_RSSP1_STATUS_DAT == rcvd_dat_pri->byte[1U])) 
			|| (INTERFACE_DAT_TYPE_SYN_RSSP1_ACTIVE_DAT == rcvd_dat_pri->byte[1U])))
		{
#ifdef  GM_RSSP1_SAVING_MODE
			GM_RSSP1_SFM_Process_RxData(GM_RSSP1_CFM_Data_Ind, rcvd_dat_pri->index, rcvd_dat_pri->byte, rcvd_dat_pri->bytes_count);

#else			/* find a Syn */
			L2U_pri.index   = rcvd_dat_pri->index;
			L2U_pri.type    = GM_RSSP1_CFM_Data_Ind;
			L2U_pri.as.data_indi.bytes_count    = rcvd_dat_pri->bytes_count;
			GM_RSSP1_memcpy((void*)L2U_pri.as.data_indi.byte , (const void*)rcvd_dat_pri->byte , (size_t)rcvd_dat_pri->bytes_count);
			write_q_rt = FSFB_Msg_Queue_Write(&(g_CFM_MQ.RCV_MQ) , &L2U_pri);
			if (Q_WRITE_OK_FULL == write_q_rt)
			{
				GM_RSSP1_Log_Msg(2,"CFM L2U_pri_Q full(CRSCD)!\n", (int)rcvd_dat_pri->index, 0, 0, 0, 0, 0);
				GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Full, rcvd_dat_pri->index, 0, 0, 0, 0, 0);
			}
			else if (Q_WRITE_FAIL == write_q_rt)
			{
				GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Fail, rcvd_dat_pri->index, 0, 0, 0, 0, 0);
				return GM_RSSP1_FALSE;
			}
			else
			{
				; /*do nothing*/
			}
#endif
		}
		else
		{
			/* unknown pkt */
			GM_RSSP1_Log_Msg(2,"CFM recvd unknown pkt,type:%d.\n", (int)rcvd_dat_pri->byte[1], 0, 0, 0, 0, 0);	/*jcf：单包下无i，将byte[i+1]改为byte[1]*/
			return GM_RSSP1_FALSE;
		}

		/* TODO: 如何挑选最合适包待定 */
		if (rcvd_dat_pri->bytes_count > GM_RSSP1_MAX_SAFETY_DAT_LEN)
		{
			GM_RSSP1_Log_Msg(2,"CFM ID:0x%X UDP socket %d recvd %d bytes,exceed GM_RSSP1_MAX_SAFETY_DAT_LEN: %d.\n", (int)rcvd_dat_pri->bytes_count, (int)GM_RSSP1_MAX_SAFETY_DAT_LEN, 0, 0, 0, 0);
		}
		else
		{
			rcvd_TC = (GM_RSSP1_UINT32)(rcvd_dat_pri->byte[6U]) + ((GM_RSSP1_UINT32)(rcvd_dat_pri->byte[7U]) << 8U) + ((GM_RSSP1_UINT32)(rcvd_dat_pri->byte[8U]) << 16U) + ((GM_RSSP1_UINT32)(rcvd_dat_pri->byte[9U]) << 24U);
			/* TODO:CRC检验 */
#ifdef SFM_Part
			GM_RSSP1_CRC_Calculate_CRC16(rcvd_dat_pri->byte, (GM_RSSP1_UINT32)rcvd_dat_pri->bytes_count, &crc16, g_Polyomia[0U].pCRC16_table);
#endif
			if (crc16 == 0U)
			{
				GM_RSSP1_CFM_PktLink_Insert(rcvd_TC,MsgType,rcvd_dat_pri->bytes_count,rcvd_dat_pri->byte);
				rcvd_TC = 0U;
			}
		}

		return GM_RSSP1_TRUE;
	}
	else
	{
		return GM_RSSP1_FALSE;
	}
}
GM_RSSP1_BOOL GM_RSSP1_CFM_Proc_Con_DatWithReSeq(GM_RSSP1_UINT16 index, GM_RSSP1_BOOL bLock) 
{
	GM_RSSP1_UINT16 tmpIndex = 0U;
	GM_RSSP1_UINT32 i = 0U;
	GM_RSSP1_INT32 chn_index = 0;
	GM_RSSP1_com_input_pri_struct rcvd_dat_pri;
	GM_RSSP1_CFM_connection_struct *pCFM_con = NULL; 
	GM_RSSP1_CFM_L2U_pri_struct L2U_pri;
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;

	GM_RSSP1_memset((void*)&RecordPkt, 0U, sizeof(RSSP1_RCV_LINK_struct));

	for (chn_index = 0 ; chn_index < (GM_RSSP1_INT32)GM_RSSP1_MAX_LINK_CHANNEL; ++chn_index)
	{
		while (GM_RSSP1_TRUE == FSFB_Msg_Queue_Get(&(g_Link_MQ.RCV_MQ[index][chn_index]), &rcvd_dat_pri))
		{
			if (rcvd_dat_pri.index < GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
			{
				pCFM_con = &(g_CFM_obj.connection[rcvd_dat_pri.index]);

				if (rcvd_dat_pri.bytes_count > GM_RSSP1_MAX_SAFETY_DAT_LEN)
				{
					GM_RSSP1_Log_Msg(2,"CFM ID:0x%X UDP socket %d recvd %d bytes,exceed GM_RSSP1_MAX_SAFETY_DAT_LEN: %d.\n", (int)pCFM_con->SaCEPID.source_addr , (int)chn_index , (int)rcvd_dat_pri.bytes_count, (int)GM_RSSP1_MAX_SAFETY_DAT_LEN, 0, 0);
					continue;
				}


				if (GM_RSSP1_TRUE ==  bLock)
				{
					g_CFM_Status.IndexArry[g_CFM_Status.netNum] = rcvd_dat_pri.index;
					g_CFM_Status.netNum ++;
					bLock = GM_RSSP1_FALSE;
				}

				pCFM_con->com_chn[chn_index].b_connected = GM_RSSP1_TRUE;
				/* CRSCD方案，收到的一个UDP数据包可能是多个FSFB包合并而成 */
				if (GM_RSSP1_TRUE == pCFM_con->b_enable_CRSCD_pack)
				{
					bRt = GM_RSSP1_CFM_ReSeqUnderPack(&rcvd_dat_pri);
				}
				else
				{
					bRt = GM_RSSP1_CFM_ReSeqAlone(&rcvd_dat_pri);
				}

				if (bRt == GM_RSSP1_FALSE)
				{
					g_CFM_obj.lib_state = GM_RSSP1_Lib_State_Failure;
					return bRt;
				}
			}
		}
	}

	L2U_pri.index   = rcvd_dat_pri.index;
	L2U_pri.type    = GM_RSSP1_CFM_Data_Ind;

	tmpIndex = RecordPkt.MinTC_Index;

	for (i = 0U; i< (GM_RSSP1_UINT32)RecordPkt.num; ++i)
	{
#ifdef  GM_RSSP1_SAVING_MODE
		GM_RSSP1_SFM_Process_RxData(GM_RSSP1_CFM_Data_Ind, rcvd_dat_pri.index, RecordPkt.Allpkt[tmpIndex].byte, RecordPkt.Allpkt[tmpIndex].bytes_count);
#else
		L2U_pri.as.data_indi.bytes_count = RecordPkt.Allpkt[tmpIndex].bytes_count;
		GM_RSSP1_memcpy((void*)L2U_pri.as.data_indi.byte, (const void*)RecordPkt.Allpkt[tmpIndex].byte, (size_t)RecordPkt.Allpkt[tmpIndex].bytes_count);/*修改数据包长度被覆盖的问题*/
		write_q_rt = FSFB_Msg_Queue_Write(&(g_CFM_MQ.RCV_MQ) , &L2U_pri);

		if (Q_WRITE_OK_FULL == write_q_rt)
		{
			GM_RSSP1_Log_Msg(2,"CFM L2U_pri_Q full!\n", (int)rcvd_dat_pri.index, 0, 0, 0, 0, 0);
			GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Full, rcvd_dat_pri.index, 0, 0, 0, 0, 0);
		}
		else if (Q_WRITE_FAIL == write_q_rt)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Fail, rcvd_dat_pri.index, 0, 0, 0, 0, 0);
			g_CFM_obj.lib_state = GM_RSSP1_Lib_State_Failure;
			return GM_RSSP1_FALSE;
		}
		else
		{
			; /*do nothing*/
		}
#endif
		tmpIndex = RecordPkt.pktInfo[tmpIndex].nextIndex;
	}

	return GM_RSSP1_TRUE;
}

/*CR:GM00004824 communication layer should support two mode to deal with redundant message*/
GM_RSSP1_BOOL GM_RSSP1_CFM_Proc_Con_Rcvd_Dat(GM_RSSP1_UINT16 index)	/*CR：GM00004955 修正index为16位，以保证conn_num>256时能够正常连接*/
{
	GM_RSSP1_UINT16 Qcount1 = 0U;
	GM_RSSP1_UINT16 Qcount2 = 0U;
	GM_RSSP1_BOOL bLock = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	

	if (index >= GM_RSSP1_MAX_ONLINE_CONNECTION_NUM)/*CR5746*/
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Param_Val_Error, (int)index, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	Qcount1 = (GM_RSSP1_UINT16)FSFB_Get_Queue_Count(&(g_Link_MQ.RCV_MQ[index][0U]));
	Qcount2 = (GM_RSSP1_UINT16)FSFB_Get_Queue_Count(&(g_Link_MQ.RCV_MQ[index][1U]));
	

	if ((Qcount1 + Qcount2) > 0U)
	{		
		if (g_CFM_Status.netNum < GM_RSSP1_MAX_ONLINE_CONNECTION_NUM)
		{
			bLock = GM_RSSP1_TRUE;
		}
	}
	
	if (MQ_SEQ_METHOD == 0U)
	{
		bRt = GM_RSSP1_CFM_Proc_Con_DatWithLatest(index, bLock);
	}
	else
	{
		bRt = GM_RSSP1_CFM_Proc_Con_DatWithReSeq(index, bLock);
	}
	return bRt;
}

void GM_RSSP1_CFM_Proc_Recvd_Dat(GM_RSSP1_CFM_object_struct * pCFM)
{
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT32 MaxCyc = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_CFM_connection_struct *pCFM_con = NULL;

	if (NULL == pCFM)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Recvd_Dat_Param_Point_Error, 0, 0, 0, 0, 0, 0);
		return;
	}

	if (GM_RSSP1_Lib_State_Operational != pCFM->lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Proc_Recvd_Dat_State_Fail, pCFM->lib_state, 0, 0, 0, 0, 0);
		return;
	}

	GM_RSSP1_memset((void*)&g_CFM_Status, 0U, sizeof(GM_RSSP1_CFM_State_Index_struct));
	/*GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 1);*/
	for (l_index = 0U; l_index < GM_RSSP1_MAX_ONLINE_CONNECTION_NUM; ++l_index)
	{
		/*con_index = GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 0);*/
		bRt = GM_RSSP1_CFM_Proc_Con_Rcvd_Dat(l_index);
	}

	return;
}

void GM_RSSP1_CFM_Report_com_State_To_SFM(GM_RSSP1_CFM_object_struct *pCFM)
{
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 chn_index = 0U;
	GM_RSSP1_CFM_L2U_pri_struct pri;
	GM_RSSP1_CFM_connection_struct *pCFM_con = NULL;
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;

	if (NULL == pCFM)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Report_com_State_To_SFM_Param_Point_Error, 0, 0, 0, 0, 0, 0);
		return;
	}

	if (GM_RSSP1_Lib_State_Operational != pCFM->lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Report_com_State_To_SFM_State_Fail, pCFM->lib_state, 0, 0, 0, 0, 0);
		return;
	}

	pri.type    = GM_RSSP1_CFM_Indi_com_State;

	pri.index   = g_CFM_Status.netNum;    /* CFM状态包数 */

	for (l_index = 0U ; l_index < g_CFM_Status.netNum ; ++l_index)
	{
		pCFM_con = &(pCFM->connection[g_CFM_Status.IndexArry[l_index]]);
		pri.as.channel_state_indi.GM_RSSP1_CFM_Channel_states[l_index].index = pCFM_con->index;
		pri.as.channel_state_indi.GM_RSSP1_CFM_Channel_states[l_index].channel_state = 0U;

		pri.as.channel_state_indi.GM_RSSP1_CFM_Channel_states[l_index].chn_num = (GM_RSSP1_UINT8)(pCFM_con->chn_num);

		for (chn_index = 0U ; chn_index < pCFM_con->chn_num ; ++chn_index)
		{
			if (GM_RSSP1_TRUE == pCFM_con->com_chn[chn_index].b_connected)
			{
				pri.as.channel_state_indi.GM_RSSP1_CFM_Channel_states[l_index].channel_state |= (GM_RSSP1_UINT8)(1U << chn_index);
				pCFM_con->com_chn[chn_index].b_connected = GM_RSSP1_FALSE;
			}
		}
	}
#ifdef  GM_RSSP1_SAVING_MODE
	GM_RSSP1_SFM_Process_RxData(GM_RSSP1_CFM_Indi_com_State, pri.index, (GM_RSSP1_UINT8 *)&(pri.as.channel_state_indi), 0U);
#else
	write_q_rt = FSFB_Msg_Queue_Write(&(g_CFM_MQ.RCV_MQ) , &pri);

	if (Q_WRITE_OK_FULL == write_q_rt)
	{
		GM_RSSP1_Log_Msg(2,"CFM L2U_pri_Q full , report UDP state fail!\n", 0, 0, 0, 0, 0, 0);
	}
	else if (Q_WRITE_FAIL == write_q_rt)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Report_com_State_To_SFM_Write_Fail, 0, 0, 0, 0, 0, 0);
		pCFM->lib_state = GM_RSSP1_Lib_State_Failure;
		return;
	}
	else
	{
		; /*do nothing*/
	}
#endif
	return;
}

void GM_RSSP1_CFM_PktLink_Insert(GM_RSSP1_UINT32 rcvd_TC, GM_RSSP1_UINT8 MsgType, GM_RSSP1_UINT16 pkt_len, GM_RSSP1_UINT8* data_pri)
{
	GM_RSSP1_UINT16 cycNum = 0U;
	GM_RSSP1_UINT16 tmpIndex = 0U;
	GM_RSSP1_BOOL tmpChk = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL breakFlag = GM_RSSP1_FALSE;

	if ((NULL != data_pri) && (RecordPkt.MaxTC_Index < GM_RSSP1_MAX_PKT_TOTAL_NUM))/*CR5746*/
	{
		if (rcvd_TC > RecordPkt.pktInfo[RecordPkt.MaxTC_Index].pktTC) /*最新数据，直接记录*/
		{
			// 记录最新数据的TC、类型和索引信息
			RecordPkt.pktInfo[RecordPkt.num].pktTC = rcvd_TC;
			RecordPkt.pktInfo[RecordPkt.num].pktType = MsgType;
			RecordPkt.pktInfo[RecordPkt.num].nextIndex = RecordPkt.num + 1U;
			// 更新最大时间戳索引和结束索引
			RecordPkt.MaxTC_Index = RecordPkt.num;
			RecordPkt.End_Index = RecordPkt.num;
			// 将当前数据包的长度写入Allpkt数组
			RecordPkt.Allpkt[RecordPkt.num].bytes_count = pkt_len;
			// 将data_ pri中的内容复制到Allpkt数组中
			GM_RSSP1_memcpy((void*)&RecordPkt.Allpkt[RecordPkt.num].byte, (const void*)data_pri , (size_t)pkt_len);/*CR:GM00004825 recorrect error dest memory address */
			RecordPkt.num++;
		}
		else if (rcvd_TC < RecordPkt.pktInfo[RecordPkt.MinTC_Index].pktTC) /* 收到包比最小包的TC还小 */
		{
			RecordPkt.pktInfo[RecordPkt.End_Index].nextIndex = RecordPkt.num + 1U;
			RecordPkt.pktInfo[RecordPkt.num].pktTC = rcvd_TC;
			RecordPkt.pktInfo[RecordPkt.num].pktType = MsgType;
			RecordPkt.pktInfo[RecordPkt.num].nextIndex = RecordPkt.MinTC_Index;
			RecordPkt.MinTC_Index = RecordPkt.num;
			RecordPkt.Allpkt[RecordPkt.num].bytes_count = (GM_RSSP1_UINT16)pkt_len;
			GM_RSSP1_memcpy((void*)&RecordPkt.Allpkt[RecordPkt.num].byte, (const void*)data_pri , (size_t)pkt_len);/*CR:GM00004825 recorrect error dest memory address */
			RecordPkt.num++;	
		}
		else if (rcvd_TC == RecordPkt.pktInfo[RecordPkt.MaxTC_Index].pktTC) /* 收到包和最新记录的包TC一致 */
		{
			tmpIndex = RecordPkt.MaxTC_Index;
			while (rcvd_TC == RecordPkt.pktInfo[tmpIndex].pktTC)
			{
				if (MsgType == RecordPkt.pktInfo[tmpIndex].pktType)
				{
					tmpChk = GM_RSSP1_TRUE;
					break;
				}
				tmpIndex = RecordPkt.pktInfo[tmpIndex].nextIndex;
			}

			if (GM_RSSP1_FALSE == tmpChk)	/*如果有不同类型包都是最大TC*/
			{
				RecordPkt.End_Index = RecordPkt.num;
				RecordPkt.pktInfo[RecordPkt.num].pktTC = rcvd_TC;
				RecordPkt.pktInfo[RecordPkt.num].pktType = MsgType;
				RecordPkt.pktInfo[RecordPkt.num].nextIndex = RecordPkt.num + 1U;
				RecordPkt.Allpkt[RecordPkt.num].bytes_count = (GM_RSSP1_UINT16)pkt_len;
				GM_RSSP1_memcpy((void*)&RecordPkt.Allpkt[RecordPkt.num].byte, (const void*)data_pri , (size_t)pkt_len);/*CR:GM00004825 recorrect error dest memory address */
				RecordPkt.num++;
			}
		}
		else if (rcvd_TC == RecordPkt.pktInfo[RecordPkt.MinTC_Index].pktTC)
		{
			tmpIndex = RecordPkt.MinTC_Index;
			while (rcvd_TC == RecordPkt.pktInfo[tmpIndex].pktTC)
			{
				if (MsgType == RecordPkt.pktInfo[tmpIndex].pktType)
				{
					tmpChk = GM_RSSP1_TRUE;
					break;
				}
				cycNum = tmpIndex;	/*最小值A的index*/
				tmpIndex = RecordPkt.pktInfo[tmpIndex].nextIndex;	/*指向最小值A的下一个，即次小值C*/
			}

			if (GM_RSSP1_FALSE == tmpChk)	/*如果有不同类型包都是最小TC*/
			{
				RecordPkt.pktInfo[RecordPkt.End_Index].nextIndex = RecordPkt.num + 1U;
				RecordPkt.pktInfo[cycNum].nextIndex = RecordPkt.num;	/*最小值A.next = 下一个最小值B(当前正在插入的)*/
				RecordPkt.pktInfo[RecordPkt.num].nextIndex = tmpIndex;	/*最小值B.next = 次小值C*/
				RecordPkt.pktInfo[RecordPkt.num].pktTC = rcvd_TC;
				RecordPkt.pktInfo[RecordPkt.num].pktType = MsgType;
				RecordPkt.Allpkt[RecordPkt.num].bytes_count = (GM_RSSP1_UINT16)pkt_len;
				GM_RSSP1_memcpy((void*)&RecordPkt.Allpkt[RecordPkt.num].byte, (const void*)data_pri , (size_t)pkt_len);/*CR:GM00004825 recorrect error dest memory address */
				RecordPkt.num++;
			}
		}
		else
		{
			cycNum = RecordPkt.MinTC_Index;
			tmpIndex = RecordPkt.pktInfo[RecordPkt.MinTC_Index].nextIndex;		/*指向次小值*/
			while (tmpIndex != RecordPkt.pktInfo[RecordPkt.MaxTC_Index].nextIndex) /*链表里有元素，次小值不是null的时候*/
			{
				if ((rcvd_TC == RecordPkt.pktInfo[tmpIndex].pktTC) && (MsgType == RecordPkt.pktInfo[tmpIndex].pktType))
				{
					tmpChk = GM_RSSP1_TRUE;
					breakFlag = GM_RSSP1_TRUE;
				}

				if (rcvd_TC < RecordPkt.pktInfo[tmpIndex].pktTC)		/*找到待插入点，则跳出*/
				{
					breakFlag = GM_RSSP1_TRUE;
				}

				if(GM_RSSP1_TRUE == breakFlag)
				{
					break;
				}

				cycNum = tmpIndex;										/*向前挪移,跳出时则是待插入点的左节点*/
				tmpIndex = RecordPkt.pktInfo[tmpIndex].nextIndex;		/*向前挪移,跳出时则是待插入点的右节点*/
			}

			if (GM_RSSP1_FALSE == tmpChk)/*如果TC一样的包是不同类型，则插入*/
			{
				RecordPkt.pktInfo[RecordPkt.End_Index].nextIndex = RecordPkt.num + 1U;
				RecordPkt.pktInfo[cycNum].nextIndex = RecordPkt.num;	/*Insert*/
				RecordPkt.pktInfo[RecordPkt.num].nextIndex = tmpIndex;
				RecordPkt.pktInfo[RecordPkt.num].pktTC = rcvd_TC;
				RecordPkt.pktInfo[RecordPkt.num].pktType = MsgType;
				RecordPkt.Allpkt[RecordPkt.num].bytes_count = (GM_RSSP1_UINT16)pkt_len;
				GM_RSSP1_memcpy((void*)&RecordPkt.Allpkt[RecordPkt.num].byte, (const void*)data_pri, (size_t)pkt_len);/*CR:GM00004825 recorrect error dest memory address */
				RecordPkt.num++;
			}
		}
	}

}


/* start:CR:GM00002526, GM162, fulintai,2015-06-09, add this function to creat all 0 message in standby's CFM*/
#ifdef CFM_Stanby_Answer
GM_RSSP1_BOOL GM_RSSP1_Standby_Info(void)
{
	GM_RSSP1_UINT16 onlineNum = 0U;
	GM_RSSP1_CFM_U2L_pri_struct cfm_pri = {0};
	GM_RSSP1_CFM_connection_struct *pCFM_con = NULL;
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;
	GM_RSSP1_CFM_object_struct *pCFM = GM_RSSP1_Get_CFM_Object();
	Local_Sys_AB_enum sys_a_b = GM_RSSP1_Local_Sys_UNLOWN;
	GM_RSSP1_BOOL b_active = GM_RSSP1_TRUE;
	GM_RSSP1_UINT16 crc16 = 0U;
	GM_RSSP1_UINT32 VSN0 = 0U;
	GM_RSSP1_UINT32 VSN1 = 0U;
	GM_RSSP1_UINT32 VSN2 = 0U;
	GM_RSSP1_UINT16 index =0U;
	GM_RSSP1_UINT16 con_index =0U;

	if (pCFM == NULL)
	{
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_Lib_State_Operational != pCFM->lib_state)
	{
		return GM_RSSP1_FALSE;
	}

	GM_RSSP1_APP_Interface_Get_Syn_AB_AS_Info(&sys_a_b , &b_active);
	if (GM_RSSP1_TRUE == b_active)
	{
		return GM_RSSP1_FALSE;
	}

	if (pCFM->all0_size > GM_RSSP1_MAX_USER_DAT_LEN )
	{
		return GM_RSSP1_FALSE;
	}

	onlineNum = GM_RSSP1_GetSeqAmount(OnLine_Object);

	GM_RSSP1_GetSeqIndexPositive(OnLine_Object, 1U);

	for(index=0U; index<onlineNum; index++)
	{
		con_index = GM_RSSP1_GetSeqIndexPositive(OnLine_Object, 0U);
		if (0xFFFFU != con_index)
		{
			pCFM_con = &(pCFM->connection[con_index]);
			cfm_pri.type                = GM_RSSP1_CFM_Data_req;
			cfm_pri.index               = pCFM_con->index;
			cfm_pri.data_req.type    = GM_RSSP1_MSG_TYPE_RSD;
			cfm_pri.data_req.TC      = VSN0;

			GM_RSSP1_VSN_Get_Callback_Fun(&VSN0, &VSN1, &VSN2);

			cfm_pri.data_req.bytes_count = 22U+pCFM->all0_size;
			cfm_pri.data_req.byte[0U] = 0x02U;
			cfm_pri.data_req.byte[1U] = sys_a_b;
			cfm_pri.data_req.byte[2U] = (GM_RSSP1_UINT8)(pCFM_con->SaCEPID.source_addr&0x00ffU);
			cfm_pri.data_req.byte[3U] = (GM_RSSP1_UINT8)((pCFM_con->SaCEPID.source_addr&0xff00U)>>8U);
			cfm_pri.data_req.byte[4U] = (GM_RSSP1_UINT8)(pCFM_con->SaCEPID.dest_addr&0x00ffU);
			cfm_pri.data_req.byte[5U] = (GM_RSSP1_UINT8)((pCFM_con->SaCEPID.dest_addr&0xff00U)>>8U);
			cfm_pri.data_req.byte[6U] = (GM_RSSP1_UINT8)(VSN0 & 0xffU);
			cfm_pri.data_req.byte[7U] = (GM_RSSP1_UINT8)((VSN0 & 0xff00U) >> 8U);
			cfm_pri.data_req.byte[8U] = (GM_RSSP1_UINT8)((VSN0 & 0xff0000U) >> 16U);
			cfm_pri.data_req.byte[9U] = (GM_RSSP1_UINT8)((VSN0 & 0xff000000) >> 24U);
			cfm_pri.data_req.byte[10U] = (GM_RSSP1_UINT8)((8U+pCFM->all0_size) & 0xffU);
			cfm_pri.data_req.byte[11U] = (GM_RSSP1_UINT8)((8U+pCFM->all0_size) >> 8U);
			GM_RSSP1_memset((void*)(cfm_pri.data_req.byte+12U), 0U, (size_t)(8U+pCFM->all0_size));

			GM_RSSP1_CRC_Calculate_CRC16(cfm_pri.data_req.byte, (20U+pCFM->all0_size), &crc16, (GM_RSSP1_UINT16*)GM_RSSP1_CRC16_Table_0x10811_LSB);

			cfm_pri.data_req.byte[20U+pCFM->all0_size] = (GM_RSSP1_UINT8)(crc16 & 0xffU);
			cfm_pri.data_req.byte[21U+pCFM->all0_size] = (GM_RSSP1_UINT8)(crc16 >> 8U);

			write_q_rt = FSFB_Msg_Queue_Write(&g_CFM_Debug_MQ , &cfm_pri);
			if (Q_WRITE_FAIL == write_q_rt)
			{
				GM_RSSP1_Log_Usr(GM_RSSP1_CFM_User_Send_Dat_Write_Fail, 0, 0, 0, 0, 0, 0);
				return GM_RSSP1_FALSE;
			}
			else if (Q_WRITE_OK_FULL == write_q_rt)
			{
				/* 成功，但队列已满 */
				GM_RSSP1_Log_Msg(2,"CFM U2L_pri_Q FULL!\n ", 0, 0, 0, 0, 0, 0);
			}
			else
			{
				;
			}
		}
	}

	return GM_RSSP1_TRUE;
}
#endif

#endif
