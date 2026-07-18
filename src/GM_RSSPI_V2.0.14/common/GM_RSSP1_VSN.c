/**
* @file GM_RSSP1_VSN.c
* @brief
* @author Hao Liming
* @date 2010-4-4 11:15:06
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>haoliming@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/

#include "GM_RSSP1_VSN.h"

typedef struct
{
    GM_RSSP1_UINT32 VSN0;
    GM_RSSP1_UINT32 VSN1;
    GM_RSSP1_UINT32 VSN2;
    GM_RSSP1_UINT32 VSNSeed1;
    GM_RSSP1_UINT32 VSNSeed2;
    GM_RSSP1_UINT32 oldVSN1;
    GM_RSSP1_UINT32 oldVSN2;
    GM_RSSP1_LFSR   channel1;
    GM_RSSP1_LFSR   channel2;
}GM_RSSP1_PACKED GM_RSSP1_VSN_Struct;/*原GM_RSSP1_VSN的结构，无VSNID*/



/*原GM_RSSP1_VSN*/
static GM_RSSP1_VSN_Struct	 GM_RSSP1_VSN_Info;

GM_RSSP1_BOOL GM_RSSP1_VSN_Init(void)
{
    GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
	static const GM_RSSP1_VSN_Struct GM_RSSP1_VSN_Ini0 =
	{
		0x0U, 0x0U, 0x0U, GM_RSSP1_VSN_SEED1, GM_RSSP1_VSN_SEED2, 0x0U, 0x0U, {0x00U, NULL, NULL}, {0x00U, NULL, NULL}
	};

    GM_RSSP1_memcpy((void*)(&GM_RSSP1_VSN_Info), (const void*)(&GM_RSSP1_VSN_Ini0),(size_t)sizeof(GM_RSSP1_VSN_Info));

    rt = GM_RSSP1_LFSR_Init((GM_RSSP1_LFSR*)(&(GM_RSSP1_VSN_Info.channel1)), GM_RSSP1_LFSR_LEFT_CHN1_TABLE, GM_RSSP1_LFSR_RIGHT_CHN1_TABLE);

    if ((GM_RSSP1_BOOL)GM_RSSP1_FALSE == (GM_RSSP1_BOOL)rt)
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_VSN_INIT_LFSR_1_ERR, 0, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    rt = GM_RSSP1_LFSR_Init((GM_RSSP1_LFSR*)(&(GM_RSSP1_VSN_Info.channel2)), GM_RSSP1_LFSR_LEFT_CHN2_TABLE, GM_RSSP1_LFSR_RIGHT_CHN2_TABLE);

    if ((GM_RSSP1_BOOL)GM_RSSP1_FALSE == (GM_RSSP1_BOOL)rt)
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_VSN_INIT_LFSR_2_ERR, 0, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    GM_RSSP1_VSN_Info.VSN0 = (GM_RSSP1_UINT32)0U;

    GM_RSSP1_VSN_Info.VSN1 = (GM_RSSP1_UINT32)GM_RSSP1_VSN_M01;

    GM_RSSP1_VSN_Info.VSN2 = (GM_RSSP1_UINT32)GM_RSSP1_VSN_M02;

    GM_RSSP1_VSN_Info.VSN1 ^= GM_RSSP1_VSN_Info.VSNSeed1;

    GM_RSSP1_VSN_Info.VSN2 ^= GM_RSSP1_VSN_Info.VSNSeed2;

    return GM_RSSP1_TRUE;
}

void GM_RSSP1_VSN_Update(void)
{
    GM_RSSP1_UINT32 temp = 0U;

    ++GM_RSSP1_VSN_Info.VSN0;
    GM_RSSP1_VSN_Info.oldVSN1 = GM_RSSP1_VSN_Info.VSN1;
    GM_RSSP1_VSN_Info.oldVSN2 = GM_RSSP1_VSN_Info.VSN2;

    GM_RSSP1_LFSR_Load((GM_RSSP1_LFSR*)(&(GM_RSSP1_VSN_Info.channel1)), (GM_RSSP1_UINT32)(GM_RSSP1_VSN_Info.VSN1 ^ (GM_RSSP1_UINT32)GM_RSSP1_VSN_M01));

    GM_RSSP1_LFSR_Add((GM_RSSP1_LFSR*)(&(GM_RSSP1_VSN_Info.channel1)), (GM_RSSP1_UINT32)0u);

    /*GM_RSSP1_LFSR_Read((GM_RSSP1_LFSR*)(&(GM_RSSP1_VSN_Info.channel1)), (GM_RSSP1_UINT32*)(&(GM_RSSP1_VSN_Info.VSN1)));*/
    GM_RSSP1_LFSR_Read((GM_RSSP1_LFSR*)(&(GM_RSSP1_VSN_Info.channel1)), &temp);
    GM_RSSP1_VSN_Info.VSN1 = temp;

    GM_RSSP1_VSN_Info.VSN1 ^= (GM_RSSP1_UINT32)(GM_RSSP1_VSN_M01);

    GM_RSSP1_LFSR_Load((GM_RSSP1_LFSR*)(&(GM_RSSP1_VSN_Info.channel2)), (GM_RSSP1_UINT32)(GM_RSSP1_VSN_Info.VSN2 ^ (GM_RSSP1_UINT32)GM_RSSP1_VSN_M02));

    GM_RSSP1_LFSR_Add((GM_RSSP1_LFSR*)(&(GM_RSSP1_VSN_Info.channel2)), (GM_RSSP1_UINT32)0u);

    GM_RSSP1_LFSR_Read((GM_RSSP1_LFSR*)(&(GM_RSSP1_VSN_Info.channel2)), &temp);
    GM_RSSP1_VSN_Info.VSN2 = temp;

    GM_RSSP1_VSN_Info.VSN2 ^= (GM_RSSP1_UINT32)GM_RSSP1_VSN_M02;

    return;
}

void GM_RSSP1_VSN_Get(GM_RSSP1_UINT32* pVSN0, GM_RSSP1_UINT32* pVSN1, GM_RSSP1_UINT32* pVSN2)
{
    if ((NULL == pVSN0) || (NULL == pVSN1) || (NULL == pVSN2))
    {
        return;
    }

    *pVSN0 = GM_RSSP1_VSN_Info.VSN0;

    *pVSN1 = GM_RSSP1_VSN_Info.VSN1;
    *pVSN2 = GM_RSSP1_VSN_Info.VSN2;

    return;
}

void GM_RSSP1_VSN_Set(GM_RSSP1_UINT32 VSN0, GM_RSSP1_UINT32 VSN1, GM_RSSP1_UINT32 VSN2)
{
    GM_RSSP1_VSN_Info.VSN0 = VSN0;
    GM_RSSP1_VSN_Info.VSN1 = VSN1;
    GM_RSSP1_VSN_Info.VSN2 = VSN2;

    return;
}
/*end:增加设置多套VSN的接口, CR: 2732, GM163, Xavier, 2015-8-3*/

/** @brief 测试
上模块:
1 0x8bceedbe 0xcfafd011
2 0xed6b0631 0xe1f49564

下模块:
1 0x1e9facab 0xd8bcd541
2 0x783a4724 0xf6e79034


void main()
{
    GM_RSSP1_UINT32 CHKW;
    GM_RSSP1_BOOL rt;
    GM_RSSP1_VSN_Init();

    rt = GM_RSSP1_VSN_Cal_Check_Word_VSN1(0x8bceedbe, 0x1e9facab, &CHKW);

    rt = GM_RSSP1_VSN_Cal_Check_Word_VSN2(0xcfafd011, 0xd8bcd541, &CHKW);

    rt = GM_RSSP1_VSN_Cal_Check_Word_VSN1(0xed6b0631, 0x783a4724, &CHKW);

    rt = GM_RSSP1_VSN_Cal_Check_Word_VSN2(0xe1f49564, 0xf6e79034, &CHKW);

    return;
}
*/

