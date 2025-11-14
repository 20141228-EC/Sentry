#include "navigation.h"
#include "vision.h"
#include "bmi.h"
#include "can_protocol.h"
#include "rp_shoot.h"
#include "rp_gimbal.h"
#include "judge.h"
#include "Car.h"


/*使用串口1*/
extern UART_HandleTypeDef huart1;//使用串口1

extern judge_t          judge;
extern Master_Head_t    Master_Head_structure;
extern bmi_t            bmi_structure;
extern car_t            car_structure;
extern vision_t         vision_structure;
extern Judge_Decision_tx_packet_t Judge_Decision_tx_structure;
navigation_rx_packet_t  navigation_rx_pack;
navigation_tx_packet_t  navigation_tx_pack;
navigation_t            navigation_structure;


/*发送Buffer*/
uint8_t                 navigation_txBuf[200]; 


/**
  * @Name    Vision_Init
  * @brief   视觉部分结构体初始化
  * @param   None
  * @retval  
  * @author  HWX
  * @Date    2022-10-21
**/
void Navigation_Init(void)
{
	/*结构体初始化*/
	navigation_structure.rx_pack = &navigation_rx_pack;
	navigation_structure.tx_pack = &navigation_tx_pack;
	
	/*数据初始化*/
	navigation_structure.tx_pack->FrameHeader.sof   = VISION_SEND_ID;
	navigation_structure.tx_pack->FrameHeader.cmd_id= CMD_PATROL;
//	navigation_structure.tx_pack->TxData.navi_enable= 1;//默认开启
//	navigation_structure.tx_pack->TxData.omni_enable= 0;//默认关闭
	
	
	
	/*工作状态初始化*/
	navigation_structure.state.offline_max_cnt = VISION_OFFLINE_MAX_CNT;
	navigation_structure.state.offline_cnt     = VISION_OFFLINE_MAX_CNT;
	navigation_structure.state.work_state      = VISION_OFFLINE;
	
}


/**
  * @Name    Vision_SendData
  * @brief   与小电脑通讯，CRC校验数据肯定要改
  * @param   None                              
  * @retval
  * @author  HWX
  * @Date    2022-10-21
**/
bool Navigation_SendData(void)
{
	/*判断是否掉线*/
	if(navigation_structure.state.offline_cnt++ >= navigation_structure.state.offline_max_cnt)
	{
		navigation_structure.state.offline_cnt--;
		navigation_structure.state.work_state = VISION_OFFLINE;
	}
	else
	{
		navigation_structure.state.work_state = VISION_ONLINE;
	}
	
	/*数据发送*/	
	memcpy(navigation_txBuf, &navigation_tx_pack, sizeof(navigation_tx_pack));
	
	/*增加CRC校验位*/
	Append_CRC8_Check_Sum(navigation_txBuf, LEN_NAVIGATION_FRAME_HEADER);
	Append_CRC16_Check_Sum(navigation_txBuf, LEN_NAVIGATION_TX_PACKET);
	
	
	if(HAL_UART_Transmit_DMA(&huart1,navigation_txBuf,sizeof(navigation_tx_pack)) == HAL_OK)
	{
		//memset(&navigation_tx_pack,0,sizeof(navigation_tx_pack));
		return true;
	}
	else
	{
		//memset(&navigation_tx_pack,0,sizeof(navigation_tx_pack));
		return false;
	}
}


/**
  * @Name    Vision_GetData
  * @brief   接收数据，一样的CRC校验的参数可能要改
  * @param   None
  * @retval  
  * @author  HWX
  * @Date    2022-10-21
**/
bool Navigation_GetData(uint8_t *rxBuf)
{

	
	if(rxBuf[0] == 0xA5)
	{
		if(Verify_CRC8_Check_Sum(rxBuf, LEN_NAVIGATION_FRAME_HEADER) == true)
		{
			if(Verify_CRC16_Check_Sum(rxBuf,LEN_NAVIGATION_RX_PACKET) == true)
			{
				memcpy(&navigation_rx_pack, rxBuf, LEN_NAVIGATION_RX_PACKET);//此处应该自动完成了全部数据的接收

				/*异常数据清除*/
				if(navigation_rx_pack.RxData.chassis_front >= 3200)
				{
					navigation_rx_pack.RxData.chassis_front = 3200;
				}
				else if(navigation_rx_pack.RxData.chassis_front <= -3200)
				{
					navigation_rx_pack.RxData.chassis_front = -3200; 
				}
				
				if(navigation_rx_pack.RxData.chassis_right >= 3200)
				{
					navigation_rx_pack.RxData.chassis_right = 3200; 
				}
				else if(navigation_rx_pack.RxData.chassis_right <= -3200)
				{
					navigation_rx_pack.RxData.chassis_right = -3200; 
				}
                
				/*掉线计数器清零*/
				navigation_structure.state.offline_cnt = 0;
				return true;
			}
		}
	}
	return false;
}

/**
  * @Name    Vision_Update
  * @brief   解析数据
  * @param   None
  * @retval
  * @author  HWX
  * @Date    2022-10-21
**/
void Navigation_Update(void)
{
	//数据接收已经在校验完成时便结束了
		
}

/**
  * @Name    USART1_rxDataHandler
  * @brief   串口1接收中断回调函数
  * @param   None
  * @retval
  * @author  HWX
  * @Date    2022-10-21
**/
void USART1_rxDataHandler(uint8_t *rxBuf)
{	
	Navigation_GetData(rxBuf);
}


void Navigation_Heart()
{
	if(navigation_structure.state.offline_cnt++ >= navigation_structure.state.offline_max_cnt)
	{
		navigation_structure.state.offline_cnt = navigation_structure.state.offline_max_cnt;
		navigation_structure.state.work_state = VISION_OFFLINE;
	}
	else
	{
		navigation_structure.state.work_state = VISION_ONLINE;
	}
}

/**
  * @Name    vision_task
  * @brief  
  * @param   None
  * @retval
  * @author  HWX
  * @Date    2022-10-24
**/
#define X_LIMIIT 14.0f

void Navigation_task(void)
{	
    navigation_structure.tx_pack->FrameHeader.sof   = VISION_SEND_ID;
	
	
	/*发送数据处理*/
		if(car_structure.mode == aim_CAR)
		{
			navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_CHECK;
			
		}
		else
		{
			if(judge.base_info->robot_commond == 'M')
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_MAP_FORWARD;		//转发小地图标记
			}
			else if(judge.base_info->robot_commond == 'N')
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_BULLET_BUY_AT_HP;	//在补血点购买允许发弹量
			}
			else if(judge.base_info->robot_commond == 'R')//不考虑，弹舱不适合补弹
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_BULLET_FILL;		//在补给站补给弹丸 --回到补给站
			}
			else if(judge.base_info->robot_commond == 'V')
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_REVIVE_AWAIT;		//确认复活
			}
			else if(judge.base_info->robot_commond == 'B')
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_BULLET_BUY_REMOTE;	//远程兑换发弹量
			}
			else if(judge.base_info->robot_commond == 'H')
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_HP_BUY_REMOTE;		//远程兑换血量
			}
			else if(judge.base_info->robot_commond == 'O')
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_REVIVE_IMMEDIATE;	//远程兑换立即复活
			}


			/* if(judge.base_info->robot_commond == 'Y')//Y
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_A;
			}
			else if(judge.base_info->robot_commond == 'K')//K
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_B;
			}
			else if(judge.base_info->robot_commond == 'I')//I
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_D;
			}
			else if(judge.base_info->robot_commond == 'V')//V
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_E;
			}
			else if(judge.base_info->robot_commond == 'H')//H
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_STOP;
			}
			else if(judge.base_info->robot_commond == 'O')//O
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_G;
			}
			else if(judge.base_info->robot_commond == 'L')//L
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_H;
			}
			else if(judge.base_info->robot_commond == 'T')//T
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_R3;
			}
			else if(judge.base_info->robot_commond == 'G')//G
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_J;
			}
			else if(judge.base_info->robot_commond == 'F')//F
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_K;
			}
			else if(judge.base_info->robot_commond == 'Q')//Q
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_OUTPOS;
			}
			else if(judge.base_info->robot_commond == 'N')//N
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_R4;
			}
			else if(judge.base_info->robot_commond == 'Z')//Z
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_GOGOGO;
				// navigation_structure.tx_pack->TxData.target_x = judge.data->ext_robot_command.target_x;
				// navigation_structure.tx_pack->TxData.target_y = judge.data->ext_robot_command.target_y;
			}
			else if(judge.base_info->robot_commond == 'J')//J
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_FUCK_OUTPOS;
			}
			else
			{
				navigation_structure.tx_pack->FrameHeader.cmd_id   = CMD_STOP;
			} */
			
		}

		if(judge.base_info->robot_commond == 'R')//R
		{
			if(judge.data->ext_robot_command.target_x >= X_LIMIIT)
			{
				//navigation_structure.tx_pack->TxData.navi_enable = 1;
			}	
			else
			{
				//navigation_structure.tx_pack->TxData.navi_enable = 0;
			}
			
		}
		

	
	/*填写发送包，转发裁判系统数据给决策*/

	// navigation_structure.tx_pack->TxData.game_strategy		= GAME_STRATEGY;	//赋值放在task内的函数

	navigation_structure.tx_pack->TxData.game_progress 		= (judge.base_info->game_progress >= 4) ? 1:0;
	//navigation_structure.tx_pack->TxData.game_progress 		= 1;

	navigation_structure.tx_pack->TxData.enemy_color    	= judge.base_info->car_color;
	navigation_structure.tx_pack->TxData.if_find_enemy 		= (uint8_t)(vision_structure.rx_pack->RxData.left_aim_data.flag.bit.is_find_target)\
                                                       		 +(uint8_t)( vision_structure.rx_pack->RxData.right_aim_data.flag.bit.is_find_target);
    
    navigation_structure.tx_pack->TxData.big_yaw   			= bmi_structure.yaw_angle;
	navigation_structure.tx_pack->TxData.remain_bullet 		= judge.base_info->remain_bullet;

	//己方血量
	navigation_structure.tx_pack->TxData.hero_HP 			= judge.base_info->ally_hero_HP;
	navigation_structure.tx_pack->TxData.engineer_HP 		= judge.base_info->ally_engineer_HP;
	navigation_structure.tx_pack->TxData.infantry3_HP 		= judge.base_info->ally_infantry3_HP;
	navigation_structure.tx_pack->TxData.infantry4_HP 		= judge.base_info->ally_infantry4_HP;
	navigation_structure.tx_pack->TxData.infantry5_HP 		= judge.base_info->ally_infantry5_HP;

	navigation_structure.tx_pack->TxData.sentry_HP 			= judge.base_info->remain_HP;
	navigation_structure.tx_pack->TxData.outpost_HP 		= judge.base_info->ally_outpost_HP;
    navigation_structure.tx_pack->TxData.base_HP 			= judge.base_info->ally_base_HP;

	//敌方血量
    navigation_structure.tx_pack->TxData.enemy_hero_HP 		= judge.base_info->enemy_hero_HP;
	navigation_structure.tx_pack->TxData.enemy_engineer_HP 	= judge.base_info->enemy_engineer_HP;
	navigation_structure.tx_pack->TxData.enemy_infantry3_HP = judge.base_info->enemy_infantry3_HP;
	navigation_structure.tx_pack->TxData.enemy_infantry4_HP = judge.base_info->enemy_infantry4_HP;
	navigation_structure.tx_pack->TxData.enemy_infantry5_HP = judge.base_info->enemy_infantry5_HP;

	navigation_structure.tx_pack->TxData.enemy_sentry_HP 	= judge.base_info->enmey_sentry_HP;
	navigation_structure.tx_pack->TxData.enemy_outpost_HP 	= judge.base_info->enemy_outpost_HP;
	navigation_structure.tx_pack->TxData.enemy_base_HP 		= judge.base_info->enemy_base_HP;

	//EnemyData_t

	/*left*/
	navigation_structure.tx_pack->TxData.left_enemys_data.target_enemy_num = vision_structure.rx_pack->RxData.left_aim_data.autoaim_enemy_data.target_enemy_num;
	navigation_structure.tx_pack->TxData.left_enemys_data.target_enemy_position.x = vision_structure.rx_pack->RxData.left_aim_data.autoaim_enemy_data.target_enemy_position.x;
	navigation_structure.tx_pack->TxData.left_enemys_data.target_enemy_position.y = vision_structure.rx_pack->RxData.left_aim_data.autoaim_enemy_data.target_enemy_position.y;
	navigation_structure.tx_pack->TxData.left_enemys_data.target_enemy_position.z = vision_structure.rx_pack->RxData.left_aim_data.autoaim_enemy_data.target_enemy_position.z;

	for(int i = 0; i < ENEMY_NUM_FQ; i++)
	{
		navigation_structure.tx_pack->TxData.left_enemys_data.enemys_positions[i].x = vision_structure.rx_pack->RxData.left_aim_data.autoaim_enemy_data.enemys_positions[i].x;
		navigation_structure.tx_pack->TxData.left_enemys_data.enemys_positions[i].y = vision_structure.rx_pack->RxData.left_aim_data.autoaim_enemy_data.enemys_positions[i].y;
		navigation_structure.tx_pack->TxData.left_enemys_data.enemys_positions[i].z = vision_structure.rx_pack->RxData.left_aim_data.autoaim_enemy_data.enemys_positions[i].z;
	}
	
	/*right*/
	navigation_structure.tx_pack->TxData.right_enemys_data.target_enemy_num = vision_structure.rx_pack->RxData.right_aim_data.autoaim_enemy_data.target_enemy_num;
	navigation_structure.tx_pack->TxData.right_enemys_data.target_enemy_position.x = vision_structure.rx_pack->RxData.right_aim_data.autoaim_enemy_data.target_enemy_position.x;
	navigation_structure.tx_pack->TxData.right_enemys_data.target_enemy_position.y = vision_structure.rx_pack->RxData.right_aim_data.autoaim_enemy_data.target_enemy_position.y;
	navigation_structure.tx_pack->TxData.right_enemys_data.target_enemy_position.z = vision_structure.rx_pack->RxData.right_aim_data.autoaim_enemy_data.target_enemy_position.z;

	for(int i = 0; i < ENEMY_NUM_FQ; i++)
	{
		navigation_structure.tx_pack->TxData.right_enemys_data.enemys_positions[i].x = vision_structure.rx_pack->RxData.right_aim_data.autoaim_enemy_data.enemys_positions[i].x;
		navigation_structure.tx_pack->TxData.right_enemys_data.enemys_positions[i].y = vision_structure.rx_pack->RxData.right_aim_data.autoaim_enemy_data.enemys_positions[i].y;
		navigation_structure.tx_pack->TxData.right_enemys_data.enemys_positions[i].z = vision_structure.rx_pack->RxData.right_aim_data.autoaim_enemy_data.enemys_positions[i].z;
	}

	//CoordInfo_t
	/*从裁判系统读取己方机器人位置*/
	navigation_structure.tx_pack->TxData.allies_coord[0].x = (uint8_t)judge.data->robot_position.hero_x;
	navigation_structure.tx_pack->TxData.allies_coord[0].y = (uint8_t)judge.data->robot_position.hero_y;

	navigation_structure.tx_pack->TxData.allies_coord[1].x = (uint8_t)judge.data->robot_position.engineer_x;
	navigation_structure.tx_pack->TxData.allies_coord[1].y = (uint8_t)judge.data->robot_position.engineer_y;

	navigation_structure.tx_pack->TxData.allies_coord[2].x = (uint8_t)judge.data->robot_position.standard_3_x;
	navigation_structure.tx_pack->TxData.allies_coord[2].y = (uint8_t)judge.data->robot_position.standard_3_y;

	navigation_structure.tx_pack->TxData.allies_coord[3].x = (uint8_t)judge.data->robot_position.standard_4_x;
	navigation_structure.tx_pack->TxData.allies_coord[3].y = (uint8_t)judge.data->robot_position.standard_4_y;

	navigation_structure.tx_pack->TxData.allies_coord[4].x = (uint8_t)judge.data->robot_position.standard_5_x;
	navigation_structure.tx_pack->TxData.allies_coord[4].y = (uint8_t)judge.data->robot_position.standard_5_y;

	/*雷达*/

	// navigation_structure.tx_pack->TxData.enemys_data[0].num = vision_structure.rx_pack->RxData.enemy1_num;


	// navigation_structure.tx_pack->TxData.armor3_size  = BIGGG_ARMOR;
	// navigation_structure.tx_pack->TxData.armor4_size  = BIGGG_ARMOR;
	// navigation_structure.tx_pack->TxData.armor5_size  = BIGGG_ARMOR;
	//navigation_structure.tx_pack->TxData.nuc_flag1     = vision_structure.rx_pack->RxData.nuc_flag1;
	/* 比赛5秒倒计时阶段 */
//	if (judge.data->game_status.game_progress == 3)
//	{
//		/* 红方 */
//		if (judge.base_info->car_color == 0)
//		{

//			if (judge.data->game_robot_HP.blue_3_robot_HP == BALANCE_HP)
//			{
//				navigation_structure.tx_pack->TxData.ammor_size_3 = BIGGG_ARMOR;
//			}
//			else
//			{
//				navigation_structure.tx_pack->TxData.ammor_size_3 = SMALL_ARMOR;
//			}
//			
//			if (judge.data->game_robot_HP.blue_4_robot_HP == BALANCE_HP)
//			{
//				navigation_structure.tx_pack->TxData.ammor_size_4 = BIGGG_ARMOR;
//			}
//			else
//			{
//				navigation_structure.tx_pack->TxData.ammor_size_4 = SMALL_ARMOR;
//			}
//			
//			if (judge.data->game_robot_HP.blue_5_robot_HP == BALANCE_HP)
//			{
//				navigation_structure.tx_pack->TxData.ammor_size_5 = BIGGG_ARMOR;
//			}
//			else
//			{
//				navigation_structure.tx_pack->TxData.ammor_size_5 = SMALL_ARMOR;
//			}
//		}
//		/* 蓝方 */
//		if (judge.base_info->car_color == 1)
//		{
//			if (judge.data->game_robot_HP.red_3_robot_HP == BALANCE_HP)
//			{
//				navigation_structure.tx_pack->TxData.ammor_size_3 = BIGGG_ARMOR;
//			}
//			else
//			{
//				navigation_structure.tx_pack->TxData.ammor_size_3 = SMALL_ARMOR;
//			}
//			
//			if (judge.data->game_robot_HP.red_4_robot_HP == BALANCE_HP)
//			{
//				navigation_structure.tx_pack->TxData.ammor_size_4 = BIGGG_ARMOR;
//			}
//			else
//			{
//				navigation_structure.tx_pack->TxData.ammor_size_4 = SMALL_ARMOR;
//			}
//			
//			if (judge.data->game_robot_HP.red_5_robot_HP == BALANCE_HP)
//			{
//				navigation_structure.tx_pack->TxData.ammor_size_5 = BIGGG_ARMOR;
//			}
//			else
//			{
//				navigation_structure.tx_pack->TxData.ammor_size_5 = SMALL_ARMOR;
//			}
//		}
//	}

	/*视觉掉线处理*/
	if(navigation_structure.state.work_state == VISION_OFFLINE)	
	{
		navigation_structure.rx_pack->RxData.chassis_front = 0;
		navigation_structure.rx_pack->RxData.chassis_right = 0;
		
//		navigation_structure.rx_pack->RxData.enemy1_num    = 0;
//		navigation_structure.rx_pack->RxData.enemy1_x      = 0;
//		navigation_structure.rx_pack->RxData.enemy1_y      = 0;
//		navigation_structure.rx_pack->RxData.enemy1_z      = 0;
//		
//		navigation_structure.rx_pack->RxData.enemy2_num    = 0;
//		navigation_structure.rx_pack->RxData.enemy2_x      = 0;
//		navigation_structure.rx_pack->RxData.enemy2_y      = 0;
//		navigation_structure.rx_pack->RxData.enemy2_z      = 0;
//		
//		navigation_structure.rx_pack->RxData.enemy3_num    = 0;
//		navigation_structure.rx_pack->RxData.enemy3_x      = 0;
//		navigation_structure.rx_pack->RxData.enemy3_y      = 0;
//		navigation_structure.rx_pack->RxData.enemy3_z      = 0;
//		
//		navigation_structure.rx_pack->RxData.enemy4_num    = 0;
//		navigation_structure.rx_pack->RxData.enemy4_x      = 0;
//		navigation_structure.rx_pack->RxData.enemy4_y      = 0;
//		navigation_structure.rx_pack->RxData.enemy4_z      = 0;
	}
	
	
	Navigation_SendData();

}




