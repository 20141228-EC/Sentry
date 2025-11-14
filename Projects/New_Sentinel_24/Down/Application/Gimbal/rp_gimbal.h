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
#ifndef __RP_GIMBAL_H
#define __RP_GIMBAL_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "drv_can.h"
#include "can.h"
#include "9015_motor.h"
#include "can_protocol.h"

/* Private macro -------------------------------------------------------------*/
#define OFFLINE_TIME_MAX   25

#define MOROT_9015_MIDDLE (911)
#define MOROT_9015_MIDDLE_ (58342)//第一装甲板
#define AMMO_1_ANGLE 58342 //(MOROT_9015_MIDDLE_ + 0)
#define AMMO_2_ANGLE 9173 //(MOROT_9015_MIDDLE_ + 16384) 
#define AMMO_3_ANGLE 25596 //(MOROT_9015_MIDDLE_ + 32768) //(+ 24576 - 65535）超圈
#define AMMO_4_ANGLE 42069 //(MOROT_9015_MIDDLE_ + 49152)

//49,152
/*输入右头当前角度，限制左头*/
//#define Dynamic_lim_L(x) (output=(int16_t(-0.0007*x*x + 9.34*x - 29470)))

/*输入左头当前角度，限制右头*/
//#define Dynamic_lim_R(x) (output=(int16_t(0.0015*x*x - 1.59*x +6543.9)))

//R 右头（Pitch单调递减）
#define Y_MOT_UPP_LIMIT_R       (5900)  	//对着左头
#define Y_MOT_LOW_LIMIT_R       (850)   	//对着吕方
// #define Y_MOT_MID_LIMIT_R       ((Y_MOT_UPP_LIMIT_R + Y_MOT_LOW_LIMIT_R)/2) 
#define Y_MOT_MID_LIMIT_R       (Y_MOT_UPP_LIMIT_R + 1106)//1106 = (8192 - 6820 + 840)/2

#define P_MOT_UPP_LIMIT_R       (1710)	//头最低，值最大
#define P_MOT_LOW_LIMIT_R       (432)		//头最高，值最小

#define P_Init_R                (1204)
#define Y_Init_R                (5470)


//L 左头
#define Y_MOT_UPP_LIMIT_L       (1840)	//对着吕方
#define Y_MOT_LOW_LIMIT_L       (4874)	//对着右头
#define Y_MOT_MID_LIMIT_L       ((Y_MOT_UPP_LIMIT_L + Y_MOT_LOW_LIMIT_L)/2)//这个不是死区中点
// #define Y_MOT_MID_LIMIT_L       (Y_MOT_LOW_LIMIT_L - 1160)//要改，死区中间值 (8192-7400+1528)/2 = 1160


#define P_MOT_UPP_LIMIT_L       (2800)	//头最高，值最大
#define P_MOT_LOW_LIMIT_L       (1650)	//头最低，值最小

#define P_Init_L                (2219)
#define Y_Init_L                (5403)


#define Auto_Up 0
#define Auto_Down 1
#define Auto_Left 1
#define Auto_Right 0

#define PITCH_ROTATE_UNIT 3
#define YAW_ROTATE_UNIT   3


/* Private function prototypes -----------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef enum 
{
	GIMBAL_OFFLINE = 0,	
	
	GIMBAL_ONLINE  = 1,
	

}gimbal_work_state_e;

typedef struct
{
	
	uint8_t          offline_cnt;
	uint8_t          offline_cnt_max;
	gimbal_work_state_e work_sate;  		/*!< @ref 底盘工作状态 */

}gimbal_work_info_t;

typedef struct
{
	int16_t             target_yaw_angle;
	int16_t             target_pit_angle;
	gimbal_work_state_e staus;
	
}gimbal_head_info_t;

typedef struct gimbal_class_t
{
	motor_9015_t         *Yaw_9015;
	pid_t                 pos_pid;
	int32_t               target_yaw_angle;
	int32_t               target_speed;
	
	gimbal_head_info_t    L_Head;
	gimbal_head_info_t	  R_Head;
	gimbal_work_info_t    staus;
	
}gimbal_t;

/* Function  body --------------------------------------------------------*/
void Gimbal_Init(gimbal_t* gimbal, motor_9015_t* motor );
void Gimbal_BigYawSpeed(gimbal_t* gimbal);
void Gimbal_BigYawPosition(gimbal_t* gimbal);
void Gimbal_BigYawBmiPosition(gimbal_t* gimbal);
void Gimbal_DynamicLimit(gimbal_t* gimbal);
void Gimbal_BigYawAngleCheck(gimbal_t* gimbal);


/*输入右头当前角度，限制左头*/
int16_t Dynamic_lim_L(int16_t x);
/*输入左头当前角度，限制右头*/
int16_t Dynamic_lim_R(int16_t x);

/*电机坐标 2 陀螺仪坐标*/
int16_t MOTOR_9015_TO_BMI(gimbal_t* gimbal, uint16_t ammo_angle);

/*装甲板感知*/
int16_t Hurt_And_Find(void);

/*巡逻动头*/
void Yaw_Auto_R(Master_Head_t* M2H);
void Yaw_Auto_L(Master_Head_t* M2H);
void Pitch_Auto_L(Master_Head_t* M2H,uint16_t up_most);
void Pitch_Auto_R(Master_Head_t* M2H,uint16_t up_most);

#endif /*__RP_GIMBAL_H*/
