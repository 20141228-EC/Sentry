#include "cap.h"
#include "chassis.h"


static float int16_to_float(int16_t a, int16_t a_max, int16_t a_min, float b_max, float b_min);
static int16_t float_to_int16(float b, float b_max, float b_min, int16_t a_max, int16_t a_min);

static void CAP_setMessage(cap_t *self,uint16_t powerBuff,uint16_t powerLimit,int16_t  cap_power_out_limit,uint16_t cap_power_in_limit);
static void CAP_rxMessage(cap_t *self, uint8_t *rxBuf);
static void CAP_heart_beat(cap_t *self);

cap_t cap = {
	.work_state = DEV_ONLINE,
	.info.canId = 0x222,
	.info.offline_cnt = 100,
	.info.offline_max_cnt = 100,
	.info.tx.chassis_power_buffer = 60,
	.info.tx.chassis_power_limit = 100,
	.info.tx.cap_power_out_limit = -120,
	.info.tx.cap_power_in_limit = 100,
	.info.tx.bit_control.cap_switch = 0,
	.info.tx.bit_control.turbo_mode = 0,
	
	.setdata = CAP_setMessage,
	.update = CAP_rxMessage,
	.heart_beat = CAP_heart_beat,
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

//下主控,发送裁判系统信息
void CAP_setBuff0x222(cap_t *self)
{
    memcpy(self->info.Buff0x222,&self->info.tx,8);
}

void CAP_setMessage(cap_t *self,uint16_t powerBuff,uint16_t powerLimit,int16_t  cap_power_out_limit,uint16_t cap_power_in_limit)
{
    self->info.tx.chassis_power_buffer = powerBuff;
    self->info.tx.chassis_power_limit  = powerLimit;
    self->info.tx.cap_power_in_limit   = cap_power_in_limit;
    self->info.tx.cap_power_out_limit      = cap_power_out_limit;
	
//	if(cap.info.tx.chassis_power_buffer <=30)
//	{
//		cap.info.tx.cap_power_in_limit = 0;
//	}
//	else
//	{
//		cap.info.tx.cap_power_in_limit = 100;
//	}
//	
	 
    CAP_setBuff0x222(self);
}



void CAP_rxMessage(cap_t *self, uint8_t *rxBuf)
{
    memcpy(&self->info.rx, rxBuf, sizeof(capboard_rx_info_t));

	self->info.Cap_U = int16_to_float(self->info.rx.now_cap_V, 32000, -32000, 25, 0);
	self->info.Cap_I = int16_to_float(self->info.rx.now_cap_I, 32000, -32000, 16, -16);
	
	self->info.offline_cnt = 0;

}

void CAP_Protect(cap_t *self)
{
    if(self->info.Cap_U > 23)
    {
        self->info.tx.cap_power_in_limit = 30;
    }
    
}


float int16_to_float(int16_t a, int16_t a_max, int16_t a_min, float b_max, float b_min)
{
    int32_t a_32 = a, a_max_32 = a_max, a_min_32 = a_min;
    int32_t diff_a = a_max_32 - a_min_32;
    
    if (diff_a == 0) return (b_max + b_min) / 2.0f; // 处理除零
    
    float ratio = (float)(a_32 - a_min_32) / (float)diff_a;
    return ratio * (b_max - b_min) + b_min;
}

int16_t float_to_int16(float b, float b_max, float b_min, int16_t a_max, int16_t a_min)
{
    // 处理除零和无效输入
    if (b_max == b_min) return (int16_t)((a_max + a_min) / 2);
    
    // 计算比例并映射到整数范围
    float ratio = (b - b_min) / (b_max - b_min);
    
    // 提升计算范围避免溢出
    int32_t a = (int32_t)(ratio * (a_max - a_min) + a_min + 0.5f); // 四舍五入
    
    // 钳位到目标范围
    a = (a < a_min) ? a_min : (a > a_max) ? a_max : a;
    
    return (int16_t)a;
}



