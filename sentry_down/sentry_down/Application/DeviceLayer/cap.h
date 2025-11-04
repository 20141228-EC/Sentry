/*
 * @Author: lion
 * @Date: 2025-04-18 20:14:24
 * @LastEditors: Chen Yiran 1060874648@qq.com
 * @LastEditTime: 2025-04-18 21:53:49
 * @FilePath: \DOWN\Application\DeviceLayer\cap.h
 * @Description: 
 * 
 */
#ifndef __CAP_H
#define __CAP_H

#include "stm32f4xx_hal.h"

#include "rp_config.h"	

/* 通信相关结构体 */
typedef struct __attribute__((packed)) rx_info_struct
{
    
    uint8_t  chassis_power_buffer;          // 底盘能量缓冲
    uint16_t chassis_power_limit ;          // 机器人底盘功率限制上限
    int16_t  cap_power_out_limit ;          // 电容放电功率限制，定义为负值
    uint16_t cap_power_in_limit  ;          // 电容充电功率限制，定义为正值
    
    struct __attribute__((packed)) bit_control_struct
    {
        uint8_t cap_switch : 1;             // 电容开关，1为开，0为关
        uint8_t turbo_mode : 1;             // 是否使用缓冲能量来充电，0为不用，1为用
        uint8_t unuse      : 6;             // 暂时未使用
    }bit_control;
    
} capboard_rx_info_t;

typedef struct __attribute__((packed)) tx_info_struct {
    
    int16_t now_chassis_power;              // 当前底盘消耗功率
    int16_t now_cap_V;                      // 当前电容组电压
    int16_t now_cap_I;                      // 当前电容组电流
    
    struct __attribute__((packed)) bit_state_struct
    {
        uint8_t ability             : 1;    // 电容是否有放电能力，0为无，1为有
        uint8_t unuse               : 7;    // 暂时未使用
    }bit_state;
    
} capboard_tx_info_t;


typedef struct cap_info_struct {
	
	uint16_t						canId;
	
	capboard_rx_info_t	tx;
	capboard_tx_info_t	rx;
	
	float Cap_U;
	float Cap_I;
	
	uint8_t		Buff0x222[8];

	uint8_t   offline_cnt;
	uint8_t   offline_max_cnt;

} cap_info_t;


typedef struct cap_struct{

	cap_info_t 	info;

	void				(*setdata)(struct cap_struct *self,uint16_t powerBuff,uint16_t powerLimit);
	void				(*update)(struct cap_struct *self, uint8_t *rxBuf);
	void				(*heart_beat)(struct cap_struct *self);
	dev_work_state_t	work_state;
	
}cap_t;

extern cap_t cap;

#endif
