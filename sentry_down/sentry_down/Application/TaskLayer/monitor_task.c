/*
 * @Author: lion
 * @Date: 2025-04-30 21:17:56
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-04-30 21:20:55
 * @FilePath: \sentry_down\Application\TaskLayer\monitor_task.c
 * @Description: 
 * 
 */
/**
  ******************************************************************************
  * @file    monitor_task.c
  * @brief   监控任务
  *          1. 各模块心跳失联检测
  *          2. 监控遥控器状态，软件复位
  ******************************************************************************
  */
#include "monitor_task.h"


void Soft_Reset(void);

void StartMonitorTask(void const * argument)
{
	for(;;)
	{
		imu_sensor.heart_beat(&imu_sensor.work_state);
		rm_motor_list_heart_beat();
 		cap.heart_beat(&cap);	
		judge.heart_beat(&judge);
		master.heart_beat(&master);
	
		hit_flag.heart_beat(&hit_flag);
		

		osDelay(1);
	}
}

/**
  * @brief  软件复位
  * @param  None
  * @retval None
  */
void Soft_Reset(void)
{
//		CAN_SendAllZero();
		HAL_Delay(500);
		__set_FAULTMASK(1);
		NVIC_SystemReset();	
}
