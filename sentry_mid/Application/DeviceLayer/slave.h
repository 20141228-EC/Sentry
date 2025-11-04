#ifndef __SLAVE_H
#define __SLAVE_H

#include "rp_config.h"

#define GAME_STATUS_ID 			0x003	//	比赛状态	剩余时间	剩余弹量	剩余金币	己方颜色
#define GAME_ROBOT_HP_ID 		0x004	//	己方血量	敌方血量
#define SHOOT_DATA_ID 			0x005	//	弹丸速度	
#define GIMBAL_OPERATE_ID		0x006	//	云台操作数据
#define RADAR_POS_COOR_ID		0x101	//	雷达标点坐标
#define GAME_ROBOT_STATUS_ID 	0x102	//	机器状态

// ------------------- 主控发送数据结构体 begin -------------------  //
typedef __packed struct
{
	uint8_t		my_color: 1;				//	自己颜色	0	红方	1	蓝方
	uint8_t		game_progress: 3;		//	比赛状态	同裁判数据
	uint8_t		unused:4;						//	未使用

	uint8_t 	cap_remain;					//	超电容量	100%
	uint16_t	remain_time;				//	剩余时间(单位：s)
	uint16_t	remain_gold;
	uint16_t	remain_bullet;
}	slave_game_status_t;

typedef __packed struct
{
	uint8_t	HP_status:1;	//	血量状态	0	己方	1	敌方
	
	uint8_t	hero_HP;			//	英雄血量
	uint8_t	engineer_HP;	//	工程血量
	uint8_t	infantry3_HP;	//	步兵3血量
	uint8_t	infantry4_HP;	//	步兵4血量
	uint8_t	sentry_HP;		//	哨兵血量
	uint8_t	outpost_HP;		//	前哨血量
	uint8_t	base_HP;			//	基地血量
}	slave_game_robot_HP_t;

typedef __packed struct
{
	uint16_t	shooter_barrel_heat;					//	当前热量
	uint16_t	shooter_barrel_heat_limit;		//	热量限制
	uint16_t	shooter_barrel_cooling_value;	//	每秒冷却热量
	uint16_t	finish_small_buff:1;					//是否激活小符
	uint16_t	unuse:15;
	
}	slave_game_robot_status_t;

typedef __packed struct
{
	uint16_t	pack_id:4;

	uint16_t	data1: 12;
	uint16_t	data2: 12;
	uint16_t	data3: 12;
	uint16_t	data4: 12;
	uint16_t	data5: 12;
}	slave_radar_pos_coor_t;

typedef __packed struct 
{
	int16_t	target_x;					//	目标x
	int16_t	target_y;					//	目标y
	uint8_t	cmd_key;					//	操作按键
	uint8_t	target_id;				//	目标ID
}salve_gimbal_operate_t;

// ------------------- 主控发送数据结构体 end -------------------  //

typedef struct slave_rx_info_struct
{
	slave_game_status_t				game_status;				//	比赛状态 	
	slave_game_robot_HP_t			game_robot_HP;			//	血量数据
	slave_game_robot_status_t	game_robot_status;	//	机器人状态
	slave_radar_pos_coor_t		radar_pos_coor;			//	雷达坐标数据
	salve_gimbal_operate_t		gimbal_operate;			//	云台操作数据
	
}slave_rx_info_t;

typedef struct slave_info_struct
{
	slave_rx_info_t	*rx;
	
	// ---------- 上主控直接使用这部分数据 begin ---------- //
	float	bullet_speed;
	
	uint8_t		my_color;				//	自己颜色	0	红方	1	蓝方
	uint8_t		game_progress;	//	比赛状态	同裁判数据
	uint8_t		cap_remain;			//	超电容量		100%
	uint16_t	remain_time;		//	剩余时间(单位：s)
	uint16_t	remain_gold;
	uint16_t	remain_bullet;
	uint32_t   finish_small_buff;		//是否完成小符
	
	uint16_t	ab_hero_HP;			//	己方英雄血量
	uint16_t	ab_engineer_HP;	//	己方工程血量
	uint16_t	ab_infantry3_HP;//	己方步兵3血量
	uint16_t	ab_infantry4_HP;//	己方步兵4血量
	uint16_t	ab_sentry_HP;		//	己方哨兵血量
	uint16_t	ab_outpost_HP;	//	己方前哨血量
	uint16_t	ab_base_HP;			//	己方基地血量
	
	uint16_t	eb_hero_HP;			//	敌方英雄血量
	uint16_t	eb_engineer_HP;	//	敌方工程血量
	uint16_t	eb_infantry3_HP;//	敌方步兵3血量
	uint16_t	eb_infantry4_HP;//	敌方步兵4血量
	uint16_t	eb_sentry_HP;		//	敌方哨兵血量
	uint16_t	eb_outpost_HP;	//	敌方前哨血量
	uint16_t	eb_base_HP;			//	敌方基地血量
	
	uint16_t	shooter_barrel_heat;				//	当前热量
	uint16_t	shooter_barrel_heat_limit;	//	热量限制
	float		self_pos_x;									//	自身坐标X
	float		self_pos_y;									//	自身坐标Y
	
	uint16_t	hero_x;				//	英雄坐标X	0~1500
	uint16_t	hero_y;				//	英雄坐标Y	0~2800
	uint16_t	engineer_x;		//	工程坐标X	0~1500
	uint16_t	engineer_y;		//	工程坐标Y	0~2800
	uint16_t	infantry3_x;	//	步兵3坐标X	0~1500
	uint16_t	infantry3_y;	//	步兵3坐标Y	0~2800
	uint16_t	infantry4_x;	//	步兵4坐标X	0~1500
	uint16_t	infantry4_y;	//	步兵4坐标Y	0~2800
	uint16_t	sentry_x;			//	哨兵坐标X	0~1500
	uint16_t	sentry_y;			//	哨兵坐标Y	0~2800

	int16_t 	operate_x;			//	操作员坐标X	0~1500
	int16_t 	operate_y;			//	操作员坐标Y	0~2800
	// ---------- 上主控直接使用这部分数据 end   ---------- //
	
	uint16_t			offline_cnt;
	uint16_t			offline_cnt_max;
}slave_info_t;

typedef struct slave_struct
{
	dev_work_state_t	work_state;
	
	slave_info_t			*info;
	
	void							(*init)(struct slave_struct *self);	                	// 设备初始化函数
	void							(*update)(struct slave_struct *self, uint32_t rx_id, uint8_t *rxBuf);	// 设备数据更新函数
	void							(*heartbeat)(struct slave_struct *self);	            // 设备心跳包
}slave_t;

extern slave_t slave;

#endif
