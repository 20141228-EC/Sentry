#include "KT_motor.h"

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

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

void kt_send_control_info(kt_motor_t *motor, int16_t current)
{
    kt_tx_info_t *motor_info = motor->info->tx_info;
    motor_info->current = current;

    uint8_t data[8];
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
void KT_Motor_ToSpeed(kt_motor_t *motor, int16_t speed)
{
	motor->motor_pid->vel_speed.target = speed;
	motor->motor_pid->vel_speed.measure = motor->info->rx_info->speed;
	motor->motor_pid->vel_speed.err = speed - motor->info->rx_info->speed;
	single_pid_ctrl(&motor->motor_pid->vel_speed);
	
	if(motor->work_state == DEV_ONLINE)
    {
        kt_send_control_info(motor, motor->motor_pid->vel_speed.out);
    }
}

/**
 * @brief  电机双环pid控制角度
 * @param  motor: 电机结构体
 * @param  angle: 角度目标值
 * @retval None
 */
void KT_Motor_ToAngle(kt_motor_t *motor, int16_t angle)
{
	motor->motor_pid->pos_angle.target = angle;
    motor->motor_pid->pos_angle.measure = motor->info->rx_info->angle;
	motor->motor_pid->pos_angle.err = angle - motor->info->rx_info->angle;

	//角度环
	single_pid_ctrl(&motor->motor_pid->pos_angle);
    motor->motor_pid->pos_speed.target = motor->motor_pid->pos_angle.out;
    motor->motor_pid->pos_speed.measure = motor->info->rx_info->speed;
	motor->motor_pid->pos_speed.err = motor->motor_pid->pos_angle.out - motor->info->rx_info->speed;
	//速度环
	single_pid_ctrl(&motor->motor_pid->pos_speed);
	//发送数据到数组
    if(motor->work_state == DEV_ONLINE)
    {
        kt_send_control_info(motor, motor->motor_pid->pos_speed.out);
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


