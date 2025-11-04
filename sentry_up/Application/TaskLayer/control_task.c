/*
 * @Author: lion
 * @Date: 2025-05-30 20:35:16
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-07-25 05:08:43
 * @FilePath: \sentry_up\Application\TaskLayer\control_task.c
 * @Description: 
 * 
 */
#include "control_task.h"
float angle;

void StartControlTask(void const * argument)
{
  for(;;)
  {		
		# if 1
			//传递发现敌人标志位以及对应数字
				Slave_get_vision_status(&slave,vision.info->rx_info->is_find_target,vision.info->rx_info->robot_num);
				//传递发现敌人的位置信息
				slave.info->tx_info->enemy_pos_info->enemy_x = (uint16_t)vision.info->rx_info->enemy_x;
				slave.info->tx_info->enemy_pos_info->enemy_y = (uint16_t)vision.info->rx_info->enemy_y;
		#endif
		if(dev_list.rc_sen->work_state == DEV_ONLINE)
		{
			
			//获取上主控开始标志位
			slave.get_start(&slave,1);
			//底盘开始控制标志位
			slave.info->tx_info->chassis_info->cmd_info.bit.control_cmd = 1;
			Robot_Ctrl(&robot);
			CAN_SendAll();
			
			rc_sensor_s_step(&rc_sensor);
		}
		else
		{
			//获取上主控结束标志位
			slave.get_start(&slave,0);
			slave.info->tx_info->chassis_info->cmd_info.bit.control_cmd = 0;

			Robot_Sleep(&robot);
			
			CAN_SendAllZero();
			
		}

			
		osDelay(1);
  }
  
}
