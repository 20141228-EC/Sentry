/**
 * @file        rp_math.c
 * @author      RobotPilots@2020
 * @Version     V1.0
 * @date        11-September-2020
 * @brief       RP Algorithm.
 */
 
/* Includes ------------------------------------------------------------------*/
#include "rp_math.h"
#include <math.h> 
#include <stdlib.h> 
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
int16_t RampInt(int16_t final, int16_t now, int16_t ramp)
{
	int32_t buffer = 0;
	
	buffer = final - now;
	if (buffer > 0)
	{
		if (buffer > ramp)
			now += ramp;
		else
			now += buffer;
	}
	else
	{
		if (buffer < -ramp)
			now += -ramp;
		else
			now += buffer;
	}

	return now;
}

float RampFloat(float final, float now, float ramp)
{
	float buffer = 0;
	
	buffer = final - now;
	if (buffer > 0)
	{
		if (buffer > ramp)
			now += ramp;
		else
			now += buffer;
	}
	else
	{
		if (buffer < -ramp)
			now += -ramp;
		else
			now += buffer;
	}

	return now;	
}

float DeathZoom(float input, float center, float death)
{
	if(fabs(input - center) < death)
		return center;
	return input;
}

float Low_Pass_Fliter(float data , float last_data , float a)
{
	return a*data + (1 - a) * last_data;
}



int16_t SmoothAccelerationUpdate(struct SmoothAcceleration *smooth_acc) 
{
    // 计算速度变化的增量
    int16_t speed_diff = smooth_acc->target_speed - smooth_acc->current_speed;

    // 限制速度变化的增量不超过给定的加速度
    float max_speed_change = smooth_acc->max_acceleration;
    if (abs(speed_diff) > max_speed_change) {
        speed_diff = (speed_diff > 0) ? max_speed_change : -max_speed_change;
    }

    // 更新当前速度
    smooth_acc->current_speed += speed_diff;

    return smooth_acc->current_speed;
}

