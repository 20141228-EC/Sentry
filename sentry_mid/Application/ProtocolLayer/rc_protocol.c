/**
 * @file        rc_protocol.c
 * @author      RobotPilots
 * @Version     v1.1
 * @brief       DT7&DR16 Rc Protocol.
 * @update
 *              v1.0(9-September-2020)
 *              v1.1(24-October-2021)
 *                  1.修改rc_potocol.c/.h->rc_protocol.c/.h 
 */
 
/* Includes ------------------------------------------------------------------*/
#include "rc_protocol.h"
#include "rp_math.h"

#include "rc_sensor.h"


/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

extern uint32_t micros(void);
uint32_t tt1, tt2, ttp1;
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 *	@brief	遥控器数据解析协议
 */
void rc_sensor_update(rc_sensor_t *rc_sen, uint8_t *rxBuf)
{
	rc_sensor_info_t *rc_info = rc_sen->info;
	/* 遥控器 */
	rc_info->ch0 = (rxBuf[0] | rxBuf[1] << 8) & 0x07FF;
	rc_info->ch0 -= 1024;
	rc_info->ch1 = (rxBuf[1] >> 3 | rxBuf[2] << 5) & 0x07FF;
	rc_info->ch1 -= 1024;
	rc_info->ch2 = (rxBuf[2] >> 6 | rxBuf[3] << 2 | rxBuf[4] << 10) & 0x07FF;
	rc_info->ch2 -= 1024;
	rc_info->ch3 = (rxBuf[4] >> 1 | rxBuf[5] << 7) & 0x07FF;
	rc_info->ch3 -= 1024;

	rc_info->thumbwheel.value = ((int16_t)rxBuf[16] | ((int16_t)rxBuf[17] << 8)) & 0x07ff;
	rc_info->thumbwheel.value -= 1024;

	rc_info->s1.value = ((rxBuf[5] >> 4) & 0x000C) >> 2;
	rc_info->s2.value = (rxBuf[5] >> 4) & 0x0003;	
	
	/* 鼠标 */
	rc_info->mouse_vx = rxBuf[6]  | (rxBuf[7 ] << 8);
	rc_info->mouse_vy = rxBuf[8]  | (rxBuf[9 ] << 8);
	rc_info->mouse_vz = rxBuf[10] | (rxBuf[11] << 8);
  rc_info->mouse_btn_l.value = rxBuf[12] & 0x01;
  rc_info->mouse_btn_r.value = rxBuf[13] & 0x01;
  
  /* 键盘 */
  //16个键用一个uint16_t的数来保存16个bit
  //最后按位与去判断键值
	rc_info->key_v   =  rxBuf[14] | (rxBuf[15] << 8);
	
  rc_info->W.value = 	KEY_PRESSED_W;
  rc_info->S.value =  KEY_PRESSED_S;
  rc_info->A.value = 	KEY_PRESSED_A;
  rc_info->D.value = 	KEY_PRESSED_D;
  rc_info->Shift.value = 	KEY_PRESSED_SHIFT;
  rc_info->Ctrl.value  = 	KEY_PRESSED_CTRL;
  rc_info->Q.value = 	KEY_PRESSED_Q;
  rc_info->E.value = 	KEY_PRESSED_E;
  rc_info->R.value = 	KEY_PRESSED_R;
  rc_info->F.value = 	KEY_PRESSED_F;
  rc_info->G.value = 	KEY_PRESSED_G;
  rc_info->Z.value = 	KEY_PRESSED_Z;
  rc_info->X.value = 	KEY_PRESSED_X;
  rc_info->C.value = 	KEY_PRESSED_C;
  rc_info->V.value = 	KEY_PRESSED_V;
  rc_info->B.value = 	KEY_PRESSED_B;
	
	rc_info->offline_cnt = 0;
	tt1 = tt2;
	tt2 = micros();
	ttp1 = tt2 - tt1;
	
}



/**
 *	@brief	在串口2中解析遥控数据协议
 */
void USART3_rxDataHandler(uint8_t *rxBuf)
{
	// 更新遥控数据
	rc_sensor.update(&rc_sensor, rxBuf);
	rc_sensor.check(&rc_sensor);
	
  //鼠标数据解析
	rc_interrupt_update(&rc_sensor);

  //拇指轮数据解析
  rc_sensor_thumbwheel_step(&rc_sensor);   //24.12.11已测试

	rc_sensor.info->offline_cnt = 0;
	
}
