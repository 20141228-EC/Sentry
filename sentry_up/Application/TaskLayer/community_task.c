/*
 * @Author: lion
 * @Date: 2025-05-30 20:35:16
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-07-13 20:57:59
 * @FilePath: \sentry_up\Application\TaskLayer\community_task.c
 * @Description: 
 * 
 */
#include "community_task.h"

extern UART_HandleTypeDef huart1;
uint32_t i;
uint8_t vision_fre = 1;



void StartCommunityTask(void const * argument)
{
	for(;;)
  {
		i++;
		if(i%vision_fre == 0)
		{
			Module_Vision_Send(&vision);
		}
		if(i%10== 0)
		{
			Slave_send_enemy_pos(&slave);	
			
			
		}
		if(i%5 == 0)
		{
			if(robot.ctrl_mode == RC_ctrl)
			{
				slave.send_chassis_info(&slave);

			}
		}
		if(i%10 == 0)
		{	
//			if(robot.ctrl_mode == RC_ctrl)
//			{
//				slave.send_chassis_info(&slave);

//			}
			WL_UART_printf("%.2f,%.2f\n",rm_motor[GIMB_Y].motor_pid.speed.out,rm_motor[GIMB_P].motor_pid.speed.out);
			slave.send_flag(&slave);
		}

		osDelay(1);
  }
}


