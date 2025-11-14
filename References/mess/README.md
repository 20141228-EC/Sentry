# **标志位统计：**

1. 换头标志（底盘取反）
2. ...

## 日记

### 2023.10.22

   **Rebuild Github Repository**

1. 设备层 添加云台、云台yaw、pitch轴电机（均为6020）
2. 控制层 添加云台初始化、pid、pid_position、更新、发送，含遥控（ch1 pitch / ch2 yaw）

### 2023.10.23

1. 写三种限幅，测试英雄（但能收不发）
2. 配置 codespace ，上课平板也能写代码🤣

### 2023.10.24

1. 将 gimbal.cal() 分为 自动跟随（内含机械模式、陀螺仪模式）、手动操控模式（内含RC、PC_INPUT）

2. 新增 chassis.follow_imu_cal ，并入 chassis.updata

3. 云台跟随底盘pid：
   
    ​    6020回归机械中值；

4. 底盘跟随云台pid：
   
    ​    **位置环：**目标为 imu角 - yaw角 = 0；
   
    ​    **速度环：**fdb_angle，底盘电机速度。                

5. 尝试写陀螺仪解算，积分求角

### 2023.10.25

1. 阅读imu相关代码，角度积分值为 imu.data.rpy.( yaw / pitch ) 单位为°

2. 基本完成了 gimbal 相关 cal（计算）函数：
   
    *void* chas_motor_follow_imu_cal(*struct* *motor_class_t* **motor*)    *//控制底盘电机，底盘跟随云台*
   
    *void* gimbal_imu_keep_horizon_cal(*void*)                    *//控制云台pitch电机，始终保持水平*
   
    👆仅作计算，不进行tx_buf赋值，在👇统一进行赋值
   
    *void* gimbal_motor_cal(*struct* *motor_class_t* **motor*)            *//云台双电机计算*

### 2023.10.26

1. 电机发疯
2. 关控保护修正

### 2023.10.27

1. 底盘pid重调
2. debug陀螺仪pid

### 2023.11.5

1. vscode 逆天单行注释快捷键，必须自己更改为NumPad Divide，如下图
   
   ![@](E:\Program%20Files\MarkText\images\vsc_comment.png)

## 2023.11 程序结构整理

1. timer

2. 高于模块的控制层（updata -> 升一层）接口+功能

3. include整理（高含低、平级不包含（-f在.c里面包含））

4. 结构
   
   **设备** --- **模块**(只做定义，不做控制) --- 模式&&**控制**(含不同模<mark>块</mark>updata，以及不同模<mark>式</mark>下统一的发送) --- **主控**(以车型为单位: 控制、监视、测试态)&?任务 --- **任务中心**(选车，选模式(引入预编译，再通过宏定义的值作为标志位切换车和模式))
   
   即：
   
   <mark>**设备** --- **模块** --- **控制(ALGO)** --- **车型主控** --- **任务中心**</mark>
   
   *其实预编译已经能让其他文件到达高于任务中心的层级了！*

5. 跳脱于结构的内容：
   
   configure 调试参数宏定义,
   
    rp_math, 常用函数库(常用功能要泛化, 带参函数),
   
    紧急添加功能的方案！

## 2023.11.18

motor[CAN1/2_F/B_tx].tx(&motor[CAN_1/2_F/B_tx], CAN_1/2_F/B_tx_buf)

发送重整

问题：

1. ”发送用“ 并无电机型号信息；

2. “发送用” ~~无接收id，的接收函数不链接，不清楚是否会导致接收函数空指针。~~    经测试，会导致HardFault，故直接链接并复用接收id。

3. 初始化位置暂且放在 RM_Chassis_Motor_init 函数内（不过电机初始化函数内有先DE_INIT，可以重复初始化电机）

4. 电机发送也暂且放在 RM_Chassis_Motor_tx   函数内

继承

模块结构体继承电机结构体，在电机（以及其他）结构体的声明过程中，变量统一用指针传递

```c
typedef struct motor_9015_t
{
    motor_9015_base_info_t *base_info;                //pointer
    motor_9015_info_t      *info;
    //motor_9015_pid_t       *pid;
    PID_Config_t pid;                                //not pointer
    PID_Config_t pid_position;
```

而且电机的定义并不在电机c文件内进行，而是声明（？）

extern motor_9015_t           motor_9015_structure;

所有采取循环的电机list相关函数均要修改，不要使用循环，不方便改list序号

对于大yaw轴，由于结构体不一样，algo部分的修改：返回值该，目标值不动，最后在sentinel task发送

龙说的文件夹整理

pid修改：限幅用函数；角度区间不固定，精度；

---

## 疑问

1. rc.init必须在usart之前，why the fuck？？

2. 瓴控电机问题：无法接收，且发0有阻力；~~莫名高频抖动~~（原因是位置环pid算法范围是8192，得映射过去）

3. CAN_tx数组定义为？_t是否有影响

4. 对于双环 PID ，feedback 正负是否影响速度环方向（陀螺仪yaw轴控制）

5. **bug：为何将motor.rx.angle赋值后给其他变量，其他变量置零？？？并且在并列情况下（yaw/pitch），另一个变量就不会？**    ----暂时将cal移到if内；限幅暂时弃用
   
   ```c
   *//机械模式：云台跟随底盘（yaw6020动）*
   
     if(gimbal.MODE_AUTO == MECHANICAL_AUTO)
   
     {
   
   ​    *//目标值接口   //反馈值接口*
   
   ​    if  (motor_num == 8)
   
   ​    {
   
   ​      Target_Ang = YAW_MECH_ANG;
   
   ​      *//Now_Ang[GIMB_Y] = motor[GIMB_Y].data.rx_raw_data.angle;*
   
   ​      fdb_Ang = PID_Output_Cal_Position(&*motor*->pid_position, Target_Ang, *motor*->data.rx_raw_data.angle, 8192);
   
   ​      gimbal_yaw_tx_buf[3] = PID_Output_Cal(&*motor*->pid, fdb_Ang, *motor*->data.rx_raw_data.speed, 13000);
   
   ​    }
   
   ​    if  (motor_num == 5)
   
   ​    {
   
   ​      Target_Ang = PITCH_MECH_ANG;
   
   ​    ***//    now_pitch_angle = motor[GIMB_P].data.rx_raw_data.angle;***
   
   ​    ***//     Now_Ang[GIMB_P] = motor[GIMB_P].data.rx_raw_data.angle;***
   
   ​      fdb_Ang = PID_Output_Cal_Position(&*motor*->pid_position, Target_Ang, *motor*->data.rx_raw_data.angle, 8192);
   
   ​      gimbal_pitch_tx_buf[0] = PITCH_pre_current + PID_Output_Cal(&*motor*->pid, fdb_Ang, *motor*->data.rx_raw_data.speed, PITCH_pid_LMT);
   
   ​    }
   
     }
   ```

6. **bug：**我的偏差角计算 *angang -=  motor[GIMB_Y].data.rx_raw_data.angle -  YAW_MECH_ANG;* 放在底盘函数内，数值会根据底盘电机数跳变，原因未知，故挪到 gimbal updata 小陀螺模式下直接计算

7. **bug：**在传入参数为motor的函数内，使用 motor[其他电机] 似乎会自动重置到当前电机

8. **bug：**在7758 ~ 8192阶段，（target -180）陀螺仪模式的就近跟随异常猛烈？？？     --- show d angle是直接用0~8192映射360度的，当处于这个范围时，角度值并不为-180+，而是正180+直到200（测），故此阶段目标也应该是180

9. ~~陀螺仪角度跳变问题如何解决：从-180 -> +180，此时pid会出问题，导致~~  
   
   ```c
   if(Target_Ang_imu_yaw > 8192)
               {
                   Target_Ang_imu_yaw -=8192;  //不丢弃这一时刻的拨杆值
               }
   
               if(Target_Ang_imu_yaw < 0)
               {
                   Target_Ang_imu_yaw += 8192; //不丢弃这一时刻的拨杆值
               }
   ```

10. ~~陀螺仪到底是世界坐标系还是载体坐标系~~
    
    若是世界坐标系，如何解释yaw 0 指向车头 --more
    
    若是载体坐标系，为何yaw 0 底盘运动时固定
    
    ​    **世界坐标系，但上电初始位置为0，故上电时得用机械中值**

11. ~~arm math三角函数使用？~~

12. ~~底盘跟随云台与底盘本身移动的冲突如何解决？->会上说的合成速度？~~

13. 各个pid的位置环是否限幅？

14. ~~陀螺仪零漂是否能通过与yaw电机比对，以便数据矫正？~~ 无法，6020本身的坐标系不是世界坐标系（能否将其也收敛世界坐标？）

15. 浮点整形转换

16. ~~limit 和 dial 的发送数组是否要与pitch公用（CAN2）~~  已进行发送数组规范，未进行统一发送

17. ...

## 提醒

1. 多主控板，只有一个直接接受遥控器控制！

2. 机械切陀螺仪要将目标初值设定为陀螺仪的当前值，否则目标初值仍为两个机械中值

3. 换debugger时记得勾选reset and run

4. 瓴控电机9015不要达到1000电流发送（包括限幅也不要写道1000）会导致高频振动

5. 同级引用防c文件：**若放头文件，又会被其他文件继续引用**

6. 宏定义看清楚，别和枚举混 YAW_MECH_ANG vs MECHANICAL_AUTO

7. imu.init不要写，似乎会导致野指针

8. pid 限幅修改：速度环限幅是限制电流，位置环限幅是限制速度

9. 连续的 if 判断注意会不会互相写入
   
   ```c
           if(Target_Ang_imu_yaw >= 8192)
           {
               Target_Ang_imu_yaw = 0;
           }
           if(Target_Ang_imu_yaw < 0)    //若 <= 0，则上判断失效
           {
               Target_Ang_imu_yaw = 8192;
           }
   ```

10. pid target = 0 与 tx_buf = 0 是不一样的，前者发送可能为负

11. RM pid init是对全体！！！！要改

12. 关控保护不用归中，而是要不给速度！

13. 标志位要回置

14. 电机debug没转可能是没有pid值

15. 电机只发不收 / 乱转 可能是CAN接错

16. debug：先拆开出来（面向过程），再根据过程修改，每处改成单电机，注释，取注

17. 电机发送值取反（如不同摆放的需要），须在PID之前，否则误差无限放大，疯转

18. tx函数内已经有清零，不用memset

19. tx_buf要申请 [4] 个空间，因为tx内循环判断为4

20. 不同CAN总线不要用同一个发送数组，因为tx函数内有发送后清零（同 CAN **单条** 发送即为全电机发送），多条发送后必清零

21. 改电机id之后，发送数组标号检查，还有一堆motor_number

22. **“因为重心不在支点上，Ki给大”** *--JSJ*

23. CAN 总线负载率不宜过高，底盘电机发送只一个

24. \ 分行后面不能有空格！

25. ".INPUT_MODE" 是 chassis 结构体成员，不要老是单独写

26. ...

----

## 待办

- [ ] 各电机、模块的 heart_beat

- [x] imu 右摇杆无法换头

- [x] 陀螺仪模式目标角没在机械模式下清零

- [x] 死区

- [x] CAN线发送数组整理

- [x] ，以及统一发送

- [ ] 一些全局变量的清零

- [x] 云台指令换头 imu跟随换头坏；pitch失控

- [x] 关控失效：原因：还没发零就return

- [ ] 机械模式pid重调

- [x] S2、thumbwheel按键映射

- [ ] task pid ?

- [x] 因为中值方向反了，所以yaw的遥控器映射加值要改

- [x] S2切换时，底盘归位两问题

- [x] 就近为头

- [x] pitch 变方向不同限幅，搁置，怪              ------->  前馈

- [x] 测陀螺仪的各角度值

- [ ] 各个pid的位置环是否限幅？

- [ ] 结构体继承（把电机结构体继承到所属模块内）

- [x] 机械中值待定

- [x] 角度超限

- [x] 遥控角度映射方式：持续往遥控方向旋转（单环速度） or  角度映射（双环角度）        ------------> 双环，但持续

- [x] ？底盘是否把多发送直接改成单发送？ 改单发送，否则与云台yaw不明原因冲突

- [ ] Target_Ang是否要分开，不然一大堆判断                                                                  --> 没做：轮流计算不需要，但在分计算内就得补上标志位

- [x] tx不够泛化，对于陀螺仪模式没有提供接口（pid返回值）

- [x] chassis rotate 对于角度pid的运动学解算

- [ ] 交叉include的事怎么解决？

- [x] 重调底盘pid

- [ ] ...

----

## **测试机构：半舵底盘**

`调底盘`

1. bug：

`焊线和接线`

1. 滑环焊线，先上滑环。电源线5合1，先热缩，后粗热缩，最后才接头焊接打胶，剪线长度要与分电板沟通好。

2. 滑环CAN线，先细热缩后中热缩，CAN头少拔会松。

3. 分电板和电池，考虑其他线材长度，可选用单电池双分电板，其中板二用板一供电和通讯，接线同电机供电和通讯。

`与机械沟通：`

1. 在机械设计某些板件（如涉及到电机结构，哪一部分旋转）的时候，需要详细讨论可行性（比如6020中孔塞滑环，但孔壁是随着舵轮方向旋转的，此时滑环的摆放）

2. 电机摆放朝向（电源和CAN线）

3. 线材摩擦，电调距离调整（机械可能不知道某器材具体大小以及线长）（线长可能影响是否与附近板材或者电机摩擦）

4. 告诉机械底盘布置：电池（可能不止一个），电池架，分电板（不止一个），裁判系统，超级电容，主控板。

------

## 视觉交互串口流程

> **Copilot：**
> 
>         这个文件包含了一些函数，主要用于机器人视觉系统的数据处理和通信。以下是每个函数的功能和执行顺序：
> 
> 1. `VISION_GET_DATA`函数：用于解析从串口3接收到的数据，并将其存储在`vision_rx_pack`结构体中。
> 
> 2. `VISION_SEND_DATA`函数：用于将`vision_tx_pack`结构体中的数据通过串口3发送出去。
> 
> 3. `VISION_CHECK`函数：用于检查从串口3接收到的数据是否有效，并根据数据内容更新`vision_rx_pack`结构体中的数据。
> 
> 4. `VISION_UPDATE`函数：用于更新`vision_rx_pack`结构体中的数据。
> 
> 5. `USART3_rxDataHandler`函数：用于处理串口3接收中断，并调用`VISION_GET_DATA`函数。
> 
> 6. `Vision_Heart`函数：用于检测视觉系统是否掉线，并根据检测结果更新`vision_structure`结构体中的数据。
> 
> 7. `vision_task`函数：用于处理视觉系统的数据和通信。首先根据机器人的模式和裁判系统的指令更新`vision_tx_pack`结构体中的数据，然后调用`VISION_SEND_DATA`函数将数据发送出去。最后根据视觉系统的状态更新`vision_rx_pack`结构体中的数据。
> 
> 总体来说，这个文件的功能是将机器人的视觉系统与其他系统进行通信，并处理从其他系统接收到的数据。

## **目录：**

1. **串口设置**

2. **通信数据包的格式以及CRC校验**

3. **具体交互通信流程**

## 正文

1. ### 串口设置

2. ### 通信数据包的格式以及CRC校验
   
   #### a. 数据包格式
   
   最外层数据包结构体包含三个部分：**接收数据包、发送数据包** 和 **状态及信息包。**
   
   ```c
   typedef  struct 
   {
   
       vision_rx_packet_t *rx_pack;           // 接收数据包
       vision_tx_packet_t *tx_pack;        // 发送数据包
       vision_state_t      state;            // 状态及信息包
   
   }vision_t;
   ```
   
    **接收包、发送包**以及**裁判视觉包**结构体，都由三个部分（**帧头、数据、帧尾**）共同构成本数据**帧**

```c
typedef __packed struct 
{
    vision_frame_header_t   FrameHeader;    // 帧头
    vision_rx_data_t        RxData;         // 数据
    vision_frame_tailer_t   FrameTailer;    // 帧尾    

} vision_rx_packet_t;                       
//} vision_tx_packet_t;
//} Jud2Vis_tx_packet_t;
```

1）其中**帧头**包含：

            同步头（固定格式，并且配合 “ __packed ” 以一个字节对齐变量）

            命令码（指明要进行哪一种视觉指令，如：巡逻模式 ... ，具体参考 vision_cmd_id_t 枚举）

            **CRC8校验码：** 通过 *三个函数（详见 b.）*  计算后得到**帧头**的校验码（1字节）存放在变量 crc8 中

                                        ***注意：此处 crc8 校验对象（传入参数）是帧头（数组）*

```c
/* 帧头格式 */
typedef __packed struct
{
    uint8_t                  sof;        // 同步头
    vision_cmd_id_t          cmd_id;     // 命令码
    uint8_t                  crc8;       // CRC8校验码
} vision_frame_heade_tailer_t;
```

   

2）其中**数据**包含：

            比赛中各种实时数据，如电机参数，又或是血量、弹丸余量等等，为**自行定义的传输内容**

```c
/* 发送数据包 */
typedef __packed struct 
{
    /*左云台*/
    int16_t     L_yaw_angle;
    int16_t     L_pitch_angle;

    /*右云台*/
    int16_t     R_yaw_angle;
    int16_t     R_pitch_angle;

    /*大YAW轴*/
    int16_t     B_yaw_angle;

    uint8_t   my_color;
    /*比赛信息*/
    uint8_t   game_progress;

    /*比赛数据*/
    uint8_t remain_time;
    uint8_t remain_bullet;
    uint8_t friendly_outpose_HP;
    uint8_t my_HP;
    uint8_t enemy_hero_HP;
    uint8_t enemy_infantry3_HP;
    uint8_t enemy_infantry4_HP;
    uint8_t enemy_sentry_HP;
    uint8_t enemy_outpose_HP;

    uint8_t engine_hit_enable;
    uint8_t hero_hit_enable;

}vision_tx_data_t;
```

   

3）其中**帧尾**包含：

            CRC16校验码：通过 *三个函数（详见 b.）* 计算后得到**完整帧**的校验码（2字节）存放在变量 crc16 中

                                        ***注意：此处 crc16 校验对象（传入参数）是完整帧（数组）*

```c
/* 帧尾格式 */
typedef __packed struct 
{
    uint16_t crc16;                    // CRC16校验码
} vision_frame_tailer_t;
```

#### b. CRC校验过程

   **CRC校验本质上是选取一个合适的除数，要进行校验的数据是被除数，然后做模2除法，得到的余数就是CRC校验值。**    ----网上随便找的

​    1）最外层函数：**Append_CRC8_Check_Sum** （和 Append_CRC16_Check_Sum，以下以 crc8 为例）

​    传入参数：被校验计算的**数组地址**（如帧头、完整帧）、**长度**（如帧头长度、发送包整包长度）

​    过程：

​        检查数据非空以及长度合法；

​        调用 Get_CRC8_Check_Sum 函数直接处理帧头数组（或完整帧），计算校验码，填入 crc8 校验结果值到帧头（或帧尾）结构体里的 “ crc8 ” 变量

​        把 帧头（或帧尾）**结构体里的** “ crc8（或16） ” 变量 放到发送数组的最后一位（?）

​            （?）（当在填帧头时，crc8 是末位，在填帧尾时，crc16 也是末位）

```c
/*
** Descriptions: append CRC8 to the end of data
** Input: Data to CRC and append,Stream length = Data + checksum
** Output: True or False (CRC Verify Result)
*/
void Append_CRC8_Check_Sum( uint8_t *pchMessage, uint16_t dwLength)
{
        uint8_t ucCRC = 0;

        if (pchMessage == 0 || dwLength <= 2)                //检查输入的数据是否为空或长度是否小于等于 2，如果是，则直接返回。
        {
              return;
        }

        ucCRC = Get_CRC8_Check_Sum( (uint8_t *)pchMessage, dwLength-1, CRC8_INIT);        //内层函数

        pchMessage[dwLength-1] = ucCRC;
}
```

​    2）内层函数：**Get_CRC8_Check_Sum**

​    传入参数：被校验计算的**数组地址**（如帧头、完整帧）、**长度**（如帧头长度、发送包整包长度）、帧头（或帧尾）**结构体里的** “ crc8（或16） ” 变量

​    过程：直接处理帧头数组（或完整帧），计算校验码，填入 crc8 校验结果值到帧头（或帧尾）结构体里的 “ crc8 ” 变量

​    返回：帧头（或帧尾）**结构体里的** “ crc8（或16） ” 变量

```c
/* CRC校验 */

uint8_t Get_CRC8_Check_Sum( uint8_t *pchMessage, uint16_t dwLength, uint8_t ucCRC8 )
{
        uint8_t ucIndex;

        while (dwLength--)
        {
                ucIndex = ucCRC8^(*pchMessage++);
                ucCRC8 = CRC8_TAB[ucIndex];
        }
        return ucCRC8;
}
```

3. ### 具体交互通信流程
   
    过程：
   
    ​    失联检测；
   
    ​    用 memcpy 进行数组搬运（缓冲数组）
   
    ​    对数组增加 crc 校验位
   
    ​    DMA 搬运，搬运后清零和状态判断
   
   ```c
   /**
     * @Name    Vision_SendData
     * @brief   与小电脑通讯，CRC校验数据肯定要改
     * @param   None                              
     * @retval
     * @author  HWX
     * @Date    2022-10-21
   **/
   bool Vision_SendData(void)
   {
       /*判断是否掉线*/
       if(vision_structure.state.offline_cnt++ >= vision_structure.state.offline_max_cnt)
       {
           vision_structure.state.offline_cnt--;
           vision_structure.state.work_state = VISION_OFFLINE;
       }
       else
       {
           vision_structure.state.work_state = VISION_ONLINE;
       }
   
       /*数据发送*/    
       memcpy(vision_txBuf, &vision_tx_pack, sizeof(vision_tx_packet_t));
   
       /*增加CRC校验位*/
       Append_CRC8_Check_Sum(vision_txBuf, LEN_FRAME_HEADER);
       Append_CRC16_Check_Sum(vision_txBuf, LEN_VISION_TX_PACKET);
   ```
   
        if(HAL_UART_Transmit_DMA(&huart3,vision_txBuf,sizeof(vision_tx_packet_t)) == HAL_OK)
        {
            memset(&vision_tx_pack,0,sizeof(vision_tx_pack));
            return true;
        }
        else
        {
            memset(&vision_tx_pack,0,sizeof(vision_tx_pack));
            return false;
        }
   
    }
   
   ```
   **Copilot：**
   
   ​    根据上下文代码，这个函数的作用是将视觉数据发送到另一个设备。在函数中，首先会检查视觉设备是否掉线，如果掉线则将工作状态设置为离线，否则    设置为在线。然后将视觉数据打包到一个缓冲区中，并添加CRC校验位。最后，使用DMA方式将缓冲区中的数据发送到UART3串口。如果发送成功，则    清空视觉数据缓冲区并返回true，否则清空视觉数据缓冲区并返回false。根据上下文代码，这个函数的作用是将视觉数据发送到另一个设备。在函数中，    首先会检查视觉设备是否掉线，如果掉线则将工作状态设置为离线，否则设置为在线。然后将视觉数据打包到一个缓冲区中，并添加CRC校验位。最    后，使用DMA方式将缓冲区中的数据发送到UART3串口。如果发送成功，则清空视觉数据缓冲区并返回true，否则清空视觉数据缓冲区并返回false。
   ```

​    memcpy 的数组越界问题

----

## 会议记录 2023.10.24

1. **Debug启停：三种情况：**

​    task.c（正常）；

​    HardFault_Handler（野指针，声明未调用）；

​    bmi_delay（陀螺仪初始化没过）（实际调用函数：bmi->init）。

​    局部标志变量返回：

​        -9（spi收/发无效）；

​        -2（i2c收/发无效）；

​        -3（陀螺仪自身问题）。

​        Bmi初始化详细流程于往年报告。

2. **陀螺仪解算**

​        （参考文档：博世官网）

​    1） 读取（imu_updata）

​        六轴对应的六个变量；

​        温度（使用加热电阻控制在一个相对固定的值，使得零漂角度基本固定，以便纠正）

​    2） 原始数据（加速度计）低通滤波

​        （陀螺仪噪声主要集中在低频；而加速度计噪声主要在高频）

​        （安装方向导致坐标系方向问题：旋转矩阵；直接取反）

​        对原始数据进行取反或旋转，注意要在解算之前，否则矫正错误。

​    3） 数据解算

​        a. 赋值，单位换算；若三轴加速度均为零，丢弃本次数据

​        b. 陀螺仪读取加速度合成为重力加速度（1）

​        c. 自定义坐标系，计算当前坐标系重力加速度（2）

​        d. 计算两个重力加速度的误差，并反馈到几个角速度上（Kp变速比例系数，上电大值高速收敛；稳定后小值防止震荡），使得自身坐标系得到修正，z            轴零漂问题得到修正。由于重力加速度垂直于x、y轴，故yaw轴零漂无法修正。

​    4） 四元数反解欧拉角

3. **小陀螺移动**：旋转速度与直行速度的矢量合成

4. **机械模式与陀螺仪模式切换**（预期：云台不动）（标志位）

5. **键盘模式**

​    （使用中间变量，而不用原始数据，解除代码耦合）

6. **发射机构标志位管理**

​    连发、单发、开弹仓（云台平，停止发射）、清空热量（高射频，20Hz左右）

​    标志位文件：module.c 在此文件内写；而其他文件只读不写。

7. **堵转处理判断**：有发送，无速度。

------

# 国庆学习内容总结

## 任务执行

1. 环境：cubemx，jlink旧驱动，添加FPU，主频相关，jscope安装
2. 熟悉了git bash，成功建库并链接本地仓库，熟悉各种git指令
3. 接触FreeRTOS任务管理，接触任务虚函数，而后也自建Task_Center.c来重写任务虚函数
4. 接触到FreeRTOS下的osDelay函数，并与阻塞式的HAL_Delay做出分别，理解了osDelay不影响高低优先级任务之间的（伪）并行（快速优先交替）
5. 熟悉板上串口，用重写printf和scanf的方式实现USART串口收发
6. 了解DMA独立于CPU进行数据转移的"搬运工"作用，熟悉DMA各项配置，理解DMA双缓冲机制，双缓存空间交替使用
7. 了解陀螺仪，理解模板代码多层级面向对象思想，并移植至工程中，启用了陀螺仪更新（SPI），在jscope上读取陀螺仪数据
8. 增加低通滤波，使得高频突变滤去，零漂是物理条件导致（温度、元件状态、噪声）
9. M3508电机+C620电调，通过查阅手册了解收发协议，设备ID，地址等，对官方文档进行移植，写出收发函数，补充CAN_Init函数以及一些配置宏定义实现电机基本控制
10. 查阅资料，自己写出PID结构体和计算函数，实现速度环单环、速度位置双环控制

****

## Debug 记录

**1. 陀螺仪模板代码层级问题**

大体通过（设备/协议）结构体分层，类似于c++的类；函数通过函数指针，内嵌于结构体中，在不同文件或层级之间传递

对代码可读性的优化：（如工作状态）宏定义，枚举定义枚举变量

**2. DSP库大量报错**

1.换源，三个都试试【取消相应定义和工程配置路径】

​    keil自带（latest version）CMSIS

​    Project/Drivers/CMSIS/DSP

​    User/CMSIS/DSP

2.最后是User内CMISIS覆盖Drivers内CMSIS有效，把替换后的Drivers压缩，备份

3.CubeMX每次regenerate就有概率需要替换（改写了DSP库内某一个头文件）

**3. 电机首次CAN通信未连接 debug：**

1.keil debug：

​        CAN中断未进入---->补齐CAN初始化函数（非HAL，为自写）---->仍旧没有返回数据

2.物理检查：怀疑是CAN接口无效，更换为CAN2，成功连接

​        

**4. 电机突发失联（收发均失效）debug：**

1.物理检查，接线，电源开关，电调ID

2.keil debug：

​    设置断点：

​        发送函数、接收函数：均没进入，发现发送函数调用HAL_AddTxMsg行被误注释【注释记得检查】----->仍旧失联

​        接收函数对应中断：没进---->往初始化中断找

​        协议（CAN）初始化函数：没进，检查发现误删CAN_Init函数，推测是CubeMX覆盖，故更改位置 ----->电机重连---->重连后出现电机间歇性运转

3.检查失联检测函数：发现在返回后的状态值进行判断（在主函数写的）时，osDelay 1s，【不可以在电机收发循环随意增添Delay】删除---->电机正常收发

**5. 云台 yaw 轴有发无力：** 

1. 电机 id（tx、rx）、CAN线
2. pid是否为0
3. 写入发送数组是否被篡改 / 是否越界：全局查找发送数组，是否有多处赋值
4. 发送邮箱、发送帧的值是否被篡改：debug 看底层发送，监视，单步
5. **以上均无问题**，尝试在任务函数直接发送，无力：看同级（各个设备更新函数）注释排查，发现为底盘更新函数导致。
6. chassis.updata() 函数内，注释块，发现为底盘发送导致。
7. **结果：底盘发送为循环发送同一数组（重复发送4次），并且有memset（每次发送后清零）** ---> 均删除后恢复正常

**一些理论上**

1.（速度环）PID输出直接作为电机发送

2.双环的理解：

​    外环为最终目的体现（期望位置---->结果是位置的固定）

​        位置环的目标是达到指定位置
​        速度环要达到目标，目标要达到目标‘
​        V achieve the speed that makes the target achieve its own target

**6. 23.12调老哨之：CAN接收中断不进：**

1. 现象：接收中断回调不进->接收中断（it.c）不进；其他串口（顺便配置了USART6（judge））和定时器能进。

2. 原因：CubeMX配置时钟为低速时钟，应改为高速HSE。

3. 历程：
   
   CubeMX外设配置检查；
   
   软件：初始化，中断使能，can_drv；
   
   主控：几个低负载主控都可以进中断；
   
   寄存器：hcan2实例（Instance）的寄存器：发现ESR值异常 -> 推断为时序问题 -> CubeMX配置时钟为低速时钟，应改为高速HSE。

4. 归根结底：

5. 

---

VSCode 快捷键

**2、移动行**：alt+up/down

**3、显示/隐藏左侧目录栏** ctrl + b

**4、复制当前行**：shift + alt +up/down

**5、删除当前行**：shift + ctrl + k

**6、底部栏**：ctrl + b

ctrl + shift + h : 全局查找替换

ctrl + h : 单文件查找替换

**折叠代码**： ctrl + k + 0-9 (0是完全折叠)

**25、展开代码**： ctrl + k + j (完全展开代码)

**行增加缩进:** ctrl + [

**11、行减少缩进:** ctrl + ]

----

哨兵机器人--展示定位模块安装支架区域并展示施加40N的力后支架的形变情况

哨兵机器人--展示测速模块安装区域（按照规范文件要求正确安装）

哨兵机器人--荧光充能装置安装位置展示（按照规范文件要求正确安装）

哨兵机器人--其余裁判系统安装展示或预留给裁判系统的安装位置

哨兵机器人--完整运动：平移、旋转

哨兵机器人--连续发射弹仓中的50发弹丸，攻击5米处静止的大装甲模块大小的目标，统计命中率并展示相关证明材料（如复写纸痕迹）

哨兵机器人--自动识别并分别跟随平移、旋转装甲模块，连续发射30发弹丸击打装甲模块，统计并展示命中率（如复写纸痕迹）；同时展示装甲板识别的可视化程序运行效果（可参考文末图1）

哨兵机器人--展示哨兵在比赛场地中移动、定位、避障、路径规划的自动运行效果与可视化程序运行效果，其中程序运行效果的数据与展示的机器人实际运行相对应（可参考文末图2）

哨兵机器人--展示哨兵机器人不同运行模式（如两点间巡逻/原地旋转/自动反击等），且模式可通过外部信号控制切换

交流赛 总结
1.线材稳定性：注意到底盘后二电机连接不稳定，有时会失连，原因1线材松，2负载可能太高了，得打开终端电阻
2.底盘热量限制可以再极限一些
3.原地转圈不在原地
4.舵轮中值不准，跑不直
5.布线有问题，阻碍小电脑安装

PID控制系统是一种常见的反馈控制系统，它由三个主要部分组成：比例（P）、积分（I）和微分（D）。这三个部分分别对应于系统的当前错误、过去的错误和未来的错误。

比例（P）：比例部分是当前错误的函数。如果错误很大，比例部分会产生一个大的控制信号，使系统快速接近目标。比例增益（Kp）决定了比例部分的强度。

积分（I）：积分部分是过去错误的累积。如果系统有一个持续的小错误，积分部分会逐渐增大，直到错误被消除。积分增益（Ki）决定了积分部分的强度。

微分（D）：微分部分是错误变化率的函数。如果错误正在快速增大，微分部分会产生一个大的控制信号，以防止错误过大。微分增益（Kd）决定了微分部分的强度。

i可以调整稳态误差？如何理解稳态误差

GitHub Copilot
稳态误差是指系统达到稳定状态后，系统输出与期望输出之间的差值。在理想情况下，我们希望系统的输出能够完全匹配我们的期望输出，但在实际情况下，由于各种因素（如模型误差、外部干扰等），系统的实际输出往往会与期望输出存在一定的差距，这就是稳态误差。

在PID控制系统中，积分（I）部分可以用来减小或消除稳态误差。积分部分是过去错误的累积，如果系统有一个持续的小错误（即稳态误差），积分部分会逐渐增大，直到错误被消除。因此，通过调整积分增益（Ki）的值，可以改变积分部分对稳态误差的影响，从而调整系统的稳态误差。

请注意，虽然增大积分增益可以减小稳态误差，但过大的积分增益可能会导致系统的响应过度，产生振荡。因此，在调整积分增益时，需要找到一个平衡点，既能减小稳态误差，又不会导致系统的响应过度。

改电机数组三部曲：1换enum 2换id数组内容 3换RM_MotorCANn

一个很严重的问题：宏定义乱飞，重复数值的宏定义要是被复用，寄；但是如果给他赋较大的特殊值，是否影响空间，进而影响负载

```c
void Chassis_Speed_Calculating(chassis_t *chassis, int16_t front, int16_t right, int16_t cycle)
{
	/*接口，写目标速度*/

	/*获取误差角*/
	// chassis->servo_F->base_info->angle_err = atan2((right-cycle),front) * (8192/(2*Pi));
	// chassis->servo_B->base_info->angle_err = atan2((right+cycle),front) * (8192/(2*Pi));
	int16_t F_angle_err = atan2((right-cycle),front) * (8192/(2*Pi));
	int16_t B_angle_err = atan2((right+cycle),front) * (8192/(2*Pi));

	/*就近归位（90°），处理误差角和电机速度*/
	/*前舵向电机*/
	if(F_angle_err >= -2048 && F_angle_err <= 2048)										/*|err|<90°，不做修改*/
	{
		chassis->base_info.output.motor_F_speed = sqrt(pow(front,2) + pow(right - cycle,2));
	}
	else if(F_angle_err > 2048)
	{
		F_angle_err -= 4096;															/*|err|>90°，-180°（取补取反）*/
		chassis->base_info.output.motor_F_speed = -sqrt(pow(front,2) + pow(right - cycle,2));	/*速度取反*/
	}
	else if(F_angle_err < -2048)
	{
		F_angle_err += 4096;															/*|err|>90°，+180°（取补取反）*/
		chassis->base_info.output.motor_F_speed = -sqrt(pow(front,2) + pow(right - cycle,2));	/*速度取反*/
	}
	/*后舵向电机*/
	if(B_angle_err >= -2048 && B_angle_err <= 2048)										/*|err|<90°，不做修改*/
	{
		chassis->base_info.output.motor_B_speed = sqrt(pow(front,2) + pow(right + cycle,2));
	}
	else if(B_angle_err > 2048)
	{
		B_angle_err -= 4096;															/*|err|>90°，-180°（取补取反）*/
		chassis->base_info.output.motor_B_speed = -sqrt(pow(front,2) + pow(right + cycle,2));	/*速度取反*/
	}
	else if(B_angle_err < -2048)
	{
		B_angle_err += 4096;															/*|err|>90°，+180°（取补取反）*/
		chassis->base_info.output.motor_B_speed = -sqrt(pow(front,2) + pow(right + cycle,2));	/*速度取反*/
	}

	/*电机目标角度*/
	chassis->servo_F->base_info->target_angle = SERVO_MIDDLE_FRONT + F_angle_err;
	chassis->servo_B->base_info->target_angle = SERVO_MIDDLE_BACK + B_angle_err;

}
```

不好的中值会导致过半圈失效（目标角超过8192）


改：电机收发，云台限位：1.yaw静态但mid 2.pitch动态3个 3.小yawpid偏移零点

发射电机id！！！以及发送

改巡逻patrol auto函数内范围

陀螺仪掉了，改用编码值
优化编码值精度？

两个头的零点都在死区内，误差极性不用改

没干死广工的秘密武器

armor ang要改
```c
enum
{
	ID_game_state                       = 0x0001,   // 比赛状态数据，1Hz
	ID_game_result                      = 0x0002,   // 比赛结果数据，比赛结束发送
	ID_game_robot_HP                    = 0x0003,   // 比赛机器人血量数据，1Hz发送
	ID_dart_status                      = 0x0004,   // 飞镖发射状态，飞镖发射时发送
	ID_ICRA_buff_debuff_zone_status     = 0x0005,   // 人工智能挑战赛加成与惩罚区状态，1Hz
	ID_event_data                       = 0x0101,   // 场地事件数据，1Hz
	ID_supply_projectile_action         = 0x0102,   // 场地补给站动作标识数据
	ID_referee_warning                  = 0x0104,   // 裁判警告数据，警告后发送
	ID_dart_remaining_time              = 0x0105,   // 飞镖发射口倒计时，1Hz
	ID_game_robot_state                 = 0x0201,   // 机器人状态数据，10Hz
	ID_power_heat_data                  = 0x0202,   // 实时功率热量数据，50Hz
	ID_game_robot_pos                   = 0x0203,   // 机器人位置数据，10Hz
	ID_buff_musk                        = 0x0204,   // 机器人增益数据，1Hz
	ID_aerial_robot_energy              = 0x0205,   // 空中机器人能量状态数据，10Hz，只有空中机器人主控发送
	ID_robot_hurt                       = 0x0206,   // 伤害状态数据，伤害发生后发送
	ID_shoot_data                       = 0x0207,   // 实时射击数据，子弹发射后发送
	ID_bullet_remaining                 = 0x0208,   // 弹丸剩余发送数，仅空中机器人，哨兵机器人以及ICRA机器人发送，1Hz
	ID_rfid_status                      = 0x0209,   // 机器人RFID状态，1Hz
	ID_interactive_header_data          = 0x0301,   // 机器人交互数据，发送方触发发送
	ID_aerial_data                      = 0x303,
	ID_ground_robot_position            = 0x020B,   // 所有己方机器人位置
	ID_map_sentry_data                  = 0x0307    // 发送哨兵路径
};

enum
{
	LEN_FRAME_HEAD                      = 5,        // 帧头长度
	LEN_CMD_ID                          = 2,        // 命令码长度
	LEN_FRAME_TAIL                      = 2,        // 帧尾CRC16
	LEN_game_state                      = 3,        // 0x0001，比赛状态数据，1Hz
	LEN_game_result                     = 1,        // 0x0002，比赛结果数据，比赛结束发送
	LEN_game_robot_HP                   = 32,       // 0x0003，比赛机器人血量数据，1Hz发送
	LEN_dart_status                     = 3,        // 0x0004，飞镖发射状态，飞镖发射时发送
	LEN_ICRA_buff_debuff_zone_status    = 3,        // 0x0005，人工智能挑战赛加成与惩罚区状态，1Hz
	LEN_event_data                      = 4,        // 0x0101，场地事件数据，1Hz
	LEN_supply_projectile_action        = 4,        // 0x0102，场地补给站动作标识数据
	LEN_referee_warning                 = 2,        // 0x0104，裁判警告数据，警告后发送
	LEN_dart_remaining_time             = 1,        // 0x0105，飞镖发射口倒计时，1Hz
	LEN_game_robot_state                = 15,       // 0x0201，机器人状态数据，10Hz
	LEN_power_heat_data                 = 16,       // 0x0202，实时功率热量数据，50Hz
	LEN_game_robot_pos                  = 16,       // 0x0203，机器人位置数据，10Hz
	LEN_buff_musk                       = 1,        // 0x0204，机器人增益数据，1Hz
	LEN_aerial_robot_energy             = 3,        // 0x0205，空中机器人能量状态数据，10Hz，只有空中机器人主控发送
	LEN_robot_hurt                      = 1,        // 0x0206，伤害状态数据，伤害发生后发送
	LEN_shoot_data                      = 7,        // 0x0207，实时射击数据，子弹发射后发送
	LEN_bullet_remaining                = 2,        // 0x0208，弹丸剩余发送数，仅空中机器人，哨兵机器人以及ICRA机器人发送，1Hz
	LEN_rfid_status                     = 4,        // 0x0209，机器人RFID状态，1Hz
	LEN_aerial_data                     = 15,
	LEN_ground_robot_position           = 40,       // 0x020B，所有己方机器人位置
	LEN_map_sentry_data                 = 103       // 0x0307，发送哨兵路径
};

```

读条未完成：
立即复活：花钱、次数            bit1

读条完成：
复活：花次数（距离补血点无关）  bit0