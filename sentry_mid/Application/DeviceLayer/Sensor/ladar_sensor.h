/*
 * @Author: cyr
 * @Date: 2024-11-28 20:37:45
 * @LastEditors: cyr
 * @LastEditTime: 2024-11-29 22:31:49
 * @FilePath: \ladar_mini_car\Application\DeviceLayer\Sensor\ladar_sensor.h
 * @Description:  À×´ï´«¸ÐÆ÷
 */
#ifndef LADAR_SENSOR_H
#define LADAR_SENSOR_H

#include "rp_config.h"
#include "rp_math.h"
#include "string.h"
#include "crc.h"
#include "driver.h"

typedef __packed struct 
{
    uint8_t         heater;
    uint8_t         mode;
    uint8_t         CRC8;

    int16_t         vx;
    int16_t         vy;
    int16_t         vw;

    float           odom_x;
    float           odom_y;

    uint16_t        CRC16;
}ladar_info_rx_t;

typedef __packed struct 
{
    uint8_t heater;
    uint8_t cmd_id;
    uint8_t CRC8;

    float cmd_nav_x;
    float cmd_nav_y;

    uint16_t CRC16;
}ladar_info_tx_t;


typedef struct 
{
    ladar_info_rx_t         *ladar_info_rx;
    ladar_info_tx_t         *ladar_info_tx;
    int16_t	            offline_cnt;
	int16_t	            offline_max_cnt;
    dev_work_state_t	work_state;
    uint8_t             rx_flag;
}ladar_info_t;


typedef struct ladar_sensor_struct
{
    ladar_info_t        *ladar_info;
    
    void                (*heart_beat)(struct ladar_sensor_struct *ladar_sensor);
    void                (*update)(struct ladar_sensor_struct *ladar_snensor,uint8_t *rxBuf);
}ladar_sensor_t;


extern void ladar_heart_beat(ladar_sensor_t *ladar_sensor);
extern void ladar_update(ladar_sensor_t *ladar_sensor,uint8_t *rxBuf);

extern ladar_sensor_t      ladar_sensor;

#endif


