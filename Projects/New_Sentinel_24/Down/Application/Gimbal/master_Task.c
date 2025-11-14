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

extern uint8_t                first2vision;
/* Private variables ---------------------------------------------------------*/

uint8_t    gimbal_init_ok = 0;
int16_t    L_Head_Limit = 650;
int16_t    R_Head_Limit = 6000;
//uint16_t   first2ParVis = 0;
uint8_t    first2outpos = 0;
uint8_t    first2shake  = 0;
int16_t    find_cnt1    = 0;
int16_t    find_cnt2    = 0;
int16_t    find_cnt3    = 0;
int16_t    find_cnt4    = 0;

int16_t    find_tem1    = 0;
int16_t    find_tem2    = 0;
int16_t    find_tem3    = 0;
int16_t    find_tem4    = 0;

int16_t    L_Dynamic_Pit, R_Dynamic_Pit;
int16_t    L_Dynamic_Pit_High, R_Dynamic_Pit_High;

/* Function  body --------------------------------------------------------*/
void master_work(void)
{
	/*ÔÆÌ¨Í¨Ñ¶*/
	Master_Head_HEART(&Master_Head_structure);
	
	M2H_SENT_DATA(&Master_Head_structure);
	

}
