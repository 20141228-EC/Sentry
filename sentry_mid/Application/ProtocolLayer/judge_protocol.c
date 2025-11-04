/* Includes ------------------------------------------------------------------*/
#include "judge_protocol.h"
#include "can_protocol.h"

#include "cap.h"

/* Private function prototypes -----------------------------------------------*/

void Determine_ID(void);
void bullet_speed_test(void);
void Judge_Update(judge_t *judge_sen, uint8_t *rxBuf);

/* Exported functions --------------------------------------------------------*/

void USART6_rxDataHandler(uint8_t *rxBuf)
{
	judge.update(&judge, rxBuf);
}


uint32_t shoot_statistics = 0;	//射击个数统计

float power_sum;
uint32_t power_num;
float power_ave;

double k1_num;
double k1_sum;

uint8_t k1_flag;
/**
	* @brief  裁判系统信息更新
  	* @param  uint8_t *rxBuf
  	* @retval judge_sen
	* @note   顺便发送信息到超电,下主控,UI
*/
void Judge_Update(judge_t *judge_sen, uint8_t *rxBuf)	//(已测试24.12.14)
{
	uint16_t frame_length;
	uint16_t cmd_id;
	
	judge_info_t *judge_info = judge_sen->info;

	memcpy(&judge_info->fream_header, rxBuf, LEN_FRAME_HEAD);//先取出帧头
	
	/* 帧首字节是否为0xA5 */
	if(rxBuf[SOF] == JUDGE_FRAME_HEADER) 
	{
		/* 帧头CRC8校验 */
		if(Verify_CRC8_Check_Sum(rxBuf, LEN_FRAME_HEAD) == true) 
		{
			/* 统计一帧的总数据长度，用于CRC16校验 */     // 长度两个字节
			frame_length = LEN_FRAME_HEAD + LEN_CMD_ID + judge_info->fream_header.data_length + LEN_FRAME_TAIL;
			
			/* 整包CRC16校验 */
			if(Verify_CRC16_Check_Sum(rxBuf, frame_length) == true)
			{
				cmd_id = (rxBuf[CMD_ID+1] << 8 | rxBuf[CMD_ID]);
				
				switch(cmd_id)
				{
					case ID_game_state:			// 0x0001   
						memcpy(&judge_info->game_status, (rxBuf+DATA_SEG), LEN_GAME_STATUS);
					
					#if GAME_START
						//比赛阶段
					if( judge.info->game_status.game_progress == 4)
					{
						Strategy.CarData->ce.game_start = 1;
					}
					else
					{
						Strategy.CarData->ce.game_start = 0;
					}
					
					if(judge.info->game_status.game_progress == 0 || judge.info->game_status.game_progress == 1)
					{
						Strategy.CarData->ce.game_perpare = 0;
					}
					else
					{
						Strategy.CarData->ce.game_perpare = 1;
					}
					#endif
					//比赛剩余时间
					Strategy.CarData->ce.game_time = judge.info->game_status.stage_remain_time;
					//发送上主控比赛状态
					game_status_send(2);
					break;
					
					case ID_game_result:		// 0x0002
						memcpy(&judge_info->game_result, (rxBuf+DATA_SEG), LEN_GAME_RESULT);
						break;
					
					case ID_game_robot_HP:	// 0x0003	
						memcpy(&judge_info->game_robot_HP, (rxBuf+DATA_SEG), LEN_GAME_ROBOT_HP);
						//决策获取敌我血量
						strategy_get_robot_HP(&Strategy,&judge);
						//发送上主控血量信息
						game_robot_HP_send(2);
						break;
					
					case ID_event_data: //0x0101
						
						memcpy(&judge_info->event_data, (rxBuf+DATA_SEG), LEN_EVENT_DATA);//(32位)
						Strategy.CarData->ce.points = judge_info->event_data.points;
						break;
					
					case ID_supply_projectile_action: //0x0102			//己方机器人补弹信息
						memcpy(&judge_info->supply_projectile_action, (rxBuf+DATA_SEG), LEN_SUPPLY_PROJECTILE_ACTION);
						break;
					
					case ID_referee_warning: //0x0104
						memcpy(&judge_info->referee_warning, (rxBuf+DATA_SEG), LEN_REFEREE_WARNING);
						break;
					
					case ID_dart_remaining_time: //0x0105
						memcpy(&judge_info->dart_remaining_time, (rxBuf+DATA_SEG), LEN_DART_REMAINING_TIME);
						break;
						
		/*能进*/	case ID_game_robot_state: //0x0201
						memcpy(&judge_info->game_robot_status, (rxBuf+DATA_SEG), LEN_GAME_ROBOT_STATUS);
						Determine_ID();         
		
						//决策传递敌方颜色信息
						Strategy.CarData->ce.enemy_color = !judge.info->my_color;
						
						//发送上主控机器人状态（颜色，发射热量限制，发射冷却速率）
						game_robot_status_send(2);
						break;
					
		/*能进*/	case ID_power_heat_data: //0x0202       //输出V,I,P,缓冲能量,步兵,英雄枪口热量
						memcpy(&judge_info->power_heat_data, (rxBuf+DATA_SEG), LEN_POWER_HEAT_DATA);
						cap.setdata(&cap, judge_info->power_heat_data.chassis_power_buffer, \
															judge_info->game_robot_status.chassis_power_limit, \
															judge_info->power_heat_data.chassis_volt, \
															judge_info->power_heat_data.chassis_current);
						power_heat_send(2);                 //将裁判系统读取的枪口热量发送给上主控
		
						power_sum += judge.info->power_heat_data.chassis_power;
						power_ave = power_sum/power_num;
						
		
						if(k1_flag == 1 && power_num<=1000)
						{
							power_num++;
							k1_sum +=(judge.info->power_heat_data.chassis_power
												-4-1.9923e-7*rm_motor[CHAS_LF].info->current*rm_motor[CHAS_LF].info->current
												-2.43243e-6 *rm_motor[CHAS_LF].info->current*rm_motor[CHAS_LF].info->speed)/
												(rm_motor[CHAS_LF].info->speed*rm_motor[CHAS_LF].info->speed);
							k1_num = k1_sum/power_num;
						}
						
						break;
					
	/*能进但无*/	case ID_game_robot_pos: //0x0203            //本机器人位置,枪口(即正面朝向)
						memcpy(&judge_info->game_robot_pos, (rxBuf+DATA_SEG), LEN_GAME_ROBOT_POS);
						robot_pos_send(2);
						break;
					
					case ID_buff_musk: //0x0204                 //增益与冷却倍率
						memcpy(&judge_info->buff, (rxBuf+DATA_SEG), LEN_BUFF_MASK); 
						break;
					
					case ID_aerial_robot_energy: //0x0205       //空中支援状态
						memcpy(&judge_info->aerial_robot_energy, (rxBuf+DATA_SEG), LEN_AERIAL_ROBOT_ENERGY);
						break;
					
/*装甲板存在离线*/	case ID_robot_hurt: //0x0206        //受击位置ID,受击类型
						memcpy(&judge_info->robot_hurt, (rxBuf+DATA_SEG), LEN_ROBOT_HURT);
						robot_hurt_send(2);	//发送上主控受击信息
						break;
					
	/*无测速模块*/	case ID_shoot_data: //0x0207        //弹丸类型,发射机构ID,射频,射速
						memcpy(&judge_info->shoot_data, (rxBuf+DATA_SEG), LEN_SHOOT_DATA);
						shoot_data_send(2);	//发送上主控射击信息
						shoot_statistics++;
						break;
					
					case ID_projectile_allowance: //0x0208      //17,42允许发弹量,剩余金币
						memcpy(&judge_info->bullet_remaining, (rxBuf+DATA_SEG), LEN_BULLET_REMAINING);
					//获取剩余单量
						Strategy.CarData->sf.bullet = judge.info->bullet_remaining.bullet_remaining_num_17mm;
					//剩余金币数量
						Strategy.CarData->sf.bullet = judge.info->bullet_remaining.coin_remaining_num;

						
						break;
					
					case ID_rfid_status: //0x0209               //增益点是否进入
						memcpy(&judge_info->rfid_status, (rxBuf+DATA_SEG), LEN_RFID_STATUS);
						break;
						
					case ID_dart_client_directive: //0x020A     //飞镖发射站状态
						memcpy(&judge_info->dart_client,(rxBuf+DATA_SEG),LEN_DART_CLIENT_DIRECTIVE);
						break;
					
					case ID_ground_robot_position: //0x020B     //己方地面单位(除哨兵)位置
						memcpy(&judge_info->ground_robot_position,(rxBuf+DATA_SEG),LEN_GROUND_ROBOT_POSITION);
						break;
					
					case ID_radar_mark_data: //0x020C           //对方地面单位标记进度,<100时为0
						memcpy(&judge_info->radar_mark_data,(rxBuf+DATA_SEG),LEN_RADAR_MARK_DATA);
						break;
				}
			
	      		judge_sen->info->offline_cnt = 0;
				
				/* 帧尾CRC16下一字节是否为0xA5 */
				if(rxBuf[frame_length] == JUDGE_FRAME_HEADER)
				{
					/* 如果一个数据包出现了多帧数据就再次读取 */
					Judge_Update( judge_sen, &rxBuf[frame_length] );
				}
			}
		}
	}
}

void Judge_Send()
{
	
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
		
		judge.info->ids.client_hero 	   = 0x0101;
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
