/**
 * @file        driver.c
 * @author      RobotPilots@2020
 * @Version     V1.0
 * @date        9-September-2020
 * @brief       Drivers' Manager.
 */
 
/* Includes ------------------------------------------------------------------*/
#include "driver.h"
#include "bmi.h"
#include "remote.h"
#include "9015_motor.h"
#include "3508_motor.h"
#include "dji_pid.h"
#include "rp_chassis.h"
#include "rp_gimbal.h"
#include "can_protocol.h"
#include "Car.h"
#include "rp_shoot.h"
#include "vision.h"
#include "judge.h"
#include "judge.h"
#include "drv_can.h"
#include "oled.h"
#include "navigation.h"
#include "string.h"
#include "stdio.h"

/*陀螺仪数据*/
bmi_t bmi_structure;
short gyrox, gyroy, gyroz;
short accx, accy, accz;

extern Master_Head_t   Master_Head_structure;

/*遥控器数据*/
rc_t                   rc_structure;
rc_info_t              rc_info_structure;
rc_base_info_t         rc_base_info_structure;

/*9015电机*/
motor_9015_t           motor_9015_structure;
motor_9015_info_t      motor_9015_info_structure;
motor_9015_base_info_t motor_9015_base_info_structure;
motor_9015_pid_t       motor_9015_pid_structure;

/*底盘电机*/
motor_3508_t           motor_3508_LF_structure;
motor_3508_base_info_t motor_3508_LF_base_info;
motor_3508_info_t      motor_3508_LF_info;

motor_3508_t           motor_3508_RF_structure;
motor_3508_base_info_t motor_3508_RF_base_info;
motor_3508_info_t      motor_3508_RF_info;

motor_3508_t           motor_3508_LB_structure;
motor_3508_base_info_t motor_3508_LB_base_info;
motor_3508_info_t      motor_3508_LB_info;

motor_3508_t           motor_3508_RB_structure;
motor_3508_base_info_t motor_3508_RB_base_info;
motor_3508_info_t      motor_3508_RB_info;

/*底盘电机PID*/
pid_t                  chassis_pid_speed_structure[4];

/*底盘跟随PID*/
pid_t                  chassis_pid_follow_structure;

  
/*底盘*/
extern chassis_t       Chassis; 

/*云台*/
gimbal_t               Gimbal;

/*整车状态*/
car_t                  car_structure;

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void DRIVER_Init(void)
{
	PWM_Init();
	USART1_Init();
	USART2_Init();
	USART3_Init();
	USART4_Init();
	USART6_Init();
	CAN1_Init();
	CAN2_Init();	
}

void DEVICE_Init(void)
{
	Vision_Init();

	Navigation_Init();

	SHOOT_INIT();

	rc_init(&rc_structure ,&rc_info_structure, &rc_base_info_structure);

	Motor_9015_Init();

	Chassis_Init(&Chassis);

	Master_Head_INIT(&Master_Head_structure);

	Gimbal_Init(&Gimbal,&motor_9015_structure);

	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14, GPIO_PIN_RESET);

	My_BMI_Init();

	OLED_Init();

	BUZZ_Freq(0);
}

