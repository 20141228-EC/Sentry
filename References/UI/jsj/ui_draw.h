#ifndef __UI_DRAW_H
#define __UI_DRAW_H

/* Includes ------------------------------------------------------------------*/
#include "ui_protocol.h"
#include "master.h"
#include "judge.h"
#include "math.h"

/* Private macro -------------------------------------------------------------*/
#define HURT_UI_TIME_MAX	6				// 600	ms
#define GRAPH_ADD_TIME		10			// 1000 ms

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
	GIMBAL,				// 云台 + 陀螺仪
	LAUNCH,				// 发射 + 摩擦轮
	CHASSIS,			// 底盘 + 小陀螺
	
	VIS_AIM_ON,		// 进入自瞄状态
	VIS_SHOOT,		// 视觉自动打弹

	STATE_CNT,
	
} char_num_e;

typedef struct
{
	uint8_t spin_switch		 : 1;		// 小陀螺开关	0关 		1开
	uint8_t fric_switch 	 : 1;		// 摩擦轮开关	0关 		1开
	uint8_t aim_on_swicth	 : 1;		// 自瞄开关		0关			1开
	uint8_t infantry_state : 1;		// 机械模式 	0陀螺仪	1机械
	uint8_t is_find_target : 1;		// 识别目标		0未识别	1识别
	uint8_t vis_hit_enable : 1;		// 自动打弹		0不可打	1可打
	uint8_t	vis_ctrl_launch: 1;		// 视觉打弹		0关			1开
	
	uint8_t vision_state	 : 1;		// 视觉状态		0在线 	1离线
	uint8_t launch_state	 : 1;		// 发射状态		0在线 	1离线
	uint8_t gimbal_state	 : 1;		// 云台状态		0在线 	1离线
	uint8_t chassis_state	 : 1;		// 底盘状态		0在线		1离线
	
	uint8_t hurt_check		 : 1;		// 受击标志		0未受击	1受击
	uint8_t cap_state			 : 1;		// 超电状态		0关闭		1打开
	
} state_info_t;

typedef	struct	
{
	state_info_t state;
	state_info_t last_state;
	
	uint32_t		 armor_hit_cnt;
	uint32_t		 last_armor_hit_cnt;
	uint8_t			 armor_id;
	
	uint16_t		 hurt_start_angle;
	uint16_t		 hurt_end_angle;
	
	float				 yaw_angle;
	
	uint16_t		 rudder_f_x;
	uint16_t		 rudder_f_y;
	uint16_t		 rudder_b_x;
	uint16_t		 rudder_b_y;
	
	int16_t			 remain_buttle;		// -32768~+32767		// uint16_t	0~65535
	int16_t			 cap_percentage;
	uint16_t		 cap_remain_v;

}UI_Info_t;

typedef struct
{
	uint8_t hurt_cnt;
	
	uint8_t draw_cnt;
	
}UI_Cnt_t;

typedef struct
{
	uint16_t	client_mid_x;
	uint16_t	client_mid_y;

	uint16_t 	quasi_star_x;
	uint16_t	quasi_star_y;
	
	uint16_t	l_switch_x;
	uint16_t	r_switch_x;
	
	uint16_t	gimbal_switch_y;
	uint16_t	launch_switch_y;
	uint16_t	aim_on_switch_y;
	uint16_t	chassis_switch_y;
	uint16_t	auto_shoot_switch_y;
	
	uint16_t	vision_l_d_x;
	uint16_t	vision_l_d_y;
	uint16_t	vision_r_u_x;
	uint16_t	vision_r_u_y;
	
	uint16_t	chassis_r;
	uint16_t	chassis_mid_x;
	uint16_t	chassis_mid_y;

}UI_Conf_t;

typedef	struct UI_struct_t
{
	UI_Cnt_t*		cnt;
	
	UI_Conf_t*	conf;
	UI_Info_t*	info;

	void	(*init)(struct UI_struct_t* UI);
	void	(*draw)(struct UI_struct_t* UI);
	void	(*check)(struct UI_struct_t* UI);
	void	(*update)(struct UI_struct_t* UI);
	
}UI_t;

/* Exported variables --------------------------------------------------------*/
extern UI_t	ui;

#endif
