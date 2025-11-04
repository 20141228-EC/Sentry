#include "can_protocol.h"
#include "communicate.h"

//CAN1
#define SLAVE_CTRL_FLAG_ID				0x002		
#define VISION_POS						0x003
#define NAV_SEND_UP_INFO 				0x006
#define NAV_SEND_LADAR_INFO				0x007

//CAN2
#define GAME_STATUS_ID 			0x003				//	比赛状态	剩余时间	剩余弹量	剩余金币	己方颜色
#define GAME_ROBOT_HP_ID 		0x004	            //	己方血量	敌方血量
#define SHOOT_DATA_ID 			0x005	                //	弹丸速度	
#define GIMBAL_OPERATE_ID		0x006	            //	云台操作数据
#define RADAR_POS_COOR_ID		0x101	            //	雷达标点坐标
#define GAME_ROBOT_STATUS_ID 	0x102	        //	机器状态

void bullet_speed_test(void);

uint8_t Cap_Buff0x2F[8], Cap_Buff0x2E[8];

void CAN_SendAll()
{
	
}

void CAN_SendAllZero()
{
	memset(CAN1_200_DATA, 0, sizeof(CAN1_200_DATA));
	
}

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
uint8_t test_buf[20];

/**
 *  @brief  CAN1 接收中断
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
		case VISION_POS:
		{
			
			memcpy(&communicate.communicate_rx_up_info->enemy_pos,rxBuf,4);
			Strategy.CarData->ad.Enemy_x = (float)communicate.communicate_rx_up_info->enemy_pos.enemy_x;
			Strategy.CarData->ad.Enemy_y = (float)communicate.communicate_rx_up_info->enemy_pos.enemy_y;
			
			break;
		}
		case SLAVE_CTRL_FLAG_ID:
		{
			get_up_info(&communicate, rxBuf);
			 communicate.communicate_chassis_info->cmd_info.bit.control_cmd = rxBuf[2];
			Strategy.CarData->ad.is_find_target = rxBuf[3];
			Strategy.CarData->ad.Enemy_num = rxBuf[4];
			//若使用遥控器拨杆暂时实现决策功能
			Strategy.CarData->sf.strategy_mode = rxBuf[7];
			
			break;
		}
		
		default:
			break;
	}
}


/**
 *  @brief  CAN2 接收中断
 */
void CAN2_rxDataHandler(uint32_t rxId, uint8_t *rxBuf)
{
	switch (rxId)
	{
		case 0x11:
		{
			dm_motor_update(&big_yaw,rxBuf);
			break;
		}
		
		// ------------- ??????????? ------------- //
		case GAME_STATUS_ID:
		case GAME_ROBOT_HP_ID:
		case SHOOT_DATA_ID:
		case RADAR_POS_COOR_ID:
		case GAME_ROBOT_STATUS_ID:
		case GIMBAL_OPERATE_ID:
			slave.update(&slave, rxId, rxBuf);
		break;
		
		default:
			break;
	}
}
