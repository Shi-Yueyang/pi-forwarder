
#include "GM_RSSP1_Syn.h"
#include "GM_RSSP1_APP_Interface.h"


extern RSSP1_MQ_Inter_struct g_SFM_MQ;
extern GM_RSSP1_UINT16 g_Max_ConnctNum;

#ifdef SFM_Part
#ifdef CFM_Part
GM_RSSP1_BOOL GM_RSSP1_Syn_Send_Fsfb_State_To_Peer(void)
{
	GM_RSSP1_SFM_connection_struct *pSFM_con = NULL;
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_Syn_Info_struct syn_info;
	GM_RSSP1_UINT8 syn_dat[GM_RSSP1_MAX_SAFETY_DAT_LEN];
	GM_RSSP1_UINT16 state_dat_len = 0U;
	GM_RSSP1_UINT16 crc16 = 0U;
	GM_RSSP1_SACEPID_struct peer_syn_sacepid = {0};
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_object_struct *pSFM = NULL;

	pSFM = GM_RSSP1_Get_SFM_Object();

	if (NULL == pSFM)
	{
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_Lib_State_Operational != pSFM->lib_state)
	{
		return GM_RSSP1_FALSE;
	}

	if (1U >= pSFM->connection_nums)
	{
		/* 只有一个连接，不可能存在同步通道 */
		return GM_RSSP1_FALSE;
	}

	syn_dat[0U] = INTERFACE_TYPE_SYN;
	syn_dat[1U] = INTERFACE_DAT_TYPE_SYN_RSSP1_STATUS_DAT;
	state_dat_len += 2U;
	/*CR:GM00004936 设置同步的地址信息*/
	peer_syn_sacepid.source_addr = pSFM->connection->source_addr;
	peer_syn_sacepid.dest_addr = GM_RSSP1_SYN_DEST_ADDR;
	GM_RSSP1_memcpy((void*)(syn_dat + state_dat_len), (const void*)&peer_syn_sacepid,  sizeof(GM_RSSP1_SACEPID_struct));
	state_dat_len += sizeof(GM_RSSP1_SACEPID_struct);

	GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 1U);
	for (l_index = 0U; l_index < pSFM->connection_nums ; ++l_index)
	{
		con_index = GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 0U);
		/*if (0xFFFFU != con_index)*/
		if(con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
		{
			pSFM_con = &(pSFM->connection[con_index]);

			if (GM_RSSP1_SYN_DEST_ADDR != pSFM_con->SaCEPID.dest_addr)
			{
				syn_info.sacepid = pSFM_con->SaCEPID;
				syn_info.time_TC    = pSFM_con->time.TC;
				syn_info.time_TS[0U] = pSFM_con->time.TS[0U];
				syn_info.time_TS[1U] = pSFM_con->time.TS[1U];
				syn_info.time_oldTS[0U] = pSFM_con->time.oldTS[0U];
				syn_info.time_oldTS[1U] = pSFM_con->time.oldTS[1U];
				syn_info.env[0U] = pSFM_con->env[0U];
				syn_info.env[1U] = pSFM_con->env[1U];

#ifndef GM_RSSP1_SYN_NO_DATA
				/*added by huang 20151215*/
				syn_info.PreciTime = pSFM_con->PreciTime;
				syn_info.SSE_SSR_Delay = pSFM_con->SSE_SSR_Delay;
				syn_info.SINIT_TremCycle = pSFM_con->SINIT_TremCycle;
				syn_info.RxSSRTrem = pSFM_con->RxSSRTrem;
				syn_info.RxSSRTloc = pSFM_con->RxSSRTloc;
				syn_info.TempDiffTime = pSFM_con->TempDiffTime;
				syn_info.TcycleLoc = pSFM_con->TcycleLoc;
				syn_info.TcycleRem = pSFM_con->TcycleRem;
#endif
				/*the end 20151215*/
				GM_RSSP1_memcpy((void*)(syn_dat + state_dat_len), (const void*)&syn_info , sizeof(syn_info));
				state_dat_len += sizeof(syn_info);
			}
		}
	}

	/* add crc16 at tail, send to peer  */
	GM_RSSP1_CRC_Calculate_CRC16(syn_dat, (GM_RSSP1_UINT32)state_dat_len, &crc16, GM_RSSP1_CRC16_Table_0x11021_MSB);

	syn_dat[state_dat_len] = (GM_RSSP1_UINT8)((crc16 & 0xff00U) >> 8U);
	state_dat_len += 1U;
	syn_dat[state_dat_len] = (GM_RSSP1_UINT8)(crc16 & 0xffU);
	state_dat_len += 1U;

	GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 1U);
	/* send syn dat to peer via FSFB syn channel.CRC checksum not used here */
	for (l_index = 0U ; l_index < pSFM->connection_nums ; ++l_index)
	{
		con_index = GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 0U);

		/*if (0xFFFFU != con_index)*/
		if(con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
		{
			pSFM_con = &(pSFM->connection[con_index]);

			if (GM_RSSP1_TRUE == GM_RSSP1_SFM_Is_SaCEPID_Equal(&peer_syn_sacepid , &(pSFM_con->SaCEPID)))
			{
				GM_RSSP1_CFM_User_Send_Dat(&peer_syn_sacepid , syn_dat , state_dat_len);
				break;
			}
		}
	}
	return rt;
}

void GM_RSSP1_Syn_Send_App_To_Peer(GM_RSSP1_UINT8 pdate[], GM_RSSP1_UINT16 len)
{
	GM_RSSP1_INT32  i = 0;
	GM_RSSP1_UINT16 crc16 = 0U;
	GM_RSSP1_UINT16  l_index = 0U;
	GM_RSSP1_UINT16  con_index = 0U;
	GM_RSSP1_UINT8 dat_to_peer[GM_RSSP1_MAX_SAFETY_DAT_LEN];
	GM_RSSP1_SACEPID_struct peer_syn_sacepid = {0U};
	GM_RSSP1_SFM_connection_struct *pSFM_con = NULL;
	GM_RSSP1_SFM_object_struct *pFSFB_SFM = NULL;
	GM_RSSP1_UINT16 byte_count = 0U;

	pFSFB_SFM = GM_RSSP1_Get_SFM_Object();

	if ((NULL == pFSFB_SFM) || (NULL == (GM_RSSP1_UINT8*)pdate))
	{
		return;
	}

	i = 0;

	byte_count = len - 6U;/*接收端应用组包包头*/
	/*9: Flag(2) + GM_RSSP1_SACEPID_struct(4) + bytes_count(2) + crc16(2)*/
	if ((len + 10U) > GM_RSSP1_MAX_SAFETY_DAT_LEN)
	{
		return;
	}

	/* syn pkt type */
	dat_to_peer[i] = INTERFACE_TYPE_SYN;
	i += 1;
	dat_to_peer[i] = INTERFACE_DAT_TYPE_SYN_RSSP1_ACTIVE_DAT;
	i += 1;

	peer_syn_sacepid.source_addr    = (GM_RSSP1_UINT16)pdate[2U] * 256U + (GM_RSSP1_UINT16)pdate[3U];

	peer_syn_sacepid.dest_addr      = (GM_RSSP1_UINT16)pdate[4U] * 256U + (GM_RSSP1_UINT16)pdate[5U];

	GM_RSSP1_memcpy((void*)(dat_to_peer + i), (const void*)&peer_syn_sacepid,  sizeof(GM_RSSP1_SACEPID_struct));

	i += (GM_RSSP1_INT32)sizeof(GM_RSSP1_SACEPID_struct);

	peer_syn_sacepid.source_addr    = (GM_RSSP1_UINT16)pdate[2U] * 256U + (GM_RSSP1_UINT16)pdate[3U];

	peer_syn_sacepid.dest_addr      = GM_RSSP1_SYN_DEST_ADDR;

	GM_RSSP1_memcpy((void*)(dat_to_peer+i), (const void*)&byte_count, sizeof(GM_RSSP1_UINT16));

	i += (GM_RSSP1_INT32)sizeof(GM_RSSP1_UINT16);

	GM_RSSP1_memcpy((void*)(dat_to_peer+i), (const void*)(pdate+6U), (size_t)byte_count);


	i += (GM_RSSP1_INT32)byte_count;

	/* add crc16 */
	GM_RSSP1_CRC_Calculate_CRC16(dat_to_peer, (GM_RSSP1_UINT32)i, &crc16, GM_RSSP1_CRC16_Table_0x11021_MSB);

	dat_to_peer[i] = (GM_RSSP1_UINT8)((crc16 & 0xff00U) >> 8U);
	i += 1;
	dat_to_peer[i]   = (GM_RSSP1_UINT8)(crc16 & 0xffU);
	i += 1;

	GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 1U);
	for (l_index = 0U; l_index < pFSFB_SFM->connection_nums ; ++l_index)
	{
		con_index = GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 0U);
		/*if (0xFFFFU != con_index)*/
		if(con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
		{
			pSFM_con = &(pFSFB_SFM->connection[con_index]);

			if (GM_RSSP1_TRUE == GM_RSSP1_SFM_Is_SaCEPID_Equal(&peer_syn_sacepid , &(pSFM_con->SaCEPID)))
			{
				GM_RSSP1_CFM_User_Send_Dat(&peer_syn_sacepid , dat_to_peer , (GM_RSSP1_UINT16)i);
			}
		}
	}
}
#endif

void GM_RSSP1_Syn_Proc_Rcvd_Syn_Dat(GM_RSSP1_UINT8 *pDat , GM_RSSP1_INT32 dat_len)
{
    GM_RSSP1_Syn_Info_struct syn_info;      /* 同步过来的连接状态数据 */
    GM_RSSP1_UINT8 interface_type = 0U;
    GM_RSSP1_UINT8 syn_dat_type = 0U;
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 syn_con_index = 0U;
    GM_RSSP1_INT32 syn_con_nums = 0;   /* 同步过来的连接数 */
    GM_RSSP1_SFM_connection_struct *pSFM_con = NULL;
    GM_RSSP1_SFM_object_struct *pSFM = NULL;
    GM_RSSP1_UINT16 crc16 = 0U;
    GM_RSSP1_UINT16 rcvd_crc16 = 0U;
    GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;
    GM_RSSP1_SFM_L2U_pri_struct dat_pri;
    GM_RSSP1_INT32  i = 0;
    GM_RSSP1_UINT32 cal_crc32 = 0U;
    GM_RSSP1_SACEPID_struct peer_syn_sacepid = {0};
    GM_RSSP1_INT32  offset = 0;

    pSFM = GM_RSSP1_Get_SFM_Object();
    if ((pDat == NULL) ||(pSFM == NULL))
    {
        return;
    }

    interface_type = *(pDat);
    i++;
    if((INTERFACE_TYPE_SYN != interface_type)||(dat_len<0))
    {
        GM_RSSP1_Log_Msg(2,"RSSP1 syn rcv interface type err:0x%X.\n",(int)interface_type,(int)dat_len,0,0,0,0);
        return;
    }
    
    /* check crc16 first */
    GM_RSSP1_CRC_Calculate_CRC16(pDat, ((GM_RSSP1_UINT32)dat_len - (GM_RSSP1_UINT32)2), &crc16, GM_RSSP1_CRC16_Table_0x11021_MSB);
    rcvd_crc16 = ((GM_RSSP1_UINT16)(*(pDat + dat_len - 2)) << 8U) + (GM_RSSP1_UINT16)(*(pDat + dat_len - 1));

    if (rcvd_crc16 != crc16)
    {
        GM_RSSP1_Log_Msg(2,"Rcvd Syn dat from peer,%d bytes , CRC16 err! rcv:0x%X,cal:0x%X.\n", (int)dat_len, (int)rcvd_crc16, (int)crc16, 0, 0, 0);
        return;
    }

    syn_dat_type = *(pDat + 1U);
    i++;
    switch (syn_dat_type)
    {
        case INTERFACE_DAT_TYPE_SYN_RSSP1_STATUS_DAT:
            GM_RSSP1_memcpy((void*)&peer_syn_sacepid, (const void*)(pDat + i), sizeof(GM_RSSP1_SACEPID_struct));
            i += (GM_RSSP1_INT32)sizeof(GM_RSSP1_SACEPID_struct);

			GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 1U);
            for (l_index = 0U; l_index < pSFM->connection_nums ; ++l_index)
            {
				offset = i + ((GM_RSSP1_INT32)l_index) * ((GM_RSSP1_INT32)sizeof(syn_info));
				GM_RSSP1_memcpy((void*)&syn_info , (const void*)(pDat + offset), sizeof(syn_info));
				syn_con_index = GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 0U);
				/*if (0xFFFFU != syn_con_index)*/
				if(syn_con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
				{
					pSFM_con = &(pSFM->connection[syn_con_index]);
					if (GM_RSSP1_FALSE == GM_RSSP1_SFM_Is_SaCEPID_Equal(&peer_syn_sacepid , &(pSFM_con->SaCEPID)))
					{
						/* if local is standby,set local fsfb connection state. */
						pSFM_con->time.TC       = syn_info.time_TC;
						pSFM_con->time.TS[0U]    = syn_info.time_TS[0U];
						pSFM_con->time.TS[1U]    = syn_info.time_TS[1U];
						pSFM_con->time.oldTS[0U] = syn_info.time_oldTS[0U];
						pSFM_con->time.oldTS[1U] = syn_info.time_oldTS[1U];
						pSFM_con->env[0U] = syn_info.env[0U];
						pSFM_con->env[1U] = syn_info.env[1U];
	#ifndef GM_RSSP1_SYN_NO_DATA
						/*added by huang 20151215*/
						pSFM_con->PreciTime = syn_info.PreciTime;
						pSFM_con->SSE_SSR_Delay = syn_info.SSE_SSR_Delay;
						pSFM_con->SINIT_TremCycle = syn_info.SINIT_TremCycle;
						pSFM_con->RxSSRTrem = syn_info.RxSSRTrem;
						pSFM_con->RxSSRTloc = syn_info.RxSSRTloc;
						pSFM_con->TempDiffTime = syn_info.TempDiffTime;
						pSFM_con->TcycleLoc = syn_info.TcycleLoc;
						pSFM_con->TcycleRem = syn_info.TcycleRem;
						/*the end 20151215*/
	#endif
					}
					else
					{
						l_index -= 1U;	/*jcf:抵消因desAddr=FFFF而导致的同步包错位，发送的同步包只有正常连接的数据，接收同步包也只应处理正常连接(跳过同步连接)*/
					}
				}
            }
            break;
			
        case INTERFACE_DAT_TYPE_SYN_RSSP1_ACTIVE_DAT:
            /*9: Flag(1) + FSFB_SACEPID_struct(4) + bytes_count(2) + crc16(2)*/

            GM_RSSP1_memset((void*)&dat_pri, 0U, sizeof(dat_pri));
            dat_pri.type = GM_RSSP1_SFM_Active_Data_Ind;

            GM_RSSP1_memcpy((void*)&(dat_pri.SaCEPID), (const void*)(pDat + i), sizeof(GM_RSSP1_SACEPID_struct));
            i += (GM_RSSP1_INT32)sizeof(GM_RSSP1_SACEPID_struct);

            pSFM_con = GM_RSSP1_SFM_Get_Connection_BY_SaCEPID(&(dat_pri.SaCEPID) , pSFM);
            if (NULL != pSFM_con)
            {
                if (dat_len > (GM_RSSP1_INT32)GM_RSSP1_MAX_SAFETY_DAT_LEN)
                {
                    return;
                }

                dat_pri.as.data_indi.bytes_count = (*(GM_RSSP1_UINT16*)(pDat + i));
                i += (GM_RSSP1_INT32)sizeof(GM_RSSP1_UINT16);

                GM_RSSP1_memcpy((void*)dat_pri.as.data_indi.byte, (const void*)(pDat + i), (size_t)dat_pri.as.data_indi.bytes_count);
                i += (GM_RSSP1_INT32)dat_pri.as.data_indi.bytes_count;

				/*@放入上层队列**/
                if (GM_RSSP1_TRUE == pSFM_con->b_enable_FSFB_on_chn)
                {
                    /* 直接储存到本通道内的队列 */
                    write_q_rt = FSFB_Msg_Queue_Write(&(pSFM_con->L2U_dat_Q) , &dat_pri);
                }
                else
                {
                    /* 存储到SFM的队列 */
                    write_q_rt = FSFB_Msg_Queue_Write(&(g_SFM_MQ.RCV_MQ/*pSFM->L2U_pri_Q*/) , &dat_pri);
                }
            }
            break;

        default:
            GM_RSSP1_Log_Msg(2,"SFM rcv unknow syn dat type:0x%X.\n", (int)syn_dat_type , 0, 0, 0, 0, 0);
            break;
    }
    return;
}

GM_RSSP1_BOOL GM_RSSP1_Syn_Data_OneNode_Get(GM_RSSP1_UINT8* pdata,GM_RSSP1_UINT32* plen,GM_RSSP1_UINT16 con_index,GM_RSSP1_UINT16 dest_addr)
{
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_object_struct *pSFM = NULL;
	GM_RSSP1_SFM_connection_struct *pSFM_con = NULL;
	GM_RSSP1_Syn_Extern_Info_struct syn_info;
	GM_RSSP1_INT32 dat_len = 0;

	if( (NULL == pdata) || (NULL == plen) ||(con_index >= GM_RSSP1_MAX_SAFETY_CONNECTION_NUM) || (GM_RSSP1_SYN_DEST_ADDR == dest_addr) )
	{
		GM_RSSP1_Log_Msg(1,"Get syn data of OneNode parameter error! con_index=%d,dest_addr=0x%X\n", (int)con_index, (int)dest_addr, (int)pdata, (int)plen, 0, 0);
		GM_RSSP1_Log_Usr(GM_RSSP1_Syn_Data_OneNode_Get_Param_Point_Error, (int)con_index, (int)dest_addr, (int)pdata, (int)plen, 0, 0);
		return GM_RSSP1_FALSE;
	}
	pSFM = GM_RSSP1_Get_SFM_Object();
	if (NULL == pSFM)
	{
		return GM_RSSP1_FALSE;
	}

	pSFM_con = &(pSFM->connection[con_index]);
	if (pSFM_con->SaCEPID.dest_addr != dest_addr)
	{
		GM_RSSP1_Log_Msg(1,"Syn dest_addr Not match the OneNode index.\n", (int)con_index, (int)pSFM_con->SaCEPID.dest_addr, (int)dest_addr, 0, 0, 0);
		GM_RSSP1_Log_Usr(GM_RSSP1_Syn_Data_OneNode_Get_Index_Addr_Not_Match_Error, (int)con_index, (int)pSFM_con->SaCEPID.dest_addr, (int)dest_addr, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	GM_RSSP1_memset((void*)&syn_info, 0U, sizeof(GM_RSSP1_Syn_Extern_Info_struct));
	syn_info.base_info.index = pSFM_con->index;
	syn_info.base_info.sacepid = pSFM_con->SaCEPID;
	syn_info.base_info.time_TC    = pSFM_con->time.TC;
	syn_info.base_info.time_TS[0U] = pSFM_con->time.TS[0U];
	syn_info.base_info.time_TS[1U] = pSFM_con->time.TS[1U];
	syn_info.base_info.time_oldTS[0U] = pSFM_con->time.oldTS[0U];
	syn_info.base_info.time_oldTS[1U] = pSFM_con->time.oldTS[1U];
	syn_info.base_info.remote_dev_AS_state = pSFM_con->remote_dev_AS_state;
	GM_RSSP1_memcpy((void*)syn_info.base_info.env, (const void*)pSFM_con->env, ((size_t)GM_RSSP1_CHECK_CHN_NUM*(sizeof(GM_RSSP1_CHN_ENV))));
	syn_info.state = pSFM_con->state;
	/*syn_info.state_check_counter1 = pSFM_con->state_check_counter1;
	syn_info.state_check_counter2 = pSFM_con->state_check_counter2;*/
#ifndef GM_RSSP1_SYN_NO_DATA
	syn_info.base_info.PreciTime = pSFM_con->PreciTime;
	syn_info.base_info.SSE_SSR_Delay = pSFM_con->SSE_SSR_Delay;
	syn_info.base_info.SINIT_TremCycle = pSFM_con->SINIT_TremCycle;
	syn_info.base_info.RxSSRTrem = pSFM_con->RxSSRTrem;
	syn_info.base_info.RxSSRTloc = pSFM_con->RxSSRTloc;
	syn_info.base_info.TempDiffTime = pSFM_con->TempDiffTime;
	syn_info.base_info.TcycleLoc = pSFM_con->TcycleLoc;
	syn_info.base_info.TcycleRem = pSFM_con->TcycleRem;
	syn_info.b_rcvd_new_dat_in_cycle = pSFM_con->b_rcvd_new_dat_in_cycle;
	syn_info.b_torlerant_dat_valid = pSFM_con->b_torlerant_dat_valid;
	syn_info.torlerate_cycle_count1 = pSFM_con->torlerate_cycle_count1;
#ifdef RSSP1_TolerateWithCycle
	syn_info.torlerated_cycle_count2 = pSFM_con->torlerated_cycle_count2;
#endif
	syn_info.dat_count = pSFM_con->last_valid_dat_pri.as.data_indi.bytes_count;
#endif

	GM_RSSP1_memcpy((void*)pdata, (const void*)&syn_info , sizeof(GM_RSSP1_Syn_Extern_Info_struct));
	dat_len = (GM_RSSP1_INT32)sizeof(GM_RSSP1_Syn_Extern_Info_struct);

#ifndef GM_RSSP1_SYN_NO_DATA

	/*printf("the usr syn data len =%d\n",syn_info.dat_count );*/
	if(pSFM_con->b_torlerant_dat_valid == GM_RSSP1_TRUE)
	{
		/* defect: 13995432 */
		GM_RSSP1_memcpy((void*)(pdata+dat_len), (const void*)&(pSFM_con->last_valid_dat_pri), 
			(size_t)(sizeof(GM_RSSP1_SFM_L2U_pri_struct)-sizeof(pSFM_con->last_valid_dat_pri.as)+ syn_info.dat_count+sizeof(GM_RSSP1_UINT16)));/*只同步用户数据的有效长度*/
		dat_len += (GM_RSSP1_INT32)(sizeof(GM_RSSP1_SFM_L2U_pri_struct)-sizeof(pSFM_con->last_valid_dat_pri.as)+ syn_info.dat_count+sizeof(GM_RSSP1_UINT16));
		GM_RSSP1_memcpy((void*)(pdata+dat_len), (const void*)pSFM_con->last_valid_dat_pri.as.data_indi.CRC, (size_t)(sizeof(GM_RSSP1_UINT32)*2));
		dat_len +=(GM_RSSP1_INT32)(sizeof(GM_RSSP1_UINT32)*2);
	}
#endif

	*plen = (GM_RSSP1_UINT32)dat_len;
	return GM_RSSP1_TRUE;
}


/* CR: 11036019*/
GM_RSSP1_BOOL GM_RSSP1_Syn_OnLineData_Get(GM_RSSP1_Pointer* pdata, GM_RSSP1_UINT32* len)
{
	GM_RSSP1_SFM_object_struct *pSFM = NULL;
	GM_RSSP1_SFM_connection_struct *pSFM_con = NULL;
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_UINT16 onlineNum = 0U;
	/*GM_RSSP1_Syn_Extern_Info_struct syn_info = {0};*/
	GM_RSSP1_INT32 state_dat_len = 0;
	GM_RSSP1_UINT16 msg_num = 0U;
	GM_RSSP1_SeqDele_Record_Structure* pRec = NULL;
	static GM_RSSP1_BOOL FirstCallFlag = GM_RSSP1_TRUE;
	static GM_RSSP1_UINT32 synDataSize = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_UINT32 synlen = 0U;
	static GM_RSSP1_UINT8* synData = NULL;

	pSFM = GM_RSSP1_Get_SFM_Object();
	if ((NULL == pSFM)||(NULL == pdata)||(NULL == len))
	{
		return bRt;
	}


	pRec = GM_RSSP1_SeqDeleteRecord_Get();/**当前周期记录的状态变化的链接**/
	if (NULL == pRec)
	{
		return bRt;
	}
	
	if (GM_RSSP1_Lib_State_Operational != pSFM->lib_state)
	{
		return bRt;
	}

	if (0U == pSFM->connection_nums)
	{
		/* 只有一个连接，不可能存在同步通道 */
		return bRt;
	}

	if (FirstCallFlag == GM_RSSP1_TRUE)
	{
		synDataSize = g_Max_ConnctNum * (GM_RSSP1_UINT32)sizeof(GM_RSSP1_Syn_Extern_Info_struct);

#ifndef GM_RSSP1_SYN_NO_DATA
		synDataSize += (GM_RSSP1_UINT32)g_Max_ConnctNum * (GM_RSSP1_UINT32)sizeof(GM_RSSP1_SFM_L2U_pri_struct);
#endif
		synDataSize +=sizeof(GM_RSSP1_UINT16);

		synData = (GM_RSSP1_UINT8*)GM_RSSP1_malloc((size_t)synDataSize);

		if (synData == NULL)
		{
			return bRt;
		}

		FirstCallFlag = GM_RSSP1_FALSE;
	}

	GM_RSSP1_memset((void*)synData, 0U, (size_t)synDataSize);

	onlineNum = GM_RSSP1_GetSeqAmount(OnLine_Object);
	GM_RSSP1_GetSeqIndexPositive(OnLine_Object, 1U);

	msg_num = onlineNum; /**同步总节点数为 当前活动节点**/

	GM_RSSP1_memcpy((void*)synData, (const void*)&msg_num, sizeof(GM_RSSP1_UINT16));
	state_dat_len += (GM_RSSP1_INT32)sizeof(GM_RSSP1_UINT16);

	for (l_index = 0U; l_index < onlineNum; ++l_index) /**活动节点的**/
	{
		con_index = GM_RSSP1_GetSeqIndexPositive(OnLine_Object, 0U);
		
		if((0xFFFFU != con_index) && (con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM))/*CR5746*/
		{
			pSFM_con = &(pSFM->connection[con_index]);

			if( GM_RSSP1_TRUE == GM_RSSP1_Syn_Data_OneNode_Get(synData+state_dat_len,&synlen,con_index,pSFM_con->dest_addr) )
			{
				state_dat_len += (GM_RSSP1_INT32)synlen;
			}
		}
	}

	pRec->RecordNum = 0U;

	if ((GM_RSSP1_INT32)synDataSize >= state_dat_len)
	{
		*pdata = (GM_RSSP1_UINT32)synData;
		*len = (GM_RSSP1_UINT32)state_dat_len;
		bRt = GM_RSSP1_TRUE;
	}
	
	return bRt;
}

/* CR: 11036019*/
GM_RSSP1_BOOL GM_RSSP1_Syn_AllData_Get(GM_RSSP1_Pointer* pdata, GM_RSSP1_UINT32* len)
{
	GM_RSSP1_SFM_object_struct *pSFM = NULL;
	GM_RSSP1_SFM_connection_struct *pSFM_con = NULL;
	GM_RSSP1_UINT16 index = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_INT32 state_dat_len = 0;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	static GM_RSSP1_BOOL FirstCallFlag = GM_RSSP1_TRUE;
	static GM_RSSP1_UINT32 synDataSize = 0U;
	GM_RSSP1_UINT32 synlen = 0;
	static GM_RSSP1_UINT8* synData = NULL;
	GM_RSSP1_UINT16 ValidIndex = 0xFFFFU;
	GM_RSSP1_UINT16 FreeIndex = 0xFFFFU;
    GM_RSSP1_UINT32 onlineNum = 0U;

	pSFM = GM_RSSP1_Get_SFM_Object();
	if ((NULL == pSFM)||(NULL == pdata)||(NULL == len))
	{
		return bRt;
	}

	if (GM_RSSP1_Lib_State_Operational != pSFM->lib_state)
	{
		return bRt;
	}

	if (0U == pSFM->connection_nums)
	{
		/* 只有一个连接，不可能存在同步通道 */
		return bRt;
	}

	if (FirstCallFlag == GM_RSSP1_TRUE)
	{
		synDataSize = pSFM->connection_nums * sizeof(GM_RSSP1_Syn_Extern_Info_struct);

#ifndef GM_RSSP1_SYN_NO_DATA
		synDataSize += pSFM->connection_nums * sizeof(GM_RSSP1_SFM_L2U_pri_struct);
#endif
		synDataSize +=sizeof(GM_RSSP1_UINT16);	/*jcf:除了同步包数据，还需扩充第一个字段syn_num所占字节数*/

		synData = (GM_RSSP1_UINT8*)GM_RSSP1_malloc((size_t)synDataSize);

		if (synData == NULL)
		{
			return bRt;
		}
		FirstCallFlag = GM_RSSP1_FALSE;
	}

	GM_RSSP1_memset((void*)synData, 0U, synDataSize);

	GM_RSSP1_memcpy((void*)synData, (const void*)&pSFM->connection_nums, sizeof(GM_RSSP1_UINT16));
	state_dat_len += sizeof(GM_RSSP1_UINT16);

	/*CR: 7293 同步之前先将所有Online的状态全部置位*/
    onlineNum = GM_RSSP1_GetSeqAmount(OnLine_Object);
    GM_RSSP1_GetSeqIndexReverse(OnLine_Object, 1U);
    for (index=0U; index<onlineNum; ++index)        /*变量应各行其道，i只负责循环、index只负责索引，如此才能避免混用*/
    {
        con_index = GM_RSSP1_GetSeqIndexReverse(OnLine_Object, 0U);
        /*if (0xFFFFU != l_index)*/
        if(con_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
        {
            pSFM_con = &(pSFM->connection[con_index]);
            
            if (GM_RSSP1_FALSE == pSFM_con->IsFixed)
            {
                bRt = GM_RSSP1_Hash_Search_DstAndIndex((GM_RSSP1_UINT32)pSFM_con->dest_addr, con_index, &ValidIndex, &FreeIndex);
                bRt = GM_RSSP1_Hash_Insert_Online(ValidIndex, GM_RSSP1_FALSE);          
            }
        }
    }
	
#ifdef CFM_Part
    GM_RSSP1_Clear_Active();
#endif
	GM_RSSP1_SeqArrayClean(OnLine_Object);

	GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 1U);
	for (index = 0U ; index < pSFM->connection_nums ; ++index)
	{
		con_index = GM_RSSP1_GetSeqIndexPositive(OffLine_Object, 0U);
		if ((0xFFFFU != con_index) && (con_index < pSFM->connection_nums))/*CR5746*/
		{
			pSFM_con = &(pSFM->connection[con_index]);

			if( GM_RSSP1_TRUE == GM_RSSP1_Syn_Data_OneNode_Get(synData+state_dat_len,&synlen,con_index,pSFM_con->dest_addr) )
			{
				state_dat_len += synlen;
			}

			if ((GM_RSSP1_TRUE == pSFM_con->IsFixed) || (GM_RSSP1_Layer_State_Connected == pSFM_con->state) || (GM_RSSP1_Layer_State_Connecting == pSFM_con->state))
			{
				bRt = GM_RSSP1_Hash_Search_DstAndIndex((GM_RSSP1_UINT32)pSFM_con->dest_addr, pSFM_con->index, &ValidIndex, &FreeIndex);
				bRt = GM_RSSP1_Hash_Insert_Online(ValidIndex, GM_RSSP1_TRUE);
#ifdef CFM_Part
				GM_RSSP1_Active_Add(con_index,1);
#endif
			}
		}
	}

	if ((GM_RSSP1_INT32)synDataSize >= state_dat_len)
	{
		*pdata = (GM_RSSP1_Pointer)synData; /* CR: 13800904 */
		*len = (GM_RSSP1_UINT32)state_dat_len;
		bRt = GM_RSSP1_TRUE;
	}
	else
	{
		bRt = GM_RSSP1_FALSE;
	}

	return bRt;
}

GM_RSSP1_BOOL GM_RSSP1_Syn_Data_OneNode_Set(GM_RSSP1_UINT8* pdata,GM_RSSP1_UINT32* plen)
{
	GM_RSSP1_Syn_Extern_Info_struct syn_info;      /* 同步过来的连接状态数据 */
	GM_RSSP1_SFM_object_struct *pSFM = NULL;
	GM_RSSP1_SFM_connection_struct *pSFM_con = NULL;
	GM_RSSP1_UINT16 ValidIndex = 0xFFFFU;
	GM_RSSP1_UINT16 FreeIndex = 0xFFFFU;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_UINT32 dataLen =0U;

	pSFM = GM_RSSP1_Get_SFM_Object();
	if ((NULL == pSFM)||(NULL == pdata)||(NULL == plen))
	{
		GM_RSSP1_Log_Msg(1,"Set syn data of OneNode parameter error! \n", (int)pSFM, (int)pdata, (int)plen, 0, 0, 0);
		GM_RSSP1_Log_Usr(GM_RSSP1_Syn_Data_OneNode_Set_Param_Point_Error, (int)pSFM, (int)pdata, (int)plen, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}

	GM_RSSP1_memcpy((void*)&syn_info, (const void*)pdata, sizeof(GM_RSSP1_Syn_Extern_Info_struct) );

	if (syn_info.base_info.index < GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)/*CR 5746*/
	{
		pSFM_con = &(pSFM->connection[syn_info.base_info.index]);
		if (GM_RSSP1_TRUE == GM_RSSP1_SFM_Is_SaCEPID_Equal(&(syn_info.base_info.sacepid) , &(pSFM_con->SaCEPID)))
		{
			/* if local is standby,set local fsfb connection state. */
			pSFM_con->time.TC = syn_info.base_info.time_TC;
			pSFM_con->time.TS[0U] = syn_info.base_info.time_TS[0U];
			pSFM_con->time.TS[1U] = syn_info.base_info.time_TS[1U];
			pSFM_con->time.oldTS[0U] = syn_info.base_info.time_oldTS[0U];
			pSFM_con->time.oldTS[1U] = syn_info.base_info.time_oldTS[1U];
			pSFM_con->remote_dev_AS_state = syn_info.base_info.remote_dev_AS_state;
			GM_RSSP1_memcpy((void*)pSFM_con->env, (const void*)syn_info.base_info.env, (size_t)(sizeof(GM_RSSP1_CHN_ENV)*2U));

#ifndef GM_RSSP1_SYN_NO_DATA
			pSFM_con->b_rcvd_new_dat_in_cycle = syn_info.b_rcvd_new_dat_in_cycle;
			pSFM_con->b_torlerant_dat_valid = syn_info.b_torlerant_dat_valid;
			pSFM_con->torlerate_cycle_count1 = syn_info.torlerate_cycle_count1;
#ifdef RSSP1_TolerateWithCycle
			pSFM_con->torlerated_cycle_count2 = syn_info.torlerated_cycle_count2;
#endif
			pSFM_con->last_valid_dat_pri.as.data_indi.bytes_count = syn_info.dat_count;
			/*added by huang 20151215*/
			pSFM_con->PreciTime = syn_info.base_info.PreciTime;
			pSFM_con->SSE_SSR_Delay = syn_info.base_info.SSE_SSR_Delay;
			pSFM_con->SINIT_TremCycle = syn_info.base_info.SINIT_TremCycle;
			pSFM_con->RxSSRTrem = syn_info.base_info.RxSSRTrem;
			pSFM_con->RxSSRTloc = syn_info.base_info.RxSSRTloc;
			pSFM_con->TempDiffTime = syn_info.base_info.TempDiffTime;
			pSFM_con->TcycleLoc = syn_info.base_info.TcycleLoc;
			pSFM_con->TcycleRem = syn_info.base_info.TcycleRem;
			/*the end 20151215*/
#endif
			pSFM_con->state = syn_info.state;
			/*pSFM_con->state_check_counter1 = syn_info.state_check_counter1;
			pSFM_con->state_check_counter2 = syn_info.state_check_counter2;*/

			if ((GM_RSSP1_TRUE == pSFM_con->IsFixed) || (GM_RSSP1_Layer_State_Connected == pSFM_con->state) || (GM_RSSP1_Layer_State_Connecting == pSFM_con->state))
			{
				bRt = GM_RSSP1_Hash_Search_DstAndIndex((GM_RSSP1_UINT32)pSFM_con->SaCEPID.dest_addr, pSFM_con->index, &ValidIndex, &FreeIndex);/**寻找hash索引值**/
				if (GM_RSSP1_TRUE == bRt)
				{
					bRt = GM_RSSP1_Hash_Insert_Online(ValidIndex, GM_RSSP1_TRUE);/**根据hash索引值,将该通道归入online**/
					if (GM_RSSP1_TRUE != bRt)
					{
						return GM_RSSP1_FALSE;
					}
				}
#ifdef CFM_Part
				GM_RSSP1_Active_Add(pSFM_con->index,1U);
#endif
			}
			dataLen = sizeof(GM_RSSP1_Syn_Extern_Info_struct);

#ifndef GM_RSSP1_SYN_NO_DATA
			if(pSFM_con->b_torlerant_dat_valid == GM_RSSP1_TRUE)
			{
				/* defect: 13995432 */
				GM_RSSP1_memset((void*)&(pSFM_con->last_valid_dat_pri), 0U, sizeof(GM_RSSP1_SFM_L2U_pri_struct));

				GM_RSSP1_memcpy((void*)&(pSFM_con->last_valid_dat_pri), (const void*)(pdata + dataLen), 
					(size_t)(sizeof(GM_RSSP1_SFM_L2U_pri_struct)-sizeof(pSFM_con->last_valid_dat_pri.as)+ syn_info.dat_count+sizeof(GM_RSSP1_UINT16)));/*只设值用户数据的有效长度*/
				dataLen += (GM_RSSP1_INT32)(sizeof(GM_RSSP1_SFM_L2U_pri_struct)-sizeof(pSFM_con->last_valid_dat_pri.as)+ syn_info.dat_count+sizeof(GM_RSSP1_UINT16));
				GM_RSSP1_memcpy((void*)pSFM_con->last_valid_dat_pri.as.data_indi.CRC, (const void*)(pdata + dataLen),(size_t)(sizeof(GM_RSSP1_UINT32)*2));
				dataLen += (GM_RSSP1_INT32)(sizeof(GM_RSSP1_UINT32)*2);
			}
			else
			{
				GM_RSSP1_memset((void*)&(pSFM_con->last_valid_dat_pri), 0U, sizeof(GM_RSSP1_SFM_L2U_pri_struct));	/*若未收到数据，则直接清零跳过*/
			}

#endif
			*plen = dataLen;
		}
		else
		{
			GM_RSSP1_Log_Msg(1,"Set syn data of OneNode src & dest not match error! \n", (int)syn_info.base_info.index, (int)syn_info.base_info.sacepid.source_addr, (int)syn_info.base_info.sacepid.dest_addr, (int)pSFM_con->SaCEPID.source_addr, (int)pSFM_con->SaCEPID.dest_addr, 0);
			GM_RSSP1_Log_Usr(GM_RSSP1_Syn_Data_OneNode_Set_SaCEPID_Not_Match_Error, (int)syn_info.base_info.index, (int)syn_info.base_info.sacepid.source_addr, (int)syn_info.base_info.sacepid.dest_addr, (int)pSFM_con->SaCEPID.source_addr, (int)pSFM_con->SaCEPID.dest_addr, 0);
			return GM_RSSP1_FALSE;
		}

		return GM_RSSP1_TRUE;
	}
	else
	{
		return GM_RSSP1_FALSE;
	}

}

GM_RSSP1_BOOL GM_RSSP1_Syn_Data_Set(GM_RSSP1_UINT8* pdata, GM_RSSP1_UINT32 dataSize)
{
	GM_RSSP1_UINT16 i = 0U;
	GM_RSSP1_UINT16 l_index = 0U;
	GM_RSSP1_UINT16 ValidIndex = 0xFFFFU;
	GM_RSSP1_UINT16 FreeIndex = 0xFFFFU;
	GM_RSSP1_INT32 syn_con_index = 0;
	GM_RSSP1_INT32 syn_con_nums = 0;   /* 同步过来的连接数 */
	GM_RSSP1_INT32 state_dat_len = 0;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_SFM_object_struct *pSFM = NULL;
	GM_RSSP1_SFM_connection_struct *pSFM_con = NULL;
	GM_RSSP1_UINT32 synLen = 0U;
	GM_RSSP1_UINT32 onlineNum = 0U;
	
	pSFM = GM_RSSP1_Get_SFM_Object();
	if ((NULL == pSFM)||(NULL == pdata))
	{
		return GM_RSSP1_FALSE;
	}

	syn_con_nums = *(GM_RSSP1_UINT16*)pdata;
	state_dat_len += (GM_RSSP1_INT32)sizeof(GM_RSSP1_UINT16);
	if(syn_con_nums == 0)
	{
		return GM_RSSP1_TRUE;
	}

	if ((GM_RSSP1_Lib_State_Operational != pSFM->lib_state)||(dataSize <(GM_RSSP1_UINT32)sizeof(GM_RSSP1_Syn_Extern_Info_struct)))
	{
		return GM_RSSP1_FALSE;
	}

	/*同步之前先将所有Online的状态全部置位*/
	onlineNum = GM_RSSP1_GetSeqAmount(OnLine_Object);
	GM_RSSP1_GetSeqIndexReverse(OnLine_Object, 1U);
	for (i=0U; i<onlineNum; ++i)		/*变量应各行其道，i只负责循环、index只负责索引，如此才能避免混用*/
	{
		l_index = GM_RSSP1_GetSeqIndexReverse(OnLine_Object, 0U);
		/*if (0xFFFFU != l_index)*/
		if(l_index < (GM_RSSP1_UINT16)GM_RSSP1_MAX_SAFETY_CONNECTION_NUM)
		{
			pSFM_con = &(pSFM->connection[l_index]);
			
			if (GM_RSSP1_FALSE == pSFM_con->IsFixed)
			{
	            bRt = GM_RSSP1_Hash_Search_DstAndIndex((GM_RSSP1_UINT32)pSFM_con->dest_addr, l_index, &ValidIndex, &FreeIndex);
	            bRt = GM_RSSP1_Hash_Insert_Online(ValidIndex, GM_RSSP1_FALSE);		    
			}
		}
	}

#ifdef CFM_Part
	GM_RSSP1_Clear_Active();
#endif
	GM_RSSP1_SeqArrayClean(OnLine_Object);

    
	for (syn_con_index = 0 ; syn_con_index < syn_con_nums ; ++syn_con_index)
	{
		if( GM_RSSP1_TRUE == GM_RSSP1_Syn_Data_OneNode_Set(pdata + state_dat_len, &synLen) )
		{
			state_dat_len += (GM_RSSP1_INT32)synLen;
		}else{ 
			break;
		}
	}
	
	if ((GM_RSSP1_INT32)dataSize == state_dat_len)
	{
		return GM_RSSP1_TRUE;
	}
	else
	{
		GM_RSSP1_Log_Msg(1,"Set syn data datasize error! \n", (int)dataSize, (int)state_dat_len, 0, 0, 0, 0);
		GM_RSSP1_Log_Usr(GM_RSSP1_Syn_Data_Set_Datasize_Error, (int)dataSize, (int)state_dat_len, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
}
#endif
