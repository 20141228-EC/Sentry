#include "master.h"

host_rx_info_t host_rx_info = {
    .chassis_speed = {0},
    .ctrl_mode = 0,
    .move_mode = 0,
    .start_flag = 0,
};

host_tx_info_t host_tx_info;

host_info_t host_info = {
    .rx_info = &host_rx_info,
    .tx_info = &host_tx_info,
	.offline_cnt = 200,
    .offline_cnt_max = 200,
};

host_t host = {
    .work_state = DEV_OFFLINE,
    .info = &host_info,
    .get_chassis_info = Host_get_chassis_info,
    .get_ctrl_flag = Host_get_ctrl_flag,
};

extern CAN_HandleTypeDef hcan2;

void Host_get_chassis_info(host_t *host,uint8_t *rxBuf)
{
    host->info->rx_info->chassis_speed[0] = (int16_t)(rxBuf[0]<<8)|rxBuf[1];
    host->info->rx_info->chassis_speed[1] = (int16_t)(rxBuf[2]<<8)|rxBuf[3];
    host->info->rx_info->chassis_speed[2] = (int16_t)(rxBuf[4]<<8)|rxBuf[5];
    host->info->rx_info->chassis_speed[3] = (int16_t)(rxBuf[6]<<8)|rxBuf[7];
}

void Host_get_ctrl_flag(host_t *host,uint8_t *rxBuf)
{
	host->info->rx_info->ctrl_mode = rxBuf[0];
    host->info->rx_info->move_mode = rxBuf[1];
    host->info->rx_info->start_flag = rxBuf[2];
	host->info->rx_info->is_find_target = rxBuf[3];
}

#define GAME_STATUS_ID 				0x003
#define GAME_ROBOT_HP_ID 			0x004
#define POWER_HEAT_ID 				0x101
#define GAME_ROBOT_STATUS_ID 	0x102
#define SHOOT_DATA_ID 				0x103
#define ROBOT_HURT_ID 				0x107
#define ROBOT_POS_ID					0x005

/* Private variables ---------------------------------------------------------*/
extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

/* Exported functions --------------------------------------------------------*/
void game_status_send(uint8_t can_num)
{
	static uint8_t game_status_tx_buf[8];

	memcpy(game_status_tx_buf, &judge.info->game_status, sizeof(game_status_tx_buf));
	switch (can_num)
	{
		case 1:
            CAN_SendData(&hcan1, GAME_STATUS_ID, game_status_tx_buf);
			break;
		case 2:
			CAN_SendData(&hcan2, GAME_STATUS_ID, game_status_tx_buf);
			break;
		default:
			break;
	}
}	

void game_robot_HP_send(uint8_t can_num)
{
	static uint8_t game_robot_HP_tx_buf[8];

	/* 红方 */
	if (judge.info->my_color == 0)
	{
		/* 向上取整 */
		game_robot_HP_tx_buf[0] = (judge.info->game_robot_HP.blue_1_robot_HP + 24) / 25;
		game_robot_HP_tx_buf[1] = (judge.info->game_robot_HP.blue_2_robot_HP + 24) / 25;
		game_robot_HP_tx_buf[2] = (judge.info->game_robot_HP.blue_3_robot_HP + 24) / 25;
		game_robot_HP_tx_buf[3] = (judge.info->game_robot_HP.blue_4_robot_HP + 24) / 25;
		game_robot_HP_tx_buf[4] = (judge.info->game_robot_HP.blue_5_robot_HP + 24) / 25;
		game_robot_HP_tx_buf[5] = (judge.info->game_robot_HP.blue_7_robot_HP + 24) / 25;
		game_robot_HP_tx_buf[6] = (judge.info->game_robot_HP.blue_outpost_HP + 24) / 25;
		game_robot_HP_tx_buf[7] = (judge.info->game_robot_HP.blue_base_HP + 24) / 25;
	}
	/* 蓝方 */
	if (judge.info->my_color == 1)
	{
		/* 向上取整 */
		game_robot_HP_tx_buf[0] = (judge.info->game_robot_HP.red_1_robot_HP + 24) / 25;
		game_robot_HP_tx_buf[1] = (judge.info->game_robot_HP.red_2_robot_HP + 24) / 25;
		game_robot_HP_tx_buf[2] = (judge.info->game_robot_HP.red_3_robot_HP + 24) / 25;
		game_robot_HP_tx_buf[3] = (judge.info->game_robot_HP.red_4_robot_HP + 24) / 25;
		game_robot_HP_tx_buf[4] = (judge.info->game_robot_HP.red_5_robot_HP + 24) / 25;
		game_robot_HP_tx_buf[5] = (judge.info->game_robot_HP.red_7_robot_HP + 24) / 25;
		game_robot_HP_tx_buf[6] = (judge.info->game_robot_HP.red_outpost_HP + 24) / 25;
		game_robot_HP_tx_buf[7] = (judge.info->game_robot_HP.red_base_HP + 24) / 25;
	}
	
	switch (can_num)
	{
		case 1:
			CAN_SendData(&hcan1, GAME_ROBOT_HP_ID, game_robot_HP_tx_buf);
			break;
		case 2:
			CAN_SendData(&hcan2, GAME_ROBOT_HP_ID, game_robot_HP_tx_buf);
			break;
		default:
			break;
	}
	memset(game_robot_HP_tx_buf, 0, 8);
}	

uint8_t power_heat_tx_buf[8];
void power_heat_send(uint8_t can_num)
{
					

	// 0    1
	memcpy(&power_heat_tx_buf, (void*)&judge.info->power_heat_data.shooter_id1_17mm_cooling_heat, 2);
//	memcpy(&power_heat_tx_buf[0], (void*)&judge.info->power_heat_data.chassis_power,4);
//	memcpy(&power_heat_tx_buf[4], (void*)&yuce_power,4);
	switch (can_num)
	{
		case 1:
			CAN_SendData(&hcan1, POWER_HEAT_ID, power_heat_tx_buf);
			break;
		case 2:
            CAN_SendData(&hcan2, POWER_HEAT_ID, power_heat_tx_buf);
			break;
		default:
			break;
	}
	memset(power_heat_tx_buf, 0, 2);
}

void game_robot_status_send(uint8_t can_num)
{
	static uint8_t game_robot_status_tx_buf[8];
	
	game_robot_status_tx_buf[0] = judge.info->my_color;

	memcpy(&game_robot_status_tx_buf[1], (void*)&judge.info->game_robot_status.shooter_barrel_heat_limit, 2);
	memcpy(&game_robot_status_tx_buf[3], (void*)&judge.info->game_robot_status.shooter_barrel_cooling_value, 2);
	memcpy(&game_robot_status_tx_buf[5], (void*)&Strategy.vision->mode.detect_mode,1);
		
	switch (can_num)
	{
		case 1:
			CAN_SendData(&hcan1, GAME_ROBOT_STATUS_ID, game_robot_status_tx_buf);
			break;
		case 2:
			CAN_SendData(&hcan2, GAME_ROBOT_STATUS_ID, game_robot_status_tx_buf);
			break;
		default:
			break;
	}
	memset(game_robot_status_tx_buf, 0, 8);
}

void robot_hurt_send(uint8_t can_num)
{
	static uint8_t robot_hurt_tx_buf[8];
	
	if (judge.info->robot_hurt.hurt_type == 0)
	{
		robot_hurt_tx_buf[0] = judge.info->robot_hurt.armor_id;
		
		switch (can_num)
		{
			case 1:
				CAN_SendData(&hcan1, ROBOT_HURT_ID, robot_hurt_tx_buf);
				break;
			case 2:
				CAN_SendData(&hcan2, ROBOT_HURT_ID, robot_hurt_tx_buf);
				break;
			default:
				break;
		}
		
		memset(robot_hurt_tx_buf, 0, 8);
	}
}

void shoot_data_send(uint8_t can_num)
{
	static uint8_t shoot_data_tx_buf[8];

	memcpy(shoot_data_tx_buf, (void*)&judge.info->shoot_data.bullet_speed, 4);
	memcpy(&shoot_data_tx_buf[4], (void*)&judge.info->shoot_data.bullet_freq, 1);
	
	switch (can_num)
	{
		case 1:
			CAN_SendData(&hcan1, SHOOT_DATA_ID, shoot_data_tx_buf);
			break;
		case 2:
			CAN_SendData(&hcan2, SHOOT_DATA_ID, shoot_data_tx_buf);
			break;
		default:
			break;
	}
	
	memset(shoot_data_tx_buf, 0, 8);
}

void robot_pos_send(uint8_t can_num)
{
	static uint8_t robot_pos_tx_buf[8];
	
	memcpy(robot_pos_tx_buf,(void*)&judge.info->game_robot_pos,8);
	
	switch (can_num)
	{
		case 1:
		{
			CAN_SendData(&hcan1,ROBOT_POS_ID,robot_pos_tx_buf);
		}
		case 2:
		{
			CAN_SendData(&hcan2,ROBOT_POS_ID,robot_pos_tx_buf);

		}
	}
	
}
