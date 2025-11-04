#ifndef __STRATEGY_H
#define __STRATEGY_H

#include "driver.h"
#include "crc.h"
#include "device.h"
#include "rp_config.h"

// 云台->底盘   记得区分前后相机
typedef __packed struct AllianceDetect_struct
{
    uint8_t is_hero;
    uint8_t is_engineer;
    uint8_t is_infantry1;
    uint8_t is_infantry2;
}AllianceDetect_t ;

typedef __packed struct EnemyDetect_struct
 {
    uint8_t is_hero;
    uint8_t is_engineer;
    uint8_t is_infantry1;
    uint8_t is_infantry2;
    uint8_t is_sentry;
    uint8_t is_outpost;
    uint8_t is_basepost;
} EnemyDetect_t;

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
    uint8_t game_start;              // 0-prepare 1-start
	uint8_t game_perpare;			//0为三分钟准备阶段，1为15秒准备倒计时
    uint16_t game_time;
    uint8_t enemy_color;                // 0-blue 1-red
    uint16_t remain_money;
	uint8_t darts;
	uint8_t darted;
    uint8_t points;
} ContestEvent_t;

typedef __packed struct SelfFitness_struct
{
    uint8_t detect_mode;             // 云台模式 (1-巡逻 2-击打)
    uint8_t strategy_mode;           // 决策模式（取决于云台手）
    uint16_t my_defend_gain;   
    uint16_t my_attack_gain;
    uint16_t bullet;                // 可发射子弹数
	uint8_t enegy;
}SelfFitness_t;

// 底盘->云台
typedef __packed struct Hit_struct {
    uint8_t hit_hero;                 // 理论上可以设计一个最终击打目标，但可以先全部交给自瞄
    uint8_t hit_engineer;
    uint8_t hit_infantry1;
    uint8_t hit_infantry2;
    uint8_t hit_sentry;
    uint8_t hit_outpost;
    uint8_t hit_basepost; 
} Hit_t;
 
// 底盘->电控 速度在电控那边也有阈值
typedef __packed struct Speed_struct 
{
    int16_t vx;
    int16_t vy;
    int16_t vw;
} Speed_t;

typedef __packed struct Mode_struct
{
    uint8_t detect_mode;              // 底盘最终决定云台模式 (0-保持 1-巡逻 2-击打) 
    uint8_t navigate_mode;            // 导航模式 (0-停止 1-寻路)
    uint8_t strategy_mode;            // 决策模式 (0-守家 1-进攻 2-逃跑)
}Mode_t;

// 底盘->裁判系统
typedef __packed struct Supply_struct
{
    uint8_t add_bullet;               // 补弹这个操作是瞬间执行并完成的，放心用，跟电控协商每次补150颗弹
    uint8_t add_blood;                // 加血这个操作可能不会主动使用，因为等待时间比较长
    uint8_t resurgence;               // 预期想法是死了直接复活
} Supply_t;

typedef __packed struct AlliancePos_struct
{
    uint16_t hero_x;
    uint16_t hero_y;
    uint16_t engineer_x;
    uint16_t engineer_y;
    uint16_t infantry1_x;
    uint16_t infantry1_y;
    uint16_t infantry2_x;
    uint16_t infantry2_y;
} AlliancePos_t;

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

typedef __packed struct Vulnerability_struct
{
    uint8_t smash_hero;        // 0-无 1-有
    uint8_t smash_engineer;
    uint8_t smash_infantry1;
    uint8_t smash_infantry2;
    uint8_t smash_sentry;
 } Vulnerability_t;



// 视觉->电控
typedef __packed struct VisionData_struct
{
    uint8_t header;
    uint8_t cmd_id;      // 没diao用吧
    uint8_t CRC8;

    Hit_t hit;   // 7Bytes
    Speed_t speed;   // 4Bytes
    Mode_t mode;   // 3Bytes
    Supply_t supply;   //3Bytes

    uint16_t CRC16;
}Vision_t;

// 电控->视觉
typedef __packed struct CarData_struct
{
    uint8_t header;
    uint8_t cmd_id;      // 没diao用吧
    uint8_t CRC8;

    AllianceDetect_t ad;   // 6Bytes
    EnemyDetect_t ed;   // 7Bytes

    AllianceBlood_t ab;   // 14Bytes
    EnemyBlood_t eb;   // 14Bytes
    ContestEvent_t ce;   // 8Bytes
    SelfFitness_t sf;   // 8Bytes

    AlliancePos_t ap;   // 16Bytes
    EnemyPos_t ep;   // 18Bytes
    Vulnerability_t vul;   // 6Bytes
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

void strategy_send_data(void);
void strategy_update(Strategy_t *Strategy, uint8_t *rxBuf);
void strategy_get_robot_HP(Strategy_t *Strategy,judge_t *judge);
void Strategy_heart_beat(Strategy_t *Strategy);

extern Strategy_t Strategy;

#endif
