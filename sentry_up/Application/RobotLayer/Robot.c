/*
 * @Author: lion
 * @Date: 2025-04-16 20:29:46
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-07-29 01:11:07
 * @FilePath: \sentry_up\Application\RobotLayer\Robot.c
 * @Description: 
 * 
 */
#include "Robot.h"

move_info_t move_info = {
	.vx = 0,
	.vy = 0,
	.w = 0,
	.chassis_vx = 0,
	.chassis_vy = 0,
	.chassis_w = 0,
	.Mech_Y = GIMBAL_MID,
	.Mech_P = GIMBAL_MECH_PITCH_MID,
	.IMU_Y = 0,
	.IMU_P = 0,
	.rc_power = 25,
	.pc_power = 1,
	.turning_speed = 0,
};

flag_info_t flag_info = {
	.init_flag = 0,
	.choose_flag = 0,
	.patrol_flag = 0,
};

Robot_t robot = {
	.move_mode = GYRO_MODE,
	.ctrl_mode = RC_ctrl,
	.last_move_mode = GYRO_MODE,
	.last_ctrl_mode = RC_ctrl,
	.move_info = &move_info,
	.flag_info = &flag_info,
	.chassis = &chassis,
	.gimbal = &gimbal,
	.booster = &booster,
};

pid_ctrl_t	gimbal_nav_angle;
float Angle_Error;

void limit_angle(Robot_t *robot);

uint16_t thumb_choose;
uint16_t time_vision;
uint8_t fire_cnt_save; 
uint8_t time_flag;
uint8_t clear_flag;
uint8_t time_shoot_flag;
uint8_t small_buff_flag = 0;

uint16_t small_buff_cnt;
void RC_Getinfo(Robot_t  *robot)
{
    //决定控制方式以及运行模式
	if(IF_RC_SW2_DOWN)
	{
		robot->move_mode = MECHANICAL_MODE;
		robot->last_move_mode = MECHANICAL_MODE;
	}
	else if(IF_RC_SW2_MID)
	{
		#if 1
		if(rc_sensor.info->thumbwheel.value >= 600)
		{
			if(robot->flag_info->choose_flag == 0)
			{
				thumb_choose++;
				robot->flag_info->choose_flag =1;
			}
		}
		else
		{
			robot->flag_info->choose_flag = 0;
		}
		if(thumb_choose % 3 == 0)
		{
			robot->move_mode = GYRO_MODE;
			robot->last_move_mode = GYRO_MODE;
			robot->move_info->turning_speed = 0;
		}
		else if (thumb_choose % 3 == 1)
		{
			robot->move_mode = WHEEL_MODE;
			robot->last_move_mode = WHEEL_MODE;
			robot->move_info->turning_speed = 3000;
		}
		else if(thumb_choose % 3 == 2)
		{
			robot->move_mode = WHEEL_MODE;
			robot->last_move_mode = WHEEL_MODE;
			robot->move_info->turning_speed = 6000;
		}
		#else
		robot->move_mode = GYRO_MODE;
		#endif
	}
	else
	{
		robot->ctrl_mode = PC_ctrl;
	}
	if(IF_RC_SW1_MID)
	{
		vision.info->tx_info->mode = 1;		//开自瞄（打人）
	}
	else if(IF_RC_SW1_DOWN)
	{
		vision.info->tx_info->mode = 0;		//关自瞄
	}
	else if(IF_RC_SW1_UP)
	{
		vision.info->tx_info->mode = 3;		//打小符
	}

	slave.get_move_mode(&slave,robot->move_mode);

	Angle_Error = (big_yaw.info->p_int - BIG_YAW_MID)*2*PI/65535 ;

	float tempx = rc_sensor.info->ch3 * robot->move_info->rc_power;
	float tempy = -rc_sensor.info->ch2 * robot->move_info->rc_power;
	robot->move_info->vx =  -tempx * arm_cos_f32(Angle_Error) + tempy * arm_sin_f32(Angle_Error);
	robot->move_info->vy = tempx * arm_sin_f32(Angle_Error) + tempy * arm_cos_f32(Angle_Error);
	
	switch (robot->move_mode)
	{
		case MECHANICAL_MODE:
		{
			#if SHOOT_TEST
//			robot->move_info->w = -rc_sensor_info.ch0 * robot->move_info->rc_power/1.5;
			robot->move_info->Mech_P += rc_sensor_info.ch1 * 1.2/660;
			robot->move_info->Mech_Y += rc_sensor_info.ch0 * 1.2/660;
			robot->move_info->IMU_P = imu_sensor.info->base_info.pitch;
            robot->move_info->IMU_Y = imu_sensor.info->base_info.yaw;
			robot->move_info->w = 0;
			
			#else
			robot->move_info->w = -rc_sensor_info.ch0 * robot->move_info->rc_power/1.5;
			robot->move_info->Mech_P -= rc_sensor_info.ch1 * 1.2/660;
			robot->move_info->IMU_P = imu_sensor.info->base_info.pitch;
            robot->move_info->IMU_Y = imu_sensor.info->base_info.yaw;
			
			#endif
			break;
		}
		
		case GYRO_MODE:
		{	
			robot->move_info->w = chassis_follow(robot->chassis,BIG_YAW_MID);
			if(vision.info->tx_info->mode == 0)
			{
				robot->move_info->IMU_Y -= rc_sensor_info.ch0 * 0.2/660;
				robot->move_info->IMU_P -= rc_sensor_info.ch1 * 0.05/660;
			}
			if(vision.work_state == DEV_ONLINE && (vision.info->tx_info->mode == 3))
			{
				robot->move_info->IMU_Y = vision.info->target_yaw_angle;
                robot->move_info->IMU_P = vision.info->target_pitch_angle;
			}
			else if(vision.work_state == DEV_ONLINE && vision.info->tx_info->mode == 1 && (vision.info->rx_info->is_find_target == 1 || vision.info->rx_info->is_find_strategy == 1) )
			{
				robot->move_info->IMU_Y = vision.info->target_yaw_angle;
                robot->move_info->IMU_P = vision.info->target_pitch_angle;
			}
			
			
			robot->move_info->Mech_P = rm_motor[GIMB_P].info->angle;
			break;
		}
		
		case WHEEL_MODE:
		{
			robot->move_info->w = robot->move_info->turning_speed;
			#if RC_SHOOT_AIM
			if(vision.work_state == DEV_ONLINE && (vision.info->is_find_target == 1)&&vision.info->tx_info->mode == 1)
			{
				robot->move_info->IMU_Y = vision.info->rx_info->yaw_angle;
                robot->move_info->IMU_P = vision.info->rx_info->pitch_angle;
			}
			#else
			robot->move_info->IMU_Y -= rc_sensor_info.ch0 * 0.3/660;
            robot->move_info->IMU_P += rc_sensor_info.ch1 * 0.2/660;
			#endif
			break;
		}
	}

	if(robot->move_mode == GYRO_MODE)
	{
			/*自瞄模式发射*/
		#if 0
			if(vision.work_state ==  DEV_ONLINE)
			{
				//摩擦轮控制
				if(vision.info->rx_info->is_find_target == 1 &&((vision.info->tx_info->mode == 1) ||(vision.info->tx_info->mode == 3)))
				{	
					if(RC_RIGH_CH_LR_VALUE > 100)	//右遥杆   <-- 关摩擦轮 , -->开摩擦轮 
					{
						robot->booster->mode = READY;
						robot->booster->last_mode = READY;
					}
					else if(RC_RIGH_CH_LR_VALUE < -100)
					{
						robot->booster->mode = SLEEP;
						robot->booster->last_mode = SLEEP;
					}
				}
				else
				{
					robot->booster->mode = SLEEP;
					robot->booster->last_mode = SLEEP;
				}
				#endif
				if(vision.info->tx_info->mode == 3)
				{
					robot->booster->mode = READY;
				}
				else
				{
					robot->booster->mode = SLEEP;
				}
				
				//拨盘设置
				if( robot->booster->mode != SLEEP)	
				{
					#if SINGLE_MODE
					
						if(RC_TB_VALUE > 600 && vision.info->rx_info->fire_cnt != 0)	//拨轮向下一次单发	
						{
							robot->booster->mode = SINGEL;

							if(robot->booster->last_mode == READY)
							{
								robot->booster->info->angle = rm_motor[DIAL].info->angle_sum + ONE_SHOOT_ANGLE;
							}

							robot->booster->last_mode = SINGEL;
							
							
						}
						else if(RC_TB_VALUE > -50 && RC_TB_VALUE <= 50 )
						{
							if(robot->booster->last_mode == SINGEL && abs(rm_motor[DIAL].info->angle_sum - robot->booster->info->angle) < 50)//卡弹原因
							{
								robot->booster->mode = READY;
								robot->booster->last_mode = READY;
							}		
						}
						

					#endif
					#if KEEP_MODE
					if( vision.info->rx_info->fire_cnt != 0 && speed_cnt == 0 )
					{
						if(vision.info->rx_info->time == 0 && time_flag == 0)
						{
							robot->booster->mode = KEEP;
							robot->booster->last_mode = KEEP;
							speed_cnt = vision.info->rx_info->fire_cnt;
						}
						else
						{
							if(time_flag == 0)
							{
								time_vision = vision.info->rx_info->time*2;
								time_flag = 1;
								fire_cnt_save = vision.info->rx_info->fire_cnt;
							}
							else
							{
								if(time_vision == 1)
								{
									time_flag =0;
									robot->booster->mode = KEEP;
									robot->booster->last_mode = KEEP;
									speed_cnt = fire_cnt_save;
								}
								else
								{
									robot->booster->mode = READY;
								}
							}
						}	
							
					}
					else if(speed_cnt > 0 && time_flag == 0)
					{
						robot->booster->mode = KEEP;
					}
					else if( time_flag == 1)
					{
						if(time_vision == 1)
						{
							time_flag =0;
							robot->booster->mode = KEEP;
							robot->booster->last_mode = KEEP;
							speed_cnt = fire_cnt_save;
						}
					}
					else
					{
						robot->booster->mode = READY;
					}
						
					
				#endif
				
				#if SMALL_BUFF
				
				if(RC_TB_VALUE > 600 && vision.info->rx_info->fire_cnt != 0 && small_buff_cnt == 0)	//拨轮向下一次单发	
				{
					robot->booster->mode = SINGEL;

					if(robot->booster->last_mode == READY)
					{
						robot->booster->info->angle = rm_motor[DIAL].info->angle_sum + ONE_SHOOT_ANGLE;
						small_buff_flag = 1;
					}
					robot->booster->last_mode = SINGEL;
				}
				
				if((robot->booster->last_mode == SINGEL) && (abs(rm_motor[DIAL].info->angle_sum - robot->booster->info->angle) < 120))
				{
					robot->booster->mode = READY;
					robot->booster->last_mode = READY;
					if(small_buff_flag == 1)
					{
						small_buff_flag = 0;
						small_buff_cnt = 1000;
					}
				}
				
				
				
				#endif
				}
			
		
	}

	
	if(robot->move_mode == MECHANICAL_MODE)
	{
		 //发射信息获取
		if(IF_RC_SW1_DOWN)
		{
			robot->booster->mode = SLEEP;
			robot->booster->last_mode = SLEEP;
			robot->booster->ready_flag = 0;
		}
		if(IF_RC_SW1_MID && robot->booster->ready_flag == 0)	//没初始化就初始化一次,向下就能关闭
		{
			robot->booster->mode = READY;
			robot->booster->last_mode = READY;
			robot->booster->ready_flag = 1;
		}
		//	 Measure_Delay_Launch();

			
		if(robot->booster->ready_flag == 1)		//开摩擦轮之后才能获取发射信息
		{
			if(IF_RC_SW1_MID && (RC_TB_VALUE > 450))	//s1中间,拇指轮向下开连发
			{
				robot->booster->mode = KEEP;
				robot->booster->last_mode = KEEP;
			}
			else if(RC_SW1_MID2UP)						//s1跳变开单发
			{
				robot->booster->mode = SINGEL;
				robot->booster->last_mode = SINGEL;
				robot->booster->info->angle = rm_motor[DIAL].info->angle_sum + ONE_SHOOT_ANGLE;
			}
			//需要加关控直接变为sleep状态（在关控那里实现状态变化？）
			else	
			{
				if((robot->booster->last_mode == SINGEL) && (abs(rm_motor[DIAL].info->angle_sum - robot->booster->info->angle) < 20))
				{
					robot->booster->mode = READY;
					robot->booster->last_mode = READY;
				}
				if(robot->booster->last_mode == KEEP)
				{
					robot->booster->mode = READY;
					robot->booster->last_mode = READY;
				}
			}
		}
	}
		
}


int16_t speed_test;

void RC_Move_Ctrl(Robot_t *robot)
{
    limit_angle(robot);

	switch(robot->move_mode)
	{
		case MECHANICAL_MODE:
		{
			//发送下主控速度信息
			Menc_P_Y(robot->move_info->Mech_P,robot->move_info->Mech_Y);
			slave.info->tx_info->chassis_info->vx = robot->move_info->vx;
			slave.info->tx_info->chassis_info->vy = robot->move_info->vy;
			slave.info->tx_info->chassis_info->w = robot->move_info->w;

			break;
		}
		case GYRO_MODE:
		{			
			Imu_P_Y(robot->move_info->IMU_P,robot->move_info->IMU_Y);
			slave.info->tx_info->chassis_info->vx = robot->move_info->vx;
			slave.info->tx_info->chassis_info->vy = robot->move_info->vy;
			slave.info->tx_info->chassis_info->w = robot->move_info->w;
			break;
		}
		case WHEEL_MODE:
		{
			Imu_P_Y(robot->move_info->IMU_P,robot->move_info->IMU_Y);
			slave.info->tx_info->chassis_info->vx = robot->move_info->vx;
			slave.info->tx_info->chassis_info->vy = robot->move_info->vy;
			slave.info->tx_info->chassis_info->w = robot->move_info->w;
			break;
		}
	}


}
uint8_t	yaw_flag = 0;
uint8_t	yaw_test = 50;
int16_t yaw_add_cnt;
uint16_t	patrol_wait;
uint8_t first_shoot_buff;
void PC_Getinfo(Robot_t  *robot)
{
	// 判断控制方式
	if(IF_RC_SW2_UP)
	{
	 	robot->ctrl_mode = PC_ctrl;
	}
	else
	{
		robot->ctrl_mode = RC_ctrl;
		return;
	}

	#if 1
	if(IF_RC_SW1_UP)
	{
		slave.info->tx_info->ctrl_flag[7] = 2;
	}
	else if(IF_RC_SW1_MID)
	{
		slave.info->tx_info->ctrl_flag[7] = 1;
	}
	else if(IF_RC_SW1_DOWN)
	{
		slave.info->tx_info->ctrl_flag[7] = 0;
	}
	#endif

	//巡逻
	if(slave.info->rx_info->mid_info->gimbal_info.gimbal_mode == 0)
	{
		robot->move_mode = PATROL_MODE;
	}
	//巡逻但是不上下摆头
	else if(slave.info->rx_info->mid_info->gimbal_info.gimbal_mode == 1)
	{
		robot->move_mode = MECHANICAL_MODE;
	}
	else if(slave.info->rx_info->mid_info->gimbal_info.gimbal_mode == 2)
	{
		robot->move_mode = GYRO_MODE;
	}

	//获取运动状态标志位
	slave.get_move_mode(&slave,robot->move_mode);

	switch(robot->move_mode)
	{
		case PATROL_MODE:
		{
			vision.info->tx_info->mode = 1;
			if(vision.work_state == DEV_ONLINE && (vision.info->rx_info->is_find_target == 1 ) &&vision.info->tx_info->mode == 1)
			{
				slave.info->tx_info->ctrl_flag[3] = 1;
				robot->move_info->IMU_Y = vision.info->target_yaw_angle;
				robot->move_info->IMU_P = vision.info->target_pitch_angle;
			}
			else if(vision.work_state == DEV_ONLINE && (vision.info->rx_info->is_find_strategy)&&vision.info->tx_info->mode == 1)
			{
				robot->move_info->IMU_Y = vision.info->target_yaw_angle;
				robot->move_info->IMU_P = vision.info->target_pitch_angle;
				patrol_wait = 2000;
			}
			
			else if(patrol_wait <= 1)
			{
					slave.info->tx_info->ctrl_flag[3] = 0;
					robot->move_info->yaw_w = yaw_test;
					robot->move_info->w = 0;
					robot->move_info->IMU_Y = imu_sensor.info->base_info.yaw;
					
					if(robot->gimbal->Reversal_flag == 0)
					{
						robot->move_info->IMU_P += 0.07;
						if(robot->move_info->IMU_P >= 2.0f)
						{
							robot->gimbal->Reversal_flag = 1;
						}
					}
					else
					{
						robot->move_info->IMU_P -= 0.07;
						if(robot->move_info->IMU_P <= -20.0f)
						{
							robot->gimbal->Reversal_flag = 0;
						}
					}
			}
			else if(patrol_wait>2)
			{
				robot->move_info->yaw_w = 0;
			}
			break;
		}
		
		case MECHANICAL_MODE:
		{
			robot->move_info->IMU_P = imu_sensor.info->base_info.pitch;
            robot->move_info->IMU_Y = imu_sensor.info->base_info.yaw;
			robot->move_info->Mech_P = GIMBAL_MECH_PITCH_MID;
			robot->move_info->Mech_Y = GIMBAL_MID;
			break;
		}
		
		case GYRO_MODE:
		{
			/*只要找到过符叶，就置标志位*/
			if(vision.info->rx_info->is_find_target != 0 && vision.info->tx_info->mode ==3)
			{
				first_shoot_buff = 1;
			}
			/*找到符叶完全响应角度*/
			if(first_shoot_buff == 1 && vision.work_state == DEV_ONLINE)
			{
				robot->move_info->IMU_Y = vision.info->target_yaw_angle;
                robot->move_info->IMU_P = vision.info->target_pitch_angle;
			}
			/*没有找到符叶*/
			if(first_shoot_buff == 0)
			{
				robot->move_info->IMU_Y =  slave.info->rx_info->mid_info->gimbal_info.gimbal_target_yaw * ECD_TO_ANGLE - 180.0f;
				if(robot->gimbal->Reversal_flag == 0)
				{
					robot->move_info->IMU_P += 0.07;
					if(robot->move_info->IMU_P >= 27.0f)
					{
						robot->gimbal->Reversal_flag = 1;
					}
				}
				else
				{
					robot->move_info->IMU_P -= 0.07;
					if(robot->move_info->IMU_P <= 10.0f)
					{
						robot->gimbal->Reversal_flag = 0;
					}
				}
			}
			
		}
	}
	
		/*存在fire_cnt 需要清空接收下一次指令，test_launch_cnt在发射中断中减1*/
	if(vision.info->tx_info->mode == 1)
	{
		 if(vision.info->rx_info->fire_cnt != 0 && speed_cnt == 0)
		{
				if(vision.info->rx_info->time == 0 && time_flag == 0)
				{
					robot->booster->mode = KEEP;
					robot->booster->last_mode = KEEP;
					speed_cnt = vision.info->rx_info->fire_cnt;
				}
				else
				{
					if(time_flag == 0)
					{
						time_vision = vision.info->rx_info->time*2;
						time_flag = 1;
						fire_cnt_save = vision.info->rx_info->fire_cnt;
					}
					else
					{
						if(time_vision == 1)
						{
							time_flag =0;
							robot->booster->mode = KEEP;
							robot->booster->last_mode = KEEP;
							speed_cnt = fire_cnt_save;
						}
						else
						{
							robot->booster->mode = READY;
						}
					}
				}
						
							
			}
		else if(speed_cnt > 0 && time_flag == 0)
		{
			robot->booster->mode = KEEP;
		}
		else if( time_flag == 1)
		{
			if(time_vision == 1)
			{
				time_flag =0;
				robot->booster->mode = KEEP;
				robot->booster->last_mode = KEEP;
				speed_cnt = fire_cnt_save;
			}
		}
		else
		{
			robot->booster->mode = READY;
		}
	}
	else if(vision.info->tx_info->mode == 3 && first_shoot_buff == 1)
	{
		robot->booster->mode = READY;
		
		if(robot->booster->mode != SLEEP)
		{
			if( vision.info->rx_info->fire_cnt != 0 && small_buff_cnt == 0)	
			{
				robot->booster->mode = SINGEL;

				if(robot->booster->last_mode == READY)
				{
					robot->booster->info->angle = rm_motor[DIAL].info->angle_sum + ONE_SHOOT_ANGLE;
					small_buff_flag = 1;
				}
				robot->booster->last_mode = SINGEL;
			}
					
			if((robot->booster->last_mode == SINGEL) && (abs(rm_motor[DIAL].info->angle_sum - robot->booster->info->angle) < 100))
			{
				robot->booster->mode = READY;
				robot->booster->last_mode = READY;
				if(small_buff_flag == 1)
				{
					small_buff_flag = 0;
					small_buff_cnt = 1000;
				}
			}
		}
		
		
	}
		
	
}

void PC_Move_Ctrl(Robot_t *robot)
{
	limit_angle(robot);
	switch (robot->move_mode)
	{
		case MECHANICAL_MODE:
		{
			Menc_P_Y(robot->move_info->Mech_P,robot->move_info->Mech_Y);
			break;	
		}
		case PATROL_MODE:
		{
			if(vision.work_state == DEV_ONLINE && ((vision.info->rx_info->is_find_target == 1 )||(vision.info->rx_info->is_find_strategy)) )
			{
				Imu_P_Y(robot->move_info->IMU_P,robot->move_info->IMU_Y);
			}
			else
			{
				imu_speed_Y(robot->move_info->yaw_w);
				imu_ToPangle(robot->move_info->IMU_P);
			}

			break;
		}
		
		case GYRO_MODE:
		{
			Imu_P_Y(robot->move_info->IMU_P,robot->move_info->IMU_Y);
		}
	}
}

void Robot_Ctrl(Robot_t *robot)
{
	//传递运动模式
	slave.get_ctrl_mode(&slave,robot->ctrl_mode);

    switch (robot->ctrl_mode)
    {
        case RC_ctrl:
        {
            RC_Getinfo(robot);
            RC_Move_Ctrl(robot);
//			Booster_CTRL_RC(robot->booster);
			break;

        }
        case PC_ctrl:
        {
			PC_Getinfo(robot);
			PC_Move_Ctrl(robot);
			if(slave.info->rx_info->game_progress == 4 )
			{
//				Booster_CTRL_PC(robot->booster);
			}
			
			break;

        }
    }
}

void Robot_Sleep(Robot_t *robot)
{
	robot->move_mode = GYRO_MODE;
	
	robot->move_info->vx = 0;
	robot->move_info->vy = 0;
	robot->move_info->w = 0;

	robot->move_info->Mech_Y = GIMBAL_MID;
	robot->move_info->Mech_P = GIMBAL_MECH_PITCH_MID;

	robot->move_info->IMU_Y = imu_sensor.info->base_info.yaw;
	robot->move_info->IMU_P = imu_sensor.info->base_info.pitch;

	robot->flag_info->init_flag = 0;
	
	robot->booster->mode = SLEEP;
	
	thumb_choose = 0;
	robot->flag_info->choose_flag = 0;
	
	robot->gimbal->view_flag = 0;
	
	robot->booster->info->buzzer_cnt = 0;
	yaw_add_cnt = 0;
	speed_cnt = 0;

}

/*******pitch限位******/
float pitch_max_err, pitch_min_err;
float max, min;
static void limit_angle(Robot_t *robot)
{
	/*Y(0,8191)		IMU_Y(-180,180)*/
	if(robot->move_info->Mech_Y < 0)			{robot->move_info->Mech_Y += 8192;}
	if(robot->move_info->Mech_Y > 8191)		{robot->move_info->Mech_Y -= 8192;}
	if(robot->move_info->IMU_Y > 180)	{robot->move_info->IMU_Y -= 360;}
	if(robot->move_info->IMU_Y < -180)	{robot->move_info->IMU_Y += 360;}

	/*Pitch*/
	if(robot->move_mode == MECHANICAL_MODE)
	{
		//固定机械限位
		if(robot->move_info->Mech_P > GIMBAL_MECH_PITCH_MAX)			{robot->move_info->Mech_P = GIMBAL_MECH_PITCH_MAX;}
		else if(robot->move_info->Mech_P < GIMBAL_MECH_PITCH_MIN)		{robot->move_info->Mech_P = GIMBAL_MECH_PITCH_MIN;}
	}
	
	#if 1
	/**	固定imu限位 */
	if(robot->move_info->IMU_P > GIMBAL_GYRO_PITCH_MAX)
	{
		robot->move_info->IMU_P = GIMBAL_GYRO_PITCH_MAX;
	}
	if(robot->move_info->IMU_P < GIMBAL_GYRO_PITCH_MIN)
	{
        robot->move_info->IMU_P = GIMBAL_GYRO_PITCH_MIN;
	}
	#else

   //动态限位
	if(robot->move_mode == GYRO_MODE)
	{
		//动态限位,上下坡时可以保持pitch始终控制在前方一个固定范围
		if(rm_motor[GIMB_P].info->angle > 4096)//跳变处理
		{
			pitch_max_err = (float)(rm_motor[GIMB_P].info->angle - 8192 - GIMBAL_MECH_PITCH_MAX) *360.0/8192;
			pitch_min_err = (float)(rm_motor[GIMB_P].info->angle - 8192 - GIMBAL_MECH_PITCH_MIN)  *360.0/8192;
		}
		else
		{	
			pitch_max_err = (float)(rm_motor[GIMB_P].info->angle - GIMBAL_MECH_PITCH_MAX) *360.0/8192;
			pitch_min_err = (float)(rm_motor[GIMB_P].info->angle - GIMBAL_MECH_PITCH_MIN) *360.0/8192;
		}

		max = pitch_max_err + imu_sensor.info->base_info.pitch;
		min = pitch_min_err + imu_sensor.info->base_info.pitch;

		if(robot->move_info->IMU_P>max)		{robot->move_info->IMU_P = max;}
		else if(robot->move_info->IMU_P<min)		{robot->move_info->IMU_P = min;}
	}
	#endif
}

void Module_Vision_Send(Vision_t *vis_sen)
{
	
	Vision_tx_info_t *tx_info = vis_sen->info->tx_info;
	
	if(slave.info->rx_info->game_progress == 4)
	tx_info->start_game = 1;
	//转发决策对应的自瞄模式
	if(robot.ctrl_mode == PC_ctrl )
	{
		tx_info->mode = slave.info->rx_info->mid_info->gimbal_info.vision_mode;
	}
	
	//小yaw的角速度
	tx_info->yaw_v = imu_sensor.info->base_info.rate_yaw*(2*PI/360);
	// angle
	//yaw初始角度:180 -> 0 -> -180,视觉需要角度:0 -> 4096 -> 8192
	tx_info->yaw_angle 	= \
	imu_sensor.info->base_info.yaw * ANGLE_TO_ECD + HALF_ECD_RANGE;

	/*pitch初始角度:180 		视觉需要角度:0 					
					^ 					^ 
					0 					4096 
					^ 					^ 
					-180					8192*/

	tx_info->pitch_angle = \
	(abs(imu_sensor.info->base_info.pitch - 180.f)) * ANGLE_TO_ECD;

	tx_info->roll_angle = \
	-imu_sensor.info->base_info.roll* ANGLE_TO_ECD + HALF_ECD_RANGE;
	
	//角度转8192
	tx_info->yaw_v = imu_sensor.info->base_info.rate_yaw *PI/180;
	tx_info->pitch_v = imu_sensor.info->base_info.rate_pitch*PI/180;

	tx_info->my_color = slave.info->rx_info->my_color;
	
	tx_info->small_motor_yaw = GIMBAL_MID - rm_motor[GIMB_Y].info->angle;
	tx_info->ladar_x = slave.info->rx_info->mid_info->ladar_info.ladar_x;
	tx_info->ladar_y = slave.info->rx_info->mid_info->ladar_info.ladar_y;
	tx_info->ladar_yaw = slave.info->rx_info->mid_info->ladar_info.current_yaw;
	tx_info->ladar_vx = (float)slave.info->rx_info->mid_info->gimbal_info.current_vx/10000.0f;
	tx_info->ladar_vy = (float)slave.info->rx_info->mid_info->gimbal_info.current_vy/10000.0f;
	
	tx_info->blood_1 = slave.info->rx_info->eb_sentry_HP;
	tx_info->blood_2 = slave.info->rx_info->eb_hero_HP;
	tx_info->blood_3 = slave.info->rx_info->eb_engineer_HP;
	tx_info->blood_4 = slave.info->rx_info->eb_infantry3_HP;
	tx_info->blood_5 = slave.info->rx_info->eb_infantry4_HP;
	tx_info->blood_6 = 0;
	tx_info->blood_7 = slave.info->rx_info->eb_outpost_HP;
	tx_info->blood_8 = slave.info->rx_info->eb_base_HP;
	
	#if 0
	tx_info->blood_1 = 100;
	tx_info->blood_2 = 100;
	tx_info->blood_3 = 100;
	tx_info->blood_4 = 100;
	
	tx_info->blood_5 = 100;
	tx_info->blood_6 = 100;
	tx_info->blood_7 = 100;
	tx_info->blood_8 = 100;
	
	#endif

	vision_send_data();
	
}
