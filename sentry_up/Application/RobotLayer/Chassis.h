#ifndef __CHASSIS_H
#define __CHASSIS_H


#include "device.h"
#include "rp_config.h"
#include "rp_math.h"

typedef struct chassis_follow_struct{
	pid_ctrl_t speed;
	pid_ctrl_t angle;

}chassis_follow_t;

typedef struct chassis_info_struct
{
    chassis_follow_t *follow_pid;
}chassis_t;

void chassis_specific(float vx,float vy,float w);
int16_t chassis_follow(chassis_t *Chassis_Follow,int16_t angle);

extern chassis_t chassis;

#endif