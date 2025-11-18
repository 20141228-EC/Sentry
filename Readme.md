# 2025_CYR

## dev1分支--全国赛哨兵代码

### sentry-up/mid/down 分别为上中下主控代码（均为c板代码）

### 对于代码的解释以及注意事项：  

sentry_up   
    # sentry_up 项目结构

```
sentry_up
│
├── TaskLayer                  # 任务层
│
├── Module                     # 模块层
│   ├── Robot.c                # 机器人整体
│   ├── Chassis.c              # 底盘
│   ├── Gimbal.c               # 云台
│   └── Booster.c              # 发射
│
├── Service                    # 命令的软件驱动层
│
├── DeviceLayer                # 设备层
│   ├── device.c               # 设备软件初始化
│   ├── motor.c                # 定义电机
│   ├── rc_sensor.c            # 定义遥控器
│   ├── imu_sensor.c           # 定义陀螺仪
│   └── Slave.c                # 主从机设备
│
├── Device/Imu                 # IMU 的驱动、解算
│
├── HardwareLayer              # 电机驱动层
│
├── ProtocolLayer              # 协议层(视觉、电机、遥控器协议等)
│
├── AlgorithmLayer             # 算法
│
├── DriverLayer                # 单片机外设驱动层
│
└── ConfigLayer                # 一些方便调试的宏定义(💩)
```

  sentry_mid 和sentry_down结构相似

  1. 布线过程最好是边布线边测试电机在线状态，不然找掉线很痛苦，然后确认在线的电机一定要固定好
  2. 在调车过程中突然发现控制异常（电机高频震动，异响），可能是负载的原因或者多个主控同时控制电机导致的
  3. 第一次上电给电机电流输出的时候一定要注意好pid极性，最好刚开始小电流输出确认，避免疯车
  4. daplink无线调试对于哨兵十分重要，强烈建议买一个好一点的daplink，或者自己有能力的话可以自研无线烧录

### 裁判系统的一点解释（做出的解释仅仅只是我这个赛季的版本，新赛季需要详细根据规则和串口手册进行一些修改）：

1.自身状态相关数据（自身血量，自身发射热量限制，自身底盘功率限制等），这些串口数据中部分是可以不连接裁判系统就可以检查是否读取正常的，主要为自身状态相关数据（自身血量，自身发射热量限制，自身底盘功率限制等），可以先读取这些数据，并且理解代码来作为写裁判系统代码的一个开头；   
2.然后就是其他接收的数据（比赛进行阶段，友方血量，经济情况等），这些需要连裁判系统才会有数据，需要及时的连裁判系统早点测好，这里犯错很可能导致很严重的错误   
3.上面两点其他兵种都有，然后哨兵稍微独特的就是决策发送部分（哨兵是否立即复活，哨兵是否要远程买弹，买血等），然后这里需要注意的一点是，发送报文id有两个id，一个是外层的id(0x301)，一个是子内容id(0x120)，详细的看代码，在子内容中修改需要实现的决策信息；
```
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
```
4.云台手发送坐标位置（id为0x303），云台手可以花费金币发送一次这个信息：   
发送方式一：   
① 点击己方机器人头像；   
②（可选）按下一个键盘按键或点击对方机器人头像；   
③点击小地图任意位置。该方式向己方选定的机器人发送地图坐标数据，若点击对方机器人头像，则以目标机器人 ID 代替坐标数据。   
发送方式二：   
①（可选）按下一个键盘按键或点击对方机器人头像；   
②点击小地图任意位置。该方式向己方所有机器人发送地图坐标数据，若点击对方机器人头像，则以目标机器人 ID 代替坐标数据。   
这个做好接口，和云台手商量好需要哪些键实现什么功能。   
5.哨兵可以向云台手描绘自己规划的路径，详细内容看id0x307，这个赛季没有实现这个功能，是因为中主控获取到决策的路径信息，路径信息需要50个点的xy坐标数据，中下主控只能can通信，这样的数据量对can负载要求过高，所以没有实现，如果之后想实现，希望可以滑环走串口，然后串口传递这些数据，避免can负载过高   


### 对于哨兵电控工作的反思：  

1.作为电控需要尽可能的把自己的工作做到稳定可靠，在这个基础上再去拓展学习其他东西（这个放在第一点尤为重要）   
2.哨兵电控需要对接的视觉队友较多，需要多加沟通，积极配合   
3.哨兵电控和视觉的通信问题尤其重要，无论是视觉还是导航，掉一个就会导致无法正常比赛，所以务必在比赛前模拟完整的比赛情况（犯错小记：平时调试视觉习惯开着显示屏看到代码正常运行再拔去显示屏接口调试，结果比赛才发现不接显示屏代码跑不起来的情况），确保视觉电控代码运行正常，确保机器人硬件牢靠   
4.对于哨兵电控可以在功率控制，打滑检测以及力矩控制上进一步提高   
5.板间通信很多，所以建议可以用daplink来debug查看一块主控，然后再用jlink来debug查看另外一块准备调试板间通信的主控，可以实时的查看板间通信是否正常

#### 对于代码有疑问或者像进一步了解一些细节的，可以来加我的微信：ynqjez（看到就回复，包亲切的嘿嘿）

