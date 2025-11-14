/**
  ******************************************************************************
  * @file           : judge.h
  * @brief          : 
  * @update         : finish 2022-2-13 20:10:12
  ******************************************************************************
  */

#ifndef __JUDGE_H
#define __JUDGE_H

#include "stm32f4xx_hal.h"
#include "judge_protocol.h"



typedef enum
{
	ammo_HURT      = 0,
	offline_HURT   = 1,
	overshoot_HURT = 2,
	overheat_HURT  = 3,
	overpower_HURT = 4,
	crush_HURT     = 5,
	no_HURT        = 6,
	   
}hurt_type_e;

#define JUDGE_ONLINE 1
#define JUDGE_OFFLINE 0

typedef struct
{
	uint16_t remain_HP;
	uint16_t last_HP;

	/*从裁判系统获取自身坐标*/
	uint16_t uwb_position_x;					//float 转换为 uint16_t
	uint16_t uwb_position_y;

	int16_t  chassis_power_buffer;           //底盘缓存功率
	int32_t  chassis_out_put_max;            //底盘最大输出

	uint16_t shooter_cooling_limit;		//机器人 17mm 枪口热量上限
	uint16_t shooter_id1_cooling_heat;
	uint16_t shooter_id2_cooling_heat; 		//机器人 17mm 枪口热量

	float    shooter_id1_speed;
	float    shooter_id2_speed;

	float    gimbal_yaw_angle;             //枪管yaw轴角度 ?? 
	hurt_type_e hurt_type;				//伤害类型
	uint8_t   last_armor_id;
	uint8_t   armor_id;
	
	uint8_t car_color;                      //1自己蓝色 0自己红色

	//伤害种类
	uint16_t chassis_power_limit;           //底盘功率限制
	uint16_t shooter_id1_42mm_speed_limit;  //射速上限
	uint8_t rfid;
	uint8_t game_progress;
	
	/*血量都改为u16类型*/
    //己方血量
    uint16_t ally_hero_HP;
    uint16_t ally_engineer_HP;
    uint16_t ally_infantry3_HP;
    uint16_t ally_infantry4_HP;
    uint16_t ally_infantry5_HP;
    uint16_t ally_sentry_HP;
    uint16_t ally_outpost_HP;
    uint16_t ally_base_HP;
	
    // 敌方血量
    uint16_t enemy_hero_HP;
    uint16_t enemy_engineer_HP;
    uint16_t enemy_infantry3_HP;
    uint16_t enemy_infantry4_HP;
    uint16_t enemy_infantry5_HP;
    uint16_t enmey_sentry_HP;
    uint16_t enemy_outpost_HP;
    uint16_t enemy_base_HP;
	
	uint16_t remain_time;
	uint16_t remain_bullet;
	
	uint8_t robot_commond;
	uint8_t last_commond;

	uint32_t test;
	
}judge_base_info_t;



typedef struct
{
	uint16_t offline_cnt_max;
	uint8_t status;
	uint16_t offline_cnt;
}judge_info_t;

typedef struct
{
	uint16_t shoot_num;
	
	uint16_t speed_260;
	uint16_t speed_270;
	uint16_t speed_271;
	uint16_t speed_272;
	uint16_t speed_273;
	uint16_t speed_274;
	uint16_t speed_275;
	uint16_t speed_276;
	uint16_t speed_277;
	uint16_t speed_278;
	uint16_t speed_279;
	uint16_t speed_280;
	uint16_t speed_281;
	uint16_t speed_282;
	uint16_t speed_283;
	uint16_t speed_284;
	uint16_t speed_285;
	uint16_t speed_286;
	uint16_t speed_287;
	uint16_t speed_288;
	uint16_t speed_289;
	uint16_t speed_290;
	
	float mean;
	float variance;
	
}shoot_data_t;

typedef struct
{
	int16_t buffer_max;
}judge_config_t;



typedef struct 
{
	ext_rfid_status_t       rfid_status;
	ext_game_status_t       game_status;
	ext_game_robot_status_t game_robot_status;
	ext_power_heat_data_t   power_heat_data;
	ext_shoot_data_t        shoot_data;
	ext_game_robot_pos_t    game_robot_pos;
	ext_robot_hurt_t        ext_robot_hurt;
	ext_aerial_data_t       ext_aerial_data;
	ext_robot_command_t     ext_robot_command;
	ext_game_robot_HP_t     game_robot_HP;
	ext_bullet_remaining_t  bullet_remaining;
	ground_robot_position_t robot_position;

	sentry_info_t		   	sentry_info;
	
}judge_rawdata_t;


typedef struct 
{
	judge_config_t *config;
	judge_base_info_t *base_info;
	judge_info_t *info;
	judge_rawdata_t *data;
}judge_t;
/*--------------------裁判系统发送内容-----------------------*/
typedef struct __packed
{
	uint8_t  			    sof;		// 同步头
	uint16_t  	            data_length;// 命令码
	uint8_t                 seq;
	uint8_t  			    crc8;		// CRC8校验码
	uint16_t                cmd_id;
} Judge_frame_header_t;

/* 帧尾格式 */
typedef struct __packed 
{
	uint16_t crc16;					// CRC16校验码
} Judge_frame_tailer_t;

/* 地图发送包格式 */
typedef struct __packed
{
	Judge_frame_header_t   FrameHeader;	// 帧头
	map_sentry_data_t	   TxData;		    // 数据
	Judge_frame_tailer_t   FrameTailer;	// 帧尾	
}Judge_Map_tx_packet_t;

/* 决策发送包格式 */
typedef struct __packed
{
	Judge_frame_header_t   FrameHeader;	// 帧头
	sentry_cmd_t		   TxData;		    // 数据
	Judge_frame_tailer_t   FrameTailer;	// 帧尾	
}Judge_Decision_tx_packet_t;


typedef struct __packed
{
	Judge_frame_header_t   FrameHeader;	// 帧头
	ext_aerial_data_t		TxData;		    // 数据
	Judge_frame_tailer_t   FrameTailer;	// 帧尾	
}Judge_Decision2J_tx_packet_t;



void Judge_Get_Data(uint32_t id, uint8_t *data);
void judge_heart(void);
void Judge_MapSend(void);
void Judge_DecisionSend(void);
void Judge_DecisionSend2Judge(void);
#endif
