/**
 * @file        drv_tim.c
 * @author      RobotPilots@2020
 * @Version     V1.0
 * @date        23-August-2020
 * @brief       TIMER Driver Package(Based on HAL).
 */

/* Includes ------------------------------------------------------------------*/
#include "drv_tim.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim11;


/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void FRICTION_PwmOut(int16_t pwm1, int16_t pwm2);
void COVER_PwmOut(int16_t pwm);
void BUZZ_Freq(uint16_t frq);

/* Exported functions --------------------------------------------------------*/
void PWM_Init(void)
{
	HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1);
	BUZZ_Freq(50);
}

void BUZZ_Freq(uint16_t frq)
{
	BUZZ_PWM = frq;
}

