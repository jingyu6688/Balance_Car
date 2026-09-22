#ifndef __I2C2_H
#define __I2C2_H

#include "stm32f10x.h"

void I2C2_Init(void);

uint8_t I2C2_ReadByte(uint8_t reg);

void I2C2_WriteByte(uint8_t reg, uint8_t data);

void I2C2_ReadBytes(uint8_t reg,uint8_t *buffer, uint8_t length);

uint8_t I2C2_Recover(void);

#endif