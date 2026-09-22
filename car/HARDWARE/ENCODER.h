#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f10x.h"

extern int16_t rpm;
extern int16_t delta_cnt;

void Encoder_Init(void);
void Encoder_Update(void);

#endif