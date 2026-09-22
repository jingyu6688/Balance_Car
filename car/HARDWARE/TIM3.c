#include "stm32f10x.h"

volatile uint8_t TIM3_Flag = 0;

void Tim3_Init()
{
    RCC->APB1ENR |= (1<<1);   // TIM3使能
    
    TIM3->PSC = 7200 - 1;  // 0.1ms
    TIM3->ARR = 100 - 1;   // 10ms
    
    TIM3->EGR |= 1;   // 立刻更新PSC和ARR
    
    TIM3->DIER |= 1;   // 允许更新中断
    NVIC_EnableIRQ(TIM3_IRQn);   // 中断函数
    
    TIM3->CR1 |= 1;   // 使能计数器
}

void TIM3_IRQHandler(void)
{
    if(TIM3->SR & (1<<0))
    {
        TIM3->SR &= ~(1<<0);  // 置0
        TIM3_Flag = 1;  // 标志置1
    }
}
