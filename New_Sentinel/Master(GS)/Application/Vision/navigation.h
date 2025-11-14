#ifndef __NAVIGATION_H
#define __NAVIGATION_H

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdbool.h>
#include "crc.h"
#include "vision.h"
/* 宏定义与全局变量 ------------------------------------------------------------*/


/* 指令与配置 ------------------------------------------------------------*/



/* 数据长度 */
typedef enum {
	/* Std */
	LEN_NAVIGATION_RX_PACKET	= 31 + 5,	// 接收包整包长度
	LEN_NAVIGATION_TX_PACKET	= 26 + 5,	// 发送包整包长度

	LEN_NAVIGATION_RX_DATA 			= LEN_VISION_RX_PACKET - 5,	// 接收数据段长度
	LEN_NAVIGATION_TX_DATA 			= LEN_VISION_TX_PACKET - 5,	// 发送数据段长度
	
	LEN_NAVIGATION_FRAME_HEADER 	  	= 3,	// 帧头长度
	LEN_NAVIGATION_FRAME_TAILER 		= 2,	// 帧尾CRC16
	
}navigation_data_length_t;


/* 数据包格式 ------------------------------------------------------------*/


/* 发送数据段格式 */
typedef struct __packed 
{
	/*大YAW轴*/
	int16_t     B_yaw_angle;
	
	float   uwb_x;
	float   uwb_y;
	uint8_t game_progress;
	uint8_t my_color;
	uint8_t ammor_size_3;
	uint8_t ammor_size_4;
	uint8_t ammor_size_5;
	
	uint8_t nuc_flag1;    //是否处于自瞄大胆状态
	uint8_t navi_enable;  //是否开启避障
	uint8_t omni_enable;  //是否开启全向感知
	
	
	float target_x;
	float target_y;

}navigation_tx_data_t;/*做好视觉数据突然掉线的准备*/


typedef struct __packed 
{
	int16_t   chassis_front; //底盘前进速度 
	int16_t   chassis_right; //底盘平移速度
	
	int16_t  yaw_angle;     //当前Yaw轴角度
	
	
	/*双机通讯*/
	uint8_t enemy1_num;
	int16_t enemy1_x;
	int8_t  enemy1_y;
	int16_t enemy1_z;
	
	uint8_t enemy2_num;
	int16_t enemy2_x;
	int8_t  enemy2_y;
	int16_t enemy2_z;
	
	uint8_t enemy3_num;
	int16_t enemy3_x;
	int8_t  enemy3_y;
	int16_t enemy3_z;
	
	uint8_t enemy4_num;
	int16_t enemy4_x;
	int8_t  enemy4_y;
	int16_t enemy4_z;
	
	uint8_t arrive_flag;  //到达标志位
	
}navigation_rx_data_t;

/* 发送包格式 */
typedef struct __packed
{
	vision_frame_header_t   FrameHeader;	// 帧头
	navigation_tx_data_t	TxData;		    // 数据
	vision_frame_tailer_t   FrameTailer;	// 帧尾	
}navigation_tx_packet_t;

/* 接收包格式 */
typedef struct __packed 
{
	vision_frame_header_t   FrameHeader;	// 帧头
	navigation_rx_data_t	RxData;		    // 数据
	vision_frame_tailer_t   FrameTailer;	// 帧尾	
} navigation_rx_packet_t;


/* 工作模式 ------------------------------------------------------------*/
/* 汇总 ------------------------------------------------------------*/

typedef  struct 
{
	navigation_rx_packet_t *rx_pack;
	navigation_tx_packet_t *tx_pack;
	vision_state_t          state;
	
	
}navigation_t;

/* 相关函数 ------------------------------------------------------------*/

void Navigation_Init(void);
void Navigation_Update(void);

bool Navigation_SendData(void);
bool Navigation_GetData(uint8_t *rxBuf);

void Navigation_task(void);

#endif
