下主控代码高度封装

CAN2传入参数	底盘坐标系front、right、cycle、power和一些标志位，仅一个数据包master_rx_t

CAN2任务		底盘控制任务、超电交互
			串口中断转发裁判数据
			心跳检测

// 变速小陀螺？