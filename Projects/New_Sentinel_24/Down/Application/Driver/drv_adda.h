#ifndef __DRV_ADDA_H
#define __DRV_ADDA_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Exported macro ------------------------------------------------------------*/
#define CAP_CUR_OUT_CHNL DAC_CHANNEL_1

/* Exported types ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void ADC_Init(void);
void DAC_Init(void);

#endif
