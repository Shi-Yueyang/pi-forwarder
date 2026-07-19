#ifndef _GM_RSSP1_Error_H
#define _GM_RSSP1_Error_H


#define GM_RSSP1_Error 0xE1U
#define GM_RSSP1_Init_Error (((GM_RSSP1_UINT32)GM_RSSP1_Error<<8U)+0x01U)				/*0xE101*/
#define GM_RSSP1_Proc_Tx_Error (((GM_RSSP1_UINT32)GM_RSSP1_Error<<8U)+0x91U)			/*0xE191*/
#define GM_RSSP1_Proc_Rx_Error (((GM_RSSP1_UINT32)GM_RSSP1_Error<<8U)+0xF1U)			/*0xE1F1*/
#define GM_RSSP1_Common_Part_Error (((GM_RSSP1_UINT32)GM_RSSP1_Error<<8U)+0x51U)	/*0xE151*/
#define GM_RSSP1_File_Error (((GM_RSSP1_UINT32)GM_RSSP1_Error<<8U)+0x11U)				/*0xE111*/
#define GM_RSSP1_VSN_Error (((GM_RSSP1_UINT32)GM_RSSP1_Error<<8U)+0x21U)				/*0xE121*/
#define GM_RSSP1_Other_Error (((GM_RSSP1_UINT32)GM_RSSP1_Error<<8U)+0x31U)			/*0xE131*/

/*错误类型*/
#define Param_Error (GM_RSSP1_UINT32)0x10U
#define Param_Point_Error (GM_RSSP1_UINT32)0x11U
#define Param_Val_Error (GM_RSSP1_UINT32)0x12U
#define Result_No_Match (GM_RSSP1_UINT32)0x20U
#define Memory_Error (GM_RSSP1_UINT32)0x50U


/*GM_RSSP1_APP_Interface_Init_Addr  1*/
#define GM_RSSP1_APP_Interface_Init_Addr_Num ((GM_RSSP1_Init_Error<<8U)+0x01U)   /*0xE10101*/
#define GM_RSSP1_APP_Interface_Init_Addr_SFM_FAIL (GM_RSSP1_APP_Interface_Init_Addr_Num<<8U)+Result_No_Match /*0xE1010120*/
#define GM_RSSP1_APP_Interface_Init_Addr_CFM_FAIL GM_RSSP1_APP_Interface_Init_Addr_SFM_FAIL+1U	/*0xE1010121*/

/*GM_RSSP1_APP_Interface_Init  2*/
#define GM_RSSP1_APP_Interface_Init_Num ((GM_RSSP1_Init_Error<<8U)+0x02U)   /*0xE10102*/
#define GM_RSSP1_APP_Interface_Init_Param_Point_Error (GM_RSSP1_APP_Interface_Init_Num<<8U)+Param_Point_Error /*0xE1010211*/
#define GM_RSSP1_APP_Interface_Init_FILE_Size_Error GM_RSSP1_APP_Interface_Init_Param_Point_Error+1U	/*0xE1010212*/
#define GM_RSSP1_APP_Interface_Init_FILE_Load_Error GM_RSSP1_APP_Interface_Init_FILE_Size_Error+1U	/*0xE1010213*/
#define GM_RSSP1_APP_Interface_Init_SFM_Init_Error GM_RSSP1_APP_Interface_Init_FILE_Load_Error+1U	/*0xE1010214*/
#define GM_RSSP1_APP_Interface_Init_CFM_Init_Error GM_RSSP1_APP_Interface_Init_SFM_Init_Error+1U	/*0xE1010215*/
#define GM_RSSP1_APP_Interface_Init_Memory_Error GM_RSSP1_APP_Interface_Init_Num+Memory_Error	/*0xE1010250*/

/*GM_RSSP1_Send_App_Dat  3*/
#define GM_RSSP1_Send_App_Dat_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x03U)   /*0xE19103*/
#define GM_RSSP1_Send_App_Dat_Num_Param_Error (GM_RSSP1_Send_App_Dat_Num<<8U)+Param_Error /*0xE1910310*/
#define GM_RSSP1_Send_App_Dat_Num_Result_No_Match (GM_RSSP1_Send_App_Dat_Num<<8U)+Result_No_Match /*0xE1910320*/

/*GM_RSSP1_APP_Clear_Msg_Queue  4*/
#define GM_RSSP1_APP_Clear_Msg_Queue_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x04U)   /*0xE15104*/
#define GM_RSSP1_APP_Clear_Msg_Queue_Result_No_Match (GM_RSSP1_APP_Clear_Msg_Queue_Num<<8U)+Result_No_Match /*0xE1510420*/

/*GM_RSSP1_APP_Interface_Rcv_App_Dat  5*/
#define GM_RSSP1_APP_Interface_Rcv_App_Dat_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x05U)   /*0xE1F105*/
#define GM_RSSP1_APP_Interface_Rcv_App_Dat_Param_Point_Error (GM_RSSP1_APP_Interface_Rcv_App_Dat_Num<<8U)+Param_Point_Error /*0xE1F10511*/
#define GM_RSSP1_APP_Interface_Rcv_App_Dat_MSG_Size_Error (GM_RSSP1_APP_Interface_Rcv_App_Dat_Num<<8U)+Result_No_Match /*0xE1F10520*/
#define GM_RSSP1_APP_Interface_Rcv_App_Dat_Standby_Log GM_RSSP1_APP_Interface_Rcv_App_Dat_MSG_Size_Error+1U /*0xE1F10521*/

/*GM_RSSP1_APP_Interface_Send_App_Dat  6*/
#define GM_RSSP1_APP_Interface_Send_App_Dat_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x06U)   /*0xE19106*/
#define GM_RSSP1_APP_Interface_Send_App_Dat_Param_Val_Error (GM_RSSP1_APP_Interface_Send_App_Dat_Num<<8U)+Param_Val_Error /*0xE1910612*/

/*GM_RSSP1_APP_Interface_CalcCNF_Init  7*/
#define GM_RSSP1_APP_Interface_CalcCNF_Init_Num ((GM_RSSP1_Init_Error<<8U)+0x07U)   /*0xE10107*/
#define GM_RSSP1_APP_Interface_CalcCNF_Init_Param_Point_Error (GM_RSSP1_APP_Interface_CalcCNF_Init_Num<<8U)+Param_Point_Error /*0xE1010711*/
#define GM_RSSP1_APP_Interface_CalcCNF_Init_FILE_Size_Error GM_RSSP1_APP_Interface_CalcCNF_Init_Param_Point_Error+1U /*0xE1010712*/
#define GM_RSSP1_APP_Interface_CalcCNF_Init_FILE_Load_Error GM_RSSP1_APP_Interface_CalcCNF_Init_FILE_Size_Error+1U /*0xE1010713*/
#define GM_RSSP1_APP_Interface_CalcCNF_Init_Memory_Error (GM_RSSP1_APP_Interface_CalcCNF_Init_Num<<8U)+Memory_Error /*0xE1010750*/

/*GM_RSSP1_CFM_Get_Library_Status  8*/
#define GM_RSSP1_CFM_Get_Library_Status_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x08U)   /*0xE15108*/
#define GM_RSSP1_CFM_Get_Library_Status_Param_Point_Error (GM_RSSP1_CFM_Get_Library_Status_Num<<8U)+Param_Point_Error /*0xE1510811*/

/*GM_RSSP1_CFM_Is_SaCEPID_Equal  9*/
#define GM_RSSP1_CFM_Is_SaCEPID_Equal_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x09U)   /*0xE15109*/
#define GM_RSSP1_CFM_Is_SaCEPID_Equal_Param_Point_Error (GM_RSSP1_CFM_Is_SaCEPID_Equal_Num<<8U)+Param_Point_Error /*0xE1510911*/

/*GM_RSSP1_CFM_Get_Connection_BY_SaCEPID  10*/
#define GM_RSSP1_CFM_Get_Connection_BY_SaCEPID_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x0AU)   /*0xE1510A*/
#define GM_RSSP1_CFM_Get_Connection_BY_SaCEPID_Param_Point_Error (GM_RSSP1_CFM_Get_Connection_BY_SaCEPID_Num<<8U)+Param_Point_Error /*0xE1510A11*/
#define GM_RSSP1_CFM_Get_Connection_BY_SaCEPID_State_Fail (GM_RSSP1_CFM_Get_Connection_BY_SaCEPID_Num<<8U)+Result_No_Match /*0xE1510A20*/
#define GM_RSSP1_CFM_Get_Connection_BY_SaCEPID_Search_Fail GM_RSSP1_CFM_Get_Connection_BY_SaCEPID_State_Fail+1U /*0xE1510A21*/

/*GM_RSSP1_CFM_Proc_SFM_Req  11*/
#define GM_RSSP1_CFM_Proc_SFM_Req_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x0BU)   /*0xE1910B*/
#define GM_RSSP1_CFM_Proc_SFM_Req_Param_Point_Error (GM_RSSP1_CFM_Proc_SFM_Req_Num<<8U)+Param_Point_Error /*0xE1910B11*/
#define GM_RSSP1_CFM_Proc_SFM_Req_State_Fail (GM_RSSP1_CFM_Proc_SFM_Req_Num<<8U)+Result_No_Match /*0xE1910B20*/

/*GM_RSSP1_CFM_Send_Dat  12*/
#define GM_RSSP1_CFM_Send_Dat_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x0CU)   /*0xE1910C*/
#define GM_RSSP1_CFM_Send_Dat_Param_Val_Error (GM_RSSP1_CFM_Send_Dat_Num<<8U)+Param_Val_Error /*0xE1910C12*/
#define GM_RSSP1_CFM_Send_Dat_State_Fail (GM_RSSP1_CFM_Send_Dat_Num<<8U)+Result_No_Match /*0xE1910C20*/
#define GM_RSSP1_CFM_Send_Dat_Write_Fail GM_RSSP1_CFM_Send_Dat_State_Fail+1U /*0xE1910C21*/

/*GM_RSSP1_CFM_User_Send_Dat  13*/
#define GM_RSSP1_CFM_User_Send_Dat_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x0DU)   /*0xE1910D*/
#define GM_RSSP1_CFM_User_Send_Dat_Param_Val_Error (GM_RSSP1_CFM_User_Send_Dat_Num<<8U)+Param_Val_Error /*0xE1910D12*/
#define GM_RSSP1_CFM_User_Send_Dat_State_Fail (GM_RSSP1_CFM_User_Send_Dat_Num<<8U)+Result_No_Match /*0xE1910C20*/
#define GM_RSSP1_CFM_User_Send_Dat_Search_Fail GM_RSSP1_CFM_User_Send_Dat_State_Fail+1U /*0xE1910C21*/
#define GM_RSSP1_CFM_User_Send_Dat_Write_Fail GM_RSSP1_CFM_User_Send_Dat_Search_Fail+1U /*0xE1910C22*/

/*GM_RSSP1_CFM_Proc_Con_Rcvd_Dat  14*/
#define GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x0EU)   /*0xE1F10E*/
#define GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Param_Val_Error (GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Num<<8U)+Param_Val_Error /*0xE1F10E12*/
#define GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Overall_Error (GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Num<<8U)+Result_No_Match /*0xE1F10E20*/
#define GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Fail GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Overall_Error+1U /*0xE1F10E21*/
#define GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Full GM_RSSP1_CFM_Proc_Con_Rcvd_Dat_Write_Fail+1U /*0xE1F10E22*/

/*GM_RSSP1_CFM_Proc_Recvd_Dat  15*/
#define GM_RSSP1_CFM_Proc_Recvd_Dat_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x0FU)   /*0xE1F10F*/
#define GM_RSSP1_CFM_Proc_Recvd_Dat_Param_Point_Error (GM_RSSP1_CFM_Proc_Recvd_Dat_Num<<8U)+Param_Point_Error /*0xE1F10F11*/
#define GM_RSSP1_CFM_Proc_Recvd_Dat_State_Fail (GM_RSSP1_CFM_Proc_Recvd_Dat_Num<<8U)+Result_No_Match /*0xE1F10F20*/

/*GM_RSSP1_CFM_Report_com_State_To_SFM  16*/
#define GM_RSSP1_CFM_Report_com_State_To_SFM_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x10U)   /*0xE1F110*/
#define GM_RSSP1_CFM_Report_com_State_To_SFM_Param_Point_Error (GM_RSSP1_CFM_Report_com_State_To_SFM_Num<<8U)+Param_Point_Error /*0xE1F11011*/
#define GM_RSSP1_CFM_Report_com_State_To_SFM_State_Fail (GM_RSSP1_CFM_Report_com_State_To_SFM_Num<<8U)+Result_No_Match /*0xE1F11020*/
#define GM_RSSP1_CFM_Report_com_State_To_SFM_Write_Fail GM_RSSP1_CFM_Report_com_State_To_SFM_State_Fail+1U /*0xE1F11021*/

/*GM_RSSP1_Init_Comm_Global_CFM  17*/
#define GM_RSSP1_Init_Comm_Global_CFM_Num ((GM_RSSP1_Init_Error<<8U)+0x11U)   /*0xE10111*/
#define GM_RSSP1_Init_Comm_Global_CFM_Param_Point_Error (GM_RSSP1_Init_Comm_Global_CFM_Num<<8U)+Param_Point_Error /*0xE1011111*/
#define GM_RSSP1_Init_Comm_Global_CFM_Connect_Num_Fail (GM_RSSP1_Init_Comm_Global_CFM_Num<<8U)+Result_No_Match /*0xE1011120*/
#define GM_RSSP1_Init_Comm_Global_CFM_Src_Addr_Fail GM_RSSP1_Init_Comm_Global_CFM_Connect_Num_Fail+1U /*0xE1011121*/
#define GM_RSSP1_Init_Comm_Global_CFM_U2L_Q_Fail GM_RSSP1_Init_Comm_Global_CFM_Src_Addr_Fail+1U /*0xE1011122*/
#define GM_RSSP1_Init_Comm_Global_CFM_U2L_Q_Mem_Fail GM_RSSP1_Init_Comm_Global_CFM_U2L_Q_Fail+1U /*0xE1011123*/
#define GM_RSSP1_Init_Comm_Global_CFM_L2U_Q_Fail GM_RSSP1_Init_Comm_Global_CFM_U2L_Q_Mem_Fail+1U /*0xE1011124*/
#define GM_RSSP1_Init_Comm_Global_CFM_L2U_Q_Mem_Fail GM_RSSP1_Init_Comm_Global_CFM_L2U_Q_Fail+1U /*0xE1011125*/
#define GM_RSSP1_Init_Comm_Global_CFM_Max_Num_Fail GM_RSSP1_Init_Comm_Global_CFM_L2U_Q_Mem_Fail+1U /*0xE1011126*/

/*GM_RSSP1_Init_Comm_Connection_CFM  18*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Num ((GM_RSSP1_Init_Error<<8U)+0x12U)   /*0xE10112*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Param_Point_Error (GM_RSSP1_Init_Comm_Connection_CFM_Num<<8U)+Param_Point_Error /*0xE1011211*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Des_Addr_Fail (GM_RSSP1_Init_Comm_Connection_CFM_Num<<8U)+Result_No_Match /*0xE1011220*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Enable_Pack_Fail GM_RSSP1_Init_Comm_Connection_CFM_Des_Addr_Fail+1U /*0xE1011221*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Enable_UDP_Fail GM_RSSP1_Init_Comm_Connection_CFM_Enable_Pack_Fail+1U /*0xE1011222*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Com_Type_Fail GM_RSSP1_Init_Comm_Connection_CFM_Enable_UDP_Fail+1U /*0xE1011223*/
#define GM_RSSP1_Init_Comm_Connection_CFM_UDP_Num_Fail GM_RSSP1_Init_Comm_Connection_CFM_Com_Type_Fail+1U /*0xE1011224*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Confilct_Fail GM_RSSP1_Init_Comm_Connection_CFM_UDP_Num_Fail+1U /*0xE1011225*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Loc_IP_Fail GM_RSSP1_Init_Comm_Connection_CFM_Confilct_Fail+1U /*0xE1011226*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Loc_Port_Fail GM_RSSP1_Init_Comm_Connection_CFM_Loc_IP_Fail+1U /*0xE1011227*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Rem_IP_Fail GM_RSSP1_Init_Comm_Connection_CFM_Loc_Port_Fail+1U /*0xE1011228*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Rem_Port_Fail GM_RSSP1_Init_Comm_Connection_CFM_Rem_IP_Fail+1U /*0xE1011229*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Rcv_Size_Fail GM_RSSP1_Init_Comm_Connection_CFM_Rem_Port_Fail+1U /*0xE101122A*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Rcv_Offset_Fail GM_RSSP1_Init_Comm_Connection_CFM_Rcv_Size_Fail+1U /*0xE101122B*/
#define GM_RSSP1_Init_Comm_Connection_CFM_Snd_Size_Fail GM_RSSP1_Init_Comm_Connection_CFM_Rcv_Offset_Fail+1U /*0xE101122C*/
#define GM_RSSP1_Init_Comm_Global_CFM_Src_Error GM_RSSP1_Init_Comm_Connection_CFM_Snd_Size_Fail+1 /*0xE101122D*/

/*GM_RSSP1_CFM_Init  19*/
#define GM_RSSP1_CFM_Init_Num ((GM_RSSP1_Init_Error<<8U)+0x13U)   /*0xE10113*/
#define GM_RSSP1_CFM_Init_Param_Point_Error (GM_RSSP1_CFM_Init_Num<<8U)+Param_Point_Error /*0xE1011311*/

/*GM_RSSP1_CFM_Init_Lib  20*/
#define GM_RSSP1_CFM_Init_Lib_Num ((GM_RSSP1_Init_Error<<8U)+0x14U)   /*0xE10114*/
#define GM_RSSP1_CFM_Init_Lib_Param_Point_Error (GM_RSSP1_CFM_Init_Lib_Num<<8U)+Param_Point_Error /*0xE1011411*/
#define GM_RSSP1_CFM_Init_Lib_Link_MQ_Fail (GM_RSSP1_CFM_Init_Lib_Num<<8U)+Result_No_Match /*0xE1011420*/
#define GM_RSSP1_CFM_Init_Lib_CFM_MQ_Fail GM_RSSP1_CFM_Init_Lib_Link_MQ_Fail+1U /*0xE1011421*/
#define GM_RSSP1_CFM_Init_Lib_CFM_Snd_Offset_Fail GM_RSSP1_CFM_Init_Lib_CFM_MQ_Fail+1U /*0xE1011422*/

/*GM_RSSP1_CFM_Interface_Proc_Recv  21*/
#define GM_RSSP1_CFM_Interface_Proc_Recv_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x15U)   /*0xE1F115*/
#define GM_RSSP1_CFM_Interface_Proc_Recv_Get_Object_Fail (GM_RSSP1_CFM_Interface_Proc_Recv_Num<<8U)+Result_No_Match /*0xE1F11520*/
#define GM_RSSP1_CFM_Interface_Proc_Recv_State_Fail GM_RSSP1_CFM_Interface_Proc_Recv_Get_Object_Fail+1U /*0xE1F11521*/

/*GM_RSSP1_CFM_Interface_Proc_Send  22*/
#define GM_RSSP1_CFM_Interface_Proc_Send_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x16U)   /*0xE19116*/
#define GM_RSSP1_CFM_Interface_Proc_Send_Get_Object_Fail (GM_RSSP1_CFM_Interface_Proc_Send_Num<<8U)+Result_No_Match /*0xE1911620*/
#define GM_RSSP1_CFM_Interface_Proc_Send_State_Fail GM_RSSP1_CFM_Interface_Proc_Send_Get_Object_Fail+1U /*0xE1911621*/

/*GM_RSSP1_Get_ConIndex  23*/
#define GM_RSSP1_Get_ConIndex_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x17U)   /*0xE15117*/
#define GM_RSSP1_Get_ConIndex_Param_Point_Error (GM_RSSP1_Get_ConIndex_Num<<8U)+Param_Point_Error /*0xE1511711*/
#define GM_RSSP1_Get_ConIndex_Get_Object_Fail (GM_RSSP1_Get_ConIndex_Num<<8U)+Result_No_Match /*0xE1511720*/

/*GM_RSSP1_Get_Address  24*/
#define GM_RSSP1_Get_Address_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x18U)   /*0xE15118*/
#define GM_RSSP1_Get_Address_Param_Point_Error (GM_RSSP1_Get_Address_Num<<8U)+Param_Point_Error /*0xE1511811*/
#define GM_RSSP1_Get_Address_Get_Object_Fail (GM_RSSP1_Get_Address_Num<<8U)+Result_No_Match /*0xE1511820*/
#define GM_RSSP1_Get_Address_Con_index_Fail GM_RSSP1_Get_Address_Get_Object_Fail+1U /*0xE1511821*/
#define GM_RSSP1_Get_Address_Chn_index_Fail GM_RSSP1_Get_Address_Con_index_Fail+1U /*0xE1511822*/

/*GM_RSSP1_Write_SFM_Interface_ergodic  25*/
#define GM_RSSP1_Write_SFM_Interface_ergodic_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x19U)   /*0xE19119*/
#define GM_RSSP1_Write_SFM_Interface_ergodic_Param_Point_Error (GM_RSSP1_Write_SFM_Interface_ergodic_Num<<8U)+Param_Point_Error /*0xE1911911*/
#define GM_RSSP1_Write_SFM_Interface_ergodic_Get_Object_Fail (GM_RSSP1_Write_SFM_Interface_ergodic_Num<<8U)+Result_No_Match /*0xE1911920*/

/*RSSP1_CFM_Clear_Msg_Queue  26*/
#define RSSP1_CFM_Clear_Msg_Queue_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x1AU)   /*0xE1511A*/
#define RSSP1_CFM_Clear_Msg_Queue_Get_Object_Fail (RSSP1_CFM_Clear_Msg_Queue_Num<<8U)+Result_No_Match /*0xE1511A20*/

/*GM_RSSP1_SND_com_Interface  27*/
#define GM_RSSP1_SND_com_Interface_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x1BU)   /*0xE1911B*/
#define GM_RSSP1_SND_com_Interface_Param_Val_Error (GM_RSSP1_SND_com_Interface_Num<<8U)+Param_Val_Error /*0xE1911B12*/
#define GM_RSSP1_SND_com_Interface_Read_Fail (GM_RSSP1_SND_com_Interface_Num<<8U)+Result_No_Match /*0xE1911B20*/

/*GM_RSSP1_RCV_com_Interface  28*/
#define GM_RSSP1_RCV_com_Interface_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x1CU)   /*0xE1F11C*/
#define GM_RSSP1_RCV_com_Interface_Param_Val_Error (GM_RSSP1_RCV_com_Interface_Num<<8U)+Param_Val_Error /*0xE1F11C12*/
#define GM_RSSP1_RCV_com_Interface_Write_Fail (GM_RSSP1_RCV_com_Interface_Num<<8U)+Result_No_Match /*0xE1F11C20*/

/*GM_RSSP1_MessageQueue_Inter_Set  29*/
#define GM_RSSP1_MessageQueue_Inter_Set_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x1DU)   /*0xE1511D*/
#define GM_RSSP1_MessageQueue_Inter_Set_Param_Point_Error (GM_RSSP1_MessageQueue_Inter_Set_Num<<8U)+Param_Point_Error /*0xE1511D11*/
#define GM_RSSP1_MessageQueue_Inter_Set_Write_Fail (GM_RSSP1_MessageQueue_Inter_Set_Num<<8U)+Result_No_Match /*0xE1511D20*/

/*GM_RSSP1_MessageQueue_Inter_Get  30*/
#define GM_RSSP1_MessageQueue_Inter_Get_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x1EU)   /*0xE1511E*/
#define GM_RSSP1_MessageQueue_Inter_Get_Param_Point_Error (GM_RSSP1_MessageQueue_Inter_Get_Num<<8U)+Param_Point_Error /*0xE1511E11*/
#define GM_RSSP1_MessageQueue_Inter_Get_Write_Fail (GM_RSSP1_MessageQueue_Inter_Get_Num<<8U)+Result_No_Match /*0xE1511E20*/

/*GM_RSSP1_CRCM_CHN_CHECK_Int  31*/
#define GM_RSSP1_CRCM_CHN_CHECK_Int_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x1FU)   /*0xE1511F*/
#define GM_RSSP1_CRCM_CHN_CHECK_Int_Rem_Time_Fail (GM_RSSP1_CRCM_CHN_CHECK_Int_Num<<8U)+Result_No_Match /*0xE1511F20*/
#define GM_RSSP1_CRCM_CHN_CHECK_Int_Loc_Time_Fail GM_RSSP1_CRCM_CHN_CHECK_Int_Rem_Time_Fail+1U /*0xE1511F21*/
#define GM_RSSP1_CRCM_CHN_CHECK_Int_CRCM_Fail GM_RSSP1_CRCM_CHN_CHECK_Int_Loc_Time_Fail+1U /*0xE1511F22*/
#define GM_RSSP1_CRCM_CHN_CHECK_Int_DELAY_TIME GM_RSSP1_CRCM_CHN_CHECK_Int_CRCM_Fail+1U /*0xE1511F23*/

/*GM_RSSP1_SSE_BUILD_Int  32*/
#define GM_RSSP1_SSE_BUILD_Int_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x20U)   /*0xE15120*/
#define GM_RSSP1_SSE_BUILD_Int_Send_Invalid (GM_RSSP1_SSE_BUILD_Int_Num<<8U)+Result_No_Match /*0xE1512020*/

/*GM_RSSP1_SSR_CHN_SET  33*/
#define GM_RSSP1_SSR_CHN_SET_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x21U)   /*0xE15121*/
#define GM_RSSP1_SSR_CHN_SET_Msg_Invalid (GM_RSSP1_SSR_CHN_SET_Num<<8U)+Result_No_Match /*0xE1512120*/
#define GM_RSSP1_SSR_CHN_SET_Msg_TimeOut GM_RSSP1_SSR_CHN_SET_Msg_Invalid+1U /*0xE1512121*/
#define GM_RSSP1_SSR_CHN_SET_Msg_NoSEE GM_RSSP1_SSR_CHN_SET_Msg_TimeOut+1U /*0xE1512122*/

/*GM_RSSP1_SSR_CHN_SET  34*/
#define GM_RSSP1_PREC_SINT_CAL_Int_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x22U)   /*0xE15122*/
#define GM_RSSP1_PREC_SINT_CAL_Int_TimeOut (GM_RSSP1_PREC_SINT_CAL_Int_Num<<8U)+Result_No_Match /*0xE1512220*/

/*GM_RSSP1_PREC_FIRSTSINIT_CAL  35*/
#define GM_RSSP1_PREC_FIRSTSINIT_CAL_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x23U)   /*0xE15123*/
#define GM_RSSP1_PREC_SINT_CAL_Int_Param_Point_Error (GM_RSSP1_PREC_FIRSTSINIT_CAL_Num<<8U)+Param_Point_Error /*0xE1512320*/

/*GM_RSSP1_POST_RXDATA_Int  36*/
#define GM_RSSP1_POST_RXDATA_Int_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x24U)   /*0xE15124*/
#define GM_RSSP1_POST_RXDATA_Int_TimeOut (GM_RSSP1_POST_RXDATA_Int_Num<<8U)+Result_No_Match /*0xE1512420*/

/*GM_RSSP1_SFM_Ini_FSFB_Offline_Const  37*/
#define GM_RSSP1_SFM_Ini_FSFB_Offline_Const_Num ((GM_RSSP1_Init_Error<<8U)+0x25U)   /*0xE10125*/
#define GM_RSSP1_SFM_Ini_FSFB_Offline_Const_Param_Point_Error (GM_RSSP1_SFM_Ini_FSFB_Offline_Const_Num<<8U)+Param_Point_Error /*0xE1012511*/
#define GM_RSSP1_SFM_Ini_FSFB_Offline_Const_SINT_Error (GM_RSSP1_SFM_Ini_FSFB_Offline_Const_Num<<8U)+Result_No_Match /*0xE1012520*/
#define GM_RSSP1_SFM_Ini_FSFB_Offline_Const_FIRSTSINIT_Error GM_RSSP1_SFM_Ini_FSFB_Offline_Const_SINT_Error+1U /*0xE1012521*/
#define GM_RSSP1_SFM_Ini_FSFB_Offline_Const_RXDATA_Error GM_RSSP1_SFM_Ini_FSFB_Offline_Const_FIRSTSINIT_Error+1U /*0xE1012522*/

/*GM_RSSP1_SFM_Update_Time  38*/
#define GM_RSSP1_SFM_Update_Time_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x26U)   /*0xE15126*/
#define GM_RSSP1_SFM_Update_Time_Param_Point_Error (GM_RSSP1_SFM_Update_Time_Num<<8U)+Param_Point_Error /*0xE1512611*/
#define GM_RSSP1_SFM_Update_Time_Param_Result_No_Match (GM_RSSP1_SFM_Update_Time_Num<<8U)+Result_No_Match /*0xE1512620*/
#define GM_RSSP1_SFM_Interface_Update_Time_ByArray_Param_Point_Error GM_RSSP1_SFM_Update_Time_Param_Point_Error+1U /*0xE1512612*/

/*GM_RSSP1_SFM_Get_Library_Status  39*/
#define GM_RSSP1_SFM_Get_Library_Status_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x27U)   /*0xE15127*/
#define GM_RSSP1_SFM_Get_Library_Status_Point_Error (GM_RSSP1_SFM_Get_Library_Status_Num<<8U)+Param_Point_Error /*0xE1512711*/

/*GM_RSSP1_SFM_Get_Connection_BY_SaCEPID  40*/
#define GM_RSSP1_SFM_Get_Connection_BY_SaCEPID_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x28U)   /*0xE15128*/
#define GM_RSSP1_SFM_Get_Connection_BY_SaCEPID_Point_Error (GM_RSSP1_SFM_Get_Connection_BY_SaCEPID_Num<<8U)+Param_Point_Error /*0xE1512811*/
#define GM_RSSP1_SFM_Get_Connection_BY_SaCEPID_Result_No_Match (GM_RSSP1_SFM_Get_Connection_BY_SaCEPID_Num<<8U)+Result_No_Match /*0xE1512820*/

/*GM_RSSP1_SFM_Is_SaCEPID_Equal  41*/
#define GM_RSSP1_SFM_Is_SaCEPID_Equal_Num ((GM_RSSP1_Common_Part_Error<<8U)+0x29U)   /*0xE15129*/
#define GM_RSSP1_SFM_Is_SaCEPID_Equal_Point_Error (GM_RSSP1_SFM_Is_SaCEPID_Equal_Num<<8U)+Param_Point_Error /*0xE1512911*/

/*GM_RSSP1_User_Put_Pri_To_SFM  42*/
#define GM_RSSP1_User_Put_Pri_To_SFM_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x2AU)   /*0xE1912A*/
#define GM_RSSP1_User_Put_Pri_To_SFM_Result_No_Match (GM_RSSP1_User_Put_Pri_To_SFM_Num<<8U)+Result_No_Match /*0xE1912A20*/
#define GM_RSSP1_User_Put_Pri_To_SFM_Type_Unknow GM_RSSP1_User_Put_Pri_To_SFM_Result_No_Match+1U /*0xE1912A21*/
#define GM_RSSP1_User_Put_Pri_To_SFM_Index_Unknow GM_RSSP1_User_Put_Pri_To_SFM_Type_Unknow+1U /*0xE1912A22*/
#define GM_RSSP1_User_Put_Pri_To_SFM_WRITE_FAIL GM_RSSP1_User_Put_Pri_To_SFM_Index_Unknow+1U /*0xE1912A23*/
#define GM_RSSP1_User_Put_Pri_To_SFM_WRITE_OK_FULL GM_RSSP1_User_Put_Pri_To_SFM_WRITE_FAIL+1U /*0xE1912A24*/
#define GM_RSSP1_User_Put_Pri_To_SFM_Point_Error (GM_RSSP1_User_Put_Pri_To_SFM_Num<<8U)+Param_Point_Error /*0xE1912A11*/

/*GM_RSSP1_User_Get_Pri_From_SFM  43*/
#define GM_RSSP1_User_Get_Pri_From_SFM_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x2BU)   /*0xE1F12B*/
#define GM_RSSP1_User_Get_Pri_From_SFM_Point_Error (GM_RSSP1_User_Get_Pri_From_SFM_Num<<8U)+Param_Point_Error /*0xE1F12B11*/
#define GM_RSSP1_User_Get_Pri_From_SFM_Result_No_Match (GM_RSSP1_User_Get_Pri_From_SFM_Num<<8U)+Result_No_Match /*0xE1F12B20*/
#define GM_RSSP1_User_Get_Pri_From_SFM_Get_Pri_Fail GM_RSSP1_User_Get_Pri_From_SFM_Result_No_Match+1U /*0xE1F12B21*/

/*GM_RSSP1_User_Send_Dat  44*/
#define GM_RSSP1_User_Send_Dat_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x2CU)   /*0xE1912C*/
#define GM_RSSP1_User_Send_Dat_Point_Error (GM_RSSP1_User_Send_Dat_Num<<8U)+Param_Point_Error /*0xE1912C11*/
#define GM_RSSP1_User_Send_Dat_Result_No_Match (GM_RSSP1_User_Send_Dat_Num<<8U)+Result_No_Match /*0xE1912C20*/
#define GM_RSSP1_User_Send_Dat_Data_Error GM_RSSP1_User_Send_Dat_Result_No_Match+1U /*0xE1912C21*/
#define GM_RSSP1_User_Send_Dat_SYS_Unknow GM_RSSP1_User_Send_Dat_Data_Error+1U /*0xE1912C22*/

/*GM_RSSP1_SFM_Send_RSD  45*/
#define GM_RSSP1_SFM_Send_RSD_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x2DU)   /*0xE1912D*/
#define GM_RSSP1_SFM_Send_RSD_Point_Error (GM_RSSP1_SFM_Send_RSD_Num<<8U)+Param_Point_Error /*0xE1912D11*/
#define GM_RSSP1_SFM_Send_RSD_Result_No_Match (GM_RSSP1_SFM_Send_RSD_Num<<8U)+Result_No_Match /*0xE1912D20*/
#define GM_RSSP1_SFM_Send_RSD_Build_Error GM_RSSP1_SFM_Send_RSD_Result_No_Match+1U /*0xE1912D21*/

/*GM_RSSP1_SFM_Send_SSE  46*/
#define GM_RSSP1_SFM_Send_SSE_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x2EU)   /*0xE1912E*/
#define GM_RSSP1_SFM_Send_SSE_Point_Error (GM_RSSP1_SFM_Send_SSE_Num<<8U)+Param_Point_Error /*0xE1912E11*/
#define GM_RSSP1_SFM_Send_SSE_Result_No_Match (GM_RSSP1_SFM_Send_SSE_Num<<8U)+Result_No_Match /*0xE1912E20*/
#define GM_RSSP1_SFM_Send_SSE_Build_Error GM_RSSP1_SFM_Send_SSE_Result_No_Match+1U /*0xE1912E21*/

/*GM_RSSP1_SFM_Send_SSR  47*/
#define GM_RSSP1_SFM_Send_SSR_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x2FU)   /*0xE1912F*/
#define GM_RSSP1_SFM_Send_SSR_Point_Error (GM_RSSP1_SFM_Send_SSR_Num<<8U)+Param_Point_Error /*0xE1912F11*/
#define GM_RSSP1_SFM_Send_SSR_Result_No_Match (GM_RSSP1_SFM_Send_SSR_Num<<8U)+Result_No_Match /*0xE1912F20*/
#define GM_RSSP1_SFM_Send_SSR_Build_Error GM_RSSP1_SFM_Send_SSR_Result_No_Match+1U /*0xE1912F21*/

/*GM_RSSP1_SFM_Proc_Rcvd_SSE  48*/
#define GM_RSSP1_SFM_Proc_Rcvd_SSE_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x30U)   /*0xE1F130*/
#define GM_RSSP1_SFM_Proc_Rcvd_SSE_Point_Error (GM_RSSP1_SFM_Proc_Rcvd_SSE_Num<<8U)+Param_Point_Error /*0xE1F13011*/
#define GM_RSSP1_SFM_Proc_Rcvd_SSE_Result_No_Match (GM_RSSP1_SFM_Proc_Rcvd_SSE_Num<<8U)+Result_No_Match /*0xE1F13020*/
#define GM_RSSP1_SFM_Proc_Rcvd_SSE_Byte_Loss GM_RSSP1_SFM_Proc_Rcvd_SSE_Result_No_Match+1U /*0xE1F13021*/
#define GM_RSSP1_SFM_Proc_Rcvd_SSE_Addr_Error GM_RSSP1_SFM_Proc_Rcvd_SSE_Byte_Loss+1U /*0xE1F13022*/
#define GM_RSSP1_SFM_Proc_Rcvd_SSE_CRC16_Error GM_RSSP1_SFM_Proc_Rcvd_SSE_Addr_Error+1U /*0xE1F13023*/

/*GM_RSSP1_SFM_Proc_Rcvd_SSR  49*/
#define GM_RSSP1_SFM_Proc_Rcvd_SSR_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x31U)   /*0xE1F131*/
#define GM_RSSP1_SFM_Proc_Rcvd_SSR_Point_Error (GM_RSSP1_SFM_Proc_Rcvd_SSR_Num<<8U)+Param_Point_Error /*0xE1F13111*/
#define GM_RSSP1_SFM_Proc_Rcvd_SSR_Result_No_Match (GM_RSSP1_SFM_Proc_Rcvd_SSR_Num<<8U)+Result_No_Match /*0xE1F13120*/
#define GM_RSSP1_SFM_Proc_Rcvd_SSR_Byte_Loss GM_RSSP1_SFM_Proc_Rcvd_SSR_Result_No_Match+1U /*0xE1F13121*/
#define GM_RSSP1_SFM_Proc_Rcvd_SSR_Addr_Error GM_RSSP1_SFM_Proc_Rcvd_SSR_Byte_Loss+1U /*0xE1F13122*/
#define GM_RSSP1_SFM_Proc_Rcvd_SSR_CRC16_Error GM_RSSP1_SFM_Proc_Rcvd_SSR_Addr_Error+1U /*0xE1F13123*/
#define GM_RSSP1_SFM_Proc_Rcvd_SSR_DataVer_Error GM_RSSP1_SFM_Proc_Rcvd_SSR_CRC16_Error+1U /*0xE1F13123*/

/*GM_RSSP1_SFM_Proc_Rcvd_RSD  50*/
#define GM_RSSP1_SFM_Proc_Rcvd_RSD_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x32U)   /*0xE1F132*/
#define GM_RSSP1_SFM_Proc_Rcvd_RSD_Point_Error (GM_RSSP1_SFM_Proc_Rcvd_RSD_Num<<8U)+Param_Point_Error /*0xE1F13211*/
#define GM_RSSP1_SFM_Proc_Rcvd_RSD_Result_No_Match (GM_RSSP1_SFM_Proc_Rcvd_RSD_Num<<8U)+Result_No_Match /*0xE1F13220*/
#define GM_RSSP1_SFM_Proc_Rcvd_RSD_Byte_Error GM_RSSP1_SFM_Proc_Rcvd_RSD_Result_No_Match+1U /*0xE1F13221*/
#define GM_RSSP1_SFM_Proc_Rcvd_RSD_Addr_Error GM_RSSP1_SFM_Proc_Rcvd_RSD_Byte_Error+1U /*0xE1F13222*/
#define GM_RSSP1_SFM_Proc_Rcvd_RSD_CRC16_Error GM_RSSP1_SFM_Proc_Rcvd_RSD_Addr_Error+1U /*0xE1F13223*/
#define GM_RSSP1_SFM_Proc_Rcvd_RSD_Check_Error GM_RSSP1_SFM_Proc_Rcvd_RSD_CRC16_Error+1U /*0xE1F13224*/
#define GM_RSSP1_SFM_Proc_Rcvd_RSD_Write_Full GM_RSSP1_SFM_Proc_Rcvd_RSD_Check_Error+1U /*0xE1F13225*/
#define GM_RSSP1_SFM_Proc_Rcvd_RSD_Write_Error GM_RSSP1_SFM_Proc_Rcvd_RSD_Write_Full+1U /*0xE1F13226*/
#define GM_RSSP1_SFM_Proc_Rcvd_RSD_Remote_AB_Error GM_RSSP1_SFM_Proc_Rcvd_RSD_Write_Error+1U /*0xE1F13227*/

/*GM_RSSP1_SFM_Reset_Connection_RSD_Sent_Flag  51*/
#define GM_RSSP1_SFM_Reset_Connection_RSD_Sent_Flag_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x33U)   /*0xE19133*/
#define GM_RSSP1_SFM_Reset_Connection_RSD_Sent_Flag_Point_Error (GM_RSSP1_SFM_Reset_Connection_RSD_Sent_Flag_Num<<8U)+Param_Point_Error /*0xE1913311*/

/*GM_RSSP1_SFM_Process_User_Req  52*/
#define GM_RSSP1_SFM_Process_User_Req_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x34U)   /*0xE19134*/
#define GM_RSSP1_SFM_Process_User_Req_Point_Error (GM_RSSP1_SFM_Process_User_Req_Num<<8U)+Param_Point_Error /*0xE1913411*/
#define GM_RSSP1_SFM_Process_User_Req_Result_No_Match (GM_RSSP1_SFM_Process_User_Req_Num<<8U)+Result_No_Match /*0xE1913420*/

/*GM_RSSP1_SFM_Report_Connection_State_To_User  53*/
#define GM_RSSP1_SFM_Report_Connection_State_To_User_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x35U)   /*0xE1F135*/
#define GM_RSSP1_SFM_Report_Connection_State_To_User_Point_Error (GM_RSSP1_SFM_Report_Connection_State_To_User_Num<<8U)+Param_Point_Error /*0xE1F13511*/
#define GM_RSSP1_SFM_Report_Connection_State_To_User_Result_No_Match (GM_RSSP1_SFM_Report_Connection_State_To_User_Num<<8U)+Result_No_Match /*0xE1F13520*/
#define GM_RSSP1_SFM_Report_Connection_State_To_User_Index_No_Match GM_RSSP1_SFM_Report_Connection_State_To_User_Result_No_Match+1U /*0xE1F13521*/
#define GM_RSSP1_SFM_Report_Connection_State_To_User_Write_Full GM_RSSP1_SFM_Report_Connection_State_To_User_Index_No_Match+1U /*0xE1F13522*/
#define GM_RSSP1_SFM_Report_Connection_State_To_User_Write_Fail GM_RSSP1_SFM_Report_Connection_State_To_User_Write_Full+1U /*0xE1F13523*/

/*GM_RSSP1_SFM_Report_Warning_To_User  54*/
#define GM_RSSP1_SFM_Report_Warning_To_User_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x36U)   /*0xE1F136*/
#define GM_RSSP1_SFM_Report_Warning_To_User_Point_Error (GM_RSSP1_SFM_Report_Warning_To_User_Num<<8U)+Param_Point_Error /*0xE1F13611*/
#define GM_RSSP1_SFM_Report_Warning_To_User_Result_No_Match (GM_RSSP1_SFM_Report_Warning_To_User_Num<<8U)+Result_No_Match /*0xE1F13620*/
#define GM_RSSP1_SFM_Report_Warning_To_User_Write_Full GM_RSSP1_SFM_Report_Warning_To_User_Result_No_Match+1U /*0xE1F13621*/
#define GM_RSSP1_SFM_Report_Warning_To_User_Write_Fail GM_RSSP1_SFM_Report_Warning_To_User_Write_Full+1U /*0xE1F13622*/

/*GM_RSSP1_SFM_Process_CFM_Ind  55*/
#define GM_RSSP1_SFM_Process_CFM_Ind_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x37U)   /*0xE1F137*/
#define GM_RSSP1_SFM_Process_CFM_Ind_Result_No_Match (GM_RSSP1_SFM_Process_CFM_Ind_Num<<8U)+Result_No_Match /*0xE1F13720*/

/*GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate  56*/
#define GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x38U)   /*0xE1F138*/
#define GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate_Result_No_Match (GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate_Num<<8U)+Result_No_Match /*0xE1F13820*/
#define GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate_Write_Full GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate_Result_No_Match+1U /*0xE1F13821*/
#define GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate_Write_Fail GM_RSSP1_SFM_Proc_Connection_Monit_And_Tolerate_Write_Full+1U /*0xE1F13822*/

/*GM_RSSP1_SFM_Dispatch_Dat_Indi  57*/
#define GM_RSSP1_SFM_Dispatch_Dat_Indi_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x39U)   /*0xE1F139*/
#define GM_RSSP1_SFM_Dispatch_Dat_Indi_Result_No_Match (GM_RSSP1_SFM_Dispatch_Dat_Indi_Num<<8U)+Result_No_Match /*0xE1F13920*/
#define GM_RSSP1_SFM_Dispatch_Dat_Indi_Write_Full GM_RSSP1_SFM_Dispatch_Dat_Indi_Result_No_Match+1U /*0xE1F13921*/
#define GM_RSSP1_SFM_Dispatch_Dat_Indi_Write_Fail GM_RSSP1_SFM_Dispatch_Dat_Indi_Write_Full+1U /*0xE1F13922*/

/*GM_RSSP1_Check_Vital_Cfg  58*/
#define GM_RSSP1_Check_Vital_Cfg_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x3AU)   /*0xE1F13A*/
#define GM_RSSP1_Check_Vital_Cfg_Point_Error (GM_RSSP1_Check_Vital_Cfg_Num<<8U)+Param_Point_Error /*0xE1F13A11*/

/*GM_RSSP1_SFM_Get_Pri_From_CFM  59*/
#define GM_RSSP1_SFM_Get_Pri_From_CFM_Num ((GM_RSSP1_Proc_Rx_Error<<8U)+0x3BU)   /*0xE1F13B*/
#define GM_RSSP1_SFM_Get_Pri_From_CFM_Point_Error (GM_RSSP1_SFM_Get_Pri_From_CFM_Num<<8U)+Param_Point_Error /*0xE1F13B11*/

/*GM_RSSP1_SFM_Put_Pri_To_CFM  60*/
#define GM_RSSP1_SFM_Put_Pri_To_CFM_Num ((GM_RSSP1_Proc_Tx_Error<<8U)+0x3CU)   /*0xE1913C*/
#define GM_RSSP1_SFM_Put_Pri_To_CFM_Result_Point_Error (GM_RSSP1_SFM_Put_Pri_To_CFM_Num<<8U)+Param_Point_Error /*0xE1913C11*/
#define GM_RSSP1_SFM_Put_Pri_To_CFM_Result_No_Match (GM_RSSP1_SFM_Put_Pri_To_CFM_Num<<8U)+Result_No_Match /*0xE1913C20*/
#define GM_RSSP1_SFM_Put_Pri_To_CFM_Type_Error GM_RSSP1_SFM_Put_Pri_To_CFM_Result_No_Match+1U /*0xE1913C21*/
#define GM_RSSP1_SFM_Put_Pri_To_CFM_Index_Error GM_RSSP1_SFM_Put_Pri_To_CFM_Type_Error+1U /*0xE1913C22*/
#define GM_RSSP1_SFM_Put_Pri_To_CFM_Write_Full GM_RSSP1_SFM_Put_Pri_To_CFM_Index_Error+1U /*0xE1F13923*/
#define GM_RSSP1_SFM_Put_Pri_To_CFM_Write_Fail GM_RSSP1_SFM_Put_Pri_To_CFM_Write_Full+1U /*0xE1F13924*/

/*GM_RSSP1_Init_Comm_Global_SFM  61*/
#define GM_RSSP1_Init_Comm_Global_SFM_Num ((GM_RSSP1_Init_Error<<8U)+0x3DU)   /*0xE1013D*/
#define GM_RSSP1_Init_Comm_Global_SFM_Point_Error (GM_RSSP1_Init_Comm_Global_SFM_Num<<8U)+Param_Point_Error /*0xE1013D11*/
#define GM_RSSP1_Init_Comm_Global_SFM_Connec_Num_Error (GM_RSSP1_Init_Comm_Global_SFM_Num<<8U)+Result_No_Match /*0xE1013D20*/
#define GM_RSSP1_Init_Comm_Global_SFM_Src_Error GM_RSSP1_Init_Comm_Global_SFM_Connec_Num_Error+1U /*0xE1013D21*/
#define GM_RSSP1_Init_Comm_Global_SFM_Loc_Sid_Error GM_RSSP1_Init_Comm_Global_SFM_Src_Error+1U /*0xE1013D22*/
#define GM_RSSP1_Init_Comm_Global_SFM_Loc_Sinit_Error GM_RSSP1_Init_Comm_Global_SFM_Loc_Sid_Error+1U /*0xE1013D23*/
#define GM_RSSP1_Init_Comm_Global_SFM_Loc_DataVer_Error GM_RSSP1_Init_Comm_Global_SFM_Loc_Sinit_Error+1U /*0xE1013D24*/
#define GM_RSSP1_Init_Comm_Global_SFM_Loc_SysChk_Error GM_RSSP1_Init_Comm_Global_SFM_Loc_DataVer_Error+1U /*0xE1013D25*/
#define GM_RSSP1_Init_Comm_Global_SFM_U2L_Q_Size_Error GM_RSSP1_Init_Comm_Global_SFM_Loc_SysChk_Error+1U /*0xE1013D26*/
#define GM_RSSP1_Init_Comm_Global_SFM_L2U_Q_Size_Error GM_RSSP1_Init_Comm_Global_SFM_U2L_Q_Size_Error+1U /*0xE1013D27*/
#define GM_RSSP1_Init_Comm_Global_SFM_Malloc_Error GM_RSSP1_Init_Comm_Global_SFM_L2U_Q_Size_Error+1U /*0xE1013D28*/
#define GM_RSSP1_Init_Comm_Global_SFM_Mem_Size_Error GM_RSSP1_Init_Comm_Global_SFM_Malloc_Error+1U /*0xE1013D29*/
#define GM_RSSP1_Init_Comm_Global_SFM_Mem_Ref_Val_Error GM_RSSP1_Init_Comm_Global_SFM_Mem_Size_Error+1U /*0xE1013D2A*/
#define GM_RSSP1_Init_Comm_Global_SFM_Mem_Rec_Val_Error GM_RSSP1_Init_Comm_Global_SFM_Mem_Ref_Val_Error+1U /*0xE1013D2B*/
#define GM_RSSP1_Init_Comm_Global_CFM_U2L_Q_Size_Error GM_RSSP1_Init_Comm_Global_SFM_Mem_Rec_Val_Error+1U /*0xE1013D2C*/
#define GM_RSSP1_Init_Comm_Global_CFM_L2U_Q_Size_Error GM_RSSP1_Init_Comm_Global_CFM_U2L_Q_Size_Error+1U /*0xE1013D2D*/
#define GM_RSSP1_Init_Comm_Global_CFM_Malloc_Error GM_RSSP1_Init_Comm_Global_CFM_L2U_Q_Size_Error+1U /*0xE1013D2E*/
#define GM_RSSP1_Init_Comm_Global_SFM_Max_Num_Error GM_RSSP1_Init_Comm_Global_CFM_Malloc_Error+1U /*0xE1013D2F*/

/*GM_RSSP1_Init_Comm_Connection_SFM  62*/
#define GM_RSSP1_Init_Comm_Connection_SFM_Num ((GM_RSSP1_Init_Error<<8U)+0x3EU)   /*0xE1013E*/
#define GM_RSSP1_Init_Comm_Connection_SFM_Point_Error (GM_RSSP1_Init_Comm_Connection_SFM_Num<<8U)+Param_Point_Error /*0xE1013E11*/
#define GM_RSSP1_Init_Comm_Connection_SFM_Des_Error (GM_RSSP1_Init_Comm_Connection_SFM_Num<<8U)+Result_No_Match /*0xE1013E20*/
#define GM_RSSP1_Init_Comm_Connection_SFM_Delta_Error GM_RSSP1_Init_Comm_Connection_SFM_Des_Error+1U /*0xE1013E21*/
#define GM_RSSP1_Init_Comm_Connection_SFM_LifeTime_Error GM_RSSP1_Init_Comm_Connection_SFM_Delta_Error+1U /*0xE1013E22*/
#define GM_RSSP1_Init_Comm_Connection_SFM_TorCyc_Error GM_RSSP1_Init_Comm_Connection_SFM_LifeTime_Error+1U /*0xE1013E23*/
#define GM_RSSP1_Init_Comm_Connection_SFM_DataVer_Error GM_RSSP1_Init_Comm_Connection_SFM_TorCyc_Error+1U /*0xE1013E24*/
#define GM_RSSP1_Init_Comm_Connection_SFM_Rem_Sid_Error GM_RSSP1_Init_Comm_Connection_SFM_DataVer_Error+1U /*0xE1013E25*/
#define GM_RSSP1_Init_Comm_Connection_SFM_Rem_Sinit_Error GM_RSSP1_Init_Comm_Connection_SFM_Rem_Sid_Error+1U /*0xE1013E26*/
#define GM_RSSP1_Init_Comm_Connection_SFM_Rem_DateVer_Error GM_RSSP1_Init_Comm_Connection_SFM_Rem_Sinit_Error+1U /*0xE1013E27*/
#define GM_RSSP1_Init_Comm_Connection_SFM_EnableUDP_Error GM_RSSP1_Init_Comm_Connection_SFM_Rem_DateVer_Error+1U /*0xE1013E28*/
#define GM_RSSP1_Init_Comm_Connection_SFM_FSFB_ID_Error GM_RSSP1_Init_Comm_Connection_SFM_EnableUDP_Error+1U /*0xE1013E29*/
#define GM_RSSP1_Init_Comm_Connection_SFM_L2U_Q_Size_Error GM_RSSP1_Init_Comm_Connection_SFM_FSFB_ID_Error+1U /*0xE1013E2A*/
#define GM_RSSP1_Init_Comm_Connection_SFM_Malloc_Error GM_RSSP1_Init_Comm_Connection_SFM_L2U_Q_Size_Error+1U /*0xE1013E2B*/
#define GM_RSSP1_Init_Comm_Connection_SFM_Loss_Error GM_RSSP1_Init_Comm_Connection_SFM_Malloc_Error+1U /*0xE1013E2C*/
#define GM_RSSP1_Init_Comm_Connection_SFM_IsFixNum_Error GM_RSSP1_Init_Comm_Connection_SFM_Loss_Error+1U /*0xE1013E2D*/

/*GM_RSSP1_SFM_MQ_Init  63*/
#define GM_RSSP1_SFM_MQ_Init_Num ((GM_RSSP1_Init_Error<<8U)+0x3FU)   /*0xE1013F*/
#define GM_RSSP1_SFM_MQ_Init_U2L_Memory_Fail (GM_RSSP1_SFM_MQ_Init_Num<<8U)+Memory_Error /*0xE1013F50*/
#define GM_RSSP1_SFM_MQ_Init_L2U_Memory_Fail GM_RSSP1_SFM_MQ_Init_U2L_Memory_Fail+1U /*0xE1013F51*/
#define GM_RSSP1_SFM_MQ_Init_CFM_U2L_Memory_Fail GM_RSSP1_SFM_MQ_Init_L2U_Memory_Fail+1U /*0xE1013F52*/
#define GM_RSSP1_SFM_MQ_Init_CFM_L2U_Memory_Fail GM_RSSP1_SFM_MQ_Init_CFM_U2L_Memory_Fail+1U /*0xE1013F53*/
#define GM_RSSP1_SFM_MQ_Init_U2L_MQ_Fail (GM_RSSP1_SFM_MQ_Init_Num<<8U)+Result_No_Match  /*0xE1013F20*/
#define GM_RSSP1_SFM_MQ_Init_L2U_MQ_Fail GM_RSSP1_SFM_MQ_Init_U2L_MQ_Fail+1U  /*0xE1013F21*/
#define GM_RSSP1_SFM_MQ_Init_CFM_U2L_MQ_Fail GM_RSSP1_SFM_MQ_Init_L2U_MQ_Fail+1U  /*0xE1013F22*/
#define GM_RSSP1_SFM_MQ_Init_CFM_L2U_MQ_Fail GM_RSSP1_SFM_MQ_Init_CFM_U2L_MQ_Fail+1U  /*0xE1013F23*/

/*GM_RSSP1_APP_Interface_ReLoad  64*/
#define GM_RSSP1_APP_Interface_ReLoad_Num ((GM_RSSP1_Init_Error<<8U)+0x40U)   /*0xE10140*/
#define GM_RSSP1_APP_Interface_ReLoad_Param_Point_Error (GM_RSSP1_APP_Interface_ReLoad_Num<<8U)+Param_Point_Error /*0xE1014011*/
#define GM_RSSP1_APP_Interface_ReLoad_FILE_Size_Error GM_RSSP1_APP_Interface_ReLoad_Param_Point_Error+1U	/*0xE1014012*/
#define GM_RSSP1_APP_Interface_ReLoad_FILE_Load_Error GM_RSSP1_APP_Interface_ReLoad_FILE_Size_Error+1U	/*0xE1014013*/
#define GM_RSSP1_APP_Interface_ReLoad_SFM_Init_Error GM_RSSP1_APP_Interface_ReLoad_FILE_Load_Error+1U	/*0xE1014014*/
#define GM_RSSP1_APP_Interface_ReLoad_CFM_Init_Error GM_RSSP1_APP_Interface_ReLoad_SFM_Init_Error+1U	/*0xE1014015*/
#define GM_RSSP1_APP_Interface_ReLoad_Memory_Error GM_RSSP1_APP_Interface_ReLoad_Num+Memory_Error	/*0xE1014050*/

/*GM_RSSP1_CFM_MQ_Init  65*/
#define GM_RSSP1_CFM_MQ_Init_Num ((GM_RSSP1_Init_Error<<8U)+0x41U)   /*0xE10141*/
#define GM_RSSP1_CFM_MQ_Init_Link_RCV_Memory_Fail (GM_RSSP1_CFM_MQ_Init_Num<<8U)+Memory_Error /*0xE1014150*/
#define GM_RSSP1_CFM_MQ_Init_Link_SND_Memory_Fail GM_RSSP1_CFM_MQ_Init_Link_RCV_Memory_Fail+1U /*0xE1014151*/
#define GM_RSSP1_CFM_MQ_Init_L2U_Memory_Fail GM_RSSP1_CFM_MQ_Init_Link_SND_Memory_Fail+1U /*0xE1014152*/
#define GM_RSSP1_CFM_MQ_Init_U2L_Memory_Fail GM_RSSP1_CFM_MQ_Init_L2U_Memory_Fail+1U /*0xE1014153*/
#define GM_RSSP1_CFM_MQ_Init_StbyAnsw_Memory_Fail GM_RSSP1_CFM_MQ_Init_U2L_Memory_Fail+1U /*0xE1014154*/
#define GM_RSSP1_CFM_MQ_Init_Link_RCV_MQ_Fail (GM_RSSP1_CFM_MQ_Init_Num<<8U)+Result_No_Match /*0xE1014120*/
#define GM_RSSP1_CFM_MQ_Init_Link_SND_MQ_Fail GM_RSSP1_CFM_MQ_Init_Link_RCV_MQ_Fail+1U /*0xE1014121*/
#define GM_RSSP1_CFM_MQ_Init_L2U_MQ_Fail GM_RSSP1_CFM_MQ_Init_Link_SND_MQ_Fail+1U  /*0xE1014122*/
#define GM_RSSP1_CFM_MQ_Init_U2L_MQ_Fail GM_RSSP1_CFM_MQ_Init_L2U_MQ_Fail+1U  /*0xE1014123*/
#define GM_RSSP1_CFM_MQ_Init_StbyAnsw_MQ_Fail GM_RSSP1_CFM_MQ_Init_U2L_MQ_Fail+1U  /*0xE1014124*/

/*GM_RSSP1_CFM_Delete  66*/
#define GM_RSSP1_CFM_Delete_Num ((GM_RSSP1_Init_Error<<8U)+0x42U)   /*0xE10142*/
#define GM_RSSP1_CFM_Delete_Param_Point_Error ((GM_RSSP1_CFM_Delete_Num<<8U)+ Param_Point_Error)  /*0xE1014211*/

/*GM_RSSP1_Syn  67*/
#define GM_RSSP1_Syn_Num ((GM_RSSP1_Init_Error<<8U)+0x43U)   /*0xE10143*/
#define GM_RSSP1_Syn_Data_OneNode_Get_Param_Point_Error  ((GM_RSSP1_Syn_Num<<8U)+ Param_Point_Error)  /*0xE1014311*/
#define GM_RSSP1_Syn_Data_OneNode_Get_Index_Addr_Not_Match_Error  ((GM_RSSP1_Syn_Num<<8U)+ Result_No_Match)  /*0xE1014320*/
#define GM_RSSP1_Syn_Data_OneNode_Set_Param_Point_Error  GM_RSSP1_Syn_Data_OneNode_Get_Param_Point_Error+1U /*0xE1014312*/
#define GM_RSSP1_Syn_Data_OneNode_Set_SaCEPID_Not_Match_Error  GM_RSSP1_Syn_Data_OneNode_Get_Index_Addr_Not_Match_Error+1U  /*0xE1014321*/
#define GM_RSSP1_Syn_Data_Set_Datasize_Error GM_RSSP1_Syn_Data_OneNode_Set_SaCEPID_Not_Match_Error+1U   /*0xE1014322*/

/*GM_RSSP1_SFM_Next_TC_CallBack  68*/
#define GM_RSSP1_SFM_Next_TC_CallBack_Num ((GM_RSSP1_Init_Error<<8U)+0x44U)   /*0xE10144*/
#define GM_RSSP1_SFM_Next_TC_CallBack_ConIndex_Err ((GM_RSSP1_SFM_Next_TC_CallBack_Num<<8U)+ Result_No_Match)  /*0xE1014420*/

/*GM_RSSP1_SFM_Process_TxData  69*/
#define GM_RSSP1_SFM_Process_TxData_Num ((GM_RSSP1_Init_Error<<8U)+0x45U)   /*0xE10145*/
#define GM_RSSP1_SFM_Process_TxData_Param_Point_Error  ((GM_RSSP1_SFM_Process_TxData_Num<<8U)+ Param_Point_Error)  /*0xE1014511*/

/*GM_RSSP1_APP_Interface_RxPrc  70*/
#define GM_RSSP1_APP_Interface_RxPrc_Num ((GM_RSSP1_Init_Error<<8U)+0x46U)   /*0xE10146*/
#define GM_RSSP1_APP_Interface_RxPrc_Point_Match_Error  ((GM_RSSP1_APP_Interface_RxPrc_Num<<8U)+ Result_No_Match)  /*0xE1014620*/

/******************FILE部分,错误号为0xE111XXXX****************/
/**GM_RSSP1_Bin_File_Load(),0xE11101XX**/
#define  GM_RSSP1_BIN_FILE_LOAD_PAR_ERR		0xE1110100U			/*GM_RSSP1_Bin_File_Load传入参数有误*/
#define  GM_RSSP1_BIN_FILE_LOAD_OPEN_ERR		0xE1110101U			/*GM_RSSP1_Bin_File_Load打开文件失败*/
#define  GM_RSSP1_BIN_FILE_LOAD_LEN_ERR		0xE1110102U			/*GM_RSSP1_Bin_File_Load文件长度错误*/
#define  GM_RSSP1_BIN_FILE_LOAD_READ_ERR		0xE1110103U			/*GM_RSSP1_Bin_File_Load读取内容失败*/
#define  GM_RSSP1_BIN_FILE_LOAD_CLOSE_ERR		0xE1110104U			/*GM_RSSP1_Bin_File_Load关闭文件失败*/

/**GM_RSSP1_Ini_File_Load(),0xE11102XX**/
#define  GM_RSSP1_INI_FILE_LOAD_PAR_ERR			0xE1110200U			/*GM_RSSP1_Ini_File_Load传入参数有误*/
#define  GM_RSSP1_INI_FILE_LOAD_OPEN_ERR			0xE1110201U			/*GM_RSSP1_Ini_File_Load打开文件失败*/
#define  GM_RSSP1_INI_FILE_LOAD_CONTENT_ERR		0xE1110202U			/*GM_RSSP1_Ini_File_Load读取内容过多*/
#define  GM_RSSP1_INI_FILE_LOAD_CLOSE_ERR			0xE1110203U			/*GM_RSSP1_Ini_File_Load关闭文件失败*/

/**GM_RSSP1_Ini_File_Read_String()，0xE11103XX**/
#define  GM_RSSP1_INI_FILE_READ_STRING_PAR_ERR		0xE1110300U			/*GM_RSSP1_Ini_File_Read_String传入参数有误*/
#define  GM_RSSP1_INI_FILE_READ_STRING_NOT_FIND_ERR	0xE1110301U			/*GM_RSSP1_Ini_File_Read_String没有找到key*/
#define  GM_RSSP1_INI_FILE_READ_STRING_SIZE_ERR		0xE1110302U			/*GM_RSSP1_Ini_File_Read_String传入的size太小*/

/**GM_RSSP1_Ini_File_Read_Int(),0xE11104XX**/
#define  GM_RSSP1_INI_FILE_READ_INT_PAR_ERR			0xE1110400U			/*GM_RSSP1_Ini_File_Read_Int传入参数有误*/
#define  GM_RSSP1_INI_FILE_READ_INT_STRING_ERR		0xE1110401U			/*GM_RSSP1_Ini_File_Read_String返回失败*/
#define  GM_RSSP1_INI_FILE_READ_INT_ATOI_ERR			0xE1110402U			/*GM_atoi返回失败*/

/**GM_RSSP1_Ini_File_Read_BOOL(),0xE11105XX**/
#define  GM_RSSP1_INI_FILE_READ_BOOL_PAR_ERR			0xE1110500U			/*GM_RSSP1_Ini_File_Read_BOOL传入参数有误*/
#define  GM_RSSP1_INI_FILE_READ_BOOL_VALUE_ERR		0xE1110501U			/*读出的值不是BOOL类型*/
#define  GM_RSSP1_INI_FILE_READ_BOOL_READ_ERR			0xE1110502U			/*GM_RSSP1_Ini_File_Read_Int返回失败*/

/*********************VSN部分,错误号为0xE121XXXX******************/
/**GM_RSSP1_VSN_Init(),0xE12101XX**/
#define GM_RSSP1_VSN_INIT_LFSR_1_ERR			0xE1210100U		/*GM_RSSP1_LFSR_Init通道1返回失败*/
#define GM_RSSP1_VSN_INIT_LFSR_2_ERR			0xE1210101U		/*GM_RSSP1_LFSR_Init通道2返回失败*/

/*********************OTHER部分,错误号为0xE131XXXX******************/
/**GM_atoui(),0xE13101XX**/
#define GM_RSSP1_ATOUI_PAR_ERR					0xE1310100U				/*GM_RSSP1_atoui()传入参数有误*/
#define GM_RSSP1_ATOUI_BUFSIZE_ZERO_ERR			0xE1310101U				/*GM_RSSP1_atoui()传入bufsize为0*/
#define GM_RSSP1_ATOUI_BUFSIZE_BITTOMAX_ERR		0xE1310102U				/*GM_RSSP1_atoui()传入bufsize位数超出最大值*/
#define GM_RSSP1_ATOUI_BUFSIZE_CHAR_ERR			0xE1310103U				/*当前字符不是数字*/
#define GM_RSSP1_ATOUI_BUFSIZE_RTTOMAX_ERR		0xE1310104U				/*传入buf大于"4294967295"*/
#define GM_RSSP1_ATOUI_BUFSIZE_2OO3_ERR			0xE1310105U				/*2OO3不支持使用*/

/**GM_atoi(),0xE13102XX**/
#define GM_RSSP1_ATOI_PAR_ERR						0xE1310200U				/*GM_RSSP1_atoi()传入参数有误*/
#define GM_RSSP1_ATOI_BUFSIZE_ZERO_ERR			0xE1310201U				/*GM_RSSP1_atoi()传入bufsize为0*/
#define GM_RSSP1_ATOI_BUFSIZE_ERR					0xE1310202U				/*GM_RSSP1_atoui()传入bufsize位数错误*/
#define GM_RSSP1_ATOI_BUFSIZE_CHAR_ERR			0xE1310203U				/*当前字符不是数字*/
#define GM_RSSP1_ATOI_BUFSIZE_RTTOMAX_ERR		0xE1310204U				/*传入buf大于最大值*/
#define GM_RSSP1_ATOI_BUFSIZE_2OO3_ERR			0xE1310205U				/*2OO3不支持使用*/


/*********************COMMEN部分,错误号为0xE141XXXX******************/
/**GM_RSSP1_Simple_Malloc(),0xE14101XX**/
#define GM_RSSP1_Simple_Malloc_PAR_ERR						0xE1410100U				/*GM_RSSP1_Simple_Malloc()传入参数有误*/
#define GM_RSSP1_Simple_Malloc_SIZE_ERR						0xE1410101U				/*GM_RSSP1_Simple_Malloc()空间已满*/

/**GM_RSSP1_Memory_Init(),0xE14102XX**/
#define GM_RSSP1_Memory_Init_PAR_ERR						0xE1410200U				/*GM_RSSP1_Memory_Init()传入参数有误*/
#define GM_RSSP1_Memory_Init_AGAIN_ERR						0xE1410201U				/*GM_RSSP1_Memory_Init()已经操作过*/


/**GM_RSSP1_memset(),0xE14103XX**/
#define GM_RSSP1_memset_PAR_ERR								0xE1410300U				/*GM_RSSP1_memset()传入参数有误*/


#endif
