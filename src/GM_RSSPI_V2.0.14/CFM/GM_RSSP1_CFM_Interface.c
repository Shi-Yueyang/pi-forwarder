#include "GM_RSSP1_CFM_Interface.h"
#include "GM_RSSP1_MQ_Interface.h"

#ifdef CFM_Part
extern RSSP1_MQ_LINK_struct g_Link_MQ;
extern RSSP1_MQ_Inter_struct g_CFM_MQ;
extern GM_RSSP1_UINT8 MQ_SEQ_METHOD;

#ifdef	CFM_Stanby_Answer
extern GM_RSSP1_Msg_Queue_struct g_CFM_Debug_MQ;
#endif
GM_RSSP1_Local_Con_struct g_loc_info = {0};

/*接收过程 先CFM，再SFM*/
void GM_RSSP1_CFM_Interface_Proc_Recv(void)
{
	GM_RSSP1_CFM_object_struct *pCFM = NULL;
	pCFM = GM_RSSP1_Get_CFM_Object();

	if (NULL == pCFM)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Interface_Proc_Recv_Get_Object_Fail, 0, 0, 0, 0, 0, 0);
		return;
	}

	if (GM_RSSP1_Lib_State_Operational != pCFM->lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Interface_Proc_Recv_State_Fail, pCFM->lib_state, 0, 0, 0, 0, 0);
		return;
	}

	GM_RSSP1_CFM_Proc_Recvd_Dat(pCFM);/*功能：将通信层数据放入冗余层，需要拆分*/
	GM_RSSP1_CFM_Report_com_State_To_SFM(pCFM);/*功能：将通信层状态通过冗余层上传，需要拆分*/
#ifndef SFM_Part
	GM_RSSP1_Reset_Active();/*重置当前周期的活动通道信息*/
#endif
}


/*发送过程 先SFM，再CFM*/
void GM_RSSP1_CFM_Interface_Proc_Send(void)
{
#ifndef GM_RSSP1_SAVING_MODE
	GM_RSSP1_CFM_object_struct *pCFM = NULL;

	pCFM = GM_RSSP1_Get_CFM_Object();

	if (NULL == pCFM)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Interface_Proc_Send_Get_Object_Fail, 0, 0, 0, 0, 0, 0);
		return;
	}

	if (GM_RSSP1_Lib_State_Operational != pCFM->lib_state)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_CFM_Interface_Proc_Send_State_Fail, pCFM->lib_state, 0, 0, 0, 0, 0);
		return;
	}

	GM_RSSP1_CFM_Proc_SFM_Req();/**处理从SFM层传过来的数据**/
#endif

}


GM_RSSP1_BOOL GM_RSSP1_Get_ConIndex(GM_RSSP1_UINT32 r_port, GM_RSSP1_UINT32 r_ip, GM_RSSP1_UINT16* con_index, GM_RSSP1_UINT8* red_index)
{
	GM_RSSP1_BOOL bRt = GM_RSSP1_TRUE;

	bRt = GM_RSSP1_Hash_GetCFMIndex(r_ip, r_port, con_index, red_index);

	return bRt;
}

GM_RSSP1_BOOL GM_RSSP1_Get_Address(GM_RSSP1_UINT32* r_port, GM_RSSP1_UINT32* r_ip, GM_RSSP1_UINT16 con_index, GM_RSSP1_UINT8 red_index)
{
	GM_RSSP1_CFM_object_struct *pCFM = NULL;
	GM_RSSP1_CFM_connection_struct *pCFM_con = NULL;

	if ((r_port == NULL)||(r_ip == NULL))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Get_Address_Param_Point_Error, (int)r_port, (int)r_ip, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	pCFM = GM_RSSP1_Get_CFM_Object();
	if (pCFM == NULL)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Get_Address_Get_Object_Fail, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	pCFM_con = &(pCFM->connection[con_index]);

	if (red_index>=pCFM_con->chn_num)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Get_Address_Chn_index_Fail, (int)red_index, (int)pCFM_con->chn_num, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	
	*r_port = pCFM_con->com_chn[red_index].rem_port;
	*r_ip = pCFM_con->com_chn[red_index].rem_ip;

	return GM_RSSP1_TRUE;
}


GM_RSSP1_Local_Con_struct* GM_RSSP1_Get_LocInfo_Obj(void)
{
	return &g_loc_info;
}

/*CR:GM00002527, GM162, fulintai,2013-06-09, much more return type for different exit*/
GM_RSSP1_INT8 GM_RSSP1_Write_SFM_Interface_ergodic(GM_RSSP1_UINT8* p_dat, GM_RSSP1_UINT16* len, GM_RSSP1_UINT32* r_port, GM_RSSP1_UINT32* r_ip)
{
	GM_RSSP1_com_out_pri_struct  temp_send_pkt;
	GM_RSSP1_BOOL write_q_rt = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	GM_RSSP1_CFM_object_struct *pCFM = NULL;
	GM_RSSP1_CFM_connection_struct *pCFM_con = NULL;

	if ((p_dat == NULL)||(len == NULL) ||(r_port == NULL) ||(r_ip == NULL))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Write_SFM_Interface_ergodic_Param_Point_Error, (int)p_dat, (int)len, (int)r_port, (int)r_ip, 0, 0);
		return -1;
	}

	pCFM = GM_RSSP1_Get_CFM_Object();
	if (pCFM == NULL)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Write_SFM_Interface_ergodic_Get_Object_Fail, 0, 0, 0, 0, 0, 0);
		return -1;
	}

	while(GM_RSSP1_TRUE == FSFB_Msg_Queue_Get(&(g_Link_MQ.SND_MQ),&temp_send_pkt))
	{
		*r_port  = temp_send_pkt.r_port;
		*r_ip = temp_send_pkt.r_ip;
		GM_RSSP1_memcpy((void*)p_dat, (const void*)temp_send_pkt.byte, (size_t)temp_send_pkt.bytes_count);
		*len = temp_send_pkt.bytes_count;
		return 1;
	}
	return 0;
}

GM_RSSP1_BOOL RSSP1_CFM_Clear_Msg_Queue(void)
{
	GM_RSSP1_CFM_object_struct *pCFM = NULL;
	register GM_RSSP1_CFM_connection_struct *pCFM_con = NULL;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_UINT8 j = 0U;
	GM_RSSP1_BOOL rt_1 = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL rt_2 = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL rt_3 = GM_RSSP1_FALSE;

	/*clear FSFB SFM queue*/
	pCFM = GM_RSSP1_Get_CFM_Object();
	if (pCFM == NULL)
	{
		GM_RSSP1_Log_Usr(RSSP1_CFM_Clear_Msg_Queue_Get_Object_Fail, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
#ifndef GM_RSSP1_SAVING_MODE
	/* u2l l2u*/
	rt_1 = FSFB_Msg_Queue_Clear(&(g_CFM_MQ.SND_MQ));

	rt_2 = FSFB_Msg_Queue_Clear(&(g_CFM_MQ.RCV_MQ));   
#endif
	/*every connection Q*/
	rt_3 = FSFB_Msg_Queue_Clear(&(g_Link_MQ.SND_MQ));
	
	if((GM_RSSP1_TRUE == rt_1) && (GM_RSSP1_TRUE == rt_2) && (GM_RSSP1_TRUE == rt_3))
	{
		return GM_RSSP1_TRUE;
	}
	else
	{
		return GM_RSSP1_FALSE;
	}
}

/* start:CR:GM00002526, GM162, fulintai,2015-06-09, add interface to get all 0 message in standby's CFM*/
#ifdef	CFM_Stanby_Answer
GM_RSSP1_INT8 GM_RSSP1_Write_StandbyInfo_Interface_ergodic(GM_RSSP1_UINT8* p_dat, GM_RSSP1_UINT16* len, GM_RSSP1_UINT16* con_index, GM_RSSP1_UINT8* reduan_index)
{
	GM_RSSP1_CFM_U2L_pri_struct pri = {0};
	GM_RSSP1_BOOL write_q_rt = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	GM_RSSP1_CFM_object_struct *pCFM = NULL;
	GM_RSSP1_CFM_connection_struct *pCFM_con = NULL;
	GM_RSSP1_UINT16 i = 0U;
	GM_RSSP1_UINT16 j = 0U;

	if ((p_dat == NULL)||(len == NULL) || (con_index == NULL) || (reduan_index== NULL))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Write_SFM_Interface_ergodic_Param_Point_Error, p_dat, len, con_index, reduan_index, 0, 0);
		return -1;
	}

	pCFM = GM_RSSP1_Get_CFM_Object();
	
	if (pCFM == NULL)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_Write_SFM_Interface_ergodic_Get_Object_Fail, 0, 0, 0, 0, 0, 0);
		return -1;
	}

	while(GM_RSSP1_TRUE == FSFB_Msg_Queue_Get(&g_CFM_Debug_MQ,&pri))
	{
		i = pri.index;
		j = pCFM->connection[i].chn_num;
		
        if ((pri.index > pCFM->connection_nums) || (GM_RSSP1_CFM_U2L_Unknow == pri.type))
        {
        	continue;
        }
		
		GM_RSSP1_memcpy((void*)p_dat, (const void*)pri.data_req.byte , (size_t)pri.data_req.bytes_count);
		*len = pri.data_req.bytes_count;
		*con_index = i;
		*reduan_index = (GM_RSSP1_UINT8)j;
		return 1;
	}
	FSFB_Msg_Queue_Clear(&g_CFM_Debug_MQ);
	return 0;
}
#endif
/* end CR:GM00002526*/


GM_RSSP1_BOOL GM_RSSP1_SND_com_Interface(GM_RSSP1_UINT8* p_dat, GM_RSSP1_UINT16* len, GM_RSSP1_UINT32 *r_ip, GM_RSSP1_UINT32 *r_port, GM_RSSP1_UINT16 *index, GM_RSSP1_UINT8 *chn_index)
{
	GM_RSSP1_com_out_pri_struct  temp_send_pkt;
	GM_RSSP1_BOOL write_q_rt = GM_RSSP1_FALSE;

	if ((p_dat == NULL)||(len == NULL)||(r_ip == NULL)||(r_port == NULL)||(index == NULL)||(chn_index == NULL))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SND_com_Interface_Param_Val_Error, (int)p_dat, (int)len, (int)r_ip, (int)r_port,(int)index,(int)chn_index);
		return GM_RSSP1_FALSE;
	}
	write_q_rt = FSFB_Msg_Queue_Get(&(g_Link_MQ.SND_MQ),&temp_send_pkt);
	if (GM_RSSP1_TRUE == write_q_rt)
	{
		GM_RSSP1_memcpy((void*)p_dat, (const void*)temp_send_pkt.byte, (size_t)temp_send_pkt.bytes_count);
		*len = temp_send_pkt.bytes_count;
		*r_ip = temp_send_pkt.r_ip;
		*r_port = temp_send_pkt.r_port;
		*index = temp_send_pkt.index;
		*chn_index = temp_send_pkt.subindex;
		return GM_RSSP1_TRUE;
	}
	else
	{
		/*GM_RSSP1_Log_Usr(GM_RSSP1_SND_com_Interface_Read_Fail, 0, 0, 0, 0, 0, 0);*/
		return GM_RSSP1_FALSE;
	}
}
/* defect: 13801674 */
GM_RSSP1_BOOL GM_RSSP1_SND_com_with_crcm_Interface(GM_RSSP1_UINT8* p_dat, GM_RSSP1_UINT16* len, GM_RSSP1_UINT32 *r_ip, GM_RSSP1_UINT32 *r_port, GM_RSSP1_UINT16 *index, GM_RSSP1_UINT8 *chn_index,GM_RSSP1_UINT16 *crcm_index)
{
	GM_RSSP1_com_out_pri_struct  temp_send_pkt;
	GM_RSSP1_BOOL write_q_rt = GM_RSSP1_FALSE;

	if ((p_dat == NULL)||(len == NULL)||(r_ip == NULL)||(r_port == NULL)||(index == NULL)||(chn_index == NULL)||(crcm_index == NULL))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_SND_com_Interface_Param_Val_Error, (int)p_dat, (int)len, (int)r_ip, (int)r_port,(int)index,(int)chn_index);
		return GM_RSSP1_FALSE;
	}
	write_q_rt = FSFB_Msg_Queue_Get(&(g_Link_MQ.SND_MQ),&temp_send_pkt);
	if (GM_RSSP1_TRUE == write_q_rt)
	{
		GM_RSSP1_memcpy((void*)p_dat, (const void*)temp_send_pkt.byte, (size_t)temp_send_pkt.bytes_count);
		*len = temp_send_pkt.bytes_count;
		*r_ip = temp_send_pkt.r_ip;
		*r_port = temp_send_pkt.r_port;
		*index = temp_send_pkt.index;
		*chn_index = temp_send_pkt.subindex;
		if((*(p_dat + 1U) == 0x80 || *(p_dat + 1U) == 0x81))
		{
			*crcm_index = (*(p_dat + 13U) << 8) | *(p_dat + 12U);
			*(p_dat + 12U) = 0U;
			*(p_dat + 13U) = 0U;
		}
		else
		{
			*crcm_index = 0xFFFFU;
		}
		return GM_RSSP1_TRUE;
	}
	else
	{
		/*GM_RSSP1_Log_Usr(GM_RSSP1_SND_com_Interface_Read_Fail, 0, 0, 0, 0, 0, 0);*/
		return GM_RSSP1_FALSE;
	}
}

GM_RSSP1_BOOL GM_RSSP1_RCV_com_Interface(GM_RSSP1_UINT8* p_dat, GM_RSSP1_INT16 recv_len, GM_RSSP1_UINT32 r_ip, GM_RSSP1_UINT32 r_port, GM_RSSP1_UINT16 index, GM_RSSP1_UINT8 chn_index, GM_RSSP1_UINT8 mode)
{
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;
	GM_RSSP1_com_input_pri_struct pri;
	GM_RSSP1_UINT16 LinkIndex = 0xFFFFU;
	GM_RSSP1_UINT16 curindex = 0xFFFFU;
	GM_RSSP1_UINT16 Nextindex = 0xFFFFU;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;

	if ((recv_len > (GM_RSSP1_INT16)GM_RSSP1_MAX_SND_DAT_LEN)||(p_dat ==NULL)||(index>=GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)||(chn_index>=GM_RSSP1_MAX_LINK_CHANNEL))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_RCV_com_Interface_Param_Val_Error, (int)recv_len, (int)index, (int)p_dat, (int)chn_index, (int)p_dat, 0);
		return bRt;
	}

	if (mode == 1U)
	{
		bRt = GM_RSSP1_Hash_GetCFMIndex(r_ip, r_port, &index, &chn_index);/**根据ip+port进行内部索引**/
	}
	else
	{
		bRt = GM_RSSP1_TRUE;
	}

	if (GM_RSSP1_TRUE == bRt)
	{
		LinkIndex = GM_RSSP1_Active_Add(index, 1U);/**将当前索引通道加入活动通道,返回活动通道索引**/

		/*if (0xFFFFU != LinkIndex)*/
		if((LinkIndex < (GM_RSSP1_UINT16)GM_RSSP1_MAX_ONLINE_CONNECTION_NUM) && (chn_index < (GM_RSSP1_UINT8)GM_RSSP1_MAX_LINK_CHANNEL))
		{
			pri.index = index;
			pri.subindex = chn_index;
			pri.bytes_count = (GM_RSSP1_UINT16)recv_len;
			GM_RSSP1_memcpy((void*)pri.byte , (const void*)p_dat , (size_t)recv_len);

			write_q_rt = FSFB_Msg_Queue_Write(&(g_Link_MQ.RCV_MQ[LinkIndex][chn_index]), &pri);
			if (Q_WRITE_FAIL == write_q_rt)
			{
				GM_RSSP1_Log_Usr(GM_RSSP1_RCV_com_Interface_Write_Fail, LinkIndex, chn_index, 0, 0, 0, 0);
				bRt = GM_RSSP1_FALSE;
			}
			else
			{
				; /**do nothing**/
			}
		}
		else
		{
			bRt = GM_RSSP1_FALSE;
		}
	}

	return bRt;
}

void GM_RSSP1_CFM_Interface_RemoveWithIndex(GM_RSSP1_UINT16 CnfIndex)
{
	GM_RSSP1_CFM_object_struct *pCFM = NULL;

	pCFM = GM_RSSP1_Get_CFM_Object();
	if (NULL != pCFM)
	{
		GM_RSSP1_CFM_Delete(CnfIndex, pCFM);/*删除CFM配置节点*/
		GM_RSSP1_SeqDelete(OffLine_Object, CnfIndex);/*释放所占双向链表节点的资源*/
	}
}

void GM_RSSP1_CFM_Interface_RemoveWithAddr(GM_RSSP1_UINT32 Dst_ip, GM_RSSP1_UINT32 Dst_port)
{
	GM_RSSP1_UINT8 i = 0U;
	GM_RSSP1_UINT8 counter = 0U;
	GM_RSSP1_UINT16 CnfIndex = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_CFM_object_struct *pCFM = NULL;

	pCFM = GM_RSSP1_Get_CFM_Object();
	if (NULL != pCFM)
	{
		bRt = GM_RSSP1_Hash_CFM_Dele(Dst_ip, Dst_port, &CnfIndex);/*删除hash表内对应的信息节点*/
		GM_RSSP1_CFM_Interface_RemoveWithIndex(CnfIndex);/**根据已删除hash表中注册的静态索引值,删除配置节点并释放所占活动资源**/
	}
}

void GM_RSSP1_Clean_Link_Status(void)
{
	GM_RSSP1_Reset_Active();
}

void GM_RSSP1_Set_ChkPKT_Seq_Method(GM_RSSP1_UINT8 method)
{
	MQ_SEQ_METHOD = method;
}
#endif
