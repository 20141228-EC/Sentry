#include "select_strategy.h"

uint8_t test_1,test_2,test_3,test4,test5;

uint8_t read_strategy_mode(void)
{
	uint8_t strategy_mode = 0;
	strategy_mode |= (HAL_GPIO_ReadPin(strategy_4_GPIO_Port, strategy_4_Pin) == GPIO_PIN_SET ? 1 : 0) << 2;
	strategy_mode |= (HAL_GPIO_ReadPin(strategy_3_GPIO_Port, strategy_3_Pin) == GPIO_PIN_SET ? 1 : 0) << 1;
	strategy_mode |= (HAL_GPIO_ReadPin(strategy_1_GPIO_Port, strategy_1_Pin) == GPIO_PIN_SET ? 1 : 0) << 0;
	
	test_1 = HAL_GPIO_ReadPin(strategy_1_GPIO_Port, strategy_1_Pin);
	test_2 = HAL_GPIO_ReadPin(strategy_4_GPIO_Port, strategy_4_Pin);
	test_3 = HAL_GPIO_ReadPin(strategy_3_GPIO_Port, strategy_3_Pin);
	test4 = HAL_GPIO_ReadPin(strategy_2_GPIO_Port, strategy_2_Pin);
	test5 = HAL_GPIO_ReadPin(strategy_5_GPIO_Port, strategy_5_Pin);
	
	return strategy_mode;
	
}