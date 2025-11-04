#include "cap.h"
#include "chassis.h"

#include "rp_math.h"

bool CAP_STATE;
float cap_u;

static void CAP_setMessage(cap_t *self,uint16_t powerBuff,uint16_t powerLimit);
static void CAP_rxMessage(cap_t *self, uint8_t *rxBuf);
static void CAP_heart_beat(cap_t *self);

cap_t cap = {
	.work_state			= DEV_ONLINE,
	.info.canId 		= 0x222,
	.info.offline_cnt 		= 100,
	.info.offline_max_cnt = 100,
	.info.tx.chassis_power_buffer	= 60,
	.info.tx.chassis_power_limit	= 100,
	.info.tx.cap_power_out_limit	= -300,
	.info.tx.cap_power_in_limit		= 300,
	.info.tx.bit_control.cap_switch = 1,
	.info.tx.bit_control.turbo_mode = 1,
	
	.setdata		= CAP_setMessage,
	.update			= CAP_rxMessage,
	.heart_beat	= CAP_heart_beat,
};

static void CAP_heart_beat(cap_t *self)
{
	cap_info_t *info = &self->info;

	info->offline_cnt++;
	if(info->offline_cnt > info->offline_max_cnt)
	{
		info->offline_cnt = info->offline_max_cnt;
		self->work_state = DEV_OFFLINE;
	}
	else 
	{
		if(self->work_state == DEV_OFFLINE)
		{
			self->work_state = DEV_ONLINE;
		}
	}
}

void CAP_setMessage(cap_t *self,uint16_t powerBuff,uint16_t powerLimit)
{
    self->info.tx.chassis_power_buffer 	= powerBuff;
    self->info.tx.chassis_power_limit  	= powerLimit;
	 
    memcpy(self->info.Buff0x222,&self->info.tx,8);
}

void CAP_rxMessage(cap_t *self, uint8_t *rxBuf)
{
	memcpy(&self->info.rx, rxBuf, sizeof(capboard_rx_info_t));

	self->info.Cap_U = int16_to_float(self->info.rx.now_cap_V, 32000, -32000, 25, 0);
	self->info.Cap_I = int16_to_float(self->info.rx.now_cap_I, 32000, -32000, 16, -16);
	
	CAP_STATE = self->info.rx.bit_state.ability;
	cap_u			=	self->info.Cap_U;
	
	self->info.offline_cnt = 0;
}


