#ifndef  __ROBOT_H
#define  __ROBOT_H

#include "rp_math.h"
#include "Chassis.h"
#include "Gimbal.h"
#include "Booster.h"
//#include  "judge.h"

#define GIMBAL_ARMOR_0				0
#define GIMBAL_ARMOR_1				1
#define GIMBAL_ARMOR_2				2
#define GIMBAL_ARMOR_3				3

#define TO_8192_360					0.043945

typedef enum{
	RC_ctrl,
	PC_ctrl,
}ctrl_mode_e;

typedef enum 
{
  MECHANICAL_MODE,
  GYRO_MODE,
  WHEEL_MODE,
  PATROL_MODE,       // 巡逻模式（云台旋转摆头，底盘正常行动）
  OUTPOST_MODE,      // 前哨模式（云台旋转摆头，底盘小陀螺）
}move_mode_e;

typedef struct move_info_struct{
	
	float vx;
	float vy;
	float w;
	float yaw_w;
	
	float chassis_vx;
	float chassis_vy;
	float chassis_w;
	
	float Mech_Y;
	float Mech_P;
	
	float IMU_Y;
	float IMU_P;
	
	float big_yaw_speed;
	
	uint8_t rc_power;
	uint8_t pc_power;

	int16_t turning_speed;              //小陀螺速度
	
}move_info_t;

typedef struct flag_info_struct
{
    uint8_t init_flag;
	uint8_t choose_flag;
	uint8_t patrol_flag;
}flag_info_t;

typedef struct Robot_struct
{
    move_mode_e     move_mode;
    ctrl_mode_e     ctrl_mode;
    move_mode_e     last_move_mode;
    ctrl_mode_e     last_ctrl_mode;

    move_info_t     *move_info;

    flag_info_t     *flag_info;

	booster_t       *booster;
    chassis_t       *chassis;
    gimbal_t        *gimbal;
    
}Robot_t;

void Robot_Ctrl(Robot_t *robot);
void Robot_Sleep(Robot_t *robot);
void Module_Vision_Send(Vision_t *vis_sen);
extern Robot_t robot;
extern uint16_t time_vision;
extern uint16_t small_buff_cnt;
extern uint16_t	patrol_wait;

#endif