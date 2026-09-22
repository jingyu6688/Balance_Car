#include "stm32f10x.h"

void USART1_Init()
{
    RCC->APB2ENR |= (1<<14);   // USART
    RCC->APB2ENR |= (1<<2);    // GPIOA
    
    GPIOA->CRH &= ~(0xF << 4);
    GPIOA->CRH |= (0xB << 4);  // PA9
    
    GPIOA->CRH &= ~(0xF << 8);
    GPIOA->CRH |= (0x4 << 8);  // PA10
    
    USART1->BRR = 625;  // 39.0625
    
    USART1->CR1|=(1<<3);   // TX
    USART1->CR1|=(1<<2);   // RX
    
    USART1->CR1|=(1<<13);   // USART
}

void USART1_SendBit(char data)
{
    while(!(USART1->SR & (1<<7)));
    USART1->DR = data;
}

void USART1_SendString(char *str)
{
    while(*str)  // 字符串最后一位是\0
    {
        USART1_SendBit(*str);
        str++;
    }
}
