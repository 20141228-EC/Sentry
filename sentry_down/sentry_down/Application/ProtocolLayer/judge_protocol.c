/* Includes ------------------------------------------------------------------*/
#include "judge_protocol.h"
#include "can_protocol.h"

#include "cap.h"
#include "master.h"
#include "rp_math.h"

extern float cap_u;
extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;

#define GAME_STATUS_ID 				0x003	//	比赛状态		剩余时间		剩余弹量		剩余金币		己方颜色
#define GAME_ROBOT_HP_ID 			0x004	//	己方血量		敌方血量
#define DECISION_RX_ID				0x005	//	哨兵接收决策数据
#define ROBOT_CMD_ID					0x006	//	云台手控制数据
#define ROBOT_POSITION_ID			0x007	//	己方机器人坐标数据
#define SHOOT_DATA_ID 				0x008	//	弹丸射速
#define RADAR_POS_COOR_ID			0x101	//	雷达标点坐标
#define GAME_ROBOT_STATUS_ID 	0x102	//	机器人状态数据

/* Private function prototypes -----------------------------------------------*/
static void Determine_ID(void);
void Judge_Update(judge_t *judge_sen, uint8_t *rxBuf);

/* Exported functions --------------------------------------------------------*/
void USART6_rxDataHandler(uint8_t *rxBuf)
{
	judge.update(&judge, rxBuf);
}

uint16_t radar_test = 0;

void Judge_Update(judge_t *judge_sen, uint8_t *rxBuf)
{
	uint16_t frame_length;
	uint16_t cmd_id;
	
	judge_info_t *judge_info = judge_sen->info;

	memcpy(&judge_info->fream_header, rxBuf, LEN_FRAME_HEAD);
	
	if(rxBuf[SOF] == JUDGE_FRAME_HEADER) 
	{
		if(Verify_CRC8_Check_Sum(rxBuf, LEN_FRAME_HEAD) == true) 
		{
			frame_length = LEN_FRAME_HEAD + LEN_CMD_ID + judge_info->fream_header.data_length + LEN_FRAME_TAIL;
			
			if(Verify_CRC16_Check_Sum(rxBuf, frame_length) == true)
			{
				cmd_id = (rxBuf[CMD_ID+1] << 8 | rxBuf[CMD_ID]);
				
				switch(cmd_id)
				{
					// ------------ 有数据转发的裁判系统数据包 begin  ------------ //
					case ID_game_state:												//0x0001			//	1Hz
						memcpy(&judge_info->game_status, (rxBuf+DATA_SEG), LEN_GAME_STATUS);
						game_status_send(2);										//连接服务器才会发送
						break;
					
					case ID_game_robot_HP:										//0x0003	3Hz
						memcpy(&judge_info->game_robot_HP, (rxBuf+DATA_SEG), LEN_GAME_ROBOT_HP);
						game_robot_HP_send(2);									//连接服务器才会发送
						break;
					
					case ID_game_robot_state: 								//0x0201		10Hz
						memcpy(&judge_info->game_robot_status, (rxBuf+DATA_SEG), LEN_GAME_ROBOT_STATUS);
						Determine_ID();         
						game_robot_status_send(2);
						break;
						
					case ID_sentry_auto_info:									//0x020D
						memcpy(&judge_info->sentry_auto, (rxBuf+DATA_SEG), LED_SENTRY_AUTO_INFO);
						sentry_decision_send(2);
						break;
					
					case ID_shoot_data:												//0x0207
						memcpy(&judge_info->shoot_data, (rxBuf+DATA_SEG), LEN_SHOOT_DATA);
						shoot_data_send(2);
						break;
					
					case ID_robot_interactive_header_data:		//0x0301		// 机器人交互数据
						memcpy(&judge_info->radar_info, (rxBuf+DATA_SEG), 26);
						uint16_t tx_id = judge_info->radar_info.sender_id;
						uint16_t rx_id = judge_info->radar_info.receiver_id;
					radar_test = 0;
						if((tx_id == 9 || tx_id == 109) && (rx_id == 7 || rx_id == 107))
						{
							radar_pos_coor_send(2);
							radar_pos_coor_send(2);								// 循环交替发送两次
						}
						break;
						
					case ID_ground_robot_position:						//0x020B     // 己方地面单位(除哨兵)位置
						memcpy(&judge_info->ground_robot_position,(rxBuf+DATA_SEG),LEN_GROUND_ROBOT_POSITION);
						game_robot_position_send(2);
						break;
						
					case ID_map_interactive_header_data:			//0x0303	// 选手端小地图交互数据，选手端触发发送
						memcpy(&judge_info->robot_command_t, (rxBuf+DATA_SEG), LEN_MAP_INTERACTIVE_HEADERDATA);
						game_robot_cmd_send(2);
						break;
					
					case ID_power_heat_data: 									//0x0202		10Hz
						memcpy(&judge_info->power_heat_data, (rxBuf+DATA_SEG), LEN_POWER_HEAT_DATA);
						cap.setdata(&cap, judge_info->power_heat_data.chassis_power_buffer, \
															judge_info->game_robot_status.chassis_power_limit);
						break;
					
					case ID_projectile_allowance:							//0x0208		//17,42允许发弹量,剩余金币
						memcpy(&judge_info->bullet_remaining, (rxBuf+DATA_SEG), LEN_BULLET_REMAINING);
						break;

					case ID_robot_hurt:												//0x0206		//受击位置ID,受击类型
						memcpy(&judge_info->robot_hurt, (rxBuf+DATA_SEG), LEN_ROBOT_HURT);
						if(judge.info->robot_hurt.hurt_type == 0)
							hit_flag.hit_cnt = 0;
						break;
					// ------------ 无数据转发的裁判系统数据包 begin ------------ //
					case ID_game_robot_pos: 									//0x0203		//本机器人位置,枪口(即正面朝向)
						memcpy(&judge_info->game_robot_pos, (rxBuf+DATA_SEG), LEN_GAME_ROBOT_POS);
						break;
					
					case ID_event_data:								//0x0101
						memcpy(&judge_info->event_data, (rxBuf+DATA_SEG), LEN_EVENT_DATA);//(32位)
						break;
					
					case ID_game_result:							// 0x0002
						memcpy(&judge_info->game_result, (rxBuf+DATA_SEG), LEN_GAME_RESULT);
						break;
					
					case ID_supply_projectile_action:	//0x0102		//己方机器人补弹信息
						memcpy(&judge_info->supply_projectile_action, (rxBuf+DATA_SEG), LEN_SUPPLY_PROJECTILE_ACTION);
						break;
					
					case ID_referee_warning:					//0x0104
						memcpy(&judge_info->referee_warning, (rxBuf+DATA_SEG), LEN_REFEREE_WARNING);
						break;
					
					case ID_dart_remaining_time:			//0x0105
						memcpy(&judge_info->dart_remaining_time, (rxBuf+DATA_SEG), LEN_DART_REMAINING_TIME);
						break;
						
					case ID_buff_musk:								//0x0204		//增益与冷却倍率
						memcpy(&judge_info->buff, (rxBuf+DATA_SEG), LEN_BUFF_MASK); 
						break;
					
					case ID_aerial_robot_energy:			//0x0205		//空中支援状态
						memcpy(&judge_info->aerial_robot_energy, (rxBuf+DATA_SEG), LEN_AERIAL_ROBOT_ENERGY);
						break;
					
					case ID_rfid_status:							//0x0209		//增益点是否进入
						memcpy(&judge_info->rfid_status, (rxBuf+DATA_SEG), LEN_RFID_STATUS);
						break;
						
					case ID_dart_client_directive:		//0x020A     //飞镖发射站状态
						memcpy(&judge_info->dart_client,(rxBuf+DATA_SEG),LEN_DART_CLIENT_DIRECTIVE);
						break;
						
					case ID_radar_mark_data:					//0x020C		// 对方地面单位标记进度,<100时为0
						memcpy(&judge_info->radar_mark_data,(rxBuf+DATA_SEG),LEN_RADAR_MARK_DATA);
						break;
					// ------------ 无数据转发的裁判系统数据包 end   ------------ //
				}
			
	      judge_sen->info->offline_cnt = 0;
				
				if(rxBuf[frame_length] == JUDGE_FRAME_HEADER)
					Judge_Update( judge_sen, &rxBuf[frame_length] );
			}
		}
	}
}


/* Private functions ---------------------------------------------------------*/
/**
	* @brief  确定本机器人的ID,并根据ID确定颜色,客户端ID
*/
void Determine_ID(void)
{
	if(judge.info->game_robot_status.robot_id < 10)//本机器人的ID，红方
	{ 
		judge.info->my_color = 0;
		judge.info->ids.teammate_hero 	   = 1;
		judge.info->ids.teammate_engineer  = 2;
		judge.info->ids.teammate_infantry3 = 3;
		judge.info->ids.teammate_infantry4 = 4;
		judge.info->ids.teammate_infantry5 = 5;
		judge.info->ids.teammate_plane	   = 6;
		judge.info->ids.teammate_sentry	   = 7;
		
		judge.info->ids.client_hero		   = 0x0101;
		judge.info->ids.client_engineer    = 0x0102;
		judge.info->ids.client_infantry3   = 0x0103;
		judge.info->ids.client_infantry4   = 0x0104;
		judge.info->ids.client_infantry5   = 0x0105;
		judge.info->ids.client_plane	   = 0x0106;
		
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
		judge.info->ids.teammate_hero 	   = 101;
		judge.info->ids.teammate_engineer  = 102;
		judge.info->ids.teammate_infantry3 = 103;
		judge.info->ids.teammate_infantry4 = 104;
		judge.info->ids.teammate_infantry5 = 105;
		judge.info->ids.teammate_plane	   = 106;
		judge.info->ids.teammate_sentry	   = 107;
		
		judge.info->ids.client_hero 	   = 0x0165;
		judge.info->ids.client_engineer    = 0x0166;
		judge.info->ids.client_infantry3   = 0x0167;
		judge.info->ids.client_infantry4   = 0x0168;
		judge.info->ids.client_infantry5   = 0x0169;
		judge.info->ids.client_plane	   = 0x016A;
		
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


/* Exported functions --------------------------------------------------------*/
void game_status_send(uint8_t can_num)
{
	static uint8_t game_status_tx_buf[8];
	
	master.info->tx->game_status.my_color 		= judge.info->my_color;																		// 自己颜色	0	红方	1	蓝方
	master.info->tx->game_status.game_progress= judge.info->game_status.game_progress;									// 比赛状态	同裁判数据
	master.info->tx->game_status.cap_remain		= (uint8_t)(cap_u * cap_u / 576.f * 100.f);								// 超电剩余
	master.info->tx->game_status.remain_time 	= judge.info->game_status.stage_remain_time;							// 剩余时间(单位：s)
	master.info->tx->game_status.remain_gold 	= judge.info->bullet_remaining.coin_remaining_num;				// 剩余金币
	master.info->tx->game_status.remain_bullet= judge.info->bullet_remaining.bullet_remaining_num_17mm;	// 剩余弹量
	master.info->tx->game_status.shooter_flag	=	judge.info->game_robot_status.power_management_shooter_output;

	memcpy(game_status_tx_buf, (void*)&master.info->tx->game_status, 8);

	switch (can_num)
	{
		case 1:
			CAN_SendData(&hcan1, GAME_STATUS_ID, game_status_tx_buf);
			break;
		case 2:
			CAN_SendData(&hcan2, GAME_STATUS_ID, game_status_tx_buf);
			break;
		default:
			break;
	}

	memset(game_status_tx_buf, 0, 8);
}	

void game_robot_HP_send(uint8_t can_num)
{
	static uint8_t HP_status_cnt = 0;
	static uint8_t game_robot_HP_tx_buf[8];

	HP_status_cnt ++;

	if(HP_status_cnt % 2 == 0)
	{
		master.info->tx->game_robot_HP.HP_status = 0;	// 己方血量
		if(judge.info->my_color == 0)
		{
			master.info->tx->game_robot_HP.hero_HP			= judge.info->game_robot_HP.red_1_robot_HP / 500.f * 255.f;		// 英雄血量		0-500
			master.info->tx->game_robot_HP.engineer_HP	= judge.info->game_robot_HP.red_2_robot_HP / 250.f * 255.f;		// 工程血量		0-250
			master.info->tx->game_robot_HP.infantry3_HP = judge.info->game_robot_HP.red_3_robot_HP / 500.f * 255.f;		// 步兵3血量	0-500
			master.info->tx->game_robot_HP.infantry4_HP = judge.info->game_robot_HP.red_4_robot_HP / 500.f * 255.f;		// 步兵4血量	0-500
			master.info->tx->game_robot_HP.sentry_HP		= judge.info->game_robot_HP.red_7_robot_HP / 400.f * 255.f;		// 哨兵血量		0-400
			master.info->tx->game_robot_HP.outpost_HP		= judge.info->game_robot_HP.red_outpost_HP / 1500.f* 255.f;		// 前哨血量		0-1500
			master.info->tx->game_robot_HP.base_HP			= judge.info->game_robot_HP.red_base_HP 	 / 5000.f* 255.f;		// 基地血量		0-5000
		}
		else if(judge.info->my_color == 1)
		{
			master.info->tx->game_robot_HP.hero_HP			= judge.info->game_robot_HP.blue_1_robot_HP / 500.f * 255.f;	// 英雄血量		0-500
			master.info->tx->game_robot_HP.engineer_HP	= judge.info->game_robot_HP.blue_2_robot_HP / 250.f * 255.f;	// 工程血量		0-250
			master.info->tx->game_robot_HP.infantry3_HP = judge.info->game_robot_HP.blue_3_robot_HP / 500.f * 255.f;	// 步兵3血量	0-500
			master.info->tx->game_robot_HP.infantry4_HP = judge.info->game_robot_HP.blue_4_robot_HP / 500.f * 255.f;	// 步兵4血量	0-500
			master.info->tx->game_robot_HP.sentry_HP		= judge.info->game_robot_HP.blue_7_robot_HP / 400.f * 255.f;	// 哨兵血量		0-400
			master.info->tx->game_robot_HP.outpost_HP		= judge.info->game_robot_HP.blue_outpost_HP / 1500.f* 255.f;	// 前哨血量		0-1500
			master.info->tx->game_robot_HP.base_HP			= judge.info->game_robot_HP.blue_base_HP 	 / 5000.f * 255.f;		// 基地血量		0-5000
		}
	}
	else if(HP_status_cnt % 2 == 1)
	{
		master.info->tx->game_robot_HP.HP_status = 1;	// 敌方血量
		if(judge.info->my_color == 0)
		{
			master.info->tx->game_robot_HP.hero_HP			= judge.info->game_robot_HP.blue_1_robot_HP / 500.f * 255.f;	// 英雄血量		0-500
			master.info->tx->game_robot_HP.engineer_HP	= judge.info->game_robot_HP.blue_2_robot_HP / 250.f * 255.f;	// 工程血量		0-250
			master.info->tx->game_robot_HP.infantry3_HP = judge.info->game_robot_HP.blue_3_robot_HP / 500.f * 255.f;	// 步兵3血量	0-500
			master.info->tx->game_robot_HP.infantry4_HP = judge.info->game_robot_HP.blue_4_robot_HP / 500.f * 255.f;	// 步兵4血量	0-500
			master.info->tx->game_robot_HP.sentry_HP		= judge.info->game_robot_HP.blue_7_robot_HP / 400.f * 255.f;	// 哨兵血量		0-400
			master.info->tx->game_robot_HP.outpost_HP		= judge.info->game_robot_HP.blue_outpost_HP / 1500.f* 255.f;	// 前哨血量		0-1500
			master.info->tx->game_robot_HP.base_HP			= judge.info->game_robot_HP.blue_base_HP 	 / 5000.f * 255.f;	// 基地血量		0-5000
		}
		else if(judge.info->my_color == 1)
		{
			master.info->tx->game_robot_HP.hero_HP			= judge.info->game_robot_HP.red_1_robot_HP / 500.f * 255.f;	// 英雄血量		0-500
			master.info->tx->game_robot_HP.engineer_HP	= judge.info->game_robot_HP.red_2_robot_HP / 250.f * 255.f;	// 工程血量		0-250
			master.info->tx->game_robot_HP.infantry3_HP = judge.info->game_robot_HP.red_3_robot_HP / 500.f * 255.f;	// 步兵3血量	0-500
			master.info->tx->game_robot_HP.infantry4_HP = judge.info->game_robot_HP.red_4_robot_HP / 500.f * 255.f;	// 步兵4血量	0-500
			master.info->tx->game_robot_HP.sentry_HP		= judge.info->game_robot_HP.red_7_robot_HP / 400.f * 255.f;	// 哨兵血量		0-400
			master.info->tx->game_robot_HP.outpost_HP		= judge.info->game_robot_HP.red_outpost_HP / 1500.f* 255.f;	// 前哨血量		0-1500
			master.info->tx->game_robot_HP.base_HP			= judge.info->game_robot_HP.red_base_HP 	 / 5000.f* 255.f;	// 基地血量		0-5000
		}
	}

	memcpy(game_robot_HP_tx_buf, (void*)&master.info->tx->game_robot_HP, 8);

	switch (can_num)
	{
		case 1:
			CAN_SendData(&hcan1, GAME_ROBOT_HP_ID, game_robot_HP_tx_buf);
			break;
		case 2:
			CAN_SendData(&hcan2, GAME_ROBOT_HP_ID, game_robot_HP_tx_buf);
			break;
		default:
			break;
	}
	memset(game_robot_HP_tx_buf, 0, 8);
}

void game_robot_status_send(uint8_t can_num)
{
	static uint8_t game_robot_status_tx_buf[8];
	
	master.info->tx->game_robot_status.shooter_barrel_heat					= judge.info->power_heat_data.shooter_id1_17mm_cooling_heat;
	master.info->tx->game_robot_status.shooter_barrel_heat_limit		= judge.info->game_robot_status.shooter_barrel_heat_limit;	// 热量限制
	master.info->tx->game_robot_status.shooter_barrel_cooling_value	=	judge.info->game_robot_status.shooter_barrel_cooling_value;
	master.info->tx->game_robot_status.finish_small_buff = judge.info->event_data.finish_small_buff_state;

	memcpy(game_robot_status_tx_buf, (void*)&master.info->tx->game_robot_status, 8);
		
	switch (can_num)
	{
		case 1:
			CAN_SendData(&hcan1, GAME_ROBOT_STATUS_ID, game_robot_status_tx_buf);
			break;
		case 2:
			CAN_SendData(&hcan2, GAME_ROBOT_STATUS_ID, game_robot_status_tx_buf);
			break;
		default:
			break;
	}
	memset(game_robot_status_tx_buf, 0, 8);
}

void game_robot_position_send(uint8_t can_num)
{
	static uint8_t robot_pos_data_tx_buf[8];
	
	master.info->tx->robot_position.hero_x			= judge.info->ground_robot_position.hero_x / 28.f * 255.f;
	master.info->tx->robot_position.hero_y			= judge.info->ground_robot_position.hero_y / 28.f * 255.f;
	master.info->tx->robot_position.engineer_x	= judge.info->ground_robot_position.engineer_x / 28.f * 255.f;
	master.info->tx->robot_position.engineer_y	= judge.info->ground_robot_position.engineer_y / 28.f * 255.f;
	master.info->tx->robot_position.infantry3_x	= judge.info->ground_robot_position.standard_3_x / 28.f * 255.f;
	master.info->tx->robot_position.infantry3_y	= judge.info->ground_robot_position.standard_3_y / 28.f * 255.f;
	master.info->tx->robot_position.infantry4_x	= judge.info->ground_robot_position.standard_4_x / 28.f * 255.f;
	master.info->tx->robot_position.infantry4_y	=	judge.info->ground_robot_position.standard_4_y / 28.f * 255.f;
	
	memcpy(robot_pos_data_tx_buf, (void*)&master.info->tx->robot_position, 8);
	
	switch (can_num)
	{
		case 1:
			CAN_SendData(&hcan1, ROBOT_CMD_ID, robot_pos_data_tx_buf);
			break;
		case 2:
			CAN_SendData(&hcan2, ROBOT_CMD_ID, robot_pos_data_tx_buf);
			break;
		default:
			break;
	}

	memset(robot_pos_data_tx_buf, 0, 8);
}

void sentry_decision_send(uint8_t can_num)
{
	static uint8_t sentry_auto_data_tx_buf[8];
	
	memcpy((void*)&master.info->tx->sentry_auto, (void*)&judge.info->sentry_auto, 6);
	memcpy(sentry_auto_data_tx_buf, (void*)&master.info->tx->sentry_auto, 8);
	
	switch (can_num)
	{
		case 1:
			CAN_SendData(&hcan1, DECISION_RX_ID, sentry_auto_data_tx_buf);
			break;
		case 2:
			CAN_SendData(&hcan2, DECISION_RX_ID, sentry_auto_data_tx_buf);
			break;
		default:
			break;
	}

	memset(sentry_auto_data_tx_buf, 0, 8);
}

void game_robot_cmd_send(uint8_t can_num)
{
	static uint8_t robot_cmd_data_tx_buf[8];

	master.info->tx->robot_cmd.target_x = float_to_int16(judge.info->robot_command_t.target_position_x, \
																											28.f, 0.f, +32767, -32768);
	master.info->tx->robot_cmd.target_y = float_to_int16(judge.info->robot_command_t.target_position_y, \
																											15.f, 0.f, +32767, -32768);
	master.info->tx->robot_cmd.cmd_key	=	judge.info->robot_command_t.commd_keyboard;
	master.info->tx->robot_cmd.target_id = judge.info->robot_command_t.target_robot_ID;
	
	memcpy(robot_cmd_data_tx_buf, (void*)&master.info->tx->robot_cmd, 8);
	
	switch (can_num)
	{
		case 1:
			CAN_SendData(&hcan1, ROBOT_CMD_ID, robot_cmd_data_tx_buf);
			break;
		case 2:
			CAN_SendData(&hcan2, ROBOT_CMD_ID, robot_cmd_data_tx_buf);
			break;
		default:
			break;
	}

	memset(robot_cmd_data_tx_buf, 0, 8);

}

void radar_pos_coor_send(uint8_t can_num)
{
	static uint8_t radar_pos_coor_tx_buf[8], radar_raw_buf[16];
	static uint8_t radar_pos_coor_cnt = 0;
	radar_pos_coor_cnt ++;						//	数据包ID，交替发送

	master.info->tx->radar_pos_coor.pack_id1 = 0x1;
	master.info->tx->radar_pos_coor.pack_id2 = 0x5;

	master.info->tx->radar_pos_coor.hero_x = judge.info->radar_info.hero_x;						// 英雄坐标X	0~1500
	master.info->tx->radar_pos_coor.hero_y = judge.info->radar_info.hero_y;						// 英雄坐标Y	0~2800
	master.info->tx->radar_pos_coor.engineer_x = judge.info->radar_info.engineer_x;		// 工程坐标X	0~1500
	master.info->tx->radar_pos_coor.engineer_y = judge.info->radar_info.engineer_y;		// 工程坐标Y	0~2800
	master.info->tx->radar_pos_coor.infantry3_x = judge.info->radar_info.infantry3_x;	// 步兵3坐标X	0~1500
	master.info->tx->radar_pos_coor.infantry3_y = judge.info->radar_info.infantry3_y;	// 步兵3坐标Y	0~2800
	master.info->tx->radar_pos_coor.infantry4_x = judge.info->radar_info.infantry4_x;	// 步兵4坐标X	0~1500
	master.info->tx->radar_pos_coor.infantry4_y = judge.info->radar_info.infantry4_y;	// 步兵4坐标Y	0~2800
	master.info->tx->radar_pos_coor.sentry_x = judge.info->radar_info.sentry_x;				// 哨兵坐标X	0~1500
	master.info->tx->radar_pos_coor.sentry_y = judge.info->radar_info.sentry_y;				// 哨兵坐标Y	0~2800
	
	memcpy(radar_raw_buf, (void*)&master.info->tx->radar_pos_coor, 16);

	if(radar_pos_coor_cnt % 2 == 0)	//	数据包ID，交替发送
		memcpy(radar_pos_coor_tx_buf, radar_raw_buf, 8);
	else
		memcpy(radar_pos_coor_tx_buf, &radar_raw_buf[8], 8);

	switch (can_num)
	{
		case 1:
			CAN_SendData(&hcan1, RADAR_POS_COOR_ID, radar_pos_coor_tx_buf);
			break;
		case 2:
			CAN_SendData(&hcan2, RADAR_POS_COOR_ID, radar_pos_coor_tx_buf);
			break;
		default:
			break;
	}
	memset(radar_pos_coor_tx_buf, 0, 8);
}

void shoot_data_send(uint8_t can_num)
{
	static uint8_t shoot_data_tx_buf[8];

	master.info->tx->bullet_speed = judge.info->shoot_data.bullet_speed;
	memcpy(shoot_data_tx_buf, (void*)&judge.info->shoot_data.bullet_speed, 4);
	
	switch (can_num)
	{
		case 1:
			CAN_SendData(&hcan1, SHOOT_DATA_ID, shoot_data_tx_buf);
			break;
		case 2:
			CAN_SendData(&hcan2, SHOOT_DATA_ID, shoot_data_tx_buf);
			break;
		default:
			break;
	}

	memset(shoot_data_tx_buf, 0, 8);
}
