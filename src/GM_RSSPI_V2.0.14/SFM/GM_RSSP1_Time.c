
#include "GM_RSSP1_Time.h"
#include "GM_RSSP1_Utils.h"
#ifdef SFM_Part

GM_RSSP1_INT32 GM_RSSP1_TIME_Init(GM_RSSP1_TIME *time, GM_RSSP1_UINT32 *left_tab1, GM_RSSP1_UINT32 *left_tab2, GM_RSSP1_UINT32 ts0_a, GM_RSSP1_UINT32 ts0_b)
{
    time->TC = 0U;

    time->oldTS[0] = 0U;
    time->oldTS[1] = 0U;

    time->TS[0] = ts0_a;
    time->TS[1] = ts0_b;

    time->left_tab[0] = left_tab1;
    time->left_tab[1] = left_tab2;

    return GM_RSSP1_OK;
}

GM_RSSP1_INT32 GM_RSSP1_TIME_STEP(GM_RSSP1_TIME *time)
{
    GM_RSSP1_INT32 i = 0;
	GM_RSSP1_LFSR l_reg = {0};

    time->TC = time->TC + 1U;

    for (i = 0; i < (GM_RSSP1_INT32)GM_RSSP1_CHECK_CHN_NUM; i++)
    {
        GM_RSSP1_LFSR_Init(&l_reg, time->left_tab[i], (GM_RSSP1_UINT32*)&i);
        GM_RSSP1_LFSR_Load(&l_reg, time->TS[i]);
        GM_RSSP1_LFSR_Add(&l_reg, 0U);
        time->oldTS[i] = time->TS[i];
        if(GM_RSSP1_ERROR == GM_RSSP1_LFSR_Read(&l_reg , &(time->TS[i])))
        {
            return GM_RSSP1_ERROR;
        }
    }

    return GM_RSSP1_OK;
}

GM_RSSP1_INT32 GM_RSSP1_TIME_GET(GM_RSSP1_TIME *time, GM_RSSP1_UINT32 *tc, GM_RSSP1_UINT32 *ts_a, GM_RSSP1_UINT32 *ts_b)
{
    *tc = time->TC;

    *ts_a = time->TS[0U];
    *ts_b = time->TS[1U];

    return GM_RSSP1_OK;
}

GM_RSSP1_INT32 GM_RSSP1_TIME_SET(GM_RSSP1_TIME *time, GM_RSSP1_UINT32 tc, GM_RSSP1_UINT32 ts_a, GM_RSSP1_UINT32 ts_b)
{

    time->TC = tc;

    time->TS[0U] = ts_a;
    time->TS[1U] = ts_b;

    return GM_RSSP1_OK;
}
#endif
