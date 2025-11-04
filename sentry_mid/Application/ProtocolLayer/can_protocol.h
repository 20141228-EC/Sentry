/*
 * @Author: lion
 * @Date: 2025-07-16 19:25:00
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-07-26 17:46:58
 * @FilePath: \sentry_mid\Application\ProtocolLayer\can_protocol.h
 * @Description: 
 * 
 */
/**
 ******************************************************************************
 * @file    can_protocol.h
 * @brief   CAN?????????
 ******************************************************************************
 * @attention
 *
 * Copyright 2024 RobotPilots
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_PROTOCOL_H
#define __CAN_PROTOCOL_H

/* Includes ------------------------------------------------------------------*/
#include "driver.h"
#include "device.h"

/* Exported macro ------------------------------------------------------------*/
/* ?????CAN ID */
#define SLAVE_TX_ID 
#define SLAVE_RX_ID 

/*CAN1*/
#define CHASSIS_CAN_ID_LF			 RM3508_CAN_ID_201
#define CHASSIS_CAN_ID_RF			 RM3508_CAN_ID_202
#define CHASSIS_CAN_ID_LB			 RM3508_CAN_ID_203
#define CHASSIS_CAN_ID_RB			 RM3508_CAN_ID_204

/*CAN2*/
#define GIMBAL_CAN_ID_PITCH			 GM6020_CAN_ID_205
#define GIMBAL_CAN_ID_YAW			 GM6020_CAN_ID_206
#define FRIC_CAN_ID_LEFT		     RM3508_CAN_ID_201
#define FRIC_CAN_ID_RIGHT			 RM3508_CAN_ID_202
#define LAUNCH_CAN_ID_DIAL	         RM2006_CAN_ID_203

/* Exported functions --------------------------------------------------------*/
void CAN1_rxDataHandler(uint32_t canId, uint8_t *rxBuf);
void CAN2_rxDataHandler(uint32_t canId, uint8_t *rxBuf);
void CAN_SendAll(void);
void CAN_SendAllZero(void);
void cap_data_send(uint8_t can_num);

#endif
