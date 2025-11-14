/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "ui_protocol.h"

uint8_t 			client_tx_buf[128];
client_info_t client_info;

/* Exported variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart5;


/* Private function prototypes -----------------------------------------------*/
// Driver
static uint8_t UI_Send_Data(uint8_t *txbuf, uint16_t length);		// 串口驱动层发送

/**
	* @brief  串口接收数据
  * @param  
  * @retval 
  */
void client_info_update(void)
{
	client_info.robot_id	= judge.info->game_robot_status.robot_id;
	client_info.client_id	= judge.info->self_client;
}

/**
	* @brief  数组求和
  * @param  
  * @retval 
  */
uint16_t arr_sum(uint8_t *addr, uint8_t n)
{
	uint8_t i = 1;
	uint16_t sum = 0;
	
	for ( ; i < n; i++)
	{
		sum += addr[i];
	}
	
	return sum;
}

/*-------------------- 获取图像数据帧 begin --------------------*/
/**
  * @brief  获取直线数据帧
  * @param  
  * @retval 图形数据结构体
  */
Graphic_data_struct_t draw_line(char *name,	// 图形名
											uint8_t operate_tpye,	// 图形操作
											uint8_t layer,				// 图层数，0~9
											uint8_t color,				// 颜色
											uint16_t width,				// 线条宽度
											uint16_t start_x,			// 起点 x 坐标
											uint16_t start_y,			// 起点 y 坐标
											uint16_t end_x,				// 终点 x 坐标
											uint16_t end_y)				// 终点 y 坐标
{
	Graphic_data_struct_t data;
	
	memcpy(data.graphic_name, name, 3);
	data.operate_tpye = operate_tpye;
	data.graphic_tpye = 0;
	data.layer = layer;
	data.color = color;
	data.details_a = 0;
	data.details_b = 0;
	data.width = width;
	data.start_x = start_x;
	data.start_y = start_y;
	data.details_c = 0;
	data.details_d = end_x;
	data.details_e = end_y;
	
	return data;
}

/**
  * @brief  获取矩形数据帧
  * @param  
  * @retval 图形数据结构体
  */
Graphic_data_struct_t draw_rectangle(char *name,	// 图形名
											uint8_t operate_tpye,				// 图形操作
											uint8_t layer,							// 图层数，0~9
											uint8_t color,							// 颜色
											uint16_t width,							// 线条宽度
											uint16_t start_x,						// 起点 x 坐标
											uint16_t start_y,						// 起点 y 坐标
											uint16_t end_x,							// 对角顶点 x 坐标
											uint16_t end_y)							// 对角顶点 y 坐标
{
	Graphic_data_struct_t data;
	
	memcpy(data.graphic_name, name, 3);
	data.operate_tpye = operate_tpye;
	data.graphic_tpye = 1;
	data.layer = layer;
	data.color = color;
	data.details_a = 0;
	data.details_b = 0;
	data.width = width;
	data.start_x = start_x;
	data.start_y = start_y;
	data.details_c = 0;
	data.details_d = end_x;
	data.details_e = end_y;
	
	return data;
}

/**
  * @brief  获取整圆数据帧
  * @param  
  * @retval 图形数据结构体
  */
Graphic_data_struct_t draw_circle(char *name,	// 图形名
											uint8_t operate_tpye,		// 图形操作
											uint8_t layer,					// 图层数，0~9
											uint8_t color,					// 颜色
											uint16_t width,					// 线条宽度
											uint16_t ciclemid_x,		// 圆心 x 坐标
											uint16_t ciclemid_y,		// 圆心 y 坐标
											uint16_t radius)				// 半径
{
	Graphic_data_struct_t data;
	
	memcpy(data.graphic_name, name, 3);
	data.operate_tpye = operate_tpye;
	data.graphic_tpye = 2;
	data.layer = layer;
	data.color = color;
	data.details_a = 0;
	data.details_b = 0;
	data.width = width;
	data.start_x = ciclemid_x;
	data.start_y = ciclemid_y;
	data.details_c = radius;
	data.details_d = 0;
	data.details_e = 0;
	
	return data;
}

/**
  * @brief  获取椭圆数据帧
  * @param  
  * @retval 图形数据结构体
  */
Graphic_data_struct_t draw_ellipse(char *name,	// 图形名
											uint8_t operate_tpye,			// 图形操作
											uint8_t layer,						// 图层数，0~9
											uint8_t color,						// 颜色
											uint16_t width,						// 线条宽度
											uint16_t start_x,					// 圆心 x 坐标
											uint16_t start_y,					// 圆心 y 坐标
											uint16_t end_x,						// x 半轴长度
											uint16_t end_y)						// y 半轴长度

{
	Graphic_data_struct_t data;
	
	memcpy(data.graphic_name, name, 3);
	data.operate_tpye = operate_tpye;
	data.graphic_tpye = 3;
	data.layer = layer;
	data.color = color;
	data.details_a = 0;
	data.details_b = 0;
	data.width = width;
	data.start_x = start_x;
	data.start_y = start_y;
	data.details_c = 0;
	data.details_d = end_x;
	data.details_e = end_y;
	
	return data;
}

/**
  * @brief  获取圆弧数据帧
  * @param  
  * @retval 图形数据结构体
  */
Graphic_data_struct_t draw_arc(char *name,			// 图形名
											uint8_t operate_tpye,			// 图形操作
											uint8_t layer,						// 图层数，0~9
											uint8_t color,						// 颜色
											uint16_t start_angle,			// 起始角度
											uint16_t end_angle,				// 终止角度
											uint16_t width,						// 线条宽度
											uint16_t circlemin_x,			// 圆心 x 坐标
											uint16_t circlemin_y,			// 圆心 y 坐标
											uint16_t end_x,						// x 半轴长度
											uint16_t end_y)						// y 半轴长度
{
	Graphic_data_struct_t data;
	
	memcpy(data.graphic_name, name, 3);
	data.operate_tpye = operate_tpye;
	data.graphic_tpye = 4;
	data.layer = layer;
	data.color = color;
	data.details_a = start_angle;
	data.details_b = end_angle;
	data.width = width;
	data.start_x = circlemin_x;
	data.start_y = circlemin_y;
	data.details_c = 0;
	data.details_d = end_x;
	data.details_e = end_y;
	
	return data;
}

/**
  * @brief  获取浮点数数据帧
  * @param  
  * @retval 图形数据结构体
  */
Graphic_data_struct_t draw_float(char *name,	// 图形名
											uint8_t operate_tpye,		// 图形操作
											uint8_t layer,					// 图层数，0~9
											uint8_t color,					// 颜色
											uint16_t size,					// 字体大小
											uint16_t decimal,				// 小数位有效个数
											uint16_t width,					// 线条宽度
											uint16_t start_x,				// 起点 x 坐标
											uint16_t start_y,				// 起点 y 坐标
											int32_t num)						// 乘以 1000 后，以 32 位整型数，int32_t
{
	Graphic_data_struct_t data;
	
	memcpy(data.graphic_name, name, 3);
	data.operate_tpye = operate_tpye;
	data.graphic_tpye = 5;
	data.layer = layer;
	data.color = color;
	data.details_a = size;
	data.details_b = decimal;
	data.width = width;
	data.start_x = start_x;
	data.start_y = start_y;
	data.details_c = num;
	data.details_d = num >> 10;
	data.details_e = num >> 21;
	
	return data;
}

/**
  * @brief  获取整型数数据帧
  * @param  
  * @retval 图形数据结构体
  */
Graphic_data_struct_t draw_int(char *name,	// 图形名
											uint8_t operate_tpye,	// 图形操作
											uint8_t layer,				// 图层数，0~9
											uint8_t color,				// 颜色
											uint16_t size,				// 字体大小
											uint16_t width,				// 线条宽度
											uint16_t start_x,			// 起点 x 坐标
											uint16_t start_y,			// 起点 y 坐标
											int32_t num)					// 32 位整型数，int32_t
{
	Graphic_data_struct_t data;
	
	memcpy(data.graphic_name, name, 3);
	data.operate_tpye = operate_tpye;
	data.graphic_tpye = 6;
	data.layer = layer;
	data.color = color;
	data.details_a = size;
	data.details_b = 0;
	data.width = width;
	data.start_x = start_x;
	data.start_y = start_y;
	data.details_c = num;
	data.details_d = num >> 10;
	data.details_e = num >> 21;
	
	return data;
}

/**
  * @brief  获取字符数据帧
  * @param  
  * @retval 图形数据结构体
  */
Graphic_data_struct_t draw_char(char *name,	// 图形名
	             uint8_t operate_tpye,				// 图形操作
               uint8_t layer,								// 图层数，0~9
               uint8_t color,								// 颜色
               uint16_t size,								// 字体大小
               uint16_t length,							// 字符长度
               uint16_t width,							// 线条宽度
               uint16_t start_x,						// 起点 x 坐标
               uint16_t start_y)						// 起点 y 坐标
{
	Graphic_data_struct_t data;
	
	memcpy(data.graphic_name, name, 3);
	data.operate_tpye = operate_tpye;
	data.graphic_tpye = 7;
	data.layer = layer;
	data.color = color;
	data.details_a = size;
	data.details_b = length;
	data.width = width;
	data.start_x = start_x;
	data.start_y = start_y;
	data.details_c = 0;
	data.details_d = 0;
	data.details_e = 0;
	
	return data;
}
/*-------------------- 获取图像数据帧 end   --------------------*/

/*-------------------- 发送数据帧 begin --------------------*/
/**
	* @brief  发送绘制一个图形帧数据
  * @param  
  * @retval 
  */
uint8_t client_send_single_graphic(ext_graphic_one_data_t data)
{
	/* 帧头 */
	data.txFrameHeader.sof = 0xA5;
	data.txFrameHeader.data_length = LEN_INTERACT_draw_one_graphic;
	data.txFrameHeader.seq = 0;
	memcpy(client_tx_buf, &data.txFrameHeader, 4);
	Append_CRC8_Check_Sum(client_tx_buf, 5);
	
	/* 命令码ID */
	data.CmdID = 0x301;
	memcpy(&client_tx_buf[5], (void*)&data.CmdID, 2);
	
	/* 数据段 */
	data.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_one_graphic;
	data.dataFrameHeader.send_ID = client_info.robot_id;
	data.dataFrameHeader.receiver_ID = client_info.client_id;
	memcpy(&client_tx_buf[7], &data.dataFrameHeader, 6);
	memcpy(&client_tx_buf[13], &data.clientData, 15);
	
	/* 帧尾 */
	Append_CRC16_Check_Sum(client_tx_buf, 5 + 2 + LEN_INTERACT_draw_one_graphic + 2);
	
	/* 发送 */
	return UI_Send_Data(client_tx_buf, 5 + 2 + LEN_INTERACT_draw_one_graphic + 2);
}

/**
	* @brief  发送绘制二个图形帧数据
  * @param  
  * @retval 
  */
uint8_t client_send_double_graphic(ext_graphic_two_data_t data)
{
	/* 帧头 */
	data.txFrameHeader.sof = 0xA5;
	data.txFrameHeader.data_length = LEN_INTERACT_draw_two_graphic;
	data.txFrameHeader.seq = 0;
	memcpy(client_tx_buf, &data.txFrameHeader, 4);
	Append_CRC8_Check_Sum(client_tx_buf, 5);
	
	/* 命令码ID */
	data.CmdID = 0x301;
	memcpy(&client_tx_buf[5], (void*)&data.CmdID, 2);
	
	/* 数据段 */
	data.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_two_graphic;
	data.dataFrameHeader.send_ID = client_info.robot_id;
	data.dataFrameHeader.receiver_ID = client_info.client_id;
	memcpy(&client_tx_buf[7], &data.dataFrameHeader, 6);
	memcpy(&client_tx_buf[13], data.clientData, 15*2);
	
	/* 帧尾 */
	Append_CRC16_Check_Sum(client_tx_buf, 5 + 2 + LEN_INTERACT_draw_two_graphic + 2);
	
	/* 发送 */
	return UI_Send_Data(client_tx_buf, 5 + 2 + LEN_INTERACT_draw_two_graphic + 2);
}

/**
	* @brief  发送绘制五个图形帧数据
  * @param  
  * @retval 
  */
uint8_t client_send_five_graphic(ext_graphic_five_data_t data)
{
	/* 帧头 */
	data.txFrameHeader.sof = 0xA5;
	data.txFrameHeader.data_length = LEN_INTERACT_draw_five_graphic;
	data.txFrameHeader.seq = 0;
	memcpy(client_tx_buf, &data.txFrameHeader, 4);
	Append_CRC8_Check_Sum(client_tx_buf, 5);
	
	/* 命令码ID */
	data.CmdID = 0x301;
	memcpy(&client_tx_buf[5], (void*)&data.CmdID, 2);
	
	/* 数据段 */
	data.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_five_graphic;
	data.dataFrameHeader.send_ID = client_info.robot_id;
	data.dataFrameHeader.receiver_ID = client_info.client_id;
	memcpy(&client_tx_buf[7], &data.dataFrameHeader, 6);
	memcpy(&client_tx_buf[13], data.clientData, 15*5);
	
	/* 帧尾 */
	Append_CRC16_Check_Sum(client_tx_buf, 5 + 2 + LEN_INTERACT_draw_five_graphic + 2);
	
	/* 发送 */
	return UI_Send_Data(client_tx_buf, 5 + 2 + LEN_INTERACT_draw_five_graphic + 2);
}

/**
	* @brief  发送绘制七个图形帧数据
  * @param  
  * @retval 
  */
uint8_t client_send_seven_graphic(ext_graphic_seven_data_t data)
{
	/* 帧头 */
	data.txFrameHeader.sof = 0xA5;
	data.txFrameHeader.data_length = LEN_INTERACT_draw_seven_graphic;
	data.txFrameHeader.seq = 0;
	memcpy(client_tx_buf, &data.txFrameHeader, 4);
	Append_CRC8_Check_Sum(client_tx_buf, 5);
	
	/* 命令码ID */
	data.CmdID = 0x301;
	memcpy(&client_tx_buf[5], (void*)&data.CmdID, 2);
	
	/* 数据段 */
	data.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_seven_graphic;
	data.dataFrameHeader.send_ID = client_info.robot_id;
	data.dataFrameHeader.receiver_ID = client_info.client_id;
	memcpy(&client_tx_buf[7], &data.dataFrameHeader, 6);
	memcpy(&client_tx_buf[13], data.clientData, 15*7);
	
	/* 帧尾 */
	Append_CRC16_Check_Sum(client_tx_buf, 5 + 2 + LEN_INTERACT_draw_seven_graphic + 2);
	
	/* 发送 */
	return UI_Send_Data(client_tx_buf, 5 + 2 + LEN_INTERACT_draw_seven_graphic + 2);
}

/**
	* @brief  发送绘制字符帧数据
  * @param  
  * @retval 
  */
uint8_t client_send_char(ext_charstring_data_t data)
{
	/* 帧头 */
	data.txFrameHeader.sof = 0xA5;
	data.txFrameHeader.data_length = LEN_INTERACT_draw_char_graphic;
	data.txFrameHeader.seq = 0;
	memcpy(client_tx_buf, &data.txFrameHeader, 4);
	Append_CRC8_Check_Sum(client_tx_buf, 5);
	
	/* 命令码ID */
	data.CmdID = 0x301;
	memcpy(&client_tx_buf[5], (void*)&data.CmdID, 2);
	
	/* 数据段 */
	data.dataFrameHeader.data_cmd_id = INTERACT_ID_draw_char_graphic;
	data.dataFrameHeader.send_ID = client_info.robot_id;
	data.dataFrameHeader.receiver_ID = client_info.client_id;
	memcpy(&client_tx_buf[7], &data.dataFrameHeader, 6);
	memcpy(&client_tx_buf[13], &data.clientData.grapic_data_struct, 15);
	memcpy(&client_tx_buf[28], data.clientData.data, 30);
	
	/* 帧尾 */
	Append_CRC16_Check_Sum(client_tx_buf, 5 + 2 + LEN_INTERACT_draw_char_graphic + 2);
	
	/* 发送 */
	return UI_Send_Data(client_tx_buf, 5 + 2 + LEN_INTERACT_draw_char_graphic + 2);
}
/*-------------------- 发送数据帧 end   --------------------*/


/**
	* @brief  串口发送数据
  * @param  
  * @retval 
  */
uint8_t UI_Send_Data(uint8_t *txbuf, uint16_t length)
{
	return HAL_UART_Transmit_DMA(&huart5, txbuf, length);
}
