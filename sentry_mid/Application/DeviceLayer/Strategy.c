#include "Strategy.h"

#define STRATEGY_TEST		1

CarData_t CarData = {
    .header = 0xA5,
    .cmd_id = 0x01,
};

Vision_t VisionData = {
	.speed = {
		.vx = 0,
		.vy = 0,
		.vw = 0,
	},
	.mode = {
		.gimbal_mode = 1, // 2--预瞄
	},
};

Strategy_t Strategy = {
    .vision = &VisionData,
    .CarData = &CarData,
	
	.offline_cnt = 200,
	.offline_cnt_max = 200,
	
	.work_state = DEV_OFFLINE,
};

extern UART_HandleTypeDef huart1;

uint8_t Vision_txBuf[105];
uint16_t tx_byte;

bool strategy_send_data(void)
{
	//完全没有用到
//	Strategy.CarData->sf.vw = imu_sensor.info->base_info.rate_yaw *2*PI/360;
	Strategy.CarData->sf.gimbal_yaw = big_yaw.info->p_int - BIG_YAW_MID;
	if(Strategy.CarData->sf.gimbal_yaw>65535/2)
	{
		Strategy.CarData->sf.gimbal_yaw -= 65535;
	}
	Strategy.CarData->sf.gimbal_yaw = (Strategy.CarData->sf.gimbal_yaw*8192/65535);
	
	/*决策板对于决策*/
//	Strategy.CarData->sf.strategy_mode = read_strategy_mode();
	/*发送云台小yaw的imu数值*/
	Strategy.CarData->sf.gimbal_imu_yaw = communicate.communicate_rx_up_info->up_imu_to_mech;
	/*是否开启裁判系统读取信息发送，置0为电空调试功能*/
	#if STRATEGY_TEST == 1
	if(slave.info->game_progress == 2 || slave.info->game_progress == 3)
	{
		Strategy.CarData->ce.game_start = 1;
	}
	else if(slave.info->game_progress == 4 )
	{
		Strategy.CarData->ce.game_start = 2;
	}
	else if(slave.info->game_progress == 0 || slave.info->game_progress == 1 )
	{
		Strategy.CarData->ce.game_start = 0;
	}

	/*剩余弹量*/
	Strategy.CarData->sf.bullet = slave.info->remain_bullet;
	
	/*剩余时间*/
	Strategy.CarData->ce.game_time = slave.info->remain_time;
	
	/*剩余金币*/
	Strategy.CarData->ce.remain_money  = slave.info->remain_gold;
	
	/*是否激活小符*/
   Strategy.CarData->ce.finish_buff = slave.info->finish_small_buff;
	
	/*敌我血量信息*/
	Strategy.CarData->ab.hero_blood = slave.info->ab_hero_HP;
	Strategy.CarData->ab.engineer_blood = slave.info->ab_engineer_HP;
	Strategy.CarData->ab.infantry1_blood = slave.info->ab_infantry3_HP;
	Strategy.CarData->ab.infantry2_blood = slave.info->ab_infantry4_HP;
	Strategy.CarData->ab.sentry_blood = slave.info->ab_sentry_HP;
	Strategy.CarData->ab.outpost_blood = slave.info->ab_outpost_HP;
	Strategy.CarData->ab.basepost_blood = slave.info->ab_base_HP;
	
	Strategy.CarData->eb.hero_blood = slave.info->eb_hero_HP;
	Strategy.CarData->eb.engineer_blood = slave.info->eb_engineer_HP;
	Strategy.CarData->eb.infantry1_blood = slave.info->eb_infantry3_HP;
	Strategy.CarData->eb.infantry2_blood = slave.info->eb_infantry4_HP;
	Strategy.CarData->eb.sentry_blood = slave.info->eb_sentry_HP;
	Strategy.CarData->eb.outpost_blood = slave.info->eb_outpost_HP;
	Strategy.CarData->eb.basepost_blood = slave.info->eb_base_HP;

	/**敌方位置信息 */
	if(slave.info->rx->game_status.my_color == 0)
	{
		Strategy.CarData->ep.hero_x = slave.info->hero_x;
		Strategy.CarData->ep.hero_y = slave.info->hero_y;
		Strategy.CarData->ep.engineer_x = slave.info->engineer_x;
		Strategy.CarData->ep.engineer_y = slave.info->engineer_y;
		Strategy.CarData->ep.infantry1_x = slave.info->infantry3_x;
		Strategy.CarData->ep.infantry1_y = slave.info->infantry3_y;
		Strategy.CarData->ep.infantry2_x = slave.info->infantry4_x;
		Strategy.CarData->ep.infantry2_y = slave.info->infantry4_y;
		Strategy.CarData->ep.sentry_x = slave.info->sentry_x;
		Strategy.CarData->ep.sentry_y = slave.info->sentry_y;
	}
	else
	{
		Strategy.CarData->ep.hero_x = 2800 - slave.info->hero_x;
		Strategy.CarData->ep.hero_y = 1500 - slave.info->hero_y;
		Strategy.CarData->ep.engineer_x = 2800 - slave.info->engineer_x;
		Strategy.CarData->ep.engineer_y = 1500 - slave.info->engineer_y;
		Strategy.CarData->ep.infantry1_x = 2800 - slave.info->infantry3_x;
		Strategy.CarData->ep.infantry1_y = 1500 - slave.info->infantry3_y;
		Strategy.CarData->ep.infantry2_x = 2800 - slave.info->infantry4_x;
		Strategy.CarData->ep.infantry2_y = 1500 - slave.info->infantry4_y;
		Strategy.CarData->ep.sentry_x = 2800 - slave.info->sentry_x;
		Strategy.CarData->ep.sentry_y = 1500 - slave.info->sentry_y;
	}
	
	#endif
	memcpy(Vision_txBuf, Strategy.CarData, sizeof(CarData_t));

	Append_CRC8_Check_Sum(Vision_txBuf, 3);
	Strategy.CarData->CRC8 = Vision_txBuf[2];
	Append_CRC16_Check_Sum(Vision_txBuf, sizeof(CarData));
	Strategy.CarData->CRC16 = (uint16_t)((Vision_txBuf[sizeof(CarData_t)-2] << 8)
                                   | Vision_txBuf[sizeof(CarData_t)-1]);

	tx_byte = sizeof(CarData_t);
	if(CDC_Transmit_FS(Vision_txBuf,sizeof(CarData_t)) == USBD_OK)
	{
			return true;
	}
	
	return false;
}

uint32_t t1, t2, tmp;//tmp可以计算每次接收间隔时间（单位为us）
/**
 * @note 更新决策发送过来的信息
 */
void strategy_update(Strategy_t *Strategy, uint8_t *rxBuf)
{
	if(rxBuf[0] == 0xA5)
	{
		if(Verify_CRC8_Check_Sum(rxBuf, 3) == true)
		{
			if(Verify_CRC16_Check_Sum(rxBuf, sizeof(Vision_t)) == true)
			{
				memcpy(Strategy->vision, rxBuf, sizeof(Vision_t));
				
				t1 = t2;
				t2 = micros();		// us
				tmp = t2 - t1;

				return;
			}
		}
	}
}

void strategy_get_robot_HP(Strategy_t *Strategy,judge_t *judge)
{
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
/*-------- protocol 鍑芥�? end --------*/

void USART1_rxDataHandler(uint8_t *rxBuf)
{
	strategy_update(&Strategy,rxBuf);
	Strategy.offline_cnt = 0;
}
