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
extern void master_work(void);

void work_check(void);

/* Exported macro ------------------------------------------------------------*/


/* Exported variables --------------------------------------------------------*/
extern chassis_t              Chassis; 
extern rc_t                   rc_structure;
extern Master_Head_t          Master_Head_structure;
extern car_t                  car_structure;
extern IWDG_HandleTypeDef     hiwdg;
extern cap_t                  cap;

/* Private variables ---------------------------------------------------------*/

/* Function  body --------------------------------------------------------*/

void ChassisTask(void const * argument)
{
	while(1)
	{
		chassis_work();
		osDelay(1);
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
		//master_work();
		osDelay(2);
	}
	
}

void BmiUpdateTask(void const * argument)
{
	while(1)
	{
		/*临时放着*/
		MODE_CHECK();
		//RC_HEART(&rc_structure);
		
		/*喂狗*/
		//HAL_IWDG_Refresh(&hiwdg);   //在这里塞#if的话，很花时间，可能来不及喂狗
		osDelay(1);
	}

}


void work_check(void)
{
    LED_RED_TOGGLE();
		LED_GREEN_TOGGLE();
		LED_BLUE_TOGGLE();
        osDelay(25);
}
