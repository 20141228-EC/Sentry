/* Includes ------------------------------------------------------------------*/
#include "judge.h"

extern void Judge_Update(judge_t *judge_sen, uint8_t *rxBuf);
static void Judge_Heartbeat(judge_t *jud_sen);

uint8_t	sentry_txBuf[50];
//uint8_t	sentry_test[50];
judge_decision_tx_packet_t 	 sentry_tx;
extern UART_HandleTypeDef huart6;

judge_info_t judge_info = {

	.offline_cnt 			= 1000,
	.offline_max_cnt 	= 1000,
	
};

judge_t judge = 
{
	.info 		= &judge_info,
	
	.update 	= Judge_Update,
	.heart_beat = Judge_Heartbeat,
	
	.work_state = DEV_OFFLINE,
};

// --------------------------------------------- //
static void Hit_Flag_Heartbeat(struct hit_flag_struct *self);

hit_flag_t hit_flag = 
{
	.hit_cnt			= 3000,
	.hit_cnt_max	= 3000,
	
	.heart_beat	=	Hit_Flag_Heartbeat,
};

/* Private functions ---------------------------------------------------------*/
void Judge_Heartbeat(judge_t *jud_sen)
{
	judge_info_t *jud_info = jud_sen->info;

	jud_info->offline_cnt++;
	
	if(jud_info->offline_cnt > jud_info->offline_max_cnt)
	{
		jud_info->offline_cnt = jud_info->offline_max_cnt;
		jud_sen->work_state 	= DEV_OFFLINE;
	} 
	else
	{
		/* 离线->在线 */
		if(jud_sen->work_state == DEV_OFFLINE)
		{
			jud_sen->work_state = DEV_ONLINE;
		}
	}
}

void Hit_Flag_Heartbeat(struct hit_flag_struct *self)
{
	self->hit_cnt ++;

	if(self->hit_cnt > self->hit_cnt_max)
	{
		self->hit_cnt		= self->hit_cnt_max;
		self->flag	= false;		//	未被击打
	} 
	else if(self->flag == false)
		self->flag = true;
}

// ---------------------- 哨兵自主决策 --------------------- //
/**
 * @brief  发送哨兵自主决策数据
*/
uint8_t Judge_Decision_send(void)
{
	sentry_tx.FrameHeader.sof         = 0xA5;
	sentry_tx.FrameHeader.data_length = 10;
	sentry_tx.FrameHeader.seq         = 0;
	sentry_tx.cmd_id      						= 0x0301;
	sentry_tx.TxData.cmd_id				=	0x0120;
	sentry_tx.TxData.send_id 			=	judge.info->game_robot_status.robot_id;
	sentry_tx.TxData.receive_id			= 	0x8080;
	sentry_tx.TxData.if_revive_await	=	1;

	/*数据发送*/	
	memcpy(sentry_txBuf, &sentry_tx, sizeof(judge_decision_tx_packet_t));
	Append_CRC8_Check_Sum(sentry_txBuf, 5);
	/*增加CRC校验位*/
	Append_CRC16_Check_Sum(sentry_txBuf, LEN_SENTRY_DECISION + 9);
	
	if(Verify_CRC8_Check_Sum(sentry_txBuf, 5) == 1)
	{
		if(Verify_CRC16_Check_Sum(sentry_txBuf, LEN_SENTRY_DECISION + 9) == 1)
		{
			if(HAL_UART_Transmit_DMA(&huart6,sentry_txBuf,sizeof(judge_decision_tx_packet_t)) == HAL_OK)
				return true;
			else
				return false;
		}
	}	
}
