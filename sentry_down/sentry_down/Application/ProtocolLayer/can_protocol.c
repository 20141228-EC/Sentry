#include "can_protocol.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

uint32_t TxMailbox;
uint8_t CAN1_200_TxData[8];
CAN_TxHeaderTypeDef TxMessage;

#define CHASSIS_SPEED_ID					0x001

// ---------------------- CAN1 接收底盘与超电 ---------------------- //
void CAN1_rxDataHandler(uint32_t rxId, uint8_t *rxBuf)
{
	switch (rxId)
	{
		case 0x201:
		{
			rm_motor_update(&rm_motor[CHAS_RF], rxBuf);
			rm_motor_check(&rm_motor[CHAS_RF]);
			break;
		}
		case 0x202:
		{
			rm_motor_update(&rm_motor[CHAS_LF], rxBuf);
			rm_motor_check(&rm_motor[CHAS_LF]);
			break;
		}
		case 0x203:
		{
			rm_motor_update(&rm_motor[CHAS_RB], rxBuf);
			rm_motor_check(&rm_motor[CHAS_RB]);
			break;
		}
		case 0x204:
		{
			rm_motor_update(&rm_motor[CHAS_LB], rxBuf);
			rm_motor_check(&rm_motor[CHAS_LB]);
			break;
		}
		case 0x211:
		{
			cap.update(&cap,rxBuf);
			break;
		}

		default:
			break;
	}
}

//	---------------------- CAN2 板间通信 ---------------------- //
void CAN2_rxDataHandler(uint32_t rxId, uint8_t *rxBuf)
{
	switch (rxId)
	{
		case CHASSIS_SPEED_ID:
		{
			master.update(&master, rxBuf);
			break;
		}
		
		default:
			break;
	}
}

// ---------------------- CAN 发送函数汇总 ---------------------- //
void cap_data_send(uint8_t can_num)
{
		switch(can_num)
		{
			case 1:
			{
				CAN1_SendData(cap.info.canId,cap.info.Buff0x222);
				break;
			}
			case 2:
			{
				CAN2_SendData(cap.info.canId,cap.info.Buff0x222);
				break;
			}
		}
}

uint8_t CAN1_Send_200_Motor(int16_t motor1,int16_t motor2,int16_t motor3,int16_t motor4)
{
	uint8_t cnt, res;
	
	TxMessage.StdId	=	0x200;
	TxMessage.IDE		=	CAN_ID_STD;
	TxMessage.RTR		=	CAN_RTR_DATA;
	TxMessage.DLC		=	8;	
		
	CAN1_200_TxData[0] = (uint8_t)(motor1>>8);
	CAN1_200_TxData[1] = (uint8_t) motor1;
	CAN1_200_TxData[2] = (uint8_t)(motor2>>8);
	CAN1_200_TxData[3] = (uint8_t) motor2;
	CAN1_200_TxData[4] = (uint8_t)(motor3>>8);
	CAN1_200_TxData[5] = (uint8_t) motor3;
	CAN1_200_TxData[6] = (uint8_t)(motor4>>8);
	CAN1_200_TxData[7] = (uint8_t) motor4;
	
	// don't send if every = 0
	for(cnt = 0;cnt < 8;cnt++)
	{
		if(CAN1_200_TxData[cnt] == 0)
			cnt++;
		if(cnt == 8)
			return 0;
	}
	
  res = HAL_CAN_AddTxMessage(&hcan1,&TxMessage,CAN1_200_TxData,&TxMailbox);
	
	memset(CAN1_200_TxData, 0, sizeof(uint8_t)*8);
		
	return res;
}
