#ifndef __MASTER_H
#define __MASTER_H

#include "rp_config.h"

typedef struct master_rx_info_struct
{
	int16_t	front;
	int16_t	right;
	int16_t	cycle;
	
	uint8_t	power_target;					//	目标底盘功率的二分之一
	
	union
	{
		uint8_t cmd;
		
		struct master_cmd_t
		{
			uint8_t control_cmd:	1;	//	控制开关	0	关	1	开
			uint8_t	cap_switch:		1;	//	超电开关	0	关	1	开
			uint8_t spin_switch:	1;	//	陀螺开关	0	关	1	开
			
			uint8_t	not_used:			5;	//	后续可扩展的标志位
		}	bit;
	}	cmd_info;
}master_rx_t;

// ------------------- 主控发送数据结构体 begin -------------------  //
typedef __packed struct
{
	uint8_t		my_color: 1;				//	自己颜色	0	红方	1	蓝方
	uint8_t		game_progress: 3;		//	比赛状态	同裁判数据
	uint8_t		shooter_flag:1;			//	电管输出口标志位
	uint8_t		unused:3;						//	未使用

	uint8_t 	cap_remain;					//	超电容量	100%
	uint16_t	remain_time;				//	剩余时间(单位：s)
	uint16_t	remain_gold;
	uint16_t	remain_bullet;
}	master_game_status_t;

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
}	master_game_robot_HP_t;

typedef __packed struct
{
	uint16_t	shooter_barrel_heat;					//	当前热量
	uint16_t	shooter_barrel_heat_limit;		//	热量限制
	uint16_t	shooter_barrel_cooling_value;	//	枪口热量每秒冷却值
	uint16_t	finish_small_buff:1;												//	未使用
	uint16_t	unuse:15;
}	master_game_robot_status_t;

typedef __packed struct
{
	int16_t	target_x;					//	目标x
	int16_t	target_y;					//	目标y
	uint8_t	cmd_key;					//	操作按键
	uint8_t	target_id;				//	目标ID
}	master_robot_cmd_t;

typedef __packed struct
{
	uint16_t	pack_id1:4;

	uint16_t	hero_x:	12;				//	英雄坐标X	0~1500
	uint16_t	hero_y:	12;				//	英雄坐标Y	0~2800
	uint16_t	engineer_x: 12;		//	工程坐标X	0~1500
	uint16_t	engineer_y: 12;		//	工程坐标Y	0~2800
	uint16_t	infantry3_x: 12;	//	步兵3坐标X	0~1500

	uint16_t	pack_id2:4;
	uint16_t	infantry3_y: 12;	//	步兵3坐标Y	0~2800
	uint16_t	infantry4_x: 12;	//	步兵4坐标X	0~1500
	uint16_t	infantry4_y: 12;	//	步兵4坐标Y	0~2800
	uint16_t	sentry_x: 12;			//	哨兵坐标X	0~1500
	uint16_t	sentry_y: 12;			//	哨兵坐标Y	0~2800
}	master_radar_pos_coor_t;

typedef struct 
{
	uint8_t	hero_x;
	uint8_t	hero_y;
	uint8_t	engineer_x;
	uint8_t	engineer_y;
	uint8_t	infantry3_x;
	uint8_t	infantry3_y;
	uint8_t	infantry4_x;
	uint8_t	infantry4_y;

}master_robot_position_t;

typedef struct 
{
	uint32_t add_bullet_num						:11;	//哨兵机器人成功兑换的允许发弹量
	uint32_t remote_add_bullet_times	:4;		//哨兵机器人成功兑换允许发弹量的次数
	uint32_t remote_add_HP_times			:4;		//哨兵机器人成功兑换血量的次数
	uint32_t is_free_rebrith					:1;		//可否确认免费复活 
	uint32_t is_pay_rebrith						:1;		//可否确认付费复活 
	uint32_t pay_for_rebrith					:10;	//复活需要金币
	uint32_t unuse_bits1							:1;
	
	uint16_t is_outof_war							:1;		//是否脱战
	uint16_t remain_bullet						:11;	//队伍剩余可购买发弹量
	uint16_t unuse_bits2							:4;
	uint16_t unused;			
}master_sentry_auto_t;

typedef struct master_tx_info_struct
{
	float	bullet_speed;

	master_game_status_t				game_status;				//	比赛状态
	master_game_robot_HP_t			game_robot_HP;			//	血量数据
	master_game_robot_status_t	game_robot_status;	//	机器人状态
	master_robot_cmd_t					robot_cmd;					//	操作手指令
	master_radar_pos_coor_t			radar_pos_coor;			//	雷达坐标数据
	master_robot_position_t			robot_position;			//	己方机器人坐标
	master_sentry_auto_t				sentry_auto;				//	哨兵自主决策 rx
}master_tx_t;
// ------------------- 主控发送数据结构体 end -------------------  //

typedef struct master_info_struct
{
	master_rx_t		*rx;
	master_tx_t		*tx;
	
	uint16_t			offline_cnt;
	uint16_t			offline_cnt_max;
}master_info_t;

typedef struct master_struct
{
	dev_work_state_t	work_state;
	
	master_info_t			*info;

	void							(*update)(struct master_struct *self, uint8_t *rxBuf);	// 设备数据更新函数
	void							(*heart_beat)(struct master_struct *self);	            // 设备心跳包
}master_t;

extern master_t master;

#endif
