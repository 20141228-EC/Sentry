/*
 * @Author: lion
 * @Date: 2025-04-24 16:37:04
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-08-05 11:41:08
 * @FilePath: \sentry_mid\Application\TaskLayer\control_task.c
 * @Description: 
 * 
 */
#include "control_task.h"

float test_speed;

pid_ctrl_t big_yaw_speed_pid = {
	.kp = -0.5,
	.out_max = 300,
};

uint16_t j;

void StartControlTask(void const * argument)
{
	float speed_target;
  	for(;;)
  	{
	  if(communicate.communicate_rx_up_info->start_flag == 1 &&communicate.work_state == DEV_ONLINE)
	  {
		  j++;
		  //使能大yaw电机（频率为200hz）
		  if(j%5 == 0)
		  {
			  dm_active_motor(&big_yaw);
		  }
		  
		  //遥控控制
		  if(communicate.communicate_rx_up_info->ctrl_mode == RC_ctrl)
		  {
			//机械模式
			if(communicate.communicate_rx_up_info->move_mode == MECHANICAL_MODE)
			{
				//角度环直接去跟随机械中值
				dm_motor_toangle(&big_yaw,BIG_YAW_MID);
			}
			  
			//陀螺仪或者小陀螺模式（大yaw一直跟随小yaw即可）
			else if(communicate.communicate_rx_up_info->move_mode == GYRO_MODE || communicate.communicate_rx_up_info->move_mode == WHEEL_MODE)
			{
				big_yaw_speed_pid.measure = rm_motor[GIMB_Y].info->angle;
				big_yaw_speed_pid.target = GIMBAL_MID;
				big_yaw_speed_pid.err = rm_motor[GIMB_Y].info->angle - GIMBAL_MID;
				//小yaw半圈处理
				if(big_yaw_speed_pid.err < 0)
				{
						big_yaw_speed_pid.err += 8192;
				}
				if(big_yaw_speed_pid.err > 4096)
				{
					big_yaw_speed_pid.err -= 8192;
				}
				single_pid_ctrl(&big_yaw_speed_pid);
				speed_target = big_yaw_speed_pid.out;
				dm_motor_tospeed(&big_yaw,speed_target);
					
				}
		  }
		  else
		  //导航模式（大yaw一直跟随小yaw即可）
		  {
			big_yaw_speed_pid.measure = rm_motor[GIMB_Y].info->angle;
			big_yaw_speed_pid.target = GIMBAL_MID;
			big_yaw_speed_pid.err = rm_motor[GIMB_Y].info->angle - GIMBAL_MID;
			//小yaw半圈处理
			if(big_yaw_speed_pid.err < 0)
			{
					big_yaw_speed_pid.err += 8192;
			}
			if(big_yaw_speed_pid.err > 4096)
			{
				big_yaw_speed_pid.err -= 8192;
			}
			single_pid_ctrl(&big_yaw_speed_pid);
			speed_target = big_yaw_speed_pid.out;
			dm_motor_tospeed(&big_yaw,speed_target);
		  }
		}
		else
		{
			//失能大yaw电机（1000hz）
			dm_sleep_motor(&big_yaw);
		}
		
		osDelay(1);
 	}
  
}
