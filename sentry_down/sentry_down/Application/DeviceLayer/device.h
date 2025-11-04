#ifndef __DEVICE_H
#define __DEVICE_H

/* Includes ------------------------------------------------------------------*/
#include "rp_config.h"

#include "cap.h"
#include "judge.h"
#include "motor.h"
#include "master.h"	
#include "RM_motor.h"
#include "imu_sensor.h"

/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct dev_list_struct 
{
	imu_sensor_t	*imu_sen;
	cap_t					*cap_sen;
	
} dev_list_t;

extern dev_list_t dev_list;

/* Exported functions --------------------------------------------------------*/
void DEVICE_Init(void);

/* Servo functions */

#endif
