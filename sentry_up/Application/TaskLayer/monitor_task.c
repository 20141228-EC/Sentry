#include "monitor_task.h"


void Soft_Reset(void);

void StartMonitorTask(void const * argument)
{
	
	for(;;)
	{
		//裁判系统需要
		if(slave.info->rx_info->down_info->game_status.shooter_flag == 0)
		{
			rm_motor[DIAL].motor_pid.speed.out_max = 0;
			rm_motor[FRIC_L].motor_pid.speed.out_max = 0;
			rm_motor[FRIC_R].motor_pid.speed.out_max = 0;
		}
		else
		{
			rm_motor[DIAL].motor_pid.speed.out_max = 8000;
			rm_motor[FRIC_L].motor_pid.speed.out_max = 8000;
			rm_motor[FRIC_R].motor_pid.speed.out_max = 8000;
		}
		
		imu_sensor.heart_beat(&imu_sensor.work_state);
		//遥控器失联检测	
		rc_sensor.heart_beat(&rc_sensor);
		//所有电机失联检测
		rm_motor_list_heart_beat();
        //视觉信息失联检测
        vision.heartbeat(&vision);
		//下主控裁判系统信息获取失联检测
		slave.heartbeat(&slave);
//        dm_sleep_motor(&big_yaw);
		// Soft_Reset();

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

