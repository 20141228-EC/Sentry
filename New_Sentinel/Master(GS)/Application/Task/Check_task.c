/**
  ************************************* Copyright ******************************   
  *                 (C) Copyright 2022, hwx, China, SZU.
  *                            N0   Rights  Reserved
  *                              
  *                   
  * @FileName   : rp_chassis.c   
  * @Version    : v2.0		
  * @Author     : hwx			
  * @Date       : 2023-7-06         
  * @Description:    
  *
  *
  ******************************************************************************
 */
 
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bmi.h"
#include "main.h"
#include "cmsis_os.h"
#include "driver.h"
#include "9015_motor.h"
#include "dji_pid.h"
#include "rp_chassis.h"
#include "rp_gimbal.h"
#include "remote.h"
#include "can_protocol.h"
#include "vision.h"
#include "Car.h"
#include "rp_shoot.h"
#include "judge.h"
#include "vision.h"
#include "navigation.h"
#include "cap.h"
#include "cap_protocol.h"
#include "drv_tim.h"
#include "judge.h"
#include "navigation.h"
#include <stdio.h>
#include "oled.h"
/* Private function prototypes -----------------------------------------------*/
extern void MODE_CHECK(void);
extern void chassis_work(void);
extern void gimbal_work(void);
extern void shoot_work(void);
extern void master_work(void);

void work_check(void);

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern judge_t                judge;
extern motor_9015_t           motor_9015_structure;
extern motor_9015_info_t      motor_9015_info_structure;
extern motor_9015_base_info_t motor_9015_base_info_structure;
extern motor_9015_pid_t       motor_9015_pid_structure;
extern pid_t                  pid_position_structure;
extern bmi_t                  bmi_structure;
extern chassis_t              Chassis; 
extern rc_t                   rc_structure;
extern Master_Head_t          Master_Head_structure;
extern gimbal_t               Gimbal;
extern vision_t               vision_structure;
extern car_t                  car_structure;
extern IWDG_HandleTypeDef     hiwdg;
extern navigation_t           navigation_structure;
extern cap_t                  cap;

/* Private variables ---------------------------------------------------------*/
int32_t test_speed = 5000;
int32_t test_angle = 1000;
uint8_t test_flag  = 1;

/* Function  body --------------------------------------------------------*/
/*优先级*/
void GimbalTask(void const * argument)
{
	while(1)
	{
		gimbal_work();
		
		osDelay(1);
	}
}
void VisionTask(void const * argument)
{
	while(1)
	{
		vision_task();
		Navigation_task();
		
		osDelay(5);
	}

}

void ChassisTask(void const * argument)
{
	while(1)
	{
		chassis_work();
		
		osDelay(1);
	}


}

void ShootTask(void const * argument)
{
	while(1)
	{
		shoot_work();
		
		osDelay(5);
	}
}

void CheckTask(void const * argument)
{
	while(1)
	{
		work_check(); 
		
		osDelay(1);
	}


}

void M2HTask(void const * argument)
{
	while(1)
	{	
		master_work();
		
		osDelay(2);
	}
	
}

void BmiUpdateTask(void const * argument)
{
	while(1)
	{
		BMI_Updata();
		
		/*临时放着*/
		MODE_CHECK();
		RC_HEART(&rc_structure);
		judge_heart();
		
		HAL_IWDG_Refresh(&hiwdg);
		osDelay(1);
	}

}

/*
底盘        -- 粉色
超电        -- 蓝色
两个云台    -- 青色
自瞄电脑    -- 绿色
导航电脑    -- 白色
裁判系统    -- 黄色
YAW轴电机   -- 红色
*/

void work_check(void)
{
	#if 1
	if(Chassis.work_info.work_sate == CHASSIC_ONLINE && 
	   judge.info->status          == JUDGE_ONLINE   &&
	   Master_Head_structure.L_Head_status.status == DEV_ONLINE && 
	   Master_Head_structure.R_Head_status.status == DEV_ONLINE &&
	   cap_receive_data.work_state == Cap_Online &&
	   Gimbal.staus.work_sate      == GIMBAL_ONLINE && 
	   vision_structure.state.work_state == VISION_ONLINE &&
	   navigation_structure.state.work_state == VISION_ONLINE )
	{
		if(judge.base_info->car_color == 1)
		{
				BLUE_LED_PWM();
		}
		else
		{
				RED_LED_PWM();
		}
	}
	else
	{
		if(Chassis.work_info.work_sate != CHASSIC_ONLINE)
		{
			PINK_ON();
			osDelay(200);
			BLACK_ON();
		}
		else
		{
			BLACK_ON();
		}
		
		if(judge.info->status != JUDGE_ONLINE)
		{
			YELLOW_ON();
			osDelay(200);
			BLACK_ON();
		}
		else
		{
			 BLACK_ON();
		}
		
		if(Master_Head_structure.L_Head_status.status != DEV_ONLINE && 
		   Master_Head_structure.R_Head_status.status != DEV_ONLINE)
		{
			LIGHT_ON();
			osDelay(200);
			BLACK_ON();
		}
		else
		{
			BLACK_ON();
		}
		
		if(cap_receive_data.work_state != Cap_Online)
		{
			BLUE_ON();
			osDelay(200);
			BLACK_ON();
		}
		else
		{
			BLACK_ON();
		}
		
		if(Gimbal.staus.work_sate != GIMBAL_ONLINE)
		{
			RED_ON();
			osDelay(200);
			BLACK_ON();
		}
		else
		{
			BLACK_ON();
		}
		
		if(vision_structure.state.work_state != VISION_ONLINE)
		{
			GREEN_ON();
			osDelay(200);
			BLACK_ON();
		}
		else
		{
			BLACK_ON();
		}
		
		if(navigation_structure.state.work_state != VISION_ONLINE)
		{
			WHITE_ON();
			osDelay(200);
			BLACK_ON();
		}
		else
		{
			BLACK_ON();
		}
	
	}
	#endif

	osDelay(1);
}

char ch1[22] = {0};
char ch2[22] = {0};
void OledTask(void)
{
	while(1)
	{

		
		sprintf(ch1, "Mea: %2.5f",judge.data->shoot_data.bullet_speed);
		
		OLED_ShowString(0, 0, (uint8_t *)ch1, 22);
		
		osDelay(1000);
	}
}

