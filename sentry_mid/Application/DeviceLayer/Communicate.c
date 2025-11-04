/*
 * @Author: lion
 * @Date: 2025-05-02 07:28:17
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-07-25 05:47:54
 * @FilePath: \sentry_mid\Application\DeviceLayer\Communicate.c
 * @Description: 
 * 
 */
#include "communicate.h"

pid_ctrl_t	chassis_angle = {
	.kp = -0.3,
	.out_max = 8000,
};

pid_ctrl_t	chassis_speed = {
	.kp = 0.4,
	.out_max = 8000,
};

extern CAN_HandleTypeDef hcan2;
extern CAN_HandleTypeDef hcan1;


communicate_rx_up_info_t	communicate_rx_up_info = {
	.ctrl_mode = RC_ctrl,
	.move_mode = MECHANICAL_MODE,
	.start_flag = 0,
};

communicate_chassis_info_t    communicate_chassis_info = {
    .vx = 0,
    .vy = 0,
    .w = 0,
    .power_target = 100,
    .cmd_info.bit.control_cmd = 0,
    .cmd_info.bit.cap_switch = 0,
    .cmd_info.bit.wheel_flag = 0,
    .cmd_info.bit.unuse = 0,
};

communicate_tx_up_info_t	communicate_tx_up_info;

communicate_t   communicate = {
    .offline_cnt = 0,
    .offline_cnt_max = 200,
    .work_state = DEV_OFFLINE,
	.communicate_rx_up_info = &communicate_rx_up_info,
    .communicate_chassis_info = &communicate_chassis_info,
	.communicate_tx_up_info = &communicate_tx_up_info,
};

uint8_t Nav_chassis_info[8];

int32_t big_yaw_toangle;
float Angle_Error;

float vx_test,vy_test;
uint16_t target_base_yaw;

void Communicate_send_chassis(communicate_t *communicate)
{
	/*ʇ·񿪆��ӂݪ/
    if(Strategy.vision->mode.navigate_mode == 1)
	{
		communicate->communicate_chassis_info->cmd_info.bit.wheel_flag = 1;
	}
	else if(Strategy.vision->mode.navigate_mode == 0)
	{
		communicate->communicate_chassis_info->cmd_info.bit.wheel_flag = 0;
	}
	/**	µׅ̗ªµ½̘¶¨½ǶȠ*/

	
	
	/*Ԇ̨תµ½̘¶¨½ǶȪ/
	else if(Strategy.vision->mode.navigate_mode == 2)
	{
		communicate->communicate_chassis_info->cmd_info.bit.wheel_flag = 0;
		uint16_t base_yaw= Strategy.vision->speed.base_yaw;
		float	target_yaw = Strategy.vision->speed.current_yaw;
		big_yaw_toangle = BIG_YAW_MID;
		if(big_yaw_toangle >= 65535)
		{
			big_yaw_toangle -= 65535;
		}
		if(big_yaw_toangle <= 0)
		{
			big_yaw_toangle += 65535;
		}
		chassis_angle.measure = big_yaw.info->angle;
		chassis_angle.target = big_yaw_toangle;
		chassis_angle.err = big_yaw_toangle - big_yaw.info->angle;
		if(chassis_angle.err < 0)
		{
				chassis_angle.err += 65535;
		}
		if(chassis_angle.err > 32767.5)
		{
			chassis_angle.err -= 65535;
		}

		single_pid_ctrl(&chassis_angle);
		
		chassis_speed.err = chassis_angle.out - imu_sensor.info->base_info.rate_yaw;	

		single_pid_ctrl(&chassis_speed);
		
		communicate->communicate_chassis_info->w = chassis_speed.out;
	}
	
	
	/*ʇ·񿪆��窯
	if(Strategy.vision->mode.electronic_mode == 1)
	{
		communicate->communicate_chassis_info->cmd_info.bit.cap_switch = 1;
	}
	else
	{
		communicate->communicate_chassis_info->cmd_info.bit.cap_switch = 0;
	}
	
    Angle_Error = (big_yaw.info->p_int - BIG_YAW_MID) * 2*PI/65535 ;
	target_base_yaw = Strategy.vision->speed.base_yaw*65535/8192.0f+65535+BIG_YAW_MID;
	if(target_base_yaw > 65535)
	{
		target_base_yaw -= 65535;
	}

//	if(Strategy.work_state == DEV_ONLINE && Strategy.CarData->ce.game_start == 2)
//	if(Strategy.work_state == DEV_ONLINE && slave.info->game_progress == 4)
//	{
		communicate->communicate_chassis_info->vx = -(Strategy.vision->speed.vx) *arm_cos_f32(Angle_Error) + (Strategy.vision->speed.vy) * arm_sin_f32(Angle_Error);
		communicate->communicate_chassis_info->vy = +(Strategy.vision->speed.vx) * arm_sin_f32(Angle_Error) + (Strategy.vision->speed.vy) * arm_cos_f32(Angle_Error);
//		communicate->communicate_chassis_info->w = Strategy.vision->speed.vw;
	if(Strategy.vision->mode.navigate_mode == 0 && Strategy.CarData->ce.game_start == 2)
	{
		communicate->communicate_chassis_info->w = chassis_follow(&chassis,target_base_yaw);
	}
//	else
//	{
//		communicate->communicate_chassis_info->w = 0;
//	}
		
//	}
	
//	if(Strategy.CarData->ce.game_start == 1 || Strategy.CarData->ce.game_start==0)
//	{
//		communicate->communicate_chassis_info->vx = 0;
//		communicate->communicate_chassis_info->vy = 0;
//	}
//	/*µ¼º½µ􏟔­µؐ¡͓*/
	else if(Strategy.work_state == DEV_OFFLINE && Strategy.CarData->ce.game_start == 2)
	{	
		communicate->communicate_chassis_info->vx = 0;
		communicate->communicate_chassis_info->vy = 0;
		communicate->communicate_chassis_info->cmd_info.bit.wheel_flag = 1;
	}
	
	memcpy(Nav_chassis_info,communicate->communicate_chassis_info,8);
	
    CAN_TxHeaderTypeDef tx_message;
	
	uint32_t send_mail_box;
	tx_message.StdId = NAV_CHASSIS_SPEED;
	tx_message.IDE = CAN_ID_STD;
	tx_message.RTR = CAN_RTR_DATA;
	tx_message.DLC = 0x08;

	HAL_CAN_AddTxMessage(&hcan2, &tx_message, Nav_chassis_info, &send_mail_box);
}
uint8_t test_size;
//CAN1
void Communicate_send_up_info(uint8_t gimbal_mode,uint8_t vision_mode,float current_yaw,float ladar_x,float ladar_y)
{
	test_size = sizeof(communicate.communicate_tx_up_info->gimbal_info);	
	uint8_t send_up_info[8];

	/*传递云台运动状态*/
	communicate.communicate_tx_up_info->gimbal_info.gimbal_mode = gimbal_mode;
	communicate.communicate_tx_up_info->gimbal_info.vision_mode = vision_mode;
	/*传递雷达数据yaw*/
	communicate.communicate_tx_up_info->ladar_info.current_yaw = current_yaw;
	/*传递雷达xy坐标*/
	communicate.communicate_tx_up_info->ladar_info.ladar_x = ladar_x*1000;
	communicate.communicate_tx_up_info->ladar_info.ladar_y = ladar_y*1000;
	/*传递云台速度信息 */
	communicate.communicate_tx_up_info->gimbal_info.current_vx = Strategy.vision->speed.current_vx*10000;
	communicate.communicate_tx_up_info->gimbal_info.current_vy = Strategy.vision->speed.current_vy*10000;
	/*发送云台特殊模式下转向特定角度*/
	communicate.communicate_tx_up_info->gimbal_info.gimbal_target_yaw = Strategy.vision->speed.tripod_yaw;

	CAN_TxHeaderTypeDef tx_message;
    
    uint32_t send_mail_box;   
	
	memcpy(send_up_info,&communicate.communicate_tx_up_info->ladar_info,8);
	
	tx_message.StdId = NAV_SEND_LADAR_INFO;
    tx_message.IDE = CAN_ID_STD;
    tx_message.RTR = CAN_RTR_DATA;
    tx_message.DLC = 0x08;

    HAL_CAN_AddTxMessage(&hcan1, &tx_message, send_up_info, &send_mail_box);
	
	memcpy(send_up_info,&communicate.communicate_tx_up_info->gimbal_info,8);
	
	tx_message.StdId = NAV_SEND_UP_INFO;
    tx_message.IDE = CAN_ID_STD;
    tx_message.RTR = CAN_RTR_DATA;
    tx_message.DLC = 0x08;

    HAL_CAN_AddTxMessage(&hcan1, &tx_message, send_up_info, &send_mail_box);
}

void get_up_info(communicate_t *communicate,uint8_t *rxBuf)
{
    communicate->communicate_rx_up_info->start_flag = rxBuf[2];
    communicate->communicate_rx_up_info->ctrl_mode = rxBuf[0];
    communicate->communicate_rx_up_info->move_mode = rxBuf[1];
	
	memcpy(&communicate->communicate_rx_up_info->up_imu_to_mech,rxBuf+5,2);

    communicate->work_state = DEV_ONLINE;
}