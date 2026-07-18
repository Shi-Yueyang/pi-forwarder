#include "GM_RSSP1_MQ_Interface.h"

RSSP1_MQ_Inter_struct g_CFM_MQ;

RSSP1_MQ_Inter_struct g_SFM_MQ;

#ifdef CFM_Stanby_Answer
GM_RSSP1_Msg_Queue_struct g_CFM_Debug_MQ = {0U};
#endif

GM_RSSP1_BOOL GM_RSSP1_MessageQueue_Inter_Set(GM_RSSP1_MQ_Type_Enum QType, GM_RSSP1_UINT8* pMsg)
{
	GM_RSSP1_Write_Q_Return_Enum write_q_rt = Q_WRITE_FAIL;
	GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;

	if (pMsg == NULL)
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_MessageQueue_Inter_Set_Param_Point_Error, 0, 0, 0, 0, 0, 0);
		return GM_RSSP1_FALSE;
	}
	switch(QType)
	{
		case RSSP1_S2C_SND:
			write_q_rt = FSFB_Msg_Queue_Write(&(g_CFM_MQ.SND_MQ) , pMsg);
			if (Q_WRITE_FAIL == write_q_rt)
			{
				GM_RSSP1_Log_Usr(GM_RSSP1_MessageQueue_Inter_Set_Write_Fail, RSSP1_S2C_SND, 0, 0, 0, 0, 0);
				return GM_RSSP1_FALSE;
			}
			else
			{
				rt = GM_RSSP1_TRUE;
			}
			break;

		case RSSP1_C2S_RCV:
			write_q_rt = FSFB_Msg_Queue_Write(&(g_CFM_MQ.RCV_MQ) , pMsg);
			if (Q_WRITE_FAIL == write_q_rt)
			{
				GM_RSSP1_Log_Usr(GM_RSSP1_MessageQueue_Inter_Set_Write_Fail, RSSP1_C2S_RCV, 0, 0, 0, 0, 0);
				return GM_RSSP1_FALSE;
			}
			else
			{
				rt = GM_RSSP1_TRUE;
			}
			break;
		default:
			GM_RSSP1_Log_Usr(GM_RSSP1_MessageQueue_Inter_Set_Write_Fail, 0, 0, 0, 0, 0, 0);
			rt = GM_RSSP1_FALSE;
			break;
	}
	return rt;
}

/*CR:GM00002527, GM162, fulintai,2013-06-09, much more return type for different exit*/
GM_RSSP1_INT8 GM_RSSP1_MessageQueue_Inter_Get(GM_RSSP1_MQ_Type_Enum QType, GM_RSSP1_UINT8* pMsg, GM_RSSP1_UINT32* len)
{
	GM_RSSP1_BOOL write_q_rt = GM_RSSP1_FALSE;
	GM_RSSP1_INT8 res = -1;
	GM_RSSP1_CFM_U2L_pri_struct S2C_Msg;
	GM_RSSP1_CFM_L2U_pri_struct C2S_Msg;
	GM_RSSP1_UINT32 lenth = 0U;

	if ((pMsg == NULL)||(len == NULL))
	{
		GM_RSSP1_Log_Usr(GM_RSSP1_MessageQueue_Inter_Get_Param_Point_Error, (int)pMsg, (int)len, 0, 0, 0, 0);
		return -1;
	}

	lenth = (GM_RSSP1_UINT32)sizeof(GM_RSSP1_CFM_L2U_type_enum)+(GM_RSSP1_UINT32)sizeof(GM_RSSP1_UINT16);

	switch(QType)
	{
		case RSSP1_S2C_SND:
			write_q_rt = FSFB_Msg_Queue_Get(&(g_CFM_MQ.SND_MQ) , &S2C_Msg);
			if (GM_RSSP1_FALSE == write_q_rt)
			{
				GM_RSSP1_Log_Msg(2,"Get message from S2C_SND Queue fail\n ", 0, 0, 0, 0, 0, 0);						
				return 0;
			}
			lenth = (GM_RSSP1_UINT32)sizeof(GM_RSSP1_CFM_U2L_pri_struct)- GM_RSSP1_MAX_SAFETY_DAT_LEN + (GM_RSSP1_UINT32)S2C_Msg.data_req.bytes_count;
			if (*len >= lenth)
			{
				GM_RSSP1_memcpy((void*)pMsg, (const void*)&S2C_Msg, (size_t)lenth);
				*len = lenth;
				res = 1;
			}
			break;

		case RSSP1_C2S_RCV:
			write_q_rt = FSFB_Msg_Queue_Get(&(g_CFM_MQ.RCV_MQ) , &C2S_Msg);
			if (GM_RSSP1_FALSE == write_q_rt)
			{				
				GM_RSSP1_Log_Msg(2,"Get message from C2S_RCV Queue fail\n ", 0, 0, 0, 0, 0, 0);				
				return 0;
			}
			switch(C2S_Msg.type)
			{
				case GM_RSSP1_CFM_Data_Ind:
					lenth += sizeof(GM_RSSP1_CFM_L2U_Dat_pri_struct)-GM_RSSP1_MAX_SAFETY_DAT_LEN+C2S_Msg.as.data_indi.bytes_count;
					if (*len >= lenth)
					{
						GM_RSSP1_memcpy((void*)pMsg, (const void*)&C2S_Msg, (size_t)lenth);
						*len = lenth;
						res = 1;
					}
					break;

				case GM_RSSP1_CFM_Indi_com_State:
					lenth += ((GM_RSSP1_UINT32)sizeof(GM_RSSP1_CFM_L2U_State_Noti_struct)/GM_RSSP1_MAX_ONLINE_CONNECTION_NUM) * (GM_RSSP1_UINT32)C2S_Msg.index;
					if (*len >= lenth)
					{
						GM_RSSP1_memcpy((void*)pMsg, (const void*)&C2S_Msg, (size_t)lenth);
						*len = lenth;
						res = 1;
					}
					break;

				case GM_RSSP1_CFM_Indi_Error_Noti:
					lenth += sizeof(GM_RSSP1_CFM_L2U_Error_Noti_struct);
					if (*len >= lenth)
					{
						*len = lenth;
						GM_RSSP1_memcpy((void*)pMsg, (const void*)&C2S_Msg, (size_t)lenth);
						res = 1;
					}
					break;

				default:
					res = -1;
					break;
			}
			break;
		default:
			GM_RSSP1_Log_Usr(GM_RSSP1_MessageQueue_Inter_Get_Write_Fail, QType, 0, 0, 0, 0, 0);
			res = -1;
			break;
	}
	return res;
}
