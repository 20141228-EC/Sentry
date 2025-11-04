#ifndef __MOTOR_H
#define __MOTOR_H

#include "rp_config.h"
#include "can_protocol.h"
#include "rm_motor.h"
#include "rm_protocol.h"
#include "KT_motor.h"
#include "motor_def.h"

extern rm_motor_pid_t motor_pid[MOTOR_LIST];
extern rm_motor_t rm_motor[MOTOR_LIST];
void rm_motor_list_init(void);
void rm_motor_list_heart_beat(void);
uint8_t rm_motor_list_workstate(void);

extern kt_motor_t Yaw_motor;


//extern KT_motor_t kt_motor[1];
//void kt_motor_list_init(void);
#endif
