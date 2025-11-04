#include "Gimbal.h"

IMU_Pid_t IMU_Angle[2] ={
	[IMU_P] = {
	  .speed.kp = -70,//-180
		.speed.ki = 0,//-0.2
		.speed.kd = 0,
		.speed.integral_max = 10000,//5000
		.speed.out_max = 18000,
		.angle.kp = 8.5,//15
		.angle.ki = 0.03,//0.05
		.angle.kd = 0,
		.angle.integral_max = 4000,
		.angle.out_max = 8000,
	
	},
	[IMU_Y] = {
		.speed.kp = 400,
		.speed.ki = 0,
		.speed.kd = 0,
		.speed.integral_max = 10000,
		.speed.out_max = 28000,
		.angle.kp = 15,
		.angle.ki = 0.01,
		.angle.kd = 0,
		.angle.integral_max = 1000,
		.angle.out_max = 8000,
	
	},
};
gimbal_t gimbal = {
    .IMU_yaw = &IMU_Angle[IMU_Y],
    .IMU_pitch = &IMU_Angle[IMU_P],
    .pitch_motor = &rm_motor[GIMB_P],
    .yaw_motor = &rm_motor[GIMB_Y],
	.work_state = DEV_OFFLINE,
    .Reversal_flag = 0,
};

void Men_To_Angle(rm_motor_t *motor, int16_t angle,float axis_rate)
{
	angle = angle % 8192;
	motor->motor_pid.angle.measure = motor->info->angle;
	motor->motor_pid.angle.target = angle;
	motor->motor_pid.angle.err = angle - motor->motor_pid.angle.measure;

	if(motor->motor_pid.angle.err < 0)
	{
		motor->motor_pid.angle.err += 8192;
	}
	if(motor->motor_pid.angle.err > 4096)
	{
		motor->motor_pid.angle.err = motor->motor_pid.angle.err - 8192;
	}
	//角度环
	single_pid_ctrl(&motor->motor_pid.angle);
	
	//角速度作为内环,速度转/s->
    motor->motor_pid.speed.measure = axis_rate;
    motor->motor_pid.speed.target = motor->motor_pid.angle.out;
	motor->motor_pid.speed.err = motor->motor_pid.angle.out - axis_rate;
	single_pid_ctrl(&motor->motor_pid.speed);
	//发送数据到数组
    if(motor->work_state == DEV_ONLINE)
    {
        Motor_SendData(motor->driver, motor->motor_pid.speed.out);
    }
	
}

void Imu_To_Angle(rm_motor_t *motor, float angle, float axis,float axis_rate,IMU_Pid_t *IMU_pid)
{
	IMU_pid->angle.measure = axis;
	IMU_pid->angle.target = angle;
	IMU_pid->angle.err = angle - IMU_pid->angle.measure;
	
	//半圈处理
	if(IMU_pid->angle.err >  180){IMU_pid->angle.err -= 360;}
	if(IMU_pid->angle.err < -180){IMU_pid->angle.err += 360;}
	
	//角度外环，计算出角速度内环目标值
	single_pid_ctrl(&IMU_pid->angle);
	
	//根据外环算出的角速度目标值，计算出内环误差
    IMU_pid->speed.measure = axis_rate;
    IMU_pid->speed.target = IMU_pid->angle.out;
	IMU_pid->speed.err = IMU_pid->angle.out - axis_rate;
	
	//角速度内环，计算真正输出
	single_pid_ctrl(&IMU_pid->speed);
	
	//发送数据到数组
    if(motor->work_state == DEV_ONLINE)
    {
        Motor_SendData(motor->driver, IMU_pid->speed.out);
    }
}

void Menc_P_Y(int16_t Pitch,int16_t Yaw)
{
	Men_To_Angle(&rm_motor[GIMB_P],Pitch,imu_sensor.info->base_info.ave_rate_pitch);
//    Motor_SendData(rm_motor[GIMB_P].driver,0);
	Men_To_Angle(&rm_motor[GIMB_Y],Yaw,imu_sensor.info->base_info.ave_rate_yaw);
}

void Imu_P_Y(float Pitch,float Yaw)
{
	Imu_To_Angle(&rm_motor[GIMB_P],\
								Pitch,\
								imu_sensor.info->base_info.pitch,\
								imu_sensor.info->base_info.ave_rate_pitch,\
								&IMU_Angle[IMU_P]);
	
	Imu_To_Angle(&rm_motor[GIMB_Y],\
								Yaw,\
								imu_sensor.info->base_info.yaw,\
								imu_sensor.info->base_info.ave_rate_yaw,\
								&IMU_Angle[IMU_Y]);
}