#ifndef __LED_TASK
#define __LED_TASK

#include "cmsis_os.h"
#include "main.h"

typedef struct{
	float gx, gy, gz;       //陀螺仪数据
	float ax, ay, az;       //加速度计数据
	float pitch, roll,yaw;  //姿态
	float pitch_rate, roll_rate,yaw_rate;//弧度制
}raw_and_EulerAngle_t;

void StartLEDTask(void const * argument);

//extern IWDG_HandleTypeDef hiwdg;
#endif
