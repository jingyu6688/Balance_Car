#include "stm32f10x.h"
#include "PWM.h"
#include "TIM1.h"

void Motor_SetDirection(int direction)
{
    if(direction > 0)
    {
        GPIOA->BSRR = (1 << 1);   // PA1 = 1
        GPIOA->BRR  = (1 << 2);   // PA2 = 0
    }
    else if(direction < 0)
    {
        GPIOA->BRR  = (1 << 1);   // PA1 = 0
        GPIOA->BSRR = (1 << 2);   // PA2 = 1
    }
    else
    {
        GPIOA->BRR = (1 << 1);    // PA1 = 0
        GPIOA->BRR = (1 << 2);    // PA2 = 0
    }
}

void Motor_SetSpeed(int speed)
{
    if(speed > 0)
    {
        Motor_SetDirection(1);
        PWM_SetCompare(speed);
    }
    else if(speed < 0)
    {
        Motor_SetDirection(-1);
        PWM_SetCompare(-speed);
    }
    else
    {
        Motor_SetDirection(0);
        PWM_SetCompare(0);
    }
}

void Motor_SetBothSpeed(int speed)
{
    Motor_SetSpeed(speed);
    Motor2_SetSpeed(speed);
}
