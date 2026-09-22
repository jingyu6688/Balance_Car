#include "I2C2.h"
#include <math.h>

#define PI 3.1415926f
float gyro_x_offset = 0;
float gyro_y_offset = 0;
float gyro_z_offset = 0;

float accel_angle_offset = 0;

void MPU6050_Init(void)
{
    I2C2_WriteByte(0x6B, 0x01);  // 唤醒 + 选择时钟 SLEEP = 0 CLKSEL = 001
    
    I2C2_WriteByte(0x1A, 0x03);  // 设置数字低通滤波 011 42 Hz
        
    I2C2_WriteByte(0x19, 0x09);  // 设置采样率 100Hz
    
    I2C2_WriteByte(0x1B, 0x08);  // 0x1B 陀螺仪量程 65.5 LSB/(°/s)
    
}

void MPU6050_ReadGyro(float *gx, float *gy, float *gz)
{
    uint8_t gyro_data[6];
    
    int16_t raw_x;
    int16_t raw_y;
    int16_t raw_z;
    
    I2C2_ReadBytes(0x43,gyro_data,6);
    
    raw_x = (gyro_data[0] << 8) | gyro_data[1];
    raw_y = (gyro_data[2] << 8) | gyro_data[3];
    raw_z = (gyro_data[4] << 8 )| gyro_data[5];
    
    *gx = raw_x / 65.5f - gyro_x_offset;
    *gy = raw_y / 65.5f - gyro_y_offset;
    *gz = raw_z / 65.5f - gyro_z_offset;
    
}

void MPU6050_CalibrateGyro(void)
{
    float gyro_x;
    float gyro_y;
    float gyro_z;
    
    float sum_x = 0;
    float sum_y = 0;
    float sum_z = 0;
    
    for(int i = 0; i < 100; i++)
    {
        MPU6050_ReadGyro(&gyro_x, &gyro_y, &gyro_z);
        
        sum_x += gyro_x;
        sum_y += gyro_y;
        sum_z += gyro_z;
    }
    
    gyro_x_offset = sum_x / 100;
    gyro_y_offset = sum_y / 100;
    gyro_z_offset = sum_z / 100;
}

void MPU6050_ReadAccel(float *ax, float *ay, float *az)
{
    uint8_t accel_data[6];
    
    int16_t raw_x;
    int16_t raw_y;
    int16_t raw_z;
    
    I2C2_ReadBytes(0x3B, accel_data, 6);
    
    raw_x = (accel_data[0] << 8) | accel_data[1];
    raw_y = (accel_data[2] << 8) | accel_data[3];
    raw_z = (accel_data[4] << 8) | accel_data[5];
    
    *ax = raw_x / 16384.0f;
    *ay = raw_y / 16384.0f;
    *az = raw_z / 16384.0f;
}

void MPU6050_CalibrateAccelAngle(void)
{
    float accel_x;
    float accel_y;
    float accel_z;
    
    float angle_sum = 0;
    
    for(int i = 0; i < 100; i++)
    {
        MPU6050_ReadAccel(&accel_x, &accel_y, &accel_z);
        
        angle_sum += atan2f(accel_x, accel_z) * 180.0f / PI;
    }
    
    accel_angle_offset = angle_sum / 100;
}

float MPU6050_GetAccelAngle(void)
{
    float accel_x;
    float accel_y;
    float accel_z;

    MPU6050_ReadAccel(&accel_x, &accel_y, &accel_z);

    return atan2f(accel_x, accel_z) * 180.0f / PI
           - accel_angle_offset;
}
