#ifndef  __ROBOT_H
#define  __ROBOT_H

#include "rp_math.h"
#include "Chassis.h"
#include "Gimbal.h"
#include "Booster.h"

typedef enum{
	RC_ctrl,
	PC_ctrl,
}ctrl_mode_e;

typedef enum 
{
  MECHANICAL_MODE,
  GYRO_MODE,
  WHEEL_MODE,
  INIT_MODE,
  PATROL_MODE,       // 巡逻模式（云台旋转摆头，底盘正常行动）
  OUTPOST_MODE,      // 前哨模式（云台旋转摆头，底盘小陀螺）
}move_mode_e;

typedef struct move_info_struct{
	
	int16_t vx;
	int16_t vy;
	int16_t w;
	
	float Mech_Y;
	float Mech_P;
	
	float IMU_Y;
	float IMU_P;
	
	uint8_t rc_power;
	uint8_t pc_power;

	int16_t turning_speed;              //小陀螺速度
	
}move_info_t;

typedef struct flag_info_struct
{
    uint8_t init_flag;

}flag_info_t;

typedef struct Robot_struct
{
    move_mode_e     move_mode;
    ctrl_mode_e     ctrl_mode;
    move_mode_e     last_move_mode;
    ctrl_mode_e     last_ctrl_mode;

    move_info_t     *move_info;

    flag_info_t     *flag_info;

//    chassis_t       *chassis;
    gimbal_t        *gimbal;
    booster_t       *booster;
}Robot_t;

void Robot_Ctrl(Robot_t *robot);
void Robot_Sleep(Robot_t *robot);
extern Robot_t robot;

#endif