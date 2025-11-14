#include "rp_gimbal.h"
#include "rp_shoot.h"
#include "can_protocol.h"

extern Master_Head_t                 Master_Head_structure;

motor_6020_t           motor_6020_YAW_structure;
motor_6020_base_info_t motor_6020_YAW_base_info;
motor_6020_info_t      motor_6020_YAW_info;


motor_6020_t           motor_6020_PIT_structure;
motor_6020_base_info_t motor_6020_PIT_base_info;
motor_6020_info_t      motor_6020_PIT_info;

pid_t                  pid_s_pit;
pid_t                  pid_s_yaw;

pid_t                  pid_p_pit;
pid_t                  pid_p_yaw;

gimbal_t               gimbal_structure;

extern bmi_t           bmi_structure;
extern shoot_t         shoot_structure;


void gimbal_init(gimbal_t* gimbal)//初始化
{
	/*电机赋值*/
	gimbal->yaw = &motor_6020_YAW_structure;
	gimbal->pitch = &motor_6020_PIT_structure;
	
	/*PID赋值*/
	gimbal->pid_s_pit = &pid_s_pit;
	gimbal->pid_s_yaw = &pid_s_yaw;
	gimbal->pid_p_pit = &pid_p_pit;
	gimbal->pid_p_yaw = &pid_p_yaw;
	
	/*陀螺仪数据赋值*/
	gimbal->bmi = &bmi_structure;
	
	/*电机初始化*/
	MOTOR_6020_INIT( &motor_6020_YAW_structure, &motor_6020_YAW_base_info, &motor_6020_YAW_info );
    MOTOR_6020_INIT( &motor_6020_PIT_structure, &motor_6020_PIT_base_info, &motor_6020_PIT_info );
	
	/*PID初始化*/
	PID_struct_init( gimbal->pid_s_pit,POSITION_PID,S_PIT_LIM_O,S_PIT_LIM_I,S_PIT_PID_P,S_PIT_PID_I,S_PIT_PID_D);
	PID_struct_init( gimbal->pid_s_yaw,POSITION_PID,S_YAW_LIM_O,S_YAW_LIM_I,S_YAW_PID_P,S_YAW_PID_I,S_YAW_PID_D);
	PID_struct_init( gimbal->pid_p_pit,POSITION_PID,P_PIT_LIM_O,P_PIT_LIM_I,P_PIT_PID_P,P_PIT_PID_I,P_PIT_PID_D);
	PID_struct_init( gimbal->pid_p_yaw,POSITION_PID,P_YAW_LIM_O,P_YAW_LIM_I,P_YAW_PID_P,P_YAW_PID_I,P_YAW_PID_D);
	
	/*模式初始化*/
	gimbal->base.mode   = DATA_FROM_BMI_AND_MOTOR;
	gimbal->work.status = Gimbal_Offline;
}


uint8_t test_gimbal_flag = 1;
uint8_t gimbal_cnt = 0;

//int K1 =   3;
//int K2 =   16;
//int K3 =   15;
//#define K4   12
int K1 =   3;//2
int K2 =   8;
int K3 =   10;
#define K4   12

int K1_ =   3;
int K2_ =   8;
int K3_ =   10;
#define K4_   12

#define G_OFFSET(x) x
int16_t gravity_offset = 0;
/*速度全部都用陀螺仪的，机械和跟随模式的区别是角度来源，即位置环测量值*/
void gimbal_work(gimbal_t* gimbal)//现在主要是角度控制，如果需要速度控制后续再加
{
	
	if(gimbal->base.mode == DATA_FROM_MOTOR)//机械模式
	{
		
		/*限位*/
		
		/*YAW-------------------------------------------------------------------*/
		gimbal->yaw->base_info->target_speed   = pid_calc_err(gimbal->pid_p_yaw,\
														  gimbal->yaw->base_info->angle,\
														  gimbal->base.target_yaw);
		
		motor_6020_YAW_structure.output_current           = pid_calc(gimbal->pid_s_yaw,\
															  gimbal->yaw->base_info->speed,\
															  gimbal->yaw->base_info->target_speed);
		
		
		/*PIT-------------------------------------------------------------------*/
		gimbal->pitch->base_info->target_speed = pid_calc(gimbal->pid_p_pit,\
														  gimbal->pitch->base_info->angle,\
														  gimbal->base.target_pit);
		
		motor_6020_PIT_structure.output_current          = pid_calc(gimbal->pid_s_pit,\
															  gimbal->pitch->base_info->speed,\
															  gimbal->pitch->base_info->target_speed);
		

		
		
	}
	else if (gimbal->base.mode == DATA_FROM_BMI_AND_MOTOR)//跟随模式
	{
		/*小型模糊？*/
//		//Kp=K1log（K2abs（error）+K3）
//		gimbal->pid_p_yaw->p = K1*(log(K2 * fabs(gimbal->pid_p_yaw->err[0]) + K3));
//		if(gimbal->pid_p_yaw->p <= K4)
//		{
//			gimbal->pid_p_yaw->p = K4;
//		}
//		
//		gimbal->pid_p_pit->p = K1*(log(K2 * fabs(gimbal->pid_p_pit->err[0]) + K3));
//		if(gimbal->pid_p_pit->p <= K4)
//		{
//			gimbal->pid_p_pit->p = K4;
//		}
//		/*大物实验*/
//		gravity_offset = G_OFFSET(gimbal->pitch->base_info->angle);
//		gravity_offset = 0;
		
		gimbal->pid_p_pit->p = K1*(log(K2 * fabs(gimbal->pid_p_pit->err[0]) + K3));
		if(gimbal->pid_p_pit->p <= K4)
		{
			gimbal->pid_p_pit->p = K4;
		}
		
		gimbal->pid_p_yaw->p = K1_*(log(K2_ * fabs(gimbal->pid_p_yaw->err[0]) + K3_));
		if(gimbal->pid_p_yaw->p <= K4_)
		{
			gimbal->pid_p_yaw->p = K4_;
		}

		
		/*YAW-------------------------------------------------------------------*/
		gimbal->yaw->base_info->target_speed   = pid_calc_err(gimbal->pid_p_yaw,\
														  gimbal->yaw->base_info->angle,\
														  gimbal->base.target_yaw);
		
		motor_6020_YAW_structure.output_current = pid_calc_pit_spd(gimbal->pid_s_yaw,\
															  gimbal->bmi->pit_gro,\
															  gimbal->yaw->base_info->target_speed);
		
		/*电机反装 */
		/*有阻尼 有大误差才?硬钩?*/
		
		/*--PIT-------------------------------------------------------------------*/
		
		gimbal->pitch->base_info->target_speed = pid_calc(gimbal->pid_p_pit,\
														  gimbal->pitch->base_info->angle,\
														  gimbal->base.target_pit);
		
		motor_6020_PIT_structure.output_current = pid_calc_pit_spd(gimbal->pid_s_pit,\
															  gimbal->bmi->yaw_gro,\
															  gimbal->pitch->base_info->target_speed) + gravity_offset;
		
	}
	else if(gimbal->base.mode == DATA_FROM_BMI)
	{
		/*YAW-------------------------------------------------------------------*/
		gimbal->yaw->base_info->target_speed   =   pid_calc_err(gimbal->pid_p_yaw,\
														  gimbal->bmi->pit_angle,\
														  gimbal->base.target_yaw);
		
		motor_6020_YAW_structure.output_current           = pid_calc(gimbal->pid_s_yaw,\
															  gimbal->bmi->pit_gro,\
															  gimbal->yaw->base_info->target_speed);
		
		
		//ok
		/*PIT(目标速度极性是反的)-------------------------------------------------------------------*/
		
		gimbal->pitch->base_info->target_speed = pid_calc_err(gimbal->pid_p_pit,\
														  gimbal->bmi->yaw_angle,\
														  gimbal->base.target_pit);
		
		motor_6020_PIT_structure.output_current          = pid_calc(gimbal->pid_s_pit,\
															  gimbal->bmi->yaw_gro,\
															  gimbal->pitch->base_info->target_speed);
	
	}

	
	if(gimbal->work.status == Gimbal_Online && Master_Head_structure.Send_L_Head.gimbal_mode == 1)
	{
		MOTOR_6020_CAN1_SENT_DATA(motor_6020_YAW_structure.output_current,motor_6020_PIT_structure.output_current,0,0);//注意顺序 
		
		//MOTOR_6020_CAN2_SENT_DATA(0,0,0,0);//这句话是上下主控通讯的关键
	}
	else
	{
		/*卸力*/
		
		MOTOR_6020_CAN1_SENT_DATA(0,0,0,0);//注意顺序 
		
		//MOTOR_6020_CAN2_SENT_DATA(0,0,0,0);//这句话是上下主控通讯的关键
		
	}
}





















