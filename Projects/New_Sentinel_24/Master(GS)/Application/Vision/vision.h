#ifndef __VISION_H
#define __VISION_H

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdbool.h>
#include "crc.h"
/* 宏定义与全局变量 ------------------------------------------------------------*/

/*起始字节，协议固定尾0xA5*/
#define VISION_SEND_ID             (0xA5)
#define VISION_OFFLINE_MAX_CNT     (300)
#define VISION_OFFLINE             (0)
#define VISION_ONLINE              (1)
#define BALANCE_HP                 (300)
#define HIT                        (1)
#define NO_HIT                     (0)

#define BIGGG_ARMOR                (1)
#define SMALL_ARMOR                (0)
/* 指令与配置 ------------------------------------------------------------*/
typedef enum {
	/*关避障要多按一次*/
	/*视觉发送模式*/
	CMD_PATROL		        = 0,	// 巡逻模式 
	CMD_GOGOGO              = 7,    // 指哪走哪       Z
	
	CMD_R3                  = 8,    // 前往R3高地     T  （）
	CMD_R4                  = 9,    // 前往R4高地     N   ()
	CMD_OUTPOS              = 10,   // 前往环高       Q   
	
	CMD_POINT_A             = 11,   // 前往我方肥婆点 Y （推前哨，自动开启打前哨）
	CMD_FUCK_OUTPOS         = 13,   // 资源岛推前哨   J （资源岛推前哨，自动开启打前哨）
	//只能按一次J
	CMD_POINT_B             = 12,   // 前往我方前哨战 K （防英雄打前哨战）
	
	CMD_POINT_D             = 14,   // 前往巡逻区右下 I （快速回家2）
	CMD_POINT_E             = 15,   // 前往对面前哨战 V （进攻1）
	CMD_STOP                = 16,   // 停止           H
	//CMD_POINT_F             = 17,   // 前往对面兑换站 B （进攻2）
	CMD_POINT_G             = 18,   // 前往对面资源岛 O （开局打工程)
	CMD_POINT_H             = 19,   // 前往对面巡逻区 L （进攻3）
	CMD_POINT_J             = 21,   // 巡逻区巡逻模式 G	
	CMD_POINT_K             = 22,   // 前往对面肥婆点 F
	
	//E 击打工程 默认开启 右半部分开启
	//C 只打英雄 默认关闭 右半部分开启 
	//R 开启避障 默认开启 右半部分开启 (回家过程中被挡,记得再次选择目的地)
	////U 只打前哨 默认关闭 右半部分开启 (开局打前哨)
	//X 强制巡逻 默认关闭 右半部分开启 (防止误识别)
	//B 只打哨兵 默认关闭 右半部分开启
	
	//WASD 右边慢 左边快

	//2024
	CMD_MAP_FORWARD			=23	,	//M	//转发小地图标记
	CMD_BULLET_BUY_AT_HP	=24	,	//N	//在补血点购买允许发弹量 
	CMD_BULLET_FILL			=25	,	//R	//在补给站补给弹丸 --回到补给站
	CMD_REVIVE_AWAIT		=26	,	//V	//确认复活 
	CMD_BULLET_BUY_REMOTE	=27	,	//B	//远程兑换发弹量 
	CMD_HP_BUY_REMOTE		=28	,	//H	//远程兑换血量 
	CMD_REVIVE_IMMEDIATE	=29	,	//O	//远程兑换立即复活 
	
	/*电控发送模式*/
	CMD_CHECK               = 2,    // 自瞄测试模式

} vision_cmd_id_t;


/* 数据长度 */
typedef enum {
	/* Std */
	LEN_VISION_RX_PACKET	= 129 + 5,	// 接收包整包长度
	LEN_VISION_TX_PACKET	= 98  + 5,	// 发送包整包长度

	LEN_RX_DATA 			= LEN_VISION_RX_PACKET - 5,	// 接收数据段长度
	LEN_TX_DATA 			= LEN_VISION_TX_PACKET - 5,	// 发送数据段长度
	
	LEN_FRAME_HEADER 	  	= 3,	// 帧头长度
	LEN_FRAME_TAILER 		= 2,	// 帧尾CRC16
	
} vision_data_length_t;


/* 数据包格式 ------------------------------------------------------------*/
/* 帧头格式 */
typedef struct __packed
{
	uint8_t  			    sof;		// 同步头
	vision_cmd_id_t  	    cmd_id;	    // 命令码
	uint8_t  			    crc8;		// CRC8校验码
} vision_frame_header_t;

/* 帧尾格式 */
typedef struct __packed 
{
	uint16_t crc16;					// CRC16校验码
} vision_frame_tailer_t;

/**
 * uint8_t表示范围为 0-255
 * int16_t -32768 到 32767
 * ：1 表示1个位域，只能表示0个1
 */


/**
 * 敌方位置（大yaw系） z=0代表未发现
 */
typedef struct __packed 
{
	int16_t x;
	int16_t y;
	int16_t z;
}EnemyPosition_t;  //2*3 = 6字节

/**
 * 敌方位置数据
 * 用于全向感知时：相对坐标
 * 用于自瞄时：绝对坐标
 * 未发现该目标时候，默认发0
 */

typedef struct __packed 
{
	uint8_t target_enemy_num;                   //目标敌方兵种
	EnemyPosition_t target_enemy_position;        //目标兵种位置

	/* 敌方位置（未发现则默认z为0） 1英雄 2工程 3-5步兵  6哨兵 7前哨站 8基地 */
	EnemyPosition_t enemys_positions[8];
	
}EnemyData_t; //6 * 9 + 1 = 55字节

typedef struct __packed 
{
	/* 目标角度（绝对世界系） */
	int16_t target_yaw;                  //-4096 ~ 4096
	int16_t target_pitch;                //-4096 ~ 4096

	/* 打弹相关*/
	uint8_t shoot_frequency;            //枪口射频，可能会高频地在0和其他数字来回切换(反陀螺），每1代表0.1Hz(0.1发/s）
	uint8_t shoot_value;                //每改变1，射1发(检测到该值改变才打弹）
	//理论上实战用不到，可能调试时会用到//

	__packed   union
    {
        uint8_t flag; 
        struct __packed
        {
			uint8_t is_find_target: 1;             //是否发现自瞄目标
			uint8_t is_find_dafu: 1;               //是否发现打符目标
			uint8_t is_hit_enable: 1;              //暂时用不到
		}bit;
    }flag;

	EnemyData_t autoaim_enemy_data;          //自瞄敌方位置数据
	
}AutoAimData_t; //6 + 1 + 55 = 62字节





/* 发送数据段格式 */
typedef struct __packed 
{
	uint8_t left_hit_mode;                        //击打模式 0-默认 1-自瞄 2/3/5打符 4-初始化 6-哨兵 7-前哨 8-基地 9-英雄 10-工程
	uint8_t right_hit_mode;                       //击打模式 0-默认 1-自瞄 2/3/5打符 4-初始化 6-哨兵 7-前哨 8-基地 9-英雄 10-工程
	uint8_t left_trigger_mode;          		  //相机触发模式 0-软触发 1-硬触发
	uint8_t right_trigger_mode;

	/* 切换目标（比赛时哨兵用不到,调试可能用到），该值只要改变(且不为0），则枪管会切换自瞄最佳目标 */
	/* 比赛时默认法定值0（不切换）*/
	uint8_t left_is_switch_target;
	uint8_t right_is_switch_target;

	/* 比赛信息 */
	uint8_t own_remain_bullets;          //剩余弹量
	uint8_t shoot_speed;                       //射速固定30  8

	/* 2024.5 新增比赛时间，当前位置*/
	uint16_t game_time;
	uint16_t uwb_position_x;
	uint16_t uwb_position_y;

	uint8_t use_vision:1;                      //0:关闭视觉识别 1：开启视觉识别 command_mode=2 && use_vision=1时候，视觉仍然自瞄但是打弹是电控控制的
	uint8_t enemy_color: 1;                    //敌方颜色 0-blue 1-red

	/* 步兵装甲板大小 */
	uint8_t armor3_size: 1;                //0-小 1-大
	uint8_t armor4_size: 1;
	uint8_t armor5_size: 1;
	/* 不打XX */
	uint8_t no_hit_enemy2: 1;           //不打工程
	uint8_t no_hit_enemy6: 1;           //不打哨兵
	uint8_t no_hit_enemy7: 1;           //不打前哨
	
	uint8_t no_hit_enemy8: 1;           //不打基地
	


	/* 枪管云台角度 电机角 (绝对世界系) */  // 10
	int16_t left_head_yaw;              //（-4096 ~ 4096）
	int16_t left_head_pitch;            //（-4096 ~ 4096）
	int16_t right_head_yaw;             //（-4096 ~ 4096）
	int16_t right_head_pitch;           //（-4096 ~ 4096）
	float big_yaw_angle;              //大yaw云台角度（-4096 ~ 4096）


	//TODO::Serialport:设置敌方初始血量值  20
	/* 血量   0自己 1英雄 2工程 3-5步兵  6哨兵 7前哨站 8基地 */
	/*为了保证在uint8_t的范围 ：blood0-6（机器人） 是 实时血量÷5； blood7-8（建筑） 是 实时血量÷10 */
	uint8_t own_blood6;                 //自己(哨兵）
	uint8_t own_blood7;                 //己方前哨站
	uint16_t own_blood8;                 //己方基地

	uint8_t enemy_blood1;
	uint8_t enemy_blood2;
	uint8_t enemy_blood3;
	uint8_t enemy_blood4;
	uint8_t enemy_blood5;
	uint8_t enemy_blood6;
	uint8_t enemy_blood7;
	uint16_t enemy_blood8;     //33

	/* 全向感知 */
	EnemyData_t omni_enemy_data;  // 55

//    /* 自瞄相关 */
//    uint8_t is_change_target=0;(哨兵切换目标为自主决策,用不上)

	/* 导航相关 */
	uint8_t is_arrive;              //是否抵达目标位置
	uint8_t is_moving;                //是否正在运动中

}vision_tx_data_t;


/*做好视觉数据突然掉线的准备*/
typedef struct __packed 
{
	uint8_t left_hit_mode;            //击打模式 0-默认 1-自瞄 2/3/5打符 4-初始化 6-哨兵 7-前哨 8-基地 9-英雄 10-工程
	uint8_t right_hit_mode;           //击打模式 0-默认 1-自瞄 2/3/5打符 4-初始化 6-哨兵 7-前哨 8-基地 9-英雄 10-工程

	/* 自瞄相关（绝对世界系） */
	int16_t B_yaw_angle;     //目标大yaw角度(陀螺仪) （-4096 ~ 4096）
	
	AutoAimData_t left_aim_data;
	AutoAimData_t right_aim_data;

	uint8_t computer_id;              //电脑id号，0-底盘 1-头
	
}vision_rx_data_t; // 4 + 62 * 2 + 1 = 129字节

/* 发送包格式 */
typedef struct __packed
{
	vision_frame_header_t   FrameHeader;	// 帧头
	vision_tx_data_t	    TxData;		    // 数据
	vision_frame_tailer_t   FrameTailer;	// 帧尾	
}vision_tx_packet_t;

/* 接收包格式 */
typedef struct __packed 
{
	vision_frame_header_t   FrameHeader;	// 帧头
	vision_rx_data_t	    RxData;		    // 数据
	vision_frame_tailer_t   FrameTailer;	// 帧尾	
} vision_rx_packet_t;


/* 工作模式 ------------------------------------------------------------*/

/*工作模式*/
typedef enum
{
	VISION_MODE_MANUAL		  = 0,	// 手动模式
	VISION_MODE_AUTO		  = 1,	// 自瞄模式
	VISION_MODE_BIG_BUFF	  = 2,	// 打大符模式
	VISION_MODE_SMALL_BUFF	  = 3,	// 打小符模式
} Vision_Mode_t;

/* 辅助标识变量 */
typedef struct
{
	uint8_t 		  my_color;			 // 用0/1表示颜色
	Vision_Mode_t	  mode;				 // 视觉模式
	uint8_t  		  rx_data_valid;     // 接收数据的正确性
	uint16_t 		  rx_err_cnt;		 // 接收数据的错误统计
	uint32_t		  rx_cnt;		     // 接收数据包的统计
	bool		      rx_data_update;    // 接收数据是否更新
    uint32_t 		  rx_time_prev;	     // 接收数据的前一时刻
	uint32_t 		  rx_time_now;	     // 接收数据的当前时刻
	uint16_t 		  rx_time_fps;	     // 帧率
	
	uint8_t           work_state;
	int16_t		      offline_cnt;
	int16_t		      offline_max_cnt;
	
} vision_state_t;
/* 汇总 ------------------------------------------------------------*/

typedef  struct 
{
	vision_rx_packet_t *rx_pack;
	vision_tx_packet_t *tx_pack;
	vision_state_t      state;
	
	
}vision_t;

/* 相关函数 ------------------------------------------------------------*/

void Vision_Init(void);
void Vision_Update(void);

bool Vision_SendData(void);
bool Vision_GetData(uint8_t *rxBuf);

void vision_task(void);

int16_t Control2Vision_pitch_left(int16_t angle);
int16_t Control2Vision_yaw_left(int16_t angle);
int16_t Control2Vision_pitch_right(int16_t angle);
int16_t Control2Vision_yaw_right(int16_t angle);

int16_t Vision2Control_pitch_left(int16_t angle);
int16_t Vision2Control_yaw_left(int16_t angle);
int16_t Vision2Control_pitch_right(int16_t angle);
int16_t Vision2Control_yaw_right(int16_t angle);




#endif
