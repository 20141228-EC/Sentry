#ifndef __CONTROL_TASK
#define __CONTROL_TASK

#include "cmsis_os.h"
#include "main.h"

#include "rp_config.h"
#include "rc_sensor.h"

#include "Robot.h"
#include "arm_math.h"

#include "communicate.h"

#include "device.h"

void StartControlTask(void const * argument);

#endif
