#ifndef __DEVICE_H
#define __DEVICE_H

/* Includes ------------------------------------------------------------------*/
#include "rp_config.h"
#include "cap.h"
#include "imu_sensor.h"
#include "rc_sensor.h"
#include "motor.h"
#include "RM_motor.h"
//#include "ladar_sensor.h"
#include "slave.h"
#include "master.h"	
#include "judge.h"
#include "Strategy.h"
#include "KT_motor.h"
#include "select_strategy.h"

/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct dev_list_struct 
{
	imu_sensor_t    *imu_sen;
	rc_sensor_t 	*rc_sen;
//	Vision_t 		*vision_sen;
//	judge_t			*judge_sen;
	cap_t			*cap_sen;
	
} dev_list_t;

extern dev_list_t dev_list;

/* Exported functions --------------------------------------------------------*/
void DEVICE_Init(void);

/* Servo functions */

#endif
