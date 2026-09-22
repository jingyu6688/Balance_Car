#include "stm32f10x.h"

void TIM1_Init(void)
{
    // 开启 TIM1、GPIOA、GPIOB 时钟
    RCC->APB2ENR |= (1 << 11);  // TIM1
    RCC->APB2ENR |= (1 << 2);   // GPIOA
    RCC->APB2ENR |= (1 << 3);   // GPIOB

    // PA4、PA5 → 普通推挽输出 50MHz
    GPIOA->CRL &= ~(0xFF << 16);
    GPIOA->CRL |=  (0x33 << 16);

    // PB13 → TIM1_CH1N，复用推挽输出 50MHz
    GPIOB->CRH &= ~(0xF << 20);
    GPIOB->CRH |=  (0xB << 20);

    // 20kHz
    TIM1->PSC = 71;
    TIM1->ARR = 49;

    // 50%占空比
    TIM1->CCR1 = 25;

    // CH1 PWM Mode 1
    TIM1->CCMR1 |= (6 << 4);

    // 开启 CH1N
    TIM1->CCER |= (1 << 2);

    // 开启 TIM1 主输出
    TIM1->BDTR |= (1 << 15);

    // 启动 TIM1
    TIM1->CR1 |= 1;
}

void TIM1_SetCompare(uint16_t compare)
{
    TIM1->CCR1 = compare;
}

void Motor2_SetDirection(int direction)
{
    if(direction > 0)
    {
        GPIOA->BSRR = (1 << 4);   // PA4 = 1
        GPIOA->BRR  = (1 << 5);   // PA5 = 0
    }
    else if(direction < 0)
    {
        GPIOA->BRR  = (1 << 4);   // PA4 = 0
        GPIOA->BSRR = (1 << 5);   // PA5 = 1
    }
    else
    {
        GPIOA->BRR = (1 << 4);    // PA4 = 0
        GPIOA->BRR = (1 << 5);    // PA5 = 0
    }
}

void Motor2_SetSpeed(int speed)
{
    if(speed > 0)
    {
        Motor2_SetDirection(1);
        TIM1_SetCompare(speed);
    }
    else if(speed < 0)
    {
        Motor2_SetDirection(-1);
        TIM1_SetCompare(-speed);
    }
    else
    {
        Motor2_SetDirection(0);
        TIM1_SetCompare(0);
    }
}
