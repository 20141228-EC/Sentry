/**
 * @file        drv_io.c
 * @author      RobotPilots@2020
 * @Version     V1.0
 * @date        15-September-2020
 * @brief       Systick driver
 */
 
/* Includes ------------------------------------------------------------------*/
#include "drv_io.h"
#include "cmsis_os.h"
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
// 定义呼吸灯效果参数
#define MIN_BRIGHTNESS 0   // 最低亮度
#define MAX_BRIGHTNESS 10 // 最高亮度
#define BREATHE_STEP 1     // 亮度变化步长

uint8_t breatheDirection  = 1;
uint8_t currentBrightness = MIN_BRIGHTNESS;
uint8_t breatheStep       = BREATHE_STEP;


void LED_breath()
{
	if (breatheDirection == 1)
    {
		currentBrightness += breatheStep;

		if (currentBrightness >= MAX_BRIGHTNESS)
        {
			currentBrightness = MAX_BRIGHTNESS;
			breatheDirection = -1;
		}
	}
    else
    {
		currentBrightness -= breatheStep;

		if (currentBrightness <= MIN_BRIGHTNESS)
        {
			currentBrightness = MIN_BRIGHTNESS;
			breatheDirection = 1;
		}
	}
}

void Blue_setLedBrightness(uint8_t brightness)
{
    // 根据当前亮度设置LED引脚的高低电平
    if (brightness > 0)
    {
        LED_BLUE_ON();
        osDelay(brightness);
    }
    if (brightness < 100)
    {
        LED_BLUE_OFF();
        osDelay(MAX_BRIGHTNESS - brightness);
    }
}

void Red_setLedBrightness(uint8_t brightness)
{
    // 根据当前亮度设置LED引脚的高低电平
    if (brightness > 0)
    {
        LED_RED_ON();
        osDelay(brightness);
    }
    if (brightness < 100)
    {
        LED_RED_OFF();
        osDelay(MAX_BRIGHTNESS - brightness);
    }
}

void Green_setLedBrightness(uint8_t brightness)
{
    // 根据当前亮度设置LED引脚的高低电平
    if (brightness > 0)
    {
        LED_GREEN_ON();
        osDelay(brightness);
    }
    if (brightness < 100)
    {
        LED_GREEN_OFF();
        osDelay(MAX_BRIGHTNESS - brightness);
    }
}

void BLUE_LED_PWM()
{
	Blue_setLedBrightness(currentBrightness);
	
	LED_breath();
}


void RED_LED_PWM()
{
	Red_setLedBrightness(currentBrightness);
	
	LED_breath();
}

void GREEN_LED_PWM()
{
    Green_setLedBrightness(currentBrightness);
    
    LED_breath();
}

