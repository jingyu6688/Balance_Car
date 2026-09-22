#include "stm32f10x.h"

uint16_t last_cnt;
uint16_t now_cnt;

int32_t delta_raw;
int16_t delta_cnt;  // delta有正负数

float  wheel_rpm;

uint8_t encoder_first = 1;

// 安装设备
void Encoder_Init(void)
{
    RCC->APB1ENR |= (1<<2);  // TIM4
    RCC->APB2ENR |= (1<<3);  // GPIOB
    
    GPIOB->CRL &= ~(0xF << 24);
    GPIOB->CRL |=  (0x8 << 24);  // PB6→CH1→A
    
    GPIOB->CRL &= ~(0xF << 28);
    GPIOB->CRL |=  (0x8 << 28);  // PB7→CH2→B
    
    TIM4->PSC = 0;          // 分频
    TIM4->ARR = 65535;      // 周期 CNT的自动重装值。
    
    // CC1S占bit0~1
    TIM4->CCMR1 &= ~(0x3 << 0); // CC1S清零
    TIM4->CCMR1 |=  (1 << 0);   // CC1S 01 输入 CH1 映射 TI1
    TIM4->CCMR1 |= 0;           // IC1F CH1滤波 无滤波
    
    TIM4->CCMR1 |= (1 << 8);  // CC2S 01 输入 CH2 映射 TI2
    TIM4->CCMR1 |= 0;       // IC2F CH2滤波 无滤波
    
    TIM4->CCER |= (1 << 0);   // CC1E CH1通道使能
    TIM4->CCER |= (1 << 4);   // CC2E CH2通道使能
    
    TIM4->SMCR |= 3;        // SMS 编码器模式设置
    
    TIM4->CR1 |= (1 << 0);    // Counter Enable 计算器使能
}

void Encoder_Update()
{
    now_cnt = TIM4->CNT;
    
    if(encoder_first)
    {
        encoder_first = 0;
        last_cnt = now_cnt;
    }
    else
    {
        delta_raw = now_cnt - last_cnt;
        last_cnt = now_cnt;
        
        //判断回绕
        if(delta_raw < -32768)
        {
            delta_raw += 65536;
        }
         if(delta_raw > 32767)
        {
            delta_raw -= 65536;
        }
        
        delta_cnt = delta_raw;
        //wheel_rpm  = (float)delta_cnt/ 1061.268f;  // 车轮一圈产生1061.268个计数，求出10ms转多少圈
        //wheel_rpm  /= 0.01f;  // 除以0.01算出一秒多少圈
        //wheel_rpm  *= 60.0f;  // 算出一分钟多少圈
        wheel_rpm = (float)delta_cnt / 1061.268f / 0.01f * 60.0f;
    }
}
