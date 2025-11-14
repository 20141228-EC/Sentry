#ifndef __NAVIGATION_H
#define __NAVIGATION_H

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdbool.h>
#include "crc.h"
#include "vision.h"
/* 宏定义与全局变量 ------------------------------------------------------------*/
#define ENEMY_NUM 9

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

// 没有找到目标（兵种 HP[uwb获取] 0 0 0）
typedef struct __packed  
{
    uint8_t num;
    int16_t HP;

    uint8_t x; // 位置
    uint8_t y;
    uint8_t z;
	
}RobotInfo_t;

// 大世界坐标，来自雷达和uwb
typedef struct __packed
{
    uint8_t x;
    uint8_t y;
}CoordInfo_t;

/* 发送数据段格式 */
typedef struct __packed 
{
	uint8_t game_progress;
    uint8_t enemy_color;

    uint8_t armor3_size; // 0:小装甲板 1:大装甲板
    uint8_t armor4_size;
    uint8_t armor5_size;
	
    // 大yaw轴
    int16_t big_yaw;
    uint8_t remain_bullet;

    uint8_t death_cnt; // 死亡次数

    RobotInfo_t allies_list[ENEMY_NUM]; // 1英雄 2工程 3-5步兵 6哨兵 7前哨 8基地
    RobotInfo_t enemys_list[ENEMY_NUM];

    CoordInfo_t allies_coord[ENEMY_NUM]; // 从uwb获取的坐标
    CoordInfo_t enemys_coord[ENEMY_NUM]; // 看雷达准不准

}navigation_tx_data_t;/*做好视觉数据突然掉线的准备*/


typedef struct __packed 
{
    uint8_t if_hit;   // 控制电控是否开枪
    uint8_t nuc_flag; // 区分小电脑
    uint8_t hit_mode; //

    /* 不打XX */
    uint8_t no_hit_enemy2 : 1; // 不打工程
    uint8_t no_hit_enemy6 : 1; // 不打哨兵
    uint8_t no_hit_enemy7 : 1; // 不打前哨
    uint8_t no_hit_enemy8 : 1; // 不打基地

    uint8_t nav_mode; // 导航

    // 导航速度
    int16_t chassis_front;
    int16_t chassis_right;
	int16_t yaw_angle;
	
	EnemyData_t omni_enemy_data;// 0目标 1英雄 2工程 3-5步兵 6哨兵 7前哨 8基地
	
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
