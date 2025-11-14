/* Includes ------------------------------------------------------------------*/
#include "ui_draw.h"

// NOTE:		ui 绘制分类
// 1、实时更新的数据
// 	底盘朝向、剩余弹量、cap
// 2、更新才更新的数据 -> 优先级
//  底盘、云台、发射、视觉状态
//	视觉识别到目标、自动打弹->准星变化
//	摩擦轮未开启、视觉识别到目标、准星都要有变化
// 3、只需要绘制一次的数据（可能存在丢包）
// 	视觉识别框、mode(mode 加入到 2 里面)、车框

// 关于准星
// 1、未开启摩擦轮		X
// 2、只开启摩擦轮		+
// 3、视觉识别到目标	识别框图加粗
// 4、视觉自动打弹		准星 + 加粗/变色
// 5、视觉控制云台		因为控制云台的前提是识别到目标，
//										所以这里可以把视觉识别框加粗表示
//										单纯识别到目标可以通过画一个动态框图表示(根据视觉的 target_pitch 和 target_yaw)
// 6、视觉控制发射

// 还需要加入进入自瞄标志位		识别到目标/大符		进入自瞄		自动打弹

/* Private function prototypes -----------------------------------------------*/
static void	UI_Init(struct UI_struct_t* UI);
static void	UI_Draw(struct UI_struct_t* UI);
static void	UI_Check(struct UI_struct_t* UI);
static void	UI_Update(struct UI_struct_t* UI);

static void UI_SendGraph1(UI_t*	UI);
static void UI_SendGraph2(UI_t*	UI);
static void UI_SendGraph3(UI_t*	UI);
static void UI_SendGraph4(UI_t*	UI);
static void UI_SendGraph5(UI_t*	UI);
static void UI_Send_Char(UI_t*	UI, char_num_e num);


ext_graphic_seven_data_t	graph1;		// 1g
ext_graphic_seven_data_t	graph2;		// 2g
ext_graphic_seven_data_t	graph3;		// 3g
ext_graphic_seven_data_t	graph4;		// 4g
ext_graphic_seven_data_t	graph5;		// 5g

/* Private variables ---------------------------------------------------------*/
uint8_t	char_buf[30];
uint8_t	ui_priority[3][8];

ext_charstring_data_t			char_shoot;			// 视觉 + 自动打弹	ch1
ext_charstring_data_t			char_vision;		// 视觉 + 进入自瞄	ch2
ext_charstring_data_t			char_launch;		// 发射 + 摩擦轮		ch3
ext_charstring_data_t			char_gimbal;		// 云台 + 陀螺仪		ch4
ext_charstring_data_t			char_chassis;		// 底盘 + 小陀螺		ch5

UI_Cnt_t	ui_cnt;
UI_Info_t	ui_info;
UI_Conf_t	ui_conf=
{
	.quasi_star_x = 940,					// 准星横坐标
	.quasi_star_y	= 540,					// 准星纵坐标
	
	.client_mid_x = 960,					// 页面中心横坐标
	.client_mid_y = 540,					// 页面中心纵坐标
	
	.l_switch_x		=	500,					// 左开关圆心横坐标
	.r_switch_x		=	1500,					// 右开关圆心横坐标
	
	.gimbal_switch_y			=	840,	// 云台开关圆心纵坐标
	.launch_switch_y			= 790,	// 发射开关圆心纵坐标
	.aim_on_switch_y			=	800,	// 自瞄开关圆心纵坐标
	.chassis_switch_y			=	740,	// 陀螺开	关圆心纵坐标
	.auto_shoot_switch_y	=	725,	// 自动打弹开关纵坐标
	
	// 字符 x 左移 450		y 上移 10
	// 字符 x 右移 50			y 上移 10
	
	.vision_l_d_x	=	690,					// 视觉识别框左下角横坐标
	.vision_l_d_y	=	320,					// 视觉识别框左下角纵坐标
	.vision_r_u_x	=	1210,					// 视觉识别框右上角横坐标
	.vision_r_u_y	=	630,					// 视觉识别框右上角纵坐标
	
	.chassis_r			=	100,				// 底盘半径
	.chassis_mid_x	=	1270,				// 底盘中心横坐标
	.chassis_mid_y	=	160,				// 底盘中心纵坐标
};

UI_t	ui=
{
	.cnt		=	&ui_cnt,
	.conf		=	&ui_conf,
	.info		= &ui_info,
	
	.init		= UI_Init,
	.draw		=	UI_Draw,
	.check	=	UI_Check,
	.update	=	UI_Update,
};

/* Exported functions --------------------------------------------------------*/
/**
  * @brief	Init UI struct
  * @param	UI_struct_t* UI
  * @retval	void
  */
void	UI_Init(struct UI_struct_t* UI)
{
	memset(ui_priority, 0, sizeof(ui_priority));
	
	UI->info->last_armor_hit_cnt = UI->info->armor_hit_cnt;
	
	// 代办：删除所有图层
	
}

/**
  * @brief	UI 数据发送
  * @param	UI_struct_t* UI
  * @retval	void
  */
void	UI_Draw(struct UI_struct_t* UI)
{
	static uint8_t static_cnt;

	// ---------- First  priority begin ---------- //
	if(ui_priority[0][0] > 0 )
	{
		// armor hit check 
		if(ui_priority[0][1] > 0)
		{
			UI_SendGraph1(UI);
			
			ui_priority[0][1] = 0;
			ui_priority[0][0] -- ;
			
			return;
		}
		
		// hit_enable
		if(ui_priority[0][2] > 0)
		{
			UI_SendGraph1(UI);
			
			ui_priority[0][2] = 0;
			ui_priority[0][0] -- ;
			
			return;
		}
		
		// spin + chassis
		if(ui_priority[0][3] > 0)
		{
			if(UI->cnt->draw_cnt < 2)							//	cnt = 0 or cnt = 1	->	add or modify
			{
				UI->cnt->draw_cnt ++;
				UI_Send_Char(UI, CHASSIS);
				
				return;
			}
			else
			{
				UI->cnt->draw_cnt = 0;
				UI_SendGraph1(UI);
				
				ui_priority[0][3] = 0;
				ui_priority[0][0] -- ;
				
				return;
			}
		}
	}
	// ---------- First  priority end   ---------- //
	
	// ---------- Second priority begin ---------- //
	else if(ui_priority[1][0] > 0)
	{
		// firc + launch
		if(ui_priority[1][1] > 0)
		{
			if(UI->cnt->draw_cnt < 2)							//	cnt = 0 or cnt = 1	->	add or modify
			{
				UI->cnt->draw_cnt ++;
				UI_Send_Char(UI, LAUNCH);
				
				return;
			}
			else
			{
				UI->cnt->draw_cnt = 0;
				UI_SendGraph2(UI);
				
				ui_priority[1][1] = 0;
				ui_priority[1][0] -- ;
				
				return;
			}
		}
		
		// target
		if(ui_priority[1][2] > 0)
		{
			UI_SendGraph2(UI);
			
			ui_priority[1][2] = 0;
			ui_priority[1][0] -- ;
			
			return;
		}
		
		// aim_on + vision
		if(ui_priority[1][3] > 0)
		{
			if(UI->cnt->draw_cnt < 2)							//	cnt = 0 or cnt = 1	->	add or modify
			{
				UI->cnt->draw_cnt ++;
				UI_Send_Char(UI, VIS_AIM_ON);
				
				return;
			}
			else
			{
				UI->cnt->draw_cnt = 0;
				UI_SendGraph2(UI);
				
				ui_priority[1][3] = 0;
				ui_priority[1][0] -- ;
				
				return;
			}
		}
		
		// auto_shoot + vision
		if(ui_priority[1][4] > 0)
		{
			if(UI->cnt->draw_cnt < 2)							//	cnt = 0 or cnt = 1	->	add or modify
			{
				UI->cnt->draw_cnt ++;
				UI_Send_Char(UI, VIS_SHOOT);
				
				return;
			}
			else
			{
				UI->cnt->draw_cnt = 0;
				UI_SendGraph2(UI);
				
				ui_priority[1][4] = 0;
				ui_priority[1][0] -- ;
				
				return;
			}
		}
		
		// infantry + gimbal
		if(ui_priority[1][5] > 0)
		{
			if(UI->cnt->draw_cnt < 2)							//	cnt = 0 or cnt = 1	->	add or modify
			{
				UI->cnt->draw_cnt ++;
				UI_Send_Char(UI, GIMBAL);
				
				return;
			}
			else
			{
				UI->cnt->draw_cnt = 0;
				UI_SendGraph2(UI);
				
				ui_priority[1][5] = 0;
				ui_priority[1][0] -- ;
				
				return;
			}
		}
	}
	// ---------- Second priority end   ---------- //
	
	// ---------- don't need priority begin   ---------- //
	static_cnt ++;
	static_cnt %= 201;
	
	if(static_cnt != 100 && static_cnt != 200)
		UI_SendGraph3(UI);
	else if(static_cnt == 100)				// 不需要实时更新的
		UI_SendGraph4(UI);
	else if(static_cnt == 200)
		UI_SendGraph5(UI);
	// ---------- don't need priority end     ---------- //
}

// Note：	其实 ui_priority[x][0] 是一行数组的和
//				可以通过求行数组的和转化成 n 个一维数组

/**
  * @brief	UI 信息优先级设置
  * @param	UI_struct_t* UI
  * @retval	void
  */
void	UI_Check(struct UI_struct_t* UI)
{
	state_info_t state 			= UI->info->state;
	state_info_t last_state = UI->info->last_state;

	// ---------- First  priority begin ---------- //
	// armor hit check
	if(state.hurt_check != last_state.hurt_check)
	{
		ui_priority[0][0] ++ ;
		last_state.hurt_check = state.hurt_check;
		ui_priority[0][1] ++;
	}
	
	// hit_enable
//	if(state.vis_hit_enable != last_state.vis_hit_enable)
//	{
//		ui_priority[0][0] ++ ;
//		last_state.vis_hit_enable = state.vis_hit_enable;
//		ui_priority[0][2] ++;
//	}
// 视觉自动打弹那里，就比如反陀螺时 vis_shoot_switch 变化是很快的，
// 会不会导致一直抢占优先级？？
	
	// spin + chassis
	if(state.spin_switch != last_state.spin_switch)
	{
		ui_priority[0][0]++;
		last_state.spin_switch = state.spin_switch;
		ui_priority[0][3]++;
	}
	if (state.chassis_state != last_state.chassis_state)		// 底盘状态		0在线 1离线
	{
		ui_priority[0][0]++;
		last_state.chassis_state = state.chassis_state;
		ui_priority[0][3]++;
	}
	// ---------- First  priority end   ---------- //


	// ---------- Second priority begin ---------- //
	// fric + launch
	if (state.fric_switch != last_state.fric_switch)				// 摩擦轮开关	0关 		1开
	{
		ui_priority[1][0]++;
		last_state.fric_switch = state.fric_switch;
		ui_priority[1][1]++;
	}
	if (state.launch_state != last_state.launch_state)			// 发射状态		0在线 1离线
	{
		ui_priority[1][0]++;
		last_state.launch_state = state.launch_state;
		ui_priority[1][1]++;
	}
	
	// target
	if (state.is_find_target != last_state.is_find_target)	// 识别目标		0未识别	1识别
	{
		ui_priority[1][0]++;
		last_state.is_find_target = master.info->is_find_target;
		ui_priority[1][2]++;
	}
	
	// aim_on_swicth + vision
	if (state.aim_on_swicth != last_state.aim_on_swicth)
	{
		ui_priority[1][0]++;
		last_state.aim_on_swicth = state.aim_on_swicth;
		ui_priority[1][3]++;
	}
	if (state.vision_state != last_state.vision_state)			// 视觉状态		0在线 1离线
	{
		ui_priority[1][0]++;
		last_state.vision_state = state.vision_state;
		ui_priority[1][3]++;
	}
	
	// vis_ctrl_launch
	if (state.vis_ctrl_launch != last_state.vis_ctrl_launch)
	{
		ui_priority[1][0]++;
		last_state.vis_ctrl_launch = state.vis_ctrl_launch;
		ui_priority[1][4]++;
	}
	
	// infantry + gimbal
	if (state.infantry_state != last_state.infantry_state)	// 机械模式 	0陀螺仪	1机械
	{
		ui_priority[1][0]++;
		last_state.infantry_state = state.infantry_state;
		ui_priority[1][5]++;
	}
	if (state.gimbal_state != last_state.gimbal_state)			// 云台状态		0在线 1离线
	{
		ui_priority[1][0]++;
		last_state.gimbal_state = state.gimbal_state;
		ui_priority[1][5]++;
	}
	// ---------- Second priority end   ---------- //
	
	
	// ---------- Third priority begin  ---------- //
	// ---------- Third priority end   ---------- //
}

/**
  * @brief	UI info 更新与绘制有关的数据
  * @param	UI_struct_t* UI
  * @retval	void
  */
void	UI_Update(struct UI_struct_t* UI)
{
	// yaw_angle 未归中

	UI->info->yaw_angle							= master.info->yaw_angle / 22.7555556f;
	UI->info->state.gimbal_state		= master.info->gimbal_state;
	UI->info->state.launch_state		= master.info->launch_state;
	UI->info->state.vision_state		= master.info->vision_state;
	UI->info->state.chassis_state		= master.info->chassis_state;
	
	UI->info->state.spin_switch			= master.info->spin_switch;
	UI->info->state.fric_switch			= master.info->fric_switch;
	UI->info->state.aim_on_swicth		= master.info->aim_on_switch;
	UI->info->state.infantry_state	= master.info->infantry_state;
	UI->info->state.is_find_target	= master.info->is_find_target;
	UI->info->state.vis_hit_enable	= master.info->vision_hit_enable;
	UI->info->state.vis_ctrl_launch	= master.info->vis_ctrl_launch;
	
	if((cap.state == CAP_ONLINE) && (cap.info.rx.cap_state.state == 0))
		UI->info->state.cap_state = 1;
	else
		UI->info->state.cap_state = 0;
	
	UI->info->armor_hit_cnt	=	judge.info->armor_hit_cnt;			// 此值变化表示受击
	UI->info->armor_id			=	judge.info->robot_hurt.armor_id;
	UI->info->remain_buttle	=	judge.info->bullet_remaining.projectile_allowance_17mm;
	// uint16_t	->	int16_t
	
	if (cap.info.rx.cap_Ucr > 3.5f)
		UI->info->cap_remain_v  = (uint16_t)((cap.info.rx.cap_Ucr - 3.5f) * 35.5f);
	else
		UI->info->cap_remain_v  = 32;
		
	UI->info->cap_percentage = UI->info->cap_remain_v * 100.f / 710.f;
	
	if(UI->info->armor_hit_cnt != UI->info->last_armor_hit_cnt)
	{
		UI->info->state.hurt_check = 1;
	
		// yaw_angle 0 - 360
		switch(UI->info->armor_id)
		{
			case 0x01:
				UI->info->hurt_start_angle = 315.f + UI->info->yaw_angle;
				UI->info->hurt_end_angle	 = 45.f  + UI->info->yaw_angle;
				break;
				
			case 0x02:
				UI->info->hurt_start_angle = 225.f + UI->info->yaw_angle;
				UI->info->hurt_end_angle	 = 315.f + UI->info->yaw_angle;
				break;
				
			case 0x03:
				UI->info->hurt_start_angle = 135.f + UI->info->yaw_angle;
				UI->info->hurt_end_angle	 = 225.f + UI->info->yaw_angle;
				break;
				
			case 0x04:
				UI->info->hurt_start_angle = 45.f  + UI->info->yaw_angle;
				UI->info->hurt_end_angle	 = 135.f + UI->info->yaw_angle;
				break;
				
			default:
				UI->info->last_armor_hit_cnt = UI->info->armor_hit_cnt;
				break;
		}
	}
	
	while(UI->info->hurt_start_angle > 360)
		UI->info->hurt_start_angle -= 360;
	while(UI->info->hurt_end_angle > 360)
		UI->info->hurt_end_angle -= 360;
	
	if(UI->info->state.hurt_check == 1)
	{
		UI->cnt->hurt_cnt ++;
		
		if(UI->cnt->hurt_cnt > HURT_UI_TIME_MAX)
		{
			UI->info->state.hurt_check = 0;
			UI->cnt->hurt_cnt = 0;
		}
	}
	else
		UI->cnt->hurt_cnt = 0;
		
	// Calculate the position of Rudder
	float r = UI->conf->chassis_r, x0 = UI->conf->chassis_mid_x, y0 = UI->conf->chassis_mid_y;
	float angle = UI->info->yaw_angle + 45.f;
	
	UI->info->rudder_f_x = x0 - r * sin(angle);
	UI->info->rudder_f_x = y0 + r * cos(angle);
	UI->info->rudder_b_x = x0 + r * sin(angle);
	UI->info->rudder_b_y = y0 - r * cos(angle);
	
}



// -------------------- ui draw -------------------- //
/* Private functions ---------------------------------------------------------*/
/**
	* @brief  第一组图形绘制
  * @param  
  * @retval 
  */
void UI_SendGraph1(UI_t*	UI)
{
	static uint8_t cnt = 0;

	uint8_t operate_type;
	uint8_t chassis_color;
	
	// armor hit check
	uint16_t start_angle = UI->info->hurt_start_angle;
	uint16_t end_angle	 = UI->info->hurt_end_angle;
	
	cnt ++;
	
	if(UI->info->state.hurt_check == 1)
		operate_type = ADD;
	else if(UI->info->state.hurt_check == 0)
		operate_type = DELETE;
	
	graph1.clientData[0] = draw_arc("1g1", operate_type, 0, PINK, start_angle, end_angle, \
																	10, UI->conf->client_mid_x,	UI->conf->client_mid_y, 300, 300);
	
	// hit_enable
//	if(UI->info->state.vis_hit_enable == 1)
//		operate_type = ADD;
//	else if(UI->info->state.vis_hit_enable == 0)
//		operate_type = DELETE;
//		
//	graph1.clientData[1] = draw_circle("1g2", operate_type, 1, RED_BLUE, \
//																	10, UI->conf->quasi_star_x, UI->conf->quasi_star_y, 50);
	
	// spin + chassis
	if(UI->info->state.chassis_state == 0)			// 底盘在线
	{
		if(UI->info->state.spin_switch == 1)			// 小陀螺打开
			chassis_color = GREEN;
		else if(UI->info->state.spin_switch == 0)
			chassis_color = WHITE;
	}
	else if(UI->info->state.chassis_state == 1)
		chassis_color = BLACK;
		
	if(cnt == 0)
		operate_type = ADD;
	else if(cnt < GRAPH_ADD_TIME)
		operate_type = MODIFY;
	else
		cnt = 0;
		
	graph1.clientData[2] = draw_circle("1g3", operate_type, 3, chassis_color, 15, \
																			UI->conf->l_switch_x, UI->conf->chassis_switch_y, 15);
	
	client_send_seven_graphic(graph1);
}

/**
	* @brief  第二组图形绘制
  * @param  
  * @retval 
  */
void UI_SendGraph2(UI_t*	UI)
{
	static uint8_t cnt = 0;
	
	uint8_t color, width;
	uint8_t operate_type;
	
	uint16_t start_x1, start_y1, end_x1, end_y1;		// 准星绘制
	uint16_t start_x2, start_y2, end_x2, end_y2;		// 准星绘制
	
	cnt ++;

	if(cnt == 0)
		operate_type = ADD;
	else if(cnt < GRAPH_ADD_TIME)
		operate_type = MODIFY;
	else
		cnt = 0;
	
	// fric + launch
	if(UI->info->state.launch_state == 0)				// 发射在线
	{
		if(UI->info->state.fric_switch == 1)			// 摩擦轮打开
			color = GREEN;
		else if(UI->info->state.fric_switch == 0)
			color = WHITE;
	}
	else if(UI->info->state.launch_state == 1)
		color = BLACK;
		
	graph2.clientData[0] = draw_circle("2g0", operate_type, 3, color, 15, \
																			UI->conf->l_switch_x, UI->conf->launch_switch_y, 15);
	
	// target
	if(UI->info->state.is_find_target == 1)
	{
		width = 8;
		color = ORANGE;
	}
	else
	{
		width = 2;
		color = RED_BLUE;
	}
	
	graph2.clientData[3] = draw_rectangle("2g3", operate_type, 2, color, width, \
																			UI->conf->vision_l_d_x, UI->conf->vision_l_d_y, \
																			UI->conf->vision_r_u_x, UI->conf->vision_r_u_y );
	
	
	
	// aim_on + vision
	if(UI->info->state.vision_state == 0)				// 视觉在线
	{
		if(UI->info->state.aim_on_swicth == 1)		// 自瞄打开
			color = GREEN;
		else if(UI->info->state.aim_on_swicth == 0)
			color = WHITE;
	}
	else if(UI->info->state.vision_state == 1)
		color = BLACK;
		
	graph1.clientData[4] = draw_circle("2g4", operate_type, 3, color, 15, \
																			UI->conf->r_switch_x, UI->conf->aim_on_switch_y, 15);
																			
	// auto_shoot + vision
	if(UI->info->state.vision_state == 0)				// 视觉在线
	{
		if(UI->info->state.vis_ctrl_launch == 1)	// 自动打弹
			color = GREEN;
		else if(UI->info->state.vis_ctrl_launch == 0)
			color = WHITE;
	}
	else if(UI->info->state.vision_state == 1)
		color = BLACK;
		
	graph1.clientData[5] = draw_circle("2g5", operate_type, 3, color, 15, \
																			UI->conf->r_switch_x, UI->conf->auto_shoot_switch_y, 15);
																			
	// infantry + gimbal
	if(UI->info->state.gimbal_state == 0)				// 云台在线
	{
		if(UI->info->state.infantry_state == 1)		// 机械模式
			color = GREEN;
		else if(UI->info->state.infantry_state == 0)
			color = WHITE;
	}
	else if(UI->info->state.gimbal_state == 1)
		color = BLACK;
		
	graph1.clientData[6] = draw_circle("2g6", operate_type, 3, color, 15, \
																			UI->conf->l_switch_x, UI->conf->gimbal_switch_y, 15);

	
	// 准星绘制
	if(UI->info->state.vision_state == 1)
	{
		color = BLACK;
		width = 3;
		
		start_x1 = UI->conf->quasi_star_x - 10;
		start_y1 = UI->conf->quasi_star_y - 10;
		end_x1 = UI->conf->quasi_star_x + 10;
		end_y1 = UI->conf->quasi_star_y + 10;
		
		start_x2 = UI->conf->quasi_star_x - 10;
		start_y2 = UI->conf->quasi_star_y + 10;
		end_x2 = UI->conf->quasi_star_x + 10;
		end_y2 = UI->conf->quasi_star_y - 10;
	}
	else if(UI->info->state.aim_on_swicth == 0)
	{
		color = RED_BLUE;
		width = 3;
		
		start_x1 = UI->conf->quasi_star_x;
		start_y1 = UI->conf->quasi_star_y - 14;
		end_x1 = UI->conf->quasi_star_x;
		end_y1 = UI->conf->quasi_star_y + 14;
		
		start_x2 = UI->conf->quasi_star_x - 14;
		start_y2 = UI->conf->quasi_star_y;
		end_x2 = UI->conf->quasi_star_x + 14;
		end_y2 = UI->conf->quasi_star_y;
	}
	else
	{
		color = ORANGE;
		width = 7;
		
		start_x1 = UI->conf->quasi_star_x;
		start_y1 = UI->conf->quasi_star_y - 14;
		end_x1 = UI->conf->quasi_star_x;
		end_y1 = UI->conf->quasi_star_y + 14;
		
		start_x2 = UI->conf->quasi_star_x - 14;
		start_y2 = UI->conf->quasi_star_y;
		end_x2 = UI->conf->quasi_star_x + 14;
		end_y2 = UI->conf->quasi_star_y;
	}
		
	
	graph2.clientData[1] = draw_line("2g1", operate_type, 1, color, width, \
																		start_x1, start_y1, end_x1, end_y1);
	
	graph2.clientData[0] = draw_line("2g2", operate_type, 1, color, width, \
																		start_x2, start_y2, end_x2, end_y2);
											
	client_send_seven_graphic(graph1);
}

/**
	* @brief  第三组图形绘制
  * @param  
  * @retval 
  */
void UI_SendGraph3(UI_t*	UI)
{
	static uint8_t cnt = 0;
	
	uint8_t operate_type, color;
	uint16_t remain_v = UI->info->cap_remain_v;
	
	cnt ++;

	if(cnt == 0)
		operate_type = ADD;
	else if(cnt < GRAPH_ADD_TIME)
		operate_type = MODIFY;
	else
		cnt = 0;
		
	// super cap remain v
	if (remain_v > 710)
	{
		remain_v = 710;
		color = GREEN;
	}
	if (remain_v <= 545)
		color = YELLOW;
	if (remain_v <= 355)
		color = ORANGE;
	
	// 图层数大的在上面
	if (UI->info->state.cap_state == 1)
	{
		graph3.clientData[0] = draw_rectangle("3g0", operate_type, 1, WHITE, 2, \
																					UI->conf->client_mid_x - 360, 10, \
																					UI->conf->client_mid_x + 360, 50);
		graph3.clientData[1] = draw_rectangle("3g1", operate_type, 1, color, 8, \
																					UI->conf->client_mid_x - 355, 20, \
																					UI->conf->client_mid_x - 355 + remain_v, 40);
		graph3.clientData[2] = draw_int("3g2", operate_type, 2, color, 25,	3,	\
																		900,	100,	UI->info->cap_percentage);
	}
	else
	{
		graph1.clientData[0] = draw_rectangle("3g0", operate_type, 1, BLACK, 2, \
																					UI->conf->client_mid_x - 360, 10, \
																					UI->conf->client_mid_x + 360, 50);
		graph1.clientData[1] = draw_rectangle("3g1", operate_type, 1, BLACK, 8, \
																					UI->conf->client_mid_x - 355, 20, \
																					UI->conf->client_mid_x + 355, 40);
		graph3.clientData[2] = draw_int("3g2", operate_type, 2, color, 25,	3,	\
																					900,	100,	0);
	}
	
	// Rudder position
	graph3.clientData[3] = draw_circle("3g3", operate_type, 8, YELLOW, 20,	\
																			UI->info->rudder_f_x, UI->info->rudder_f_y, 30);
	graph3.clientData[4] = draw_circle("3g4", operate_type, 8, YELLOW, 20,	\
																			UI->info->rudder_b_x, UI->info->rudder_b_y, 30);
		
	// remain buttle
	graph3.clientData[5] = draw_int("3g2", operate_type, 2, RED_BLUE, 60,	6,	\
																	 1360,	430,	UI->info->remain_buttle);
																					
	client_send_seven_graphic(graph3);
}

/**
	* @brief  第四组图形绘制
  * @param  
  * @retval 
  */
void UI_SendGraph4(UI_t*	UI)
{
//	static uint8_t cnt = 0;
//	
//	uint8_t operate_type;
//	
//	cnt ++;

//	if(cnt == 0)
//		operate_type = ADD;
//	else if(cnt < GRAPH_ADD_TIME)
//		operate_type = MODIFY;
//	else
//		cnt = 0;
		
	// 对于只要绘制一次的 UI 图像，UI 图像可以一直 ADD。
	
	// the whole of the chassis
	graph4.clientData[0] = draw_circle("4g0", ADD, 1, BLACK, 5,	\
																			UI->conf->chassis_mid_x, UI->conf->chassis_mid_y, 100);
	
	// the gimbal
	graph4.clientData[1] = draw_line("4g1", ADD, 1, BLACK, 5,	\
																		UI->conf->chassis_mid_x, UI->conf->chassis_mid_y, \
																		UI->conf->chassis_mid_x, UI->conf->chassis_mid_y + 140);
	graph4.clientData[2] = draw_line("4g2", ADD, 1, BLACK, 5,	\
																		UI->conf->chassis_mid_x, UI->conf->chassis_mid_y + 140, \
																		UI->conf->chassis_mid_x - 30, UI->conf->chassis_mid_y - 30);										
	graph4.clientData[3] = draw_line("4g3", ADD, 1, BLACK, 5,	\
																		UI->conf->chassis_mid_x, UI->conf->chassis_mid_y + 140, \
																		UI->conf->chassis_mid_x + 30, UI->conf->chassis_mid_y - 30);
	// lane of chassis

	client_send_seven_graphic(graph4);
}

/**
	* @brief  第五组图形绘制
  * @param  
  * @retval 
  */
void UI_SendGraph5(UI_t*	UI)
{
//	static uint8_t cnt = 0;
//	
//	uint8_t operate_type;
//	
//	cnt ++;

//	if(cnt == 0)
//		operate_type = ADD;
//	else if(cnt < GRAPH_ADD_TIME)
//		operate_type = MODIFY;
//	else
//		cnt = 0;
		
	// 对于只要绘制一次的 UI 图像，UI 图像可以一直 ADD。	
	
	// Gun line scale line
	// 枪线刻度
	graph5.clientData[0] = draw_line("5g0", ADD, 1, WHITE, 1, \
													UI->conf->quasi_star_x, UI->conf->quasi_star_y - 10, \
													UI->conf->quasi_star_x, UI->conf->quasi_star_y - 100);
	// 5m 枪线刻度
	graph2.clientData[1] = draw_line("5g1", ADD, 1, WHITE, 1, \
													UI->conf->quasi_star_x - 30, UI->conf->quasi_star_y - 40, \
													UI->conf->quasi_star_x + 30, UI->conf->quasi_star_y - 40);
	// 9m 线线刻度
	graph2.clientData[2] = draw_line("5g2", ADD, 1, WHITE, 1, \
													UI->conf->quasi_star_x - 15, UI->conf->quasi_star_y - 55, \
													UI->conf->quasi_star_x + 15, UI->conf->quasi_star_y - 55);
	// 11m 枪线刻度
	graph2.clientData[3] = draw_line("5g3", ADD, 1, WHITE, 1, \
													UI->conf->quasi_star_x - 8, UI->conf->quasi_star_y - 65, \
													UI->conf->quasi_star_x + 8, UI->conf->quasi_star_y - 65);										
	// 13m 枪线刻度
	graph2.clientData[4] = draw_line("5g4", ADD, 1, WHITE, 1, \
													UI->conf->quasi_star_x - 8, UI->conf->quasi_star_y - 85, \
													UI->conf->quasi_star_x + 8, UI->conf->quasi_star_y - 85);

	client_send_seven_graphic(graph5);
}

/**
	* @brief  字符绘制
  * @param  
  * @retval 
  */
void UI_Send_Char(UI_t*	UI, char_num_e num)
{
	uint8_t	cnt = UI->cnt->draw_cnt;
	uint8_t operate_tpye;
	uint8_t draw_color;
	
	if(cnt == 0)
		operate_tpye = ADD;
	else
		operate_tpye = MODIFY;
	
	switch (num)
	{
		case GIMBAL:
			if(UI->info->state.gimbal_state == 0)				// 云台在线
			{
				draw_color = WHITE;
			
				if(UI->info->state.infantry_state == 1)		// 机械模式
					strcpy((char *)char_buf, "gimbal_mode:  mec  ");
				else if(UI->info->state.infantry_state == 0)
					strcpy((char *)char_buf, "gimbal_mode:  imu  ");
			}
			else if(UI->info->state.gimbal_state == 1)
			{
				draw_color = BLACK;
				
				strcpy((char *)char_buf, "gimbal_mode:  FAIL ");
			}
			
			char_gimbal.clientData.grapic_data_struct = draw_char("ch4", operate_tpye, 3, draw_color, \
																														20, sizeof(char_buf), 2, \
																														UI->conf->l_switch_x - 450,
																														UI->conf->gimbal_switch_y + 10);
																														
			memcpy(char_gimbal.clientData.data, char_buf, sizeof(char_buf));
			memset(char_buf, (int32_t)'\0', sizeof(char_buf));
			client_send_char(char_gimbal);
			break;
			
		case LAUNCH:
			if(UI->info->state.launch_state == 0)				// 发射在线
			{
				draw_color = WHITE;
			
				if(UI->info->state.fric_switch == 1)			// 摩擦轮打开
					strcpy((char *)char_buf, "launch_mode:  open ");
				else if(UI->info->state.fric_switch == 0)
					strcpy((char *)char_buf, "launch_mode:  off  ");
			}
			else if(UI->info->state.launch_state == 1)
			{
				draw_color = BLACK;
				
				strcpy((char *)char_buf, "launch_mode:  FAIL ");
			}
			
			char_launch.clientData.grapic_data_struct = draw_char("ch3", operate_tpye, 3, draw_color, \
																														20, sizeof(char_buf), 2, \
																														UI->conf->l_switch_x - 450,
																														UI->conf->launch_switch_y + 10);
																														
			memcpy(char_launch.clientData.data, char_buf, sizeof(char_buf));
			memset(char_buf, (int32_t)'\0', sizeof(char_buf));
			client_send_char(char_launch);
			break;
			
		case CHASSIS:
			if(UI->info->state.chassis_state == 0)			// 底盘在线
			{
				draw_color = WHITE;
			
				if(UI->info->state.spin_switch == 1)			// 小陀螺打开
					strcpy((char *)char_buf, "chassis_mode: spin ");
				else if(UI->info->state.spin_switch == 0)
					strcpy((char *)char_buf, "chassis_mode: off  ");
			}
			else if(UI->info->state.chassis_state == 1)
			{
				draw_color = BLACK;
				
				strcpy((char *)char_buf, "chassis_mode: FAIL ");
			}
			
			char_chassis.clientData.grapic_data_struct = draw_char("ch5", operate_tpye, 3, draw_color, \
																														20, sizeof(char_buf), 2, \
																														UI->conf->l_switch_x - 450,
																														UI->conf->chassis_switch_y + 10);
																														
			memcpy(char_chassis.clientData.data, char_buf, sizeof(char_buf));
			memset(char_buf, (int32_t)'\0', sizeof(char_buf));
			client_send_char(char_chassis);
			break;
			
		case VIS_AIM_ON:
			if(UI->info->state.vision_state == 0)				// 视觉在线
			{
				draw_color = WHITE;
			
				if(UI->info->state.aim_on_swicth == 1)		// 进入自瞄
					strcpy((char *)char_buf, "vision_mode: aim_on ");
				else if(UI->info->state.aim_on_swicth == 0)
					strcpy((char *)char_buf, "vision_mode:  off   ");
			}
			else if(UI->info->state.vision_state == 1)
			{
				draw_color = BLACK;
				
				strcpy((char *)char_buf, "vision_mode:  FAIL  ");
			}
			
			char_vision.clientData.grapic_data_struct = draw_char("ch2", operate_tpye, 3, draw_color, \
																														20, sizeof(char_buf), 2, \
																														UI->conf->r_switch_x - 50,
																														UI->conf->aim_on_switch_y + 10);
																														
			memcpy(char_vision.clientData.data, char_buf, sizeof(char_buf));
			memset(char_buf, (int32_t)'\0', sizeof(char_buf));
			client_send_char(char_vision);
			break;
			
		case VIS_SHOOT:
			if(UI->info->state.vision_state == 0)				// 视觉在线
			{
				draw_color = WHITE;
			
				if(UI->info->state.vis_ctrl_launch == 1)	// 自动打弹
					strcpy((char *)char_buf, "shoot_mode:  auto ");
				else if(UI->info->state.vis_ctrl_launch == 0)
					strcpy((char *)char_buf, "shoot_mode:  off  ");
			}
			else if(UI->info->state.vision_state == 1)
			{
				draw_color = BLACK;
				
				strcpy((char *)char_buf, "shoot_mode:   FAIL  ");
			}
			
			char_shoot.clientData.grapic_data_struct = draw_char("ch1", operate_tpye, 3, draw_color, \
																														20, sizeof(char_buf), 2, \
																														UI->conf->r_switch_x + 50,
																														UI->conf->auto_shoot_switch_y + 10);
																														
			memcpy(char_shoot.clientData.data, char_buf, sizeof(char_buf));
			memset(char_buf, (int32_t)'\0', sizeof(char_buf));
			client_send_char(char_shoot);
			break;
		
		default:
			break;
	}
}
