/**
 ******************************************************************************
 * @file        drv_uart.c
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
 
/* Includes ------------------------------------------------------------------*/
#include "drv_uart.h"
#include "string.h"
#include <stdio.h>
#include "stdarg.h"

/* Exported variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;

/* Private macro -------------------------------------------------------------*/
#define USART3_RX_DATA_FRAME_LEN	(18u)
#define USART3_RX_BUF_LEN			(USART3_RX_DATA_FRAME_LEN + 6u)

#define USART1_RX_BUF_LEN			200

/* Private function prototypes -----------------------------------------------*/
__weak void USART3_rxDataHandler(uint8_t *rxBuf);
__weak void USART1_rxDataHandler(uint8_t *rxBuf);
__weak void USART6_rxDataHandler(uint8_t *rxBuf);

/* Private variables ---------------------------------------------------------*/
uint8_t usart1_dma_rxbuf[USART1_RX_BUF_LEN];
uint8_t usart3_dma_rxbuf[2][USART3_RX_BUF_LEN];


volatile uint8_t usart1_dma_tx_over = 1;
/* Private typedef -----------------------------------------------------------*/



/* Private functions ---------------------------------------------------------*/

//	可以接多个rxHandler，然后在协议层回调 
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if(huart == &huart3)
	{
		/*接收到一帧数据时进入，已清除idle标志位（在UART_Handler里）
		此时DMA也还开启着，EN位未置零（不能写CR，也就是双缓冲地址不能配置）
		*/
		
		//置EN为0，让DMA处于可配置状态
		//同时接收到一帧数据也要将dma失能，才能重新配置dma
		__HAL_DMA_DISABLE(huart->hdmarx);
		
		
		if ((USART3_RX_BUF_LEN - huart->hdmarx->Instance->NDTR) == USART3_RX_DATA_FRAME_LEN)			//若为dbus数据帧
		{
			if(huart->hdmarx->Instance->CR & DMA_SxCR_CT)			//若此时为memory1
			{
				// 将当前目标内存设置为Memory0
				huart->hdmarx->Instance->CR &= ~(uint32_t)(DMA_SxCR_CT);
				
				//重新设定目标长度（重配dma）
				huart->hdmarx->Instance->NDTR = USART3_RX_BUF_LEN;
				
				//重启dma
				__HAL_DMA_ENABLE(huart->hdmarx);
				
				//处理数据
				USART3_rxDataHandler(usart3_dma_rxbuf[1]);
			}
			else
			{
				// 将当前目标内存设置为Memory1
				huart->hdmarx->Instance->CR |= (uint32_t)(DMA_SxCR_CT);
				
				//重新设定目标长度
				huart->hdmarx->Instance->NDTR = USART3_RX_BUF_LEN;
				
				//重启dma
				__HAL_DMA_ENABLE(huart->hdmarx);
				
				//处理数据
				USART3_rxDataHandler(usart3_dma_rxbuf[0]);
			}
				
		}
		else													//若不是数据帧，有干扰，则重置dma
		{
			//重新设定目标长度（重配dma）
				huart->hdmarx->Instance->NDTR = USART3_RX_BUF_LEN;
			
			//重启dma
				__HAL_DMA_ENABLE(huart->hdmarx);
			
		}
		
	}
	
	else if(huart == &huart1)
	{
		//停止DMA通道
		HAL_UART_DMAStop(&huart1);
		
		//处理数据
		USART1_rxDataHandler(usart1_dma_rxbuf);
		memset(usart1_dma_rxbuf, 0, USART1_RX_BUF_LEN);
		
		//重新开启接收
		HAL_UARTEx_ReceiveToIdle_DMA(&huart1,usart1_dma_rxbuf,USART1_RX_BUF_LEN);	
		
	}
	
}

void UART3_Init(void)
{
	//配置接收数据种类为IDLE
		huart3.ReceptionType = HAL_UART_RECEPTION_TOIDLE; 
	
	//配置接收事件为IDLE，在HAL_UART_IRQHandler里可以判断进入事件回调
	  huart3.RxEventType = HAL_UART_RXEVENT_IDLE; 
	
	//配置接收数据长度
	  huart3.RxXferSize    = USART3_RX_BUF_LEN; 
	
	//打开DMA_Receive
	  SET_BIT(huart3.Instance->CR3,USART_CR3_DMAR); 
	
	//置IDLE使能位
	  __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE); 
	
	//配置多缓冲区地址
	HAL_DMAEx_MultiBufferStart(huart3.hdmarx,\
								(uint32_t)&huart3.Instance->DR,\
								(uint32_t)usart3_dma_rxbuf[0],\
								(uint32_t)usart3_dma_rxbuf[1],\
								USART3_RX_BUF_LEN);
}

void UART1_Init(void)
{
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1,usart1_dma_rxbuf,USART1_RX_BUF_LEN);
}

//发送完成回调函数，不知道为啥不用这个就不能实现连续发送
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)
	{
  		 usart1_dma_tx_over = 1;
	}
}

//在协议层调用
__weak void USART3_rxDataHandler(uint8_t *rxBuf)
{
	
	
}

__weak void USART1_rxDataHandler(uint8_t *rxBuf)
{
	
	
}

__weak void USART6_rxDataHandler(uint8_t *rxBuf)
{
	
	
}
