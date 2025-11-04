#include "Chassis.h"

chassis_follow_t follow_pid = {
	.speed.kp = 3,
	.speed.ki = 0.01,
	.speed.kd = 0,
	.speed.integral_max = 1000,
	.speed.out_max = 8000,

	.angle.kp = -0.1,
	.angle.ki = 0,
	.angle.kd = 0,
	.angle.integral_max = 0,
	.angle.out_max = 1000,
};

chassis_t chassis = {
    .follow_pid = &follow_pid,
	
	.max_power_limit = 60.0f,
	.power_buffer = 0,
	.power = 0,
    .empty_k1 = 1.970097e-7,
    .stuck_k2 = 1.9923e-7,
    .toque_coefficient = 2.43243e-6,
    .constant = 1.001,
	
	.chassis_motor = {
		&rm_motor[CHAS_LF],
		&rm_motor[CHAS_LB],
		&rm_motor[CHAS_RF],
		&rm_motor[CHAS_RB],
	},
};

float chassis_max_power = 0;
float initial_total_power = 0;
float last_initial_total_power;
float a = 0.2;
float motor_chassis_out[4];

double heat_rate, Limit_k, CHAS_LimitOutput, CHAS_TotalOutput;
/*目标转速和实际转速的误差*/
int32_t	speed_err_sum;


float control_kp = 0;
int16_t err_belive = 0;
double initial_give_power[4];

float temp1,temp2,temp3,temp4;
float yuce_power;

//void chassis_power_control(chassis_t *chassis_control)
//{
//    /*底盘最大限制功率*/
//    float chassis_max_power = 0;
//    /*每个电机初始给定功率*/
//    
//    /*初始总功率*/
////    double initial_total_power = 0;
//    /*缩放后的给定功率*/
//    double scaled_give_power[4];
//	/*每个电机目标速度和实际速度的误差*/
//	int16_t	motor_speed_err[4] = {0};

//////	/*读取裁判系统信息*/
//	chassis_control->power_buffer = judge.info->power_heat_data.chassis_power_buffer;
//	chassis_control->max_power_limit =  100;//judge.game_robot_status.chassis_power_limit
//    #if CAP_ENABLE
//	/*是否开启超电(开了就可以提升最大限制功率,然后用新的进行计算)*/
//	

//    #else
//    chassis_max_power = chassis_control->max_power_limit;
//    #endif
////	/*计算初始给定功率*///(功率 = stuck_k2 * 电流^2 + 力矩 * 角速度 + empty_k1 * 速度^2 + 补偿常数) (力矩 = 力矩系数 * 电流)
//	for(uint8_t i = 0; i < 4; i++)
//	{
//		initial_give_power[i] = chassis_control->chassis_motor[i]->motor_pid.speed.out * chassis_control->toque_coefficient * chassis_control->chassis_motor[i]->info->speed
//								+ chassis_control->empty_k1 * chassis_control->chassis_motor[i]->info->speed * chassis_control->chassis_motor[i]->info->speed
//								+ chassis_control->stuck_k2 * chassis_control->chassis_motor[i]->motor_pid.speed.out * chassis_control->chassis_motor[i]->motor_pid.speed.out
//								+ chassis_control->constant;
//		temp1 = chassis_control->chassis_motor[i]->motor_pid.speed.out * chassis_control->toque_coefficient * chassis_control->chassis_motor[i]->info->speed;
//		temp2 = chassis_control->empty_k1 * chassis_control->chassis_motor[i]->info->speed * chassis_control->chassis_motor[i]->info->speed;
//		temp3 = chassis_control->stuck_k2 * chassis_control->chassis_motor[i]->motor_pid.speed.out * chassis_control->chassis_motor[i]->motor_pid.speed.out;
//		
//		motor_chassis_out[i] = chassis_control->chassis_motor[i]->motor_pid.speed.out;
//		
//		temp4 = chassis_control->constant;
//		
//		/*功率非负*/
//		if(initial_give_power[i] < 0)
//			continue;
//		initial_total_power += initial_give_power[i];
//		
//		motor_speed_err[i] = chassis_control->chassis_motor[i]->motor_pid.speed.err;
//		 
//		speed_err_sum += motor_speed_err[i];
//	}
//	yuce_power = initial_total_power;
//	initial_total_power = a*initial_total_power+(1-a)*last_initial_total_power; 
//	last_initial_total_power = initial_total_power;

//	/*超功率后缩放*/
//	if(initial_total_power > chassis_max_power)
//	{
//		chassis_control->power_scale = chassis_max_power / initial_total_power;

//		for(uint8_t i = 0; i < 4; i++)
//		{
//			//当误差不是很大时不做处理
//			if(motor_speed_err[i]>=err_belive)
//			{
//				scaled_give_power[i] = (1-control_kp)*initial_give_power[i] * chassis_control->power_scale 
//									+control_kp *(float)motor_speed_err[i]/speed_err_sum;
//			}
//			else
//			{
//				scaled_give_power[i] = (1-control_kp)*initial_give_power[i] * chassis_control->power_scale;
//			}
//			
//			if(scaled_give_power[i] < 0)
//				continue;

//			/*功率分配好之后计算电流值*///(由前面功率公式反推电流值)
//			double a = chassis_control->stuck_k2;
//			double b = chassis_control->toque_coefficient * chassis_control->chassis_motor[i]->info->speed;
//			double c = chassis_control->empty_k1 * chassis_control->chassis_motor[i]->info->speed * chassis_control->chassis_motor[i]->info->speed + chassis_control->constant - scaled_give_power[i];
//			float32_t delta = b * b - 4 * a * c;  //这里不用double,因为不需要太高的精度,而且嵌入式中常用float32_t,特别是包含DSP库
//			float32_t pout;
//			arm_sqrt_f32(delta, &pout);
//			
//			if(chassis_control->chassis_motor[i]->motor_pid.speed.out > 0)
//			{
//				float32_t temp = (-b + pout) / (2 * a);
//				if(temp > 16000)
//					chassis_control->chassis_motor[i]->motor_pid.speed.out = 16000;
//				else
//					chassis_control->chassis_motor[i]->motor_pid.speed.out = temp;
//			}
//			else
//			{
//				float32_t temp = (-b - pout) / (2 * a);
//				if(temp < -16000)
//					chassis_control->chassis_motor[i]->motor_pid.speed.out = -16000;
//				else
//					chassis_control->chassis_motor[i]->motor_pid.speed.out = temp;
//			}
//		}
//	}
//	initial_total_power = 0;
//	
//	/*祖传功率限制作为保险*/
//	double OUT_MAX = 0.f;
//	OUT_MAX = CHAS_SP_MAX_OUT * 4.f;
//	
//	if(chassis_control->power_buffer > 60.f)
//    {
//        /*防止飞坡之后缓冲250J变为正增益系数*/
//        chassis_control->power_buffer = 60.f;
//    }
//	
//	Limit_k = chassis_control->power_buffer / 60.f;
//	
//	if(chassis_control->power_buffer <25)
//	{
//		Limit_k = Limit_k*Limit_k;
//	}
//	
//    /*与之前的区别在于只有buffer小于15才限制*/
//	if(chassis_control->power_buffer <= 60.f)
//		CHAS_LimitOutput = Limit_k * OUT_MAX;
//	else 
//		CHAS_LimitOutput = OUT_MAX;    
//	
//    CHAS_TotalOutput = abs(chassis_control->chassis_motor[0]->motor_pid.speed.out) + abs(chassis_control->chassis_motor[1]->motor_pid.speed.out) 
//                        + abs(chassis_control->chassis_motor[2]->motor_pid.speed.out) + abs(chassis_control->chassis_motor[3]->motor_pid.speed.out) ;
//	
//	heat_rate = CHAS_LimitOutput / CHAS_TotalOutput;
//	
//   if(CHAS_TotalOutput >= CHAS_LimitOutput)
//   {
//		for(char i = 0 ; i < 4 ; i++)
//		{	
//			chassis_control->chassis_motor[i]->motor_pid.speed.out = (chassis_control->chassis_motor[i]->motor_pid.speed.out * heat_rate);		
//		}
//    }

//}


//void chassis_specific(int16_t vx,int16_t vy,int16_t w)
//{	
//	int16_t LF_Speed, RF_Speed, LB_Speed, RB_Speed;
//	
//	#if 0
//	float Angle_Error = (imu_sensor.info->base_info.yaw/360) * 2*PI;
//	
//	float tempx = Strategy.vision->speed.vx;
//	float tempy = Strategy.vision->speed.vy;
//	
//	vx =  -tempx * arm_cos_f32(Angle_Error) + tempy * arm_sin_f32(Angle_Error);
//	vy = tempx * arm_sin_f32(Angle_Error) + tempy * arm_cos_f32(Angle_Error);
//	#endif
//	
//    LF_Speed = -vx - vy + w;
//    LB_Speed = -vx + vy + w;
//    RF_Speed = +vx - vy + w;
//    RB_Speed = +vx + vy + w;

//    Motor_ToSpeed(&rm_motor[CHAS_LF],LF_Speed);
//	Motor_ToSpeed(&rm_motor[CHAS_LB],LB_Speed);
//	Motor_ToSpeed(&rm_motor[CHAS_RF],RF_Speed);
//	Motor_ToSpeed(&rm_motor[CHAS_RB],RB_Speed);
//	
////	chassis_power_control(&chassis);
//	
//	Motor_SendData(rm_motor[CHAS_LF].driver,chassis.chassis_motor[CHAS_LF]->motor_pid.speed.out);
//	Motor_SendData(rm_motor[CHAS_LB].driver,chassis.chassis_motor[CHAS_LB]->motor_pid.speed.out);
//	Motor_SendData(rm_motor[CHAS_RF].driver,chassis.chassis_motor[CHAS_RF]->motor_pid.speed.out);
//	Motor_SendData(rm_motor[CHAS_RB].driver,chassis.chassis_motor[CHAS_RB]->motor_pid.speed.out);

//}

int16_t chassis_follow(chassis_t *Chassis_Follow,int16_t angle)
{
	Chassis_Follow->follow_pid->angle.measure = big_yaw.info->p_int;
    Chassis_Follow->follow_pid->angle.target = angle;
	Chassis_Follow->follow_pid->angle.err = Chassis_Follow->follow_pid->angle.target - Chassis_Follow->follow_pid->angle.measure;
	 if(Chassis_Follow->follow_pid->angle.err < 0)
	{
			Chassis_Follow->follow_pid->angle.err += 65535;
	}
	if(Chassis_Follow->follow_pid->angle.err > 65535/2)
	{
		Chassis_Follow->follow_pid->angle.err -= 65535;
	}
	
	single_pid_ctrl(&Chassis_Follow->follow_pid->angle);
	
	Chassis_Follow->follow_pid->speed.err = Chassis_Follow->follow_pid->angle.out - imu_sensor.info->base_info.rate_yaw;
	single_pid_ctrl(&Chassis_Follow->follow_pid->speed);

	return Chassis_Follow->follow_pid->speed.out;
}




