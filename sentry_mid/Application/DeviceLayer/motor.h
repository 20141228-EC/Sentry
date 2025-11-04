#ifndef __MOTOR_H
#define __MOTOR_H

#include "rp_config.h"
#include "can_protocol.h"
#include "rm_motor.h"
#include "rm_protocol.h"
#include "KT_motor.h"
#include "motor_def.h"
#include "DM_motor.h"

extern rm_motor_pid_t motor_pid[MOTOR_LIST];
extern rm_motor_t rm_motor[MOTOR_LIST];
void rm_motor_list_init(void);
void rm_motor_list_heart_beat(void);
uint8_t rm_motor_list_workstate(void);

extern dm_motor_t big_yaw;

#endif
