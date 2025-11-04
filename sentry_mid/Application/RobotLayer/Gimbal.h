#ifndef __GIMBAL_H
#define __GIMBAL_H


#define GIMBAL_MID          	3713	

#define GIMBAL_MECH_PITCH_MAX	5300
#define GIMBAL_MECH_PITCH_MID   4772
#define GIMBAL_MECH_PITCH_MIN	4170
#define GIMBAL_GYRO_PITCH_MAX	30
#define GIMBAL_GYRO_PITCH_MIN	-28

#define BIG_YAW_MID				43470


#include "device.h"
#include "rp_config.h"

typedef enum {
	IMU_P = 0,
	IMU_Y = 1,
}IMU_cnt_t;

typedef struct IMU_Pid_struct {
	pid_ctrl_t speed;
	pid_ctrl_t angle;
} IMU_Pid_t;

typedef struct gimbal_struct
{
    IMU_Pid_t *IMU_pitch;
    IMU_Pid_t *IMU_yaw;

    rm_motor_t *pitch_motor;
    rm_motor_t *yaw_motor;

    uint8_t Reversal_flag;

    dev_work_state_t	work_state;		
}gimbal_t;

void Menc_P_Y(int16_t Pitch,int16_t Yaw);
void Imu_P_Y(float Pitch,float Yaw);

extern gimbal_t gimbal;

#endif