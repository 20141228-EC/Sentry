#ifndef WL_DEBUG_UART
#define WL_DEBUG_UART

/* Macro ------------------------------------------------------------------------------------------------------------------------------------------------- */
/*选择定义：队内绿色小主控定义 -- OUR_BOARD ，并根据需要定义 USE_USART3 或 USE_USART6 */
/*选择定义：C板定义 ------------- C_BOARD   ，并根据需要定义 USE_USART1 或 USE_USART6 */
/*选择定义：上交电容板定义 ------ SHANGJIAO_CAP_BOARD ，并根据需要定义 USE_USART1 */
/*选择定义：老电容板定义 -------- OLD_CAP_BOARD ，并根据需要定义 USE_USART2 */
/*选择定义：2025新电容板定义 ---- RP2025_CAP_BOARD ，并根据需要定义 USE_USART3 */
#define OLD_CAP_BOARD
#define USE_USART1
#define C_BOARD

/* Including Files -------------------------------------------------------------------------------------------------------------------------------------------------- */

#ifdef OUR_BOARD
#include "stm32f4xx_hal.h"
#endif

#ifdef C_BOARD
#include "stm32f4xx_hal.h"
#endif

#ifdef SHANGJIAO_CAP_BOARD
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_uart.h"
#include "stm32g4xx_hal_uart_ex.h"
// 要去这些.c/.h文件内定义HAL_UART_MODULE_ENABLED
#endif

#ifdef OLD_CAP_BOARD
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
//#include "stm32f4xx_hal_uart_ex.h"
// 要去这些.c/.h文件内定义HAL_UART_MODULE_ENABLED
#endif

#ifdef RP2025_CAP_BOARD
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_uart.h"
#include "stm32g4xx_hal_uart_ex.h"
// 要去这些.c/.h文件内定义HAL_UART_MODULE_ENABLED
#endif

/* Define Exported Variables Type ----------------------------------------------------------------------------------------------------------------------------------- */


/* Define Privated Variables Type ----------------------------------------------------------------------------------------------------------------------------------- */


/* Exported Variables Declarations ---------------------------------------------------------------------------------------------------------------------------------- */
/* 使用队内主控开始 */
#ifdef OUR_BOARD
  
  #ifdef USE_USART6
      extern UART_HandleTypeDef huart6;
  #endif
  
  #ifdef USE_USART3
      extern UART_HandleTypeDef huart3;
  #endif
  
#endif
/* 使用队内主控结束 */


/* 使用C板开始 */
#ifdef C_BOARD
  
  #ifdef USE_USART1
      extern UART_HandleTypeDef huart1;
  #endif
  
  #ifdef USE_USART6
      extern UART_HandleTypeDef huart6;
  #endif
  
#endif
/* 使用C板结束 */

/* 使用新电容板开始 */
#ifdef SHANGJIAO_CAP_BOARD
  
  #ifdef USE_USART1
      extern UART_HandleTypeDef huart1;
  #endif
  
#endif
/* 使用新电容板结束 */

/* 使用老电容板开始 */
#ifdef OLD_CAP_BOARD
  
  #ifdef USE_USART2
      extern UART_HandleTypeDef huart2;
  #endif
  
#endif
/* 使用老电容板结束 */

/* 使用2025新电容板电容板开始 */
#ifdef RP2025_CAP_BOARD
  
  #ifdef USE_USART3
      extern UART_HandleTypeDef huart3;
  #endif
  
#endif
/* 使用2025新电容板电容板结束 */

/* Exported Functions Declarations ---------------------------------------------------------------------------------------------------------------------------------- */
void WL_UART_Init(void);
void WL_UART_printf(char *format, ...);

#endif
