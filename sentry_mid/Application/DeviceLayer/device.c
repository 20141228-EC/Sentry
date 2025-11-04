/**
 * @file  device.c
 */
 
/* Includes ------------------------------------------------------------------*/
#include "device.h"

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
dev_list_t dev_list = {
	.imu_sen    = &imu_sensor,
	.rc_sen     = &rc_sensor,
//	.vision_sen = &vision,
	.cap_sen	= &cap,
//	.slave_dev	= &slave,
	
};

/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void DEVICE_Init(void)
{
	dev_list.rc_sen->init(dev_list.rc_sen);
	rm_motor_list_init();
//	dev_list.vision_sen->init(dev_list.vision_sen);
}
