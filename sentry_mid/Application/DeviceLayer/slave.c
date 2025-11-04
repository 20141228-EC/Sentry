#include "slave.h"

static void Slave_Init(slave_t *self);
static void Slave_Heartbeat(slave_t *self);
static void Slave_Update(slave_t *self, uint32_t rx_id, uint8_t *rxBuf);

static float int16_to_float(int16_t a, int16_t a_max, int16_t a_min, float b_max, float b_min);

slave_rx_info_t slave_rx_info;
slave_info_t slave_info = {
	.rx = &slave_rx_info,
};

slave_t slave = {
	.info	=	&slave_info,
	
	.init		= Slave_Init,
	.update		= Slave_Update,
	.heartbeat	= Slave_Heartbeat,
};

void Slave_Init(slave_t *self)
{
	self->info->remain_time = 0;
	self->info->my_color = 0;
	self->info->game_progress = 0;
	self->info->remain_gold = 0;
	self->info->remain_bullet = 0;
	
	self->info->ab_hero_HP = 0;
	self->info->ab_engineer_HP = 0;
	self->info->ab_infantry3_HP = 0;
	self->info->ab_infantry4_HP = 0;
	self->info->ab_sentry_HP = 0;
	self->info->ab_outpost_HP = 0;
	self->info->ab_base_HP = 0;
	
	self->info->eb_hero_HP = 0;
	self->info->eb_engineer_HP = 0;
	self->info->eb_infantry3_HP = 0;
	self->info->eb_infantry4_HP = 0;
	self->info->eb_sentry_HP = 0;
	self->info->eb_outpost_HP = 0;
	self->info->eb_base_HP = 0;	
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

void Slave_Update(slave_t *self, uint32_t rx_id, uint8_t *rxBuf)
{
	slave_info_t *info = self->info;
	
	info->offline_cnt = 0;
	
	switch(rx_id)
	{
		case GAME_STATUS_ID:
			memcpy(&info->rx->game_status, rxBuf, 8);
			
			info->my_color			= info->rx->game_status.my_color;
			info->game_progress	= info->rx->game_status.game_progress;
			info->cap_remain		=	info->rx->game_status.cap_remain;
			info->remain_time		= info->rx->game_status.remain_time;
			info->remain_gold		= info->rx->game_status.remain_gold;
			info->remain_bullet = info->rx->game_status.remain_bullet;
		
		break;

		case GAME_ROBOT_HP_ID:
			memcpy(&info->rx->game_robot_HP, rxBuf, 8);
			
			if(info->rx->game_robot_HP.HP_status == 0)		// 己方血�?
			{
				info->ab_hero_HP		= info->rx->game_robot_HP.hero_HP / 255.f * 500.f;		// 英雄血�?		0-500
				info->ab_engineer_HP	= info->rx->game_robot_HP.engineer_HP / 255.f * 250.f;	// 工程血�?		0-250
				info->ab_infantry3_HP 	= info->rx->game_robot_HP.infantry3_HP / 255.f * 500.f;	// 步兵3血�?	0-500
				info->ab_infantry4_HP 	= info->rx->game_robot_HP.infantry4_HP / 255.f * 500.f;	// 步兵4血�?	0-500
				info->ab_sentry_HP		= info->rx->game_robot_HP.sentry_HP / 255.f * 400.f;	// 哨兵血�?		0-400
				info->ab_outpost_HP		= info->rx->game_robot_HP.outpost_HP / 255.f * 1500.f;	// 前哨血�?		0-1500
				info->ab_base_HP		= info->rx->game_robot_HP.base_HP / 255.f * 5000.f;		// 基地血�?		0-5000
			}
			else if(info->rx->game_robot_HP.HP_status == 1)	// 敌方血�?
			{
				info->eb_hero_HP 		= info->rx->game_robot_HP.hero_HP / 255.f * 500.f;		// 英雄血�?		0-500
				info->eb_engineer_HP	= info->rx->game_robot_HP.engineer_HP / 255.f * 250.f;	// 工程血�?		0-250
				info->eb_infantry3_HP 	= info->rx->game_robot_HP.infantry3_HP / 255.f * 500.f;	// 步兵3血�?	0-500
				info->eb_infantry4_HP 	= info->rx->game_robot_HP.infantry4_HP / 255.f * 500.f;	// 步兵4血�?	0-500
				info->eb_sentry_HP		= info->rx->game_robot_HP.sentry_HP / 255.f * 400.f;	// 哨兵血�?		0-400
				info->eb_outpost_HP		= info->rx->game_robot_HP.outpost_HP / 255.f * 1500.f;	// 前哨血�?		0-1500
				info->eb_base_HP		= info->rx->game_robot_HP.base_HP / 255.f * 5000.f;		// 基地血�?		0-5000
			}
			break;

		case GIMBAL_OPERATE_ID:
		{
			memcpy(&info->rx->gimbal_operate,rxBuf,8);
			info->operate_x = info->rx->gimbal_operate.target_x;
			info->operate_y = info->rx->gimbal_operate.target_y;
			break;
		}

		case RADAR_POS_COOR_ID:
			memcpy(&info->rx->radar_pos_coor, rxBuf, 8);

			if(info->rx->radar_pos_coor.pack_id == 0x01)
			{
				info->hero_x			= info->rx->radar_pos_coor.data1;
				info->hero_y			= info->rx->radar_pos_coor.data2;
				info->engineer_x	= info->rx->radar_pos_coor.data3;
				info->engineer_y	= info->rx->radar_pos_coor.data4;
				info->infantry3_x = info->rx->radar_pos_coor.data5;
			}
			else if(info->rx->radar_pos_coor.pack_id == 0x05)
			{
				info->infantry3_y = info->rx->radar_pos_coor.data1;
				info->infantry4_x = info->rx->radar_pos_coor.data2;
				info->infantry4_y = info->rx->radar_pos_coor.data3;
				info->sentry_x		= info->rx->radar_pos_coor.data4;
				info->sentry_y		= info->rx->radar_pos_coor.data5;
			}
		break;

		case GAME_ROBOT_STATUS_ID:
			memcpy(&info->rx->game_robot_status, rxBuf, 8);
		
			info->shooter_barrel_heat				= info->rx->game_robot_status.shooter_barrel_heat;
			info->shooter_barrel_heat_limit	= info->rx->game_robot_status.shooter_barrel_heat_limit;
			info->finish_small_buff = info->rx->game_robot_status.finish_small_buff;
		break;
		
		case SHOOT_DATA_ID:
			memcpy(&info->bullet_speed, rxBuf, 4);
		break;
		
		default:
			break;
	}
}

float int16_to_float(int16_t a, int16_t a_max, int16_t a_min, float b_max, float b_min)
{
    int32_t a_32 = a, a_max_32 = a_max, a_min_32 = a_min;
    int32_t diff_a = a_max_32 - a_min_32;
    
    if (diff_a == 0) return (b_max + b_min) / 2.0f; // 处理除零
    
    float ratio = (float)(a_32 - a_min_32) / (float)diff_a;
    return ratio * (b_max - b_min) + b_min;
}
