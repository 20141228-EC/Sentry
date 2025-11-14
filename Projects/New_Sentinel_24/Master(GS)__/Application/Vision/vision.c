#include "vision.h"
#include "navigation.h"
#include "bmi.h"
#include "can_protocol.h"
#include "rp_shoot.h"
#include "rp_gimbal.h"
#include "judge.h"
#include "Car.h"

/*使用串口3*/
extern UART_HandleTypeDef huart3;//使用串口3

extern judge_t          judge;
extern Master_Head_t    Master_Head_structure;
extern bmi_t            bmi_structure;
extern car_t            car_structure;
extern navigation_t     navigation_structure;

vision_rx_packet_t      vision_rx_pack;
vision_tx_packet_t      vision_tx_pack;
vision_t                vision_structure;

uint8_t                 must_patrol_enable = 0;
extern float            yaw_vision_use;

/*发送Buffer*/
uint8_t                 vision_txBuf[140]; 


#define LEFT_YAW_MID_ANGLE 5387
#define LEFT_PITCH_MID_ANGLE 4274
#define RIGHT_YAW_MID_ANGLE 5470
#define RIGHT_PITCH_MID_ANGLE 1204

int16_t EC2Vision_yaw(int16_t angle, int16_t MID)
{
	int16_t angle_out;
	int16_t err;

	/*偏移零点至中值对面*/
	err = (MID - 4096);

	if(angle >= err && angle <= 8192)
	{
		angle_out = angle - err;
	}
	else if(angle < err && angle >= 0)
	{
		angle_out = angle - err + 8192;
	}

	/*左右镜像，取补*/
	angle_out = 8192 - angle_out;

	/*转为-4096 ~ 0 ~ 4096系*/
	angle_out -= 4096;

	/*超圈*/
	if(angle_out > 8191)
	{
		angle_out -= 8192;
	}
	else if(angle_out < 0)
	{
		angle_out += 8192;
	}
	return angle_out;
}

int16_t Vision2EC_yaw(int16_t angle, int16_t MID)
{
	int16_t angle_out;
	int16_t err;

	/*偏移零点至中值对面*/
	err = (MID - 4096);

	/*转为0 ~ 8192系*/
	angle += 4096;

	/*左右镜像，取补*/
	angle = 8192 - angle;

	if(angle >= err && angle <= 8192)
	{
		angle_out = angle - err;
	}
	else if(angle < err && angle >= 0)
	{
		angle_out = angle - err + 8192;
	}

	/*超圈*/
	if(angle_out > 8191)
	{
		angle_out -= 8192;
	}
	else if(angle_out < 0)
	{
		angle_out += 8192;
	}
	return angle_out;
}

int16_t Control2Vision_pitch_left(int16_t angle)
{
	int16_t angle_out;
	angle_out = (-1) * angle + (LEFT_PITCH_MID_ANGLE - 4096);
	
	if(angle_out > 8191)
	{
		angle_out -= 8192;
	}
	else if(angle_out < 0)
	{
		angle_out += 8192;
	}
	return angle_out;
}

int16_t Control2Vision_yaw_left(int16_t angle)
{
	int16_t angle_out;
	angle_out = (1) * angle - (LEFT_YAW_MID_ANGLE - 4096);
	if(angle_out > 8191)
	{
		angle_out -= 8192;
	}
	else if(angle_out < 0)
	{
		angle_out += 8192;
	}
	return angle_out;
}

int16_t Control2Vision_pitch_right(int16_t angle)
{
	int16_t angle_out;
	angle_out = (1) * angle - (RIGHT_PITCH_MID_ANGLE - 4096);
	if(angle_out > 8191)
	{
		angle_out -= 8192;
	}
	else if(angle_out < 0)
	{
		angle_out += 8192;
	}
	return angle_out;
}

int16_t Control2Vision_yaw_right(int16_t angle)
{
	int16_t angle_out;
	angle_out = (1) * angle - (RIGHT_YAW_MID_ANGLE - 4096);
	if(angle_out > 8191)
	{
		angle_out -= 8192;
	}
	else if(angle_out < 0)
	{
		angle_out += 8192;
	}
	return angle_out;
}

int16_t Vision2Control_pitch_left(int16_t angle)
{
	volatile int16_t angle_out;
	angle_out = (-1) * angle + (LEFT_PITCH_MID_ANGLE - 4096);
//	if(angle_out > 8191)
//	{
//		angle_out -= 8192;
//	}
//	else if(angle_out < 0)
//	{
//		angle_out += 8192;
//	}
	return angle_out;

}

int16_t Vision2Control_yaw_left(int16_t angle)
{
	volatile int16_t angle_out;
	angle_out = (-1) * angle + (LEFT_YAW_MID_ANGLE - 4096);
//	if(angle_out > 8191)
//	{
//		angle_out -= 8192;
//	}
//	else if(angle_out < 0)
//	{
//		angle_out += 8192;
//	}
	return angle_out;
}

int16_t Vision2Control_pitch_right(int16_t angle)
{
	 volatile int16_t angle_out;
	angle_out = (-1) * angle - (RIGHT_PITCH_MID_ANGLE - 4096);
	
//	if(angle_out > 8191)
//	{
//		angle_out -= 8192;
//	}
//	else if(angle_out < 0)
//	{
//		angle_out += 8192;
//	}
	return angle_out;
}

int16_t test_yaw_angle;
int16_t Vision2Control_yaw_right(int16_t angle)
{
	volatile int16_t angle_out;
	angle_out = (-1) * angle + (RIGHT_YAW_MID_ANGLE - 4096);
//	if(angle_out > 8191)
//	{
//		angle_out -= 8192;
//	}
//	else if(angle_out < 0)
//	{
//		angle_out += 8192;
//	}
    test_yaw_angle = angle_out;
	return angle_out;
}

/**
  * @Name    Vision_Init
  * @brief   视觉部分结构体初始化
  * @param   None
  * @retval  
  * @author  HWX
  * @Date    2022-10-21
**/
uint16_t test_len = 0;
void Vision_Init(void)
{
	/*结构体初始化*/
	vision_structure.rx_pack = &vision_rx_pack;
	vision_structure.tx_pack = &vision_tx_pack;
	
	/*数据初始化*/
	vision_structure.tx_pack->FrameHeader.sof                   = VISION_SEND_ID;
	vision_structure.tx_pack->FrameHeader.cmd_id                = CMD_PATROL;

	
	
	/*工作状态初始化*/
	vision_structure.state.offline_max_cnt = VISION_OFFLINE_MAX_CNT;
	vision_structure.state.offline_cnt     = VISION_OFFLINE_MAX_CNT;
	vision_structure.state.work_state      = VISION_OFFLINE;
	
	//test_len = 1*sizeof(vision_tx_packet_t);
	
	
	
	
	
}


/**
  * @Name    Vision_SendData
  * @brief   与小电脑通讯，CRC校验数据肯定要改
  * @param   None                              
  * @retval
  * @author  HWX
  * @Date    2022-10-21
**/
bool Vision_SendData(void)
{
	/*判断是否掉线*/
	if(vision_structure.state.offline_cnt++ >= vision_structure.state.offline_max_cnt)
	{
		vision_structure.state.offline_cnt--;
		vision_structure.state.work_state = VISION_OFFLINE;
	}
	else
	{
		vision_structure.state.work_state = VISION_ONLINE;
	}
	
	/*数据发送*/	
	memcpy(vision_txBuf, &vision_tx_pack, sizeof(vision_tx_packet_t));
	
	/*增加CRC校验位*/
	Append_CRC8_Check_Sum(vision_txBuf, LEN_FRAME_HEADER);
	Append_CRC16_Check_Sum(vision_txBuf, LEN_VISION_TX_PACKET);
	
	
	if(HAL_UART_Transmit_DMA(&huart3,vision_txBuf,sizeof(vision_tx_packet_t)) == HAL_OK)
	{
		//memset(&vision_tx_pack,0,sizeof(vision_tx_pack));
		return true;
	}
	else
	{
		//memset(&vision_tx_pack,0,sizeof(vision_tx_pack));
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
bool Vision_GetData(uint8_t *rxBuf)
{

	
	if(rxBuf[0] == 0xA5)
	{
		if(Verify_CRC8_Check_Sum(rxBuf, LEN_FRAME_HEADER) == true)
		{
			if(Verify_CRC16_Check_Sum(rxBuf,LEN_VISION_RX_PACKET) == true)
			{
				memcpy(&vision_rx_pack, rxBuf, LEN_VISION_RX_PACKET);
//				/*超热量测试代码*/
//				vision_structure.rx_pack->RxData.L_is_find_target = 1;
//				vision_structure.rx_pack->RxData.L_shoot_speed = 250;
//				vision_structure.rx_pack->RxData.L_shoot_cnt += 3;
				
				/*异常数据清除*/
				vision_rx_pack.RxData.left_aim_data.shoot_frequency = 250;
				vision_rx_pack.RxData.right_aim_data.shoot_frequency = 250;
				

				
				/*数据更新*/
				vision_rx_pack.RxData.left_aim_data.target_pitch = Vision2Control_pitch_left(vision_rx_pack.RxData.left_aim_data.target_pitch) + 4096;
				
				vision_rx_pack.RxData.left_aim_data.target_yaw = ((1)*Vision2Control_yaw_left(vision_rx_pack.RxData.left_aim_data.target_yaw) + 4096);
				
				vision_rx_pack.RxData.right_aim_data.target_pitch = (-1)*Vision2Control_pitch_right(vision_rx_pack.RxData.right_aim_data.target_pitch) + 4096;
				
				vision_rx_pack.RxData.right_aim_data.target_yaw =((1)*Vision2Control_yaw_right(vision_rx_pack.RxData.right_aim_data.target_yaw) + 4096);
				
				vision_rx_pack.RxData.B_yaw_angle               = ((-1)*vision_rx_pack.RxData.B_yaw_angle + 4096); 
                
                
                if(	vision_rx_pack.RxData.right_aim_data.target_yaw > 8191)
                {
                    	vision_rx_pack.RxData.right_aim_data.target_yaw -= 8192;
                }
                else if(	vision_rx_pack.RxData.right_aim_data.target_yaw < 0)
                {
                    	vision_rx_pack.RxData.right_aim_data.target_yaw += 8192;
                }
                
				if(	vision_rx_pack.RxData.left_aim_data.target_yaw > 8191)
                {
                    	vision_rx_pack.RxData.left_aim_data.target_yaw -= 8192;
                }
                else if(	vision_rx_pack.RxData.left_aim_data.target_yaw < 0)
                {
                    	vision_rx_pack.RxData.left_aim_data.target_yaw += 8192;
                }
				/*没干死广工的秘密武器
				if(vision_rx_pack.RxData.right_aim_data.target_pitch <= P_MOT_LOW_LIMIT || vision_rx_pack.RxData.right_aim_data.target_pitch >= 4000)
				{
					vision_rx_pack.RxData.right_aim_data.target_pitch = P_MOT_LOW_LIMIT;
				}
				else if(vision_rx_pack.RxData.right_aim_data.target_pitch >= P_MOT_UPP_LIMIT && vision_rx_pack.RxData.right_aim_data.target_pitch <= 4000)
				{
					vision_rx_pack.RxData.right_aim_data.target_pitch = P_MOT_UPP_LIMIT;
				}

				if(vision_rx_pack.RxData.left_aim_data.target_pitch <= P_MOT_LOW_LIMIT_ && vision_rx_pack.RxData.left_aim_data.target_pitch >= 3000)
				{
					vision_rx_pack.RxData.left_aim_data.target_pitch = P_MOT_LOW_LIMIT_;
				}
				else if(vision_rx_pack.RxData.left_aim_data.target_pitch >= P_MOT_UPP_LIMIT_ || vision_rx_pack.RxData.left_aim_data.target_pitch <= 3000)
				{
					vision_rx_pack.RxData.left_aim_data.target_pitch = P_MOT_UPP_LIMIT_;
				}*/

				/*掉线计数器清零*/
				vision_structure.state.offline_cnt = 0;
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
void Vision_Update(void)
{
	//数据接收已经在校验完成时便结束了
		
}

/**
  * @Name    USART3_rxDataHandler
  * @brief   串口3接收中断回调函数
  * @param   None
  * @retval
  * @author  HWX
  * @Date    2022-10-21
**/
void USART3_rxDataHandler(uint8_t *rxBuf)
{	
	Vision_GetData(rxBuf);
}


void Vision_Heart()
{
	if(vision_structure.state.offline_cnt++ >= vision_structure.state.offline_max_cnt)
	{
		vision_structure.state.offline_cnt = vision_structure.state.offline_max_cnt;
		vision_structure.state.work_state = VISION_OFFLINE;
	}
	else
	{
		vision_structure.state.work_state = VISION_ONLINE;
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

int16_t test_1 = 0;
int16_t test_2 = 0;
int16_t test_3 = 0;
int16_t test_4 = 0;


void vision_task(void)
{	
	
	
	/*发送数据处理*/
		if(car_structure.mode == aim_CAR)
		{
			vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_CHECK;
			
		}
		else
		{
			if(judge.base_info->robot_commond == 'Y')//Y
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_A;
			}
			else if(judge.base_info->robot_commond == 'K')//K
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_B;
			}
			else if(judge.base_info->robot_commond == 'I')//I
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_D;
			}
			else if(judge.base_info->robot_commond == 'V')//V
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_E;
			}
			else if(judge.base_info->robot_commond == 'H')//H
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_STOP;
			}
			else if(judge.base_info->robot_commond == 'O')//O
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_G;
			}
			else if(judge.base_info->robot_commond == 'L')//L
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_H;
			}
			else if(judge.base_info->robot_commond == 'T')//T
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_R3;
			}
			else if(judge.base_info->robot_commond == 'G')//G
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_J;
			}
			else if(judge.base_info->robot_commond == 'F')//F
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_POINT_K;
			}
			else if(judge.base_info->robot_commond == 'Q')//Q
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_OUTPOS;
			}
			else if(judge.base_info->robot_commond == 'N')//N
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_R4;
			}
			else if(judge.base_info->robot_commond == 'Z')//Z
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_GOGOGO;
			}
			else if(judge.base_info->robot_commond == 'J')//J
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_FUCK_OUTPOS;
			}

			else
			{
				vision_structure.tx_pack->FrameHeader.cmd_id   = CMD_STOP;
			}
			
		}

		if(judge.base_info->robot_commond == 'E')//E
		{
			if(judge.data->ext_robot_command.target_x >= X_LIMIIT)
			{
				//vision_structure.tx_pack->TxData.flag.bit.engine_hit_enable = 1;
			}	
			else
			{
				//vision_structure.tx_pack->TxData.flag.bit.engine_hit_enable = 0;
			}
			
		}
		
		if(judge.base_info->robot_commond == 'C')//C
		{
			
			if(judge.data->ext_robot_command.target_x >= X_LIMIIT)
			{
				//vision_structure.tx_pack->TxData.flag.bit.hero_hit_enable = 1;
			}	
			else
			{  
				//vision_structure.tx_pack->TxData.flag.bit.hero_hit_enable = 0;
			}
			
		}
					
		if(judge.base_info->robot_commond == 'X')//X
		{
			
			if(judge.data->ext_robot_command.target_x >= X_LIMIIT)
			{
				must_patrol_enable = 1;
			}	
			else
			{  
			    must_patrol_enable = 0;
			}
			
		}
		
		if(judge.base_info->robot_commond == 'B')//B
		{
			
			if(judge.data->ext_robot_command.target_x >= X_LIMIIT)
			{
				//vision_structure.tx_pack->TxData.only_sentry = 1;
			}	
			else
			{  
			   // vision_structure.tx_pack->TxData.only_sentry = 0;
			}
			
		}
		
		if(vision_structure.tx_pack->FrameHeader.cmd_id == CMD_POINT_A || vision_structure.tx_pack->FrameHeader.cmd_id == CMD_FUCK_OUTPOS) 
		{
			//vision_structure.tx_pack->TxData.flag.bit.only_outpose = 1;
		}	
		else
		{  
			//vision_structure.tx_pack->TxData.flag.bit.only_outpose = 0;
		}

		
		
		
		
		
		vision_structure.tx_pack->TxData.left_head_pitch           = Control2Vision_pitch_left(Master_Head_structure.From_L_Head.measure_pit) - 4096;
		vision_structure.tx_pack->TxData.left_head_yaw             = (-1)*(Control2Vision_yaw_left(Master_Head_structure.From_L_Head.measure_yaw) - 4096);
		vision_structure.tx_pack->TxData.right_head_pitch          = Control2Vision_pitch_right(Master_Head_structure.From_R_Head.measure_pit) - 4096;
		vision_structure.tx_pack->TxData.right_head_yaw            = (-1)*(Control2Vision_yaw_right(Master_Head_structure.From_R_Head.measure_yaw) - 4096);
		
		vision_structure.tx_pack->TxData.big_yaw_angle             = (-1)*(yaw_vision_use - 4096.0f);
		
		
		
		
		vision_structure.tx_pack->TxData.left_hit_mode = 1;
		vision_structure.tx_pack->TxData.right_hit_mode = 1;
		
		vision_structure.tx_pack->TxData.left_trigger_mode = 0;
		vision_structure.tx_pack->TxData.right_trigger_mode = 0;
		
		vision_structure.tx_pack->TxData.left_is_switch_target = 0;
		vision_structure.tx_pack->TxData.right_is_switch_target = 0;
		
		
		vision_structure.tx_pack->TxData.own_remain_bullets = judge.base_info->remain_bullte;
		vision_structure.tx_pack->TxData.shoot_speed = 30;
		vision_structure.tx_pack->TxData.use_vision = 1;
		
		vision_structure.tx_pack->TxData.enemy_color       = judge.base_info->car_color;
		
		

		vision_structure.tx_pack->TxData.armor3_size = 0;
		vision_structure.tx_pack->TxData.armor4_size = 0;
		vision_structure.tx_pack->TxData.armor5_size = 0;
		
		vision_structure.tx_pack->TxData.no_hit_enemy2 = 0;
		vision_structure.tx_pack->TxData.no_hit_enemy6 = 0;
		vision_structure.tx_pack->TxData.no_hit_enemy7 = 0;
		vision_structure.tx_pack->TxData.no_hit_enemy8 = 0;
		
		//TODO::Serialport:设置敌方初始血量值
        /* 血量   0自己 1英雄 2工程 3-5步兵  6哨兵 7前哨站 8基地 */
        /*为了保证在uint8_t的范围 ：blood0-6（机器人） 是 实时血量÷5； blood7-8（建筑） 是 实时血量÷10 */
		vision_structure.tx_pack->TxData.own_blood6 = judge.base_info->remain_HP;
		vision_structure.tx_pack->TxData.own_blood7 = judge.base_info->friendly_outposts_HP;
		vision_structure.tx_pack->TxData.own_blood8 = judge.base_info->friendly_base_HP;
		
		vision_structure.tx_pack->TxData.enemy_blood1 = judge.base_info->enemy_hero_HP;
		vision_structure.tx_pack->TxData.enemy_blood2 = judge.base_info->enemy_engeing;
		vision_structure.tx_pack->TxData.enemy_blood3 = judge.base_info->enemy_infanry3_HP;
		vision_structure.tx_pack->TxData.enemy_blood4 = judge.base_info->enemy_infanry4_HP;
		vision_structure.tx_pack->TxData.enemy_blood5 = judge.base_info->enemy_infanry5_HP;
		vision_structure.tx_pack->TxData.enemy_blood6 = judge.base_info->enemy_Shaobing_HP;
		vision_structure.tx_pack->TxData.enemy_blood7 = judge.base_info->enemy_outposts_HP;
		vision_structure.tx_pack->TxData.enemy_blood8 = judge.base_info->enemy_base_HP;
		
		
		
		
		/* 比赛5秒倒计时阶段 */
//		if (judge.data->game_status.game_progress == 3)
//		{
//			/* 红方 */
//			if (judge.base_info->car_color == 0)
//			{

//				if (judge.data->game_robot_HP.blue_3_robot_HP == BALANCE_HP)
//				{
//					vision_structure.tx_pack->TxData.flag.bit.ammor_size_3 = BIGGG_ARMOR;
//				}
//				else
//				{
//					vision_structure.tx_pack->TxData.flag.bit.ammor_size_3 = SMALL_ARMOR;
//				}
//				
//				if (judge.data->game_robot_HP.blue_4_robot_HP == BALANCE_HP)
//				{
//					vision_structure.tx_pack->TxData.flag.bit.ammor_size_4 = BIGGG_ARMOR;
//				}
//				else
//				{
//					vision_structure.tx_pack->TxData.flag.bit.ammor_size_4 = SMALL_ARMOR;
//				}
//				
//				if (judge.data->game_robot_HP.blue_5_robot_HP == BALANCE_HP)
//				{
//					vision_structure.tx_pack->TxData.flag.bit.ammor_size_5 = BIGGG_ARMOR;
//				}
//				else
//				{
//					vision_structure.tx_pack->TxData.flag.bit.ammor_size_5 = SMALL_ARMOR;
//				}
//			}
//			/* 蓝方 */
//			if (judge.base_info->car_color == 1)
//			{
//				if (judge.data->game_robot_HP.red_3_robot_HP == BALANCE_HP)
//				{
//					vision_structure.tx_pack->TxData.flag.bit.ammor_size_3 = BIGGG_ARMOR;
//				}
//				else
//				{
//					vision_structure.tx_pack->TxData.flag.bit.ammor_size_3 = SMALL_ARMOR;
//				}
//				
//				if (judge.data->game_robot_HP.red_4_robot_HP == BALANCE_HP)
//				{
//					vision_structure.tx_pack->TxData.flag.bit.ammor_size_4 = BIGGG_ARMOR;
//				}
//				else
//				{
//					vision_structure.tx_pack->TxData.flag.bit.ammor_size_4 = SMALL_ARMOR;
//				}
//				
//				if (judge.data->game_robot_HP.red_5_robot_HP == BALANCE_HP)
//				{
//					vision_structure.tx_pack->TxData.flag.bit.ammor_size_5 = BIGGG_ARMOR;
//				}
//				else
//				{
//					vision_structure.tx_pack->TxData.flag.bit.ammor_size_5 = SMALL_ARMOR;
//				}
//			}
//		}
		

		/*视觉掉线处理*/
		if(vision_structure.state.work_state == VISION_OFFLINE)	
		{
			vision_structure.rx_pack->RxData.left_aim_data.shoot_frequency = 0;
			vision_structure.rx_pack->RxData.left_aim_data.flag.bit.is_find_target = 0;
			
			vision_structure.rx_pack->RxData.right_aim_data.shoot_frequency = 0;
			vision_structure.rx_pack->RxData.right_aim_data.flag.bit.is_find_target = 0;
			
			vision_structure.rx_pack->FrameHeader.cmd_id = 0;
			
		}
	
	
	Vision_SendData();

}




