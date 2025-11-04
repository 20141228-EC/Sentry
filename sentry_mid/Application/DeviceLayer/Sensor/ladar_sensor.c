/*
 * @Author: cyr
 * @Date: 2024-11-28 20:37:21
 * @LastEditors: cyr
 * @LastEditTime: 2024-11-29 22:33:25
 * @FilePath: \ladar_mini_car\Application\DeviceLayer\Sensor\ladar_sensor.c
 * @Description:  雷达传感器
 */
#include "ladar_sensor.h"


ladar_info_rx_t    ladar_info_rx = {
    .heater = 0,
    .mode = 0,
    .CRC8 = 0,
    
    .vx = 0,
    .vy = 0,
    .vw = 0,
    
    .odom_x = 0,
    .odom_y = 0,

    .CRC16 = 0,
};

ladar_info_tx_t     ladar_info_tx = {
    .cmd_id = 0,
    .cmd_nav_x = 0,
    .cmd_nav_y = 0,
    .CRC16 = 0,
    .CRC8 = 0,
    .heater = 0xA5,
};

ladar_info_t     ladar_info = {
    .ladar_info_rx = &ladar_info_rx,
    .ladar_info_tx = &ladar_info_tx,
    .offline_cnt = 0,
    .offline_max_cnt = 200,
    .work_state = DEV_OFFLINE,
    .rx_flag = 0,
};

ladar_sensor_t      ladar_sensor = {
    .ladar_info = &ladar_info,
    
    .heart_beat = ladar_heart_beat,
    .update = ladar_update,
};

/**
 * @description: 雷达心跳检测
 * @param {ladar_sensor_t} *ladar_sensor
 * @return {*}
 */
void ladar_heart_beat(ladar_sensor_t *ladar_sensor)
{
    ladar_sensor->ladar_info->offline_cnt++;

    if(ladar_sensor->ladar_info->offline_cnt>=ladar_sensor->ladar_info->offline_max_cnt)
    {
        ladar_sensor->ladar_info->work_state = DEV_OFFLINE;
        ladar_sensor->ladar_info->offline_cnt = ladar_sensor->ladar_info->offline_max_cnt;
    }
    else
    {
        if(ladar_sensor->ladar_info->work_state == DEV_OFFLINE)
        {
            ladar_sensor->ladar_info->work_state = DEV_ONLINE;
        }
    }
}

/**
 * @description: 雷达传感器数据更新
 * @param {ladar_sensor_t} *ladar_sensor
 * @param {uint8_t} rxBuf
 * @return {*}
 */
void ladar_update(ladar_sensor_t *ladar_sensor,uint8_t *rxBuf)
{
    uint32_t t1 = 0, t2 = 0, tmp = 0;
   
    if(rxBuf[0] == 0xA5)
	{
		if(Verify_CRC8_Check_Sum(rxBuf, 3) == true)
		{
			if(Verify_CRC16_Check_Sum(rxBuf, sizeof(ladar_info_rx_t)) == true)
			{
				memcpy(&ladar_info_rx, rxBuf, sizeof(ladar_info_rx_t));
				ladar_sensor->ladar_info->rx_flag = 1;
				
				
				t1 = t2;
				t2 = micros();
				tmp = t2 - t1;
				return;
			}
		}
	}
	ladar_sensor->ladar_info->rx_flag = 0;
}

//void USART1_rxDataHandler(uint8_t *rxBuf)
//{
//    ladar_sensor.update(&ladar_sensor,rxBuf);
//    ladar_sensor.ladar_info->offline_cnt = 0;
//}
