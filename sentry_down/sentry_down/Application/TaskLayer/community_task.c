#include "community_task.h"

uint8_t send_flag;
uint16_t judge_sim_cnt;

void StartCommunityTask(void const * argument)
{
	for(;;)
  {
//		imu_sensor.update(&imu_sensor);

		//	调试用，未连接服务器时更新
		judge_sim_cnt ++;
		if(judge_sim_cnt > 10000 )
		{
			judge_sim_cnt = 0;
			
			game_status_send(2);
			game_robot_HP_send(2);
		}
		
		if(judge_sim_cnt % 100 == 0)
			send_flag = Judge_Decision_send();	// 哨兵决策数据

    osDelay(1);
  }
}


