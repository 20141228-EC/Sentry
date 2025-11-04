#include "KT_motor.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

void kt_single_pid_ctrl(kt_pid_ctrl_t *pid)
{
    // 保存误差值(需要在外面自行计算误差)
	//pid->err = err;
    // 积分
    pid->integral += pid->err;
    pid->integral = constrain(pid->integral, -pid->integral_max, +pid->integral_max);
    // p i d 输出项计算
    pid->pout = pid->kp * pid->err;
    pid->iout = pid->ki * pid->integral;
    pid->dout = pid->kd * (pid->err - pid->last_err);
    // 累加pid输出值
    pid->out = pid->pout + pid->iout + pid->dout;
    pid->out = constrain(pid->out, -pid->out_max, pid->out_max);
    // 记录上次误差值
    pid->last_err = pid->err;
}

void Kt_motor_init(kt_motor_t *motor, kt_pid_t *motor_pid, kt_driver_t *driver)
{
    motor->driver = driver;
    motor->motor_pid = motor_pid;
}

void kt_get_rx_info(kt_motor_t *motor, uint8_t *rxBuf)
{
    kt_rx_info_t *motor_info = motor->info->rx_info;
    motor_info->temperature = (int8_t)rxBuf[1];
    motor_info->current = (int16_t)(rxBuf[3] << 8 | rxBuf[2]);
    motor_info->speed = (int16_t)(rxBuf[5] << 8 | rxBuf[4]);
    motor_info->angle = (int16_t)(rxBuf[7] << 8 | rxBuf[6]);
}
uint8_t data[8];
void kt_send_control_info(kt_motor_t *motor, int16_t current)
{
    kt_tx_info_t *motor_info = motor->info->tx_info;
    motor_info->current = current;

    
//    data[0] = CLOSE_KT_CURRENT;
//    data[4] = current;
//    data[5] = current >> 8;
	data[6] = current;
	data[7] = current>>8;
    
    if(motor->driver->id == DRV_CAN1)
    {
        CAN_SendData(&hcan1,motor->driver->tx_id,data);
    }
    else if(motor->driver->id == DRV_CAN2)
    {
        CAN_SendData(&hcan2,motor->driver->tx_id,data);
    }
   
}
uint8_t data[8] = {0};
void kt_send_request_info(kt_motor_t *motor)
{
    
    data[0] = GET_KT_STATE2;

    if(motor->driver->id == DRV_CAN1)
    {
        CAN_SendData(&hcan1,motor->driver->tx_id,data);
    }
    else if(motor->driver->id == DRV_CAN2)
    {
        CAN_SendData(&hcan2,motor->driver->tx_id,data);
    }
}

/**
 * @brief  电机单环pid控制速度
 * @param  motor: 电机结构体
 * @param  speed: 速度目标值
 * @retval None
 */
void KT_Motor_ToSpeed(kt_motor_t *motor, float speed)
{
	motor->motor_pid->vel_speed.target = speed;
	motor->motor_pid->vel_speed.measure = imu_sensor.info->base_info.rate_yaw;
	motor->motor_pid->vel_speed.err = speed -imu_sensor.info->base_info.rate_yaw;
	kt_single_pid_ctrl(&motor->motor_pid->vel_speed);
	
//	//如果输出值为最大电流值，标志位加1
//	if(motor->motor_pid->vel_speed.out == motor->motor_pid->vel_speed.out_max)
//	{
//		motor->info->output_time++;
//		//限制最大输出标志位最大值
//		if(motor->info->output_time >= motor->info->output_time_max)
//		{
//			motor->info->output_time = motor->info->output_time_max;
//		}
//	}
//	//如果输出不为最大值，清零最大输出标志位
//	else if(motor->motor_pid->vel_speed.out <= motor->motor_pid->vel_speed.out_max - 200)
//	{
//		motor->info->output_time = 0;
//	}
//	
//	//输出电流前提：输出最大值电流值标志位小于最大输出标志位最大值-100？
//	if(motor->work_state == DEV_ONLINE && motor->info->output_time < motor->info->output_time < motor->info->output_time_max-100)
    if(motor->work_state == DEV_ONLINE)
	{
        kt_send_control_info(motor, (int16_t)motor->motor_pid->vel_speed.out);
    }
}
int32_t err_test;

float Lowpass_kt(int16_t X_last, int16_t X_new, float K)
{
	return (X_last + (X_new - X_last) * K);
}
/**
 * @brief  电机双环pid控制角度
 * @param  motor: 电机结构体
 * @param  angle: 角度目标值
 * @retval None
 */
void KT_Motor_ToAngle(kt_motor_t *motor, uint32_t angle)
{
	static float last_speed;
	motor->motor_pid->pos_angle.target = angle;
    motor->motor_pid->pos_angle.measure = motor->info->rx_info->angle;
	motor->motor_pid->pos_angle.err = angle - motor->info->rx_info->angle;
	err_test = angle - motor->info->rx_info->angle;
	if(motor->motor_pid->pos_angle.err < 0)
	{
		motor->motor_pid->pos_angle.err += 65535;
	}
	if(motor->motor_pid->pos_angle.err > 32767.5)
	{
		motor->motor_pid->pos_angle.err = motor->motor_pid->pos_angle.err - 65535;
	}

	//角度环
	kt_single_pid_ctrl(&motor->motor_pid->pos_angle);
    motor->motor_pid->pos_speed.target = motor->motor_pid->pos_angle.out;
    motor->motor_pid->pos_speed.measure = imu_sensor.info->base_info.rate_yaw;
	last_speed = imu_sensor.info->base_info.rate_yaw;
	motor->motor_pid->pos_speed.err = motor->motor_pid->pos_angle.out - imu_sensor.info->base_info.rate_yaw;
	//速度环
	single_pid_ctrl(&motor->motor_pid->pos_speed);
	//发送数据到数组
    if(motor->work_state == DEV_ONLINE)
    {
        kt_send_control_info(motor, (int16_t)motor->motor_pid->pos_speed.out);
    }
}

void KT_Motor_HeartBeat(kt_motor_t *motor)
{
    motor->info->offline_cnt++;
    if(motor->info->offline_cnt > motor->info->offline_max_cnt) 
    {
        motor->info->offline_cnt = motor->info->offline_max_cnt;
        motor->work_state = DEV_OFFLINE;
                
    }
    else 
    {
        if(motor->work_state == DEV_OFFLINE)
        motor->work_state = DEV_ONLINE;
                
    }
}


