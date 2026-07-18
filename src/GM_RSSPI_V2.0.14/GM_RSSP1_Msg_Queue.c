
#include "GM_RSSP1_Msg_Queue.h"

/* CR: 11036019*/
GM_RSSP1_BOOL FSFB_Msg_Queue_Init(GM_RSSP1_Msg_Queue_struct *pQueuestruct , GM_RSSP1_UINT32 Q_size , GM_RSSP1_Pointer Q_offset , GM_RSSP1_Msg_Queue_TYPE_Enum Qtype)
{
	if ((NULL == pQueuestruct) || (0U == Q_offset))
	{
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_FALSE == GM_RSSP1_Mutex_Init(&(pQueuestruct->mutex)))
	{
		return GM_RSSP1_FALSE;
	}

	pQueuestruct->type          = Qtype;

	pQueuestruct->head_Position = 0U;
	pQueuestruct->tail_Position = 0U;
	pQueuestruct->count         = 0U;
	pQueuestruct->Q_size        = Q_size;
	pQueuestruct->Q_offset      = Q_offset;

	return GM_RSSP1_TRUE;
}

GM_RSSP1_BOOL FSFB_Msg_Queue_Clear(GM_RSSP1_Msg_Queue_struct *pQueuestruct)
{
	if (NULL == pQueuestruct)
	{
		return GM_RSSP1_FALSE;
	}
	if (GM_RSSP1_FALSE == GM_RSSP1_Mutex_Lock(pQueuestruct->mutex))
	{
		return GM_RSSP1_FALSE;
	}
	pQueuestruct->head_Position = 0U;

	pQueuestruct->tail_Position = 0U;
	pQueuestruct->count         = 0U;
	if (GM_RSSP1_FALSE == GM_RSSP1_Mutex_Unlock(pQueuestruct->mutex))
	{
		return GM_RSSP1_FALSE;
	}
	return GM_RSSP1_TRUE;
}

GM_RSSP1_BOOL FSFB_Is_Msg_Queue_Empty(GM_RSSP1_Msg_Queue_struct *pQueuestruct)
{
	if (NULL == pQueuestruct)
	{
		return GM_RSSP1_TRUE;
	}

	if (pQueuestruct->count > 0U)
	{
		return GM_RSSP1_FALSE;
	}
	else
	{
		return GM_RSSP1_TRUE;
	}
}

GM_RSSP1_BOOL FSFB_Is_Msg_Queue_Full(GM_RSSP1_Msg_Queue_struct *pQueuestruct)
{
	if (NULL == pQueuestruct)
	{
		return GM_RSSP1_FALSE;
	}

	if (pQueuestruct->count == pQueuestruct->Q_size)
	{
		return GM_RSSP1_TRUE;
	}
	else
	{
		return GM_RSSP1_FALSE;
	}
}

GM_RSSP1_UINT32 FSFB_Get_Queue_Count(GM_RSSP1_Msg_Queue_struct *pQueuestruct)
{
	if (NULL == pQueuestruct)
	{
		return 0;
	}

	if (pQueuestruct->count > pQueuestruct->Q_size)
	{
		return 0;
	}

	return pQueuestruct->count;
}

GM_RSSP1_BOOL FSFB_Msg_Queue_Get(GM_RSSP1_Msg_Queue_struct *pQueuestruct, void *pQElement)
{
	GM_RSSP1_SFM_U2L_pri_struct *pSFM_U2L_pri = NULL;
	GM_RSSP1_SFM_L2U_pri_struct *pSFM_L2U_pri = NULL;
	GM_RSSP1_CFM_U2L_pri_struct *pCFM_U2L_pri = NULL;
	GM_RSSP1_CFM_L2U_pri_struct *pCFM_L2U_pri = NULL;
	GM_RSSP1_com_input_pri_struct *precv_pri = NULL;
	GM_RSSP1_com_out_pri_struct *psend_pri = NULL;

	if ((NULL == pQueuestruct) || (NULL == pQElement))
	{
		return GM_RSSP1_FALSE;
	}

	if ((GM_RSSP1_UINT32)NULL == (pQueuestruct->Q_offset))
	{
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_TRUE == FSFB_Is_Msg_Queue_Empty(pQueuestruct))
	{
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_FALSE == GM_RSSP1_Mutex_Lock(pQueuestruct->mutex))
	{
		return GM_RSSP1_FALSE;
	}
	switch (pQueuestruct->type)
	{

	case FSFB_Q_TYPE_SFM_U2L_PRI:
		pSFM_U2L_pri = (GM_RSSP1_SFM_U2L_pri_struct *)((GM_RSSP1_SFM_U2L_pri_struct *)(pQueuestruct->Q_offset) + pQueuestruct->head_Position);
		GM_RSSP1_memcpy((void*)pQElement, (const void*)pSFM_U2L_pri, sizeof(GM_RSSP1_SFM_U2L_pri_struct));
		break;

	case FSFB_Q_TYPE_SFM_L2U_PRI:
		pSFM_L2U_pri = (GM_RSSP1_SFM_L2U_pri_struct *)((GM_RSSP1_SFM_L2U_pri_struct *)(pQueuestruct->Q_offset) + pQueuestruct->head_Position);
		GM_RSSP1_memcpy((void*)pQElement, (const void*)pSFM_L2U_pri, sizeof(GM_RSSP1_SFM_L2U_pri_struct));
		break;

	case FSFB_Q_TYPE_CFM_U2L_PRI:
		pCFM_U2L_pri = (GM_RSSP1_CFM_U2L_pri_struct *)((GM_RSSP1_CFM_U2L_pri_struct *)(pQueuestruct->Q_offset) + pQueuestruct->head_Position);
		GM_RSSP1_memcpy((void*)pQElement, (const void*)pCFM_U2L_pri, sizeof(GM_RSSP1_CFM_U2L_pri_struct));
		break;

	case FSFB_Q_TYPE_CFM_L2U_PRI:
		pCFM_L2U_pri = (GM_RSSP1_CFM_L2U_pri_struct *)((GM_RSSP1_CFM_L2U_pri_struct *)(pQueuestruct->Q_offset) + pQueuestruct->head_Position);
		GM_RSSP1_memcpy((void*)pQElement, (const void*)pCFM_L2U_pri, sizeof(GM_RSSP1_CFM_L2U_pri_struct));
		break;

	case FSFB_Q_TYPE_SENT_PKT:
		psend_pri = (GM_RSSP1_com_out_pri_struct *)((GM_RSSP1_com_out_pri_struct *)(pQueuestruct->Q_offset) + pQueuestruct->head_Position);
		GM_RSSP1_memcpy((void*)pQElement, (const void*)psend_pri, sizeof(GM_RSSP1_com_out_pri_struct));
		break;

	case FSFB_Q_TYPE_RCVD_PKT:
		precv_pri = (GM_RSSP1_com_input_pri_struct *)((GM_RSSP1_com_input_pri_struct *)(pQueuestruct->Q_offset) + pQueuestruct->head_Position);
		GM_RSSP1_memcpy((void*)pQElement, (const void*)precv_pri, sizeof(GM_RSSP1_com_input_pri_struct));
		break;
	default:
		GM_RSSP1_Mutex_Unlock(pQueuestruct->mutex);
		return GM_RSSP1_FALSE;
	}


	--(pQueuestruct->count);

	pQueuestruct->head_Position = (pQueuestruct->head_Position + 1U) % (pQueuestruct->Q_size);

	if (GM_RSSP1_FALSE == GM_RSSP1_Mutex_Unlock(pQueuestruct->mutex))
	{
		return GM_RSSP1_FALSE;
	}
	return GM_RSSP1_TRUE;
}

GM_RSSP1_Write_Q_Return_Enum FSFB_Msg_Queue_Write(GM_RSSP1_Msg_Queue_struct *pQueuestruct, void *pQElement)
{
	GM_RSSP1_SFM_U2L_pri_struct *pSFM_U2L_pri = NULL;
	GM_RSSP1_SFM_L2U_pri_struct *pSFM_L2U_pri = NULL;
	GM_RSSP1_CFM_U2L_pri_struct *pCFM_U2L_pri = NULL;
	GM_RSSP1_CFM_L2U_pri_struct *pCFM_L2U_pri = NULL;
	GM_RSSP1_com_input_pri_struct *precv_pri = NULL;
	GM_RSSP1_com_out_pri_struct *psend_pri = NULL;

	GM_RSSP1_BOOL bQ_full = GM_RSSP1_FALSE;

	if ((NULL == pQueuestruct) || (NULL == pQElement))
	{
		return Q_WRITE_FAIL;
	}

	if (0U >= pQueuestruct->Q_size)
	{
		return Q_WRITE_FAIL;
	}

	if (GM_RSSP1_TRUE == FSFB_Is_Msg_Queue_Full(pQueuestruct))
	{
		bQ_full     = GM_RSSP1_TRUE;

		if (GM_RSSP1_FALSE == FSFB_Msg_Queue_Pop(pQueuestruct))
		{
			return Q_WRITE_FAIL;
		}
	}

	if (GM_RSSP1_FALSE == GM_RSSP1_Mutex_Lock(pQueuestruct->mutex))
	{
		return Q_WRITE_FAIL;
	}

	++(pQueuestruct->count);

	switch (pQueuestruct->type)
	{

	case FSFB_Q_TYPE_SFM_U2L_PRI:
		pSFM_U2L_pri = (GM_RSSP1_SFM_U2L_pri_struct *)((GM_RSSP1_SFM_U2L_pri_struct *)(pQueuestruct->Q_offset) + pQueuestruct->tail_Position);
		GM_RSSP1_memcpy((void*)pSFM_U2L_pri, (const void*)pQElement, sizeof(GM_RSSP1_SFM_U2L_pri_struct));
		break;

	case FSFB_Q_TYPE_SFM_L2U_PRI:
		pSFM_L2U_pri = (GM_RSSP1_SFM_L2U_pri_struct *)((GM_RSSP1_SFM_L2U_pri_struct *)(pQueuestruct->Q_offset) + pQueuestruct->tail_Position);
		GM_RSSP1_memcpy((void*)pSFM_L2U_pri, (const void*)pQElement, sizeof(GM_RSSP1_SFM_L2U_pri_struct));
		break;

	case FSFB_Q_TYPE_CFM_U2L_PRI:
		pCFM_U2L_pri = (GM_RSSP1_CFM_U2L_pri_struct *)((GM_RSSP1_CFM_U2L_pri_struct *)(pQueuestruct->Q_offset) + pQueuestruct->tail_Position);
		GM_RSSP1_memcpy((void*)pCFM_U2L_pri, (const void*)pQElement, sizeof(GM_RSSP1_CFM_U2L_pri_struct));
		break;

	case FSFB_Q_TYPE_CFM_L2U_PRI:
		pCFM_L2U_pri = (GM_RSSP1_CFM_L2U_pri_struct *)((GM_RSSP1_CFM_L2U_pri_struct *)(pQueuestruct->Q_offset) + pQueuestruct->tail_Position);
		GM_RSSP1_memcpy((void*)pCFM_L2U_pri, (const void*)pQElement, sizeof(GM_RSSP1_CFM_L2U_pri_struct));
		break;

	case FSFB_Q_TYPE_SENT_PKT:
		psend_pri = (GM_RSSP1_com_out_pri_struct *)((GM_RSSP1_com_out_pri_struct *)(pQueuestruct->Q_offset) + pQueuestruct->tail_Position);
		GM_RSSP1_memcpy((void*)psend_pri, (const void*)pQElement, sizeof(GM_RSSP1_com_out_pri_struct));
		break;

	case FSFB_Q_TYPE_RCVD_PKT:
		precv_pri = (GM_RSSP1_com_input_pri_struct *)((GM_RSSP1_com_input_pri_struct *)(pQueuestruct->Q_offset) + pQueuestruct->tail_Position);
		GM_RSSP1_memcpy((void*)precv_pri, (const void*)pQElement, sizeof(GM_RSSP1_com_input_pri_struct));
		break;

	default:
		--(pQueuestruct->count);
		GM_RSSP1_Mutex_Unlock(pQueuestruct->mutex);
		return Q_WRITE_FAIL;
	}

	pQueuestruct->tail_Position = (pQueuestruct->tail_Position + 1U) % (pQueuestruct->Q_size);

	if (GM_RSSP1_FALSE == GM_RSSP1_Mutex_Unlock(pQueuestruct->mutex))
	{
		return Q_WRITE_FAIL;
	}

	if (GM_RSSP1_TRUE == bQ_full)
	{
		return Q_WRITE_OK_FULL;
	}
	else
	{
		return Q_WRITE_OK_NOT_FULL;
	}
}

GM_RSSP1_BOOL FSFB_Msg_Queue_Pop(GM_RSSP1_Msg_Queue_struct  *pQueuestruct)
{
	if (NULL == pQueuestruct)
	{
		return GM_RSSP1_FALSE;
	}

	if (GM_RSSP1_TRUE == FSFB_Is_Msg_Queue_Empty(pQueuestruct))
	{
		return GM_RSSP1_TRUE;
	}

	if (GM_RSSP1_FALSE == GM_RSSP1_Mutex_Lock(pQueuestruct->mutex))
	{
		return GM_RSSP1_FALSE;
	}

	--(pQueuestruct->count);

	pQueuestruct->head_Position = (pQueuestruct->head_Position + 1U) % (pQueuestruct->Q_size);

	if (GM_RSSP1_FALSE == GM_RSSP1_Mutex_Unlock(pQueuestruct->mutex))
	{
		return GM_RSSP1_FALSE;
	}

	return GM_RSSP1_TRUE;
}

