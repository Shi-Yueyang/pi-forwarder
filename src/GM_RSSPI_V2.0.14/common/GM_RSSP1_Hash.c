/**
* @file GM_RSSP1_Hash.c
* @brief CR:GM00004752
* @author Fu Lintai
* @date 2018-9-10 14:23:22
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Add the new file with hash algorithm for on-line connections efficiency  .</pre>
*/

#include "GM_RSSP1_Hash.h"

Hash_Table_SFM_Struct gSFMHashTable;


Hash_Table_CFM_Struct gCFMHashTable;


Hash_CFM_Active_Structure g_Active;

GM_RSSP1_BOOL bUpdate = GM_RSSP1_FALSE;
GM_RSSP1_Polyomial_struct g_Polyomia[2U] = {
	{(GM_RSSP1_UINT32 *)GM_RSSP1_LFSR_LEFT_CHN1_TABLE,(GM_RSSP1_UINT32 *)GM_RSSP1_LFSR_RIGHT_CHN1_TABLE, (GM_RSSP1_UINT32 *)GM_RSSP1_CRC32_Channel1_Table, (GM_RSSP1_UINT16 *)GM_RSSP1_CRC16_Table_0x10811_LSB},
	{(GM_RSSP1_UINT32 *)GM_RSSP1_LFSR_LEFT_CHN2_TABLE,(GM_RSSP1_UINT32 *)GM_RSSP1_LFSR_RIGHT_CHN2_TABLE, (GM_RSSP1_UINT32 *)GM_RSSP1_CRC32_Channel2_Table, (GM_RSSP1_UINT16 *)GM_RSSP1_CRC16_Table_0x10811_LSB}	
};

GM_RSSP1_Sequen_Structure g_OffLineArry = {0U};

GM_RSSP1_Sequen_Structure g_OnlineArray = {0U};

GM_RSSP1_SeqDele_Record_Structure g_DeleRecord = {0U};

GM_RSSP1_UINT32 GM_RSSP1_Next_Prime(GM_RSSP1_UINT32 limit)
{
    GM_RSSP1_UINT32 i = 0U;
    GM_RSSP1_UINT32 prime = 0U;

	prime = limit + 1U;
	
    while(1)
    {
        for (i = 2U; i < prime; ++i)
        {
            if (prime % i == 0U)
            {
                break;
            }
        }

        if (i == prime)
        {
            break;/**比limit大的素数**/
        }

		++prime;
    }

    return prime;
}

GM_RSSP1_BOOL GM_RSSP1_Hash_SFM_Init(void)
{	
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_UINT32 cycNum = 0U;

	/**申请Hash表数据空间**/
	gSFMHashTable.hashSize = GM_RSSP1_Next_Prime(GM_RSSP1_MAX_SAFETY_CONNECTION_NUM*2U);
	gSFMHashTable.sizeAvail = GM_RSSP1_MAX_SAFETY_CONNECTION_NUM;

    gSFMHashTable.SFMelem = (Hash_SFM_Elem_Struct *)GM_RSSP1_malloc((size_t)gSFMHashTable.hashSize * sizeof(Hash_SFM_Elem_Struct));
    if (gSFMHashTable.SFMelem != NULL)
    {
		for (cycNum = 0U; cycNum<gSFMHashTable.hashSize; ++cycNum)
		{
			gSFMHashTable.SFMelem[cycNum].Cnf_index = 0xFFFFU;
			gSFMHashTable.SFMelem[cycNum].Online_Status = GM_RSSP1_FALSE;
			gSFMHashTable.SFMelem[cycNum].DstAddr = 0U;
			gSFMHashTable.SFMelem[cycNum].DataType = GM_RSSP1_EMPTY;
		}
        bRt = GM_RSSP1_TRUE;
    }

	return GM_RSSP1_TRUE;
}

GM_RSSP1_UINT16 GM_RSSP1_Hash_Fill_SFM_Index(GM_RSSP1_UINT32 key, GM_RSSP1_UINT16 val)
{	
	GM_RSSP1_BOOL bRt = GM_RSSP1_TRUE;
	GM_RSSP1_UINT16 curindex = 0xFFFFU;
	GM_RSSP1_UINT16 FreeIndex = 0xFFFFU;

	bRt = GM_RSSP1_Hash_Search_DstAndIndex(key, val, &curindex, &FreeIndex);/**根据key进行Hash查表并匹配val**/
	if (GM_RSSP1_FALSE == bRt)
	{
		if ((GM_RSSP1_EMPTY == gSFMHashTable.SFMelem[FreeIndex].DataType) || (GM_RSSP1_DELETE == gSFMHashTable.SFMelem[FreeIndex].DataType))/**未找到同时匹配key和val,重新写入Hash表内**/
		{
			gSFMHashTable.SFMelem[FreeIndex].DataType = GM_RSSP1_EXIST;
			gSFMHashTable.SFMelem[FreeIndex].DstAddr = (GM_RSSP1_UINT16)key;

			gSFMHashTable.SFMelem[FreeIndex].Cnf_index = val;
			bRt = GM_RSSP1_TRUE;
		}
	}
	else
	{
		FreeIndex = 0xFFFFU;
	}

	return FreeIndex;
}


/* 用平方探测法进行解决冲突 */
GM_RSSP1_BOOL GM_RSSP1_Hash_Search_DstAndIndex(GM_RSSP1_UINT32 key, GM_RSSP1_UINT16 val, GM_RSSP1_UINT16* pCurindex, GM_RSSP1_UINT16* pFreeIndex)
{
	GM_RSSP1_UINT16 index = 0xFFFFU;
	GM_RSSP1_UINT16 TobeWriteIndex = 0xFFFFU;
	GM_RSSP1_UINT16 collisionNum = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL breakFlag = GM_RSSP1_FALSE;

	if ((NULL == pCurindex) || (NULL == pFreeIndex))
	{
		return bRt;
	}

	index = (GM_RSSP1_UINT16)(key % gSFMHashTable.hashSize);

	while ((gSFMHashTable.SFMelem[index].DstAddr != key)||(gSFMHashTable.SFMelem[index].Cnf_index != val))
	{
		++collisionNum;
		if (collisionNum > gSFMHashTable.sizeAvail)/**最多索引次数为本Hash表的有效数据空间**/
		{
			breakFlag = GM_RSSP1_TRUE;
		}
		else
		{
			if (GM_RSSP1_EMPTY == gSFMHashTable.SFMelem[index].DataType)/**遍历到EMPTY,表明该key下的Hash已索引结束**/
			{
				if (0xFFFFU == TobeWriteIndex)
				{
					*pFreeIndex = index;
				}
				breakFlag = GM_RSSP1_TRUE;
			}
			else
			{
				if (GM_RSSP1_DELETE == gSFMHashTable.SFMelem[index].DataType)/**当遍历时发现有可写入位置,记录第一个可写入信息位.可节约遍历时间**/
				{
					if (0xFFFFU == TobeWriteIndex)
					{
						TobeWriteIndex = index;
						*pFreeIndex = index;
					}
				}

				index += 2U * collisionNum - 1U;

				if(index >= gSFMHashTable.hashSize)/**如果新的定位越过数组范围，重新拉到范围内**/
				{
					index = (GM_RSSP1_UINT16)((GM_RSSP1_UINT32)index % gSFMHashTable.hashSize);
				}
			}
		}

		if(GM_RSSP1_TRUE == breakFlag)
		{
			break;
		}
	}
	
	if ((gSFMHashTable.SFMelem[index].DstAddr == key) && (gSFMHashTable.SFMelem[index].Cnf_index == val))
	{
		bRt = GM_RSSP1_TRUE;
		*pCurindex = index;
	}

	return bRt;
}


GM_RSSP1_BOOL GM_RSSP1_Hash_Search_CnfDst(GM_RSSP1_UINT32 key, GM_RSSP1_UINT16* pCurIndex, GM_RSSP1_UINT16* pHashIndex)
{
	GM_RSSP1_UINT16 index = 0xFFFFU;
	GM_RSSP1_UINT16 collisionNum = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL breakFlag = GM_RSSP1_FALSE;

	if ((NULL == pCurIndex) || (NULL == pHashIndex))
	{
		return bRt;
	}

	index = (GM_RSSP1_UINT16)(key % gSFMHashTable.hashSize);

	while (gSFMHashTable.SFMelem[index].DstAddr != key)
	{
		++collisionNum;
		if (collisionNum > gSFMHashTable.sizeAvail)/**最多索引次数为本Hash表的有效数据空间**/
		{
			breakFlag = GM_RSSP1_TRUE;
		}
		else
		{
			if (gSFMHashTable.SFMelem[index].DataType == GM_RSSP1_EMPTY)/**遍历到EMPTY,表明该key下的Hash已索引结束**/
			{
				breakFlag = GM_RSSP1_TRUE;
			}
			else
			{
				index += 2U * collisionNum - 1U;

				if (index >= gSFMHashTable.hashSize)/* 如果新的定位越过数组范围，重新拉到范围内 */
				{
					index = (GM_RSSP1_UINT16)((GM_RSSP1_UINT32)index % gSFMHashTable.hashSize);
				}
			}
		}

		if(GM_RSSP1_TRUE == breakFlag)
		{
			break;
		}
	}

	if (gSFMHashTable.SFMelem[index].DstAddr == key)
	{
		*pCurIndex = gSFMHashTable.SFMelem[index].Cnf_index;
		*pHashIndex = index;
		bRt = GM_RSSP1_TRUE;
	}
	return bRt;
}

GM_RSSP1_BOOL GM_RSSP1_Hash_CnfDst_ResArray(GM_RSSP1_UINT32 key, GM_RSSP1_UINT16* ConArray, GM_RSSP1_UINT8* ArraySize)
{
	GM_RSSP1_UINT8 Counter = 0U;
	GM_RSSP1_UINT16 index = 0xFFFFU;
	GM_RSSP1_UINT16 collisionNum = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL breakFlag = GM_RSSP1_FALSE;

	if ((NULL == ConArray) || (NULL == ArraySize))
	{
		return bRt;
	}

	index = (GM_RSSP1_UINT16)(key % gSFMHashTable.hashSize);

	while(1)
	{
		if (GM_RSSP1_EXIST == gSFMHashTable.SFMelem[index].DataType)
		{
			if (gSFMHashTable.SFMelem[index].DstAddr == key)
			{
				if ((Counter+1U) < (*ArraySize))
				{
					*(ConArray+Counter) = gSFMHashTable.SFMelem[index].Cnf_index;
					Counter++;
					*(ConArray+Counter) = index;
					Counter++;
					bRt = GM_RSSP1_TRUE;
				}
				else
				{
					breakFlag = GM_RSSP1_TRUE;
				}
			}

			if(breakFlag != GM_RSSP1_TRUE)
			{
				++collisionNum;
				if (collisionNum > gSFMHashTable.sizeAvail)
				{
					breakFlag = GM_RSSP1_TRUE;
				}
				else
				{	
					index += 2U * collisionNum - 1U;

					if (index >= gSFMHashTable.hashSize)  /* 如果新的定位越过数组范围，重新拉到范围内 */
					{
						index = (GM_RSSP1_UINT16)((GM_RSSP1_UINT32)index % gSFMHashTable.hashSize);
					}
				}
			}
			
		}
		else if (GM_RSSP1_EMPTY == gSFMHashTable.SFMelem[index].DataType)
		{
			breakFlag = GM_RSSP1_TRUE;
		}
		else
		{
			/*CR: 11275184*/
			++collisionNum;
			if (collisionNum > gSFMHashTable.sizeAvail)
			{
				breakFlag = GM_RSSP1_TRUE;
			}
			else
			{	
				index += 2U * collisionNum - 1U;

				if (index >= gSFMHashTable.hashSize)  /* 如果新的定位越过数组范围，重新拉到范围内 */
				{
					index = (GM_RSSP1_UINT16)((GM_RSSP1_UINT32)index % gSFMHashTable.hashSize);
				}
			}
		}

		if(GM_RSSP1_TRUE == breakFlag)
		{
			break;
		}
	}

	*ArraySize = Counter;
	return bRt;
}


GM_RSSP1_BOOL GM_RSSP1_Hash_SFM_Dele(GM_RSSP1_UINT32 key, GM_RSSP1_UINT8 *counter, GM_RSSP1_UINT16 *cnfIndex)
{	
	GM_RSSP1_UINT16 index = 0xFFFFU;
	GM_RSSP1_UINT16 collisionNum = 0U;

	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL breakFlag = GM_RSSP1_FALSE;

	if ((NULL == counter) || (NULL == cnfIndex) || (gSFMHashTable.hashSize == 0U)/*CR5746*/)
	{
		return bRt;
	}

	index = (GM_RSSP1_UINT16)(key % gSFMHashTable.hashSize);

	while (gSFMHashTable.SFMelem[index].DataType != GM_RSSP1_EMPTY)
	{
		++collisionNum;
		if (collisionNum > gSFMHashTable.sizeAvail)/**最多索引次数为本Hash表的有效数据空间**/
		{
			breakFlag = GM_RSSP1_TRUE;
		}
		else if (*counter > 4U) /**内部越界**/
		{
			breakFlag = GM_RSSP1_TRUE;
		}
		else
		{
			if (gSFMHashTable.SFMelem[index].DstAddr == key)
			{
				*(cnfIndex+ (*counter)) = gSFMHashTable.SFMelem[index].Cnf_index;
				(*counter)++;
				gSFMHashTable.SFMelem[index].Cnf_index = GM_RSSP1_MAX_SAFETY_CONNECTION_NUM;
				gSFMHashTable.SFMelem[index].Online_Status = GM_RSSP1_FALSE;
				gSFMHashTable.SFMelem[index].DstAddr = 0U;
				gSFMHashTable.SFMelem[index].DataType = GM_RSSP1_DELETE;
				bRt = GM_RSSP1_TRUE;
			}

			index += 2U * collisionNum - 1U;

			if (index >= gSFMHashTable.hashSize)/* 如果新的定位越过数组范围，重新拉到范围内 */
			{
				index = (GM_RSSP1_UINT16)((GM_RSSP1_UINT32)index % gSFMHashTable.hashSize);
			}
		}

		if(GM_RSSP1_TRUE == breakFlag)
		{
			break;
		}
	}

	return bRt;
}


GM_RSSP1_BOOL GM_RSSP1_Hash_Insert_Online(GM_RSSP1_UINT16 HashIndex, GM_RSSP1_BOOL Res)
{
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;

	if (HashIndex < gSFMHashTable.hashSize)
	{
#ifdef SFM_Part
		if (gSFMHashTable.SFMelem[HashIndex].Online_Status != Res)
		{
			if (Res == GM_RSSP1_TRUE)
			{
				if (g_OnlineArray.SeqNum < g_OnlineArray.MaxNum)
				{
					GM_RSSP1_SeqInsert(OnLine_Object, gSFMHashTable.SFMelem[HashIndex].Cnf_index);
					gSFMHashTable.SFMelem[HashIndex].Online_Status = Res;
				}
			}
			else
			{
				if (g_OnlineArray.SeqNum != 0U)
				{
					GM_RSSP1_SeqDelete(OnLine_Object, gSFMHashTable.SFMelem[HashIndex].Cnf_index);
					gSFMHashTable.SFMelem[HashIndex].Online_Status = Res;
				}
			}
		}

		bRt = gSFMHashTable.SFMelem[HashIndex].Online_Status;
#endif
	}

	return bRt;
}

GM_RSSP1_BOOL GM_RSSP1_Hash_CFM_ActInit(GM_RSSP1_UINT32 MaxArryVal) 
{
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_UINT32 cycNum = 0U;

	g_Active.hashSize = GM_RSSP1_Next_Prime(MaxArryVal*2U);
	g_Active.sizeAvail = MaxArryVal;
	g_Active.FreeSize = MaxArryVal;

	bRt = GM_RSSP1_Mutex_Init(&g_Active.Active_Mutex);

	if (GM_RSSP1_TRUE == bRt)
	{
		g_Active.elem = (Hash_CFM_ActiveElem_Structure *)GM_RSSP1_malloc((size_t)g_Active.hashSize * sizeof(Hash_CFM_ActiveElem_Structure));

		if (NULL == g_Active.elem)
		{
			bRt = GM_RSSP1_FALSE;
		}
		else
		{
			for (cycNum = 0U; cycNum<g_Active.hashSize; ++cycNum)
			{
				g_Active.elem[cycNum].cnf_index = 0xFFFFU;
				g_Active.elem[cycNum].ActiveType = GM_RSSP1_EMPTY;
			}
		}
	}

	return bRt;
}

GM_RSSP1_BOOL GM_RSSP1_Hash_CFM_Init(void)
{
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_UINT32 cycNum = 0U;

	gCFMHashTable.hashSize = GM_RSSP1_Next_Prime(GM_RSSP1_MAX_SAFETY_CONNECTION_NUM*4U);
	gCFMHashTable.sizeAvail = GM_RSSP1_MAX_SAFETY_CONNECTION_NUM*2U;

	gCFMHashTable.CFMelem = (Hash_CFM_Elem_Struct *)GM_RSSP1_malloc((size_t)gCFMHashTable.hashSize * sizeof(Hash_CFM_Elem_Struct));
	if (NULL != gCFMHashTable.CFMelem)
	{
		for (cycNum = 0U; cycNum<gCFMHashTable.hashSize; ++cycNum)
		{
			gCFMHashTable.CFMelem[cycNum].Cnf_index = 0xFFFFU;
			gCFMHashTable.CFMelem[cycNum].IPAddr = 0U;
			gCFMHashTable.CFMelem[cycNum].port = 0U;
			gCFMHashTable.CFMelem[cycNum].DataType = GM_RSSP1_EMPTY;
		}
		bRt = GM_RSSP1_TRUE;
	}

	return bRt;
}

GM_RSSP1_BOOL GM_RSSP1_Hash_Fill_CFM_Index(GM_RSSP1_UINT32 key, GM_RSSP1_UINT32 val, GM_RSSP1_UINT16 index, GM_RSSP1_UINT8 subindex)
{
	GM_RSSP1_BOOL bRt = GM_RSSP1_TRUE;
	GM_RSSP1_UINT16 curindex = 0xFFFFU;
	GM_RSSP1_UINT16 FreeIndex = 0xFFFFU;

	bRt = GM_RSSP1_Hash_Search_IP(key, val, &curindex, &FreeIndex);
	if ((bRt == GM_RSSP1_FALSE)&&(FreeIndex < gCFMHashTable.hashSize))
	{
		if (gCFMHashTable.CFMelem[FreeIndex].DataType != GM_RSSP1_EXIST)
		{
			gCFMHashTable.CFMelem[FreeIndex].DataType = GM_RSSP1_EXIST;
			gCFMHashTable.CFMelem[FreeIndex].IPAddr = key;
			gCFMHashTable.CFMelem[FreeIndex].port = val;
			gCFMHashTable.CFMelem[FreeIndex].Cnf_index = index;
			gCFMHashTable.CFMelem[FreeIndex].sub_index = subindex;
		}
	}

	return bRt;
}

GM_RSSP1_BOOL GM_RSSP1_Hash_CFM_Dele(GM_RSSP1_UINT32 key, GM_RSSP1_UINT32 val, GM_RSSP1_UINT16 *cnfIndex)
{	
	GM_RSSP1_UINT16 index = 0xFFFFU;
	GM_RSSP1_UINT16 collisionNum = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_TRUE;
	GM_RSSP1_BOOL breakFlag = GM_RSSP1_FALSE;
	
	if ((NULL == cnfIndex) || (gCFMHashTable.hashSize == 0U)/*CR5746*/)
	{
		bRt = GM_RSSP1_FALSE;
		return bRt;
	}

	index = (GM_RSSP1_UINT16)(key % gCFMHashTable.hashSize);

	while (gCFMHashTable.CFMelem[index].DataType != GM_RSSP1_EMPTY)
	{
		++collisionNum;
		if (collisionNum > gCFMHashTable.sizeAvail)
		{
			breakFlag = GM_RSSP1_TRUE;
		}
		else
		{
			if ((gCFMHashTable.CFMelem[index].IPAddr == key) && (gCFMHashTable.CFMelem[index].port == val))
			{
				*cnfIndex = gCFMHashTable.CFMelem[index].Cnf_index;
				gCFMHashTable.CFMelem[index].port = 0U;
				gCFMHashTable.CFMelem[index].IPAddr = 0U;
				gCFMHashTable.CFMelem[index].Cnf_index = GM_RSSP1_MAX_SAFETY_CONNECTION_NUM;
				gCFMHashTable.CFMelem[index].sub_index = 0xFFU;
				gCFMHashTable.CFMelem[index].DataType = GM_RSSP1_DELETE;
				breakFlag = GM_RSSP1_TRUE;
			}
			else
			{
				index += 2U * collisionNum - 1U;

				if (index >= gCFMHashTable.hashSize)  /* 如果新的定位越过数组范围，重新拉到范围内 */
				{
					index = (GM_RSSP1_UINT16)((GM_RSSP1_UINT32)index % gCFMHashTable.hashSize);
				}
			}
		}
		
		if(GM_RSSP1_TRUE == breakFlag)
		{
			break;
		}
	}

	return bRt;
}

GM_RSSP1_BOOL GM_RSSP1_Hash_Search_IP(GM_RSSP1_UINT32 key, GM_RSSP1_UINT32 val, GM_RSSP1_UINT16* pCurindex, GM_RSSP1_UINT16* pFreeIndex)
{
	GM_RSSP1_UINT16 index = 0xFFFFU;
	GM_RSSP1_UINT16 TobeWriteIndex = 0xFFFFU;
	GM_RSSP1_UINT16 collisionNum = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL breakFlag = GM_RSSP1_FALSE;
	
	if ((NULL == pCurindex) || (NULL == pFreeIndex) || (gCFMHashTable.hashSize == 0U)/*CR5746*/)
	{
		return bRt;
	}

	index = (GM_RSSP1_UINT16)(key % gCFMHashTable.hashSize);

	while ((gCFMHashTable.CFMelem[index].IPAddr != key)||(gCFMHashTable.CFMelem[index].port != val))
	{
		++collisionNum;
		if (collisionNum > gCFMHashTable.sizeAvail)/**最多索引次数为本Hash表的有效数据空间**/
		{
			breakFlag = GM_RSSP1_TRUE;
		}
		else
		{
			if (GM_RSSP1_EMPTY == gCFMHashTable.CFMelem[index].DataType)
			{
				if (0xFFFFU == TobeWriteIndex)
				{
					*pFreeIndex = index;
				}

				breakFlag = GM_RSSP1_TRUE;
			}
			else
			{
				if (GM_RSSP1_DELETE == gCFMHashTable.CFMelem[index].DataType)
				{
					if (0xFFFFU == TobeWriteIndex)
					{
						TobeWriteIndex = index;
						*pFreeIndex = index;
					}
				}

				index += 2U * collisionNum - 1U;

				if (index >= gCFMHashTable.hashSize)/* 如果新的定位越过数组范围，重新拉到范围内 */
				{
					index = (GM_RSSP1_UINT16)((GM_RSSP1_UINT32)index % gCFMHashTable.hashSize);
				}
			}
		}

		if(GM_RSSP1_TRUE == breakFlag)
		{
			break;
		}
	}

	if ((gCFMHashTable.CFMelem[index].IPAddr == key) && (gCFMHashTable.CFMelem[index].port == val))
	{
		*pCurindex = index;
		bRt = GM_RSSP1_TRUE;
	}

	return bRt;
}


GM_RSSP1_BOOL GM_RSSP1_Hash_GetCFMIndex(GM_RSSP1_UINT32 key, GM_RSSP1_UINT32 val, GM_RSSP1_UINT16 *pIndex, GM_RSSP1_UINT8 *pSubIndex)
{
	GM_RSSP1_BOOL bRt = GM_RSSP1_TRUE;
	GM_RSSP1_UINT16 curindex = 0xFFFFU;
	GM_RSSP1_UINT16 FreeIndex = 0xFFFFU;

	if ((NULL == pIndex) || (NULL == pSubIndex))
	{
		bRt = GM_RSSP1_FALSE;
		return bRt;
	}

	bRt = GM_RSSP1_Hash_Search_IP(key, val, &curindex, &FreeIndex);
	if((GM_RSSP1_TRUE == bRt)&&(curindex<gCFMHashTable.hashSize))
	{
		*pIndex = gCFMHashTable.CFMelem[curindex].Cnf_index;
		*pSubIndex = gCFMHashTable.CFMelem[curindex].sub_index;
	}

	return bRt;
}

void GM_RSSP1_Clear_Active(void)
{
    GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
    bRt = GM_RSSP1_Mutex_Lock(g_Active.Active_Mutex);
    if (GM_RSSP1_TRUE == bRt) 
    {
        g_Active.FreeSize = g_Active.sizeAvail;
        GM_RSSP1_memset((void*)g_Active.elem, 0xFFU, sizeof(Hash_CFM_ActiveElem_Structure) * g_Active.hashSize);
    }
    
    GM_RSSP1_Mutex_Unlock(g_Active.Active_Mutex);
}

void GM_RSSP1_Reset_Active(void)/*CR:GM00004827 SFM or CFM active index update*/
{
#ifdef SFM_Part
	GM_RSSP1_UINT16 i = 0U;
	GM_RSSP1_UINT16 onlineNum = 0U;
	GM_RSSP1_UINT16 con_index = 0U;
	GM_RSSP1_UINT16 LinkIndex = 0U;
#endif
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;

	bRt = GM_RSSP1_Mutex_Lock(g_Active.Active_Mutex);

	if (GM_RSSP1_TRUE == bRt) 
	{
		if (GM_RSSP1_FALSE == bUpdate)
		{
			g_Active.FreeSize = g_Active.sizeAvail;
			GM_RSSP1_memset((void*)g_Active.elem, 0xFFU, sizeof(Hash_CFM_ActiveElem_Structure) * g_Active.hashSize);
		}
		else
		{
			bUpdate = GM_RSSP1_FALSE;
		}
#ifdef SFM_Part
		onlineNum = GM_RSSP1_GetSeqAmount(OnLine_Object);
		GM_RSSP1_GetSeqIndexPositive(OnLine_Object, 1U);
		for (i=0U; i<onlineNum; ++i)
		{
			bRt = GM_RSSP1_TRUE;

			con_index = GM_RSSP1_GetSeqIndexPositive(OnLine_Object, 0U);
			if (0xFFFFU != con_index)
			{
				LinkIndex = GM_RSSP1_Update_Active(con_index, 1U);/**将当前索引通道加入活动通道,返回活动通道索引**/
				if ((0xFFFF == LinkIndex) || (LinkIndex > GM_RSSP1_MAX_ONLINE_CONNECTION_NUM))
				{
					bRt = GM_RSSP1_FALSE;
				}
			}
			else
			{
				bRt = GM_RSSP1_FALSE;
			}
		}
#endif
	}

	GM_RSSP1_Mutex_Unlock(g_Active.Active_Mutex);
}

GM_RSSP1_UINT32 GM_RSSP1_Get_Active_Num(void)
{
	return (g_Active.sizeAvail - g_Active.FreeSize);
}

GM_RSSP1_BOOL GM_RSSP1_Hash_Search_Active(GM_RSSP1_UINT32 key, GM_RSSP1_UINT16* pCurindex, GM_RSSP1_UINT16* pFreeIndex)
{
	GM_RSSP1_UINT16 index = 0xFFFFU;
	GM_RSSP1_UINT16 collisionNum = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	GM_RSSP1_BOOL breakFlag = GM_RSSP1_FALSE;

	if ((NULL == pCurindex) || (NULL == pFreeIndex))
	{
		return bRt;
	}

	index = (GM_RSSP1_UINT16)(key % g_Active.hashSize);

	while (g_Active.elem[index].cnf_index != key)
	{
		++collisionNum;
		if (collisionNum > g_Active.sizeAvail)/**最多索引次数为本Hash表的有效数据空间**/
		{
			breakFlag = GM_RSSP1_TRUE;
		}
		else
		{
			if ((GM_RSSP1_UINT32)(g_Active.elem[index].ActiveType) >= (GM_RSSP1_UINT32)GM_RSSP1_EMPTY)/**Rest按字节刷0xFF,枚举值匹配不了任何值**/
			{
				*pFreeIndex = index;
				breakFlag = GM_RSSP1_TRUE;
			}
			else
			{
				index += 2U * collisionNum - 1U;

				if (index >= g_Active.hashSize)/* 如果新的定位越过数组范围，重新拉到范围内 */
				{
					index = (GM_RSSP1_UINT16)((GM_RSSP1_UINT32)index % g_Active.hashSize);
				}
			}
		}

		if(GM_RSSP1_TRUE == breakFlag)
		{
			break;
		}
	}

	if (g_Active.elem[index].cnf_index == key)
	{
		bRt = GM_RSSP1_TRUE;
		*pCurindex = index;
	}

	return bRt;
}

GM_RSSP1_UINT16 GM_RSSP1_Active_Add(GM_RSSP1_UINT16 Index, GM_RSSP1_UINT8 SetVal)/*CR:GM00004827  SFM or CFM update active index*/
{
	GM_RSSP1_UINT16 activeIndex = 0xFFFFU;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;
	bRt = GM_RSSP1_Mutex_Lock(g_Active.Active_Mutex);
	if (GM_RSSP1_TRUE == bRt)
	{
		activeIndex = GM_RSSP1_Update_Active(Index, SetVal);
	}
	GM_RSSP1_Mutex_Unlock(g_Active.Active_Mutex);
	
	return activeIndex;
}


GM_RSSP1_BOOL GM_RSSP1_Active_AddByArry(GM_RSSP1_UINT16* IndexArray)/*CR:GM00004827 Set active index*/
{
	GM_RSSP1_UINT16 i = 0U;
	GM_RSSP1_UINT16 arryNum = 0U;
	GM_RSSP1_UINT16 LinkIndex = 0U;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;

	if (NULL != IndexArray)
	{
		arryNum = *IndexArray;
		bRt = GM_RSSP1_Mutex_Lock(g_Active.Active_Mutex);

		if (GM_RSSP1_TRUE == bRt)
		{
			g_Active.FreeSize = g_Active.sizeAvail;
			GM_RSSP1_memset((void*)g_Active.elem, 0xFFU, sizeof(Hash_CFM_ActiveElem_Structure) * g_Active.hashSize);

			for (i=0U; i< arryNum; ++i)
			{
				LinkIndex = GM_RSSP1_Update_Active(*(IndexArray+(GM_RSSP1_UINT16)i+(GM_RSSP1_UINT16)1), (GM_RSSP1_UINT8)1U);
				if ((0xFFFFU == LinkIndex) || ((GM_RSSP1_UINT32)LinkIndex > g_Active.sizeAvail))
				{
					bRt = GM_RSSP1_FALSE;
					break;
				}
			}

			bUpdate = GM_RSSP1_TRUE;
		}

		GM_RSSP1_Mutex_Unlock(g_Active.Active_Mutex);
	}

	return bRt;
}

GM_RSSP1_UINT16 GM_RSSP1_Update_Active(GM_RSSP1_UINT16 Index, GM_RSSP1_UINT8 SetVal)
{
	GM_RSSP1_UINT16 activeIndex = 0xFFFFU;
	GM_RSSP1_UINT16 curindex = 0xFFFFU;
	GM_RSSP1_UINT16 FreeIndex = 0xFFFFU;
	GM_RSSP1_BOOL bRt = GM_RSSP1_FALSE;		

	/*GM00004905:删除无意义的if分支*/
	bRt = GM_RSSP1_Hash_Search_Active((GM_RSSP1_UINT32)Index, &curindex, &FreeIndex);
		
	if ((GM_RSSP1_FALSE == bRt)&&(FreeIndex<g_Active.hashSize)) /*未找到活动节点*/
	{
		if (1U == SetVal)  /*添加活动节点*/
		{
			if (g_Active.FreeSize> 0U)
			{
				if (g_Active.elem[FreeIndex].ActiveType != GM_RSSP1_EXIST)
				{
					g_Active.elem[FreeIndex].ActiveType = GM_RSSP1_EXIST;
				}

				g_Active.elem[FreeIndex].online_index = (GM_RSSP1_UINT16)(g_Active.sizeAvail - g_Active.FreeSize);
				g_Active.elem[FreeIndex].cnf_index = Index;
				g_Active.FreeSize --;
				activeIndex = g_Active.elem[FreeIndex].online_index;
			}
		}
	}
	else if (GM_RSSP1_TRUE == bRt)/*找到活动节点*/
	{
		activeIndex = g_Active.elem[curindex].online_index;
		/*
		if (0 == SetVal)
		{
			g_Active.elem[curindex].online_index = 0xFFFF;
			g_Active.elem[curindex].cnf_index = 0xFFFF;
			g_Active.elem[curindex].ActiveType = GM_RSSP1_DELETE;
			g_Active.sizeAvail ++;
		}
		*/
	}
	else
	{
		; /*do nothing*/
	}
	
	return activeIndex;
}

GM_RSSP1_UINT16 GM_RSSP1_GetSeqAmount(GM_RSSP1_OperObject_Enum ArrtType)
{
	GM_RSSP1_UINT16 amount = 0U;
	switch(ArrtType)
	{
	case OffLine_Object:
		amount = g_OffLineArry.SeqNum;
		break;
	case OnLine_Object:
		amount = g_OnlineArray.SeqNum;
		break;

	default:
		/*do nothing*/
		break;
	}

	return amount;
}

void GM_RSSP1_InitSeqArray(GM_RSSP1_OperObject_Enum ArrtType, GM_RSSP1_UINT16 MaxAmout)
{
	GM_RSSP1_UINT16 l_SeqNum = 0U;
	GM_RSSP1_Sequen_Structure *pLine = NULL;

	switch(ArrtType)
	{
	case OffLine_Object:
		pLine = &g_OffLineArry;
		break;

	case OnLine_Object:
		pLine = &g_OnlineArray;
		g_DeleRecord.RecordNum = 0U;
		break;

	default:
		/*do nothing*/
		break;
	}

	if (NULL != pLine)
	{
		pLine->MaxNum = MaxAmout;
		pLine->SeqNum = 0u;
		pLine->startNode = 0U;
		pLine->EndNode = 0U;
		pLine->Front_Seq = 0xFFFFU;
		pLine->Back_Seq = 0xFFFFU;

		pLine->node = (GM_RSSP1_Point_Elem_Struct *)GM_RSSP1_malloc((size_t)MaxAmout * sizeof(GM_RSSP1_Point_Elem_Struct));

		if (NULL != pLine->node)
		{
			for (l_SeqNum=0U; l_SeqNum<MaxAmout; ++l_SeqNum)/**初始化双向链表,默认好前后关系**/
			{
				pLine->node[l_SeqNum].preNode = (GM_RSSP1_UINT16)(l_SeqNum - 1U);
				pLine->node[l_SeqNum].Index = 0xFFFFU;
				pLine->node[l_SeqNum].NexNode = l_SeqNum + 1U;
			}

			pLine->node[0U].preNode = MaxAmout-1U;
			pLine->node[MaxAmout-1U].NexNode = 0U;
		}
	}
}

void GM_RSSP1_SeqArrayClean(GM_RSSP1_OperObject_Enum ArrtType) /* CR7293: flt clear as the seqArry size*/
{
	GM_RSSP1_UINT16 l_SeqNum = 0U;
	GM_RSSP1_Sequen_Structure *pLine = NULL;

	switch(ArrtType)
	{
	case OffLine_Object:
		pLine = &g_OffLineArry;
		break;

	case OnLine_Object:
		pLine = &g_OnlineArray;
		g_DeleRecord.RecordNum = 0U;
		break;

	default:
		/*do nothing*/
		break;
	}

	if (NULL != pLine)
	{
		pLine->startNode = 0U;
		pLine->SeqNum = 0U;
		pLine->EndNode = 0U;
		pLine->Front_Seq = 0xFFFFU;
		pLine->Back_Seq = 0xFFFFU;

		if (NULL != pLine->node)
		{
			GM_RSSP1_memset(pLine->node, 0U, (size_t)pLine->MaxNum * sizeof(GM_RSSP1_Point_Elem_Struct));

			for (l_SeqNum=0U; l_SeqNum<pLine->MaxNum; ++l_SeqNum)/**初始化双向链表,默认好前后关系**/
			{
				pLine->node[l_SeqNum].preNode = (GM_RSSP1_UINT16)(l_SeqNum - 1U);
				pLine->node[l_SeqNum].Index = 0xFFFFU;
				pLine->node[l_SeqNum].NexNode = l_SeqNum + 1U;
			}

			pLine->node[0U].preNode = pLine->MaxNum-1U;
			pLine->node[pLine->MaxNum-1U].NexNode = 0U;
		}
	}
}


GM_RSSP1_UINT16 GM_RSSP1_InitSeqInsert(GM_RSSP1_OperObject_Enum ArrtType)
{
	GM_RSSP1_UINT16 RetNode = 0xFFFFU;
	GM_RSSP1_Sequen_Structure *pLine = NULL;

	switch(ArrtType)
	{
	case OffLine_Object:
		pLine = &g_OffLineArry;
		break;

	case OnLine_Object:
		pLine = &g_OnlineArray;
		break;

	default:
		/*do nothing*/
		break;
	}
	
	if (NULL != pLine)
	{
		if (pLine->MaxNum > pLine->SeqNum)/**初始化填充链表内容**/
		{
			RetNode = pLine->EndNode;
			pLine->node[RetNode].Index = RetNode;
			pLine->EndNode = pLine->node[RetNode].NexNode;
			pLine->SeqNum++;
		}
	}

	return RetNode;
}

void GM_RSSP1_SeqInsert(GM_RSSP1_OperObject_Enum ArrtType, GM_RSSP1_UINT16 index)
{
	GM_RSSP1_UINT16 End_Seq = 0xFFFFU;
	GM_RSSP1_Sequen_Structure *pLine = NULL;

	switch(ArrtType)
	{
	case OffLine_Object:
		pLine = &g_OffLineArry;
		break;

	case OnLine_Object:
		pLine = &g_OnlineArray;
		break;

	default:
		/*do nothing*/
		break;
	}

	if (NULL != pLine)/**填充链表内容**/
	{
		End_Seq = pLine->EndNode;
		pLine->node[End_Seq].Index = index;
		pLine->EndNode = pLine->node[End_Seq].NexNode;
		pLine->SeqNum++;
	}

}

void GM_RSSP1_SeqDelete(GM_RSSP1_OperObject_Enum ArrtType, GM_RSSP1_UINT16 index)
{
	GM_RSSP1_UINT16 Seq = 0U;
	GM_RSSP1_UINT16 cycCnt = 0U;
	GM_RSSP1_UINT16 preID = 0U;
	GM_RSSP1_UINT16 NxtID = 0U;
	GM_RSSP1_Sequen_Structure *pLine = NULL;

	switch(ArrtType)
	{
	case OffLine_Object:
		pLine = &g_OffLineArry;
		break;

	case OnLine_Object:
		pLine = &g_OnlineArray;
		break;

	default:
		/*do nothing*/
		break;
	}

	if (NULL != pLine)
	{
		 /* 初始化当前节点索引为链表的起始节点 */
		Seq = pLine->startNode;
		
		while (cycCnt < pLine->SeqNum)
		{
			cycCnt++;
			if (pLine->node[Seq].Index == index)  /* 在链表中根据cfg index找到要删除的结点 */
			{
				/* 如果是在线对象，则调用记录删除函数 */
				if(OnLine_Object == ArrtType)
				{
					GM_RSSP1_SeqDeleteRecord(index);
				}
				/*根据删除位置,进行转表*/
				if (Seq == pLine->node[pLine->EndNode].preNode) /* 情况一：删除的是尾节点的前驱节点 */
				{
					pLine->EndNode = Seq;
				}
				else if (Seq == pLine->EndNode) /* 情况二：删除的是尾节点本身 */
				{
					pLine->startNode = pLine->node[Seq].NexNode;
				}
				else
				{
					if (Seq == pLine->startNode) /* 如果是起始节点，则更新起始指针 */
					{
						pLine->startNode = pLine->node[Seq].NexNode; /* 更新起始结点为其后驱结点 */
					}
					 /* 获取要删除的结点的前驱和后驱节点索引 */
					preID = pLine->node[Seq].preNode;
					NxtID = pLine->node[Seq].NexNode;
					 /* 跳过要删除的结点 */
					pLine->node[preID].NexNode = NxtID;
					pLine->node[NxtID].preNode = preID;

					/* 删除的结点替代原始的end结点 */
					preID = pLine->node[pLine->EndNode].preNode; /* 获取原结束节点的前驱索引 */
					pLine->node[Seq].preNode = preID;  /* 当前节点的前驱指针指向这个前驱节点 */
					pLine->node[Seq].NexNode = pLine->EndNode; /* 当前节点的后驱指向原来的结束节点 */
					pLine->node[preID].NexNode = Seq;
					pLine->node[pLine->EndNode].preNode = Seq;
					pLine->EndNode = Seq; /* 更新结束节点为当前节点 */
				}

				pLine->node[Seq].Index = 0xFFFFU;
				pLine->SeqNum--;
				break;
			}
			else
			{
				Seq = pLine->node[Seq].NexNode; /* 向下个置移动 */
			}
		}		
	}
}

GM_RSSP1_UINT16 GM_RSSP1_GetSeqIndexPositive(GM_RSSP1_OperObject_Enum ArrtType, GM_RSSP1_UINT8 isReset)
{
	GM_RSSP1_UINT16 RetIndex = 0xFFFFU;
	GM_RSSP1_Sequen_Structure *pLine = NULL;

	switch(ArrtType)
	{
	case OffLine_Object:
		pLine = &g_OffLineArry;
		break;

	case OnLine_Object:
		pLine = &g_OnlineArray;
		break;

	default:
		/*do nothing*/
		break;
	}

	if (NULL != pLine)
	{
		if (1U == isReset)
		{
			pLine->Front_Seq = 0xFFFFU;
		}
		else if (0xFFFFU == pLine->Front_Seq)
		{
			pLine->Front_Seq = pLine->startNode;
			RetIndex = pLine->node[pLine->Front_Seq].Index;
		}
		else
		{
			pLine->Front_Seq = pLine->node[pLine->Front_Seq].NexNode;
			RetIndex = pLine->node[pLine->Front_Seq].Index;
		}
	}

	return RetIndex;
}

GM_RSSP1_UINT16 GM_RSSP1_GetSeqIndexReverse(GM_RSSP1_OperObject_Enum ArrtType, GM_RSSP1_UINT8 isReset)
{
	GM_RSSP1_UINT16 RetIndex = 0xFFFFU;
	GM_RSSP1_Sequen_Structure *pLine = NULL;

	switch(ArrtType)
	{
	case OffLine_Object:
		pLine = &g_OffLineArry;
		break;

	case OnLine_Object:
		pLine = &g_OnlineArray;
		break;

	default:
		/*do nothing*/
		break;
	}

	if (NULL != pLine)
	{
		/*CR:7293, 修改查询，获取当前节点信息后，将查询索引至下一个节点*/
		if (1U == isReset)
		{
			pLine->Back_Seq = pLine->node[pLine->EndNode].preNode;
		}
		else if (pLine->Back_Seq != 0xFFFFU)
		{
			RetIndex = pLine->node[pLine->Back_Seq].Index;
			pLine->Back_Seq = pLine->node[pLine->Back_Seq].preNode;
		}
		else
		{
			;/*do nothing*/
		}
	}

	return RetIndex;
}

void GM_RSSP1_SeqDeleteRecord(GM_RSSP1_UINT16 index)
{
	GM_RSSP1_UINT16 i = 0U;

	if(g_DeleRecord.RecordNum == 0U) /* 初始化记录的数组 */
	{
		GM_RSSP1_memset((void*)&(g_DeleRecord.RecordIndex), 0xFFU, sizeof(GM_RSSP1_SeqDele_Record_Structure) - sizeof(GM_RSSP1_UINT16));
	}
	
	if (g_DeleRecord.RecordNum < GM_RSSP1_MAX_ONLINE_CONNECTION_NUM)
	{
		for (i=0U; i<g_DeleRecord.RecordNum; ++i)
		{
			if (g_DeleRecord.RecordIndex[i] == index) /* 不重复记录 */
			{
				break;
			}
		}

		if (i == g_DeleRecord.RecordNum)
		{
			g_DeleRecord.RecordIndex[g_DeleRecord.RecordNum] = index; /* 记录再末尾 */
			g_DeleRecord.RecordNum++;
		}
	}
}

GM_RSSP1_SeqDele_Record_Structure* GM_RSSP1_SeqDeleteRecord_Get(void)
{
	return &g_DeleRecord;
}
