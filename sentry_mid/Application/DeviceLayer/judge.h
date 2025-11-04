#ifndef __JUDGE_H
#define __JUDGE_H

/* Includes ------------------------------------------------------------------*/
#include "crc.h"
#include "string.h"
#include "stdbool.h"
#include "rp_config.h"

#include "judge_def.h"

#include "drv_uart.h"

// 此为帧头的SOF,帧头分为 SOF,length,Seq,CRC8
#define JUDGE_FRAME_HEADER	0xA5  

/*--------- 偏移位置 ---------*/
// 帧字节（单位/字节）
// 帧头占五个字节，命令码占两个字节，从第七个开始为数据帧
enum judge_frame_offset_t {
	FRAME_HEADER	= 0,
	CMD_ID		    = 5,
	DATA_SEG		= 7
};

// 帧头字节（单位/bit）
enum judge_frame_header_offset_t {
	SOF			    = 0,					//帧头的详细位置
	DATA_LENGTH		= 1,
	SEQ			    = 3,
	CRC8		    = 4
};

typedef struct 
{
	std_frame_header_t							fream_header;							// 帧头信息
	
	ext_game_status_t 							game_status;							// 0x0001
	ext_game_result_t 							game_result;							// 0x0002
	ext_game_robot_HP_t 						game_robot_HP;						// 0x0003
	
	ext_event_data_t							event_data;								// 0x0101
	ext_supply_projectile_action_t				supply_projectile_action;	// 0x0102
	ext_referee_warning_t						referee_warning;					// 0x0104
	ext_dart_remaining_time_t					dart_remaining_time;			// 0x0105
	
	ext_game_robot_status_t						game_robot_status;				// 0x0201
	ext_power_heat_data_t						power_heat_data;					// 0x0202
	ext_game_robot_pos_t						game_robot_pos;						// 0x0203
	ext_buff_t									buff;											// 0x0204
	ext_aerial_robot_energy_t					aerial_robot_energy;			// 0x0205
	ext_robot_hurt_t							robot_hurt;								// 0x0206
	ext_shoot_data_t							shoot_data;								// 0x0207
	ext_bullet_remaining_t						bullet_remaining;					// 0x0208	
	ext_rfid_status_t							rfid_status;							// 0x0209	
	ext_dart_client_cmd_t           			dart_client;        			// 0x020A
	ext_ground_robot_position_t					ground_robot_position;		// 0x020B
	ext_radar_mark_data_t						radar_mark_data;					// 0x020C
	
	ext_interact_id_t							ids;											// 与本机交互的机器人id
	
	uint16_t                        			self_client;        			// 本机客户端
	uint8_t										my_color;
	
	int16_t	 offline_cnt;
	int16_t	 offline_max_cnt;
	
}judge_info_t;

typedef struct judge_struct
{
	judge_info_t      *info;
	
	dev_work_state_t	work_state;
	dev_id_t			    id;
	
	void				     (*send)(struct judge_struct *self, uint8_t *txbuf);
	void				     (*update)(struct judge_struct *self, uint8_t *rxbuf);
	void				     (*heart_beat)(struct judge_struct *self);
	
}judge_t;

/* 主机端信息 */
typedef struct
{
	uint8_t rxbuf[8];
	
	uint8_t launcher_state 	: 1;		/* 发射机构状态 0在线 1离线 */
	uint8_t gimbal_state 	: 1;		/* 云台状态 0在线 1离线 */
	uint8_t chassis_state 	: 1;		/* 底盘状态 0在线 1离线 */
	uint8_t vision_state 	: 1;		/* 视觉状态 0在线 1离线 */
	uint8_t fric_switch 	: 1;		/* 摩擦轮开关 0关 1开 */
	uint8_t cap_swicth 		: 1;		/* 超电开关 0关 1开 */
	uint8_t wheel_swicth 	: 1;		/* 陀螺开关 0关 1开 */
	uint8_t move_mode 		: 1;		/* 整车模式 0陀螺仪 1机械 */

	uint8_t is_aim 			: 1;		/* 锁定标志位 0无 1有 */
	uint8_t aim_big_buff 	: 1;		/* 锁定大符标志位 0无 1大符 */
	uint8_t aim_small_buff 	: 1;		/* 锁定小符标志位 0无 1小符 */
	uint8_t meaning_less 	: 5;		/* 无意义位 */

	uint8_t armor_num 		: 4;		/* 视觉识别装甲板号 */
	uint8_t armor_id 		: 4;		/* 视觉识别装甲板id */

	// uint8_t cap_U_low  	: 1;		/* 小符标志位 0无 1小符 */
	// uint8_t cap_I_low  	: 1;		/* 摩擦轮状态 0关 1开 */
	// uint8_t cap_U_high 	: 1;		/* 发射机构状态 0在线 1离线 */
	// uint8_t cap_I_high 	: 1;		/* 云台状态 0在线 1离线 */
	
	int16_t	 offline_cnt;
	int16_t	 offline_max_cnt;
	
}master_info_t;

typedef struct master_struct
{
	master_info_t			*info;
	dev_work_state_t		work_state;
	
	void				     (*update)(struct master_struct *self, uint8_t *rxbuf);
	void				     (*heart_beat)(struct master_struct *self);
	
}master_t;

extern judge_t	judge;
extern master_t master;

/* Exported functions --------------------------------------------------------*/
void Judge_Heartbeat(judge_t *jud_sen);
void master_update(master_t *mas_sen, uint8_t *rxbuf);
void master_heart_beat(master_t *mas_sen);

#endif
