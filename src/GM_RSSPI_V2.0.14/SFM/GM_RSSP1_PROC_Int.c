#include "GM_RSSP1_PROC_Int.h"
#include "GM_RSSP1_Msg_Define.h"
#include "GM_RSSP1_Utils.h"

#ifdef SFM_Part
extern GM_RSSP1_VSN_GET_CALLBACK_FUN GM_RSSP1_VSN_Get_Callback_Fun;	/*added by huang 201511219*/
extern GM_RSSP1_UINT16 g_CrcmIndex;
extern GM_RSSP1_UINT8* g_pCrcmAddr;

/********************GM_RSSP1_CRCM_CHN_CHECK_Int********************************************************
用途:对接收到的报文进行FSFB校验
参数:crcm(接收报文某一通道的crcm值)
  crc32(接收报文CRC校验值)
  syscheck(系统校核字)
  left_tab(查表法线形逻辑左移运算表)
  right_tab(查表法线形逻辑右移运算表)
  msgTC(报文中计数器数值)
  tc(本机某通道计数器数值)
  localTC(本机器最近一次接收到报文时本机该通道计数器数值)
  remoteTC(本机器最近一次接收到报文时报文中计数器数值)
  dynamickey(sint+(sid~t(n-1))
  prec_sint(用于计算从dynamickey计算出新的dynamickey)
  post_data(存放正确计算结果, 范围跟随deltaTime变化)
  cycle(存放计算用周期常数)
  p_peer_crc (接收到的数据中的CRC值)
返回值:正确(GM_RSSP1_OK)
    错误(GM_RSSP1_ERROR)
***********************************************************************************************/
#ifdef OFFLINE_TOOL_SUPPORT
GM_RSSP1_INT32 GM_RSSP1_CRCM_CHN_CHECK_Int(GM_RSSP1_UINT32 crcm, GM_RSSP1_UINT32 crc32, GM_RSSP1_UINT32 syscheck, GM_RSSP1_UINT32 *left_tab, GM_RSSP1_UINT32 *right_tab, GM_RSSP1_UINT32 msgTC, GM_RSSP1_UINT32 tc, GM_RSSP1_UINT32 *localTC,
                            GM_RSSP1_UINT32 *remoteTC, GM_RSSP1_UINT32 *dynamickey, GM_RSSP1_UINT32 *prec_sint, GM_RSSP1_UINT32 *post_data, GM_RSSP1_UINT16 delta_time, GM_RSSP1_UINT16 Delay_time, GM_RSSP1_UINT32* cal_crc32, GM_RSSP1_UINT32* dynlockey, 
							GM_RSSP1_UINT32* CON1, GM_RSSP1_UINT32* CON2, GM_RSSP1_UINT32* CON3, GM_RSSP1_BOOL is_chn2)
#else
GM_RSSP1_INT32 GM_RSSP1_CRCM_CHN_CHECK_Int(GM_RSSP1_UINT32 crcm, GM_RSSP1_UINT32 crc32, GM_RSSP1_UINT32 syscheck, GM_RSSP1_UINT32 *left_tab, GM_RSSP1_UINT32 *right_tab, GM_RSSP1_UINT32 msgTC, GM_RSSP1_UINT32 tc, GM_RSSP1_UINT32 *localTC,
                            GM_RSSP1_UINT32 *remoteTC, GM_RSSP1_UINT32 *dynamickey, GM_RSSP1_UINT32 *prec_sint, GM_RSSP1_UINT32 *post_data, GM_RSSP1_UINT16 delta_time, GM_RSSP1_UINT16 Delay_time, GM_RSSP1_UINT32* cal_crc32)
#endif
{
    GM_RSSP1_UINT32 i = 0U;
    GM_RSSP1_UINT32 x = 0U;
    GM_RSSP1_UINT32 y = 0U;
    GM_RSSP1_UINT32 z = 0U;
    GM_RSSP1_LFSR l_reg = {0};
#ifdef OFFLINE_TOOL_SUPPORT
    GM_RSSP1_UINT32 VSN0 = 0U;
    GM_RSSP1_UINT32 VSN1 = 0U;
    GM_RSSP1_UINT32 VSN2 = 0U;
#endif
	GM_RSSP1_UINT32 offset = 0U;
	GM_RSSP1_UINT32 tmpValue = 0U;

    if (msgTC <= *remoteTC)/*不接收过时和重复数据*/
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_CRCM_CHN_CHECK_Int_Rem_Time_Fail, (int)msgTC, (int)*remoteTC, 0, 0, 0, 0);
        GM_RSSP1_Log_Msg(1,"rsd check:msgTC=%x, *remoteTC=%x\n", (int)msgTC, (int)*remoteTC, 0, 0, 0, 0);  
        return GM_RSSP1_RETURN_RESERVED;
    }

    if ((msgTC - *remoteTC) >= delta_time)/*不接收超过配置有效时间范围外的数据*/
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_CRCM_CHN_CHECK_Int_DELAY_TIME, (int)msgTC, (int)*remoteTC, (int)delta_time, 0, 0, 0);
        GM_RSSP1_Log_Msg(1,"rsd check:msgTC=%x, *remoteTC=%x, cycle->deltaTime=%d\n", (int)msgTC, (int)*remoteTC, (int)delta_time, 0, 0, 0);
        return GM_RSSP1_ERROR;
    }

	if (Delay_time > 0U)/*如果Delay_time为0,不进行本地接收时效范围检查*/
	{
		if ((tc - *localTC) >= Delay_time)
		{
			GM_RSSP1_Log_Usr(GM_RSSP1_CRCM_CHN_CHECK_Int_Loc_Time_Fail, (int)tc, (int)*localTC, (int)Delay_time, 0, 0, 0);
			GM_RSSP1_Log_Msg(1,"rsd check:tc=%d, *localTC=%d, cycle->Delay_time=%d\n", (int)tc, (int)*localTC, (int)Delay_time, 0, 0, 0);
			return GM_RSSP1_ERROR;
		}
	}

    /* Start, LFSR to check CRCM.  */
    x = crcm ^ crc32 ^ syscheck;

    GM_RSSP1_memset((void*)&l_reg, 0U, sizeof(l_reg));
    GM_RSSP1_LFSR_Init(&l_reg, left_tab, right_tab);
    GM_RSSP1_LFSR_Load(&l_reg, *dynamickey);

    for (i = 0U; i < (msgTC - *remoteTC - 1U); i++)
    {
		GM_RSSP1_LFSR_Add(&l_reg, 0U);
    }

    GM_RSSP1_LFSR_Add(&l_reg, x);

    if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , &x))
    {
        return GM_RSSP1_ERROR;
    }
    GM_RSSP1_LFSR_Load(&l_reg, 0U);
    GM_RSSP1_LFSR_Add(&l_reg, x);
    if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , &x))
    {
        return GM_RSSP1_ERROR;
    }

	offset = msgTC - *remoteTC - 1U;
	tmpValue = *(post_data + offset);
    if (x != tmpValue)
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_CRCM_CHN_CHECK_Int_CRCM_Fail, (int)msgTC, (int)offset, 0, 0, 0, 0);
        GM_RSSP1_Log_Msg(1,"rsd check:x=0x%x, *(post_data+(msgTC - *remoteTC-1)):0x%x\n",  (int)x, (int)tmpValue,(int)offset, 0,0,0);
        return GM_RSSP1_ERROR;
    }
	/* End, LFSR to check CRCM.  */

#ifdef OFFLINE_TOOL_SUPPORT
	/*将消息包内的CRCM中的Ts_r替换为Ts_l*/
	GM_RSSP1_VSN_Get_Callback_Fun(&VSN0, &VSN1, &VSN2);
	GM_RSSP1_LFSR_Load(&l_reg, *dynamickey);
	for (i = 0U; i < (msgTC - *remoteTC-1U); i++)
	{
		GM_RSSP1_LFSR_Add(&l_reg, 0U);
	}
	if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , &x))
	{
		return GM_RSSP1_ERROR;
	}
	if (GM_RSSP1_FALSE == is_chn2)
	{
		y = x^VSN1;
	}
	else
	{
		y = x^VSN2;
	}

	*dynlockey = y^crcm^crc32^(*(CON1 + msgTC - *remoteTC-1U));

	GM_RSSP1_LFSR_Add(&l_reg, (*(CON2 + msgTC - *remoteTC-1U)));

	if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , dynamickey))
	{
		return GM_RSSP1_ERROR;
	}

	*cal_crc32 = y^crcm^(*(CON3 + msgTC - *remoteTC-1U));
#else

    /* seperate crc32 from CRCM*/
    GM_RSSP1_LFSR_Load(&l_reg, 0U);
	GM_RSSP1_LFSR_Post(&l_reg, *(post_data + (msgTC - *remoteTC - 1U)));

    if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , &x))
    {
        return GM_RSSP1_ERROR;
    }
    GM_RSSP1_LFSR_Load(&l_reg, *dynamickey);
    for (i = 0U; i < (msgTC - *remoteTC-1U); i++)
    {
        GM_RSSP1_LFSR_Add(&l_reg, 0U);
    }

    GM_RSSP1_LFSR_Post(&l_reg, x);
    if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , &z))
    {
        return GM_RSSP1_ERROR;
    }
    *cal_crc32 ^= z ^ crcm ^ syscheck;

    /* Start, to generate latest dynamic key.*/
    GM_RSSP1_LFSR_Load(&l_reg, *dynamickey);

    for (i = 0U; i <= (msgTC - *remoteTC); i++)
    {
        GM_RSSP1_LFSR_Add(&l_reg, 0U);
    }

    if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , &x))
    {
        return GM_RSSP1_ERROR;
    }

    GM_RSSP1_LFSR_Load(&l_reg, *(prec_sint + (msgTC - *remoteTC - 1U)));
    GM_RSSP1_LFSR_Add(&l_reg, crcm ^ crc32 ^ syscheck);
    GM_RSSP1_LFSR_Add(&l_reg, crcm ^ crc32 ^ syscheck);
    if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , &y))
    {
        return GM_RSSP1_ERROR;
    }

    *dynamickey = x ^ y;
    /* End, to generate latest dynamic key.  */

#endif

    *remoteTC = msgTC;
    *localTC = tc;
    
    return GM_RSSP1_OK;
}

/**************************FSFB_SSE_BUILD*************************************************************************************
用途:生成SSE报文
参数:msg(SSE报文)
  sourAddr(FSFB SSE报文中 源地址)
  desAddr(FSFB SSE报文中 目标地址)
  tc(本机器的时间计数器)
  sid_1(通道1 sid)
  ts_1(通道1 时间戳)
  sid_2(通道2 sid)
  ts_2(通道2 时间戳)
  preSSETC1(上一次发送SSE 通道1 的时间计数器数值)
  preSSETC2(上一次发送SSE 通道2 的时间计数器数值)
  bSendSSE1(0xff(发送了SSE但还未收到SSR), 0x00(未发送SSE))
  bSendSSE2(0xff(发送了SSE但还未收到SSR), 0x00(未发送SSE))
返回值:生成SSE报文成功(GM_RSSP1_OK)
    生成SSE报文失败(GM_RSSP1_ERROR)
*****************************************************************************************************************************/
GM_RSSP1_INT32 GM_RSSP1_SSE_BUILD_Int(GM_RSSP1_UINT8 *pResult , GM_RSSP1_UINT16 *pResult_len , GM_RSSP1_UINT16 sourAddr, GM_RSSP1_UINT16 desAddr, GM_RSSP1_UINT32 tc, GM_RSSP1_UINT32 sid_1, GM_RSSP1_UINT32 ts_1, GM_RSSP1_UINT32 sid_2, GM_RSSP1_UINT32 ts_2,
                       GM_RSSP1_UINT32 *preSSETC1, GM_RSSP1_UINT32 *preSSETC2, GM_RSSP1_UINT32 *preSSETS1, GM_RSSP1_UINT32 *preSSETS2, GM_RSSP1_UINT8 *bSendSSE1, GM_RSSP1_UINT8 *bSendSSE2, GM_RSSP1_UINT16 lifeTime , GM_RSSP1_UINT16 *pCRC16_table)
{
    GM_RSSP1_UINT32 SEQENQ_1 = 0U;
    GM_RSSP1_UINT32 SEQENQ_2 = 0U;
    GM_RSSP1_UINT16 crc16 = 0U;

    if (*bSendSSE1 == 0xffU)
    {
        if ((tc - *preSSETC1) <= lifeTime) /*未超过消息失效范围，不允许多发SSE*/
        {
			GM_RSSP1_Log_Usr(GM_RSSP1_SSE_BUILD_Int_Send_Invalid, desAddr, 0, (int)tc, (int)*preSSETC1, 0, 0);
			GM_RSSP1_Log_Msg(1,"The last sendout SSE has not overtiome:LastTc1 0x%x, CurTC 0x%x, LifeTime  0x%x\n",  (int)*preSSETC1, (int)tc,(int)lifeTime, 0, 0, 0);
            return GM_RSSP1_ERROR;
        }
    }

    if (*bSendSSE2 == 0xffU) /*已发送过SSE*/
    {
        if ((tc - *preSSETC2) <= lifeTime)
        {
			GM_RSSP1_Log_Usr(GM_RSSP1_SSE_BUILD_Int_Send_Invalid, desAddr, 1, (int)tc, (int)*preSSETC2, 0, 0);
			GM_RSSP1_Log_Msg(1,"The last sendout SSE has not overtiome:LastTc2 0x%x, CurTC 0x%x, LifeTime  0x%x\n",  (int)*preSSETC2, (int)tc,(int)lifeTime, 0, 0, 0);
            return GM_RSSP1_ERROR;
        }
    }

    SEQENQ_1    = sid_1 ^ ts_1;

    SEQENQ_2    = sid_2 ^ ts_2;

	/**Formate SSE Message Structure**/
    *(pResult + 0U) = RSSP1_SSE_PROTOCOL_TYPE;
    *(pResult + 1U) = RSSP1_SSE_FRAME_TYPE;
    *(pResult + 2U) = (GM_RSSP1_UINT8)(sourAddr & 0xffU);
    *(pResult + 3U) = (GM_RSSP1_UINT8)(sourAddr >> 8U);
    *(pResult + 4U) = (GM_RSSP1_UINT8)(desAddr & 0xffU);
    *(pResult + 5U) = (GM_RSSP1_UINT8)(desAddr >> 8U);

    *(pResult + 6U) = (GM_RSSP1_UINT8)(tc & 0xffU);
    *(pResult + 7U) = (GM_RSSP1_UINT8)((tc & 0xff00U) >> 8U);
    *(pResult + 8U) = (GM_RSSP1_UINT8)((tc & 0xff0000U) >> 16U);
    *(pResult + 9U) = (GM_RSSP1_UINT8)((tc & 0xff000000U) >> 24U);

    *(pResult + 10U) = (GM_RSSP1_UINT8)(SEQENQ_1 & 0xffU);
    *(pResult + 11U) = (GM_RSSP1_UINT8)((SEQENQ_1 & 0xff00U) >> 8U);
    *(pResult + 12U) = (GM_RSSP1_UINT8)((SEQENQ_1 & 0xff0000U) >> 16U);
    *(pResult + 13U) = (GM_RSSP1_UINT8)((SEQENQ_1 & 0xff000000U) >> 24U);

    *(pResult + 14U) = (GM_RSSP1_UINT8)(SEQENQ_2 & 0xffU);
    *(pResult + 15U) = (GM_RSSP1_UINT8)((SEQENQ_2 & 0xff00U) >> 8U);
    *(pResult + 16U) = (GM_RSSP1_UINT8)((SEQENQ_2 & 0xff0000U) >> 16U);
    *(pResult + 17U) = (GM_RSSP1_UINT8)((SEQENQ_2 & 0xff000000U) >> 24U);

    GM_RSSP1_CRC_Calculate_CRC16(pResult, 18U, &crc16, pCRC16_table);
    *(pResult + 18U)        = (GM_RSSP1_UINT8)(crc16 & 0xffU);
    *(pResult + 19U)        = (GM_RSSP1_UINT8)(crc16 >> 8U);

	/**Formate End**/

    *pResult_len = 20U;

    *preSSETC1 = tc;
    *preSSETC2 = tc;
    *preSSETS1 = ts_1;
    *preSSETS2 = ts_2;
    *bSendSSE1 = 0xffU;
    *bSendSSE2 = 0xffU;

    GM_RSSP1_Log_Msg(4,"Sending SSE: Dst 0x%x, TC= %d, TS_1 = %x, TS_2 = %x\n", (int)desAddr, (int)tc, (int)ts_1, (int)ts_2, 0, 0);

    if (*pResult_len > GM_RSSP1_MAX_SAFETY_DAT_LEN)
    {
        return GM_RSSP1_ERROR;
    }
    else
    {
        return GM_RSSP1_OK;
    }
}

/*********************************FSFB_SSR_BUILD****************************************************************************
用途:生成SSR报文
参数:msg(生成报文)
  sourAddr(FSFB SSR报文中 源地址)
  desAddr(FSFB SSR报文中 目标地址)
  seqenq_1(对方发送SSE报文中通道1 seqenq数值)
  seqenq_2(对方发送SSE报文中通道2 seqenq数值)
  sid_r1(本机通道1 sid)
  sid_r2(本机通道2 sid)
  dataver_r1(对方设备通道1的dataver)
  dataver_r2(对方设备通道1的dataver)
  ts_r1(本机通道1 时间戳)
  ts_r2(本机通道2 时间戳)
  tc_e(对应sse报文中时间计数器数值)
  tc_r(本机时间计数器数值)
  num_dataver(版本数值,固定为1)
返回值:生成SSR报文成功(GM_RSSP1_OK)
    生成SSR报文失败(GM_RSSP1_ERROR)
****************************************************************************************************************************/
GM_RSSP1_INT32 GM_RSSP1_SSR_BUILD_Int(GM_RSSP1_UINT8 *pResult , GM_RSSP1_UINT16 *pResult_len , GM_RSSP1_UINT16 sourAddr, GM_RSSP1_UINT16 desAddr, GM_RSSP1_UINT32 seqenq_1, GM_RSSP1_UINT32 seqenq_2, GM_RSSP1_UINT32 sid_r1, GM_RSSP1_UINT32 sid_r2,
                       GM_RSSP1_UINT32 dataver_r1, GM_RSSP1_UINT32 dataver_r2, GM_RSSP1_UINT32 ts_r1, GM_RSSP1_UINT32 ts_r2, GM_RSSP1_UINT32 tc_e, GM_RSSP1_UINT32 tc_r, GM_RSSP1_UINT8 num_dataver , GM_RSSP1_UINT16 *pCRC16_table)
{
    GM_RSSP1_UINT32 SEQINI_1 = 0U;
    GM_RSSP1_UINT32 SEQINI_2 = 0U;
    GM_RSSP1_UINT16 crc16 = 0U;

    SEQINI_1 = seqenq_1 ^ sid_r1 ^ dataver_r1 ^ ts_r1;
    SEQINI_2 = seqenq_2 ^ sid_r2 ^ dataver_r2 ^ ts_r2;

	/**Formate SSR Message Structure**/
    *(pResult + 0U) = RSSP1_SSR_PROTOCOL_TYPE;
    *(pResult + 1U) = RSSP1_SSR_FRAME_TYPE;
    *(pResult + 2U) = (GM_RSSP1_UINT8)(sourAddr & 0xffU);
    *(pResult + 3U) = (GM_RSSP1_UINT8)(sourAddr >> 8U);
    *(pResult + 4U) = (GM_RSSP1_UINT8)(desAddr & 0xffU);
    *(pResult + 5U) = (GM_RSSP1_UINT8)(desAddr >> 8U);

    *(pResult + 6U) = (GM_RSSP1_UINT8)(tc_r & 0xffU);
    *(pResult + 7U) = (GM_RSSP1_UINT8)((tc_r & 0xff00U) >> 8U);
    *(pResult + 8U) = (GM_RSSP1_UINT8)((tc_r & 0xff0000U) >> 16U);
    *(pResult + 9U) = (GM_RSSP1_UINT8)((tc_r & 0xff000000U) >> 24U);

    *(pResult + 10U) = (GM_RSSP1_UINT8)(tc_e & 0xffU);
    *(pResult + 11U) = (GM_RSSP1_UINT8)((tc_e & 0xff00U) >> 8U);
    *(pResult + 12U) = (GM_RSSP1_UINT8)((tc_e & 0xff0000U) >> 16U);
    *(pResult + 13U) = (GM_RSSP1_UINT8)((tc_e & 0xff000000U) >> 24U);

    *(pResult + 14U) = (GM_RSSP1_UINT8)(SEQINI_1 & 0xffU);
    *(pResult + 15U) = (GM_RSSP1_UINT8)((SEQINI_1 & 0xff00U) >> 8U);
    *(pResult + 16U) = (GM_RSSP1_UINT8)((SEQINI_1 & 0xff0000U) >> 16U);
    *(pResult + 17U) = (GM_RSSP1_UINT8)((SEQINI_1 & 0xff000000U) >> 24U);

    *(pResult + 18U) = (GM_RSSP1_UINT8)(SEQINI_2 & 0xffU);
    *(pResult + 19U) = (GM_RSSP1_UINT8)((SEQINI_2 & 0xff00U) >> 8U);
    *(pResult + 20U) = (GM_RSSP1_UINT8)((SEQINI_2 & 0xff0000U) >> 16U);
    *(pResult + 21U) = (GM_RSSP1_UINT8)((SEQINI_2 & 0xff000000U) >> 24U);

    *(pResult + 22U) = (GM_RSSP1_UINT8)(num_dataver);

	GM_RSSP1_CRC_Calculate_CRC16(pResult, 23U, &crc16, pCRC16_table);

    *(pResult + 23U) = (GM_RSSP1_UINT8)(crc16 & 0xffU);
    *(pResult + 24U) = (GM_RSSP1_UINT8)(crc16 >> 8U);
	/**Formate End**/

    *pResult_len = 25U;

	GM_RSSP1_Log_Msg(4,"Sending SSR: TC_R= %d, TC_E = %d, TS_R1 = %x\n", (int)tc_r, (int)tc_e, (int)ts_r1, 0, 0, 0);

	if (*pResult_len > GM_RSSP1_MAX_SAFETY_DAT_LEN)
    {
        return GM_RSSP1_ERROR;
    }
    else
    {
        return GM_RSSP1_OK;
    }
}

/**********************************GM_RSSP1_SSR_CHN_SET***************************************************************************
用途:检查接收到SSR报文，如消息有效则根据报文设置接收方的dynamic用于RSD中的CRCM校验
参数:left_tab(查表法线形逻辑左移运算表)
  right_tab(查表法线形逻辑右移运算表)
  seqini(接收到SSR报文中的seqini数值)
  prec_fisrtsint(PREC_FISRTSINT数值)
  preSSEtc(本机发送SSE报文的时间计数器数值)
  preSSEts(本机发送SSE报文的时间计数器数值)
  dynamic
  tc(本机当前时间计数器数值)
  localTC(接收到报文时本机的时间计数器数值)
  remoteTC(接收到报文时对方的时间计数器数值)
  TC_E(SSR报文中对应的SSE报文中的计数器数值)
  TC_R(SSR报文中对方的时间计数器数值)
  bSendSSE(本机是否发送SSE, 0x00表示未发送 0xff表示发送后未收到SSR)
返回值:成功(GM_RSSP1_OK)
    失败(GM_RSSP1_ERROR)
*****************************************************************************************************************************/
GM_RSSP1_INT32 GM_RSSP1_SSR_CHN_SET(GM_RSSP1_UINT32 *left_tab, GM_RSSP1_UINT32 *right_tab, GM_RSSP1_UINT32 seqini, GM_RSSP1_UINT32 prec_firstsint, GM_RSSP1_UINT32 preSSEtc, GM_RSSP1_UINT32 preSSEts, GM_RSSP1_UINT32 *dynamic,
                     GM_RSSP1_UINT32 tc, GM_RSSP1_UINT32 *localTC, GM_RSSP1_UINT32 *remoteTC, GM_RSSP1_UINT32 TC_E,  GM_RSSP1_UINT32 TC_R, GM_RSSP1_UINT8 *bSendSSE , GM_RSSP1_UINT32 ssr_valid_cycle)
{
	GM_RSSP1_LFSR l_reg = {0};

    if (TC_E != preSSEtc) /**判断此包SSR回复的是否为本地最新的申请消息SSE**/
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_SSR_CHN_SET_Msg_Invalid, TC_E, preSSEtc, 0, 0, 0, 0);
        GM_RSSP1_Log_Msg(1,"SSR set: TC_E=%ld, preSSEtc=%ld\n", (int)TC_E, (int)preSSEtc, 0, 0, 0, 0);
        return GM_RSSP1_ERROR;
    }

    if (0x00U == *bSendSSE) /**判断是否发出过申请SSE**/
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_SSR_CHN_SET_Msg_NoSEE, 0, 0, 0, 0, 0, 0);
        GM_RSSP1_Log_Msg(1,"SSR set err! local didn't send SSE!\n", (int)TC_E, (int)preSSEtc, 0, 0, 0, 0);
        return GM_RSSP1_ERROR;
    }

    /* bgn 增加对SSR时效性判断,不考虑TC回卷，。 jianghongjun TSRS00000125 */
    if (tc - TC_E > ssr_valid_cycle)
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_SSR_CHN_SET_Msg_TimeOut, (int)TC_E, (int)tc, (int)ssr_valid_cycle, 0, 0, 0);
        GM_RSSP1_Log_Msg(1,"SSR expire! Tsendsse:%d,now:%d.\n", (int)TC_E, (int)tc, (int)ssr_valid_cycle, 0, 0, 0);
        return GM_RSSP1_ERROR;
    }
    /* end */

    GM_RSSP1_memset((void*)&l_reg, 0U, sizeof(l_reg));

    GM_RSSP1_LFSR_Init(&l_reg, left_tab, right_tab);
    GM_RSSP1_LFSR_Load(&l_reg, prec_firstsint);
	GM_RSSP1_LFSR_Add(&l_reg, seqini ^ preSSEts);

    if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , dynamic))
    {
        return GM_RSSP1_ERROR;
    }
    GM_RSSP1_Log_Msg(4,"Rcving SSR, TC =  %d , TC_R: = %d \n", (int)tc, (int)TC_R, 0, 0, 0, 0);

    *localTC = tc;
    *remoteTC = TC_R;

    *bSendSSE = 0U;

    return GM_RSSP1_OK;
}

/*******************************FSFB_XSD_BUILD*********************************************************************
用途:生成RSD报文
参数:msg(报文)
     len(需要封装报文的长度)
  mstorslv(主备机标志,0x01:主机, 0x02备机)
  aorb(A\B系标志, 0x80:A机, 0x81:B机)
  sourAddr(FSFB RSD报文中 源地址)
  desAddr(FSFB RSD报文中 目标地址)
  tc(本机时间计数器数值)
  crcm1(通道1 crcm数值)
  crcm2(通道2 crcm数值)
返回值:成功(GM_RSSP1_OK)
    失败(GM_RSSP1_ERROR)
******************************************************************************************************************/
GM_RSSP1_INT32 GM_RSSP1_RSD_BUILD_Int(GM_RSSP1_UINT8 *pResult , GM_RSSP1_UINT16 *pResult_len , GM_RSSP1_UINT8 *app_data, GM_RSSP1_UINT16 app_data_len, GM_RSSP1_UINT8 mstorslv, GM_RSSP1_UINT8 aorb, GM_RSSP1_UINT16 sourAddr, GM_RSSP1_UINT16 desAddr,
                       GM_RSSP1_UINT32 tc, GM_RSSP1_UINT32 crcm1, GM_RSSP1_UINT32 crcm2 , GM_RSSP1_UINT16* pCRC16_table, GM_RSSP1_UINT8* CrcmRes)
{
    GM_RSSP1_UINT16 crc16 = 0U;



		/**Formate RSD Message Structure**/ /* RAM1 */
		*(pResult + 0U) = mstorslv;

		*(pResult + 1U) = aorb;
		*(pResult + 2U) = (GM_RSSP1_UINT8)(sourAddr & 0xffU);
		*(pResult + 3U) = (GM_RSSP1_UINT8)(sourAddr >> 8U);
		*(pResult + 4U) = (GM_RSSP1_UINT8)(desAddr & 0xffU);
		*(pResult + 5U) = (GM_RSSP1_UINT8)(desAddr >> 8U);

		*(pResult + 6U) = (GM_RSSP1_UINT8)(tc & 0xffU);
		*(pResult + 7U) = (GM_RSSP1_UINT8)((tc & 0xff00U) >> 8U);
		*(pResult + 8U) = (GM_RSSP1_UINT8)((tc & 0xff0000U) >> 16U);
		*(pResult + 9U) = (GM_RSSP1_UINT8)((tc & 0xff000000U) >> 24U);

		*(pResult + 10U) = (GM_RSSP1_UINT8)((app_data_len + 8U) & 0xffU);
		*(pResult + 11U) = (GM_RSSP1_UINT8)((app_data_len + 8U) >> 8U);


		/* 与CRCM异或 */
		*(pResult + 12U) ^= (GM_RSSP1_UINT8)(crcm1 & 0xffU);
		*(pResult + 13U) ^= (GM_RSSP1_UINT8)((crcm1 & 0xff00U) >> 8U);
		*(pResult + 14U) ^= (GM_RSSP1_UINT8)((crcm1 & 0xff0000U) >> 16U);
		*(pResult + 15U) ^= (GM_RSSP1_UINT8)((crcm1 & 0xff000000U) >> 24U);

		*(pResult + 16U) ^= (GM_RSSP1_UINT8)(crcm2 & 0xffU);
		*(pResult + 17U) ^= (GM_RSSP1_UINT8)((crcm2 & 0xff00U) >> 8U);
		*(pResult + 18U) ^= (GM_RSSP1_UINT8)((crcm2 & 0xff0000U) >> 16U);
		*(pResult + 19U) ^= (GM_RSSP1_UINT8)((crcm2 & 0xff000000U) >> 24U);


		GM_RSSP1_memcpy((void*)(pResult + 20U), (const void*)app_data, (size_t)app_data_len);


		GM_RSSP1_CRC_Calculate_CRC16(pResult, 20U + app_data_len, &crc16, pCRC16_table);
		
		/* CR: 11036019*/
		if (NULL != CrcmRes)
		{
			g_CrcmIndex = g_CrcmIndex  % GM_RSSP1_MAX_TX_MSG_CYCLE;
			CrcmRes = g_pCrcmAddr + g_CrcmIndex * 8U;
		

			/* 与CRCM异或 */
			*(pResult + 12U) = (GM_RSSP1_UINT8)(g_CrcmIndex & 0xffU);
			*(pResult + 13U) = (GM_RSSP1_UINT8)((g_CrcmIndex & 0xff00U) >> 8U);
			*(pResult + 14U) = (GM_RSSP1_UINT8)0;
			*(pResult + 15U) = (GM_RSSP1_UINT8)0;

			*(pResult + 16U) = (GM_RSSP1_UINT8)0;
			*(pResult + 17U) = (GM_RSSP1_UINT8)0;
			*(pResult + 18U) = (GM_RSSP1_UINT8)0;
			*(pResult + 19U) = (GM_RSSP1_UINT8)0;

			/* RAM2 */
			*(CrcmRes + 0U) ^= (GM_RSSP1_UINT8)(crcm1 & 0xffU);
			*(CrcmRes + 1U) ^= (GM_RSSP1_UINT8)((crcm1 & 0xff00U) >> 8U);
			*(CrcmRes + 2U) ^= (GM_RSSP1_UINT8)((crcm1 & 0xff0000U) >> 16U);
			*(CrcmRes + 3U) ^= (GM_RSSP1_UINT8)((crcm1 & 0xff000000U) >> 24U);

			*(CrcmRes + 4U) ^= (GM_RSSP1_UINT8)(crcm2 & 0xffU);
			*(CrcmRes + 5U) ^= (GM_RSSP1_UINT8)((crcm2 & 0xff00U) >> 8U);
			*(CrcmRes + 6U) ^= (GM_RSSP1_UINT8)((crcm2 & 0xff0000U) >> 16U);
			*(CrcmRes + 7U) ^= (GM_RSSP1_UINT8)((crcm2 & 0xff000000U) >> 24U);

			g_CrcmIndex = g_CrcmIndex + 1;
		}


	*(pResult + 20U + app_data_len)        = (GM_RSSP1_UINT8)(crc16 & 0xffU);
	*(pResult + 21U + app_data_len)        = (GM_RSSP1_UINT8)(crc16 >> 8U);
	/**Formate End**/

    *pResult_len = 22U + app_data_len;

    GM_RSSP1_Log_Msg(4,"Sending to %x RSD: TC = %d CRCM1 = %x CRCM2 = %x\n", (int)desAddr, (int)tc, (int)crcm1, (int)crcm2, 0, 0);


    if (*pResult_len > GM_RSSP1_MAX_SAFETY_DAT_LEN)
    {		
        return GM_RSSP1_ERROR;
    }
    else
    {
        return GM_RSSP1_OK;
    }
}

/***************************************FSFB_PREC_SINT_CAL***********************************************************
用途:初始化时计算prec_sinit
参数:sinit(sint参数)
     sid(sid参数)
  prec_sinit()
  left_tab(查表法线形逻辑左移运算表)
  right_tab(查表法线形逻辑右移运算表)
返回值:成功(GM_RSSP1_OK)
    失败(GM_RSSP1_ERROR)
*********************************************************************************************************************/
GM_RSSP1_INT32 GM_RSSP1_PREC_SINT_CAL_Int(GM_RSSP1_UINT32 sinit, GM_RSSP1_UINT32 sid, GM_RSSP1_UINT32 *prec_sinit, GM_RSSP1_UINT32 *left_tab, GM_RSSP1_UINT32 *right_tab , GM_RSSP1_INT32 max_delta_time)
{
    GM_RSSP1_INT32 i = 0;
    GM_RSSP1_INT32 j = 0;
    GM_RSSP1_UINT32 x = 0U;
    GM_RSSP1_LFSR l_reg = {0U};
    GM_RSSP1_memset((void*)&l_reg, 0U, sizeof(l_reg));
    GM_RSSP1_LFSR_Init(&l_reg, left_tab, right_tab);

    if ((max_delta_time < 0) || (max_delta_time > (GM_RSSP1_INT32)GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE))
    {
		GM_RSSP1_Log_Usr(GM_RSSP1_PREC_SINT_CAL_Int_TimeOut, (int)max_delta_time, (int)GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE, 0, 0, 0, 0);
        return GM_RSSP1_ERROR;
    }

    for (i = 0; i < max_delta_time; i++)
    {
        GM_RSSP1_LFSR_Load(&l_reg, sinit);
		GM_RSSP1_LFSR_Add(&l_reg, sid);

        
        for (j = 0; j < i; j++)
        {
            GM_RSSP1_LFSR_Add(&l_reg, 0U);
        }


		GM_RSSP1_LFSR_Post(&l_reg, sinit);

        

        if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , &x))
        {
            return GM_RSSP1_ERROR;
        }
        *(prec_sinit + i) = x ^ sid;
    }

    return GM_RSSP1_OK;
}

/***************************GM_RSSP1_PREC_FIRSTSINIT_CAL**************************************************************************
用途:计算prec_firstinit
参数:sint_r(对方节点的sint数值)
  sid_e(本节点的sid数值)
  dataver_r(对方节点的dataver数值)
  prec_firstsinit(返回的针对对方节点的prec_firstsinit数值)
  left_tab(查表法线形逻辑左移运算表)
  right_tab(查表法线形逻辑右移运算表)
返回值:成功(GM_RSSP1_OK)
    失败(GM_RSSP1_ERROR)
*****************************************************************************************************************************/
GM_RSSP1_INT32 GM_RSSP1_PREC_FIRSTSINIT_CAL(GM_RSSP1_UINT32 sint_r, GM_RSSP1_UINT32 sid_e, GM_RSSP1_UINT32 dataver_r, GM_RSSP1_UINT32 *prec_firstsinit, GM_RSSP1_UINT32 *left_tab, GM_RSSP1_UINT32 *right_tab)
{
    GM_RSSP1_UINT32 x = 0U;
    GM_RSSP1_LFSR l_reg = {0};

    if((NULL == prec_firstsinit) || (NULL == left_tab) || (NULL == right_tab) )
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_PREC_SINT_CAL_Int_Param_Point_Error, (int)prec_firstsinit, (int)left_tab, (int)right_tab, 0, 0, 0);
        return GM_RSSP1_ERROR;
    }
    GM_RSSP1_memset((void*)&l_reg, 0U, sizeof(l_reg));
    GM_RSSP1_LFSR_Init(&l_reg, left_tab, right_tab);
    GM_RSSP1_LFSR_Load(&l_reg, sint_r);
	GM_RSSP1_LFSR_Add(&l_reg, sid_e ^ dataver_r);

    
    if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , &x))
    {
        return GM_RSSP1_ERROR;
    }
    GM_RSSP1_LFSR_Load(&l_reg, 0U);
	GM_RSSP1_LFSR_Post(&l_reg, x);

    
    if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , prec_firstsinit))
    {
        return GM_RSSP1_ERROR;
    }
    return GM_RSSP1_OK;
}

/*********************************GM_RSSP1_POST_RXDATA_Int***************************************************************************
用途:预计算校验结果post_rxdata
参数:sinit(对方节点sinit)
     sid(对方节点的sid)
  post_rxdata(post_rxdata计算结果)
  left_tab(查表法线形逻辑左移运算表)
  right_tab(查表法线形逻辑右移运算表)
返回值:成功(GM_RSSP1_OK)
    失败(GM_RSSP1_ERROR)
****************************************************************************************************************************/
GM_RSSP1_INT32 GM_RSSP1_POST_RXDATA_Int(GM_RSSP1_UINT32 sinit, GM_RSSP1_UINT32 sid, GM_RSSP1_UINT32 *post_rxdata, GM_RSSP1_UINT32 *left_tab, GM_RSSP1_UINT32 *right_tab , GM_RSSP1_INT32 max_delta_time)
{
    GM_RSSP1_INT32 i = 0;
    GM_RSSP1_INT32 j = 0;
    GM_RSSP1_UINT32 x = 0U;
    GM_RSSP1_LFSR l_reg = {0};

    if ((max_delta_time < 0) || (max_delta_time > (GM_RSSP1_INT32)GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE))
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_POST_RXDATA_Int_TimeOut, (int)max_delta_time, (int)GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE, 0, 0, 0, 0);
        return GM_RSSP1_ERROR;
    }

    GM_RSSP1_memset((void*)&l_reg, 0U, sizeof(l_reg));

    GM_RSSP1_LFSR_Init(&l_reg, left_tab, right_tab);

    for (i = 0; i < max_delta_time; i++)
    {
        GM_RSSP1_LFSR_Load(&l_reg, sinit);
		GM_RSSP1_LFSR_Add(&l_reg, sid);

        
        for (j = 0; j < i; j++)
        {
            GM_RSSP1_LFSR_Add(&l_reg, 0U);
        }

        GM_RSSP1_LFSR_Add(&l_reg, sid);

        if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , &x))
        {
            return GM_RSSP1_ERROR;
        }
        GM_RSSP1_LFSR_Load(&l_reg, 0U);
        GM_RSSP1_LFSR_Add(&l_reg, x);
        if(GM_RSSP1_FALSE == GM_RSSP1_LFSR_Read(&l_reg , post_rxdata + i))
        {
            return GM_RSSP1_ERROR;
        }
    }

    return GM_RSSP1_OK;
}
#endif
