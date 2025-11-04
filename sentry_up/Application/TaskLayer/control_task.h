#ifndef __CONTROL_TASK
#define __CONTROL_TASK

#include "cmsis_os.h"
#include "main.h"

#include "rp_config.h"
#include "device.h"
#include "rp_math.h"

#include "Robot.h"

void StartControlTask(void const * argument);

#endif
