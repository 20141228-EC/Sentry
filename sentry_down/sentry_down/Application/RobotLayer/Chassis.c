#include "Chassis.h"

static void Chassis_Work(Chassis_t *Chassis);

Chassis_Info_t chassis_info;

Chassis_t chassis = {
	.info	=	&chassis_info,

	.motor = {
		&rm_motor[CHAS_LF],
		&rm_motor[CHAS_LB],
		&rm_motor[CHAS_RF],
		&rm_motor[CHAS_RB],
	},
	
	.work = Chassis_Work,
};

void Chassis_Work(Chassis_t *Chassis)
{
	float vx = Chassis->info->front_speed;
	float vy = Chassis->info->right_speed;
	float w	 = Chassis->info->cycle_speed;

  Chassis->info->chas_target_lf = -vx - vy + w;
	Chassis->info->chas_target_lb = -vx + vy + w;
	Chassis->info->chas_target_rf = +vx - vy + w;
	Chassis->info->chas_target_rb = +vx + vy + w;

  Motor_ToSpeed(&rm_motor[CHAS_LF], Chassis->info->chas_target_lf);
	Motor_ToSpeed(&rm_motor[CHAS_LB], Chassis->info->chas_target_lb);
	Motor_ToSpeed(&rm_motor[CHAS_RF], Chassis->info->chas_target_rf);
	Motor_ToSpeed(&rm_motor[CHAS_RB], Chassis->info->chas_target_rb);
	
	Chassis->info->chas_lf_set = Chassis->motor[CHAS_LF]->motor_pid.speed.out;
	Chassis->info->chas_lb_set = Chassis->motor[CHAS_LB]->motor_pid.speed.out;
	Chassis->info->chas_rf_set = Chassis->motor[CHAS_RF]->motor_pid.speed.out;
	Chassis->info->chas_rb_set = Chassis->motor[CHAS_RB]->motor_pid.speed.out;
	
	// 测量值 测试用
	Chassis->info->chas_measure_lb = Chassis->motor[CHAS_LB]->info->speed;

	// 测量值 - 用于测试和调试
	Chassis->info->chas_measure_lf = Chassis->motor[CHAS_LF]->info->speed;
	Chassis->info->chas_measure_lb = Chassis->motor[CHAS_LB]->info->speed;
	Chassis->info->chas_measure_rf = Chassis->motor[CHAS_RF]->info->speed;
	Chassis->info->chas_measure_rb = Chassis->motor[CHAS_RB]->info->speed;

	Chassis->info->current_lf = Chassis->motor[CHAS_LF]->info->current;
	Chassis->info->current_lb = Chassis->motor[CHAS_LB]->info->current;
	Chassis->info->current_rf = Chassis->motor[CHAS_RF]->info->current;
	Chassis->info->current_rb = Chassis->motor[CHAS_RB]->info->current;

}

