/*
 * @Author: lion
 * @Date: 2025-04-16 20:29:46
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-07-29 01:09:09
 * @FilePath: \sentry_up\Application\DeviceLayer\Slave.h
 * @Description: 
 * 
 */
#ifndef __SLAVE_H
#define __SLAVE_H

#include "rp_config.h"
#include "driver.h"
#include "vision.h"

/*CAN2*/
#define CHASSIS_SPEED_ID					0x001
#define SLAVE_CTRL_FLAG_ID				0x002

//裁判系统
#define GAME_STATUS_ID 				0x003	//	比赛状态		剩余时间		剩余弹量		剩余金币		己方颜色
#define GAME_ROBOT_HP_ID 			0x004	//	己方血量		敌方血量
#define DECISION_RX_ID				0x005	//	哨兵接收决策数据
#define ROBOT_CMD_ID					0x006	//	云台手控制数据
#define ROBOT_POSITION_ID			0x007	//	己方机器人坐标数据
#define SHOOT_DATA_ID 				0x008	//	弹丸射速
#define RADAR_POS_COOR_ID			0x101	//	雷达标点坐标
#define GAME_ROBOT_STATUS_ID 	0x102	//	机器人状态数据

/*CAN1*/
#define VISION_ENEMY_POS				0x003

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
	int16_t	target_x;					//	目标x
	int16_t	target_y;					//	目标y
	uint8_t	cmd_key;					//	操作按键
	uint8_t	target_id;				//	目标ID
}	slave_robot_cmd_t;

typedef __packed struct
{
	uint16_t	shooter_barrel_heat;				//	当前热量
	uint16_t	shooter_barrel_heat_limit;	//	热量限制
	uint16_t	shooter_barrel_cooling_value;
	uint16_t	unused;
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
}slave_sentry_auto_t;


typedef  struct slave_rx_mid_info_struct
{
	struct gimbal_info_struct
	{
		uint8_t gimbal_mode;
		uint8_t vision_mode;
		
		int16_t current_vx;
		int16_t current_vy;
		
		uint16_t gimbal_target_yaw;
	}gimbal_info;
	
	struct ladar_info_struct
	{
		uint16_t	  ladar_x;
		uint16_t   ladar_y;
		float   current_yaw;
	}ladar_info;
	
}slave_rx_mid_info_t;

typedef struct slave_rx_down_info_struct
{
	slave_game_status_t				game_status;				//	比赛状态
	slave_game_robot_HP_t			game_robot_HP;			//	血量数据
	slave_game_robot_status_t	game_robot_status;	//	机器人状态
	slave_radar_pos_coor_t		radar_pos_coor;			//	雷达坐标数据
	slave_robot_cmd_t			robot_cmd;					//操作手指令
	slave_sentry_auto_t				sentry_auto;				//	哨兵自主决策 rx

	
}slave_rx_down_info_t;


typedef struct slave_rx_info_struct
{
	
    slave_rx_mid_info_t	 *mid_info;
    slave_rx_down_info_t    *down_info;
	
    // ---------- 上主控直接使用这部分数据 begin ---------- //
	float	bullet_speed;
	
	uint8_t		my_color;				//	自己颜色	0	红方	1	蓝方
	uint8_t		game_progress;	//	比赛状态	同裁判数据
	uint8_t		cap_remain;			//	超电容量		100%
	uint16_t	remain_time;		//	剩余时间(单位：s)
	uint16_t	remain_gold;
	uint16_t	remain_bullet;
	
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
	uint16_t	shooter_barrel_cooling_value;	//热量冷却
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
	// ---------- 上主控直接使用这部分数据 end   ---------- //
	
	uint16_t			offline_cnt;
	uint16_t			offline_cnt_max;

}slave_rx_info_t;

typedef struct slave_tx_chassis_info_struct
{
    int16_t vx;
    int16_t vy;
    int16_t w;

    uint8_t	power_target;	

    union
	{
		uint8_t cmd;
		
		struct master_cmd_t
		{
			uint8_t control_cmd:	1;	//	控制开关	0	关	1开
			uint8_t	cap_switch:		1;	//	超电开关	0	关	1开
			
			uint8_t	wheel_flag:		1;
			uint8_t unuse:			5;
		}	bit;
	}	cmd_info;
}slave_tx_chassis_info_t;

typedef struct enemy_pos_info_struct
{
	uint16_t enemy_x;
	uint16_t enemy_y;
}enemy_pos_info_t;

typedef struct slave_tx_info_struct
{
    uint8_t   ctrl_flag[8];					//标志位数组（主要是中主控使用） 0--控制方式 1--运动方式 2--开始标志位 3--前相机发现敌人标志位 4发现敌人对应的数字 56 小yaw对应的机械角度
								            //8 - 暂时替代决策板实现三种决策选择
    slave_tx_chassis_info_t *chassis_info;    
	enemy_pos_info_t *enemy_pos_info;
	uint8_t	tx_mid_enemy_num;
}slave_tx_info_t;


typedef struct slave_info_struct
{
    slave_rx_info_t         *rx_info;
    slave_tx_info_t         *tx_info;
	
	uint16_t			offline_cnt;
	uint16_t			offline_cnt_max;
}slave_info_t;

typedef struct slave_struct
{
    dev_work_state_t        work_state;
    slave_info_t            *info;

    void (*get_chassis_info)(struct slave_struct *slave,int16_t LF_speed,int16_t RF_speed,int16_t LB_speed,int16_t RB_speed);
    void (*send_chassis_info)(struct slave_struct *slave);
    void (*get_ctrl_mode)(struct slave_struct *slave,uint8_t ctrl_mode);
    void (*get_move_mode)(struct slave_struct *slave,uint8_t move_mode);
    void (*send_flag)(struct slave_struct *slave);
    void (*get_start)(struct slave_struct *slave,uint8_t start);
	void (*get_vision_status)(struct slave_struct *slave,uint8_t is_find_target,uint8_t enemy_num);
	void (*send_vision_pos)(struct slave_struct *slave);
    //裁判系统函数
    void							(*init)(struct slave_struct *self);	                	// 设备初始化函数
	void							(*update)(struct slave_struct *self, uint32_t rx_id, uint8_t *rxBuf);	// 设备数据更新函数
	void							(*heartbeat)(struct slave_struct *self);	            // 设备心
}slave_t;

void Slave_get_ctrl_mode(slave_t *slave,uint8_t ctrl_mode);
void Slave_get_move_mode(slave_t *slave,uint8_t move_mode);
void Slave_send_chassis_info(slave_t *slave);
void Slave_send_flag(slave_t *slave);
void Slave_get_start(slave_t *slave,uint8_t start);
void Slave_get_vision_status(slave_t *slave,uint8_t vision_status,uint8_t enemy_num);
void Slave_send_enemy_pos(slave_t *slave);
//void Slave_send_imu_info(slave_t *slave);

extern slave_t slave ;
extern uint32_t	speed_cnt;






#endif