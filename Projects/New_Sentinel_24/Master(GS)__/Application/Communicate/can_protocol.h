#ifndef __CAN_PROTOCOL_H
#define __CAN_PROTOCOL_H

#include "stm32f4xx_hal.h"
#include "drv_can.h"


#define M2H_MODE_INIT 0
#define M2H_MODE_WORK 1

/*��̨���ݷ���*/
#define GIMBAL_DATA_ID 0xf1
#define MASTER_MODE_ID 0x200


/*���ݰ�����*/
#define SEND_L_DATA_ID 0x011
#define SEND_R_DATA_ID 0x012

/*ģʽ������*/
#define SEND_L_MODE_ID 0x021
#define SEND_R_MODE_ID 0x022

/*��̨����*/
#define FROM_L_HEAD_ID  0x2FF
#define FROM_R_HEAD_ID  0x1FF


#define MH_DRV_CAN_USE     (hcan2)  /*hcan1  hcan2*/



extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;


#define DEV_OFFLINE        0
#define DEV_ONLINE         1


#define M2H_OFFLINE_TIME_MAX   25

#define CHOKE_TIME_MAX   25

/** 
  * @brief  ͨѶ״̬ö��
  */ 
typedef enum 
{
	M2H_OFFLINE = 0,	
	
	M2H_ONLINE  = 1,
	

}M2H_work_state_e;

typedef enum 
{
	CHOKE_L     = 0,	
	
	CHOKE_R     = 0,	
	
	NO_CHOKE    = 1,
	

}gim_work_state_e;


typedef struct 
{
	uint8_t          offline_cnt;
	uint8_t          offline_cnt_max;
	M2H_work_state_e status;
	
	uint8_t          choke_time;
	gim_work_state_e gimbal_status;
	
}M2H_work_t;


typedef struct __packed 
{
	/*��̨�˶�*/
	int16_t  target_yaw;
	int16_t  target_pit;
	/*�������*/
	int16_t  shoot_speed;
	int16_t  frict_speed;
	
	/*ģʽ����*/
	uint8_t  gimbal_mode;
	uint8_t  shoot_mode;
	
}master_tx_data_t;



typedef struct __packed 
{
	/*ģʽ����*/
	uint8_t  gimbal_mode;
	uint8_t  shoot_mode;
	
	int16_t  measure_yaw;
	int16_t  measure_pit;
	
}master_rx_data_t;



typedef struct
{		
	master_tx_data_t     Send_L_Head;
	master_tx_data_t     Send_R_Head;
	
	master_rx_data_t     From_L_Head;	
	master_rx_data_t     From_R_Head;
	
	M2H_work_t           L_Head_status; 
	M2H_work_t           R_Head_status;
	
	
}Master_Head_t;


//���߼��
void Master_Head_HEART(Master_Head_t * pack);
//���ݰ���ʼ��
void Master_Head_INIT(Master_Head_t * pack);

//������Ϣ����
void M2H_SENT_DATA(Master_Head_t * pac);

//������Ϣ���ս���
void H2M_GETT_DATA( uint32_t id ,Master_Head_t * pack , uint8_t *rxBuf);



#endif

