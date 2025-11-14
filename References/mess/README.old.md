**Log**

1.复位才有用：verify and reset

2.xtal：168，MicroLIB

//3.printf需要在for中usart init；直接调用U1 Transmit不用

4.中英乱码：U8、ANSI来回切



**ToDo

电机

![image-20231006120445226](C:\Users\lenovo\AppData\Roaming\Typora\typora-user-images\image-20231006120445226.png)



![image-20231006120422513](C:\Users\lenovo\AppData\Roaming\Typora\typora-user-images\image-20231006120422513.png)

![image-20231006195947554](C:\Users\lenovo\AppData\Roaming\Typora\typora-user-images\image-20231006195947554.png)

pid = 1.75，0.8，0.5



*数据来自单片机串口返回



return current 2000









**TASK9.26**

------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

**1.以函数void USART2_IRQHandler(void)为开始，画出流程图讲解串口的DMA双缓冲机制，用自己的语言说明，勿抄袭**

![16062d23a4907e919504fae0cd8a998](E:\WeChat\Chat_record\WeChat Files\wxid_tp1yuspiigso22\FileStorage\Temp\16062d23a4907e919504fae0cd8a998.png)

![32cacb40ee73ae55a62c0623af2e3cc](E:\WeChat\Chat_record\WeChat Files\wxid_tp1yuspiigso22\FileStorage\Temp\32cacb40ee73ae55a62c0623af2e3cc.jpg)

**2.HAL_Delay 与 osDelay 的区别**

​	开始运行前，任务1、2处于**就绪态**（任务1优先级更高）

​	开始运行，RTOS选择**任务1优先进行**。

​	在HAL_Delay下，HAL_Delay使得[整个处理器阻塞*](https://zhuanlan.zhihu.com/p/647737904)，也就同时将任务2阻塞，当延时结束，任务1重回就绪态，RTOS继续选择优先级高的任务1，开始运行。



*//蓝0.9s 红0.5s*

(BLUE)

HAL_Delay

 

​	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET/*high level*/);

​	osDelay(900);    

​	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET/*on*/);

​	HAL_Delay(900);

 在这个例子中，两个任务一开始**并行**（osDelay），而后每次循环中高优先级内的HAL_Delay**将低优先级的任务一并阻塞**，直到下一个循环，低优先级任务才得以运行。

(BLUE)

osDelay

 

​	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET/*high level*/);

​	osDelay(900);    

​	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET/*on*/);

​	osDelay(900);

对照：在使用osDelay函数进行延时的例子中，低优先级的任务**不会被阻塞**（只有当前任务进入阻塞态），于是两个灯都能以各自频率进行闪烁，而不是红等蓝。

其他区别：HAL_Delay 函数参数是以**毫秒**为单位的时间，而osDelay函数的参数是以**tick**为单位的**节拍**，并与**周期**一起决定延时时间。





----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

The very first test, LED Flip.  :)

The 2nd commit is realizing Breathing LED through TIM1_CH1, GPIOE_Pin9.

Timers of STM32 F407 VET6.

![img](https://img-blog.csdnimg.cn/5401a497c5a44f859fd80a23d98699e6.png)



**FreeRTOS 待办**

- [x] 1.优先级与HAL_Delay的冲突 --> osDelay

- [x] 2.FreeRTOS.c与main.c里的（函数&变量）定义或声明 的作用域 的问题

	​	与main文件的关系（default task、main while1、中断函数）优先级和时序

- [ ] 3.
