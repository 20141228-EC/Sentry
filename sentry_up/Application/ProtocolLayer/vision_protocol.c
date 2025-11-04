/* Includes ------------------------------------------------------------------*/
#include "vision_protocol.h"

void USART1_rxDataHandler(uint8_t *rxBuf)
{
	vision.update(&vision, rxBuf);
	vision.check(&vision);

//	slave.info->tx_info->ctrl_flag[3] = vision.info->rx_info->is_find_target;
}
