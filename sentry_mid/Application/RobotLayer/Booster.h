#ifndef __BOOSTER_H
#define __BOOSTER_H

//射击速度
#define FRIC_25          6750

//射击频率(还需测试)
#define DIAL_1_SPEED 		270  // 1发/s
#define DIAL_5_SPEED 		1350 
#define DIAL_10_SPEED 		2700 // 10发/s
#define DIAL_20_SPEED 		4860 // 18发/s 
#define DIAL_25_SPEED 		6750 // 25发/s

#define ONE_SHOOT_ANGLE 	36860	//36864


#include "device.h"
#include "rp_config.h"


typedef enum booster_mode_enum{
	READY,
	SINGEL,
	KEEP,
	SLEEP,
    CLEAR,    // 清空热量
}booster_mode_e;

typedef struct booster_info_struct
{
    uint16_t shoot_v;
    uint16_t shoot_f;

    int32_t angle;			//经过减速电机之后的角度


}booster_info_t;

typedef struct Booster_struct
{
    booster_mode_e  mode;
    booster_mode_e  last_mode;
    booster_info_t  *info;

    rm_motor_t      *L_FRIC_motor;
    rm_motor_t      *R_FRIC_motor;
    rm_motor_t      *DIAL_motor;

    dev_work_state_t	work_state;

    uint8_t ready_flag;
}booster_t;

extern booster_t booster;

#endif