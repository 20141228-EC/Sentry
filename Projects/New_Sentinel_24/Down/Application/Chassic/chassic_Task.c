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
#include "rp_chassis.h"
#include "remote.h"
#include "Car.h"
#include "vision.h"
#include "bmi.h"
#include "rp_gimbal.h"
#include "judge.h"
#include "judge_protocol.h"
#include <math.h>
#include <stdlib.h>
#include "rp_math.h"
#include "navigation.h"

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern chassis_t              Chassis; 
extern rc_t                   rc_structure;
extern Master_Head_t          Master_Head_structure;
extern car_t                  car_structure;
extern pid_t                  chassis_pid_follow_structure;
extern pid_t                  chassis_pid_speed_structure[4];

/* Private variables ---------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Function  body --------------------------------------------------------*/

/**
  * @Name    chassis_work
  * @brief   µ×ÅÌÈÎÎñ
  * @param   µ×ÅÌ 
  * @retval
  * @author  HWX
  * @Date    2022-11-06
**/
void chassis_work()
{
	Chassis.base_info.target.front_speed = Master_Head_structure.Send_Chassis.speed_front;
  Chassis.base_info.target.right_speed = Master_Head_structure.Send_Chassis.speed_right;
  Chassis.base_info.target.cycle_speed = Master_Head_structure.Send_Chassis.speed_cycle;
	
	Chassis_Work(&Chassis);

}

