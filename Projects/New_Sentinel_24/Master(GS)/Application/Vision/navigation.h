#ifndef __NAVIGATION_H
#define __NAVIGATION_H

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdbool.h>
#include "crc.h"
#include "vision.h"
/* 宏定义与全局变量 ------------------------------------------------------------*/
#define ENEMY_NUM_GS 9
#define ENEMY_NUM_FQ 8


/* 指令与配置 ------------------------------------------------------------*/



/* 数据长度 */
/*@note: uint16 - [0 : 65535]
         int16  - [-32768 : 32767]*/
typedef enum {
	/* Std */
	LEN_NAVIGATION_RX_PACKET	= 23 + 55 + 5,	// 接收包整包长度   78 + 5
	LEN_NAVIGATION_TX_PACKET	= 13 + 55 + 55 + 32 + 32 + 5,	    // 发送包整包长度   187 + 5

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
	uint8_t game_progress;      // 比赛开始标志位
    uint8_t game_strategy;      // 比赛策略

    uint8_t  enemy_color;        // 敌方颜色
    uint16_t remain_money;       // 剩余金币 2024.5新增

    uint8_t if_find_enemy;      // 左右头标志位相加  取值 0-2

    uint8_t armor3_size;        // 0:小装甲板 1:大装甲板
    uint8_t armor4_size;    
    uint8_t armor5_size;        // 国赛才有5号步兵

    // 大yaw轴
    int16_t  big_yaw;
    uint16_t remain_bullet;      //开局400

    //己方血量
    uint16_t hero_HP;
    uint16_t engineer_HP;
    uint16_t infantry3_HP;
    uint16_t infantry4_HP;
    uint16_t infantry5_HP;
    uint16_t sentry_HP;         // 前哨站爆掉之后+600
    uint16_t outpost_HP;
    uint16_t base_HP;
    
    // 敌方血量
    uint16_t enemy_hero_HP;
    uint16_t enemy_engineer_HP;
    uint16_t enemy_infantry3_HP;
    uint16_t enemy_infantry4_HP;
    uint16_t enemy_infantry5_HP;
    uint16_t enemy_sentry_HP;
    uint16_t enemy_outpost_HP;
    uint16_t enemy_base_HP;
    
    // 从自瞄获取敌方位置
    // 1英雄 2工程 3-5步兵 6哨兵 7前哨 8基地
    EnemyData_t left_enemys_data;    // 6 * 9 + 1 = 55字节  //左自瞄相机来
    EnemyData_t right_enemys_data;   // 6 * 9 + 1 = 55字节  //右自瞄相机来

    CoordInfo_t allies_coord[ENEMY_NUM_FQ]; // 己方坐标，从裁判系统获取
    CoordInfo_t enemys_coord[ENEMY_NUM_FQ]; // 敌方坐标，看雷达准不准

//    RobotInfo_t allies_list[ENEMY_NUM]; // 1英雄 2工程 3-5步兵 6哨兵 7前哨 8基地
//    RobotInfo_t enemys_list[ENEMY_NUM];

//    CoordInfo_t allies_coord[ENEMY_NUM_GS]; // 从uwb获取的坐标
//    CoordInfo_t enemys_coord[ENEMY_NUM_GS]; // 看雷达准不准

}navigation_tx_data_t;/*做好视觉数据突然掉线的准备*/

/* 接收包格式 */
typedef struct __packed 
{
    uint8_t if_hit;   // 控制电控是否开枪
    uint8_t nuc_flag; // 区分小电脑
    uint8_t hit_mode;

    uint16_t add_bullet_num;    // 在补给点兑换子弹数量，默认为0
    uint8_t remote_add_HP_times;     // 远程兑换血量次数，每次单调递增+1， 兑换后回复60%血量
    uint8_t remote_add_bullet_times;     // 远程兑换弹量次数，每次单调递增+1，兑换后100金币换150发子弹

    uint8_t if_revive_immediate;          // 是否复活，默认为0
    uint8_t if_revive_await;              // 是否延迟复活，默认为0

    // 发给自瞄控制击打目标
    //4并1注意！！！
    uint8_t no_hit_enemy2 : 1; // 不打工程
    uint8_t no_hit_enemy6 : 1; // 不打哨兵
    uint8_t no_hit_enemy7 : 1; // 不打前哨
    uint8_t no_hit_enemy8 : 1; // 不打基地

    // 转发给自瞄
    uint8_t is_arrive; // 到达目标点标志位
    uint8_t is_moving; // 移动中标志位

    uint8_t nav_mode; // 导航正常运行标志位 0： 正常  1：不正常

    // 导航目标点   2024.5新增
    uint16_t nav_target_x;
    uint16_t nav_target_y;
    
    // 导航速度
    int16_t chassis_front;
    int16_t chassis_right;

	int16_t yaw_angle;// 雷达的角度

	//全向感知数据 // 0目标 1英雄 2工程 3-5步兵 6哨兵 7前哨 8基地
	EnemyData_t omni_enemy_data;//6 * 9 + 1 = 55字节    //全向感知相机来
	
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
