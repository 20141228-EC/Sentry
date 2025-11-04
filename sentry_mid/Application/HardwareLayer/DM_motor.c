/*
 * @Author: lion
 * @Date: 2025-07-12 20:07:58
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-08-05 11:26:00
 * @FilePath: \sentry_mid\Application\HardwareLayer\DM_motor.c
 * @Description: 
 * 
 */
#include "DM_motor.h"

int float_to_uint(float x_float, float x_min, float x_max, int bits);
float uint_to_float(int x_int, float x_min, float x_max, int bits);
										

/**  @brief  激活电机
 *   @param  dm_motor: 电机结构体
 *   @note 放在检测任务中，心跳检测中，当离线的时候失能，在线使能
 **/
void dm_active_motor(dm_motor_t *dm_motor)
{
	uint8_t data[8];
	
	data[0] = 0xFF;
	data[1] = 0xFF;
	data[2] = 0xFF;
	data[3] = 0xFF;
	data[4] = 0xFF;
	data[5] = 0xFF;
	data[6] = 0xFF;
	data[7] = 0xFC;

    CAN2_SendData(dm_motor->driver->CAN_Tx_ID, data);
}

/**
 * @brief  休眠电机
 * @param  dm_motor: 电机结构体
 * @note 放在检测任务中，心跳检测中，当离线的时候失能，在线使能
 **/
void dm_sleep_motor(dm_motor_t *dm_motor)
{
	uint8_t data[8];
	
	data[0] = 0xFF;
	data[1] = 0xFF;
	data[2] = 0xFF;
	data[3] = 0xFF;
	data[4] = 0xFF;
	data[5] = 0xFF;
	data[6] = 0xFF;
	data[7] = 0xFD;

    CAN2_SendData(dm_motor->driver->CAN_Tx_ID, data);
}

/**
************************************************************************
* @brief:      	clear_err: 清除电机错误函数
* @param[in]:   hcan:     指向CAN_HandleTypeDef结构的指针
* @param[in]:   motor_id: 电机ID，指定目标电机
* @param[in]:   mode_id:  模式ID，指定要清除错误的模式
* @retval:     	void
* @details:    	通过CAN总线向特定电机发送清除错误的命令。
************************************************************************
**/
void dm_clear_err(dm_motor_t *dm_motor)
{
	uint8_t data[8];
	
	data[0] = 0xFF;
	data[1] = 0xFF;
	data[2] = 0xFF;
	data[3] = 0xFF;
	data[4] = 0xFF;
	data[5] = 0xFF;
	data[6] = 0xFF;
	data[7] = 0xFB;

	CAN2_SendData(dm_motor->driver->CAN_Tx_ID, data);
}

/**
 * @brief  电机心跳失联检测
 * @param  dm_motor: 电机结构体
 * @retval 无
 * @note 放在检测任务中，心跳检测中，当离线的时候失能，在线使能
 */
void dm_motor_heart_beat(dm_motor_t *dm_motor)
{
    dm_motor_info_t *dm_motor_info = dm_motor->info;
    dm_motor_info->offline_cnt++;
    if(dm_motor_info->offline_cnt > dm_motor_info->offline_max_cnt) 
	{
        dm_motor_info->offline_cnt = dm_motor_info->offline_max_cnt;
        dm_motor->work_state = DEV_OFFLINE;
    }
    else 
	{
        if(dm_motor->work_state == DEV_OFFLINE)
        dm_motor->work_state = DEV_ONLINE;
    }
}

/**
 * @brief  更新电机状态 
 * @param  dm_motor: 电机结构体
 * @param  rxBuf: 接收的CAN数据缓冲区
 * @retval 无
 * @note 解析接收到的CAN数据，更新电机状态信息
 */

void dm_motor_update(dm_motor_t *dm_motor,uint8_t *rxBuf)
{
	dm_motor->info->id = (rxBuf[0])&0x0F;
	dm_motor->info->state = (rxBuf[0])>>4;
	
	//清除错误标志位（目前只遇到通讯丢失标志位）
	if(dm_motor->info->state == 0x0D)
	{
		dm_clear_err(dm_motor);
	}
	
	//原始数据
	dm_motor->info->p_int = (rxBuf[1]<<8)|rxBuf[2];
	dm_motor->info->v_int = (rxBuf[3]<<4)|(rxBuf[4]>>4);
	dm_motor->info->t_int = ((rxBuf[4]&0xF)<<8)|rxBuf[5];
	//转化为浮点数处理值
	dm_motor->info->pos = uint_to_float(dm_motor->info->p_int, P_MIN, P_MAX, 16); // (-12.5,12.5)
	dm_motor->info->vel = uint_to_float(dm_motor->info->v_int, V_MIN, V_MAX, 12); // (-45.0,45.0)
	dm_motor->info->tor = uint_to_float(dm_motor->info->t_int, T_MIN, T_MAX, 12);  // (-18.0,18.0)
	dm_motor->info->Tmos = (float)(rxBuf[6]);
	dm_motor->info->Tcoil = (float)(rxBuf[7]);
	
	//更新数据清零离线计数值
	dm_motor->info->offline_cnt = 0;
}

/**
************************************************************************
* @brief:      	mit_ctrl: MIT模式下的电机控制函数
* @param[in]:   hcan:			指向CAN_HandleTypeDef结构的指针，用于指定CAN总线
* @param[in]:   motor_id:	电机ID，指定目标电机
* @param[in]:   pos:			位置给定值
* @param[in]:   vel:			速度给定值
* @param[in]:   kp:				位置比例系数
* @param[in]:   kd:				位置微分系数
* @param[in]:   torq:			转矩给定值
* @retval:     	void
* @details:    	通过CAN总线向电机发送MIT模式下的控制帧。
* @note:		本项目一直使用纯控制电流的方式使用（即第二第三个参数直接置0，只根据自身在外部函数计算出来的电流值进行控制）
************************************************************************
**/
void dm_motor_mit_ctrl(dm_motor_t *dm_motor, float pos, float vel,float kp, float kd,float torq)
{
	uint8_t data[8];
	uint16_t pos_tmp,vel_tmp,kp_tmp,kd_tmp,tor_tmp;
	uint16_t id = dm_motor->driver->CAN_Tx_ID;

	pos_tmp = float_to_uint(pos,  P_MIN,  P_MAX,  16);
	vel_tmp = float_to_uint(vel,  V_MIN,  V_MAX,  12);
	kp_tmp  = float_to_uint(kp,   KP_MIN, KP_MAX, 12);
	kd_tmp  = float_to_uint(kd,   KD_MIN, KD_MAX, 12);
	tor_tmp = float_to_uint(torq, T_MIN, T_MAX, 12);

	data[0] = (pos_tmp >> 8);
	data[1] = pos_tmp;
	data[2] = (vel_tmp >> 4);
	data[3] = ((vel_tmp&0xF)<<4)|(kp_tmp>>8);
	data[4] = kp_tmp;
	data[5] = (kd_tmp >> 4);
	data[6] = ((kd_tmp&0xF)<<4)|(tor_tmp>>8);
	data[7] = tor_tmp;
	
	CAN2_SendData(id, data);
}

/**
 * @brief  电机速度控制（MIT控制电流）
 * @param  dm_motor: 电机结构体
 * @param  speed: 速度目标值
 * @retval 无
 * @note   速度环使用的观测值为imu角速度值（可以使得响应好一点）
 */
void dm_motor_tospeed(dm_motor_t *dm_motor, float speed)
{
    dm_motor->pid->single_gyro_speed_pid.target = speed;
    dm_motor->pid->single_gyro_speed_pid.measure = imu_sensor.info->base_info.rate_yaw;
    dm_motor->pid->single_gyro_speed_pid.err = speed - imu_sensor.info->base_info.rate_yaw;
    //速度环
    single_pid_ctrl(&dm_motor->pid->single_gyro_speed_pid);
    //发送数据到数组
    dm_motor_mit_ctrl(dm_motor,0,0,0,
                    0,dm_motor->pid->single_gyro_speed_pid.out);
}
float last_yaw;
/**
 * @brief  电机角度控制（MIT控制电流）
 * @param  dm_motor: 电机结构体
 * @param  angle: 角度目标值
 * @retval 无
 * @note 这里没有继续使用imu角速度作为速度环的观测值，其实可以使用
 */
void dm_motor_toangle(dm_motor_t *dm_motor, float angle)
{
    dm_motor->pid->mech_angle_pid.target = angle;
    dm_motor->pid->mech_angle_pid.measure = dm_motor->info->p_int;
    dm_motor->pid->mech_angle_pid.err = dm_motor->pid->mech_angle_pid.target - dm_motor->pid->mech_angle_pid.measure;
	 if(dm_motor->pid->mech_angle_pid.err < 0)
	{
			dm_motor->pid->mech_angle_pid.err += 65535;
	}
	if(dm_motor->pid->mech_angle_pid.err > 65535/2)
	{
		dm_motor->pid->mech_angle_pid.err -= 65535;
	}
    //角度环
    single_pid_ctrl(&dm_motor->pid->mech_angle_pid);

    //速度环
	
    dm_motor->pid->mech_speed_pid.target = dm_motor->pid->mech_angle_pid.out;
//    dm_motor->pid->mech_speed_pid.measure = Lowpass(last_yaw,imu_sensor.info->base_info.rate_yaw,0.8);
	dm_motor->pid->mech_speed_pid.measure = dm_motor->info->vel;
	last_yaw = imu_sensor.info->base_info.rate_yaw;
    dm_motor->pid->mech_speed_pid.err = dm_motor->pid->mech_angle_pid.out - dm_motor->info->vel;
    single_pid_ctrl(&dm_motor->pid->mech_speed_pid);
    //发送数据到数组
    dm_motor_mit_ctrl(dm_motor,0,0,0,0,dm_motor->pid->mech_speed_pid.out);
	
}



/**
************************************************************************
* @brief:      	float_to_uint: 浮点数转换为无符号整数函数
* @param[in]:   x_float:	待转换的浮点数
* @param[in]:   x_min:		范围最小值
* @param[in]:   x_max:		范围最大值
* @param[in]:   bits: 		目标无符号整数的位数
* @retval:     	无符号整数结果
* @details:    	将给定的浮点数 x 在指定范围 [x_min, x_max] 内进行线性映射，映射结果为一个指定位数的无符号整数
************************************************************************
**/
int float_to_uint(float x_float, float x_min, float x_max, int bits)
{
	/* Converts a float to an unsigned int, given range and number of bits */
	float span = x_max - x_min;
	float offset = x_min;
	return (int) ((x_float-offset)*((float)((1<<bits)-1))/span);
}

/**
************************************************************************
* @brief:      	uint_to_float: 无符号整数转换为浮点数函数
* @param[in]:   x_int: 待转换的无符号整数
* @param[in]:   x_min: 范围最小值
* @param[in]:   x_max: 范围最大值
* @param[in]:   bits:  无符号整数的位数
* @retval:     	浮点数结果
* @details:    	将给定的无符号整数 x_int 在指定范围 [x_min, x_max] 内进行线性映射，映射结果为一个浮点数
************************************************************************
**/
float uint_to_float(int x_int, float x_min, float x_max, int bits)
{
	/* converts unsigned int to float, given range and number of bits */
	float span = x_max - x_min;
	float offset = x_min;
	return ((float)x_int)*span/((float)((1<<bits)-1)) + offset;
}