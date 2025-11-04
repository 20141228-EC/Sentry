#include "Strategy.h"

CarData_t CarData = {
    .header = 0xA5,
    .cmd_id = 0x01,
};

Vision_t VisionData;

Strategy_t Strategy = {
    .vision = &VisionData,
    .CarData = &CarData,
	
	.offline_cnt = 200,
	.offline_cnt_max = 200,
	
	.work_state = DEV_OFFLINE,
};

extern UART_HandleTypeDef huart1;

uint8_t Vision_txBuf[100];

void strategy_send_data(void)
{
}

uint32_t t1, t2, tmp;
/*-------- protocol 函数 begin --------*/
void strategy_update(Strategy_t *Strategy, uint8_t *rxBuf)
{
}

void strategy_get_robot_HP(Strategy_t *Strategy,judge_t *judge)
{
	//自身蓝色
	if(judge->info->my_color == 1)
	{
		Strategy->CarData->eb.infantry1_blood = judge->info->game_robot_HP.red_3_robot_HP;
		Strategy->CarData->eb.hero_blood = judge->info->game_robot_HP.red_1_robot_HP;
		Strategy->CarData->eb.engineer_blood = judge->info->game_robot_HP.red_2_robot_HP;
		Strategy->CarData->eb.infantry2_blood = judge->info->game_robot_HP.red_4_robot_HP;
		Strategy->CarData->eb.sentry_blood = judge->info->game_robot_HP.red_7_robot_HP;
		Strategy->CarData->eb.basepost_blood = judge->info->game_robot_HP.red_base_HP;
		Strategy->CarData->eb.outpost_blood = judge->info->game_robot_HP.red_outpost_HP;
		
		Strategy->CarData->ab.infantry1_blood = judge->info->game_robot_HP.blue_3_robot_HP;
		Strategy->CarData->ab.hero_blood = judge->info->game_robot_HP.blue_1_robot_HP;
		Strategy->CarData->ab.engineer_blood = judge->info->game_robot_HP.blue_2_robot_HP;
		Strategy->CarData->ab.infantry2_blood = judge->info->game_robot_HP.blue_4_robot_HP;
		Strategy->CarData->ab.sentry_blood = judge->info->game_robot_HP.blue_7_robot_HP;
		Strategy->CarData->ab.basepost_blood = judge->info->game_robot_HP.blue_base_HP;
		Strategy->CarData->ab.outpost_blood = judge->info->game_robot_HP.blue_outpost_HP;
	}
	//自身红色
	else
	{
		Strategy->CarData->ab.infantry1_blood = judge->info->game_robot_HP.red_3_robot_HP;
		Strategy->CarData->ab.hero_blood = judge->info->game_robot_HP.red_1_robot_HP;
		Strategy->CarData->ab.engineer_blood = judge->info->game_robot_HP.red_2_robot_HP;
		Strategy->CarData->ab.infantry2_blood = judge->info->game_robot_HP.red_4_robot_HP;
		Strategy->CarData->ab.sentry_blood = judge->info->game_robot_HP.red_7_robot_HP;
		Strategy->CarData->ab.basepost_blood = judge->info->game_robot_HP.red_base_HP;
		Strategy->CarData->ab.outpost_blood = judge->info->game_robot_HP.red_outpost_HP;
		
		Strategy->CarData->eb.infantry1_blood = judge->info->game_robot_HP.blue_3_robot_HP;
		Strategy->CarData->eb.hero_blood = judge->info->game_robot_HP.blue_1_robot_HP;
		Strategy->CarData->eb.engineer_blood = judge->info->game_robot_HP.blue_2_robot_HP;
		Strategy->CarData->eb.infantry2_blood = judge->info->game_robot_HP.blue_4_robot_HP;
		Strategy->CarData->eb.sentry_blood = judge->info->game_robot_HP.blue_7_robot_HP;
		Strategy->CarData->eb.basepost_blood = judge->info->game_robot_HP.blue_base_HP;
		Strategy->CarData->eb.outpost_blood = judge->info->game_robot_HP.blue_outpost_HP;
	}
}

void Strategy_heart_beat(Strategy_t *Strategy)
{
	Strategy->offline_cnt++;
	if(Strategy->offline_cnt < Strategy->offline_cnt_max)
	{
		Strategy->work_state = DEV_ONLINE;
	}
	else
	{
		Strategy->work_state = DEV_OFFLINE;
	}
}
/*-------- protocol 函数 end --------*/

void USART1_rxDataHandler(uint8_t *rxBuf)
{
	strategy_update(&Strategy,rxBuf);
	Strategy.offline_cnt = 0;
}
