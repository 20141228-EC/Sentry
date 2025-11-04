/*
 * @Author: lion
 * @Date: 2025-04-24 16:37:04
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-04-30 21:20:18
 * @FilePath: \sentry_down\Application\TaskLayer\control_task.c
 * @Description: 
 * 
 */
#include "control_task.h"

bool Cap_Switch;

extern CAN_HandleTypeDef hcan1;
uint32_t can1_empty;

// spin_switch = 1
static void Chassis_Dynamic_Change(Chassis_t *Chassis, bool hit_flag);

void StartControlTask(void const * argument)
{
  for(;;)
  {
		// --------------- 底盘控制 begin --------------- //
		// 底盘速度更新
		chassis.info->front_speed = \
		constrain(master.info->rx->front, -CHASSIS_MOVE_SPEED, CHASSIS_MOVE_SPEED);
		chassis.info->right_speed = \
		constrain(master.info->rx->right, -CHASSIS_MOVE_SPEED, CHASSIS_MOVE_SPEED);
		chassis.info->cycle_speed = \
		constrain(master.info->rx->cycle, -CHASSIS_MOVE_SPEED, CHASSIS_MOVE_SPEED);
		
		// 动态小陀螺
		if(master.info->rx->cmd_info.bit.spin_switch == 1)
			Chassis_Dynamic_Change(&chassis, hit_flag.flag);
		
		chassis.work(&chassis);
		Chassis_Power_Limit_Ctrl(&power_limit);
		Chassis_Motor_Power_Limit(	&chassis.info->chas_lf_set, &chassis.info->chas_rf_set, \
																&chassis.info->chas_lb_set, &chassis.info->chas_rb_set,	25.f);
		
		int16_t lf = chassis.info->chas_lf_set;
		int16_t rf = chassis.info->chas_rf_set;
		int16_t lb = chassis.info->chas_lb_set;
		int16_t rb = chassis.info->chas_rb_set;
	
		if(master.info->rx->cmd_info.bit.control_cmd && master.work_state == DEV_ONLINE)
			CAN1_Send_200_Motor(rf, lf, rb, lb);
		else
		{
			CAN1_Send_200_Motor(0, 0, 0, 0);
			
			chassis.info->front_speed = 0;
			chassis.info->right_speed = 0;
			chassis.info->cycle_speed = 0;

			chassis.info->chas_lf_set = 0;
			chassis.info->chas_rf_set = 0;
			chassis.info->chas_lb_set = 0;
			chassis.info->chas_rb_set = 0;
		}
		// --------------- 底盘控制 end   --------------- //
	
		if((master.info->rx->cmd_info.bit.cap_switch == 1 || hit_flag.flag == true)&& \
				master.work_state == DEV_ONLINE)
		{
			Cap_Switch = true;
			cap.info.tx.cap_power_out_limit = -300;
		}
		else
		{
			Cap_Switch = false;
			cap.info.tx.cap_power_out_limit = -30;
		}
			
		cap_data_send(1);
		
		can1_empty = HAL_CAN_GetTxMailboxesFreeLevel(&hcan1);
		
		// 功率限制采集数据
//		Chassis_Power_Limit_Update(&power_limit);
//		Chassis_Power_Limit_Verify(&power_limit);
		
		osDelay(1);
  } 
}

void Chassis_Dynamic_Change(Chassis_t *Chassis, bool hit_flag)
{
	float chas_f = Chassis->info->front_speed;
	float chas_r = Chassis->info->right_speed;
	
	// --------- 限制矢量和 Begin --------- //
	Chassis->info->move_speed = sqrt(pow(chas_f, 2.f)+pow(chas_r, 2.f));
	if(Chassis->info->move_speed > CHASSIS_MOVE_SPEED)
	{
		Chassis->info->front_speed = chas_f / Chassis->info->move_speed * CHASSIS_MOVE_SPEED;
		Chassis->info->right_speed = chas_r / Chassis->info->move_speed * CHASSIS_MOVE_SPEED;
	}
	Chassis->info->move_speed = sqrt(pow(Chassis->info->front_speed, 2.f)+pow(Chassis->info->right_speed, 2.f));
	// --------- 限制矢量和 End  --------- //

	if(hit_flag == false)
		Chassis->info->cycle_speed = 2000.f;
	else
		Chassis->info->cycle_speed = 8000.f - Chassis->info->move_speed / 9000.f * 7000.f;
	// move speed		9000	-	0
	// spin speed		1000	-	8000		
}
