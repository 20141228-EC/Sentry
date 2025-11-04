#include "led_task.h"
#include "can_protocol.h"
#include "rp_math.h"

#include "imu_sensor.h"

uint8_t size;

void StartLEDTask(void const * argument)
{
	
	HAL_GPIO_WritePin(GPIOH, LED1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOH, LED2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOH, LED3_Pin, GPIO_PIN_RESET);
	
	
  for(;;)
  {
		size = sizeof(master_game_robot_status_t);
	  osDelay(1);

  }
}
