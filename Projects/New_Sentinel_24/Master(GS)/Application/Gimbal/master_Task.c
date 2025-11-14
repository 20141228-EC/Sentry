/**
  ************************************* Copyright ******************************   
  *                 (C) Copyright 2022, hwx, China, SZU.
  *                            N0   Rights  Reserved
  *                              
  *                   
  * @FileName   : rp_chassis.c   
  * @Version    : v2.0		
  * @Author     : hwx			
  * @Date       : 2023-7-06         
  * @Description:    
  *
  *
  ******************************************************************************
 */
 
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bmi.h"
#include "main.h"
#include "cmsis_os.h"
#include "driver.h"
#include "9015_motor.h"
#include "dji_pid.h"
#include "rp_chassis.h"
#include "rp_gimbal.h"
#include "remote.h"
#include "can_protocol.h"
#include "vision.h"
#include "Car.h"
#include "rp_shoot.h"
#include "judge.h"
#include "vision.h"

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern judge_t                judge;
extern motor_9015_t           motor_9015_structure;
extern motor_9015_info_t      motor_9015_info_structure;
extern motor_9015_base_info_t motor_9015_base_info_structure;
extern motor_9015_pid_t       motor_9015_pid_structure;
extern pid_t                  pid_position_structure;
extern bmi_t                  bmi_structure;
extern chassis_t              Chassis; 
extern rc_t                   rc_structure;
extern Master_Head_t          Master_Head_structure;
extern gimbal_t               Gimbal;
extern vision_t               vision_structure;
extern car_t                  car_structure;
extern IWDG_HandleTypeDef     hiwdg;

extern uint8_t                hurt_mode;
extern uint32_t               hurt_cnt;
extern uint8_t                first2vision;
/* Private variables ---------------------------------------------------------*/

uint8_t    gimbal_init_ok = 0;
int16_t    L_Head_Limit = 650;
int16_t    R_Head_Limit = 6000;
//uint16_t   first2ParVis = 0;
uint8_t    first2outpos = 0;
uint8_t    first2shake  = 0;
int16_t    find_cnt1    = 0;
int16_t    find_cnt2    = 0;
int16_t    find_cnt3    = 0;
int16_t    find_cnt4    = 0;

int16_t    find_tem1    = 0;
int16_t    find_tem2    = 0;
int16_t    find_tem3    = 0;
int16_t    find_tem4    = 0;

int16_t    L_Dynamic_Pit, R_Dynamic_Pit;
int16_t    L_Dynamic_Pit_High, R_Dynamic_Pit_High;

/* Function  body --------------------------------------------------------*/

int16_t error=0;

void master_work(void)
{
	/*云台通讯*/
	Master_Head_HEART(&Master_Head_structure);
	
	/*接收数据解析*/
	/*云台部分*/
	if(gimbal_init_ok == 0)
	{
		Master_Head_structure.Send_L_Head.target_pit = P_Init_L;
		Master_Head_structure.Send_L_Head.target_yaw = Y_Init_L;
		
		Master_Head_structure.Send_R_Head.target_pit = P_Init_R;
		Master_Head_structure.Send_R_Head.target_yaw = Y_Init_R;
		gimbal_init_ok = 1;
	}
	else if(rc_structure.info->status == REMOTE_OFFLINE)
	{
		gimbal_init_ok = 1;
	}
	/*发射机构部分*/

	/*发送数据处理*/
	if(car_structure.mode == two_CAR)
	{

		Master_Head_structure.Send_L_Head.target_pit  -= (float)(rc_structure.base_info->ch3*(-0.008));
		Master_Head_structure.Send_L_Head.target_yaw  += (float)(rc_structure.base_info->ch2*(-0.008));
		
		Master_Head_structure.Send_R_Head.target_pit  += (float)(rc_structure.base_info->ch1*(-0.008));
		Master_Head_structure.Send_R_Head.target_yaw  += (float)(rc_structure.base_info->ch0*(-0.008));
		
		Gimbal.L_Head.target_pit_angle = Master_Head_structure.Send_L_Head.target_pit;
		Gimbal.L_Head.target_yaw_angle = Master_Head_structure.Send_L_Head.target_yaw;
		
		Gimbal.R_Head.target_pit_angle = Master_Head_structure.Send_R_Head.target_pit;
		Gimbal.R_Head.target_yaw_angle = Master_Head_structure.Send_R_Head.target_yaw;
		
		Master_Head_structure.Send_L_Head.gimbal_mode = 1;
		Master_Head_structure.Send_R_Head.gimbal_mode = 1;
		
	}
	else if(car_structure.mode == aim_CAR)
	{		
			/*确认处于测试自瞄模式*/
		if(vision_structure.rx_pack->RxData.left_aim_data.flag.bit.is_find_target == 1)
		{
			Master_Head_structure.Send_L_Head.target_pit  = vision_structure.rx_pack->RxData.left_aim_data.target_pitch;
			Master_Head_structure.Send_L_Head.target_yaw  = vision_structure.rx_pack->RxData.left_aim_data.target_yaw;
		}
		else
		{
			Master_Head_structure.Send_L_Head.target_pit  -= (float)(rc_structure.base_info->ch3*(-0.008));
			Master_Head_structure.Send_L_Head.target_yaw  += (float)(rc_structure.base_info->ch2*(-0.008));
		}
		
		if(vision_structure.rx_pack->RxData.right_aim_data.flag.bit.is_find_target == 1)
		{
			Master_Head_structure.Send_R_Head.target_pit  = vision_structure.rx_pack->RxData.right_aim_data.target_pitch;
			Master_Head_structure.Send_R_Head.target_yaw  = vision_structure.rx_pack->RxData.right_aim_data.target_yaw;
		}
		else
		{
			Master_Head_structure.Send_R_Head.target_pit  += (float)(rc_structure.base_info->ch1*(-0.008));
			Master_Head_structure.Send_R_Head.target_yaw  += (float)(rc_structure.base_info->ch0*(-0.008));
		}
	}
	else if(car_structure.mode == shake_CAR)
	{
		/*摇头模式*/
		
		static uint32_t shake_cnt = 0;
		
		if(first2shake == 0)
		{
			find_cnt1  = 0;
			find_cnt2  = 0;
			find_cnt3  = 0;
			find_cnt4  = 0;
			
			find_tem1  = Master_Head_structure.Send_L_Head.target_pit;
			find_tem2  = Master_Head_structure.Send_L_Head.target_yaw;
			find_tem3  = Master_Head_structure.Send_R_Head.target_pit;
			find_tem4  = Master_Head_structure.Send_R_Head.target_yaw;
		}
		first2shake = 1;
		
		if(vision_structure.rx_pack->FrameHeader.cmd_id == 3 )
		{
			Master_Head_structure.Send_L_Head.target_pit  = vision_structure.rx_pack->RxData.left_aim_data.target_pitch  + find_cnt1;
			Master_Head_structure.Send_L_Head.target_yaw  = vision_structure.rx_pack->RxData.left_aim_data.target_yaw    + find_cnt2;
			Master_Head_structure.Send_R_Head.target_pit  = vision_structure.rx_pack->RxData.right_aim_data.target_pitch  + find_cnt3;
			Master_Head_structure.Send_R_Head.target_yaw  = vision_structure.rx_pack->RxData.right_aim_data.target_yaw    + find_cnt4;
			
			if(shake_cnt++ % 140 >= 70)
			{
					find_cnt1  -= 1;
					find_cnt2  -= 3;
					find_cnt3  += 1;
					find_cnt4  += 3;
			}
			else
			{
					find_cnt1  += 1;
					find_cnt2  += 3;
					find_cnt3  -= 1;
					find_cnt4  -= 3;	
			}
		}
		else 
		{
			
			
			#if 0
			
			shake_cnt++;
			
			Master_Head_structure.Send_L_Head.target_pit = find_tem1 + (int16_t)240*sin((shake_cnt%140)*2*3.14f);
			Master_Head_structure.Send_L_Head.target_yaw = find_tem2 + (int16_t)240*cos((shake_cnt%140)*2*3.14f);
			Master_Head_structure.Send_R_Head.target_pit = find_tem3 + (int16_t)240*sin((shake_cnt%140)*2*3.14f);
			Master_Head_structure.Send_R_Head.target_yaw = find_tem4 + (int16_t)200*cos((shake_cnt%140)*2*3.14f);
			
			
			#endif
			
			#if 1
			
			shake_cnt++;

//			if(shake_cnt % 400 >= 150)
//			{
//					Master_Head_structure.Send_L_Head.target_pit  -= 0;
//					Master_Head_structure.Send_L_Head.target_yaw  += 5;
//					Master_Head_structure.Send_R_Head.target_pit  += 0;
//					Master_Head_structure.Send_R_Head.target_yaw  -= 5;
//			}
//			else
//			{
//					Master_Head_structure.Send_L_Head.target_pit  += 0;
//					Master_Head_structure.Send_L_Head.target_yaw  -= 5;
//					Master_Head_structure.Send_R_Head.target_pit  -= 0;
//					Master_Head_structure.Send_R_Head.target_yaw  += 5;	
//			}
			if(shake_cnt % 400 >= 200)
			{
					Master_Head_structure.Send_L_Head.target_pit  -= 2;
					Master_Head_structure.Send_L_Head.target_yaw  -= 3;
					Master_Head_structure.Send_R_Head.target_pit  += 2;
					Master_Head_structure.Send_R_Head.target_yaw  += 3;
            }
			else
			{
					Master_Head_structure.Send_L_Head.target_pit  += 2;
					Master_Head_structure.Send_L_Head.target_yaw  += 3;
					Master_Head_structure.Send_R_Head.target_pit  -= 2;
					Master_Head_structure.Send_R_Head.target_yaw  -= 3;	
			}
			#endif
			
		}
		
		Master_Head_structure.Send_L_Head.gimbal_mode = 1;
		Master_Head_structure.Send_R_Head.gimbal_mode = 1;
		
		
		
	}
	else if(car_structure.mode == vision_CAR)
	{
		if(vision_structure.state.work_state == VISION_ONLINE)
		{
			
			Master_Head_structure.Send_L_Head.target_pit  = vision_structure.rx_pack->RxData.left_aim_data.target_pitch;
			Master_Head_structure.Send_L_Head.target_yaw  = vision_structure.rx_pack->RxData.left_aim_data.target_yaw;

			Master_Head_structure.Send_R_Head.target_pit  = vision_structure.rx_pack->RxData.right_aim_data.target_pitch;
			Master_Head_structure.Send_R_Head.target_yaw  = vision_structure.rx_pack->RxData.right_aim_data.target_yaw;

			Master_Head_structure.Send_L_Head.gimbal_mode = 1;
			Master_Head_structure.Send_R_Head.gimbal_mode = 1;
		}
		else //保护视觉 
		{
			Master_Head_structure.Send_L_Head.target_pit  -= (float)(rc_structure.base_info->ch3*(-0.008));
			Master_Head_structure.Send_L_Head.target_yaw  += (float)(rc_structure.base_info->ch2*(-0.008));
			
			Master_Head_structure.Send_R_Head.target_pit  += (float)(rc_structure.base_info->ch1*(-0.008));
			Master_Head_structure.Send_R_Head.target_yaw  += (float)(rc_structure.base_info->ch0*(-0.008));
			
				Master_Head_structure.Send_L_Head.gimbal_mode = 1;
				Master_Head_structure.Send_R_Head.gimbal_mode = 1;
			
		}
	}
	else if(car_structure.mode == offline_CAR)
	{
		Master_Head_structure.Send_L_Head.target_pit  = Master_Head_structure.From_L_Head.measure_pit;
		Master_Head_structure.Send_L_Head.target_yaw  = Master_Head_structure.From_L_Head.measure_yaw;
		
		Master_Head_structure.Send_R_Head.target_pit  = Master_Head_structure.From_R_Head.measure_pit;
		Master_Head_structure.Send_R_Head.target_yaw  = Master_Head_structure.From_R_Head.measure_yaw;
		
		Master_Head_structure.Send_L_Head.gimbal_mode = 0;
		Master_Head_structure.Send_R_Head.gimbal_mode = 0;
		
	}
	else if(car_structure.mode == patrol_CAR)
	{
		/*全方位巡逻*/
		Yaw_Auto_L(&Master_Head_structure);
		Pitch_Auto_L(&Master_Head_structure,0);
		
		Yaw_Auto_R(&Master_Head_structure);
		Pitch_Auto_R(&Master_Head_structure,0);
		
		if(hurt_cnt <= 500 &&  hurt_mode != 0)
		{
			Master_Head_structure.Send_L_Head.target_yaw = Y_Init_L;
			Master_Head_structure.Send_R_Head.target_yaw = Y_Init_R;
		}

		Master_Head_structure.Send_L_Head.gimbal_mode = 1;
		Master_Head_structure.Send_R_Head.gimbal_mode = 1;
		
	}
	else if(car_structure.mode == machine_CAR)
	{
		Master_Head_structure.Send_L_Head.target_pit  = P_Init_L;
		Master_Head_structure.Send_L_Head.target_yaw  = Y_Init_L;
		
		Master_Head_structure.Send_R_Head.target_pit  = P_Init_R;/* - error机械模式测散布*/
		Master_Head_structure.Send_R_Head.target_yaw  = Y_Init_R;
		
		Master_Head_structure.Send_L_Head.gimbal_mode = 1;
		Master_Head_structure.Send_R_Head.gimbal_mode = 1;
		
	}
	else
	{
//			Master_Head_structure.Send_L_Head.target_pit = Gimbal.L_Head.target_pit_angle;
//			Master_Head_structure.Send_L_Head.target_yaw = Gimbal.L_Head.target_yaw_angle;
//			
//			Master_Head_structure.Send_R_Head.target_pit = Gimbal.R_Head.target_pit_angle;
//			Master_Head_structure.Send_R_Head.target_yaw = Gimbal.R_Head.target_yaw_angle;
	}
	
	
	/*限位*/
	
	/*YAW*/
	/*超圈(必须先处理，否则动态限位值怪)*/
	if(Master_Head_structure.Send_R_Head.target_yaw >= 8191)
	{
		Master_Head_structure.Send_R_Head.target_yaw -= 8191;	
	}
	else if(Master_Head_structure.Send_R_Head.target_yaw < 0)
	{
		Master_Head_structure.Send_R_Head.target_yaw += 8191;
	}
	
	if(Master_Head_structure.Send_L_Head.target_yaw >= 8191)
	{
		Master_Head_structure.Send_L_Head.target_yaw -= 8191;	
	}
	else if(Master_Head_structure.Send_L_Head.target_yaw < 0 )
	{
		Master_Head_structure.Send_L_Head.target_yaw += 8191;
	}
	

	/*PITCH*/
	/*L_Dynamic_Pit R_Dynamic_Pit*/
	/*前哨模式，向高处巡逻*/
/* 	if(vision_structure.tx_pack->TxData.flag.bit.only_outpose == 0)
	{
		if(Master_Head_structure.From_L_Head.measure_yaw >= 6429 && Master_Head_structure.From_L_Head.measure_yaw <= 7860 )
		{
			L_Dynamic_Pit = 6843;
		}
		else if(Master_Head_structure.From_L_Head.measure_yaw >= 1624  && Master_Head_structure.From_L_Head.measure_yaw <= 2740)
		{
			L_Dynamic_Pit = 7063;
		}
		else 
		{
			L_Dynamic_Pit = 6843;
		}
		
		if(Master_Head_structure.From_R_Head.measure_yaw >= 5860 && Master_Head_structure.From_R_Head.measure_yaw <= 7228 )
		{
			R_Dynamic_Pit = 1411;
		}
		else if(Master_Head_structure.From_R_Head.measure_yaw >= 2613  && Master_Head_structure.From_R_Head.measure_yaw <= 3683)
		{
			R_Dynamic_Pit = 1138;
		}
		else 
		{
			R_Dynamic_Pit = 1411;
		} 
	}
	else
	{
		//R_Dynamic_Pit = P_MOT_LOW_LIMIT_R;
	}*/

	/*R全局动态限位*/
	//头高，值小
	if(Master_Head_structure.From_R_Head.measure_yaw <= 1238)
	{
		R_Dynamic_Pit_High = P_MOT_LOW_LIMIT_R;//822;//
	}
	else if (Master_Head_structure.From_R_Head.measure_yaw >= 7080)
	{
		R_Dynamic_Pit_High = P_MOT_LOW_LIMIT_R;//890;
	}
	else
	{
		R_Dynamic_Pit_High = P_MOT_LOW_LIMIT_R;//所能最高，值最小
	}
	//头最低，值最大
	if(Master_Head_structure.From_R_Head.measure_yaw >= 5904)
	{
		R_Dynamic_Pit = 1587;//头最低，值最大
	}
	else if(Master_Head_structure.From_R_Head.measure_yaw <= 1582)
	{
		R_Dynamic_Pit = 1611;//头最低，值最大
	}
	else
	{
		R_Dynamic_Pit = P_MOT_UPP_LIMIT_R;//所能最低
	}

	if( Master_Head_structure.Send_R_Head.target_pit >= R_Dynamic_Pit)//头最低，值最大
	{
		Master_Head_structure.Send_R_Head.target_pit = R_Dynamic_Pit;
	}
	else if(Master_Head_structure.Send_R_Head.target_pit <= R_Dynamic_Pit_High)//越小越向上
	{
		Master_Head_structure.Send_R_Head.target_pit = R_Dynamic_Pit_High;
	}


	/*L全局动态限位*/
	//头高，值大*左右头电机反置
	if(Master_Head_structure.From_L_Head.measure_yaw <= Y_MOT_LOW_LIMIT_R - 50 && Master_Head_structure.From_L_Head.measure_yaw >= Y_MOT_MID_LIMIT_R)
	{
		L_Dynamic_Pit_High = 2665;//躲雷达
	}
	else if (Master_Head_structure.From_L_Head.measure_yaw >= 1616 && Master_Head_structure.From_L_Head.measure_yaw <= Y_MOT_MID_LIMIT_L)
	{
		L_Dynamic_Pit_High = 2740;//躲灯条
	}
	else
	{
		L_Dynamic_Pit_High = P_MOT_UPP_LIMIT_L;//所能最高 mechMAX
	}
	//头最低，值最小
	if(Master_Head_structure.From_L_Head.measure_yaw >= 1169 && Master_Head_structure.From_L_Head.measure_yaw < 2000)
	{
		L_Dynamic_Pit = P_MOT_LOW_LIMIT_L;//1747;//后平台板
	}
	else if(Master_Head_structure.From_L_Head.measure_yaw >= 2000 && Master_Head_structure.From_L_Head.measure_yaw <= Y_Init_L)
	{
		L_Dynamic_Pit = P_MOT_LOW_LIMIT_L;//前平台板
	}
	else
	{
		L_Dynamic_Pit = P_MOT_LOW_LIMIT_L;//前平台板
	}

	if( Master_Head_structure.Send_L_Head.target_pit <= L_Dynamic_Pit)//下限，低
	{
		Master_Head_structure.Send_L_Head.target_pit = L_Dynamic_Pit;
	}
	else if(Master_Head_structure.Send_L_Head.target_pit >= L_Dynamic_Pit_High)//上限，高
	{
		Master_Head_structure.Send_L_Head.target_pit = L_Dynamic_Pit_High;
	}
	
	/*YAW 死区就近限位*/
	/*R*/
	if( Master_Head_structure.Send_R_Head.target_yaw <= Y_MOT_LOW_LIMIT_R || Master_Head_structure.Send_R_Head.target_yaw >= Y_MOT_MID_LIMIT_R)
	{
		Master_Head_structure.Send_R_Head.target_yaw = Y_MOT_LOW_LIMIT_R;
	}
	else if(Master_Head_structure.Send_R_Head.target_yaw >= Y_MOT_UPP_LIMIT_R && Master_Head_structure.Send_R_Head.target_yaw <= Y_MOT_MID_LIMIT_R)
	{
		Master_Head_structure.Send_R_Head.target_yaw = Y_MOT_UPP_LIMIT_R;
	}
	

	/*L*/
	if( Master_Head_structure.Send_L_Head.target_yaw <= Y_MOT_LOW_LIMIT_L &&  Master_Head_structure.Send_L_Head.target_yaw >= Y_MOT_MID_LIMIT_L)
	{
		Master_Head_structure.Send_L_Head.target_yaw = Y_MOT_LOW_LIMIT_L;
	}
	else if(Master_Head_structure.Send_L_Head.target_yaw >= Y_MOT_UPP_LIMIT_L && Master_Head_structure.Send_L_Head.target_yaw <= Y_MOT_MID_LIMIT_L)
	{
		Master_Head_structure.Send_L_Head.target_yaw = Y_MOT_UPP_LIMIT_L;
	}
	
	
	
	/*标志位清零*/
//	if(car_structure.mode != patrol_CAR && car_structure.mode != vision_CAR)
//	{
//		first2ParVis = 1;
//	}
	if(car_structure.mode != shake_CAR)
	{
		first2shake = 0;
	}
	/*卡头保护*/
	M2H_SENT_DATA(&Master_Head_structure);
	// M2C_SENT_CHASSIS(&Master_Head_structure);//写在底盘核心函数Chassic_work里面

}
