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

typedef struct radar_rx_struct
{
	uint16_t data_cmd_id;
	uint16_t sender_id;
	uint16_t receiver_id;
	
	int16_t	hero_x;
	int16_t	hero_y;
	int16_t	engineer_x;
	int16_t	engineer_y;
	int16_t	infantry3_x;
	int16_t	infantry3_y;
	int16_t	infantry4_x;
	int16_t	infantry4_y;
	int16_t	sentry_x;
	int16_t	sentry_y;
	
}radar_rx_t;

typedef struct 
{
	std_frame_header_t							fream_header;							// 帧头信息
	
	ext_game_status_t 							game_status;							// 0x0001
	ext_game_result_t 							game_result;							// 0x0002
	ext_game_robot_HP_t 						game_robot_HP;						// 0x0003
	
	ext_event_data_t								event_data;								// 0x0101
	ext_supply_projectile_action_t	supply_projectile_action;	// 0x0102
	ext_referee_warning_t						referee_warning;					// 0x0104
	ext_dart_remaining_time_t				dart_remaining_time;			// 0x0105
	
	ext_game_robot_status_t					game_robot_status;				// 0x0201
	ext_power_heat_data_t						power_heat_data;					// 0x0202
	ext_game_robot_pos_t						game_robot_pos;						// 0x0203
	ext_buff_t											buff;											// 0x0204
	ext_aerial_robot_energy_t				aerial_robot_energy;			// 0x0205
	ext_robot_hurt_t								robot_hurt;								// 0x0206
	ext_shoot_data_t								shoot_data;								// 0x0207
	ext_bullet_remaining_t					bullet_remaining;					// 0x0208	
	ext_rfid_status_t								rfid_status;							// 0x0209	
	ext_dart_client_cmd_t           dart_client;        			// 0x020A
	ext_ground_robot_position_t			ground_robot_position;		// 0x020B
	ext_radar_mark_data_t						radar_mark_data;					// 0x020C
	ext_robot_command_t							robot_command_t;					// 0x0303
	ext_sentry_auto_t								sentry_auto;
	
	ext_interact_id_t								ids;											// 与本机交互的机器人id
	
	radar_rx_t											radar_info;								// 接受雷达数据
	
	uint16_t                        self_client;        			// 本机客户端
	uint8_t													my_color;
	
	int16_t	 offline_cnt;
	int16_t	 offline_max_cnt;
	
}judge_info_t;

typedef struct judge_struct
{
	judge_info_t      *info;
	
	dev_work_state_t	work_state;
	dev_id_t			    id;

	void				     (*update)(struct judge_struct *self, uint8_t *rxbuf);
	void				     (*heart_beat)(struct judge_struct *self);
	
}judge_t;

extern judge_t	judge;

// --------------------------------------------- //

typedef struct hit_flag_struct
{
	bool flag;

	int16_t	 hit_cnt;				//	被击打清零
	int16_t	 hit_cnt_max;		//	高速转动持续时间

	void	(*heart_beat)(struct hit_flag_struct *self);
} hit_flag_t;

extern hit_flag_t hit_flag;

// ---------------------------------------------- //
/* 决策发送包格式 */
typedef __packed struct
{
	std_frame_header_t  FrameHeader;	// 帧头
	uint16_t						cmd_id;
	sentry_cmd_t				TxData;		    // 数据
	uint16_t						FrameTail;	
}judge_decision_tx_packet_t;

uint8_t Judge_Decision_send(void);

#endif
