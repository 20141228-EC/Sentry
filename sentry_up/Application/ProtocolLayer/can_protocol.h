/**
 ******************************************************************************
 * @file    can_protocol.h
 * @brief   CAN通信协议层
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
/* 下主控CAN ID */
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

#define GAME_STATUS_ID 			0x003	//	比赛状态	剩余时间	剩余弹量	剩余金币	己方颜色
#define GAME_ROBOT_HP_ID 		0x004	//	己方血量	敌方血量
#define SHOOT_DATA_ID 			0x008	//	弹丸速度	
#define RADAR_POS_COOR_ID		0x101	//	雷达标点坐标
#define GAME_ROBOT_STATUS_ID 	0x102	//	机器状态

/*CAN1*/
#define NAV_SEND_UP_INFO 0x006
#define NAV_SEND_LADAR_INFO	0x007



/* Exported functions --------------------------------------------------------*/
void CAN1_rxDataHandler(uint32_t canId, uint8_t *rxBuf);
void CAN2_rxDataHandler(uint32_t canId, uint8_t *rxBuf);
void CAN_SendAll(void);
void CAN_SendAllZero(void);
void CAN_LINK(void);
void cap_data_send(uint8_t can_num);
void CAN_LINKZero();


#endif
