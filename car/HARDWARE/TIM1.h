#ifndef __TIM1_H
#define __TIM1_H

#include "stm32f10x.h"

void TIM1_Init(void);

void TIM1_SetCompare(uint16_t compare);

void Motor2_SetDirection(int direction);

void Motor2_SetSpeed(int speed);

#endif
