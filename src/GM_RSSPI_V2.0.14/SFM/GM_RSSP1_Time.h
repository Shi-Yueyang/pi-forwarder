#ifndef GM_RSSP1_TIME_H_
#define GM_RSSP1_TIME_H_

#include "GM_RSSP1_Utils.h"

#ifdef __cplusplus
extern "C"
{
#endif

    struct _GM_RSSP1_TIME_CYCLE_
    {
        GM_RSSP1_UINT8 lifeTime;
        GM_RSSP1_UINT8 deltaTime;
    };

    typedef struct _GM_RSSP1_TIME_CYCLE_ GM_RSSP1_TIME_CYCLE_;

    struct _GM_RSSP1_TIME_
    {
        /*
         SEM_ID  semMTime;     / *互斥信号量* /
        */
        GM_RSSP1_UINT32 *left_tab[GM_RSSP1_CHECK_CHN_NUM];
        GM_RSSP1_UINT32  TC;
        GM_RSSP1_UINT32  TS[GM_RSSP1_CHECK_CHN_NUM];
        GM_RSSP1_UINT32  oldTS[GM_RSSP1_CHECK_CHN_NUM];
    };

    typedef struct _GM_RSSP1_TIME_ GM_RSSP1_TIME;

	GM_RSSP1_INT32 GM_RSSP1_TIME_Init(GM_RSSP1_TIME *time, GM_RSSP1_UINT32 *left_tab1, GM_RSSP1_UINT32 *left_tab2, GM_RSSP1_UINT32 ts0_a, GM_RSSP1_UINT32 ts0_b);
    GM_RSSP1_INT32 GM_RSSP1_TIME_STEP(GM_RSSP1_TIME *time);
    GM_RSSP1_INT32 GM_RSSP1_TIME_GET(GM_RSSP1_TIME *time, GM_RSSP1_UINT32 *tc, GM_RSSP1_UINT32 *ts_a, GM_RSSP1_UINT32 *ts_b);
    GM_RSSP1_INT32 GM_RSSP1_TIME_SET(GM_RSSP1_TIME *time, GM_RSSP1_UINT32 tc, GM_RSSP1_UINT32 ts_a, GM_RSSP1_UINT32 ts_b);

#ifdef __cplusplus
}
#endif


#endif
