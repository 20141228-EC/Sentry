/*
 * @Author: lion
 * @Date: 2025-05-30 20:35:16
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-07-13 21:01:23
 * @FilePath: \sentry_up\Application\RobotLayer\Chassis.c
 * @Description: 
 * 
 */
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
	
};

void chassis_specific(float vx,float vy,float w)
{	
	int16_t LF_Speed, RF_Speed, LB_Speed, RB_Speed;

    LF_Speed = -vx - vy + w;
    LB_Speed = +vx + vy + w;
    RF_Speed = -vx - vy + w;
    RB_Speed = +vx + vy + w;
}

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




