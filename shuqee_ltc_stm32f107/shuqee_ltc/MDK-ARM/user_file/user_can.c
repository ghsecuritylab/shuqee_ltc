#include "user_can.h"
#include "user_config.h"
#include "user_time.h"
#include "user_uart.h"
#include <string.h>
 
CanTxMsgTypeDef txmessage;
CanRxMsgTypeDef rxmessage;
extern CAN_HandleTypeDef hcan1;

void can_txmsg_config()
{
	hcan1.pTxMsg=&txmessage; 
	hcan1.pTxMsg->StdId=0x123;      //���û���ID��
	hcan1.pTxMsg->IDE=CAN_ID_STD;   //����Ϊ��׼��ʽ��
	hcan1.pTxMsg->RTR=CAN_RTR_DATA; //����Ϊ����֡��
	hcan1.pTxMsg->DLC=8;            //�������ݳ���Ϊ8���ֽڣ�
	hcan1.pTxMsg->Data[0]='C';
	hcan1.pTxMsg->Data[1]='A';
	hcan1.pTxMsg->Data[2]='N';
	hcan1.pTxMsg->Data[3]=' ';
	hcan1.pTxMsg->Data[4]='T';
	hcan1.pTxMsg->Data[5]='E';
	hcan1.pTxMsg->Data[6]='S';
	hcan1.pTxMsg->Data[7]='T';
}

void can_rxmsg_config()
{
	CAN_FilterConfTypeDef sfilterconfig;
	hcan1.pRxMsg = &rxmessage;
	sfilterconfig.BankNumber=14;
	sfilterconfig.FilterActivation=ENABLE;
	sfilterconfig.FilterFIFOAssignment=0;
	sfilterconfig.FilterIdHigh=(HEART_BEAT<<5);
	sfilterconfig.FilterIdLow=0x0000;
	sfilterconfig.FilterMaskIdHigh=(HEART_BEAT<<5);
	sfilterconfig.FilterMaskIdLow=0x0000;
	sfilterconfig.FilterMode=CAN_FILTERMODE_IDMASK;
	sfilterconfig.FilterNumber=0;
	sfilterconfig.FilterScale=CAN_FILTERSCALE_32BIT;

	HAL_CAN_ConfigFilter(&hcan1,&sfilterconfig);	
}	

/*32λ�������ģʽ*/
void can_scale32_idmask(void)  
{  
	uint16_t      mask,num,tmp,i;  
  CAN_FilterConfTypeDef  sfilterconfig;
	hcan1.pRxMsg = &rxmessage;
  uint32_t stdid_array[SEAT_AMOUNT]={STATUS_MSG_ID,NM_MSG_ID};  
//	for(i=0;i<SEAT_AMOUNT;i++)
//	{
//		stdid_array[i]=HEART_BEAT+i;
//	}	
     
  sfilterconfig.FilterNumber = 1;               //ʹ�ù�����0  
  sfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;     //����Ϊ����ģʽ  
  sfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;    //����Ϊ32λ��  
  sfilterconfig.FilterIdHigh =(stdid_array[0]<<5);     //��֤���������ΪStdIdArray[]����������һ��������ʹ��StdIdArray[0]��Ϊ��֤��  
  sfilterconfig.FilterIdLow =0;  
    
  mask =0x7ff;                      //���濪ʼ����������  
  num =sizeof(stdid_array)/sizeof(stdid_array[0]);  
  for(i =0; i<num; i++)      //������λStdIdArray[]���������г�Ա��ͬ����  
  {  
    tmp =stdid_array[i] ^ (~stdid_array[0]);  //���������Ա���0����Ա����ͬ�����  
    mask &=tmp;  
  }  
  sfilterconfig.FilterMaskIdHigh =(mask<<5);  
  sfilterconfig.FilterMaskIdLow =0|0x02;        //ֻ��������֡  
    
  sfilterconfig.FilterFIFOAssignment = 0;       //����ͨ��������֡���뵽FIFO0��  
  sfilterconfig.FilterActivation = ENABLE;  
  sfilterconfig.BankNumber = 14;  
    
  if(HAL_CAN_ConfigFilter(&hcan1, &sfilterconfig) != HAL_OK)  
  {  
    Error_Handler();  
  }  
} 


void user_can_init(void)
{ 
	can_txmsg_config();
	can_rxmsg_config();
  can_scale32_idmask();  	
	CAN1->MCR|=(1<<6);
	HAL_CAN_Receive_IT(&hcan1,CAN_FIFO0);
}

/**
  * @brief  Transmits a CAN frame message.
  * @param  dest_addr: pointer to which dest_adrr  
  * @param  *data: Pointer to data buffer  
  * @param  len: Amount of data to be send  ,the rang of  0-8;
  * @retval HAL status
  */

void can_send(uint16_t msg_id, uint8_t *data, uint16_t len)
{  
	uint8_t i;
	if(msg_id>=0x7ff)
	{
		msg_id=0x7ff;
	}	
	if(len>=8)
	{
		len=8;
	}	
	hcan1.pTxMsg->StdId=msg_id; /*����Ҫ�������ݵ�Ŀ���ַ*/
	hcan1.pTxMsg->DLC=len;
	for(i=0;i<len;i++)
	{
		hcan1.pTxMsg->Data[i]=data[i];
	}
	if(HAL_CAN_Transmit(&hcan1, 1)!=HAL_OK)
	{
		; /* do nothing */
	} 
	CAN1->IER|=(1<<1);   //��ֹ�Ͽ���IER^2λ��λ��
}

void HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{  
	if(hcan->Instance==CAN1)
	{	
		set_can_rx_flag(hcan->pRxMsg->StdId); 
	}

	CAN1->IER|=(1<<1);
	HAL_CAN_Receive_IT(&hcan1,CAN_FIFO0);
}



#pragma pack(1)

typedef struct buscan_control_pack
{	
	uint8_t high[3];
	uint8_t sp_seat_env_id[3];
  uint8_t speed[3];
} buscan_control_pack_t;

#pragma pack()
buscan_control_pack_t pack_can ;
static uint8_t mark_cantx;

void buscan_control(uint8_t *high, uint8_t sp_seat, uint8_t sp_env,uint8_t *speed, uint8_t seat_id) 
{
	memcpy(pack_can.high,high,sizeof(pack_can.high));
	//memcpy(pack_can.speed,speed,sizeof(pack_can.speed));
	pack_can.sp_seat_env_id[0]=sp_env;
	pack_can.sp_seat_env_id[1]=sp_seat;
	pack_can.sp_seat_env_id[2]=seat_id;
	can_send(HIGHT_MSG_ID,pack_can.high,8);    //�ȷ�	HIGHT_MSG_ID=0x100,  //�߶�ID
//	switch(mark_cantx)
//	{
//		case 0: mark_cantx++;
//						can_send(HIGHT_MSG_ID,pack_can.high,8);    //�ȷ�	HIGHT_MSG_ID=0x100,  //�߶�ID
//						break;
//		case 1: mark_cantx++;
////						can_send(SPEED_MSG_ID,pack_can.speed, 8)	;			//SPEED_MSG_ID  �ٶ�ID
//						break;
//		case 2: mark_cantx=0;
//						can_send(SP_MSG_ID,pack_can.sp_seat_env_id,8);     //SP_MSG_ID				  //��ЧID
//						break;
//	}			
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef* hcan)
{

}	
