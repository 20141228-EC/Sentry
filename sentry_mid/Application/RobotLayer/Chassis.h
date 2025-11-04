#ifndef __CHASSIS_H
#define __CHASSIS_H


#include "device.h"
#include "rp_config.h"
#include "rp_math.h"

#define CHAS_SP_MAX_OUT 9000.f


typedef struct chassis_follow_struct{
	pid_ctrl_t speed;
	pid_ctrl_t angle;

}chassis_follow_t;

typedef struct chassis_info_struct
{
    chassis_follow_t *follow_pid;
	
	uint16_t max_power_limit;
	uint16_t power_buffer;
	float power;
	uint8_t cap_switch;

	float           judge_power;        // 裁判功率
    uint8_t         cap_state;          // 开关超电
	
	double 			power_scale;
    double          empty_k1;           // 空载系数k1=功率/速度^2
    double          stuck_k2;           // 堵转系数k2=功率/电流^2
    double          toque_coefficient;  // 力矩系数
    double          constant;           // 补偿常数	
	
	rm_motor_t  *chassis_motor[4];
}chassis_t;

void chassis_move(void);
int16_t chassis_follow(chassis_t *Chassis_Follow,int16_t angle);

extern chassis_t chassis;
extern float initial_total_power;
extern float yuce_power;

#endif