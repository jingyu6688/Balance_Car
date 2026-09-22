——————————第三部分 PWM TIM2——————————
	使能 TIM2
	使能 GPIOA
	配置 PA0
	配置 PSC
	配置 ARR
	配置 CCR1  占空比
	配置 PWM模式
	使能 CH1
	使能 CNT

	PWM引脚选择：TIM2定时器对应通道的引脚比如：TIM4 CH1->PA0通道 将CH1然后设置成PWM模式

	TIM2时钟 = 72 MHz
	PWM频率 = 20 kHz
	PWM频率 = 定时器时钟 / ((PSC+1) × (ARR+1))
	72000000 / ((PSC+1) × (ARR+1)) = 20000
	
	ARR决定一整个周期有多少个“格子”。
	CCR1决定在这些格子里的哪个位置发生高低电平切换。

	PWM频率主要由 PSC + ARR 决定。
	PWM占空比主要由 CCR1 相对于 ARR 的比例决定。
	
	TIM2时钟 72MHz定时器最原始的“节拍”
	PSC把72MHz降下来，让CNT慢一点

	CNT按这个速度不断数数
	ARR数到哪里算一个周期

	CCR1CNT数到哪里时改变CH1输出状态

	PWM频率一秒钟有多少个完整周期
	占空比一个周期里高电平占多少时间

	CC1E Capture/Compare 1 output Enable
	CEN：让TIM2这个“发动机”转起来。
	CC1E：让CH1这个“出口”打开。
	TIMx_CCMR1里CC1S配置输入还是输出
	
	CC1S  CH1的身份选择器  CH1输入输出
——————————第四部分 编码器 TIM4——————————
——————————编码器初始化 开始——————————
|	使能 TIM4   RCC->APB1ENR
|	使能 GPIOB   RCC->APB2ENR
|
|	配置 PB6 复用上拉/下拉输入   GPIOB->CRL
|	配置 PB7 复用上拉/下拉输入   GPIOB->CRL
|
|	配置 TIM4 分频 PSC = 0；
|	配置 TIM4 周期 ARR = 65535；   // TIM416位计数器范围0~65535
|
|	配置 TIM4 编码器 CH1 CCMR1->CCIS 01 输入 IC1映射在TI1上
|	配置 TIM4 编码器 CH2 CCMR1->CCIS 01 输入 IC2映射在TI2上
|
|	配置 TIM4 输入滤波 CCMR1->IC1F   // 暂时不需要
|	配置 TIM4 输入滤波 CCMR1->IC2F   // 暂时不需要
|
|	使能 TIM4 CH1 通道 CCER -> CC1E
|	使能 TIM4 CH2 通道 CCER -> CC2E
|
|	配置 TIM4 Model 3 模式 SMCR-> SMS 011
|
|	使能 TIM4 计数器 CR1 -> CEN
——————————编码器初始化 结束——————————
	PB6->CH1->A相
	PB7->CH2->B相
	配置 前先对应清零 TIM4->CCMR1 &= ~(0x3 << 0);  // CC1S清零
——————————编码器 函数声明——————————
	now_cnt = TIM4->CNT  // 读取当前计数
	
	if(encoder_first)   // 判断是否为第一次
		第一次
		encoder_first = 0;
        	last_cnt = now_cnt;
	 	直接放入last_cnt 结束 无法计算
		第二次及以上
		delta_raw = now_cnt - last_cnt;   // 算出10ms，计数器变化多少次
        	last_cnt = now_cnt;

	判断是否回绕
	取arr周期一半 判断，如果cnt小于负的一般加上arr，如果cnt大于一般，减去arr
	
	计算 用delta除以车轮一圈的计数，算出10ms车轮转了多少圈。
		然后除以0.01算出1s车轮转多少圈
		最后乘60算出一分钟转多少
	
	需要知道：编码器PPR，每转一圈产生多少个脉冲。是不是单通道
	
	减速比是多少？
		比如1：20
		最终算出车轮转一圈的计数多少

	第二次及以上拿到的delta需要判断是否回绕 
		65530 -> 65531 -> 65532 -> 65533 -> 65534 -> 65535 -> 0 -> 1 -> 2
		怎么判断：真实 |ΔCNT| < 计数器容量 / 2
		对于65536：|ΔCNT| < 32768

	理解：我们只看到了“0”和“900”，不知道中间到底走哪条路。
   		   实际有两条路可走
		   0，1，2，3，4，....，900	// cnt = 900
		   0，999，998，.......，900	// cnt = 100
		   所以才需要：500。
		   |900| > 500	×	大于arr一半舍去
		   |100| > 500	√	小于arr一半留下
		   这个限制来帮助我们判断。

	一个编码器通常有A相、B相
		作用：电机转了多少？往哪个方向转？转得多快 ？
		TIM4_CH1 -> PB6 -> 编码器A
		TIM4_CH2 -> PB7 -> 编码器B
	
	TIM4 16位计数器
		PSC = 0
		ARR 可以设置成：65535
	这样：CNT
		0 → 1 → 2 → ... → 65535 → 0
		让计数器尽可能快地响应外部编码器信号。

	TI1FP1 / TI2FP2 是输入滤波和极性处理后的内部信号
				   编码器逻辑再根据两相转换序列产生计数和方向
	TI1FP1：TI1->FP1
		FP1 可以理解成：经过输入处理之后的TI1信号。

	CH1输出模式：
	CH1作为输出时设置
	OC1M   输出比较器应该怎么控制输出？

	SMCR → SMS 它回答的是：TIM4这个定时器的“计数器CNT”由什么事件驱动、按照什么规则计数？
			SMS = TIM4计数器的工作/触发模式。
				    TIM4按照什么方式利用TI1/TI2驱动CNT


	001：编码器模式1 – 根据TI1FP1的电平，计数器在TI2FP2的边沿向上/下计数是什么意思？
		 TI1FP1 = A相 
		 TI2FP2 = B相
		 当TI2FP2出现上升或者下降沿，看TI1FP1的电平
		 TI1FP1= 0 CNT++ TI1FP1 = 1   CNT--

	011：编码器模式3 – 根据另一个信号的输入电平，计数器在TI1FP1和TI2FP2的边沿向上/下计数是什么意思
		 AB两项 怎么影响CNT++ CNT--
		 所以 Mode 3 的核心可以记成：A变了看B，B变了看A。
		 这就是为什么两相能够判断方向

	CC1P CH1输入信号的极性选择。决定：TIM4把哪个边沿当成有效边沿。
	CC1P -> 哪个边沿作为有效边沿

	OC1M
	↓
	只是在CH1作为输出时
	决定CH1输出模式

 	IC = Input Capture（输入捕获）。
	Input Capture 1 Filter，输入捕获1的数字滤波设置。


	TIM4->CCMR1 &= ~(0x3 << 0);先取反，在执行与运算//这就是“位掩码”理解成一张：“只允许修改某几个bit的遮罩。”

	Encoder.c里的变量，main.c能不能用？
	Encoder.c
 	   ↓
	定义变量

	Encoder.h
    	↓
	告诉其他文件“这个变量存在”

	main.c
    	↓
	通过 #include "Encoder.h"
	使用

	例如以后可以：
	// Encoder.c
	int16_t rpm;

	然后在 Encoder.h 里声明：
	extern int16_t rpm;

	main.c：
	#include "Encoder.h"
	这样 main.c 就能使用 rpm。

	定义放 Encoder.c，声明放 Encoder.h，使用放 main.c。

	编码器：100 PPR
	我们使用：AB两相 + 双边沿 = 四倍频
	CPR = 100 × 4 = 400 counts/revolution

	减速比解释 -> 1 : 20.409 -> 输入轴转20.409圈 → 输出轴转1圈。

	功率 ≈ 扭矩 × 转速
	所以如果功率差不多：
	转速 ↑ → 扭矩 ↓
	转速 ↓ → 扭矩 ↑
	功率 = 扭矩 × 角速度.


	abs(ΔCNT)  // 取绝对值，不要随便取会把方向丢掉

	用钟表理解就更简单了
	钟表只有：1 2 3 ... 12
	假设：现在是10点，过了一段时间是2点

	普通减法：2 - 10 = -8
	你难道认为时间倒退8小时了吗？没有。
	实际上：10 → 11 → 12 → 1 → 2
	前进了：4小时
	为什么？因为钟表是循环的。
	如果我们用“12小时制”：
	-8 + 12 = 4是不是就出来了？

	3 - 65530 = -65527
	-65527 + 65536= 9

	实际：+9 CNT
	普通减法：-65527
	回绕修正：-65527 + 65536
	最终：+9 CNT

	uint16介绍：总容量65536个 0~65535 
	  int16介绍：由于有正负，然后总量还是65536个 就表示-32768到+32767
	就对半分 但是由于需要符号位0正1负
	然后全0表示零划分到正数
	全1就表示-32768 
	一共负加零加正一共65536个
	负数       0       正数
	↓          ↓        ↓
	-32768 ... 0 ... +32767

	我们规定：一次采样的真实运动必须小于半个计数器。
	所以：真实 ΔCNT：-32768 < ΔCNT < +32768

——————————第五部分 I2C 通信——————————
I2C2
PB10   I2C2_SCL
PB11   I2C2_SDA

GPIOB->CRH &= ~(0xF<<8);   // PB10

7位地址       0x68
写地址：
0x68 << 1
= 0xD0
读地址：
0xD0 | 1
= 0xD1

D0
 ↓
“我要访问你，我要写”
 ↓
0x3B
 ↓
“我要访问你的3B寄存器”
 ↓
重复START
 ↓
D1
 ↓
“现在我要读”
 ↓
MPU6050
 ↓
数据

MPU6050的行为
收到 D1
 ↓
ACK
 ↓
准备发送数据

STM32的行为
ADDR = 1
 ↓
读 SR1
 ↓
读 SR2
 ↓
ADDR被清除
 ↓
继续处理接收流程

D1被MPU6050确认后，通信已经进入从机发送数据阶段；STM32仍然需要按照自己的硬件流程清除ADDR，之后才能正常继续接收处理。

MPU 在收到 D1 并 ACK 后，就已经进入从机发送阶段；STM32 同时处理自己的 ADDR 状态。两边可以重叠。

官方手册明确写了：单字节主接收时，关闭 ACK 和产生 STOP 都是在 EV6，也就是 ADDR 清除之后进行的。
EV = Event（事件）
EV6
就是：
I2C通信进行到某一个特定状态了。

EV6 对应什么？
在我们现在这个场景：
发送D1
 ↓
MPU ACK
 ↓
ADDR = 1

EV6_1 可以理解成：
EV6之后，针对“只接收1个字节”的特殊处理阶段。

EV6
= 地址阶段完成

EV6_1
= 地址阶段完成后，准备接收“最后一个字节”的特殊阶段

EV5
START发送完成
SB = 1

EV6
地址阶段完成
ADDR = 1

EV6_1
单字节接收
清ADDR之后
关闭ACK + 设置STOP

EV7
收到数据
RXNE = 1

ADDR=1
不是简单的：
“一个变量变成1而已。”

而是：
I2C硬件状态机停在这个阶段，等待CPU按照规定处理。

所以你现在终于理解了为什么：
temp = I2C2->SR1;
temp = I2C2->SR2;
不是“为了把一个变量改成0”。
而是：
CPU通过规定的操作告诉 I2C 硬件：“地址阶段我处理完了，你可以继续了。”

这比单纯背“SR1+SR2清ADDR”重要得多。

MPU6050 地址：0x68
写地址：0xD0
读地址：0xD1
要读取的寄存器：0x3B
WHO_AM_I 寄存器地址：0x75

单字节主接收的官方流程是：ADDR 事件时关闭 ACK → 清 ADDR → 设置 STOP → 等 RxNE → 读 DR。也就是说，ACK=0 是在 ADDR 清除之前写入，然后再清 ADDR。我们前面为了理解概念把顺序讲反了一点，现在以官方手册为准。

为什么 AF Open-Drain？
你说：
AF：把 PB10/PB11 的控制权交给 I2C2 外设。

Open-Drain：符合 I2C 总线的电气工作方式。

CR2
 ↓
告诉I2C：APB1给你的时钟是多少

CCR
 ↓
告诉I2C：SCL要跑多快

TRISE
 ↓
告诉I2C：SCL/SDA上升沿相关的最大允许时间

我们现在目标：
I2C标准模式
100 kHz
STM32F1 标准模式下：
CCR = Fpclk1 / (2 × Fscl)
代入：
Fpclk1 = 36MHz
Fscl = 100kHz
所以：

CCR = 36,000,000 / (2 × 100,000)
    = 180
SCL = APB1时钟 ÷ (2 × CCR)
所以：
I2C2->CCR = 180;

标准模式下：
TRISE = FREQ(MHz) + 1
所以：
36 + 1 = 37
因此：
I2C2->TRISE = 37;


第一次stm32发送D1，mpu收到然后回复ACK，然后mpu进入准备发送数据，这时候stm32卡在了ADDR，这时候时钟线会一直拉低，等待stm32处理完成，stm32读取sr1和2让addr置零，然后设置nack和stop这时候时钟线回复，接着读取数据。


D1
 ↓
MPU ACK
 ↓
MPU进入发送阶段
 ↓
STM32：ADDR=1
 ↓
SCL被硬件保持低
 ↓
STM32读取SR1 → SR2
 ↓
ADDR清除
 ↓
ACK=0
 ↓
STOP=1
 ↓
SCL继续
 ↓
MPU发送数据
 ↓
I2C硬件接收完整字节
 ↓
RXNE=1
 ↓
CPU读取DR

——————————第六部分 MPU6050——————————
初始化MPU6050 调用I2C里的ReadByte函数
往0x6B 里 写0x00 来唤醒MPU6050
PWR_MGMT_1:
0x6B：	mpu6050里的一个寄存器
0x00：	 0      	0	0      0      0      	0	0      0
		bit7 		bit6	bit5 	bit4 bit3 		bit2~0
	 	SLEEP						CLKSEL
	sleep = 1 睡眠
	sleep = 0 工作

0      	0	0      0      0      	0	0      1

最终0x01
CLKSEL = 001表示：选择 X 轴陀螺仪 PLL。
最终得到：
SLEEP = 0
→ MPU6050 唤醒

CLKSEL = 001
→ 使用 X 轴陀螺仪 PLL


SLEEP → “干不干活”
CLKSEL → “用什么节拍干活”

MPU6050内部
    ↓
自己的时钟
    ↓
8 MHz
它不需要外部东西提供时钟，自己就能工作。

I2C频率
↓
“我和你交流多快？”

MPU6050时钟
↓
“我自己内部工作多快？”

I2C = 通信的速度
CLKSEL = MPU6050内部工作的时钟来源

I2C2->DR = 0xD1;   // 0xD0 | 1 读
    while(!(I2C2->SR1 & (1<<1)));   // 等待ACK ADDR = 1
    
    // MPU6050 回复ACK完成 准备给stm发送数据 此时i2c时钟拉低 等待stm addr置零处理完成
    
    I2C2->CR1 &= ~(1<<10);   // ACK = 0关闭 ACK
temp = I2C2->SR1;
    temp = I2C2->SR2;   // 读取sr1和sr2 ADDR = 0

    // stm准备接收数据


CR2
↓
告诉 I2C 外设：
“你的输入时钟是多少？”

CCR
↓
根据这个输入时钟：
“我要把 SCL 设置成多快？”

CR2 提供“参考时钟信息”，CCR 用这个信息产生 SCL 通信速度。
——————————第七部分 mpu chushihua——————————
0x19
↓
SMPLRT_DIV
↓
采样率分频寄存器
这个“多久产生一次新的采样结果”，就是采样率。
它不是：
I2C通信速度 ❌
而是：
MPU6050传感器采样相关的频率设置 ✅

寄存器：
0x19 → SMPLRT_DIV
根据：
1000 ÷ (1 + SMPLRT_DIV) = 100
1 + SMPLRT_DIV = 10

SMPLRT_DIV = 9

基础采样率 = 1 kHz
不是永远都是 1 kHz。
它和另一个东西有关：
DLPF_CFG（数字低通滤波器）

所以真正完整的关系是：
DLPF
 ↓
决定 Gyroscope Output Rate
 ↓
SMPLRT_DIV
 ↓
决定最终 Sample Rate
这也是为什么 MPU6050 初始化不能只机械地写：
0x19 = 9
我们还要先配置 DLPF。


0x6B   PWR_MGMT_1寄存器
选择内部时钟源
        ↓
让 MPU6050 正常、稳定地运行

DLPF_CFG
        ↓
设置数字低通滤波
        ↓
影响陀螺仪输出速率

0x19   SMPLRT_DIV
对输出速率进行分频
        ↓
得到最终采样率
        ↓
100Hz
        ↓
10ms一次

PWR_MGMT_1
选择时钟
   ↓
CONFIG / DLPF_CFG
选择滤波带宽
   ↓
确定陀螺仪输出速率
   ↓
SMPLRT_DIV
进行分频
   ↓
最终采样率


总理解
① CLKSEL
② DLPF_CFG
③ SMPLRT_DIV
第一层：CLKSEL —— 内部时钟
CLKSEL
 ↓
MPU6050内部用哪个时钟源运行
它解决：
“我的内部电路拿谁当节拍？”

它不是直接决定：
“每多少毫秒给 STM32 一个数据。”
第二层：DLPF_CFG —— 滤波器
DLPF_CFG
 ↓
数字低通滤波器
 ↓
过滤高频变化
它解决：
“传感器数据中，哪些快速变化我要过滤掉？”

但关键来了：
DLPF 的设置同时会影响陀螺仪的输出速率。
所以它不只是“让数据平滑”。
第三层：SMPLRT_DIV —— 分频
它是在陀螺仪输出速率的基础上继续分频：
陀螺仪输出速率
       ↓
SMPLRT_DIV
       ↓
最终 Sample Rate
它解决：
“我最终想让 STM32 多久得到一次新的采样结果？”




& → 取地址
* → 根据地址取数据
test(data);
因为 data 是数组，所以传给函数时，会自动变成：
&data[0]

buffer[0]
等价于：
*(buffer + 0)

buffer[0] == *buffer

±500 °/s这个量程对应的灵敏度是：
65.5 LSB/(°/s)

陀螺仪获取的原始数值是整数，但是除以65.5会出现小数，所以需要浮点类型


STM32 PA0
   │
   │ PWM控制信号
   ↓
TB6612 PWMA
   │
   ↓
控制内部MOSFET什么时候导通
   │
   ↓
VM电源
   │
   ↓
A01 ───→ 电机 ───→ A02
          ↑
       电流流过

-----------------------电机编码器-------------------
PA1 → AIN1
PA2 → AIN2
PA0 → pwma

PA6 → TIM3_CH1 → PWMB
PA4 → BIN2
PA5 → BIN1

电机1：
PA0 → TIM2_CH1 → PWMA
PA1 → AIN2
PA2 → AIN1

电机2：
PB8 → TIM4_CH3 → PWMB
PA4、PA5 → BIN1、BIN2

PA3 → STBY



电机1：
PA0  → TIM2_CH1 → PWMA
PA1  → AIN2
PA2  → AIN1
PA3  → STBY

电机2：
PA8  → TIM1_CH1 → PWMB
PA4  → BIN1
PA5  → BIN2

编码器：
PB6  → TIM4_CH1 → Encoder A
PB7  → TIM4_CH2 → Encoder B

I2C：
PB10 → SCL
PB11 → SDA



tim3 ch3失败原因tim3让编码器使用cnt一个tim只有一个cnt 编码器等待轮子动变化cnt pwmcnt固定变化

电机1：TIM2 → PA0（PWMA），PA1/PA2（方向）
电机2：TIM1 → PB13（PWMB），PA4/PA5（方向）
——————————知识总结——————————
PPR  -> Pulses Per Revolution    每转一圈，产生多少个脉冲。
CPR  -> Counts Per Revolution    每转一圈，最终产生多少个计数。
RPM -> Revolutions Per Minute   每分钟转多少圈
1000 RPM -> 一分钟转1000圈。

扭矩 = 1 N·m   用1牛顿的力，在距离旋转中心1米的位置产生1牛·米的扭矩。

1000RPM -> 转得多快
1 N·m -> 拧得有多大劲

	