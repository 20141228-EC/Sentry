/* Includes ------------------------------------------------------------------*/
#include "judge_protocol.h"
#include "com_protocol.h"
#include "ui_protocol.h"

#include "cap.h"

/* Private function prototypes -----------------------------------------------*/
void Determine_ID(void);
void judge_send(judge_t *self,uint8_t *txbuf);
void judge_update(judge_t *judge_sen, uint8_t *rxBuf);

/* Exported functions --------------------------------------------------------*/
void USART5_rxDataHandler(uint8_t *rxBuf)
{
	judge.update(&judge, rxBuf);
}

void judge_send(judge_t *self,uint8_t *txbuf)
{
}

void judge_update(judge_t *judge_sen, uint8_t *rxBuf)
{
	uint16_t frame_length;
	uint16_t cmd_id;

	judge_info_t *judge_info = judge_sen->info;

	memcpy(&judge_info->fream_header, rxBuf, LEN_FRAME_HEAD);		// 5个字节
	
	/* 帧首字节是否为0xA5 */
	if(rxBuf[SOF] == JUDGE_FRAME_HEADER) 
	{
		/* 帧头CRC8校验 */
		if(Verify_CRC8_Check_Sum(rxBuf, LEN_FRAME_HEAD) == true) 
		{
			/* 统计一帧的总数据长度，用于CRC16校验 */     				// 2个字节
			frame_length = LEN_FRAME_HEAD + LEN_CMD_ID + judge_info->fream_header.data_length + LEN_FRAME_TAIL;
			
			if(Verify_CRC16_Check_Sum(rxBuf, frame_length) == true)
			{
				cmd_id = (rxBuf[CMD_ID+1] << 8 | rxBuf[CMD_ID]);
				
				switch(cmd_id)
				{
					case ID_game_state:								// 0x0001
						memcpy(&judge_info->game_status, (rxBuf+DATA_SEG), LEN_GAME_STATUS);
						game_status_send(1);
						break;
					
					case ID_game_result:							// 0x0002
						memcpy(&judge_info->game_result, (rxBuf+DATA_SEG), LEN_GAME_RESULT);
						break;
					
					// 所有机器人的血量都可以获得
					case ID_game_robot_HP:						// 0x0003
						memcpy(&judge_info->game_robot_HP, (rxBuf+DATA_SEG), LEN_GAME_ROBOT_HP);
						game_robot_HP_send(1);
						break;
					
					case ID_event_data:								// 0x0101
						memcpy(&judge_info->event_data, (rxBuf+DATA_SEG), LEN_EVENT_DATA);
						break;
					
					case ID_supply_projectile_action:	// 0x0102
						memcpy(&judge_info->supply_projectile_action, (rxBuf+DATA_SEG), LEN_SUPPLY_PROJECTILE_ACTION);
						break;
					
					case ID_referee_warning:					// 0x0104
						memcpy(&judge_info->referee_warning, (rxBuf+DATA_SEG), LEN_REFEREE_WARNING);
						break;
					
					case ID_dart_remaining_time:			// 0x0105
						memcpy(&judge_info->dart_remaining_time, (rxBuf+DATA_SEG), LEN_DART_REMAINING_TIME);
						break;
						
					/*2024.03.12 JSJ*/
					case ID_game_robot_state:					// 0x0201		热量限制
						memcpy(&judge_info->game_robot_status, (rxBuf+DATA_SEG), LEN_GAME_ROBOT_STATUS);
						Determine_ID();
						client_info_update();
						game_robot_status_send(1);
						break;
					
					case ID_power_heat_data:					// 0x0202		底盘缓冲，枪口热量
						memcpy(&judge_info->power_heat_data, (rxBuf+DATA_SEG), LEN_POWER_HEAT_DATA);
						power_heat_send(1);
						
						// 限制功率从而限制电流
						cap.modify_limit(&cap);
						
						cap.setdata(&cap, judge_info->power_heat_data.chassis_power_buffer, \
															judge_info->game_robot_status.chassis_power_limit, \
															judge_info->power_heat_data.chassis_volt, \
															judge_info->power_heat_data.chassis_current);
						cap_data_send(2);
						break;
					
					case ID_game_robot_pos:						// 0x0203
						memcpy(&judge_info->game_robot_pos, (rxBuf+DATA_SEG), LEN_GAME_ROBOT_POS);
						break;
					
					case ID_buff_musk:								// 0x0204
						memcpy(&judge_info->buff, (rxBuf+DATA_SEG), LEN_BUFF_MASK);
						break;
					
					case ID_aerial_robot_energy:			// 0x0205
						memcpy(&judge_info->aerial_robot_energy, (rxBuf+DATA_SEG), LEN_AERIAL_ROBOT_ENERGY);
						break;
					
					case ID_robot_hurt:								// 0x0206
						memcpy(&judge_info->robot_hurt, (rxBuf+DATA_SEG), LEN_ROBOT_HURT);
//						hurt_armor_check();
						robot_hurt_send(1);
						if(judge.info->robot_hurt.hurt_type == 0)
							judge.info->armor_hit_cnt ++;
						break;
					
					case ID_shoot_data:								// 0x0207		枪口射速
						memcpy(&judge_info->shoot_data, (rxBuf+DATA_SEG), LEN_SHOOT_DATA);
						shoot_data_send(1);
						break;
					
					case ID_projectile_allowance:			// 0x0208
						memcpy(&judge_info->bullet_remaining, (rxBuf+DATA_SEG), LEN_BULLET_REMAINING);
						break;
					
					case ID_rfid_status:							// 0x0209
						memcpy(&judge_info->rfid_status, (rxBuf+DATA_SEG), LEN_RFID_STATUS);
						break;
						
					case ID_dart_client_directive:		// 0x020A
						memcpy(&judge_info->dart_client,(rxBuf+DATA_SEG),LEN_DART_CLIENT_DIRECTIVE);
						break;
					
					case ID_ground_robot_position:		// 0x020B
						memcpy(&judge_info->ground_robot_position,(rxBuf+DATA_SEG),LEN_GROUND_ROBOT_POSITION);
						break;
					
					case ID_radar_mark_data:					// 0x020C
						memcpy(&judge_info->radar_mark_data,(rxBuf+DATA_SEG),LEN_RADAR_MARK_DATA);
						break;
					
//					case ID_COMMUNICATION: 
//						//JUDGE_ReadFromCom();
//						break;
				}
	
	      judge_sen->info->offline_cnt = 0;
				
				/* 帧尾CRC16下一字节是否为0xA5 */
				if(rxBuf[frame_length] == JUDGE_FRAME_HEADER)
				{
					/* 如果一个数据包出现了多帧数据就再次读取 */
					judge_update( judge_sen, &rxBuf[frame_length] );
				}
			}
		}
	}
}


/* Private functions ---------------------------------------------------------*/
//判断自己是哪个队伍
void Determine_ID(void)
{
	if(judge.info->game_robot_status.robot_id < 10)//本机器人的ID，红方
	{
		judge.info->my_color = 0;
		judge.info->ids.teammate_hero 		 = 1;
		judge.info->ids.teammate_engineer  = 2;
		judge.info->ids.teammate_infantry3 = 3;
		judge.info->ids.teammate_infantry4 = 4;
		judge.info->ids.teammate_infantry5 = 5;
		judge.info->ids.teammate_plane		 = 6;
		judge.info->ids.teammate_sentry		 = 7;
		
		judge.info->ids.client_hero 		 	= 0x0101;
		judge.info->ids.client_engineer   = 0x0102;
		judge.info->ids.client_infantry3  = 0x0103;
		judge.info->ids.client_infantry4  = 0x0104;
		judge.info->ids.client_infantry5  = 0x0105;
		judge.info->ids.client_plane			= 0x0106;
		
		if     (judge.info->game_robot_status.robot_id == hero_red)//不断刷新放置在比赛中更改颜色
			judge.info->self_client = judge.info->ids.client_hero;
		else if(judge.info->game_robot_status.robot_id == engineer_red)
			judge.info->self_client = judge.info->ids.client_engineer;
		else if(judge.info->game_robot_status.robot_id == infantry3_red)
			judge.info->self_client = judge.info->ids.client_infantry3;
		else if(judge.info->game_robot_status.robot_id == infantry4_red)
			judge.info->self_client = judge.info->ids.client_infantry4;
		else if(judge.info->game_robot_status.robot_id == infantry5_red)
			judge.info->self_client = judge.info->ids.client_infantry5;
		else if(judge.info->game_robot_status.robot_id == plane_red)
			judge.info->self_client = judge.info->ids.client_plane;
	}
	else //蓝方
	{
		judge.info->my_color = 1;
		judge.info->ids.teammate_hero 		 = 101;
		judge.info->ids.teammate_engineer  = 102;
		judge.info->ids.teammate_infantry3 = 103;
		judge.info->ids.teammate_infantry4 = 104;
		judge.info->ids.teammate_infantry5 = 105;
		judge.info->ids.teammate_plane		 = 106;
		judge.info->ids.teammate_sentry		 = 107;
		
		judge.info->ids.client_hero 		 	= 0x0165;
		judge.info->ids.client_engineer   = 0x0166;
		judge.info->ids.client_infantry3  = 0x0167;
		judge.info->ids.client_infantry4  = 0x0168;
		judge.info->ids.client_infantry5  = 0x0169;
		judge.info->ids.client_plane			= 0x016A;
		
		if     (judge.info->game_robot_status.robot_id == hero_blue)
			judge.info->self_client = judge.info->ids.client_hero;
		else if(judge.info->game_robot_status.robot_id == engineer_blue)
			judge.info->self_client = judge.info->ids.client_engineer;
		else if(judge.info->game_robot_status.robot_id == infantry3_blue)
			judge.info->self_client = judge.info->ids.client_infantry3;
		else if(judge.info->game_robot_status.robot_id == infantry4_blue)
			judge.info->self_client = judge.info->ids.client_infantry4;
		else if(judge.info->game_robot_status.robot_id == infantry5_blue)
			judge.info->self_client = judge.info->ids.client_infantry5;
		else if(judge.info->game_robot_status.robot_id == plane_blue)
			judge.info->self_client = judge.info->ids.client_plane;
	}
}
