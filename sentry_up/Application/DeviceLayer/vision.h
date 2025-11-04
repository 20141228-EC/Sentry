/*
 * @Author: lion
 * @Date: 2025-04-16 20:29:46
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-07-25 04:42:46
 * @FilePath: \sentry_up\Application\DeviceLayer\vision.h
 * @Description: 
 * 
 */
#ifndef __VISION_H
#define __VISION_H

/* Includes ------------------------------------------------------------------*/
#include "drv_tick.h"

#include "rp_config.h"
#include "rp_math.h"
#include "string.h"
#include "crc.h"
#include "usbd_cdc_if.h"


/* Exported types ------------------------------------------------------------*/
typedef enum {
	AIM_OFF 		    = 0x00,	// 不启动自瞄(视觉不跑代码
	AIM_ON			    = 0x01,	// 启动自瞄
	AIM_OUTPOST			= 0x02,	// 击打前哨
	AIM_SMALL_BUFF		= 0x03,	// 识别小符
	AIM_BIG_BUFF	  	= 0x04,	// 识别大符
}vision_cmd_e;

typedef __packed struct 
{
	uint8_t  			SOF;
	vision_cmd_e  		mode;        			// data u8_1
	uint8_t  			CRC8;
	
	uint8_t  			my_color;    			// data u8_3	0:红	1:蓝 12

	float    			yaw_angle;	 			// data f_1
	float    			pitch_angle; 			// data f_2  11
	float				roll_angle;

	float 				small_motor_yaw;
	
	uint16_t 			ladar_x;
	uint16_t			ladar_y;
	float				ladar_yaw;

	float 				ladar_vx;
	float				ladar_vy;
	

	uint16_t				blood_1;
	uint16_t				blood_2;
	uint16_t				blood_3;
	uint16_t				blood_4;
	uint16_t				blood_5;
	uint16_t				blood_6;
	uint16_t				blood_7;
	uint16_t				blood_8;
		
	float 				yaw_v;
	float				pitch_v;	
	
	uint8_t				start_game;

	uint16_t 			CRC16;              //28
}Vision_tx_info_t;  //Byte: 28

typedef __packed struct 
{
	uint8_t			  	SOF;
	vision_cmd_e  		mode;           // data u8_1
	uint8_t 			CRC8;
	
	float   			yaw_angle;    // data f_1
	float    			pitch_angle;      // data f_2
	
	uint8_t  			is_find_target; // data u8_2
	uint8_t				is_find_strategy;
	uint8_t  			fire_cnt;  // data u8_4
	uint16_t				time;
	
	float				enemy_x;
	float 			enemy_y;

	
	uint8_t 			robot_num;
	
	uint16_t 			CRC16;
}vision_rx_info_t;							// 接收视觉数据

typedef struct
{
	Vision_tx_info_t  *tx_info;
	vision_rx_info_t  *rx_info;

	//-------- 发送视觉数据 begin--------//
	vision_cmd_e  		cmd_mode;
	uint8_t  			color;

	//-------- 接收视觉数据 begin--------//
	vision_cmd_e  mode;

	float    target_pitch_angle;
	float    target_yaw_angle;
	uint8_t  is_find_target;
	uint8_t  fire_cnt;
	//-------- 接收视觉数据 end--------//
	
	//-------- 通信 begin--------//
	uint8_t  rx_flag;
	int16_t	 offline_cnt;
	int16_t	 offline_max_cnt;
	//-------- 通信 end--------//

}vision_info_t;

typedef struct vision_struct {
	vision_info_t	    *info;
	drv_uart_t		    *driver;
	
	dev_work_state_t	work_state;
	
	void				     (*init)(struct vision_struct *self);
	void				     (*update)(struct vision_struct *self, uint8_t *rxBuf);
	void				     (*check)(struct vision_struct *self);	
	void				     (*heartbeat)(struct vision_struct *self);
	
} Vision_t;

extern Vision_t vision;
extern bool vision_send_data(void);

#endif
