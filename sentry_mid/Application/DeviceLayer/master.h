#ifndef __MASTER_H
#define __MASTER_H


#include "driver.h"
#include "rp_config.h"
#include "judge.h"
#include "Chassis.h"


#define CHASSIS_SPEED_ID					0x001
#define SLAVE_CTRL_FLAG_ID				0x002

typedef struct host_rx_info_struct
{
    int16_t         chassis_speed[4];
    uint8_t         ctrl_mode;
    uint8_t         move_mode;
    uint8_t         start_flag;
	uint8_t 			is_find_target;
}host_rx_info_t;

typedef struct host_tx_info_struct
{
    uint8_t				mode;
}host_tx_info_t;


typedef struct host_info_struct
{
    host_rx_info_t         *rx_info;
    host_tx_info_t         *tx_info;
	
	uint16_t					offline_cnt;
	uint16_t					offline_cnt_max;
}host_info_t;

typedef struct host_struct
{
    dev_work_state_t        work_state;
    host_info_t            *info;

    void (*get_chassis_info)(struct host_struct *host,uint8_t *rxBuf);
    void (*get_ctrl_flag)(struct host_struct *host,uint8_t *rxBuf);

}host_t;



void Host_get_chassis_info(host_t *host,uint8_t *rxBuf);
void Host_get_ctrl_flag(host_t *host,uint8_t *rxBuf);
void game_status_send(uint8_t can_num);
void game_robot_HP_send(uint8_t can_num);
void power_heat_send(uint8_t can_num); 
void game_robot_status_send(uint8_t can_num);
void shoot_data_send(uint8_t can_num);
void robot_hurt_send(uint8_t can_num);
void robot_pos_send(uint8_t can_num);

extern host_t host;

#endif