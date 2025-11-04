/* Includes ------------------------------------------------------------------*/
#include "power_limit.h"

#include "Chassis.h"

#define CHAS_SP_MAX_OUT 12000.f

extern float cap_u;
extern bool CAP_STATE, Cap_Switch;

// 静态功率			5.157132992481203 W

Power_Limit_t power_limit = 
{
	// 左前轮 3508，实时更新采集数据，拟合效果		R^2 = 83.7421943703666%
	.k1[CHAS_LF] = 1.674217150088263e-06,
	.k2[CHAS_LF] = 2.057556686880685e-07,
	.k3[CHAS_LF] = 3.603877217197214e-07,
	.p0[CHAS_LF] = 1.431388280455585,
	
	// 右前轮 3508，实时更新采集数据，拟合效果		R^2 = 87.3152280831174%
	.k1[CHAS_RF] = 2.229585596054395e-06,
	.k2[CHAS_RF] = 1.526637367567503e-07,
	.k3[CHAS_RF] = 2.851356877862325e-07,
	.p0[CHAS_RF] = 1.522214145825096,
	
	// 左后轮 3508，实时更新采集数据，拟合效果		R^2 = 90.6398634637620%
	.k1[CHAS_LB] = 2.322332765160292e-06,
	.k2[CHAS_LB] = 1.708981399274932e-07,
	.k3[CHAS_LB] = 2.190190031967800e-07,
	.p0[CHAS_LB] = 1.797657355015395,
	
	// 右后轮 3508，实时更新采集数据，拟合效果		R^2 = 93.7478024517859%
	.k1[CHAS_RB] = 2.414066356080789e-06,
	.k2[CHAS_RB] = 1.544231239635734e-07,
	.k3[CHAS_RB] = 3.215052204806410e-07,
	.p0[CHAS_RB] = 1.239865124121610,
};

/* Exported functions --------------------------------------------------------*/
// 此函数基于西交利物浦功率限制模型，用于采集数据拟合系数
// 电机参数不一，通过更改接线和 chassis_motor_num 采集数据
dev_motor_list_e chassis_motor_num = CHAS_LB;
float matlab_x, matlab_y, matlab_z, matlab_p;
void Chassis_Power_Limit_Update(struct Power_Limit_Struct_t* Power_Limit)
{
		switch(chassis_motor_num)
		{
			case CHAS_LF:
				matlab_x =	chassis.motor[CHAS_LF]->info->speed * \
										chassis.motor[CHAS_LF]->info->current ;
				matlab_y =	chassis.motor[CHAS_LF]->info->current * \
										chassis.motor[CHAS_LF]->info->current ;
				matlab_z =	chassis.motor[CHAS_LF]->info->speed * \
										chassis.motor[CHAS_LF]->info->speed ;
				break;
			
			case CHAS_RF:
				matlab_x =	chassis.motor[CHAS_RF]->info->speed * \
										chassis.motor[CHAS_RF]->info->current ;
				matlab_y =	chassis.motor[CHAS_RF]->info->current * \
										chassis.motor[CHAS_RF]->info->current ;
				matlab_z =	chassis.motor[CHAS_RF]->info->speed * \
										chassis.motor[CHAS_RF]->info->speed ;
				break;
			
			case CHAS_LB:
				matlab_x =	chassis.motor[CHAS_LB]->info->speed * \
										chassis.motor[CHAS_LB]->info->current ;
				matlab_y =	chassis.motor[CHAS_LB]->info->current * \
										chassis.motor[CHAS_LB]->info->current ;
				matlab_z =	chassis.motor[CHAS_LB]->info->speed * \
										chassis.motor[CHAS_LB]->info->speed ;
				break;
				
			case CHAS_RB:
				matlab_x =	chassis.motor[CHAS_RB]->info->speed * \
										chassis.motor[CHAS_RB]->info->current ;
				matlab_y =	chassis.motor[CHAS_RB]->info->current * \
										chassis.motor[CHAS_RB]->info->current ;
				matlab_z =	chassis.motor[CHAS_RB]->info->speed * \
										chassis.motor[CHAS_RB]->info->speed ;
				break;
				
			default:
				break;
		}
	
	matlab_p = judge.info->power_heat_data.chassis_power;
	
	// 电流范围是 -16384~+16384		->		-20A~+20A
	// 转速单位是 rpm							->		rad / s
	// k1		*=	16384/20*268/17*60/2/pi	=	123323.885	0.3		N·m/A		0.000002433
	// k2		*=	16384/20*16384/20				=	671,088.64	0.194 Ω				0.000000289
	// k3		*=	14*60/2/pi*14*60/2/pi		=	17873.0568	0.097 mH			
}


// 用于验证模型正确性，仅测试用
void Chassis_Power_Limit_Verify(struct Power_Limit_Struct_t* Power_Limit)
{
	Power_Limit->power[CHAS_LF] = \
	Power_Limit->k1[CHAS_LF] * chassis.motor[CHAS_LF]->info->speed * \
	chassis.motor[CHAS_LF]->info->current	+
	Power_Limit->k2[CHAS_LF] * chassis.motor[CHAS_LF]->info->current * \
	chassis.motor[CHAS_LF]->info->current	+
	Power_Limit->k3[CHAS_LF] * chassis.motor[CHAS_LF]->info->speed * \
	chassis.motor[CHAS_LF]->info->speed	+
	Power_Limit->p0[CHAS_LF] ;

	Power_Limit->power[CHAS_LB] = \
	Power_Limit->k1[CHAS_LB] * chassis.motor[CHAS_LB]->info->speed * \
	chassis.motor[CHAS_LB]->info->current	+
	Power_Limit->k2[CHAS_LB] * chassis.motor[CHAS_LB]->info->current * \
	chassis.motor[CHAS_LB]->info->current	+
	Power_Limit->k3[CHAS_LB] * chassis.motor[CHAS_LB]->info->speed * \
	chassis.motor[CHAS_LB]->info->speed	+
	Power_Limit->p0[CHAS_LB] ;
	
	Power_Limit->power[CHAS_RF] = \
	Power_Limit->k1[CHAS_RF] * chassis.motor[CHAS_RF]->info->speed * \
	chassis.motor[CHAS_RF]->info->current	+
	Power_Limit->k2[CHAS_RF] * chassis.motor[CHAS_RF]->info->current * \
	chassis.motor[CHAS_RF]->info->current	+
	Power_Limit->k3[CHAS_RF] * chassis.motor[CHAS_RF]->info->speed * \
	chassis.motor[CHAS_RF]->info->speed	+
	Power_Limit->p0[CHAS_RF] ;
	
	Power_Limit->power[CHAS_RB] = \
	Power_Limit->k1[CHAS_RB] * chassis.motor[CHAS_RB]->info->speed * \
	chassis.motor[CHAS_RB]->info->current	+
	Power_Limit->k2[CHAS_RB] * chassis.motor[CHAS_RB]->info->current * \
	chassis.motor[CHAS_RB]->info->current	+
	Power_Limit->k3[CHAS_RB] * chassis.motor[CHAS_RB]->info->speed * \
	chassis.motor[CHAS_RB]->info->speed	+
	Power_Limit->p0[CHAS_RB] ;
	
	// 低通滤波，截止频率与裁判系统功率读取频率一致
	Power_Limit->power_last = Power_Limit->power_now;
	Power_Limit->power_now = Power_Limit->power[CHAS_LF] + Power_Limit->power[CHAS_LB] +
														Power_Limit->power[CHAS_RF] + Power_Limit->power[CHAS_RB];
	Power_Limit->power_now = 0.9f * Power_Limit->power_last + 0.1f * Power_Limit->power_now;
}

float chas_power_max, k_change;
float new_set_lf, new_set_rf, new_set_lb, new_set_rb, k_change_last, judge_power_max;
void Chassis_Power_Limit_Ctrl(struct Power_Limit_Struct_t* Power_Limit)
{
	float power_lf, power_rf, power_lb, power_rb;
	float p_pre_lf, p_pre_rf, p_pre_lb, p_pre_rb;
	float p_lim_lf, p_lim_rf, p_lim_lb, p_lim_rb;
	float predict_p;
	
	// 系数用于补偿模型稳态误差
	judge_power_max = judge.info->game_robot_status.chassis_power_limit * 0.95f;
	
	if(CAP_STATE == 1 && Cap_Switch ==	true)			// 超电正常工作
	{
		if(cap_u > 15.0f )			// 大于 40 %
			chas_power_max =  judge_power_max + 190.f;		//	240 - 290
		else if(cap_u > 10.f)		// 大于 5.5 %							// 要 > 8V
			chas_power_max =  judge_power_max + 38.f * (cap_u - 10.f);		// 此系数需再考虑
		else
			chas_power_max =  judge_power_max;
	}
	else
		chas_power_max = judge_power_max ;
	
	// -------------------- model fit begin -------------------- //
	power_lf = cal_power(Power_Limit, CHAS_LF);
	power_rf = cal_power(Power_Limit, CHAS_RF);
	power_lb = cal_power(Power_Limit, CHAS_LB);
	power_rb = cal_power(Power_Limit, CHAS_RB);
	
	// 低通滤波，注意截止频率
	power_lf = 0.6f * p_pre_lf + 0.4f * power_lf;
	power_rf = 0.6f * p_pre_rf + 0.4f * power_rf;
	power_lb = 0.6f * p_pre_lb + 0.4f * power_lb;
	power_rb = 0.6f * p_pre_rb + 0.4f * power_rb;
	
	predict_p = power_lf + power_rf + power_lb + power_rb;
	// -------------------- model fit end   -------------------- //
	
	if(predict_p < chas_power_max)
	{
		k_change_last = k_change;
		k_change = 1.f;
		
		Power_Limit->power_target[CHAS_LF] = power_lf;
		Power_Limit->power_target[CHAS_RF] = power_rf;
		Power_Limit->power_target[CHAS_LB] = power_lb;
		Power_Limit->power_target[CHAS_RB] = power_rb;
	}
	else
	{
		k_change_last = k_change;
		k_change =  chas_power_max / predict_p;

		//	底盘陀螺仪角度调整

		Power_Limit->power_target[CHAS_LF] = power_lf * k_change;
		Power_Limit->power_target[CHAS_RF] = power_rf * k_change;
		Power_Limit->power_target[CHAS_LB] = power_lb * k_change;
		Power_Limit->power_target[CHAS_RB] = power_rb * k_change;
		
		if(k_change > 0.f)
		{
			// 注意截止频率
			k_change = 0.8f * k_change_last + 0.2f * k_change;
			
			// 更新
			if(power_lf > 0.f)
			{
				p_lim_lf	=	Power_Limit->power_target[CHAS_LF];
			
				new_set_lf	= cal_new_current(chassis.motor[CHAS_LF]->info->speed,\
											p_lim_lf, Power_Limit->k1[CHAS_LF], Power_Limit->k2[CHAS_LF], \
											Power_Limit->k3[CHAS_LF], Power_Limit->p0[CHAS_LF], chassis.info->chas_lf_set);
																		
				chassis.info->chas_lf_set = new_set_lf;
			}
			
			if(power_rf > 0.f)
			{
				p_lim_rf	=	Power_Limit->power_target[CHAS_RF];
			
				new_set_rf	= cal_new_current(chassis.motor[CHAS_RF]->info->speed,\
											p_lim_rf, Power_Limit->k1[CHAS_RF], Power_Limit->k2[CHAS_RF], \
											Power_Limit->k3[CHAS_RF], Power_Limit->p0[CHAS_RF], chassis.info->chas_rf_set);
																		
				chassis.info->chas_rf_set = new_set_rf;
			}
			
			if(power_lb > 0.f)
			{
				p_lim_lb	=	Power_Limit->power_target[CHAS_LB];
			
				new_set_lb	= cal_new_current(chassis.motor[CHAS_LB]->info->speed,\
											p_lim_lb, Power_Limit->k1[CHAS_LB], Power_Limit->k2[CHAS_LB], \
											Power_Limit->k3[CHAS_LB], Power_Limit->p0[CHAS_LB], chassis.info->chas_lb_set);
																		
				chassis.info->chas_lb_set = new_set_lb;
			}
			
			if(power_rb > 0.f)
			{
				p_lim_rb	=	Power_Limit->power_target[CHAS_RB];
				
				new_set_rb	= cal_new_current(chassis.motor[CHAS_RB]->info->speed,\
											p_lim_rb, Power_Limit->k1[CHAS_RB], Power_Limit->k2[CHAS_RB], \
											Power_Limit->k3[CHAS_RB], Power_Limit->p0[CHAS_RB], chassis.info->chas_rb_set);
																		
				chassis.info->chas_rb_set = new_set_rb;
			}
		}
		else
		{
			k_change	= 0.f;
			
			new_set_lf = 0.f;
			new_set_rf = 0.f;
			new_set_lb = 0.f;
			new_set_rb = 0.f;
			
			chassis.info->chas_lf_set = new_set_lf;
			chassis.info->chas_rf_set = new_set_rf;
			chassis.info->chas_lb_set = new_set_lb;
			chassis.info->chas_rb_set = new_set_rb;
		}
	}

}

// w 当前转速		// i 即将发出的目标电流		// p 期望功率
float a, b, c, d;
float cal_new_current(float w, float p, float k1, float k2, float k3, float p0, float i)
{
	float i_new;
	
	a = k2;
	b = w*k1;
	c = k3*pow(w,2)+p0-p;
	d = pow(b,2)-4*k2*c;
	
	if(d >= 0)
	{
		d = sqrt(d);
		
		if(i >= 0)
			i_new = (-b+d)	/	(2.f*a);
		else
			i_new = (-b-d)	/	(2.f*a);
	}
	else
		i_new = i;

	i_new = constrain(i_new, -16000.f, +16000.f);

	return i_new;
}

float cal_power(Power_Limit_t* Power_Limit, dev_motor_list_e num)
{
	float power;
	
	switch(num)
	{
		case CHAS_LF:
			power =	\
			Power_Limit->k1[CHAS_LF]*chassis.motor[CHAS_LF]->info->speed*chassis.info->chas_lf_set+
			Power_Limit->k2[CHAS_LF]*chassis.info->chas_lf_set*chassis.info->chas_lf_set	+
			Power_Limit->k3[CHAS_LF]*chassis.motor[CHAS_LF]->info->speed*chassis.motor[CHAS_LF]->info->speed+
			Power_Limit->p0[CHAS_LF];
			break;
			
		case CHAS_RF:
			power =	\
			Power_Limit->k1[CHAS_RF]*chassis.motor[CHAS_RF]->info->speed*chassis.info->chas_rf_set+
			Power_Limit->k2[CHAS_RF]*chassis.info->chas_rf_set*chassis.info->chas_rf_set	+
			Power_Limit->k3[CHAS_RF]*chassis.motor[CHAS_RF]->info->speed*chassis.motor[CHAS_RF]->info->speed+
			Power_Limit->p0[CHAS_RF];
			break;
			
		case CHAS_LB:
			power =	\
			Power_Limit->k1[CHAS_LB]*chassis.motor[CHAS_LB]->info->speed*chassis.info->chas_lb_set+
			Power_Limit->k2[CHAS_LB]*chassis.info->chas_lb_set*chassis.info->chas_lb_set	+
			Power_Limit->k3[CHAS_LB]*chassis.motor[CHAS_LB]->info->speed*chassis.motor[CHAS_LF]->info->speed	+
			Power_Limit->p0[CHAS_LB];
			break;
			
		case CHAS_RB:
			power =	\
			Power_Limit->k1[CHAS_RB]*chassis.motor[CHAS_RB]->info->speed*chassis.info->chas_rb_set	+
			Power_Limit->k2[CHAS_RB]*chassis.info->chas_rb_set*chassis.info->chas_rb_set	+
			Power_Limit->k3[CHAS_RB]*chassis.motor[CHAS_RB]->info->speed*chassis.motor[CHAS_RB]->info->speed	+
			Power_Limit->p0[CHAS_RB];
			break;
			
		default:
			break;
	}

	return power;
}

void Chassis_Motor_Power_Limit(int16_t* lf, int16_t* rf, int16_t* lb, int16_t* rb,float limit)
{
	float buffer = judge.info->power_heat_data.chassis_power_buffer;
	float Limit_k;

	if(buffer > limit)
		buffer = limit;
	
	Limit_k = buffer / limit;

	if(buffer < limit)
	{
		Limit_k = Limit_k * Limit_k *  Limit_k;
		
		*lf = (int16_t)(*lf * Limit_k);	
		*rf = (int16_t)(*rf * Limit_k);	
		*lb = (int16_t)(*lb * Limit_k);	
		*rb = (int16_t)(*rb * Limit_k);
	}
	else
		Limit_k = 1.f;
}
