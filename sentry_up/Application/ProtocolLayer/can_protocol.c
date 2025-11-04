#include "can_protocol.h"

void bullet_speed_test(void);

uint8_t Cap_Buff0x2F[8], Cap_Buff0x2E[8];

uint8_t k;

//发送所有电机数据
void CAN_SendAll(void)
{
	k++;
	
	if(k%5==0)
	{
		CAN1_CMD_1FF();
	}
		
		
	
	if(k% 20 == 0)
	{
		CAN1_CMD_200();
	}
	
	
	CAN2_CMD_200();
	
}

//清空发送电机数据
void CAN_SendAllZero()
{
	memset(CAN1_200_DATA, 0, sizeof(CAN1_200_DATA));
	memset(CAN1_1FF_DATA, 0, sizeof(CAN1_1FF_DATA));
	memset(CAN2_200_DATA, 0, sizeof(CAN2_200_DATA));
	
	CAN_SendAll();
}

/**
 *  @brief  CAN1 接收数据
 */
void CAN1_rxDataHandler(uint32_t rxId, uint8_t *rxBuf)
{
	switch (rxId)
	{
		case 0x201:
		{
			rm_motor_update(&rm_motor[FRIC_L], rxBuf);
			rm_motor_check(&rm_motor[FRIC_L]);
			break;
		}
		case 0x202:
		{
			rm_motor_update(&rm_motor[FRIC_R], rxBuf);
			rm_motor_check(&rm_motor[FRIC_R]);
			break;
		}
		case 0x205:
		{
			rm_motor_update(&rm_motor[GIMB_P], rxBuf);
			rm_motor_check(&rm_motor[GIMB_P]);
			break;
		}
		case 0x206:
		{
			rm_motor_update(&rm_motor[GIMB_Y], rxBuf);
			rm_motor_check(&rm_motor[GIMB_Y]);
			break;
		}
		/*雷达自身定位信息（x,y,yaw）,转发视觉*/
		case NAV_SEND_LADAR_INFO:
		{
			memcpy(&slave.info->rx_info->mid_info->ladar_info,rxBuf,8);
			break;
		}
		/*中主控转发上主控信息（云台模式，以及指定角度状态下的yaw）*/
		case NAV_SEND_UP_INFO:
		{
			memcpy(&slave.info->rx_info->mid_info->gimbal_info,rxBuf,8);
			break;
		}
		
		default:
			break;
	}
}

/**
 *  @brief  CAN2 接收数据
 */
void CAN2_rxDataHandler(uint32_t rxId, uint8_t *rxBuf)
{
	switch (rxId)
	{
		
		case 0x203:
		{
			rm_motor_update(&rm_motor[DIAL], rxBuf);
			rm_motor_check(&rm_motor[DIAL]);
			break;
		}
		
		case 0x11:
		{
			dm_motor_update(&big_yaw,rxBuf);
			break;
		}
		
		/*裁判系统信息*/
		case GAME_STATUS_ID:
		case GAME_ROBOT_HP_ID:
		case SHOOT_DATA_ID:
		case RADAR_POS_COOR_ID:
		case GAME_ROBOT_STATUS_ID:
			slave.update(&slave, rxId, rxBuf);
		break;
		
		default:
			break;
	}
}
