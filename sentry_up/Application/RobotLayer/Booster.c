#include "Booster.h"

booster_info_t booster_info = {
    .shoot_v = -FRIC_25,
    .shoot_f = DIAL_20_SPEED,
    .angle = 0,
	.buzzer_cnt = 200,
};

booster_t booster = {
    .DIAL_motor = &rm_motor[DIAL],
    .L_FRIC_motor = &rm_motor[FRIC_L],
    .R_FRIC_motor = &rm_motor[FRIC_R],

    .info = &booster_info,
    .work_state = DEV_OFFLINE,
    .ready_flag = 0,
};

static void FRIC_TO_SPEED(int16_t speed);
static void DIAL_TO_SPEED(int16_t speed);
static void DIAL_TO_ANGLE(int32_t angle);

static void ready_shoot(booster_t *booster);
static void single_shoot(booster_t *booster);
static void keep_shoot(booster_t *booster);
static void keep_nolimit_shoot(booster_t *booster);
static void sleep_shoot(booster_t *booster);
static void Judge_Booster_Ctrl(booster_t* booster);




void Booster_CTRL_PC(booster_t *booster)
{	
	Judge_Booster_Ctrl(booster);
    switch(booster->mode)
    {
        case READY:
        {
            ready_shoot(booster);
            break;
        }
        case SINGEL:
        {
            single_shoot(booster);
            break;
        }
        case KEEP:
		{
            keep_shoot(booster);
            break;
        }
        case SLEEP:
        {
            sleep_shoot(booster);
            break;
        }
    }
}

void Booster_CTRL_RC(booster_t *booster)
{
	Judge_Booster_Ctrl(booster);
    switch(booster->mode)
    {
        case READY:
        {
            ready_shoot(booster);
            break;
        }
        case SINGEL:
        {
            single_shoot(booster);
            break;
        }
        case KEEP:
		{
            keep_nolimit_shoot(booster);
            break;
        }
        case SLEEP:
        {
            sleep_shoot(booster);
            break;
        }
    }
}



static void ready_shoot(booster_t *booster)
{
	FRIC_TO_SPEED(booster->info->shoot_v);
	DIAL_TO_SPEED(0);

	booster->info->angle = rm_motor[DIAL].info->angle_sum;
	if((abs(rm_motor[DIAL].info->speed - 0) < 50) && (abs(rm_motor[FRIC_L].info->speed - booster->info->shoot_v) < 20 || abs(rm_motor[FRIC_R].info->speed - booster->info->shoot_v) < 20))
	{
		booster->ready_flag = 1;
	}
}

static void single_shoot(booster_t *booster)
{
	FRIC_TO_SPEED(booster->info->shoot_v);
	DIAL_TO_ANGLE(booster->info->angle);

	Motor_HandleStuck(&rm_motor[DIAL]);
}

//连发控制热量（后续视觉控制热量，射频）
static void keep_shoot(booster_t *booster)
{
	#if 1
    if(400-slave.info->rx_info->shooter_barrel_heat <= 20)
   {
       booster->info->shoot_f = 0;
   }
   else if(400-slave.info->rx_info->shooter_barrel_heat   >= 20 &&400-slave.info->rx_info->shooter_barrel_heat<=30)
   {
	   booster->info->shoot_f = DIAL_1_SPEED;
   }
   else if(400-slave.info->rx_info->shooter_barrel_heat <= 80 && 400-slave.info->rx_info->shooter_barrel_heat >= 30)
   {
	    booster->info->shoot_f = DIAL_8_SPEED;
   }
   else
   {
       booster->info->shoot_f = DIAL_20_SPEED;
   }
	#else
	booster->info->shoot_f = DIAL_20_SPEED;
	#endif
   
    DIAL_TO_SPEED(booster->info->shoot_f);
	FRIC_TO_SPEED(booster->info->shoot_v);
	Motor_HandleStuck(&rm_motor[DIAL]);

	booster->info->angle = rm_motor[DIAL].info->angle_sum;
	
}

static void keep_nolimit_shoot(booster_t *booster)
{
	booster->info->shoot_f = DIAL_10_SPEED;
	
	DIAL_TO_SPEED(booster->info->shoot_f);
	FRIC_TO_SPEED(booster->info->shoot_v);
	Motor_HandleStuck(&rm_motor[DIAL]);

	booster->info->angle = rm_motor[DIAL].info->angle_sum;
}

static void sleep_shoot(booster_t *booster)
{
	FRIC_TO_SPEED(0);
	DIAL_TO_SPEED(0);

	booster->ready_flag = 0;
}


static void FRIC_TO_SPEED(int16_t speed)
{
	Motor_ToSpeed(&rm_motor[FRIC_L],speed);
	Motor_ToSpeed(&rm_motor[FRIC_R],-speed);
}

static void DIAL_TO_SPEED(int16_t speed)
{
	Motor_ToSpeed(&rm_motor[DIAL],speed);
}

static void DIAL_TO_ANGLE(int32_t angle)
{
	Motor_ToAxleAngle(&rm_motor[DIAL],angle);
}


shoot_delay_data_t shoot_delay;
uint32_t test_launch_t1, test_launch_t2, test_launch_tmp, shoot_delay_ms;
uint32_t test_time_all,	test_time_num, test_time_avg;

//void Measure_Delay_Launch(void)
//{
//	static uint32_t last_cnt = 0;
//	float delay_ms;

//	if(RC_SW1_MID2UP)
//		test_launch_t1 = micros();
//		
//	if(last_cnt  !=	test_launch_cnt)
//	{
//		last_cnt = test_launch_cnt;
//		
//		test_launch_t2	= micros();
//		test_launch_tmp = test_launch_t2 - test_launch_t1;
//		shoot_delay_ms	=	test_launch_tmp / 1000.f;
//		if(shoot_delay_ms < 500 && shoot_delay_ms > 20)
//		{
//			test_time_all += test_launch_tmp;
//			test_time_num ++;
//			test_time_avg = test_time_all / test_time_num;

//			shoot_delay.shoot_num ++;
//		}
//		
//		
//		
//		
//		
//		// 计算均值
//		delay_ms = shoot_delay_ms;

//		if (delay_ms >= 40 && delay_ms < 45)
//			shoot_delay.delay_40 ++;
//		else if (delay_ms >= 45 && delay_ms < 50)
//			shoot_delay.delay_45 ++;
//		else if (delay_ms >= 50 && delay_ms < 55)
//			shoot_delay.delay_50 ++;
//		else if (delay_ms >= 55 && delay_ms < 60)
//			shoot_delay.delay_55 ++;
//		else if (delay_ms >= 60 && delay_ms < 65)
//			shoot_delay.delay_60 ++;
//		else if (delay_ms >= 65 && delay_ms <70)
//			shoot_delay.delay_65++;
//		else if (delay_ms >= 70 && delay_ms < 75)
//			shoot_delay.delay_70 ++;
//		else if (delay_ms >= 75 && delay_ms < 80)
//			shoot_delay.delay_75 ++;
//		else if (delay_ms >= 80 && delay_ms < 85)
//			shoot_delay.delay_80 ++;
//		else if (delay_ms >= 85 && delay_ms < 90)
//			shoot_delay.delay_85 ++;
//		else if (delay_ms >= 90 && delay_ms <95)
//			shoot_delay.delay_90++;
//		else if (delay_ms >= 95 && delay_ms < 100)
//			shoot_delay.delay_95 ++;
//		else if (delay_ms >= 100 && delay_ms < 105)
//			shoot_delay.delay_100 ++;
//		else if (delay_ms >= 105 && delay_ms < 110)
//			shoot_delay.delay_105 ++;
//		else if (delay_ms >= 110 && delay_ms < 115)
//			shoot_delay.delay_110 ++;
//		else if (delay_ms >= 115 && delay_ms <120)
//			shoot_delay.delay_115++;
//	}
//}

 float	shoot_speed;
float shoot_sum;
uint32_t shoot_num;
float shoot_speed_avg;
float shoot_speed_var_sum;
float shoot_speed_var;

void Static_shoot(void)
{
	shoot_speed = slave.info->rx_info->bullet_speed;
	//求和（进中断就累加）
	shoot_sum+=shoot_speed;
	shoot_num++;

	//求平均值
	shoot_speed_avg = shoot_sum / shoot_num;

	//求方差
	shoot_speed_var_sum += (shoot_speed - shoot_speed_avg) * (shoot_speed - shoot_speed_avg);
	shoot_speed_var = shoot_speed_var_sum / shoot_num;

}

int8_t high_cnt = 0, low_cnt = 0;
void Judge_Booster_Ctrl(booster_t* booster)
{
    
    float last_measure_speed = 0.f;
    float speed_adapt = 0.f, adapt_k = 2.48f;
    float target_speed = 24.1f;  // 修改目标射速

    /* ---------- 射速限制 ---------- */

	booster->info->limit_speed = 25.0;
    last_measure_speed = booster->info->bullet_speed;
    booster->info->bullet_speed = slave.info->rx_info->bullet_speed;
        
    
        if ((booster->info->bullet_speed != last_measure_speed) && 
						(booster->info->bullet_speed > 0.1f))
				{
					if (booster->info->bullet_speed > target_speed + 0.6f)       // (24.7, +inf)
					{
							low_cnt = 0;
							high_cnt = 0;
							speed_adapt = -25.f;
					}
					else if (booster->info->bullet_speed > target_speed + 0.3f)  // (24.4, 24.7]
					{
							low_cnt = 0;
							high_cnt = 0;
							speed_adapt = -12.f;
					}
					else if (booster->info->bullet_speed > target_speed + 0.25f)  // (24.35, 24.4]
					{
							low_cnt = 0;
							high_cnt = 0;
							speed_adapt = -8.f;
					}
					else if (booster->info->bullet_speed > target_speed + 0.15f)  // (24.25, 24.35]
					{
							low_cnt = 0;
							high_cnt++;
							speed_adapt = -3.f;
					}
					else if (booster->info->bullet_speed > target_speed + 0.1f)  // (24,20 24.25]
					{
							low_cnt = 0;
							high_cnt++;
							if (high_cnt > 1)
								speed_adapt = -12.f;
					}
					else if (booster->info->bullet_speed > target_speed )  // (24.1, 24.20]
					{
							low_cnt = 0;
							high_cnt++;
							if (high_cnt > 2)
								speed_adapt = -1.f;
					}
					else if (booster->info->bullet_speed < target_speed - 0.95f)  // (-inf, 23.15)
					{
							low_cnt = 0;
							high_cnt = 0;
							speed_adapt = +20.f;
					}
					else if (booster->info->bullet_speed < target_speed - 0.75f)  // [23.15, 23.35)
					{
							low_cnt = 0;
							high_cnt = 0;
							speed_adapt = +12.f;
					}
					else if (booster->info->bullet_speed < target_speed - 0.55f)  // [23.35, 23.55)
					{
							low_cnt = 0;
							high_cnt = 0;
							speed_adapt = +9.f;
					}
					else if (booster->info->bullet_speed < target_speed - 0.35f)  // [23.55, 23.75)
					{
							low_cnt = 0;
							high_cnt = 0;
							speed_adapt = +5.f;
					}
					else if (booster->info->bullet_speed < target_speed - 0.25f)  // [23.75, 23.85)
					{
							low_cnt++;
							high_cnt = 0;
							speed_adapt = +3.f;
					}
					else if (booster->info->bullet_speed < target_speed - 0.15f)  // [23.85, 23.95)
					{
							low_cnt++;
							high_cnt = 0;
							if (low_cnt > 1)
								speed_adapt = +2.f;
					}
					else if (booster->info->bullet_speed < target_speed - 0.05f)  // [23.95, 24.05)
					{
							low_cnt++;
							high_cnt = 0;
							if (low_cnt > 2)
								speed_adapt = +1.f;
					}
					else                                                      // [24.05, 24.1]
					{
							low_cnt = 0;
							high_cnt = 0;
							speed_adapt = 0;
					}
					
					booster->info->shoot_v -= speed_adapt * adapt_k;
			}
    


	/* ---------- 热量限制 ---------- */
	#if HEAT_LIMIT_SWITCH == 1
	uint8_t heat_low = 0, heat_high = 0;
	int16_t mid_add_shoot;			//	中热量时的射频
	
	Launch->info->launch_limit_heat 	= Judge->info->game_robot_status.shooter_barrel_heat_limit;
	Launch->info->launch_measure_heat = Judge->info->power_heat_data.shooter_id1_17mm_cooling_heat;
	Launch->info->launch_cooling_heat	=	Judge->info->game_robot_status.shooter_barrel_cooling_value;
	Launch->info->launch_remain_heat 	= Launch->info->launch_limit_heat - Launch->info->launch_measure_heat;
	
	// 中热量射频计算
	mid_add_shoot = Launch->info->launch_cooling_heat / 10 + 2;		//	比冷却多打两发
	if(mid_add_shoot > LAUNCH_DIAL_SPEED_SET / 270)
		mid_add_shoot = LAUNCH_DIAL_SPEED_SET / 270;
		
	if (Launch->info->launch_remain_heat > 40)		//	低热量
		heat_low = 1;
	else 
		heat_low = 0;
	
	if (Launch->info->launch_remain_heat <= 20)		//	高热量
		heat_high = 1;
	else
		heat_high = 0;
	
	if (Judge->work_state == DEV_OFFLINE)
	{
		Infantry->flag->LAUNCH_HIGH_HEAT_FLAG = false;
		
		Launch->conf->dial_speed_config = LAUNCH_DIAL_SPEED_SET;
		Launch->info->launch_limit_heat		= 40;
		Launch->info->launch_measure_heat	= 0;
		Launch->info->launch_remain_heat	= Launch->info->launch_limit_heat - Launch->info->launch_measure_heat;
	}
	else
	{
		if (heat_low == 1)					//	低热量		//	第一级
		{
			Launch->conf->dial_speed_config 			= LAUNCH_DIAL_SPEED_SET;
			Infantry->flag->LAUNCH_HIGH_HEAT_FLAG = false;
		}
		else if(heat_low == 0)
		{
			if(heat_high == 0)				//	中热量		//	第二级
			{
				// 1 Hz -> 270 rpm
				Launch->conf->dial_speed_config				= mid_add_shoot	* 270.f;
				Infantry->flag->LAUNCH_HIGH_HEAT_FLAG	= false;
			}
			else if(heat_high == 1)		//	高热量		//	第三级
			{
				Launch->conf->dial_speed_config 			= +0.f;
				Launch->cmd->LAUNCH_SINGLE						=	false;
				Launch->cmd->LAUNCH_CLEAN_HEAT				=	false;
				Infantry->flag->LAUNCH_HIGH_HEAT_FLAG = true;
			}
		}
	}
	#endif
}
 

void booster_clear_bullet(booster_t *booster)
{
	if( vision.info->rx_info->fire_cnt != 0 && speed_cnt == 0)//拨轮向上连发
	{
		booster->mode = KEEP;
		booster->last_mode = KEEP;
		speed_cnt = vision.info->rx_info->fire_cnt;
	}
	else if(speed_cnt > 0)
	{
		booster->mode = KEEP;
	}
	else
	{
		booster->mode = READY;
	}
}
 