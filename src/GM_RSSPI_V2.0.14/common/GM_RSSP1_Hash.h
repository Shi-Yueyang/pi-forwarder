#ifndef GM_RSSP1_HASH_H
#define GM_RSSP1_HASH_H

#include "GM_RSSP1_Mutex.h"
#include "GM_RSSP1_LFSR.h"
#include "GM_RSSP1_CRC.h"
#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum
	{
		GM_RSSP1_EXIST  = 0x11,  /**< 数据已存在 */
		GM_RSSP1_DELETE = 0x33,  /**< 数据已删除 */
		GM_RSSP1_EMPTY	= 0x77,  /**< 数据不存在 */
	}GM_RSSP1_Status_Enum;

	typedef enum
	{
		OffLine_Object = 0x22,  /**< 离线静态 二维指针队列*/
		OnLine_Object = 0x44,  /**<  在线动态 二维指针队列 */
		Unkonwn_Object = 0x88,
	}GM_RSSP1_OperObject_Enum;

	typedef struct
	{
		GM_RSSP1_UINT32                          *pLFSR_left_table;
		GM_RSSP1_UINT32                          *pLFSR_right_table;
		GM_RSSP1_UINT32                          *pCRC32_table;
		GM_RSSP1_UINT16                          *pCRC16_table;      /**< 系统中只有一个CRC16表。两个chn相同 */
	}GM_RSSP1_PACKED GM_RSSP1_Polyomial_struct;

	extern GM_RSSP1_Polyomial_struct g_Polyomia[2];

    typedef struct
    {
        GM_RSSP1_UINT16 Cnf_index; /*key1*/
        GM_RSSP1_UINT16 DstAddr;/*key2*/
		GM_RSSP1_BOOL Online_Status;/*key3*/
		GM_RSSP1_Status_Enum DataType;
    } GM_RSSP1_PACKED Hash_SFM_Elem_Struct;
    
    typedef struct
    {
        GM_RSSP1_UINT32 hashSize;
        GM_RSSP1_UINT32 sizeAvail;      /* 表中尚未填装元素的大小，即可用大小 */
        Hash_SFM_Elem_Struct *SFMelem;
    } GM_RSSP1_PACKED Hash_Table_SFM_Struct;
    
	typedef struct
	{
		GM_RSSP1_UINT16 online_index;
		GM_RSSP1_UINT32 cnf_index;      /* 表中尚未填装元素的大小，即可用大小 */
		GM_RSSP1_Status_Enum ActiveType;
	} GM_RSSP1_PACKED Hash_CFM_ActiveElem_Structure;

	typedef struct
	{
		GM_RSSP1_UINT32 hashSize;
		GM_RSSP1_UINT32 sizeAvail;
		GM_RSSP1_UINT32 FreeSize;      /* 表中尚未填装元素的大小，即可用大小 */
		GM_RSSP1_MUTEX	Active_Mutex;
		Hash_CFM_ActiveElem_Structure* elem;
	} GM_RSSP1_PACKED Hash_CFM_Active_Structure;

	typedef struct
	{
		GM_RSSP1_UINT8 sub_index;
		GM_RSSP1_UINT16 Cnf_index; /*key1*/
		GM_RSSP1_UINT32 port;/*key3*/
		GM_RSSP1_UINT32 IPAddr;/*key2*/
		GM_RSSP1_Status_Enum DataType;
	} GM_RSSP1_PACKED Hash_CFM_Elem_Struct;

	typedef struct
	{
		Hash_CFM_Elem_Struct *CFMelem;
		GM_RSSP1_UINT32 hashSize;
		GM_RSSP1_UINT32 sizeAvail;      /* 表中尚未填装元素的大小，即可用大小 */
	} GM_RSSP1_PACKED Hash_Table_CFM_Struct;

	typedef struct
	{
		GM_RSSP1_UINT16 preNode;
		GM_RSSP1_UINT16 Index;
		GM_RSSP1_UINT16 NexNode;
	} GM_RSSP1_PACKED GM_RSSP1_Point_Elem_Struct;

	typedef struct
	{
		GM_RSSP1_UINT16 MaxNum;
		GM_RSSP1_UINT16 SeqNum;
		GM_RSSP1_UINT16 Front_Seq;
		GM_RSSP1_UINT16 Back_Seq;
		GM_RSSP1_UINT16 startNode;
		GM_RSSP1_UINT16 EndNode;
		GM_RSSP1_Point_Elem_Struct *node;
	} GM_RSSP1_PACKED GM_RSSP1_Sequen_Structure;

	typedef struct
	{
		GM_RSSP1_UINT16 RecordNum;
		GM_RSSP1_UINT16 RecordIndex[GM_RSSP1_MAX_ONLINE_CONNECTION_NUM];
	} GM_RSSP1_PACKED GM_RSSP1_SeqDele_Record_Structure;
    
	/**
    * @brief GM_RSSP1_Next_Prime
    *
    * 找到比limit的最小素数
    * @param[in] limit
    * @return GM_RSSP1_UINT32  最小素数
    */
    GM_RSSP1_UINT32 GM_RSSP1_Next_Prime(GM_RSSP1_UINT32 limit);

    /**
    * @brief 初始化SFM层的Hash表
    *
    * 初始化哈希表
    * @return GM_RSSP1_BOOL 初始化结果
    */
    GM_RSSP1_BOOL GM_RSSP1_Hash_SFM_Init(void);

	/**
    * @brief SFM层的Hash表插入
    *
    * 哈希表根据KEY查找，并匹配到VAL，如没有，则将该值填入
    * @return GM_RSSP1_BOOL 插入结果
    */
	GM_RSSP1_UINT16 GM_RSSP1_Hash_Fill_SFM_Index(GM_RSSP1_UINT32 key, GM_RSSP1_UINT16 val);
	
	/**
    * @brief Hash表根据目的地址进行Hash查找
    *
    * 哈希表根据KEY查找
    * @return GM_RSSP1_BOOL 查找结果
    */
	GM_RSSP1_BOOL GM_RSSP1_Hash_Search_CnfDst(GM_RSSP1_UINT32 key, GM_RSSP1_UINT16* pCurIndex, GM_RSSP1_UINT16* pHashIndex);

	/**
    * @brief Hash表根据目的地址和索引号进行Hash查找
    *
    * 哈希表根据KEY，VAL查找
    * @return GM_RSSP1_BOOL 查找结果
    */
	GM_RSSP1_BOOL GM_RSSP1_Hash_Search_DstAndIndex(GM_RSSP1_UINT32 key, GM_RSSP1_UINT16 val, GM_RSSP1_UINT16* pCurindex, GM_RSSP1_UINT16* pFreeIndex);
	
	/**
    * @brief 根据HashIndex，直接插入新hash节点
    *
    * @return GM_RSSP1_BOOL 插入结果
    */
	GM_RSSP1_BOOL GM_RSSP1_Hash_Insert_Online(GM_RSSP1_UINT16 HashIndex, GM_RSSP1_BOOL Res);

	/**
    * @brief SFM层的Hash表插入
    *
    * 哈希表根据KEY查找，并匹配到VAL，如没有，则将该值填入
    * @return GM_RSSP1_BOOL 插入结果
    */
	GM_RSSP1_BOOL GM_RSSP1_Hash_Fill_CFM_Index(GM_RSSP1_UINT32 key, GM_RSSP1_UINT32 val, GM_RSSP1_UINT16 index, GM_RSSP1_UINT8 subindex);

	/**
    * @brief Hash表根据IP和Port进行Hash查找
    *
    * 哈希表根据KEY，VAL匹配查找
    * @return GM_RSSP1_BOOL 查找结果
    */
	GM_RSSP1_BOOL GM_RSSP1_Hash_Search_IP(GM_RSSP1_UINT32 key, GM_RSSP1_UINT32 val, GM_RSSP1_UINT16* pCurindex, GM_RSSP1_UINT16* pFreeIndex);
	
	/**
    * @brief 初始化CFM层的Hash表
    *
    * 初始化哈希表
    * @return GM_RSSP1_BOOL 初始化结果
    */
	GM_RSSP1_BOOL GM_RSSP1_Hash_CFM_Init(void);

	/**
    * @brief Hash表根据key和val进行Hash查找
    *
    * 哈希表根据KEY，VAL匹配查找
    * @return GM_RSSP1_BOOL 查找结果
    */
	GM_RSSP1_BOOL GM_RSSP1_Hash_GetCFMIndex(GM_RSSP1_UINT32 key, GM_RSSP1_UINT32 val, GM_RSSP1_UINT16 *pIndex, GM_RSSP1_UINT8 *pSubIndex);

	/**
    * @brief Hash表根据key和val进行Hash查找
    *
    * 哈希表根据KEY，VAL匹配查找
    * @return GM_RSSP1_BOOL 查找结果
    */
	GM_RSSP1_BOOL GM_RSSP1_Hash_SFM_Dele(GM_RSSP1_UINT32 key, GM_RSSP1_UINT8 *counter, GM_RSSP1_UINT16 *cnfIndex);

	GM_RSSP1_BOOL GM_RSSP1_Hash_CFM_Dele(GM_RSSP1_UINT32 key, GM_RSSP1_UINT32 val, GM_RSSP1_UINT16 *cnfIndex);

	GM_RSSP1_BOOL GM_RSSP1_Hash_Search_Active(GM_RSSP1_UINT32 key, GM_RSSP1_UINT16* pCurindex, GM_RSSP1_UINT16* pFreeIndex);

	void GM_RSSP1_Reset_Active(void);

	void GM_RSSP1_Clear_Active(void);
	
	GM_RSSP1_UINT16 GM_RSSP1_Update_Active(GM_RSSP1_UINT16 Index, GM_RSSP1_UINT8 SetVal);

	void GM_RSSP1_InitSeqArray(GM_RSSP1_OperObject_Enum ArrtType, GM_RSSP1_UINT16 MaxAmout);

	GM_RSSP1_BOOL GM_RSSP1_Hash_CFM_ActInit(GM_RSSP1_UINT32 MaxArryVal);

	void GM_RSSP1_SeqArrayClean(GM_RSSP1_OperObject_Enum ArrtType);

	GM_RSSP1_UINT16 GM_RSSP1_InitSeqInsert(GM_RSSP1_OperObject_Enum ArrtType);

	void GM_RSSP1_InitSeqDelete(GM_RSSP1_UINT16 index);

	GM_RSSP1_UINT16 GM_RSSP1_GetSeqAmount(GM_RSSP1_OperObject_Enum ArrtType);

	GM_RSSP1_UINT16 GM_RSSP1_GetSeqIndexPositive(GM_RSSP1_OperObject_Enum ArrtType, GM_RSSP1_UINT8 isReset);

	GM_RSSP1_UINT16 GM_RSSP1_GetSeqIndexReverse(GM_RSSP1_OperObject_Enum ArrtType, GM_RSSP1_UINT8 isReset);

	/**
    * @brief 链表插入，包括在线和离线
    *
    * @ArrtType：链表类型
    * @index：结点的cfg index
    */
	void GM_RSSP1_SeqInsert(GM_RSSP1_OperObject_Enum ArrtType, GM_RSSP1_UINT16 index);

	/**
    * @brief 链表删除，包括在线和离线
    *
    * @ArrtType：链表类型
    * @index：结点的cfg index
    */
	void GM_RSSP1_SeqDelete(GM_RSSP1_OperObject_Enum ArrtType, GM_RSSP1_UINT16 index);

	GM_RSSP1_UINT32 GM_RSSP1_Get_Active_Num(void);

	/**
    * @brief 记录删除的在线结点的cfg index
    *
    * @index：结点的cfg index
    */
	void GM_RSSP1_SeqDeleteRecord(GM_RSSP1_UINT16 index);

	GM_RSSP1_SeqDele_Record_Structure* GM_RSSP1_SeqDeleteRecord_Get(void);

	GM_RSSP1_BOOL GM_RSSP1_Hash_CnfDst_ResArray(GM_RSSP1_UINT32 key, GM_RSSP1_UINT16* ConArray, GM_RSSP1_UINT8* ArraySize);

	GM_RSSP1_UINT16 GM_RSSP1_Active_Add(GM_RSSP1_UINT16 Index, GM_RSSP1_UINT8 SetVal);

	GM_RSSP1_BOOL GM_RSSP1_Active_AddByArry(GM_RSSP1_UINT16* IndexArray);
#ifdef __cplusplus
}

#endif


#endif

