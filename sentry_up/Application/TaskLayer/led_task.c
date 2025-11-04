#include "led_task.h"

void Buzzer_Note(void)
{
	  HAL_GPIO_WritePin(Buzzer_GPIO_Port,Buzzer_Pin,1);
	 
	  osDelay(1);
	  
	  HAL_GPIO_WritePin(Buzzer_GPIO_Port,Buzzer_Pin,0);
	  
	  osDelay(1);
}

void StartLEDTask(void const * argument)
{
	
	HAL_GPIO_WritePin(GPIOH, LED1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOH, LED2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOH, LED3_Pin, GPIO_PIN_RESET);
	
	
  for(;;)
  {
//		 if(judge.info->shoot_data.bullet_speed>=25)
//		{
//			robot.booster->info->buzzer_cnt = 1;
//		}
//		if(robot.booster->info->buzzer_cnt == 1)
//		{
//			Buzzer_Note();
//		}
	  if(slave.info->rx_info->bullet_speed >= 25)
	  {
		  robot.booster->info->buzzer_cnt = 1;
	  }
	  if(robot.booster->info->buzzer_cnt == 1)
	  {
		  Buzzer_Note();
	  }
		osDelay(1);
  }
}


