#include "Robot.h"

move_info_t move_info = {
	.vx = 0,
	.vy = 0,
	.w = 0,
	.Mech_Y = GIMBAL_MID,
	.Mech_P = GIMBAL_MECH_PITCH_MID,
	.IMU_Y = 0,
	.IMU_P = 0,
	.rc_power = 1,
	.pc_power = 1,
	.turning_speed = 0,
};

flag_info_t flag_info = {
	.init_flag = 0,
};

Robot_t robot = {
	.move_mode = INIT_MODE,
	.ctrl_mode = RC_ctrl,
	.last_move_mode = INIT_MODE,
	.last_ctrl_mode = RC_ctrl,
	.move_info = &move_info,
	.flag_info = &flag_info,
	.chassis = &chassis,
	.gimbal = &gimbal,
	.booster = &booster,
};

void limit_angle(Robot_t *robot);


void RC_Getinfo(Robot_t  *robot)
{
    robot->move_mode = host.info->rx_info->move_mode;
	
}

void RC_Move_Ctrl(Robot_t *robot)
{

	chassis_move();
}

void PC_Getinfo(Robot_t  *robot)
{
	robot->ctrl_mode = host.info->rx_info->ctrl_mode;

	robot->move_info->vx = Strategy.vision->speed.vx;
	robot->move_info->vy = Strategy.vision->speed.vy;
	robot->move_info->w = Strategy.vision->speed.vw;
}

void PC_Move_Ctrl(Robot_t *robot)
{
	chassis_specific(robot->move_info->vx,robot->move_info->vy,robot->move_info->w);
}

void Robot_Ctrl(Robot_t *robot)
{
	//上主控发送控制模式
	robot->ctrl_mode = host.info->rx_info->ctrl_mode;
    switch (robot->ctrl_mode)
    {
        case RC_ctrl:
        {
            RC_Getinfo(robot);
            RC_Move_Ctrl(robot);
			break;
        }
        case PC_ctrl:
        {
			PC_Getinfo(robot);
			PC_Move_Ctrl(robot);
			break;
        }
    }
}

void Robot_Sleep(Robot_t *robot)
{
	robot->move_info->vx = 0;
	robot->move_info->vy = 0;
	robot->move_info->w = 0;
	
	Strategy.vision->speed.vw = 0;
	Strategy.vision->speed.vx  = 0;
	Strategy.vision->speed.vy = 0;
}

/*******pitch限位******/
float pitch_max_err, pitch_min_err;
float max, min;
static void limit_angle(Robot_t *robot)
{
	/*Y(0,8191)		IMU_Y(-180,180)*/
	if(robot->move_info->Mech_Y < 0)			{robot->move_info->Mech_Y += 8192;}
	if(robot->move_info->Mech_Y > 8191)		{robot->move_info->Mech_Y -= 8192;}
	if(robot->move_info->IMU_Y > 180)	{robot->move_info->IMU_Y -= 360;}
	if(robot->move_info->IMU_Y < -180)	{robot->move_info->IMU_Y += 360;}

	/*Pitch*/
	if(robot->move_mode == MECHANICAL_MODE)
	{
		//固定限位
		if(robot->move_info->Mech_P < GIMBAL_MECH_PITCH_MAX)			{robot->move_info->Mech_P = GIMBAL_MECH_PITCH_MAX;}
		else if(robot->move_info->Mech_P > GIMBAL_MECH_PITCH_MIN)		{robot->move_info->Mech_P = GIMBAL_MECH_PITCH_MIN;}
	}
	
	if(robot->move_info->IMU_P > GIMBAL_GYRO_PITCH_MAX)
	{
		robot->move_info->IMU_P = GIMBAL_GYRO_PITCH_MAX;
	}
	if(robot->move_info->IMU_P < -GIMBAL_GYRO_PITCH_MIN)
	{
        robot->move_info->IMU_P = -GIMBAL_GYRO_PITCH_MIN;
	}

    //动态限位
//	if(Module.move_mode == GYRO_MODE || Module.move_mode == WHEEL_MODE)
//	{
//		//动态限位,上下坡时可以保持pitch始终控制在前方一个固定范围
//		if(rm_motor[GIMB_P].info->angle > 4096)//跳变处理
//		{
//			pitch_max_err = (float)(rm_motor[GIMB_P].info->angle - 8192 - P_UP_Angle) * _8192_TO_360;
//			pitch_min_err = (float)(rm_motor[GIMB_P].info->angle - 8192 - P_DOWN_Angle) * _8192_TO_360;
//		}
//		else
//		{	
//			pitch_max_err = (float)(rm_motor[GIMB_P].info->angle - P_UP_Angle) * _8192_TO_360;
//			pitch_min_err = (float)(rm_motor[GIMB_P].info->angle - P_DOWN_Angle) * _8192_TO_360;
//		}

//		max = pitch_max_err + imu_sensor.info->base_info.pitch;
//		min = pitch_min_err + imu_sensor.info->base_info.pitch;

//		if(info->IMU_P > max) 			{info->IMU_P = max;}
//		else if(info->IMU_P < min)		{info->IMU_P = min;}
//	}
}