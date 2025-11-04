#ifndef __CONTROL_TASK
#define __CONTROL_TASK

#include "main.h"
#include "cmsis_os.h"

#include "master.h"
#include "chassis.h"
#include "power_limit.h"
#include "can_protocol.h"

void StartControlTask(void const * argument);

#endif
