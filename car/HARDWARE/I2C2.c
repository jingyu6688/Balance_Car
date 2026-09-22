#include "stm32f10x.h"
#include "I2C2.h"

uint16_t temp;

void I2C2_Init(void)
{
    RCC->APB1ENR |= (1 << 22);      // I2C2使能
    RCC->APB2ENR |= (1 << 3);       // GPIOB使能
    
    GPIOB->CRH &= ~(0xF << 8);
    GPIOB->CRH |=  (0xF << 8);      //PB10 → SCL
    
    GPIOB->CRH &= ~(0xF << 12);
    GPIOB->CRH |=  (0xF << 12);     //PB11 → SDA
    
    I2C2->CR2 = 36;                 //APB1 = 36MHz
    
    I2C2->CCR = 180;                //I2C标准模式 100kHz
    
    I2C2->TRISE = 37;               //最大上升时间
    
    I2C2->CR1 |= (1 << 0);          //开启 I2C2 外设
}

uint8_t I2C2_ReadByte(uint8_t reg)
{
    uint8_t data;
    
    I2C2->CR1 |= (1<<8);            // START
    while(!(I2C2->SR1 & (1<<0)));   // SB
    
    I2C2->DR = 0xD0;                // 0x68 << 1 = 0xD0 写
    while(!(I2C2->SR1 & (1<<1)));   //等待ack ADDR = 1
    
    temp = I2C2->SR1;
    temp = I2C2->SR2;   // ADDR = 0
    
    I2C2->DR = reg;   // 发送寄存器
    while(!(I2C2->SR1 & (1<<2)));   // 等待发送完成 BTF

    I2C2->CR1 |= (1<<8);    //RESTART
    while(!(I2C2->SR1 & (1<<0)));  // SB
    
    I2C2->DR = 0xD1;    // 0xD0 | 1 读
    while(!(I2C2->SR1 & (1<<1)));   // 等待ACK ADDR = 1
    
    I2C2->CR1 &= ~(1<<10);  // ACK = 0 关闭ACK
        
    temp = I2C2->SR1;
    temp = I2C2->SR2;       // ADDR = 0
    
    I2C2->CR1 |= (1<<9);    // STOP
    
    while(!(I2C2->SR1 & (1 << 6)));  // RXNE 接收寄存器非空
    data = I2C2->DR;
    
    return data;
}

void I2C2_WriteByte(uint8_t reg, uint8_t data)
{
    uint8_t retry_count = 0;
    
retry:
    I2C2->CR1 |= (1 << 8);          // START
    
    // SB
    uint32_t timeout = 100000;
    while(!(I2C2->SR1 & (1 << 0)))
    {
        if(--timeout == 0)
        {
            retry_count++;
            
            if(retry_count >= 3)
            {
                return;
            }
        
            if(!I2C2_Recover())
            {
                return;
            }
            
            goto retry;
        }
    }
    
    I2C2->DR = 0xD0;                // D0 写
    
    // ADDR
    timeout = 100000;
    while(!(I2C2->SR1 & (1 << 1)))
    {
        if(--timeout == 0)
        {
            return;
        }
    }
    
    temp = I2C2->SR1;
    temp = I2C2->SR2;               // ADDR = 0
    
    I2C2->DR = reg;                 // 发送reg
    
    // 等待TXE
    timeout = 100000;
    while(!(I2C2->SR1 & (1 << 7)))
    {
        if(--timeout == 0)
        {
            return;
        }
    }
    
    I2C2->DR = data;                // 发送data
    
    // 等待BTF
    timeout = 100000;
    while(!(I2C2->SR1 & (1 << 2)))
    {
        if(--timeout == 0)
        {
            return;
        }
    }
    
    I2C2->CR1 |= (1 << 9);          // STOP
}

void I2C2_ReadBytes(uint8_t reg,uint8_t *buffer, uint8_t length)
{
    uint8_t i = 0;
    
    // START
    I2C2->CR1 |= (1 << 8);
    while(!(I2C2->SR1 & (1 << 0)));

    // MPU6050 写地址
    I2C2->DR = 0xD0;
    while(!(I2C2->SR1 & (1 << 1)));
   
    // 清 ADDR
    temp = I2C2->SR1;
    temp = I2C2->SR2;
    
    // 寄存器地址
    I2C2->DR = reg;
    while(!(I2C2->SR1 & (1 << 2)));
    
    // RESTART
    I2C2->CR1 |= (1 << 8);
    while(!(I2C2->SR1 & (1 << 0)));
    
    // MPU6050 读地址
    I2C2->DR = 0xD1;
    while(!(I2C2->SR1 & (1 << 1)));
    
    // 清 ADDR
    temp = I2C2->SR1;
    temp = I2C2->SR2;
    
    I2C2->CR1 |= (1 << 10);  // ACK = 1
    
    // 连续接收
    while(length)
    {
        if(length == 1)
        {
            I2C2->CR1 &= ~(1 << 10);  // NACK
            I2C2->CR1 |=  (1 << 9);   // STOP
        }
        
        while(!(I2C2->SR1 & (1 << 6)));  // RXNE
        
        buffer[i] = I2C2->DR;
        
        length--;
        i++;
    }
}

uint8_t I2C2_Recover()
{
    //关闭 I2C2
    I2C2->CR1 &= ~(1 << 0);
    
    // ② PB10、PB11 改成 GPIO 开漏输出
    GPIOB->CRH &= ~(0xFF << 8);
    GPIOB->CRH |=  (0x77 << 8);
    
    // ③ 最多发送9个SCL脉冲
    for(int i = 0; i < 9; i++)
    {
        GPIOB->BRR = (1 << 10);      // SCL = 0

        for(volatile int j = 0; j < 100; j++);

        GPIOB->BSRR = (1 << 10);     // 释放SCL，SCL → 1

        for(volatile int j = 0; j < 100; j++);
        
        // SDA已经释放
        if(GPIOB->IDR & (1 << 11))   // SDA = 1？
        {
            break;   // SDA=1，退出循环
        } 
    }
    
    // ④ 判断 SDA 是否已经释放
    if(GPIOB->IDR & (1 << 11))
    {
        // ⑤ SDA已经是1，制造STOP
        GPIOB->BSRR = (1 << 10);     // SCL = 1
        GPIOB->BRR  = (1 << 11);     // SDA = 0

        for(volatile int j = 0; j < 100; j++);

        GPIOB->BSRR = (1 << 11);     // SDA释放，0 → 1

        //恢复I2C2
        I2C2_Init();

        return 1;
    }

    // ⑦ 9次之后SDA仍然是0
    return 0;
}

