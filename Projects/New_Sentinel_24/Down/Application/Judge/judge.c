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
extern vision_t               vision_structure;
extern cap_receive_data_t cap_receive_data;

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
	.remain_bullet = 150,
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

	/*发送	2Hz*/
	static uint16_t send_cnt = 0;
	// if(send_cnt++ == 999)
	// {
	// 	//Judge_MapSend();
	// 	Judge_DecisionSend();
	// 	// judge.base_info->test++;
        
	// 	send_cnt = 0;
	// }
	if(send_cnt++ == 245)
	{
		// Judge_MapSend();
		// Judge_DecisionSend();
		Judge_DecisionSend2Judge();
	}
	if(send_cnt == 499)
	{
		// Judge_MapSend();
		// Judge_DecisionSend();
		send_cnt = 0;
	}
	

	static uint16_t one_second_cnt = 0;
	static uint8_t half_flag = 1;
	if(judge.base_info->game_progress != 4)
	{
		one_second_cnt = 0;									//比赛开始前不进行计时
		vision_structure.tx_pack->TxData.game_time = 0;		//赛前初始化为0
		half_flag = 1;
	}
	else
	{
		/*比赛阶段*/
		if(one_second_cnt++ == 999)
		{
			one_second_cnt = 0;
			// if(half_flag == 1)
			// {
				vision_structure.tx_pack->TxData.game_time+=2;
			// }
			// half_flag*=-1;
		}
	}
	if(vision_structure.tx_pack->TxData.game_time >= 420)	//七分钟
	{
		vision_structure.tx_pack->TxData.game_time = 420;
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
			
			// if(judge.base_info->game_progress == 4)
			// {
			// 	judge.base_info->remain_time = judge.data->game_status.stage_remain_time;	//比赛当阶段（阶段由progress决定）剩余时间
			// }
			// else
			// {
			// 	judge.base_info->remain_time = 420;
			// }
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
				/*己方为红色*/
				judge.base_info->car_color = 0;
			}
			else if(judge.data->game_robot_status.robot_id == 107)
			{
				/*己方为蓝色*/
				judge.base_info->car_color = 1;
			}
			judge.base_info->chassis_power_limit = judge.data->game_robot_status.chassis_power_limit;
			judge.base_info->last_HP             = judge.base_info->remain_HP;
			judge.base_info->remain_HP           = judge.data->game_robot_status.current_HP;
			
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
			
			break;
		case ID_game_robot_pos:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->game_robot_pos, rxBuf, LEN_game_robot_pos);
			/*数据处理-----------------------------------------------------------------------------------------*/
			judge.base_info->uwb_position_x = (uint16_t)(judge.data->game_robot_pos.x * 1000);//m转mm
			judge.base_info->uwb_position_y = (uint16_t)(judge.data->game_robot_pos.y * 1000);

			break;
		case ID_robot_hurt:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->ext_robot_hurt, rxBuf, LEN_robot_hurt);
			/*数据处理-----------------------------------------------------------------------------------------*/
			// judge.base_info->armor_id  = judge.data->ext_robot_hurt.armor_id;
            // judge.base_info->hurt_type = judge.data->ext_robot_hurt.hurt_type;
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
				/*红色ally*/
				judge.base_info->ally_hero_HP 		= judge.data->game_robot_HP.red_1_robot_HP;
				judge.base_info->ally_engineer_HP 	= judge.data->game_robot_HP.red_2_robot_HP;
				judge.base_info->ally_infantry3_HP 	= judge.data->game_robot_HP.red_3_robot_HP;
				judge.base_info->ally_infantry4_HP 	= judge.data->game_robot_HP.red_4_robot_HP;
				judge.base_info->ally_infantry5_HP 	= judge.data->game_robot_HP.red_5_robot_HP;
				judge.base_info->ally_sentry_HP 	= judge.data->game_robot_HP.red_7_robot_HP;

				judge.base_info->ally_outpost_HP 	= judge.data->game_robot_HP.red_outpost_HP;
				judge.base_info->ally_base_HP    	= judge.data->game_robot_HP.red_base_HP;

				
				/*蓝色enemy*/
				judge.base_info->enemy_hero_HP 		= judge.data->game_robot_HP.blue_1_robot_HP;
				judge.base_info->enemy_engineer_HP 	= judge.data->game_robot_HP.blue_2_robot_HP;
				judge.base_info->enemy_infantry3_HP = judge.data->game_robot_HP.blue_3_robot_HP;
				judge.base_info->enemy_infantry4_HP = judge.data->game_robot_HP.blue_4_robot_HP;
				judge.base_info->enemy_infantry5_HP = judge.data->game_robot_HP.blue_5_robot_HP;
				judge.base_info->enmey_sentry_HP 	= judge.data->game_robot_HP.blue_7_robot_HP;

				judge.base_info->enemy_outpost_HP 	= judge.data->game_robot_HP.blue_outpost_HP;
				judge.base_info->enemy_base_HP     	= judge.data->game_robot_HP.blue_base_HP;

			}
			else if(judge.data->game_robot_status.robot_id == 107)
			{
				/*蓝色ally*/
				judge.base_info->ally_hero_HP 		= judge.data->game_robot_HP.blue_1_robot_HP;
				judge.base_info->ally_engineer_HP 	= judge.data->game_robot_HP.blue_2_robot_HP;
				judge.base_info->ally_infantry3_HP 	= judge.data->game_robot_HP.blue_3_robot_HP;
				judge.base_info->ally_infantry4_HP 	= judge.data->game_robot_HP.blue_4_robot_HP;
				judge.base_info->ally_infantry5_HP 	= judge.data->game_robot_HP.blue_5_robot_HP;
				judge.base_info->ally_sentry_HP 	= judge.data->game_robot_HP.blue_7_robot_HP;

				judge.base_info->ally_outpost_HP 	= judge.data->game_robot_HP.blue_outpost_HP;
				judge.base_info->ally_base_HP     	= judge.data->game_robot_HP.blue_base_HP;


				/*红色enemy*/
				judge.base_info->enemy_hero_HP 		= judge.data->game_robot_HP.red_1_robot_HP;
				judge.base_info->enemy_engineer_HP 	= judge.data->game_robot_HP.red_2_robot_HP;
				judge.base_info->enemy_infantry3_HP = judge.data->game_robot_HP.red_3_robot_HP;
				judge.base_info->enemy_infantry4_HP = judge.data->game_robot_HP.red_4_robot_HP;
				judge.base_info->enemy_infantry5_HP = judge.data->game_robot_HP.red_5_robot_HP;
				judge.base_info->enmey_sentry_HP 	= judge.data->game_robot_HP.red_7_robot_HP;

				judge.base_info->enemy_outpost_HP 	= judge.data->game_robot_HP.red_outpost_HP;
				judge.base_info->enemy_base_HP    	= judge.data->game_robot_HP.red_base_HP;

			}
			break;
		case ID_bullet_remaining:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->bullet_remaining, rxBuf, LEN_bullet_remaining);
			/*数据处理-----------------------------------------------------------------------------------------*/
			judge.base_info->remain_bullet       = judge.data->bullet_remaining.bullet_remaining_num_17mm;
			break;
		
		case ID_ground_robot_position:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->robot_position, rxBuf, LEN_ground_robot_position);

			break;
		/*哨兵自主决策信息同步*/
		case ID_sentry_decision_info:
			/*数据接收-----------------------------------------------------------------------------------------*/
			memcpy(&judge.data->sentry_info, rxBuf, LEN_sentry_decision_info);
			// judge.base_info->test = ID_sentry_decision_info;
			/*数据处理-----------------------------------------------------------------------------------------*/
			break;
		
		default:
			break;
	}

	// if(judge.base_info->hurt_type == ammo_HURT/*0*/)
	// {
	// 	judge.base_info->hurt_type = 6;
	// 	judge.data->ext_robot_hurt.hurt_type = 6;
	// 	judge.base_info->last_armor_id = judge.base_info->armor_id;
	// }
	// else
	// {
	// 	// static uint8_t cnt= 0;
	// 	// if(cnt++ == 400)
	// 	// {
	// 	judge.base_info->armor_id = 6;
	// 	// cnt = 0;
	// 	// }
	// }

//            if(judge.base_info->hurt_type == ammo_HURT/*0*/)
//			{
////				judge.base_info->armor_id      = judge.data->ext_robot_hurt.armor_id;
//				judge.base_info->hurt_type     = 6;
//                judge.data->ext_robot_hurt.hurt_type = 6;
//			}
//			else
//			{
//				judge.base_info->armor_id      = 6;
//			}

}



void judge_recive(uint8_t *rxBuf)
{
	uint16_t frame_length;
	if( rxBuf == NULL )
	{
		return;
	}
	/* 读取帧头信息 */
	drv_judge_info.frame_header->SOF = rxBuf[0];
	/* 帧头校验 */
	if(drv_judge_info.frame_header->SOF == 0xA5)
	{
		/* 读取数据长度 */
		memcpy(&drv_judge_info.frame_header->data_length, rxBuf + 1, 4);
		/* 帧头crc校验 */
		if(Verify_CRC8_Check_Sum(rxBuf, 5) == 1)
		{
			/* 计算整包帧长度 */
			frame_length = 5 + 2 + drv_judge_info.frame_header->data_length + 2;
			/* 整包crc校验 */
			if(Verify_CRC16_Check_Sum(rxBuf, frame_length) == 1)
			{
				/* 读取命令码 */
				memcpy(&drv_judge_info.cmd_id, rxBuf + 5, 2);
				/* 数据获取与处理 */
				judge_update(drv_judge_info.cmd_id, rxBuf + 7);
			}
			/* 读取帧尾 */
			memcpy(&drv_judge_info.frame_tail, rxBuf + 5 + 2 + drv_judge_info.frame_header->data_length, 2);
			
			/* 如果一个数据包出现了多帧数据就再次读取 */
			//@note	frame_length 计算在crc8校验通过之后，未赋值时为0。若写在外面，只要crc8不过，则死循环（rxbuf[0]===A5）
			if(rxBuf[frame_length] == 0xA5)
			{
				judge_recive( &rxBuf[frame_length] );
			}
		}
	}
}


/*---------发送部分-----------------*/
extern UART_HandleTypeDef huart6;//使用串口6


Judge_Map_tx_packet_t      Judge_Map_tx_structure;
Judge_Decision_tx_packet_t Judge_Decision_tx_structure;
Judge_Decision2J_tx_packet_t Judge_Decision2J_tx_structure;

uint8_t                Judge_txBuf[200];



void judge_Init()
{
	judge.base_info->armor_id = 6;
}



void Judge_Map_send(void)
{
	/*数据发送*/
	memcpy(Judge_txBuf, &Judge_Map_tx_structure, sizeof(Judge_Map_tx_structure));
	
	/*增加CRC校验位*/
	Append_CRC8_Check_Sum(Judge_txBuf, 5);
	Append_CRC16_Check_Sum(Judge_txBuf, LEN_map_sentry_data + 9);
	
	HAL_UART_Transmit_DMA(&huart6,Judge_txBuf,sizeof(Judge_Map_tx_packet_t)); 
}


/**
 * @brief  发送哨兵自主决策数据（包含云台手指令）
*/
void Judge_Decision_send(void)
{
	/*数据发送*/	
	memcpy(Judge_txBuf, &Judge_Decision_tx_structure, sizeof(Judge_Decision_tx_structure));
	
	/*增加CRC校验位*/
	Append_CRC8_Check_Sum(Judge_txBuf, 5);
	Append_CRC16_Check_Sum(Judge_txBuf, LEN_sentry_decision_cmd + 9);
	
	if(Verify_CRC8_Check_Sum(Judge_txBuf, 5) == 1)
	{
		if(Verify_CRC16_Check_Sum(Judge_txBuf, LEN_sentry_decision_cmd + 9) == 1)
		{

			if(HAL_UART_Transmit_DMA(&huart6,Judge_txBuf,sizeof(Judge_Decision_tx_packet_t)) == HAL_OK)
			{
				judge.base_info->test = 66;
			}
			else
			{
				judge.base_info->test = 44;
			}
		}
	}	
}

void Judge_Decision_send2judge(void)
{
	/*数据发送*/	
	memcpy(Judge_txBuf, &Judge_Decision2J_tx_structure, sizeof(Judge_Decision2J_tx_structure));
	
	/*增加CRC校验位*/
	Append_CRC8_Check_Sum(Judge_txBuf, 5);
	Append_CRC16_Check_Sum(Judge_txBuf, LEN_aerial_data + 9);
	
	if(Verify_CRC8_Check_Sum(Judge_txBuf, 5) == 1)
	{
		if(Verify_CRC16_Check_Sum(Judge_txBuf, LEN_aerial_data + 9) == 1)
		{

			if(HAL_UART_Transmit_DMA(&huart6,Judge_txBuf,sizeof(Judge_Decision2J_tx_packet_t)) == HAL_OK)
			{
				judge.base_info->test = 77;
			}
			else
			{
				judge.base_info->test = 33;
			}
		}
	}	
}


extern vision_t    vision_structure;
uint8_t     test_intention = 1;
uint16_t    start_position_x = 140;
uint16_t    start_position_y = 0;
int8_t      dx = 0;
int8_t      dy = 125;



void Judge_MapSend(void)
{
	Judge_Map_tx_structure.FrameHeader.sof         = 0xA5;
	Judge_Map_tx_structure.FrameHeader.data_length = LEN_map_sentry_data;
	Judge_Map_tx_structure.FrameHeader.seq         = 0;
	Judge_Map_tx_structure.FrameHeader.cmd_id      = ID_map_sentry_data;
	
	Judge_Map_tx_structure.TxData.intention        = test_intention;
	Judge_Map_tx_structure.TxData.start_position_x = start_position_x;
	Judge_Map_tx_structure.TxData.start_position_y = start_position_y;
	Judge_Map_tx_structure.TxData.delta_x[0]       = dx;
	Judge_Map_tx_structure.TxData.delta_y[0]       = dy;
	
	Judge_Map_tx_structure.TxData.sender_id		   = judge.data->game_robot_status.robot_id;

//	if(navigation_structure.tx_pack->TxData.navi_enable == 0)
//	{
//		Judge_Map_tx_structure.TxData.delta_x[1]   = 50;
//	}
//	else
//	{
//		Judge_Map_tx_structure.TxData.delta_x[1]   = 0;
//	}
	
	if(must_patrol_enable == 1)
	{
		dy = 50;
	}
	else
	{
		dy = 125;
	}

	Judge_Map_send();
}



void Judge_DecisionSend(void)
{
	Judge_Decision_tx_structure.FrameHeader.sof         = 0xA5;
	Judge_Decision_tx_structure.FrameHeader.data_length = LEN_sentry_decision_cmd;
	Judge_Decision_tx_structure.FrameHeader.seq         = 0;
	Judge_Decision_tx_structure.FrameHeader.cmd_id      = ID_sentry_decision_cmd;

	//测试用，记得注
	// Judge_Decision_tx_structure.TxData.sentry_cmd.bit.if_revive_await = 1;
	// // Judge_Decision_tx_structure.TxData.sentry_cmd.bit.if_revive_immediate = navigation_structure.rx_pack->RxData.if_revive_immediate;
	// Judge_Decision_tx_structure.TxData.sentry_cmd.bit.add_bullet_num = navigation_structure.rx_pack->RxData.add_bullet_num;
	// Judge_Decision_tx_structure.TxData.sentry_cmd.bit.remote_add_bullet_times = navigation_structure.rx_pack->RxData.remote_add_bullet_times;
	// Judge_Decision_tx_structure.TxData.sentry_cmd.bit.remote_add_HP_times = navigation_structure.rx_pack->RxData.remote_add_HP_times;

	// Judge_Decision_tx_structure.TxData.sentry_cmd.bit.if_revive_await = navigation_structure.rx_pack->RxData.if_revive_await;
	// Judge_Decision_tx_structure.TxData.sentry_cmd.bit.if_revive_immediate = navigation_structure.rx_pack->RxData.if_revive_immediate;
	// Judge_Decision_tx_structure.TxData.sentry_cmd.bit.add_bullet_num = navigation_structure.rx_pack->RxData.add_bullet_num;
	// Judge_Decision_tx_structure.TxData.sentry_cmd.bit.remote_add_bullet_times = navigation_structure.rx_pack->RxData.remote_add_bullet_times;
	// Judge_Decision_tx_structure.TxData.sentry_cmd.bit.remote_add_HP_times = navigation_structure.rx_pack->RxData.remote_add_HP_times;

	Judge_Decision_send();
}


void Judge_DecisionSend2Judge(void)
{
	/* 写入数据 */
	Judge_Decision2J_tx_structure.FrameHeader.sof         = 0xA5;
	Judge_Decision2J_tx_structure.FrameHeader.data_length = LEN_aerial_data;
	Judge_Decision2J_tx_structure.FrameHeader.seq         = 0;
	Judge_Decision2J_tx_structure.FrameHeader.cmd_id      = ID_interactive_header_data;

	Judge_Decision2J_tx_structure.TxData.cmd_id = ID_sentry_decision_cmd;					//子内容ID
	Judge_Decision2J_tx_structure.TxData.send_id = 107;//judge.data->game_robot_status.robot_id;	//发送者ID
	Judge_Decision2J_tx_structure.TxData.receive_id = ID_judge_server;						//接收者ID

	// Judge_Decision2J_tx_structure.TxData.if_revive_await = 0;
	// Judge_Decision2J_tx_structure.TxData.if_revive_immediate = 0;
	// Judge_Decision2J_tx_structure.TxData.add_bullet_num = 0;
	// Judge_Decision2J_tx_structure.TxData.remote_add_bullet_times = 0;
	// Judge_Decision2J_tx_structure.TxData.remote_add_HP_times = 0;

	/* 发送数据 */
	Judge_Decision_send2judge();
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