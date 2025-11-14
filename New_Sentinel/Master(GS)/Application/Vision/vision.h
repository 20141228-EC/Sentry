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
	
	/*电控发送模式*/
	CMD_CHECK               = 2,    // 自瞄测试模式

} vision_cmd_id_t;


/* 数据长度 */
typedef enum {
	/* Std */
	LEN_VISION_RX_PACKET	= 17 + 5,	// 接收包整包长度
	LEN_VISION_TX_PACKET	= 55 + 5,	// 发送包整包长度

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


/* 发送数据段格式 */
typedef struct __packed 
{
	/*左云台*/
    int16_t     L_yaw_angle;
    int16_t     L_pitch_angle;
	
	/*右云台*/
	int16_t     R_yaw_angle;
    int16_t     R_pitch_angle;
	
	/*大YAW轴*/
	int16_t     B_yaw_angle;
	
	/*比赛数据*/
	uint8_t remain_bullet;
	uint8_t friendly_outpose_HP;
	uint8_t my_HP;
	uint8_t enemy_hero_HP;
	uint8_t enemy_engening_HP;
	uint8_t enemy_infantry3_HP;
	uint8_t enemy_infantry4_HP;
	uint8_t enemy_infantry5_HP;
	uint8_t enemy_sentry_HP;
	uint8_t enemy_outpose_HP;
	
	 __packed   union
    {
        uint8_t flag; 
        struct __packed
        {
            uint8_t engine_hit_enable : 1;    //只打英雄
            uint8_t hero_hit_enable : 1;      //是否打工程
            uint8_t my_color : 1;             //我的颜色
            uint8_t game_progress : 1;        //比赛进程
			uint8_t ammor_size_3 : 1;         //三号步兵装甲板大小
			uint8_t ammor_size_4 : 1;         //四号步兵装甲板大小
			uint8_t ammor_size_5 : 1;         //五号步兵装甲板大小
			uint8_t only_outpose : 1;         //只打前哨
		}bit;
    }flag;
	
	/*双机通讯*/
	uint8_t enemy1_num;
	int16_t enemy1_x;
	int8_t  enemy1_y;
	int16_t enemy1_z;
	
	uint8_t enemy2_num;
	int16_t enemy2_x;
	int8_t  enemy2_y;
	int16_t enemy2_z;
	
	uint8_t enemy3_num;
	int16_t enemy3_x;
	int8_t  enemy3_y;
	int16_t enemy3_z;
	
	uint8_t enemy4_num;
	int16_t enemy4_x;
	int8_t  enemy4_y;
	int16_t enemy4_z;
	
	float uwb_x;
	float uwb_y;
	
	uint8_t arrive_flag;
	uint8_t only_sentry;

}vision_tx_data_t;

/*做好视觉数据突然掉线的准备*/
typedef struct __packed 
{
	/*左云台*/
	int16_t      L_yaw_angle;
    int16_t      L_pitch_angle;
	
	uint8_t      L_shoot_speed;
    uint8_t      L_is_find_target;
	uint8_t      L_shoot_cnt;
	
	/*右云台*/
	uint16_t     R_yaw_angle;
    uint16_t     R_pitch_angle;
	
	uint8_t      R_shoot_speed;
    uint8_t      R_is_find_target;
	uint8_t      R_shoot_cnt;
	
	uint16_t     B_yaw_angle;
	
	uint8_t      nuc_flag1;
	
}vision_rx_data_t;

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

#endif
