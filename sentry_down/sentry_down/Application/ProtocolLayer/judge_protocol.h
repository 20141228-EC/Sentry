	
#ifndef __JUDGE_POTOCOL_H
#define __JUDGE_POTOCOL_H

/* Includes ------------------------------------------------------------------*/
#include "rp_config.h"
#include "judge.h"

/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void game_status_send(uint8_t can_num);					// 1Hz 发送	//	剩余时间2	标志颜色1
void game_robot_HP_send(uint8_t can_num);				// 3Hz 发送	//	敌方血量7	// 己方血量7	
void game_robot_status_send(uint8_t can_num);		// 10Hz发送	//	当前热量2	热量限制2	自身坐标4
void game_robot_position_send(uint8_t can_num);	// 1Hz 发送	//	己方机器人坐标数据
void sentry_decision_send(uint8_t can_num);			// 1Hz 发送	//	哨兵决策数据
void game_robot_cmd_send(uint8_t can_num);			// 操作手发送
void radar_pos_coor_send(uint8_t can_num);			// 雷达发送	//	循环发送两个数据包	12bit
void shoot_data_send(uint8_t can_num);					// 射击发送	//	弹丸射速4

#endif
