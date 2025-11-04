#ifndef __COMMUNITY_TASK_H
#define __COMMUNITY_TASK_H

#include "cmsis_os.h"
#include "main.h"
#include "bmi.h"
#include "rc_sensor.h"
#include "rc_protocol.h"
#include "can_protocol.h"
#include "cap.h"
#include "Robot.h"
#include "device.h"

void StartCommunityTask(void const * argument);

#endif
