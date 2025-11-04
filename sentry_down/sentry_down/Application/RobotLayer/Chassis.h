#ifndef __CHASSIS_H
#define __CHASSIS_H

#include "device.h"
#include "rp_config.h"
#include "rp_math.h"

#define CHASSIS_MOVE_SPEED 9000.f

typedef struct {
	float front_speed;
	float right_speed;
	float cycle_speed;
	
	float move_speed;

	// 电机目标速度
	int16_t chas_target_lf;
	int16_t chas_target_rf;
	int16_t chas_target_lb;
	int16_t chas_target_rb;
	
	int16_t chas_measure_lf;
	int16_t chas_measure_rf;
	int16_t chas_measure_lb;
	int16_t chas_measure_rb;
	
	int16_t current_lf;
	int16_t current_lb;
	int16_t current_rb;
	int16_t current_rf;
	
	// CAN发送电流
	int16_t chas_lf_set;
  int16_t chas_rf_set;
  int16_t chas_lb_set;
  int16_t chas_rb_set;
} Chassis_Info_t ;

typedef struct chassis_info_struct
{
	Chassis_Info_t	*info;
	rm_motor_t  		*motor[4];
	
	void						(*work)(struct chassis_info_struct *self);
	
}Chassis_t;

void Chassis_Work(Chassis_t *Chassis);

extern Chassis_t chassis;

#endif
