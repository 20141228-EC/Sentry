 
/* Includes ------------------------------------------------------------------*/
#include "judge_protocol.h"

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
int32_t test_launch_cnt;


void judge_update(judge_t *self, uint16_t canId, uint8_t *rxbuf)
{
	switch (canId)
	{
		case GAME_STATUS_ID: // 3Hz  比赛阶段
			memcpy((void*)&judge.info->game_status, (void*)rxbuf, 8);
			if(judge.info->game_status.game_progress == 4)
			{
				vision.info->tx_info->start_game = 1;
			}
			else
			{
				vision.info->tx_info->start_game = 0;
			}
			
			break;
		case GAME_ROBOT_HP_ID: // 1Hz 敌人血量
			
			break;
		case POWER_HEAT_ID: // 50Hz 枪口热量
			memcpy((void*)&judge.info->power_heat_data.shooter_id1_17mm_cooling_heat, (void*)rxbuf, 2);
			break;
		case GAME_ROBOT_STATUS_ID: // 10Hz 发射热量限制和冷却速率
			judge.info->my_color = rxbuf[0];
			memcpy((void*)&judge.info->game_robot_status.shooter_barrel_heat_limit, (void*)&rxbuf[1], 2);
			memcpy((void*)&judge.info->game_robot_status.shooter_barrel_cooling_value, (void*)&rxbuf[3], 2);
			break;
		case ROBOT_HURT_ID: // max25Hz 受击打装甲板
			judge.info->robot_hurt.armor_id = rxbuf[0];
			judge.flag->hurt_flag = 1;
			break;
		case SHOOT_DATA_ID: // max25Hz 弹速和频率
			test_launch_cnt--;
			memcpy((void*)&judge.info->shoot_data.bullet_speed, (void*)rxbuf, 4);
			memcpy((void*)&judge.info->shoot_data.bullet_freq, (void*)&rxbuf[4], 1);
			Static_shoot();
			break;
		
		default:
			break;
	}
	
	self->info->offline_cnt = 0;
}
