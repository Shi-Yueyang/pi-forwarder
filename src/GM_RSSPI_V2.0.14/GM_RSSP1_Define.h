/**
* @file GM_RSSP1_Define.h
* @brief ����RSSP1ģ���õ��ĺ�
* @author Hao Liming
* @date 2010-11-2 15:56:31
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>haoliming@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/
#ifndef _GM_RSSP1_H
#define _GM_RSSP1_H

/* ***********************************  用户定义区1，选择操作系统  8选1 ******************************************* */
//#define OS_64 // 64位系统宏，可与任意操作系统搭配使用

 /* ************系统1：若为windows则定义************ */
#define WIN32
#define GM_RSSP1_MAX_TX_MSG_CYCLE (100U)

/* *************系统2：EVC/ 无操作系统 ************* */
/* 无操作系统宏需要用户定义如下内容 */

#define EVC //此为测试宏，后续删除（UDP等需要用到）

/* **** 1. 头文件 **** */
//common里的头文件移到此处，用户根据需要自行增删
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <errno.h>

/* **** 2.内存操作 有两种方案 **** */
/* PLANA: 定义宏使用P1提供的简单内存malloc操作函数，为静态操作，会影响效率，用户需结合自身情况考虑是否使用（具体使用看UM） */
//#define GM_RSSP1_MEMORY_WITH_STATIC


/* PLANB: 用户重定义以下内存操作函数 */
//#include "TB_test.h" //此为测试宏，后续删除（内部包含重定义的P1函数）
//
//#define  GM_RSSP1_malloc	TB_RSSP1_malloc
//#define	 GM_RSSP1_free		TB_RSSP1_free
//#define	 GM_RSSP1_memcpy	TB_RSSP1_memcpy
//#define	 GM_RSSP1_memset	TB_RSSP1_memset

/* **** 3. 加锁操作 **** */
/* 若用户不使用锁，则通过此宏屏蔽锁，但用户需保证为单线程操作，否则线程冲突由用户自行解决 */
//#define GM_RSSP1_DISABLE_LOCK

/* **** 4. 字节对齐操作，用户需结合自身情况定义字节对齐 **** */
//#define PRAGMA_PACK_USER 
//#pragma pack(1)
//#define GM_RSSP1_PACKED

/* **** 5. 需开此宏，屏蔽系统库 **** */
//#define NO_SYS
//#define NO_SYS_MEMORY
//#define GM_RSSP1_Log_Msg TB_GM_RSSP1_Log_Msg

/* 6. 若不支持文件系统加载，则注释掉下列宏 */
//#define FILE_SYSTEM



/* ************* EVC TEST ************* */

//#ifndef WIN32
//#define WIN32
//#define OS_64
//#endif


/* ��ΪVXWORKS5_5������:
#ifndef VXWORKS5_5
#define VXWORKS5_5
#endif*/

/* ��ΪVXWORKS6_6������:
#ifndef VXWORKS6_6
#define VXWORKS6_6
#endif*/

/* ��ΪQNX������:
#ifndef QNX
#define QNX
#endif*/

/*CR5143*/
/* ��ΪLINUX������:
#ifndef LINUX
#define LINUX
#endif
*/

/* ��ΪREWORKS������:
#ifndef REWORKS
#define REWORKS
#endif
*/


/* 200T/300C�������汾 */
//#ifndef REWORKS_CERTV2
//#define OS_64
//#define REWORKS_CERTV2
//#endif

/**ϵͳ��ؽӿڶ���**/
//#ifdef REWORKS_CERTV2
//#define GM_RSSP1_malloc  TB_malloc
//#define	GM_RSSP1_memcpy	TB_memcpy
//#define	GM_RSSP1_memset	TB_memset
//#define	GM_RSSP1_free
//#endif


#ifdef REWORKS_CERTV2
typedef unsigned long size_t;
#else
#define FILE_SYSTEM
#endif


/************  �û�������2�� ����Ҫ��ӡ��log��Ϣ�ȼ� *********/
/*ԭ RSSP1_LOG_LEVEL ��ȫ�ֱ������� �ı�Ϊͨ��API�ӿڱ�����������ͷֱ��Ϊ1��2��4�������ͣ����໥���*/



/************  �û�������3������SFM, CFM��ʹ�����**************/
#ifndef SFM_Part
#define SFM_Part
#endif

#ifndef CFM_Part
#define CFM_Part
#endif


//#ifdef CFM_Part
//#define LINK_Part
//#endif


/************************************************************************/
/*                    软件性能自定义区                                  */
/************************************************************************/

#define GM_RSSP1_MAX_USER_DAT_LEN	(1378U)		/**根据用户容量设计,配置发送数据长度,本定义的单位长度为“字节”CR:3238 **/
#define GM_RSSP1_MAX_SAFETY_CONNECTION_NUM (1000U)		/**根据用户容量设计,软件需要包含的最大配置离线通信对象数 **/
#define GM_RSSP1_MAX_ONLINE_CONNECTION_NUM (300U)		/**根据用户容量设计,软件需要支持的最大在线通信对象数 **/
/*#define GM_RSSP1_ONLINE_SYN_ONLY*/			/**根据用户容量设计,当离线对象数过多导致数据量过大无法同步时,使用该模式数据将只同步当前周期的活动以及脱离活动的节点,最大同步数据节点为GM_RSSP1_MAX_ONLINE_CONNECTION_NUM*2. 如果系统设计存在软件运行过程中存在离线配置的操作,则必须使用该定义**/

/************************************************************************/
/*                    软件数据接口自定义区                              */
/************************************************************************/

/*#define InterfaceData_With_CRC			*该宏定义表示用户在与软件的数据接口中,无论发送和接口,数据末尾需增加8字节（2个CRC32）**/
/*#define GM_RSSP1_RxMsgWithTime			*本地提供数据中前8个字节包含了对应2个时间信息：一、网络传输基准值（该值为发送SSE收到SSR的时间差）;二、本包数据基于传输基准值的偏差（收发两端的时间差）**/
/*#define GM_RSSP1_SYN_NO_DATA				*该宏定义情况下,主备同步接口Get和Set所使用的数据中不包含安全数据 **/

/************************************************************************/
/*                    软件功能自定义区                                  */
/************************************************************************/

/*#define OFFLINE_TOOL_SUPPORT				*该宏为满足特殊需求,将需配合RSSP-I离线工具产生的bin文件使用,主要作用将接收的CRCM中的Remote TS替换成Local TS的CRCM,并提供给用户 **/

#ifdef CVC_CONF_CPU_A
#define GM_RSSP1_SYSCKW_FROM_USER
#endif

/*#define GM_RSSP1_ENABLE_CRSCD_PACK		*该宏定义情况下，CFM发送的数据包会根据标准要求,将RSD消息放置最前面，将所有数据组包后发送 **/
/*#define GM_RSSP1_MAX_PKT_NUM_PER_CYCLE 5U  *该宏因在GM_RSSP1_ENABLE_CRSCD_PACK使用才有效,每周期可能产生的每种数据包最大数量.包括RSD、SSE、SSR **/

//#define GM_RSSP1_MEMORY_WITH_STATIC		/**该宏定义表示软件申请使用的堆内存为用户指定内存范围,否则使用系统malloc随机分配 **/
//#define RSSP1_BASE_BUF_SIZE (1024*1024*100U)
#define CSSC_TEST
/*#define CFM_Stanby_Answer					*该宏定义情况下,在备系安全层不输出的系统架构设计下.CFM层会主动回馈全0数据包给远端 **/		

#define GM_RSSP1_SNDPROC_ON_USRDATA_MEM      /* 数据处理在用户数据内存里,所传应用数据地址之前需预留*位用于发送的封包 */

//#define GM_RSSP1_SAVING_MODE

/************************************************************************/
/*                    �����ص������Զ�����                                  */
/************************************************************************/

#define GM_RSSP1_UpdateTC_Assign(x)	BUpdate_VSN_CallBack(x)		/**该宏函数定义情况下，允许对指定节点配置不同的通信周期，并以此提供实时待更新的VSN,*/

/**#define RSSP1_TolerateWithCycle	��ǰ���ƿ�ˡ������ӿڵ��÷�ʽ���ڹ�����Ч�ڵ������ͬʱ���ƻ�ȡ��������һ����ֻ�ܻ�ȡһ�� add by flt 20210426**/

#define GM_RSSP1_Get_CRCM(x)	/**�ú꺯������GM_RSSP1_ENABLE_CH1�����������Ч����ȡУ��ֵ*/
#define GM_RSSP1_Support_CRCM(x)	/**�ú꺯������GM_RSSP1_ENABLE_CH2�����������Ч���ṩУ��ֵ*/
#define GM_RSSP1_MAX_DELTATIME_ARRAY_SIZE       100U	/* CR3765 */
#define GM_RSSP1_MAX_DELAYTIME					100U	/* CR3765 */
#define GM_RSSP1_MAX_TOLERATE_CYCLE             100U	/* CR3765 */

/************  �û������� ���� *********/
#ifndef SFM_Part
#undef OFFLINE_TOOL_SUPPORT
#endif

#ifdef OFFLINE_TOOL_SUPPORT
#define InterfaceData_With_CRC
#define GM_RSSP1_SYSCKW_FROM_USER
#endif

#ifdef InterfaceData_With_CRC
#define GM_RSSP1_MAX_SFM_DAT_LEN GM_RSSP1_MAX_USER_DAT_LEN+8U
#else
#define GM_RSSP1_MAX_SFM_DAT_LEN GM_RSSP1_MAX_USER_DAT_LEN
#endif



#if !defined(SFM_Part) || !defined(CFM_Part)
#undef GM_RSSP1_SAVING_MODE
#endif


#if !defined(GM_RSSP1_ENABLE_CH1) && !defined(GM_RSSP1_ENABLE_CH2)
#define GM_RSSP1_ENABLE_CH1
#define GM_RSSP1_ENABLE_CH2
#endif


/*�������ϵͳ�ظ�����*/
/*��ΪVXWORKSʱ����VXWORKS*/
#ifdef VXWORKS5_5
#ifndef VXWORKS
#define VXWORKS
#undef REWORKS_CERTV2
#endif

#undef VXWORKS6_6
#undef WIN32
#undef QNX
#undef REWORKS
#undef REWORKS_CERTV2
#endif

#ifdef VXWORKS6_6
#ifndef VXWORKS
#define VXWORKS
#undef REWORKS_CERTV2
#endif

#undef VXWORKS5_5
#undef WIN32
#undef QNX
#undef REWORKS
#undef REWORKS_CERTV2
#endif


#ifdef EVC
#undef VXWORKS
#undef VXWORKS5_5
#undef VXWORKS6_6
#undef QNX
#undef REWORKS
#undef REWORKS_CERTV2
#endif

#ifdef QNX
#undef VXWORKS
#undef VXWORKS5_5
#undef VXWORKS6_6
#undef WIN32
#undef REWORKS
#undef REWORKS_CERTV2
#endif

#ifdef LINUX
#undef QNX
#undef VXWORKS
#undef VXWORKS5_5
#undef VXWORKS6_6
#undef WIN32
#undef REWORKS
#undef REWORKS_CERTV2
#endif

#ifdef REWORKS
#undef QNX
#undef VXWORKS
#undef VXWORKS5_5
#undef VXWORKS6_6
#undef WIN32
#undef LINUX
#undef REWORKS_CERTV2
#endif



#define GM_RSSP1_TABLE_Ver "Bcode_RSSP-I_V2.0.14_Build_20260302"

#endif

