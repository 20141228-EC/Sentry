/**
  ******************************************************************************
  * @file    motor.c
  * @brief   电机驱动
  ******************************************************************************
  * @attention
  * typedef struct
  * {
  *  	can_id_t    id;				// CAN1或CAN2
  * 	uint32_t	rx_id;  		// 反馈报文标识符
  * 	uint32_t	tx_id;  		// 上传报文标识符
  * 	uint8_t		data_idx;		// 数组索引
  * 	uint8_t		*CANx_XXX_DATA; // 发送的数组
  * } drv_can_t;
  * 3508最大电流16384
  * 6020最大电压25000
  * 2006最大电流10000
  * 最大电流不应超过额定值
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "motor.h"

/* Private variables ---------------------------------------------------------*/
drv_can_t rm_motor_driver[] = {
	//底盘电机驱动
	[CHAS_LF] =  {
		.id = DRV_CAN1,
		.rx_id = 0x202,
		.tx_id = 0x200,
		.data_id = 2,
		.CANx_XXX_DATA = CAN1_200_DATA,
	},
	[CHAS_LB] = {
		.id = DRV_CAN1,
		.rx_id = 0x201,
		.tx_id = 0x200,
		.data_id = 0,
		.CANx_XXX_DATA = CAN1_200_DATA,
	},
	[CHAS_RF] = {
		.id = DRV_CAN1,
		.rx_id = 0x203,
		.tx_id = 0x200,
		.data_id = 4,
		.CANx_XXX_DATA = CAN1_200_DATA,
	},
    [CHAS_RB] = {
		.id = DRV_CAN1,
		.rx_id = 0x204,
		.tx_id = 0x200,
		.data_id = 6,
		.CANx_XXX_DATA = CAN1_200_DATA,
	},
	//云台电机驱动
	[GIMB_P] = {
		.id = DRV_CAN2,
		.rx_id = 0x205,
		.tx_id = 0x1FF,
		.data_id = 0,
		.CANx_XXX_DATA = CAN2_1FF_DATA,
	},
	[GIMB_Y] = {
		.id = DRV_CAN2,
		.rx_id = 0x206,
		.tx_id = 0x1FF,
		.data_id = 2,
		.CANx_XXX_DATA = CAN2_1FF_DATA,
	},
	//发射轮
	[FRIC_L] = {
		.id = DRV_CAN2,
		.rx_id = 0x202,
		.tx_id = 0x200,
		.data_id = 2,
		.CANx_XXX_DATA = CAN2_200_DATA,
	},
	[FRIC_R] = {
		.id = DRV_CAN2,
		.rx_id = 0x203,
		.tx_id = 0x200,
		.data_id = 4,
		.CANx_XXX_DATA = CAN2_200_DATA,
	},
	[DIAL] = {
		.id = DRV_CAN2,
		.rx_id = 0x201,
		.tx_id = 0x200,
		.data_id = 0,
		.CANx_XXX_DATA = CAN2_200_DATA,
	},
	[VIEW] = {
		.id = DRV_CAN2,
		.rx_id = 0x204,
		.tx_id = 0x200,
		.data_id = 6,
		.CANx_XXX_DATA = CAN2_200_DATA,
	},
};

rm_motor_pid_t motor_pid[] = {
	[CHAS_LF] = {
		.speed.kp = 16,
		.speed.ki = 0.9,
		.speed.kd = 4,
		.speed.integral_max = 8000,
		.speed.out_max = 9000,
		.angle.kp = 0,
		.angle.ki = 0,
		.angle.kd = 0,
		.angle.integral_max = 0,
		.angle.out_max = 8000,
	},
	[CHAS_LB] = {
		.speed.kp = 16,
		.speed.ki = 0.9,
		.speed.kd = 4,
		.speed.integral_max = 8000,
		.speed.out_max = 9000,
		.angle.kp = 0,
		.angle.ki = 0,
		.angle.kd = 0,
		.angle.integral_max = 0,
		.angle.out_max = 8000,
	},
	[CHAS_RF] = {
		.speed.kp = 16,
		.speed.ki = 0.9,
		.speed.kd = 4,
		.speed.integral_max = 8000,
		.speed.out_max = 9000,
		.angle.kp = 0,
		.angle.ki = 0,
		.angle.kd = 0,
		.angle.integral_max = 0,
		.angle.out_max = 8000,
	},
	[CHAS_RB] = {
		.speed.kp = 16,
		.speed.ki = 0.9,
		.speed.kd = 4,
		.speed.integral_max = 8000,
		.speed.out_max = 9000,
		.angle.kp = 0,
		.angle.ki = 0,
		.angle.kd = 0,
		.angle.integral_max = 0,
		.angle.out_max = 8000,
	},
	[GIMB_P] = {
		.speed.kp = -150,//-160
		.speed.ki = 0,
		.speed.kd = 0,
		.speed.integral_max = 8000,
		.speed.out_max = 18000,
		.angle.kp = -0.94,//-1
		.angle.ki = -0.004,//-0.008
		.angle.kd = 0,
		.angle.integral_max = 2000,
		.angle.out_max = 18000,
	},
	[GIMB_Y] = {
		.speed.kp =200,	//700
		.speed.ki = 0.01,		//0
		.speed.kd = 0,
		.speed.integral_max = 10000,
		.speed.out_max = 18000,
		.angle.kp = 0.40,	//0.64
		.angle.ki = 0.01,		//0
		.angle.kd = 0,
		.angle.integral_max = 1000,//0
		.angle.out_max = 18000,
	},
	[FRIC_R] = {
		.speed.kp = 12,
		.speed.ki = 0.2,//0.2
		.speed.kd = 10,
		.speed.integral_max = 5000,
		.speed.out_max = 10000,	
		.angle.kp = 0,
		.angle.ki = 0,
		.angle.kd = 0,
		.angle.integral_max = 0,
		.angle.out_max = 8000,
	},
	[FRIC_L] = {
		.speed.kp = 10,
		.speed.ki = 0.2,
		.speed.kd = 0,
		.speed.integral_max = 6000,
		.speed.out_max = 10000,
		.angle.kp = 0,
		.angle.ki = 0,
		.angle.kd = 0,
		.angle.integral_max = 0,
		.angle.out_max = 8000,
	},
	[DIAL] = {
		.speed.kp = 15,
		.speed.ki = 0.2,
		.speed.kd = 0,
		.speed.integral_max = 8000,
		.speed.out_max = 9500,		//速度环最大输出就是电流值,控制转矩电流范围-10000~10000,对应电机转矩电流-10~10A 
		.angle.kp = 0.25,
		.angle.ki = 0,
		.angle.kd = 0,
		.angle.integral_max = 1000,
		.angle.out_max = 10000,		//角度环的最大输出,就是目标速度的最大值,应该就是10000rpm
	},
	[VIEW] = {
		.speed.kp = 15,
		.speed.ki = 0.2,
		.speed.kd = 0,
		.speed.integral_max = 8000,
		.speed.out_max = 9500,		//速度环最大输出就是电流值,控制转矩电流范围-10000~10000,对应电机转矩电流-10~10A 
		.angle.kp = 0.25,
		.angle.ki = 0,
		.angle.kd = 0,
		.angle.integral_max = 1000,
		.angle.out_max = 10000,		//角度环的最大输出,就是目标速度的最大值,应该就是10000rpm
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
	[GIMB_Y] = {
        .info = &rm_motor_info[GIMB_Y],
        .driver = &rm_motor_driver[GIMB_Y],
        .init = rm_motor_init,
        .update = rm_motor_update,
        .check = rm_motor_check,
        .heart_beat = rm_motor_heart_beat,
        .work_state = DEV_OFFLINE,
	},
	[GIMB_P] = {
        .info = &rm_motor_info[GIMB_P],
        .driver = &rm_motor_driver[GIMB_P],
        .init = rm_motor_init,
        .update = rm_motor_update,
        .check = rm_motor_check,
        .heart_beat = rm_motor_heart_beat,
        .work_state = DEV_OFFLINE,
	},
	[FRIC_L] = {
        .info = &rm_motor_info[FRIC_L],
        .driver = &rm_motor_driver[FRIC_L],
        .init = rm_motor_init,
        .update = rm_motor_update,
        .check = rm_motor_check,
        .heart_beat = rm_motor_heart_beat,
        .work_state = DEV_OFFLINE,
	},
	[FRIC_R] = {
        .info = &rm_motor_info[FRIC_R],
        .driver = &rm_motor_driver[FRIC_R],
        .init = rm_motor_init,
        .update = rm_motor_update,
        .check = rm_motor_check,
        .heart_beat = rm_motor_heart_beat,
        .work_state = DEV_OFFLINE,
	},
	[DIAL] = {
        .info = &rm_motor_info[DIAL],
        .driver = &rm_motor_driver[DIAL],
        .init = rm_motor_init,
        .update = rm_motor_update,
        .check = rm_motor_check,
        .heart_beat = rm_motor_heart_beat,
        .work_state = DEV_OFFLINE,
		.cnt = 0,
		.flag = 0,
	},
	[VIEW] = {
        .info = &rm_motor_info[VIEW],
        .driver = &rm_motor_driver[VIEW],
        .init = rm_motor_init,
        .update = rm_motor_update,
        .check = rm_motor_check,
        .heart_beat = rm_motor_heart_beat,
        .work_state = DEV_OFFLINE,
		.cnt = 0,
		.flag = 0,
	}
};

dm_motor_info_t big_yaw_info = {
	.offline_cnt = 0,
	.offline_max_cnt = 200,
	
};

dm_drv_can_t big_yaw_driver = {
	.CAN_Rx_ID = 0x11,
	.CAN_Tx_ID = 0x10,
};

dm_motor_pid_t big_yaw_pid = {
	.mech_angle_pid = {
		.kp = 0.003,
		.ki = 0,
		.kd = 0,
		.integral_max = 0,
		.out_max = 10,
	},
	.mech_speed_pid = {
		.kp = 1,
		.ki = 0,
		.kd = 0,
		.integral_max = 0,
		.out_max = T_MAX,
	},
	.gyro_speed_pid = {
		.kp = 0.2,
		.ki = 0,
		.kd = 0,
		.integral_max = 0,
		.out_max = T_MAX,
	},
	.single_gyro_speed_pid = {
		.kp = 0.1,
		.ki = 0,
		.kd = 0,
		.integral_max = 0,
		.out_max = 10,
	},
};

dm_motor_t big_yaw = {
	.info = &big_yaw_info,
	.driver = &big_yaw_driver,
	.pid = &big_yaw_pid,
	.work_state = DEV_OFFLINE,
};


/* Exported functions --------------------------------------------------------*/

/**
 * @brief 所有电机pid初始化（清零了积分项） + OFFLINE + 初始化标志位 + 失联检测最大值设定
 * @param  无
 * @retval 无
 * @note  	会改变对应的工作状态
 */
void rm_motor_list_init()
{
	rm_motor_init(&rm_motor[CHAS_LF], motor_pid[CHAS_LF]);
	rm_motor_init(&rm_motor[CHAS_RF], motor_pid[CHAS_RF]);
	rm_motor_init(&rm_motor[CHAS_RB], motor_pid[CHAS_RB]);
	rm_motor_init(&rm_motor[CHAS_LB], motor_pid[CHAS_LB]);
	rm_motor_init(&rm_motor[GIMB_Y], motor_pid[GIMB_Y]);
	rm_motor_init(&rm_motor[GIMB_P], motor_pid[GIMB_P]);
	rm_motor_init(&rm_motor[FRIC_L], motor_pid[FRIC_L]);
	rm_motor_init(&rm_motor[FRIC_R], motor_pid[FRIC_R]);
	rm_motor_init(&rm_motor[DIAL], motor_pid[DIAL]);
	
}

/**
 * @brief 所有电机失联检测
 * @param  无
 * @retval 无
 * @note  	会改变对应的工作状态
 */
void rm_motor_list_heart_beat()
{
	rm_motor_heart_beat(&rm_motor[GIMB_Y]);
	rm_motor_heart_beat(&rm_motor[GIMB_P]);
	rm_motor_heart_beat(&rm_motor[FRIC_L]);
	rm_motor_heart_beat(&rm_motor[FRIC_R]);
	rm_motor_heart_beat(&rm_motor[DIAL]);


}

/**
 * @brief 所有电机工作状态
 * @param  无
 * @retval 全部在线返回1，否则返回0
 */
uint8_t rm_motor_list_workstate()
{
	if(rm_motor[CHAS_LF].work_state == DEV_ONLINE
        && rm_motor[CHAS_RF].work_state == DEV_ONLINE
        && rm_motor[CHAS_RB].work_state == DEV_ONLINE
        && rm_motor[CHAS_LB].work_state == DEV_ONLINE
        && rm_motor[GIMB_Y].work_state == DEV_ONLINE
        && rm_motor[GIMB_P].work_state == DEV_ONLINE
        && rm_motor[FRIC_L].work_state == DEV_ONLINE
        && rm_motor[FRIC_R].work_state == DEV_ONLINE
		&& rm_motor[DIAL].work_state == DEV_ONLINE)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

