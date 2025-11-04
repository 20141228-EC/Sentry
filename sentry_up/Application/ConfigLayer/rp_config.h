/**
 * @file        rp_config.c
 * @author      RobotPilots
 * @Version     v1.1
 * @brief       RobotPilots Robots' Configuration.
 * @update
 *              v1.0(9-September-2020)
 *              v1.1(7-November-2021)
 *                  1.优化设�?�类信息与结构体的变量定义，增加volatile/const关键�?
 *                  //2.将rp_config.h分成driver_config.h, device_config.h, user_config.h三个头文�?    
 */
#ifndef __RP_CONFIG_H
#define __RP_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stdbool.h"
#include "string.h"
// 驱动层配�?
#include "rp_driver_config.h"
// 设�?�层配置
#include "rp_device_config.h"
// 用户层配�?
#include "rp_user_config.h"

/************************************ 开�? *****************************************/
/*超电*/
#define CAP_ENABLE		 1
#define CAP_RECORD       0

/*裁判系统*/
#define JUDGE_ENABLE 	 0

/*过热保护*/
#define OVERHEAT_PROTECT 1

/*功率限制*/
#define POWER_LIMIT		0

/* 陀螺仪滤波 */
#define IMU_USE_MAHONY  1
#define IMU_USE_EKF     0

/************************************ 测试 *****************************************/

/*******************�?�?********************/

/*遥控器自瞄模�? */
#define  RC_SHOOT_AIM	0  /*�?瞄模�?*/
#define  SINGLE_MODE	0   
#define  KEEP_MODE	    0
#define SMALL_BUFF		1

#define SMALL_BUF_TEST		1
#define BIG_BUF_TEST			0

/*�?瞄信�?发�?*/
#define  RED	0
#define  BLUE	1


#define COLOR		RED

/*******************发射********************/

/*射速测�?*/
#define  BULLET_SPEED_TEST  1
/* 拨弹延时测试 */
#define  BULLET_DELAY_TEST  1

/********************测试巡逻模式和前哨站模�?********************** */
#define PC_CTRL_WHEEL  1
#define PC_CTRL_PATROl  0           
#define PC_CTRL_OUTPOST 0

#define SHOOT_TEST			0

/* Exported types ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/


#endif
