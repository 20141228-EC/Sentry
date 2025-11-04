/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
 * @Files:WL_debug_uart.c/h
 * 
 * @Author: Ye Jinyi
 * 
 * @First  Edit Date: 2024.11.28
 * @Latest Edit Date: 2024.11.29
 * 
 * @illustrate: 配套vofa+使用，也可以自己用别的
 *              使用方法，第一步：在WL_debug_uart.h按需求宏定义
 *                        第二步：在main.c调用WL_UART_Init(void)初始化
 *                        第三步：在某个任务调用WL_UART_printf(char *format, ...)，任务执行频率决定发送频率
 * 
 * @attention: 不用自己去配串口，加上这两个文件就可以用。如果配了就要把配的注释掉，否则会重复定义。
 *             波特率为115200，也可以自己改。
 *             使用vofa+就要用软件内的FireWater数据协议，在软件内点？就可以查看怎么用。
 * 
 *-------------------------------------------------------------------------------------------------------------------------------------------------------------------*/

/* Including Files  ------------------------------------------------------------------------------------------------------------------------------------------------- */
#include "WL_debug_uart.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* Separated Code Blocks ------------------------------------------------------------------------------------------------------------------------------------------------- */
/* 使用队内主控开始 */
#ifdef OUR_BOARD
  
  /* 使用串口六部分开始 */
  #ifdef USE_USART6
  
  UART_HandleTypeDef huart6;
  
  void WL_UART_Init(void)
  {
      __HAL_RCC_USART6_CLK_ENABLE();
      __HAL_RCC_GPIOC_CLK_ENABLE();
      /**USART6 GPIO Configuration
      PC6     ------> USART6_TX
      PC7     ------> USART6_RX
      */
      GPIO_InitTypeDef GPIO_InitStruct = {0};
      GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
      
      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    
    huart6.Instance = USART6;
    huart6.Init.BaudRate = 115200;
    huart6.Init.WordLength = UART_WORDLENGTH_8B;
    huart6.Init.StopBits = UART_STOPBITS_1;
    huart6.Init.Parity = UART_PARITY_NONE;
    huart6.Init.Mode = UART_MODE_TX;
    huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart6.Init.OverSampling = UART_OVERSAMPLING_16;
    
    HAL_UART_Init(&huart6);
    
  }
  
  void WL_UART_printf(char *format, ...)
  {
      char String[100];
      va_list arg;
      va_start(arg, format);
      vsprintf(String, format, arg);
      va_end(arg);
      
      HAL_UART_Transmit(&huart6, (uint8_t *)String, strlen(String), 1000);
  }
  #endif
  /* 使用串口六部分结束 */
  
  /* 使用串口三部分开始 */
  #ifdef USE_USART3
  
  UART_HandleTypeDef huart3;
  
  void WL_UART_Init(void)
  {
      __HAL_RCC_USART3_CLK_ENABLE();
      __HAL_RCC_GPIOB_CLK_ENABLE();
      /**USART3 GPIO Configuration
      PB10     ------> USART3_TX
      PB11     ------> USART3_RX
      */
      GPIO_InitTypeDef GPIO_InitStruct = {0};
      GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
      
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      
    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    
    HAL_UART_Init(&huart3);
    
  }
  
  void WL_UART_printf(char *format, ...)
  {
      char String[100];
      va_list arg;
      va_start(arg, format);
      vsprintf(String, format, arg);
      va_end(arg);
      
      HAL_UART_Transmit(&huart3, (uint8_t *)String, strlen(String), 1000);
  }
  #endif
  /* 使用串口三部分结束 */
  
#endif
/* 使用队内主控结束 */


/* 使用C板开始 */
#ifdef C_BOARD
  
  /* 使用串口一部分开始 */
  #ifdef USE_USART1
  
  UART_HandleTypeDef huart1;
  
  void WL_UART_Init(void)
  {
      __HAL_RCC_USART1_CLK_ENABLE();
      __HAL_RCC_GPIOB_CLK_ENABLE();
      __HAL_RCC_GPIOA_CLK_ENABLE();
      
      /**USART1 GPIO Configuration
      PB7     ------> USART1_RX
      PA9     ------> USART1_TX
      */
      GPIO_InitTypeDef GPIO_InitStruct = {0};
      GPIO_InitStruct.Pin = GPIO_PIN_7;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
      
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      
      GPIO_InitStruct.Pin = GPIO_PIN_9;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
      
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    HAL_UART_Init(&huart1);
  }
  
  void WL_UART_printf(char *format, ...)
  {
      char String[100];
      va_list arg;
      va_start(arg, format);
      vsprintf(String, format, arg);
      va_end(arg);
      
      HAL_UART_Transmit(&huart1, (uint8_t *)String, strlen(String), 1000);
  }
  
  #endif
  /* 使用串口一部分结束 */
  
  /* 使用串口六部分开始 */
  #ifdef USE_USART6
  
  UART_HandleTypeDef huart6;
  
  void WL_UART_Init(void)
  {
      __HAL_RCC_USART6_CLK_ENABLE();
      __HAL_RCC_GPIOG_CLK_ENABLE();
      
      /**USART6 GPIO Configuration
      PG14     ------> USART6_TX
      PG9     ------> USART6_RX
      */
      GPIO_InitTypeDef GPIO_InitStruct = {0};
      GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_9;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
      HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
      
      
    huart6.Instance = USART6;
    huart6.Init.BaudRate = 115200;
    huart6.Init.WordLength = UART_WORDLENGTH_8B;
    huart6.Init.StopBits = UART_STOPBITS_1;
    huart6.Init.Parity = UART_PARITY_NONE;
    huart6.Init.Mode = UART_MODE_TX;
    huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart6.Init.OverSampling = UART_OVERSAMPLING_16;
    
    HAL_UART_Init(&huart6);
    
  }
  
  void WL_UART_printf(char *format, ...)
  {
      char String[100];
      va_list arg;
      va_start(arg, format);
      vsprintf(String, format, arg);
      va_end(arg);
      
      HAL_UART_Transmit(&huart6, (uint8_t *)String, strlen(String), 1000);
  }
  
  #endif
  /* 使用串口六部分结束 */
  
#endif
/* 使用C板结束 */

/* 使用上交电容板开始 */
#ifdef SHANGJIAO_CAP_BOARD
  
  /* 使用串口一部分开始 */
  #ifdef USE_USART1
  
  UART_HandleTypeDef huart1;
  DMA_HandleTypeDef hdma_usart1_tx;
  
  void WL_UART_Init(void)
  {
    /* DMA1_Channel6_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
    
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
      
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
    
    __HAL_RCC_USART1_CLK_ENABLE();
    
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PB6     ------> USART1_TX
    PB7     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 3000000;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  
  HAL_UART_Init(&huart1);
  
  HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8);
  
  HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8);
  
  HAL_UARTEx_DisableFifoMode(&huart1);
  
    /* USART1 DMA Init */
    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA1_Channel6;
    hdma_usart1_tx.Init.Request = DMA_REQUEST_USART1_TX;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    
    HAL_DMA_Init(&hdma_usart1_tx);

    __HAL_LINKDMA(&huart1,hdmatx,hdma_usart1_tx);
    
    
    // 这里原本需要打开串口中断，但是为了时序要求就不开了
  }
  
  void WL_UART_printf(char *format, ...)
  {
      char String[100];
      va_list arg;
      va_start(arg, format);
      vsprintf(String, format, arg);
      va_end(arg);
      
      huart1.gState = HAL_UART_STATE_READY;         // 加了这一行才能发，因为串口中断没使能，导致dma传输太快发生数据帧错误中断但标志位无法被清除，使huart1.gState一直busy，这里手动ready即可
      HAL_UART_Transmit_DMA(&huart1, (uint8_t *)String, strlen(String));

  }
  
  
  #endif
  /* 使用串口一部分结束 */
  
#endif
/* 使用上交电容板结束 */

/* 使用老电容板开始 */
#ifdef OLD_CAP_BOARD
  
  /* 使用串口二部分开始 */
  #ifdef USE_USART2
  
  UART_HandleTypeDef huart2;
  DMA_HandleTypeDef hdma_usart2_tx;
  
  void WL_UART_Init(void)
  {
    /* DMA1_Channel7_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
    
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PB3     ------> USART2_TX
    PB4     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
    
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 2000000;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  
  HAL_UART_Init(&huart2);
  
    /* USART2 DMA Init */
    /* USART2_TX Init */
    hdma_usart2_tx.Instance = DMA1_Channel7;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_HIGH;
    
    HAL_DMA_Init(&hdma_usart2_tx);

    __HAL_LINKDMA(&huart2,hdmatx,hdma_usart2_tx);
    
    
    // 这里原本需要打开串口中断，但是为了时序要求就不开了
  }
  
  /**
  * @brief 手动开DMA的同时需要手动处理DMA的中断
  */
  void DMA1_Channel7_IRQHandler(void)
  {
    /* USER CODE BEGIN DMA1_Channel2_IRQn 0 */
    
    /* USER CODE END DMA1_Channel2_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_usart2_tx);
    /* USER CODE BEGIN DMA1_Channel2_IRQn 1 */
      
      
    /* USER CODE END DMA1_Channel2_IRQn 1 */
  }
  
  
  void WL_UART_printf(char *format, ...) 
  {
    char String[256]; // 根据实际需求调整
    va_list arg;
    va_start(arg, format);
    int len = vsnprintf(String, sizeof(String), format, arg);
    va_end(arg);
    
    huart2.gState = HAL_UART_STATE_READY;         // 加了这一行才能发，因为串口中断没使能，导致dma传输太快发生数据帧错误中断但标志位无法被清除，使huart1.gState一直busy，这里手动ready即可
    HAL_UART_Transmit_DMA(&huart2, (uint8_t *)String, (uint16_t)len);
  }
  
  
  #endif
  /* 使用串口二部分结束 */
  
  
#endif
/* 使用老电容板结束 */

/* 使用2025新电容板结束 */
#ifdef RP2025_CAP_BOARD
  
  /* 使用串口三部分开始 */
  #ifdef USE_USART3
  
  UART_HandleTypeDef huart3;
  DMA_HandleTypeDef hdma_usart3_tx;
  
  void WL_UART_Init(void)
  {
    /* DMA1_Channel6_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
    
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
      
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);
    
    __HAL_RCC_USART3_CLK_ENABLE();
    
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 3000000;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  
  HAL_UART_Init(&huart3);
  
  HAL_UARTEx_SetTxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8);
  
  HAL_UARTEx_SetRxFifoThreshold(&huart3, UART_TXFIFO_THRESHOLD_1_8);
  
  HAL_UARTEx_DisableFifoMode(&huart3);
  
    /* USART3 DMA Init */
    /* USART3_TX Init */
    hdma_usart3_tx.Instance = DMA1_Channel6;
    hdma_usart3_tx.Init.Request = DMA_REQUEST_USART3_TX;
    hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart3_tx.Init.Mode = DMA_NORMAL;
    hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;
    
    HAL_DMA_Init(&hdma_usart3_tx);

    __HAL_LINKDMA(&huart3,hdmatx,hdma_usart3_tx);
    
    
    // 这里原本需要打开串口中断，但是为了时序要求就不开了
  }
  
  void WL_UART_printf(char *format, ...)
  {
      char String[256];     // 根据实际需求调整
      va_list arg;
      va_start(arg, format);
      int len = vsnprintf(String, sizeof(String), format, arg);
      va_end(arg);
      
      huart3.gState = HAL_UART_STATE_READY;         // 加了这一行才能发，因为串口中断没使能，导致dma传输太快发生数据帧错误中断但标志位无法被清除，使huart1.gState一直busy，这里手动ready即可
      HAL_UART_Transmit_DMA(&huart3, (uint8_t *)String, (uint16_t)len);

  }
  
  
  #endif
  /* 使用串口三部分结束 */
  
#endif
/* 使用2025新电容板结束 */

