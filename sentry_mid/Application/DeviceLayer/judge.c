/* Includes ------------------------------------------------------------------*/
#include "judge.h"

extern void Judge_Update(judge_t *judge_sen, uint8_t *rxBuf);

judge_info_t judge_info = {

	.offline_cnt 			= 1000,
	.offline_max_cnt 	= 1000,
	
};

judge_t judge = 
{
	.info 		= &judge_info,
	
	.update 	= Judge_Update,
	.heart_beat = Judge_Heartbeat,
	.work_state = DEV_OFFLINE,
};

master_info_t master_info = {

	.offline_cnt 			= 1000,
	.offline_max_cnt 	= 1000,
	
};

master_t master = 
{
	.info 		= &master_info,
	
	.update 	= master_update,
	.heart_beat = master_heart_beat,
	.work_state = DEV_OFFLINE,
};

/* Private functions ---------------------------------------------------------*/
void Judge_Heartbeat(judge_t *jud_sen)
{
	judge_info_t *jud_info = jud_sen->info;

	jud_info->offline_cnt++;
	
	if(jud_info->offline_cnt > jud_info->offline_max_cnt)
	{
		jud_info->offline_cnt = jud_info->offline_max_cnt;
		jud_sen->work_state 	= DEV_OFFLINE;
	} 
	else
	{
		/* 离线->在线 */
		if(jud_sen->work_state == DEV_OFFLINE)
		{
			jud_sen->work_state = DEV_ONLINE;
		}
	}
}

void master_update(master_t *mas_sen, uint8_t *rxbuf)
{
	master_info_t *mas_info = mas_sen->info;
	memcpy(mas_info->rxbuf, rxbuf, 8);
	mas_info->launcher_state 				= (rxbuf[0] >> 7) & 0x01;
	mas_info->gimbal_state 					= (rxbuf[0] >> 6) & 0x01;
	mas_info->chassis_state	 				= (rxbuf[0] >> 5) & 0x01;
	mas_info->vision_state					= (rxbuf[0] >> 4) & 0x01;
	mas_info->fric_switch					= (rxbuf[0] >> 3) & 0x01;
	mas_info->cap_swicth 					= (rxbuf[0] >> 2) & 0x01;
	mas_info->wheel_swicth 					= (rxbuf[0] >> 1) & 0x01;
	mas_info->move_mode 					= rxbuf[0] & 0x01;
	
	mas_info->is_aim 						= (rxbuf[1] >> 7) & 0x01;
	mas_info->aim_big_buff 					= (rxbuf[1] >> 6) & 0x01;
	mas_info->aim_small_buff				= (rxbuf[1] >> 5) & 0x01;
	mas_info->meaning_less 					= rxbuf[1] >> 4;
	
	mas_info->armor_num 					= (rxbuf[2] >> 4) & 0x0F;
	mas_info->armor_id 						= rxbuf[2] & 0x0F;

	mas_info->offline_cnt 					= 0;
}

void master_heart_beat(master_t *mas_sen)
{
	master_info_t *mas_info = mas_sen->info;

	mas_info->offline_cnt++;
	
	if(mas_info->offline_cnt > mas_info->offline_max_cnt)
	{
		mas_info->offline_cnt = mas_info->offline_max_cnt;
		mas_sen->work_state = DEV_OFFLINE;
	} 
	else
	{
		/* 离线->在线 */
		if(mas_sen->work_state == DEV_OFFLINE)
		{
			mas_sen->work_state = DEV_ONLINE;
		}
	}
}
