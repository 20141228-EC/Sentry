#ifndef __BOOSTER_H
#define __BOOSTER_H


//射击速度
//#define FRIC_25          6450

#define FRIC_25				6000
//射击频率(还需测试)
#define DIAL_1_SPEED 		270  // 1发/s
#define DIAL_5_SPEED 		1350 
#define DIAL_8_SPEED		2160
#define DIAL_10_SPEED 		2700 // 10发/s
#define DIAL_20_SPEED 		4860 // 18发/s 
#define DIAL_25_SPEED 		6750 // 25发/s

#define ONE_SHOOT_ANGLE 	36864	//36864


#include "device.h"
#include "rp_config.h"

typedef struct
{
	uint16_t shoot_num;
	
	uint8_t delay_40;		//	40	-	45
	uint8_t delay_45;		//	45	-	50
	uint8_t delay_50;		//	50	-	55
	uint8_t delay_55;		//	55	-	60
	uint8_t delay_60;		//	60	-	65
	uint8_t delay_65;		//	65	-	70
	uint8_t delay_70;		//	70	-	75
	uint8_t delay_75;		//	75	-	80
	uint8_t delay_80;		//	80	-	85
	uint8_t delay_85;		//	85	-	90
	uint8_t delay_90;		//	90	-	95
	uint8_t delay_95;		//	95	-	100
	uint8_t delay_100;	//	100	-	105
	uint8_t delay_105;	//	105	-	110
	uint8_t delay_110;	//	110	-	115
	uint8_t delay_115;	//	115	-	120
	uint8_t delay_120;	//	120	-	125
	uint8_t delay_125;	//	125	-	130
	uint8_t delay_130;	//	130	-	135
	uint8_t delay_135;	//	135	-	140
	uint8_t delay_140;	//	140	-	145

	float mean;
	float variance;
	
}shoot_delay_data_t;

typedef enum booster_mode_enum{
	READY,
	SINGEL,
	KEEP,
	SLEEP,
}booster_mode_e;

typedef struct booster_info_struct
{
    uint16_t shoot_v;
    uint16_t shoot_f;

    int32_t angle;			//经过减速电机之后的角度
	float bullet_speed;
	float last_bullet_speed;

	float limit_speed;
	
	uint16_t	buzzer_cnt;	
	
	uint8_t		shoot_start;
	uint16_t	shoot_cnt;
	
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

void Booster_CTRL_PC(booster_t *booster);
void Static_shoot(void);
void booster_clear_bullet(booster_t *booster);

extern booster_t booster;
extern void Measure_Delay_Launch(void);


#endif