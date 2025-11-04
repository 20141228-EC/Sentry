/**
 ******************************************************************************
 * @file        drv_uart.h
 * @author      RobotPilots@2020
 * @brief       UART Driver Package(Based on HAL).
 ******************************************************************************
 * @attention
 * 
 * Copyright 2020 RobotPilots
 * 
 * @Version     V1.0
 * @date        29-9-2024
 ******************************************************************************
 */
#ifndef __DRV_UART_H
#define __DRV_UART_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
int myprintf(const char *format,...);
void UART1_Init(void);
void UART3_Init(void);
void UART6_Init(void);

#endif
