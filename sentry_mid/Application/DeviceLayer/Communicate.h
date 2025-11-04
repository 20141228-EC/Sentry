/*
 * @Author: lion
 * @Date: 2025-05-02 06:55:29
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-07-26 23:05:46
 * @FilePath: \sentry_mid\Application\DeviceLayer\Communicate.h
 * @Description: 
 * 
 */
#ifndef __COMMUNICATE_H
#define __COMMUNICATE_H


#include "Strategy.h"
#include "rp_config.h"
#include "can_protocol.h"
#include "Robot.h"

//can2
#define NAV_CHASSIS_SPEED 0x001
//can1
#define NAV_SEND_UP_INFO 0x006
#define NAV_SEND_LADAR_INFO	0x007

typedef  struct  communicate_chassis_info_struct
{
    int16_t vx;
    int16_t vy;
    int16_t w;

    uint8_t	power_target;	

    union
	{
		uint8_t cmd;
		
		struct  master_cmd_t
		{
			uint8_t control_cmd:	1;	//	控制开�?	0	�?	1开
			uint8_t	cap_switch:		1;	//	超电开�?	0	�?	1开
			
			uint8_t	wheel_flag:		1;
			uint8_t unuse:			5;
		}	bit;
	}	cmd_info;
    /* data */
}communicate_chassis_info_t;

typedef struct communicate_rx_enemy_pos_struct
{
	uint16_t enemy_x;
	uint16_t enemy_y;
}communciate_rx_enemy_pos_t;

typedef struct communicate_rx_up_info_struct
{
    uint8_t ctrl_mode;	
    uint8_t move_mode;	
    uint8_t start_flag; 
	uint8_t	vision_flag;
	uint16_t	up_yaw_info;
	uint16_t up_imu_to_mech;
	
	communciate_rx_enemy_pos_t enemy_pos;	//敌人坐标信息
}  communicate_rx_up_info_t;

typedef struct communicate_tx_up_info_struct
{
	struct  gimbal_info_struct
	{
		uint8_t gimbal_mode;
		uint8_t vision_mode;

		int16_t current_vx;
		int16_t current_vy;
		
		uint16_t gimbal_target_yaw;
	}gimbal_info;
	
	struct	ladar_info_struct
	{
		uint16_t 	ladar_x;
		uint16_t 	ladar_y;
		float   current_yaw;
	}ladar_info;
	
	
}communicate_tx_up_info_t;

typedef struct communicate_struct
{
    communicate_chassis_info_t *communicate_chassis_info;
    communicate_rx_up_info_t *communicate_rx_up_info;	//	上位机信�?
	communicate_tx_up_info_t	*communicate_tx_up_info;

    uint16_t					offline_cnt;
	uint16_t					offline_cnt_max;

    dev_work_state_t work_state;
}communicate_t;




void Communicate_send_chassis_speed(int16_t x, int16_t y);
void get_up_info(communicate_t *communicate,uint8_t *rxBuf);
void Communicate_send_up_info(uint8_t gimbal_mode,uint8_t vision_mode,float current_yaw,float ladar_x,float ladar_y);

extern communicate_t   communicate;


#endif