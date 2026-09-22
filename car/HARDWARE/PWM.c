#include "stm32f10x.h"

void PWM_Init()
{
    RCC->APB1ENR |= 1;       // TIM2
    RCC->APB2ENR |= (1 << 2);  // GPIOA
    
    GPIOA->CRL &= ~(0xF << 0);
    GPIOA->CRL |=  (0xB << 0);     // PA0 PWMA
    
    GPIOA->CRL &= ~(0xF << 4);
    GPIOA->CRL |=  (0x3 << 4);     // PA1 AIN1
    
    GPIOA->CRL &= ~(0xF << 8);
    GPIOA->CRL |=  (0x3 << 8);     // PA2 AIN2
    
    GPIOA->CRL &= ~(0xF << 12);
    GPIOA->CRL |=  (0x3 << 12);     // PA3 STBY
    
    GPIOA->BSRR = (1 << 3);    // PA3 = 1，退出待机 STBY = 1
    
    TIM2->PSC = 71;  // 71+1 72000000/72 = 1000000/s 一次一微秒
    TIM2->ARR = 49;  // 49+1
    TIM2->CCR1 = 25; // 20kHz 占空比50%
    
    // 没用 TIM2->CCMR1 |= 0;  // CC1S = 00 CH1设置输出
    TIM2->CCMR1 |= (6<<4);  // OC1M PWM模式1 CH1输出什么东西
    TIM2->CCER |= 1;        // CC1E 信号输出到对应的输出引脚。CH1
    TIM2->CR1 |= 1;         // CEN  使能计数器
}

void PWM_SetCompare(uint16_t compare)
{
    TIM2->CCR1 = compare;
}