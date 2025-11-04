/*
 * @Author: lion
 * @Date: 2025-07-16 19:25:00
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-07-26 22:59:42
 * @FilePath: \sentry_mid\Application\TaskLayer\community_task.c
 * @Description: 
 * 
 */
#include "community_task.h"

uint32_t i;
int16_t input = 300,output = -300;


void StartCommunityTask(void const * argument)
{
	uint16_t test_yaw;
	for(;;)
  {
    i++;
    
    if(i>=60000)
    {
        i=0;
    }
	if(i % 1 == 0)
	{
		strategy_send_data();	
	}
	if(i % 5 == 0)
	{
    
		//pc电脑控制底盘速度（一定要注意在这个模式下遥控不能再对底盘发送速度控制信息）
		if(communicate.communicate_rx_up_info->ctrl_mode == PC_ctrl)
		{
			Communicate_send_chassis(&communicate);
		}
		
	}
    if(i % 10== 0)
    {
		/*发送云台信息*/
		Communicate_send_up_info(Strategy.vision->mode.gimbal_mode,Strategy.vision->mode.detect_mode,Strategy.vision->speed.current_yaw
																,Strategy.vision->speed.current_x,Strategy.vision->speed.current_y);
    }
    

    osDelay(1);
  }
}


