/*
 * @Author: lion
 * @Date: 2025-07-16 19:25:00
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-07-26 18:40:26
 * @FilePath: \sentry_mid\Application\TaskLayer\monitor_task.c
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

//  RTC_TimeTypeDef	mytime;
//	uint8_t	hours;
//	uint8_t minutes;
//	uint8_t seconds;
//	
//			RTC_TimeTypeDef sTime;
//RTC_DateTypeDef sDate;

uint8_t mode;
	
void StartMonitorTask(void const * argument)
{
	
	for(;;)
	{		
		imu_sensor.heart_beat(&imu_sensor.work_state);
		//遥控器失联检测	
		rc_sensor.heart_beat(&rc_sensor);
		//所有电机失联检测
		rm_motor_list_heart_beat();
        //决策失联检测
        Strategy_heart_beat(&Strategy);
		//大yaw心跳检测
        dm_motor_heart_beat(&big_yaw);
		
		mode = read_strategy_mode();

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
	if(rc_sensor.info->V.value == 1 && rc_sensor.info->X.value == 1 && rc_sensor.info->C.value == 1)
	{
		CAN_SendAllZero();
		//为了给电机持续发送500ms数据
		HAL_Delay(500);
		__set_FAULTMASK(1);
		NVIC_SystemReset();	
	}
}