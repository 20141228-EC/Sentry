#include "master.h"

#include "driver.h"
#include "judge.h"
#include "Chassis.h"

void master_update(master_t *mas_sen, uint8_t *rxbuf);
void master_heart_beat(master_t *mas_sen);

master_tx_t master_tx_info;
master_rx_t master_rx_info;

master_info_t master_info = 
{
	.rx = &master_rx_info,
	.tx = &master_tx_info,
	
	.offline_cnt_max	= 10,
	.offline_cnt			=	0,
};

master_t master =
{
	.info	=	&master_info,
	
	.update			= master_update,
	.heart_beat	=	master_heart_beat,
};

void master_update(master_t *mas_sen, uint8_t *rxbuf)
{
	master_rx_t *rx_info = mas_sen->info->rx;
	
	memcpy(rx_info,	rxbuf,	8);
	mas_sen->info->offline_cnt	= 0;
}

void master_heart_beat(master_t *mas_sen)
{
	master_info_t *mas_info = mas_sen->info;

	mas_info->offline_cnt++;
	
	if(mas_info->offline_cnt > mas_info->offline_cnt_max)
	{
		mas_info->offline_cnt = mas_info->offline_cnt_max;
		mas_sen->work_state = DEV_OFFLINE;
	}
	else if(mas_sen->work_state == DEV_OFFLINE)
		mas_sen->work_state = DEV_ONLINE;						/* 离线->在线 */
}
