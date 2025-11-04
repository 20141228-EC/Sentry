#ifndef __KT_MOTOR_H
#define __KT_MOTOR_H

#include "rp_config.h"
#include "pid.h"
#include "rp_math.h"
#include "drv_can.h"
#include "imu_sensor.h"

#define GET_KT_STATE1          0x9A
#define CLEAR_KT_ERR_STATE     0x9B
#define GET_KT_STATE2          0x9C
#define CLOSE_KT_CURRENT       0xA1

//控制电流最大值2048

typedef struct kt_pid_ctrl {
	float	target;
	float	measure;
	int32_t 	err;
	float 	last_err;
	float	kp;
	float 	ki;
	float 	kd;
	float 	pout;
	float 	iout;
	float 	dout;
	float 	out;
	float	integral;
	float 	integral_max;
	int32_t 	out_max;
} kt_pid_ctrl_t;

typedef struct kt_rx_info_struct
{
    // 反馈信息
    volatile uint16_t	angle;		// 角度
    volatile int16_t	speed;		// 速度
    volatile int16_t	current;	// 电流
    volatile uint8_t   temperature; // 温度
}kt_rx_info_t;

typedef struct kt_tx_info_struct
{
    float current;	// 电流
}kt_tx_info_t;

typedef struct kt_info_struct
{
    kt_tx_info_t *tx_info; // 发送信息
    kt_rx_info_t *rx_info; // 接收信息
	
	uint32_t		output_time;
	uint32_t 		output_time_max;

    uint8_t	offline_cnt;
	uint8_t	offline_max_cnt;	

} kt_info_t;

typedef enum
{
    ANGLE,
    SPEED,
}pid_e;

typedef struct kt_pid_struct
{
    //双环pid控制
    pid_ctrl_t	pos_speed; // 速度pid
    kt_pid_ctrl_t	pos_angle; // 角度pid

    kt_pid_ctrl_t	vel_speed; // 速度pid
} kt_pid_t;

typedef struct kt_driver_struct
{
    can_id_t    id;				// CAN1或CAN2
	uint32_t	rx_id;  		// 反馈报文标识符
	uint32_t	tx_id;  		// 上传报文标识符
}kt_driver_t;

typedef struct kt_motor_struct
{
    kt_info_t *info; // motor information
    kt_pid_t *motor_pid; // 电机pid结构体
    kt_driver_t *driver; // 驱动结构体

    dev_work_state_t work_state; // 工作状态
}kt_motor_t;


void KT_Motor_ToSpeed(kt_motor_t *motor, float speed);
void KT_Motor_ToAngle(kt_motor_t *motor, uint32_t angle);
void KT_Motor_HeartBeat(kt_motor_t *motor);
void kt_send_request_info(kt_motor_t *motor);
void kt_get_rx_info(kt_motor_t *motor, uint8_t *rxBuf);
void kt_send_control_info(kt_motor_t *motor, int16_t current);


#endif