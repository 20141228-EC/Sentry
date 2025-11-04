/*
 * @Author: lion
 * @Date: 2025-07-12 20:08:16
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-07-13 20:07:05
 * @FilePath: \sentry_up\Application\HardwareLayer\DM_motor.h
 * @Description: 
 * 
 */
#ifndef __DM_MOTOR_H
#define __DM_MOTOR_H

#include "rp_config.h"
#include "driver.h"
#include "pid.h"
#include "string.h"
#include "rp_math.h"

#define DM_MOTOR_ACCURACY 8192
#define P_MIN -3.141593f
#define P_MAX 3.141593f
#define V_MIN -30.0f
#define V_MAX 30.0f
#define KP_MIN 0.0f
#define KP_MAX 500.0f
#define KD_MIN 0.0f
#define KD_MAX 5.0f
#define T_MIN -10.0f
#define T_MAX 10.0f


typedef struct dm_motor_info_struct
{
    int id;
	int state;
	int p_int;
	int v_int;
	int t_int;
	int kp_int;
	int kd_int;
	float pos;
	float vel;
	float tor;
	float Kp;
	float Kd;
	float Tmos;
	float Tcoil;
	
	uint16_t angle;

    uint16_t offline_cnt; // 离线计数
    uint16_t offline_max_cnt; // 离线最大计数
    
} dm_motor_info_t;

typedef struct dm_drv_can_struct
{	
    uint16_t CAN_Rx_ID;
    // 发数据绑定的CAN ID, 是上位机驱动参数CAN_ID加上控制模式的偏移量
    uint16_t CAN_Tx_ID;

} dm_drv_can_t;

typedef struct dm_motor_pid_struct
{
	pid_ctrl_t mech_angle_pid; // 双环机械角度PID
	pid_ctrl_t mech_speed_pid; // 双环机械速度PID
	pid_ctrl_t gyro_speed_pid; // 双环陀螺仪速度PID
	pid_ctrl_t gyro_angle_pid; // 双环陀螺仪角度PID
	pid_ctrl_t single_gyro_speed_pid; // 单轴陀螺仪速度PID

} dm_motor_pid_t;

typedef struct dm_motor_struct
{
    dm_drv_can_t *driver; // 电机驱动
    dm_motor_info_t *info; // 电机信息
	dm_motor_pid_t *pid;
    
    dev_work_state_t work_state; // 工作状态
    
} dm_motor_t;



void dm_active_motor(dm_motor_t *dm_motor);
void dm_sleep_motor(dm_motor_t *dm_motor);
void dm_motor_update(dm_motor_t *dm_motor,uint8_t *rxBuf);
void dm_motor_mit_ctrl(dm_motor_t *dm_motor, float pos, float vel,float kp, float kd,float torq);
void dm_motor_toangle(dm_motor_t *dm_motor, float angle);
void dm_motor_tospeed(dm_motor_t *dm_motor, float speed);

#endif