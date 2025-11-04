#ifndef __STRATEGY_H
#define __STRATEGY_H

#include "driver.h"
#include "crc.h"
#include "device.h"
#include "rp_config.h"
#include "usbd_cdc_if.h"
#include "communicate.h"

// 云台->底盘   记得区分前后相机
typedef __packed struct AllianceDetect_struct
{
   float Enemy_x;
   float Enemy_y;
   uint8_t Enemy_num;
   uint8_t is_find_target;
}AllianceDetect_t ;

// 裁判系统->底盘
typedef __packed struct AllianceBlood_struct
{
    uint16_t hero_blood;
    uint16_t engineer_blood;
    uint16_t infantry1_blood;
    uint16_t infantry2_blood;
    uint16_t sentry_blood;
    uint16_t outpost_blood;
    uint16_t basepost_blood;
}AllianceBlood_t ;

typedef __packed struct EnemyBlood_struct
{
    uint16_t hero_blood;
    uint16_t engineer_blood;
    uint16_t infantry1_blood;
    uint16_t infantry2_blood;
    uint16_t sentry_blood;
    uint16_t outpost_blood;
    uint16_t basepost_blood;
} EnemyBlood_t;

typedef __packed struct ContestEvent_struct
{
    uint8_t game_start;              // 0-待机 1-prepare15秒 2-start5秒
    uint16_t game_time;				//剩余时间
    uint16_t remain_money;		//剩余金币
	uint8_t	oprator_mode;			//uio   直接回堡垒   先回补单再回堡垒 		梯形高小陀螺				p		确认不复活
	float operator_x;					//云台指定坐标x
    float operator_y;					//云台指定坐标y
	uint8_t resurgence;              // 云台手买活发1
	uint8_t finish_buff;				//完成打符标志位
} ContestEvent_t;

typedef __packed struct SelfFitness_struct
{
    uint8_t strategy_mode;           // 决策板模式
    uint16_t bullet;
    float vw;                        // 云台转速
	int16_t gimbal_yaw;			//0--0
	uint16_t gimbal_imu_yaw;
}SelfFitness_t;
 
// 底盘->电控 速度在电控那边也有阈值
typedef __packed struct Speed_struct 
{
    float vx;
    float vy;
    float vw;
		
	int16_t tripod_yaw;					//云台转向角度（世界坐标系）
    int16_t base_yaw;					//底盘转向（世界坐标系）
	
	//传递上主控信息（自身坐标以及角度）
	float	current_x;
	float current_y;
	float current_yaw;
	
	float current_vx;
	float current_vy;
} Speed_t;

typedef __packed struct Mode_struct
{
    uint8_t detect_mode;               // 0---无 1---自瞄 2---前哨 3---小符
    uint8_t navigate_mode;           // 导航模式 (0--不小陀螺听对应角度 1--小陀螺忽视w)
    uint8_t gimbal_mode;             	// 决策模式 (0--巡逻 1--过洞 2--预瞄)
	uint8_t electronic_mode;			//超电模式（0--不开超级电容 1--开超级电容）
}Mode_t;

typedef __packed struct EnemyPos_struct
{
    uint16_t hero_x;
    uint16_t hero_y;
    uint16_t engineer_x;
    uint16_t engineer_y;
    uint16_t infantry1_x;
    uint16_t infantry1_y;
    uint16_t infantry2_x;
    uint16_t infantry2_y;
    uint16_t sentry_x;
    uint16_t sentry_y;
}EnemyPos_t;



// 视觉->电控
typedef __packed struct VisionData_struct
{
    uint8_t header;
    uint8_t cmd_id;      
    uint8_t CRC8;

    Speed_t speed;   // 10
    Mode_t mode;   // 3Bytes
	
    uint16_t CRC16;
}Vision_t;

// 电控->视觉
typedef __packed struct CarData_struct
{
    uint8_t header;
    uint8_t cmd_id;      
    uint8_t CRC8;

    AllianceDetect_t ad;   // 4

    AllianceBlood_t ab;   // 14Bytes
    EnemyBlood_t eb;   // 14Bytes

    EnemyPos_t ep;   // 20
	ContestEvent_t ce;   // 9
    SelfFitness_t sf;   // 12	    
	uint16_t CRC16;
} CarData_t;

typedef __packed struct Strategy_struct
{
    Vision_t *vision;
    CarData_t *CarData;
	
	uint16_t	offline_cnt;
	uint16_t	offline_cnt_max;
	
	dev_work_state_t	work_state;
} Strategy_t;

bool strategy_send_data(void);
void strategy_update(Strategy_t *Strategy, uint8_t *rxBuf);
void strategy_get_robot_HP(Strategy_t *Strategy,judge_t *judge);
void Strategy_heart_beat(Strategy_t *Strategy);

extern Strategy_t Strategy;

#endif
