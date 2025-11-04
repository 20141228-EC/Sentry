/**
  ******************************************************************************
  * @file    motor.c
  * @brief   电机驱动
  ******************************************************************************
  * @attention
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "motor.h"

/* Private variables ---------------------------------------------------------*/
drv_can_t rm_motor_driver[] = {
	[CHAS_LF] =  {
		.id = DRV_CAN1,
		.rx_id = 0x202,
		.tx_id = 0x200,
		.data_id = 2,
		.CANx_XXX_DATA = CAN1_200_DATA,
	},
	[CHAS_LB] = {
		.id = DRV_CAN1,
		.rx_id = 0x204,
		.tx_id = 0x200,
		.data_id = 6,
		.CANx_XXX_DATA = CAN1_200_DATA,
	},
	[CHAS_RF] = {
		.id = DRV_CAN1,
		.rx_id = 0x201,
		.tx_id = 0x200,
		.data_id = 0,
		.CANx_XXX_DATA = CAN1_200_DATA,
	},
    [CHAS_RB] = {
		.id = DRV_CAN1,
		.rx_id = 0x203,
		.tx_id = 0x200,
		.data_id = 4,
		.CANx_XXX_DATA = CAN1_200_DATA,
	},
};

rm_motor_pid_t motor_pid[] = {
	[CHAS_LF] = {
		.speed.kp = 12.f,
		.speed.ki = 0.75f,
		.speed.kd = 1.f,
		.speed.integral_max = 8000.f,
		.speed.out_max = 16000.f,
		.angle.kp = 0,
		.angle.ki = 0,
		.angle.kd = 0,
		.angle.integral_max = 0,
		.angle.out_max = 8000,
	},
	[CHAS_LB] = {
		.speed.kp = 12.f,
		.speed.ki = 0.75f,
		.speed.kd = 1.f,
		.speed.integral_max = 8000.f,
		.speed.out_max = 16000.f,
		.angle.kp = 0,
		.angle.ki = 0,
		.angle.kd = 0,
		.angle.integral_max = 0,
		.angle.out_max = 8000,
	},
	[CHAS_RF] = {
		.speed.kp = 12.f,
		.speed.ki = 0.75f,
		.speed.kd = 1.f,
		.speed.integral_max = 8000.f,
		.speed.out_max = 16000.f,
		.angle.kp = 0,
		.angle.ki = 0,
		.angle.kd = 0,
		.angle.integral_max = 0,
		.angle.out_max = 8000,
	},
	[CHAS_RB] = {
		.speed.kp = 12.f,
		.speed.ki = 0.75f,
		.speed.kd = 1.f,
		.speed.integral_max = 8000.f,
		.speed.out_max = 16000.f,
		.angle.kp = 0,
		.angle.ki = 0,
		.angle.kd = 0,
		.angle.integral_max = 0,
		.angle.out_max = 8000,
	},
};

rm_motor_info_t rm_motor_info[MOTOR_LIST];

rm_motor_t rm_motor[] = {
	[CHAS_LF] = {
        .info = &rm_motor_info[CHAS_LF],
        .driver = &rm_motor_driver[CHAS_LF],
        .init = rm_motor_init,
        .update = rm_motor_update,
        .check = rm_motor_check,
        .heart_beat = rm_motor_heart_beat,
        .work_state = DEV_OFFLINE,
	},
	[CHAS_RF] = {
        .info = &rm_motor_info[CHAS_RF],
        .driver = &rm_motor_driver[CHAS_RF],
        .init = rm_motor_init,
        .update = rm_motor_update,
        .check = rm_motor_check,
        .heart_beat = rm_motor_heart_beat,
        .work_state = DEV_OFFLINE,
	},
	[CHAS_RB] = {
        .info = &rm_motor_info[CHAS_RB],
        .driver = &rm_motor_driver[CHAS_RB],
        .init = rm_motor_init,
        .update = rm_motor_update,
        .check = rm_motor_check,
        .heart_beat = rm_motor_heart_beat,
        .work_state = DEV_OFFLINE,
	},
	[CHAS_LB] = {
        .info = &rm_motor_info[CHAS_LB],
        .driver = &rm_motor_driver[CHAS_LB],
        .init = rm_motor_init,
        .update = rm_motor_update,
        .check = rm_motor_check,
        .heart_beat = rm_motor_heart_beat,
        .work_state = DEV_OFFLINE,
	},
};

kt_rx_info_t	Yaw_rx_info;
kt_tx_info_t	Yaw_tx_info;
kt_info_t	Yaw_info = {
	.tx_info			= &Yaw_tx_info,
	.rx_info 			= &Yaw_rx_info,
	.offline_cnt			= 100,
	.offline_max_cnt	= 100,
};

kt_pid_t Yaw_motor_pid = {
	.pos_speed.kp = 0.1,
	.pos_speed.ki = 0,
	.pos_speed.kd = 0,
	.pos_speed.integral_max = 0,
	.pos_speed.out_max = 1800,
	.pos_angle.kp = 0.1,
	.pos_angle.ki = 0,
	.pos_angle.kd = 0,
	.pos_angle.integral_max = 0,
	.pos_angle.out_max = 0,

	.vel_speed.kp = 0.5,
	.vel_speed.ki = 0,
	.vel_speed.kd = 0,
	.vel_speed.integral_max = 0,
	.vel_speed.out_max = 1800,
};

kt_driver_t Yaw_motor_driver = {
	.id = DRV_CAN2,
	.rx_id = 0x144,
	.tx_id = 0x280,
};


kt_motor_t	Yaw_motor = {
	.info				= &Yaw_info,
	.driver			= &Yaw_motor_driver,
	.work_state	= DEV_OFFLINE,
	.motor_pid	= &Yaw_motor_pid,
};


/* Exported functions --------------------------------------------------------*/

/**
 * @brief 所有电机pid初始化（清零了积分项） + OFFLINE + 初始化标志位 + 失联检测最大值设定
 * @param  无
 * @retval 无
 * @note  	会改变对应的工作状态
 */
void rm_motor_list_init(void)
{
	for(dev_motor_list_e i = CHAS_LF; i < MOTOR_LIST; i++)
		rm_motor_init(&rm_motor[i], motor_pid[i]);
}

/**
 * @brief 所有电机失联检测
 * @param  无
 * @retval 无
 * @note  	会改变对应的工作状态
 */
void rm_motor_list_heart_beat(void)
{
	for(dev_motor_list_e i = CHAS_LF; i < MOTOR_LIST; i++)
		rm_motor_heart_beat(&rm_motor[i]);
}

/**
 * @brief 所有电机工作状态
 * @param  无
 * @retval 全部在线返回1，否则返回0
 */
uint8_t rm_motor_list_workstate(void)
{
	bool flag;
	
	for(dev_motor_list_e i = CHAS_LF; i < MOTOR_LIST; i++)
		flag &= rm_motor[i].work_state;
		
	return flag;
}
