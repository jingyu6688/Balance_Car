#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f10x.h"

void MPU6050_Init(void);

void MPU6050_ReadGyro(float  *gx, float  *gy, float  *gz);

void MPU6050_CalibrateGyro(void);

void MPU6050_ReadAccel(float *ax, float *ay, float *az);

void MPU6050_CalibrateAccelAngle(void);

float MPU6050_GetAccelAngle(void);

#endif