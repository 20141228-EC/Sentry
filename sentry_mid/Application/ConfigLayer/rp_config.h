/**
 * @file        rp_config.c
 * @author      RobotPilots
 * @Version     v1.1
 * @brief       RobotPilots Robots' Configuration.
 * @update
 *              v1.0(9-September-2020)
 *              v1.1(7-November-2021)
 *                  1.优化设备类信息与结构体的变量定义，增加volatile/const关键字
 *                  //2.将rp_config.h分成driver_config.h, device_config.h, user_config.h三个头文件    
 */
#ifndef __RP_CONFIG_H
#define __RP_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stdbool.h"
#include "string.h"
// 驱动层配置
#include "rp_driver_config.h"
// 设备层配置
#include "rp_device_config.h"
// 用户层配置
#include "rp_user_config.h"

/************************************ 开关 *****************************************/
/*超电*/
#define CAP_ENABLE		 0
#define CAP_RECORD       0

/*裁判系统*/
#define JUDGE_ENABLE 	 0

/*过热保护*/
#define OVERHEAT_PROTECT 1

/*功率限制*/
#define POWER_LIMIT		1

/* 陀螺仪滤波 */
#define IMU_USE_MAHONY  1
#define IMU_USE_EKF     0

/************************************ 测试 *****************************************/

/*******************自瞄********************/

/*遥控器自瞄模式 */
#define RC_SHOOT_AIM	0   /*自瞄模式*/
#define  SINGLE_MODE	1   
#define  KEEP_MODE	    1

/*自瞄信息发送*/
#define  RED	0
#define  BLUE	1

#define COLOR		BLUE

/*******************发射********************/

/*射速测试*/
#define  BULLET_SPEED_TEST  1
/* 拨弹延时测试 */
#define  BULLET_DELAY_TEST  1


/*任务层*/

/* 代码测试 */
#define TEST            0



/************************************ 宏定义变量 *****************************************/

/*运动模式*/
#define  TEST_MODE	GYRO_MODE       //开小陀螺的时候记得还得把转速从0改回来


#define  KEEP_50_TEST       1

#define GAME_START				1



/* Exported types ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/


#endif
