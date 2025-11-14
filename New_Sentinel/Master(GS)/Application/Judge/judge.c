/**
  ************************************* Copyright ******************************   
  *                 (C) Copyright 2022, hwx, China, SZU.
  *                            N0   Rights  Reserved
  *                              
  *                   
  * @FileName   : rp_chassis.c   
  * @Version    : v1.1		
  * @Author     : hwx			
  * @Date       : 2023年2月28日         
  * @Description:    
  *
  *
  ******************************************************************************
 */
 
#include "judge_protocol.h"
#include "cap_protocol.h"
#include "judge.h"
#include "drv_can.h"
#include "remote.h"
#include "Car.h"
#include "crc.h"
#include "cap.h"
#include "vision.h"
#include "navigation.h"

extern rc_t                   rc_structure;
extern navigation_t           navigation_structure;

extern CAN_HandleTypeDef hcan1;

judge_frame_header_t judge_frame_header;

drv_judge_info_t drv_judge_info = {
	.frame_header = &judge_frame_header,
};
//shoot_data_t shoot_statistics;

judge_config_t judge_config = 
{
	.buffer_max = 200,
};

judge_base_info_t judge_base_info =
{
	.remain_bullte = 150,
};
judge_rawdata_t judge_rawdata;

judge_info_t judge_info = 
{
	.offline_cnt_max = 1000,
	
};

judge_t judge = 
{
	.config    = &judge_config,
	.base_info = &judge_base_info,
	.info      = &judge_info,
	.data      = &judge_rawdata,
};



uint8_t shoot_1_over_speed_flag = 105;
uint8_t shoot_2_over_speed_flag = 105;

uint32_t shoot_1_time = 0;
uint32_t shoot_2_time = 0;
uint32_t elapsed_time = 0;
float elapsed_seconds = 0;

extern uint8_t  must_patrol_enable;
shoot_data_t shoot_statistics;

void judge_heart()
{
	if(judge.info->offline_cnt ++ >= judge.info->offline_cnt_max)
	{
		judge.info->offline_cnt = judge.info->offline_cnt_max;
		judge.info->status = JUDGE_OFFLINE;
	}
	else
	{
		judge.info->status = JUDGE_ONLINE;
	}
	/*发送*/
	static uint16_t Map_cnt = 0;
	if(Map_cnt++ == 999)
	{
		Judge_MapSend();
		Map_cnt = 0;
	}
	
}

/*其中的所有函数，若为连接到裁判系统或裁判系统不发送该指令，则不会运行*/
void judge_update(uint16_t id, uint8_t *rxBuf)
{
	/*掉线计数器清零*/
	judge.info->offline_cnt = 0;
	
	switch(id)
	{
		case ID_rfid_status:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->rfid_status, rxBuf, LEN_rfid_status);
			/*数据处理-----------------------------------------------------------------------------------------*/
			break;
		case ID_game_state:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->game_status, rxBuf, LEN_game_state);
			/*数据处理-----------------------------------------------------------------------------------------*/
			judge.base_info->game_progress = judge.data->game_status.game_progress;
			break;
		
		case ID_power_heat_data:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->power_heat_data, rxBuf, LEN_power_heat_data);
			cap_send_2E();
			/*数据处理-----------------------------------------------------------------------------------------*/
			judge.base_info->shooter_cooling_limit    = judge.data->power_heat_data.shooter_id1_17mm_cooling_heat;
			judge.base_info->chassis_power_buffer     = judge.data->power_heat_data.chassis_power_buffer;
			judge.base_info->shooter_id1_cooling_heat = judge.data->power_heat_data.shooter_id1_17mm_cooling_heat;
			judge.base_info->shooter_id2_cooling_heat = judge.data->power_heat_data.shooter_id2_17mm_cooling_heat;
		
			break;
		case ID_game_robot_state:
			/*数据接收-----------------------------------------------------------------------------------------*/			
			memcpy(&judge.data->game_robot_status, rxBuf, LEN_game_robot_state);
			cap_send_2F();
			/*数据处理-----------------------------------------------------------------------------------------*/
			if(judge.data->game_robot_status.robot_id == 7)
			{
				judge.base_info->car_color = 0;
			}
			else if(judge.data->game_robot_status.robot_id == 107)
			{
				judge.base_info->car_color = 1;
			}
			judge.base_info->chassis_power_limit = judge.data->game_robot_status.chassis_power_limit;
			judge.base_info->last_HP             = judge.base_info->remain_HP;
			judge.base_info->remain_HP           = judge.data->game_robot_status.remain_HP;
			
			break;
		case ID_shoot_data:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->shoot_data, rxBuf, LEN_shoot_data);
			/*数据处理-----------------------------------------------------------------------------------------*/
			/*右1左2*/
			if(judge.data->shoot_data.shooter_id == 1)
			{
				judge.base_info->shooter_id1_speed = judge.data->shoot_data.bullet_speed;
				if(judge.base_info->shooter_id1_speed >= 29.5)
				{
					shoot_1_over_speed_flag -= 3;
				}
				else if(judge.base_info->shooter_id1_speed >= 28.5)
				{
					shoot_1_over_speed_flag -= 2;
				}
				else if(judge.base_info->shooter_id1_speed <= 27.5)
				{
					shoot_1_over_speed_flag += 1;
				}
			}
			else if(judge.data->shoot_data.shooter_id == 2)
			{
				judge.base_info->shooter_id2_speed = judge.data->shoot_data.bullet_speed;
				if(judge.base_info->shooter_id2_speed >=  29.5)
				{
					shoot_2_over_speed_flag -= 3;
				}
				else if(judge.base_info->shooter_id2_speed >= 28.5)
				{
					shoot_2_over_speed_flag -= 2;
				}
				else if(judge.base_info->shooter_id2_speed <= 27.5 )
				{
					shoot_2_over_speed_flag += 1;
				}
			}
			#if 0
			float s_speed = judge.data->shoot_data.bullet_speed;

			if (s_speed <= 27.0)
			{
				shoot_statistics.speed_260++;
				shoot_statistics.shoot_num--;
			}
			else if (s_speed >= 27.0 && s_speed <= 27.1)
			{
				shoot_statistics.speed_270++;
			}
			else if (s_speed >= 27.1 && s_speed <= 27.2)
			{
				shoot_statistics.speed_271++;
			}
			else if (s_speed >= 27.2 && s_speed <= 27.3)
			{
				shoot_statistics.speed_272++;
			}
			else if (s_speed >= 27.3 && s_speed <= 27.4)
			{
				shoot_statistics.speed_273++;
			}
			else if (s_speed >= 27.4 && s_speed <= 27.5)
			{
				shoot_statistics.speed_274++;
			}
			else if (s_speed >= 27.5 && s_speed <= 27.6)
			{
				shoot_statistics.speed_275++;
			}
			else if (s_speed >= 27.6 && s_speed <= 27.7)
			{
				shoot_statistics.speed_276++;
			}
			else if (s_speed >= 27.7 && s_speed <= 27.8)
			{
				shoot_statistics.speed_277++;
			}
			else if (s_speed >= 27.8 && s_speed <= 27.9)
			{
				shoot_statistics.speed_278++;
			}
			else if (s_speed >= 27.9 && s_speed <= 28.0)
			{
				shoot_statistics.speed_279++;
			}
			else if (s_speed >= 28.0 && s_speed <= 28.1)
			{
				shoot_statistics.speed_280++;
			}
			else if (s_speed >= 28.1 && s_speed <= 28.2)
			{
				shoot_statistics.speed_281++;
			}
			else if (s_speed >= 28.2 && s_speed <= 28.3)
			{
				shoot_statistics.speed_282++;
			}
			else if (s_speed >= 28.3 && s_speed <= 28.4)
			{
				shoot_statistics.speed_283++;
			}
			else if (s_speed >= 28.4 && s_speed <= 28.5)
			{
				shoot_statistics.speed_284++;
			}
			else if (s_speed >= 28.5 && s_speed <= 28.6)
			{
				shoot_statistics.speed_285++;
			}
			else if (s_speed >= 28.6 && s_speed <= 28.7)
			{
				shoot_statistics.speed_286++;
			}
			else if (s_speed >= 28.7 && s_speed <= 28.8)
			{
				shoot_statistics.speed_287++;
			}
			else if (s_speed >= 28.8 && s_speed <= 28.9)
			{
				shoot_statistics.speed_288++;
			}
			else if (s_speed >= 28.9 && s_speed <= 29.0)
			{
				shoot_statistics.speed_289++;
			}
			else if (s_speed >= 29.0)
			{
				shoot_statistics.speed_290++;
				shoot_statistics.shoot_num--;
			}
			
			/*统计部分*/
			shoot_statistics.shoot_num++;
			
			shoot_statistics.mean = ( shoot_statistics.speed_270 * 27.0 + shoot_statistics.speed_271 * 27.1 + shoot_statistics.speed_272 * 27.2 + shoot_statistics.speed_273 * 27.3 + shoot_statistics.speed_274 * 27.4 + shoot_statistics.speed_275 * 27.5 + shoot_statistics.speed_276 * 27.6 + shoot_statistics.speed_277 * 27.7 + shoot_statistics.speed_278 * 27.8 + shoot_statistics.speed_279 * 27.9 + shoot_statistics.speed_280 * 28.0 + shoot_statistics.speed_281 * 28.1 + shoot_statistics.speed_282 * 28.2 + shoot_statistics.speed_283 * 28.3 + shoot_statistics.speed_284 * 28.4 + shoot_statistics.speed_285 * 28.5 + shoot_statistics.speed_286 * 28.6 + shoot_statistics.speed_287 * 28.7 + shoot_statistics.speed_288 * 28.8 + shoot_statistics.speed_289 * 28.9 ) / (float)shoot_statistics.shoot_num;
			
			// Calculate the variance
			float sum_of_squares = (shoot_statistics.speed_270 * ((27.0 - shoot_statistics.mean) * (27.0 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_271 * ((27.1 - shoot_statistics.mean) * (27.1 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_272 * ((27.2 - shoot_statistics.mean) * (27.2 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_273 * ((27.3 - shoot_statistics.mean) * (27.3 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_274 * ((27.4 - shoot_statistics.mean) * (27.4 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_275 * ((27.5 - shoot_statistics.mean) * (27.5 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_276 * ((27.6 - shoot_statistics.mean) * (27.6 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_277 * ((27.7 - shoot_statistics.mean) * (27.7 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_278 * ((27.8 - shoot_statistics.mean) * (27.8 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_279 * ((27.9 - shoot_statistics.mean) * (27.9 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_280 * ((28.0 - shoot_statistics.mean) * (28.0 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_281 * ((28.1 - shoot_statistics.mean) * (28.1 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_282 * ((28.2 - shoot_statistics.mean) * (28.2 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_283 * ((28.3 - shoot_statistics.mean) * (28.3 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_284 * ((28.4 - shoot_statistics.mean) * (28.4 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_285 * ((28.5 - shoot_statistics.mean) * (28.5 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_286 * ((28.6 - shoot_statistics.mean) * (28.6 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_287 * ((28.7 - shoot_statistics.mean) * (28.7 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_288 * ((28.8 - shoot_statistics.mean) * (28.8 - shoot_statistics.mean))) +
								   (shoot_statistics.speed_289 * ((28.9 - shoot_statistics.mean) * (28.9 - shoot_statistics.mean))) ;

			shoot_statistics.variance = sum_of_squares / (float)shoot_statistics.shoot_num;
			
			#endif
			
			break;
		case ID_game_robot_pos:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->game_robot_pos, rxBuf, LEN_game_robot_pos);
			/*数据处理-----------------------------------------------------------------------------------------*/
			break;
		case ID_robot_hurt:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->ext_robot_hurt, rxBuf, LEN_robot_hurt);
			/*数据处理-----------------------------------------------------------------------------------------*/
			if(judge.data->ext_robot_hurt.hurt_type == ammo_HURT)
			{
				judge.base_info->armor_id      = judge.data->ext_robot_hurt.armor_id;
			}
			else
			{
				judge.base_info->armor_id      = 6;
			}
			
			break;
		case ID_aerial_data:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->ext_robot_command, rxBuf, LEN_aerial_data);
			/*数据处理-----------------------------------------------------------------------------------------*/
			//云台手命令处理完后清零
			judge.base_info->last_commond  = judge.data->ext_robot_command.commd_keyboard;
			judge.base_info->robot_commond = judge.data->ext_robot_command.commd_keyboard;
			
			break;
		case ID_game_robot_HP:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->game_robot_HP, rxBuf, LEN_game_robot_HP);
			/*数据处理-----------------------------------------------------------------------------------------*/
			if(judge.data->game_robot_status.robot_id == 7)
			{
				/*红色*/
				judge.base_info->friendly_outposts_HP = (uint8_t)(judge.data->game_robot_HP.red_outpost_HP/10);
				judge.base_info->enemy_outposts_HP = (uint8_t)(judge.data->game_robot_HP.blue_outpost_HP/10);
				judge.base_info->enemy_hero_HP = (uint8_t)(judge.data->game_robot_HP.blue_1_robot_HP/5);
				judge.base_info->enemy_engeing = (uint8_t)(judge.data->game_robot_HP.blue_2_robot_HP/5);
				judge.base_info->enemy_infanry3_HP = (uint8_t)(judge.data->game_robot_HP.blue_3_robot_HP/5);
				judge.base_info->enemy_infanry4_HP = (uint8_t)(judge.data->game_robot_HP.blue_4_robot_HP/5);
				judge.base_info->enemy_infanry5_HP = (uint8_t)(judge.data->game_robot_HP.blue_5_robot_HP/5);
				judge.base_info->enemy_Shaobing_HP = (uint8_t)(judge.data->game_robot_HP.blue_7_robot_HP/5);
				judge.base_info->remain_time = (uint8_t)(judge.data->game_status.stage_remain_time/5);

			}
			else if(judge.data->game_robot_status.robot_id == 107)
			{
				/*蓝色*/
				judge.base_info->friendly_outposts_HP = (uint8_t)(judge.data->game_robot_HP.blue_outpost_HP/10);
				judge.base_info->enemy_outposts_HP = (uint8_t)(judge.data->game_robot_HP.red_outpost_HP/10);
				judge.base_info->enemy_hero_HP = (uint8_t)(judge.data->game_robot_HP.red_1_robot_HP/5);
				judge.base_info->enemy_engeing = (uint8_t)(judge.data->game_robot_HP.red_2_robot_HP/5);
				judge.base_info->enemy_infanry3_HP = (uint8_t)(judge.data->game_robot_HP.red_3_robot_HP/5);
				judge.base_info->enemy_infanry4_HP = (uint8_t)(judge.data->game_robot_HP.red_4_robot_HP/5);
				judge.base_info->enemy_infanry5_HP = (uint8_t)(judge.data->game_robot_HP.red_5_robot_HP/5);
				judge.base_info->enemy_Shaobing_HP = (uint8_t)(judge.data->game_robot_HP.red_7_robot_HP/5);
				judge.base_info->remain_time = (uint8_t)(judge.data->game_status.stage_remain_time/5);

			}
			break;
		case ID_bullet_remaining:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->bullet_remaining, rxBuf, LEN_bullet_remaining);
			/*数据处理-----------------------------------------------------------------------------------------*/
			judge.base_info->remain_bullte       = judge.data->bullet_remaining.bullet_remaining_num_17mm;
			break;
		
		case ID_ground_robot_position:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->robot_position, rxBuf, LEN_ground_robot_position);
		default:
			break;
	}
}

void judge_recive(uint8_t *rxBuf)
{
	uint16_t frame_length;
	if( rxBuf == NULL )
	{
		return;
	}
	drv_judge_info.frame_header->SOF = rxBuf[0];
	if(drv_judge_info.frame_header->SOF == 0xA5)
	{
		memcpy(&drv_judge_info.frame_header->data_length, rxBuf + 1, 4);
		if(Verify_CRC8_Check_Sum(rxBuf, 5) == 1)
		{
			frame_length = 5 + 2 + drv_judge_info.frame_header->data_length + 2;
			if(Verify_CRC16_Check_Sum(rxBuf, frame_length) == 1)
			{
				memcpy(&drv_judge_info.cmd_id, rxBuf + 5, 2);
				judge_update(drv_judge_info.cmd_id, rxBuf + 7);
			}
			memcpy(&drv_judge_info.frame_tail, rxBuf + 5 + 2 + drv_judge_info.frame_header->data_length, 2);
		}
	}

	/* 如果一个数据包出现了多帧数据就再次读取 */
	if(rxBuf[frame_length] == 0xA5)
	{
		judge_recive( &rxBuf[frame_length] );
	}
}


/*---------发送部分-----------------*/
extern UART_HandleTypeDef huart6;//使用串口6


Judge_tx_packet_t      Judge_tx_structure;
uint8_t                Judge_txBuf[200]; 

void judge_Init()
{
	judge.base_info->armor_id = 6;
}

void Judge_send(void)
{
	/*数据发送*/	
	memcpy(Judge_txBuf, &Judge_tx_structure, sizeof(Judge_tx_structure));
	
	/*增加CRC校验位*/
	Append_CRC8_Check_Sum(Judge_txBuf, 5);
	Append_CRC16_Check_Sum(Judge_txBuf, LEN_map_sentry_data + 9);
	
	HAL_UART_Transmit_DMA(&huart6,Judge_txBuf,sizeof(Judge_tx_packet_t)); 
}


extern vision_t    vision_structure;
uint8_t     test_intention = 1;
uint16_t    start_position_x = 140;
uint16_t    start_position_y = 0;
int8_t      dx = 0;
int8_t      dy = 125;
void Judge_MapSend(void)
{
	Judge_tx_structure.FrameHeader.sof         = 0xA5;
	Judge_tx_structure.FrameHeader.data_length = LEN_map_sentry_data;
	Judge_tx_structure.FrameHeader.seq         = 0	;
	Judge_tx_structure.FrameHeader.cmd_id      = ID_map_sentry_data;
	
	Judge_tx_structure.TxData.intention        = test_intention;
	Judge_tx_structure.TxData.start_position_x = start_position_x;
	Judge_tx_structure.TxData.start_position_y = start_position_y;
	Judge_tx_structure.TxData.delta_x[0]       = dx;
	Judge_tx_structure.TxData.delta_y[0]       = dy;
	
	if(navigation_structure.tx_pack->TxData.navi_enable == 0)
	{
		Judge_tx_structure.TxData.delta_x[1]   = 50;
	}
	else
	{
		Judge_tx_structure.TxData.delta_x[1]   = 0;
	}
	
	if(must_patrol_enable == 1)
	{
		dy = 50;
	}
	else
	{
		dy = 125;	
	}
	Judge_send();
}
/*串口6中断回调函数*/
void USART6_rxDataHandler(uint8_t *rxBuf)
{
	judge_recive(rxBuf);
}

/*
556发 28.27 0.167

打算把摩擦轮PID的I调大一点 感觉不知道是响应不够快还是超调（误差预载 就相当于把I调大）

548发 28.26 0.185 

分段控制

556发 28.27 0.162

分段更精细，PD更大

564发 28.29 0.157

关闭弹速适应

614发 28.32 0.148

减小头上允许弹速波动的范围 允许误差20
*/