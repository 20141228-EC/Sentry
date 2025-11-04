/*
 * @Author: lion
 * @Date: 2025-04-16 20:29:46
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-07-29 01:11:29
 * @FilePath: \sentry_up\Application\DeviceLayer\Slave.c
 * @Description: 
 * 
 */
#include "Slave.h"

extern CAN_HandleTypeDef hcan2;
extern CAN_HandleTypeDef hcan1;

static void Slave_Init(slave_t *self);
static void Slave_Heartbeat(slave_t *self);
static void Slave_Update(slave_t *self, uint32_t rx_id, uint8_t *rxBuf);

slave_tx_chassis_info_t slave_tx_chassis_info = {
    .cmd_info.cmd = 0,
    .power_target = 100,
    .w = 0,
    .vx = 0,
    .vy = 0,
    .cmd_info.bit.control_cmd = 0,
    .cmd_info.bit.cap_switch = 0,
    .cmd_info.bit.wheel_flag = 0,
    .cmd_info.bit.unuse = 0,
	
	
};
enemy_pos_info_t enemy_pos_info;

slave_tx_info_t slave_tx_info = {
    .ctrl_flag = {0},
    .chassis_info = &slave_tx_chassis_info,
	.enemy_pos_info = &enemy_pos_info,
};

slave_rx_mid_info_t slave_rx_mid_info;

slave_rx_down_info_t    slave_rx_down_info;

slave_rx_info_t slave_rx_info = {
    .mid_info = &slave_rx_mid_info,
    .down_info = &slave_rx_down_info,
};

slave_info_t slave_info = {
    .tx_info = &slave_tx_info,
    .rx_info = &slave_rx_info,
};

slave_t slave = {
    .work_state = DEV_OFFLINE,
    .info = &slave_info,

    .send_chassis_info = Slave_send_chassis_info,
    .get_ctrl_mode = Slave_get_ctrl_mode,
    .get_move_mode = Slave_get_move_mode,
    .send_flag = Slave_send_flag,
    .get_start = Slave_get_start,
	.get_vision_status = Slave_get_vision_status,
	.send_vision_pos = Slave_send_enemy_pos,
	
	.init		= Slave_Init,
	.update		= Slave_Update,
	.heartbeat	= Slave_Heartbeat,
};

extern CAN_HandleTypeDef hcan2;

void Slave_send_chassis_info(slave_t *slave)
{
	uint8_t rc_send_chassis_info[8];
	
	memcpy(rc_send_chassis_info,slave->info->tx_info->chassis_info,8);
    CAN_TxHeaderTypeDef tx_message;
	
	uint32_t send_mail_box;
	tx_message.StdId = CHASSIS_SPEED_ID;
	tx_message.IDE = CAN_ID_STD;
	tx_message.RTR = CAN_RTR_DATA;
	tx_message.DLC = 0x08;

	HAL_CAN_AddTxMessage(&hcan2, &tx_message, rc_send_chassis_info, &send_mail_box);
}

void Slave_get_ctrl_mode(slave_t *slave,uint8_t ctrl_mode)
{
	slave->info->tx_info->ctrl_flag[0] = ctrl_mode;
}

void Slave_get_move_mode(slave_t *slave,uint8_t move_mode)
{
    slave->info->tx_info->ctrl_flag[1] = move_mode;
}

void Slave_get_start(slave_t *slave,uint8_t start)
{
    slave->info->tx_info->ctrl_flag[2] = start;
}

void Slave_get_vision_status(slave_t *slave,uint8_t vision_status,uint8_t enemy_num)
{
    slave->info->tx_info->ctrl_flag[3] = vision_status;
	slave->info->tx_info->ctrl_flag[4] = enemy_num;
}

void Slave_send_enemy_pos(slave_t *slave)
{
	CAN_TxHeaderTypeDef tx_message;
	uint8_t send_buf[4] = {0};
	
	uint32_t send_mail_box;
	tx_message.StdId = VISION_ENEMY_POS;
	tx_message.IDE = CAN_ID_STD;
	tx_message.RTR = CAN_RTR_DATA;
	tx_message.DLC = 0x08;
	memcpy(send_buf,slave->info->tx_info->enemy_pos_info,4);

	HAL_CAN_AddTxMessage(&hcan1, &tx_message, send_buf, &send_mail_box);
}
uint16_t imu_yaw_to_mech = 0;
void Slave_send_flag(slave_t *slave)
{
	CAN_TxHeaderTypeDef tx_message;
	imu_yaw_to_mech = imu_sensor.info->base_info.yaw * 22.7555556f + 4096;
	if(imu_yaw_to_mech >8192)
	{
		imu_yaw_to_mech-=8192;
	}
	memcpy(&slave->info->tx_info->ctrl_flag[5],&imu_yaw_to_mech,2);
	
	
	uint32_t send_mail_box;
	tx_message.StdId = SLAVE_CTRL_FLAG_ID;
	tx_message.IDE = CAN_ID_STD;
	tx_message.RTR = CAN_RTR_DATA;
	tx_message.DLC = 0x08;

	HAL_CAN_AddTxMessage(&hcan1, &tx_message, slave->info->tx_info->ctrl_flag, &send_mail_box);
}

//void Slave_send_imu_info(slave_t *slave)
//{
//	uint8_t	up_yaw_info[2];
//	memcpy(up_yaw_info,&slave->info->tx_info->up_yaw_info,2);
//	CAN_TxHeaderTypeDef tx_message;
//	
//	uint32_t send_mail_box;
//	tx_message.StdId = UP_YAW_INFO;
//	tx_message.IDE = CAN_ID_STD;
//	tx_message.RTR = CAN_RTR_DATA;
//	tx_message.DLC = 0x02;

//	HAL_CAN_AddTxMessage(&hcan1, &tx_message, up_yaw_info, &send_mail_box);
//}


void Slave_Init(slave_t *self)
{
	self->info->rx_info->remain_bullet = 0;
	self->info->rx_info->remain_time = 0;
	self->info->rx_info->my_color = 0;
	self->info->rx_info->game_progress = 0;
	self->info->rx_info->remain_gold = 0;
	self->info->rx_info->remain_bullet = 0;
	
	self->info->rx_info->ab_hero_HP = 0;
	self->info->rx_info->ab_engineer_HP = 0;
	self->info->rx_info->ab_infantry3_HP = 0;
	self->info->rx_info->ab_infantry4_HP = 0;
	self->info->rx_info->ab_sentry_HP = 0;
	self->info->rx_info->ab_outpost_HP = 0;
	self->info->rx_info->ab_base_HP = 0;
	
	self->info->rx_info->eb_hero_HP = 0;
	self->info->rx_info->eb_engineer_HP = 0;
	self->info->rx_info->eb_infantry3_HP = 0;
	self->info->rx_info->eb_infantry4_HP = 0;
	self->info->rx_info->eb_sentry_HP = 0;
	self->info->rx_info->eb_outpost_HP = 0;
	self->info->rx_info->eb_base_HP = 0;	
}

void Slave_Heartbeat(slave_t *self)
{
	slave_info_t *info = self->info;
	
	info->offline_cnt++;
	
	if(info->offline_cnt > info->offline_cnt_max)
	{
		info->offline_cnt = info->offline_cnt_max;
		self->work_state = DEV_OFFLINE;
	}
	else if(self->work_state == DEV_OFFLINE)
		self->work_state = DEV_ONLINE;						/* 离线->在线 */
}
uint32_t	speed_cnt;
uint32_t 	shoot_cnt;
void Slave_Update(slave_t *self, uint32_t rx_id, uint8_t *rxBuf)
{
	slave_info_t *info = self->info;
	
	
	switch(rx_id)
	{
		case GAME_STATUS_ID:
			memcpy(&info->rx_info->down_info->game_status, rxBuf, 8);
			
			info->rx_info->my_color			= info->rx_info->down_info->game_status.my_color;
			info->rx_info->game_progress	= info->rx_info->down_info->game_status.game_progress;
			info->rx_info->cap_remain		=	info->rx_info->down_info->game_status.cap_remain;
			info->rx_info->remain_time		= info->rx_info->down_info->game_status.remain_time;
			info->rx_info->remain_gold		= info->rx_info->down_info->game_status.remain_gold;
			info->rx_info->remain_bullet = info->rx_info->down_info->game_status.remain_bullet;
		break;

		case GAME_ROBOT_HP_ID:
			memcpy(&info->rx_info->down_info->game_robot_HP, rxBuf, 8);
			
			if(info->rx_info->down_info->game_robot_HP.HP_status == 0)		// 己方血�?
			{
				info->rx_info->ab_hero_HP		= info->rx_info->down_info->game_robot_HP.hero_HP / 255.f * 500.f;		// 英雄血�?		0-500
				info->rx_info->ab_engineer_HP	= info->rx_info->down_info->game_robot_HP.engineer_HP / 255.f * 250.f;	// 工程血�?		0-250
				info->rx_info->ab_infantry3_HP 	= info->rx_info->down_info->game_robot_HP.infantry3_HP / 255.f * 500.f;	// 步兵3血�?	0-500
				info->rx_info->ab_infantry4_HP 	= info->rx_info->down_info->game_robot_HP.infantry4_HP / 255.f * 500.f;	// 步兵4血�?	0-500
				info->rx_info->ab_sentry_HP		= info->rx_info->down_info->game_robot_HP.sentry_HP / 255.f * 400.f;	// 哨兵血�?		0-400
				info->rx_info->ab_outpost_HP	= info->rx_info->down_info->game_robot_HP.outpost_HP / 255.f * 1500.f;	// 前哨血�?		0-1500
				info->rx_info->ab_base_HP		= info->rx_info->down_info->game_robot_HP.base_HP / 255.f * 5000.f;		// 基地血�?		0-5000
			}
			else if(info->rx_info->down_info->game_robot_HP.HP_status == 1)	// 敌方血�?
			{
				info->rx_info->eb_hero_HP 		= info->rx_info->down_info->game_robot_HP.hero_HP / 255.f * 500.f;		// 英雄血�?		0-500
				info->rx_info->eb_engineer_HP	= info->rx_info->down_info->game_robot_HP.engineer_HP / 255.f * 250.f;	// 工程血�?		0-250
				info->rx_info->eb_infantry3_HP 	= info->rx_info->down_info->game_robot_HP.infantry3_HP / 255.f * 500.f;	// 步兵3血�?	0-500
				info->rx_info->eb_infantry4_HP 	= info->rx_info->down_info->game_robot_HP.infantry4_HP / 255.f * 500.f;	// 步兵4血�?	0-500
				info->rx_info->eb_sentry_HP		= info->rx_info->down_info->game_robot_HP.sentry_HP / 255.f * 400.f;	// 哨兵血�?		0-400
				info->rx_info->eb_outpost_HP	= info->rx_info->down_info->game_robot_HP.outpost_HP / 255.f * 1500.f;	// 前哨血�?		0-1500
				info->rx_info->eb_base_HP		= info->rx_info->down_info->game_robot_HP.base_HP / 255.f * 5000.f;		// 基地血�?		0-5000
			}
			break;

		case RADAR_POS_COOR_ID:
			memcpy(&info->rx_info->down_info->radar_pos_coor, rxBuf, 8);

			if(info->rx_info->down_info->radar_pos_coor.pack_id == 0x01)
			{
				info->rx_info->hero_x			= info->rx_info->down_info->radar_pos_coor.data1;
				info->rx_info->hero_y			= info->rx_info->down_info->radar_pos_coor.data2;
				info->rx_info->engineer_x	= info->rx_info->down_info->radar_pos_coor.data3;
				info->rx_info->engineer_y	= info->rx_info->down_info->radar_pos_coor.data4;
				info->rx_info->infantry3_x = info->rx_info->down_info->radar_pos_coor.data5;
			}
			else if(info->rx_info->down_info->radar_pos_coor.pack_id == 0x05)
			{
				info->rx_info->infantry3_y = info->rx_info->down_info->radar_pos_coor.data1;
				info->rx_info->infantry4_x = info->rx_info->down_info->radar_pos_coor.data2;
				info->rx_info->infantry4_y = info->rx_info->down_info->radar_pos_coor.data3;
				info->rx_info->sentry_x		= info->rx_info->down_info->radar_pos_coor.data4;
				info->rx_info->sentry_y		= info->rx_info->down_info->radar_pos_coor.data5;
			}
		break;

		case GAME_ROBOT_STATUS_ID:
			memcpy(&info->rx_info->down_info->game_robot_status, rxBuf, 8);
			
			info->rx_info->shooter_barrel_cooling_value = info->rx_info->down_info->game_robot_status.shooter_barrel_cooling_value;
			info->rx_info->shooter_barrel_heat			= info->rx_info->down_info->game_robot_status.shooter_barrel_heat;
			info->rx_info->shooter_barrel_heat_limit	= info->rx_info->down_info->game_robot_status.shooter_barrel_heat_limit;
		break;
		
		case SHOOT_DATA_ID:
			shoot_cnt++;
			memcpy(&info->rx_info->bullet_speed, rxBuf, 4);
			speed_cnt--;
		if(speed_cnt>65535)
		{
			speed_cnt = 0;
		}
		break;
		
		default:
			break;
	}
}

