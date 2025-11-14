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
#include "rp_chassis.h"
#include "remote.h"
#include "Car.h"
#include "vision.h"
#include "bmi.h"
#include "rp_gimbal.h"
#include "judge.h"
#include "judge_protocol.h"
#include <math.h>
#include <stdlib.h>
#include "rp_math.h"
#include "navigation.h"

/* Private macro -------------------------------------------------------------*/
#define change_p              (2.0f)

#define FAST_MOV_SPEED		  (2500)	//达到阈值（已*change_p）后，将小陀螺旋转速度限制到 SLOW_SPIN_SPEED
#define SLOW_SPIN_SPEED		  (1600)		//小陀螺旋转速度上限

#define pi                    (3.14159265354f)

#define UNDER_SMALL_HIT       (1)
#define UNDER_BIGGG_HIT       (2)
#define NO_HIT                (0)

#define AERIAL_W              (1)
#define AERIAL_A              (2)
#define AERIAL_S              (3)
#define AERIAL_D              (4)
#define AERIAL_NO             (5)
#define AERIAL_MOVE_TIME      (1700)
#define X_LIMIIT              (14.0f)


/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
extern chassis_t              Chassis; 
extern rc_t                   rc_structure;
extern car_t                  car_structure;
extern vision_t               vision_structure;
extern navigation_t           navigation_structure;
extern bmi_t                  bmi_structure;
extern gimbal_t               Gimbal;
extern judge_t                judge;
extern pid_t                  chassis_pid_follow_structure;
extern pid_t                  chassis_pid_speed_structure[4];
/* Private variables ---------------------------------------------------------*/
float                         cita        = 0;  

uint32_t                      sin_cnt     = 0;
uint32_t                      sin_tick    = 0; 
uint32_t                      cos_tick    = 0; 
uint32_t                      hurt_cnt    = 0;

uint16_t                      right_cnt   = 0;
uint16_t                      fornt_cnt   = 0;
uint16_t                      right_time  = 0;
uint16_t                      fornt_time  = 0;
uint16_t                      aerial_time = 0; //云台手指令运行时间

int32_t                       hurt_change = 0;
int32_t                       vision_spin_time = 0;

int16_t                       spin_speed  = 0;
int16_t                       vision_spin_speed = 0;

uint8_t                       game_star   = 0;
uint8_t                       position_ok = 0;
uint8_t                       hurt_mode   = 0;

uint8_t                       chassis_aerial_cmd = AERIAL_NO;

Max_Acc_Fliter                Front_Acc_Fliter;
Max_Acc_Fliter                Right_Acc_Fliter;
/* Exported macro ------------------------------------------------------------*/

/* Function  body --------------------------------------------------------*/

void chassis_cycle_speed_change(void)
{
	if(vision_spin_time >= 300000)
	{
		vision_spin_time = 0;
	}
	if(sin_tick >= 1300000)
	{
		sin_tick = 0;
	}
	/*转速变化*/
	if(vision_spin_time++ %20000 <= 7000)
	{
		vision_spin_speed = 4000;
	}
	else if(vision_spin_time++ %30000 >= 14000)
	{
		vision_spin_speed = 4500 +  800*sin( ((sin_tick)%1300)*2*3.14f );
	}
	else
	{
		vision_spin_speed = 4000;
	}
	
	
	/*变速小陀螺*/
	if(judge.base_info->game_progress == 4 && judge.base_info->ally_outpost_HP == 0)
	{
		Chassis.base_info.target.cycle_speed = vision_spin_speed + 0*sin( ((sin_tick++)%1000)*2*3.14f ) + hurt_change;
	}
	else
	{
		/*增加判断，假如寻找到敌人，就停转*/
		if(vision_structure.rx_pack->RxData.left_aim_data.flag.bit.is_find_target || vision_structure.rx_pack->RxData.right_aim_data.flag.bit.is_find_target)
		{
			Chassis.base_info.target.cycle_speed = 0;
		}
		else
		{
			Chassis.base_info.target.cycle_speed = 300;
		}
		
	}

	/*状态改变*/
	if(judge.base_info->last_HP - judge.base_info->remain_HP >= 9 && hurt_mode == NO_HIT)
	{
		hurt_mode = UNDER_SMALL_HIT;
	}
	else if(judge.base_info->last_HP - judge.base_info->remain_HP >= 99 && hurt_mode == NO_HIT)
	{
		hurt_mode = UNDER_BIGGG_HIT;
	}
	
	/*状态清零*/
	if(hurt_mode == UNDER_SMALL_HIT)
	{
		if(hurt_cnt++ > 4000)
		{
			hurt_cnt  = 0;
			hurt_mode = NO_HIT;
		}
		
		if(hurt_cnt%1700 >= 900)
		{
			if(vision_spin_speed>= 0 )
			{
				hurt_change = +600;
			}
			else
			{
				hurt_change = -700;
			}						
		}
		else
		{
			hurt_change = 700;
		}
	}
	else if(hurt_mode == UNDER_BIGGG_HIT)
	{
		if(hurt_cnt++ > 6000)
		{
			hurt_cnt  = 0;
			hurt_mode = NO_HIT;
		}
		
		if(hurt_cnt%3000 >= 2000)
		{
			if(vision_spin_speed>= 0 )
			{
				hurt_change = +500;
			}
			else
			{
				hurt_change = -1000;
			}						
		}
		else if(hurt_cnt%3000 <= 1000)
		{
			if(vision_spin_speed>= 0 )
			{
				hurt_change = 900;
			}
			else
			{
				hurt_change = -800;
			}	
		}
		else
		{
			hurt_change = 1000;
		}
	}
	else
	{
		hurt_change = 0;
	}
}

void chassis_fast_move_cycle_speed_limit(void)
{
	if(abs(Chassis.base_info.target.front_speed) > FAST_MOV_SPEED || abs(Chassis.base_info.target.right_speed) > FAST_MOV_SPEED)
	{
		if(Chassis.base_info.target.cycle_speed > SLOW_SPIN_SPEED || Chassis.base_info.target.cycle_speed < SLOW_SPIN_SPEED)
		{
			Chassis.base_info.target.cycle_speed = SLOW_SPIN_SPEED;
		}
	}
}


/**
  * @Name    chassis_work
  * @brief   底盘任务
  * @param   底盘 
  * @retval
  * @author  HWX
  * @Date    2022-11-06
**/
void chassis_work()
{
	/*计算三个方向速度分量*/
	if(car_structure.ctrl == RC_CAR)
	{

		/*机械模式*/
		if(car_structure.mode == machine_CAR)
		{
			/*底盘控制部分*/
			Chassis.base_info.target.front_speed = (1)*(float)rc_structure.base_info->ch3 * (float)Chassis.work_info.config.speed_max / 660.f;
			Chassis.base_info.target.right_speed = (1)*(float)rc_structure.base_info->ch2 * (float)Chassis.work_info.config.speed_max / 660.f;
			Chassis.base_info.target.cycle_speed = (1)*(float)rc_structure.base_info->ch0 / 660.f * Chassis.work_info.config.speed_max;
			
			/*状态位清除*/
			Chassis.config_top_calc(CHASSIS_TOP_CALC_OFF);
			sin_cnt = 0;
			
			/*调整速度限制
			if(rc_structure.base_info->thumbwheel.value == -660)
			{
				if(Chassis.work_info.config.speed_max ++ >= 10000)
				{
					Chassis.work_info.config.speed_max = 10000;
				}
			}
			else if(rc_structure.base_info->thumbwheel.value == 660)
			{
				if(Chassis.work_info.config.speed_max -- <= 0)
				{
					Chassis.work_info.config.speed_max = 0;
				}					
			}*/
		}
		else if(car_structure.mode == spin_CAR )
		{
			Chassis.base_info.target.front_speed = (1)*(float)rc_structure.base_info->ch3 * (float)CHASSIS_SPEED_MAX / 660.f;
			Chassis.base_info.target.right_speed = (1)*(float)rc_structure.base_info->ch2 * (float)CHASSIS_SPEED_MAX / 660.f;
			Chassis.base_info.target.cycle_speed = spin_speed;

			
			Chassis.base_info.measure.top_detal_angle = (float)((MOROT_9015_MIDDLE -  Gimbal.Yaw_9015->base_info->encoder )/65536.0)*2*PI;
			
			Chassis_Top_Speed_Calculating(&Chassis);
			
			Chassis.config_top_calc(CHASSIS_TOP_CALC_ON);
			
			/*调整速度限制*/
			if(rc_structure.base_info->thumbwheel.value == -660)
			{
				if(spin_speed ++ >= 7000)
				{
					spin_speed = 7000;
				}
			}
			else if(rc_structure.base_info->thumbwheel.value == 660)
			{
				if(spin_speed -- <= -7000)
				{
					spin_speed = -7000;
				}
			}
			
		}
		else if(car_structure.mode == two_CAR)
		{
			Chassis.base_info.target.front_speed = 0;
			Chassis.base_info.target.right_speed = 0;
			Chassis.base_info.target.cycle_speed = 0;

			
		}
		else if(car_structure.mode == follow_CAR)
		{
			/*底盘控制部分*/
			Chassis.base_info.target.front_speed = (1)*(float)rc_structure.base_info->ch3 * (float)Chassis.work_info.config.speed_max / 660.f;
			Chassis.base_info.target.right_speed = (1)*(float)rc_structure.base_info->ch2 * (float)Chassis.work_info.config.speed_max / 660.f;
			Chassis.base_info.target.cycle_speed = pid_calc_err_9015(&chassis_pid_follow_structure,
																	  Gimbal.Yaw_9015->base_info->encoder,\
																	  MOROT_9015_MIDDLE); //由pid计算给出
			
			Chassis.base_info.measure.top_detal_angle = (float)((MOROT_9015_MIDDLE -  Gimbal.Yaw_9015->base_info->encoder )/65536.0)*2*PI;
			
			Chassis_Top_Speed_Calculating(&Chassis);
			
			if(Gimbal.Yaw_9015->info->status == _9015_OFFLINE)
			{
				Chassis.base_info.target.cycle_speed = 0;	
			}
			Chassis.config_top_calc(CHASSIS_TOP_CALC_OFF);
			

			
		}
		/*导航模式*/
		else if(car_structure.mode == navigation_CAR)
		{	
			/*2024.4：front 1；right -1*/
			/*旋转外赋值*/
			Chassis.base_info.target.front_speed = (1)*(float)navigation_structure.rx_pack->RxData.chassis_front	* change_p;	//chassis_front 视觉正 前移
			Chassis.base_info.target.right_speed = (-1)*(float)navigation_structure.rx_pack->RxData.chassis_right	* change_p; //视觉正 右移
			/*旋转量赋值*/
			Chassis.base_info.target.cycle_speed = 500;
			chassis_fast_move_cycle_speed_limit();

			Chassis.base_info.measure.top_detal_angle = (float)((MOROT_9015_MIDDLE -  Gimbal.Yaw_9015->base_info->encoder )/65536.0)*2*PI;
			
			#if !TEST_NAV_MOV_DIRECTLY
				/*未开始比赛强制清零*/
				if(judge.base_info->game_progress != 4)			//比赛结束后也不能乱跑
				{
					Chassis.base_info.target.front_speed = 0;
					Chassis.base_info.target.right_speed = 0;
				}
			#endif

			Chassis_Top_Speed_Calculating(&Chassis);
			Chassis.config_top_calc(CHASSIS_TOP_CALC_ON);
			
		}
		/*大小巡逻和自瞄模式*/
		else if(car_structure.mode == patrol_CAR || car_structure.mode ==  shake_CAR || car_structure.mode == vision_CAR)
		{
			/*旋转外赋值*/
			Chassis.base_info.target.front_speed = (1)*(float)navigation_structure.rx_pack->RxData.chassis_front	* change_p;				//chassis_front 视觉正 前移
			Chassis.base_info.target.right_speed = (-1)*(float)navigation_structure.rx_pack->RxData.chassis_right	* change_p; 			//视觉正 右移
			
			/*旋转量赋值*/
			chassis_cycle_speed_change();		
			chassis_fast_move_cycle_speed_limit();

			Chassis.base_info.measure.top_detal_angle = (float)((MOROT_9015_MIDDLE -  Gimbal.Yaw_9015->base_info->encoder )/65536.0)*2*PI;
			
			#if !TEST_NAV_MOV_DIRECTLY
				/*未开始比赛强制清零*/
				if(judge.base_info->game_progress != 4)			//比赛结束后也不能乱跑
				{
					Chassis.base_info.target.front_speed = 0;
					Chassis.base_info.target.right_speed = 0;
				}
			#endif
            
			Chassis_Top_Speed_Calculating(&Chassis);
			Chassis.config_top_calc(CHASSIS_TOP_CALC_ON);
			
		}
		else 
		{
			Chassis.base_info.target.front_speed = 0;
			Chassis.base_info.target.right_speed = 0;
			Chassis.base_info.target.cycle_speed = 0;
		}
		
	}
	
	/*云台手指令*/
	if(judge.info->status == JUDGE_ONLINE)
	{
		if(judge.base_info->robot_commond == 'W') //'W'
		{
			chassis_aerial_cmd = AERIAL_W;
		}
		else if(judge.base_info->robot_commond == 'A') //'A'
		{
			chassis_aerial_cmd = AERIAL_A;
		}
		else if(judge.base_info->robot_commond == 'S') //'S'
		{
			chassis_aerial_cmd = AERIAL_S;
		} 
		else if(judge.base_info->robot_commond == 'D') //'D'
		{
			chassis_aerial_cmd = AERIAL_D;
		}
	}
	else
	{
		//chassis_aerial_cmd = AERIAL_NO;
	}
	
	/*地图上我在左上角
		W
	A	S	D
	*/
	
	/*云台手指令相应*/
	//uint16_t Suppose_Middle_Angle = MOROT_9015_MIDDLE - 8*(bmi_structure.first_yaw_angle - bmi_structure.yaw_angle);
	uint16_t Suppose_Middle_Angle = MOROT_9015_MIDDLE - 8*(4096 - navigation_structure.rx_pack->RxData.yaw_angle);
	if(chassis_aerial_cmd == AERIAL_W)
	{
		if(aerial_time++ >= AERIAL_MOVE_TIME)
		{
			chassis_aerial_cmd = AERIAL_NO;
			judge.base_info->robot_commond = 0;
			aerial_time = 0;
		}
		else
		{
			if(judge.data->ext_robot_command.target_x >= X_LIMIIT)
			{
				Chassis.base_info.target.front_speed = 0;
				Chassis.base_info.target.right_speed = 900 ;
			}
			else
			{
				Chassis.base_info.target.front_speed = 0;
				Chassis.base_info.target.right_speed = 2500 ;
			}
				
			/*底盘小陀螺解算*/
			Chassis.base_info.measure.top_detal_angle = (float)((Suppose_Middle_Angle -  Gimbal.Yaw_9015->base_info->encoder )/65536.0)*2*PI;
			Chassis_Top_Speed_Calculating(&Chassis);
			
			
			Chassis.config_top_calc(CHASSIS_TOP_CALC_ON);
		}
	}
	else if(chassis_aerial_cmd == AERIAL_A)
	{
		if(aerial_time++ >= AERIAL_MOVE_TIME)
		{
			chassis_aerial_cmd = AERIAL_NO;
			judge.base_info->robot_commond = 0;
			aerial_time = 0;
		}
		else
		{
			if(judge.data->ext_robot_command.target_x >= X_LIMIIT)
			{
				Chassis.base_info.target.front_speed = 900;
				Chassis.base_info.target.right_speed = 0 ;
			}
			else
			{
				Chassis.base_info.target.front_speed = 2500;
				Chassis.base_info.target.right_speed = 0 ;
			}
				
			/*底盘小陀螺解算*/
			Chassis.base_info.measure.top_detal_angle = (float)((Suppose_Middle_Angle -  Gimbal.Yaw_9015->base_info->encoder )/65536.0)*2*PI;
			Chassis_Top_Speed_Calculating(&Chassis);
			
			Chassis.config_top_calc(CHASSIS_TOP_CALC_ON);
		}
	}
	else if(chassis_aerial_cmd == AERIAL_D)
	{
		if(aerial_time++ >= AERIAL_MOVE_TIME)
		{
			chassis_aerial_cmd = AERIAL_NO;
			judge.base_info->robot_commond = 0;
			aerial_time = 0;
		}
		else
		{
			if(judge.data->ext_robot_command.target_x >= X_LIMIIT)
			{
				Chassis.base_info.target.front_speed = -900;
				Chassis.base_info.target.right_speed = 0 ;
			}
			else
			{
				Chassis.base_info.target.front_speed = -2500;
				Chassis.base_info.target.right_speed = 0;
			}
			/*底盘小陀螺解算*/
			Chassis.base_info.measure.top_detal_angle = (float)((Suppose_Middle_Angle -  Gimbal.Yaw_9015->base_info->encoder )/65536.0)*2*PI;
			Chassis_Top_Speed_Calculating(&Chassis);
			
			
			Chassis.config_top_calc(CHASSIS_TOP_CALC_ON);
		}
	}
	else if(chassis_aerial_cmd == AERIAL_S)
	{
		if(aerial_time++ >= AERIAL_MOVE_TIME)
		{
			chassis_aerial_cmd = AERIAL_NO;
			judge.base_info->robot_commond = 0;
			aerial_time = 0;
		}
		else
		{
			if(judge.data->ext_robot_command.target_x >= X_LIMIIT)
			{
				Chassis.base_info.target.front_speed = 0;
				Chassis.base_info.target.right_speed = -900 ;
			}
			else
			{
				Chassis.base_info.target.front_speed = 0;
				Chassis.base_info.target.right_speed = -2500 ;
			}
			/*底盘小陀螺解算*/
			/*改变MIDDLE，虚拟的9015MIDDLE，陀螺仪与初始差值*8？*/
			Chassis.base_info.measure.top_detal_angle = (float)((Suppose_Middle_Angle -  Gimbal.Yaw_9015->base_info->encoder )/65536.0)*2*PI;
			Chassis_Top_Speed_Calculating(&Chassis);
			
			
			
			Chassis.config_top_calc(CHASSIS_TOP_CALC_ON);
		}
	}
	
	/*加速曲线--加速度限制
	Front_Acc_Fliter.target_speed     = Chassis.base_info.target.front_speed;
	Front_Acc_Fliter.current_speed    = Chassis.base_info.measure.front_speed;
	Front_Acc_Fliter.max_acceleration = 120;
	
	Right_Acc_Fliter.target_speed     = Chassis.base_info.target.right_speed;
	Right_Acc_Fliter.current_speed    = Chassis.base_info.measure.right_speed;
	Right_Acc_Fliter.max_acceleration = 120;
	
	Chassis.base_info.target.front_speed = SmoothAccelerationUpdate(&Front_Acc_Fliter);
	Chassis.base_info.target.right_speed = SmoothAccelerationUpdate(&Right_Acc_Fliter);*/
	
	Chassis_Work(&Chassis);

}

