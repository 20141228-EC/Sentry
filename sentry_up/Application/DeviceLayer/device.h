#ifndef __DEVICE_H
#define __DEVICE_H

/* Includes ------------------------------------------------------------------*/
#include "rp_config.h"
#include "cap.h"
#include "imu_sensor.h"
#include "rc_sensor.h"
#include "vision.h"
#include "motor.h"
#include "RM_motor.h"
#include "Slave.h"
#include "judge.h"
#include "WL_debug_uart.h"

/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct dev_list_struct 
{
	imu_sensor_t    *imu_sen;
	rc_sensor_t 	*rc_sen;
	Vision_t 		*vision_sen;
	cap_t			*cap_sen;
	
} dev_list_t;

extern dev_list_t dev_list;

/* Exported functions --------------------------------------------------------*/
void DEVICE_Init(void);

/* Servo functions */

#endif
