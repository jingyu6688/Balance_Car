#include "stm32f10x.h"
#include "USART.h"      // 2026/8/13
#include "TIM3.h"       // 2026/8/14
#include "ENCODER.h"    // 2026/8/19
#include "I2C2.h"       // 2026/8/23
#include "MPU6050.h"    // 2026/8/23
#include "MOTOR.h"      // 2026/9/5
#include "PWM.h"        // 2026/9/5
#include "TIM1.h"       // 2026/9/10

//uint8_t mpu_id;

float gyro_x;
float gyro_y;
float gyro_z;

float angle_x = 0;
float angle_acc;

float Kp = 10.0f;
float Kd = 0.5f;
float motor_output = 0;

int main(void)
 {
    USART1_Init();
    Tim3_Init();
    Encoder_Init();
     
    I2C2_Init();
    PWM_Init();
    
    MPU6050_Init();
    MPU6050_CalibrateGyro();
    MPU6050_CalibrateAccelAngle();
     
    TIM1_Init();
    
    while(1)
    {
        //10ms进入中断
        if(TIM3_Flag)
        {
            TIM3_Flag = 0;
            
            MPU6050_ReadGyro(&gyro_x, &gyro_y, &gyro_z);
            
            angle_acc = MPU6050_GetAccelAngle();
            
            angle_x = 0.98f * (angle_x + gyro_x * 0.01f)
                    + 0.02f * angle_acc;  // 小车现在到底倾斜了多少度
            
            motor_output = Kp * angle_x - Kd * gyro_x;
            
            Motor_SetBothSpeed((int)motor_output);
        }
    }
}
