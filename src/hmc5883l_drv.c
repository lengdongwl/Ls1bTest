/*
 * hmc5883l_drv.c
 *
 * created: 2022/7/14
 *  author:
 */

#include "hmc5883l_drv.h"
#include <math.h>
#include "bsp.h"
#include "stdint.h"

/************************HCM5883L IIC驱动 代码***************************/

void i2c_init(void)
{

    gpio_enable(28,DIR_OUT);
    gpio_enable(29,DIR_OUT);
    HMC_SCL_H;
    HMC_SDA_H;
}

void SDA_INPUT(void)
{
    gpio_enable(29,DIR_IN);
}


void SDA_OUTPUT(void)
{

    gpio_enable(29,DIR_OUT);

}


void i2c_Delay(void)
{
    uint8_t i;
    for (i = 0; i < 50; i++);
}


void i2c_Start(void)
{

    SDA_OUTPUT();
    HMC_SDA_H;
    delay_us(20);
    HMC_SCL_H;
    delay_us(50);
    HMC_SDA_L;
    delay_us(50);
    HMC_SCL_L;
    delay_us(10);
}


void i2c_Stop(void)
{

    SDA_OUTPUT();
    HMC_SDA_L;	//STOP:when CLK is high DATA change form low to high
    delay_us(20);
    HMC_SCL_H;
    delay_us(50);
    HMC_SDA_H;
    delay_us(30);
}


void i2c_Ack(void)
{
    SDA_OUTPUT();
    HMC_SDA_L;
    delay_us(20);

    HMC_SCL_H;
    delay_us(50);
    HMC_SCL_L;
    delay_us(10);
}


void i2c_NoAck(void)
{
    SDA_OUTPUT();
    HMC_SDA_H;
    delay_us(20);

    HMC_SCL_H;
    delay_us(50);
    HMC_SCL_L;
    delay_us(10);
}


uint8_t i2c_WaitAck(void)
{
    uint8_t ucErrTime = 0x00;


    SDA_INPUT();
    HMC_SDA_H;
    delay_us(30);

    HMC_SCL_H;
    delay_us(30);

    while (HMC_SDA)
    {
        ucErrTime++;
        if (ucErrTime > 250)
        {
            HMC_SCL_L;
            delay_us(30);
            i2c_Stop();
            return (0);
        }
    }
    HMC_SCL_L;
    delay_us(30);
    return (1);
}


void i2c_SendByte(uint8_t _ucByte)
{
    uint8_t i;


    SDA_OUTPUT();

    for (i = 0; i < 8; i++)
    {
        if (_ucByte & 0x80)
            HMC_SDA_H;
        else
            HMC_SDA_L;

        _ucByte <<= 1;

        delay_us(10);
        HMC_SCL_H;
        delay_us(30);
        HMC_SCL_L;
        delay_us(10);
    }
}

uint8_t i2c_ReadByte(void)
{
    uint8_t i;
    uint8_t value = 0x00;


    for (i = 0; i < 8; i++)
    {
        SDA_OUTPUT();
        HMC_SDA_H;
        delay_us(20);

        HMC_SCL_H;
        delay_us(50);

        SDA_INPUT();
        value <<= 1;

        if (HMC_SDA)
            value++;

        HMC_SCL_L;
        delay_us(10);
    }
    return (value);
}
void HMC5883L_Init(void)
{

    i2c_Start();
    i2c_SendByte(0x3c);
    i2c_WaitAck();
    i2c_SendByte(0x00);
    i2c_WaitAck();
    i2c_SendByte(0x58);  //输出速率75hz
    i2c_Stop();

    i2c_Start();
    i2c_SendByte(0x3c); //写指令
    i2c_WaitAck();
    i2c_SendByte(0x01);
    i2c_WaitAck();
    i2c_SendByte(0x60); //测量范围
    i2c_WaitAck();
    i2c_Stop();

    i2c_Start();
    i2c_SendByte(0x3c); //写指令
    i2c_WaitAck();
    i2c_SendByte(0x02);
    i2c_WaitAck();
    i2c_SendByte(0x00); //连续测量模式
    i2c_WaitAck();
    i2c_Stop();

}

void HMC5883L_READ(int16_t *x,int16_t *y)
{
    uint8_t XYZ_Data[6]= {0};
    uint8_t i=0;
    i2c_Start();
    i2c_SendByte(0x3c);
    i2c_WaitAck();
    i2c_SendByte(0x03);  //X轴数据地址
    i2c_WaitAck();
    i2c_Stop();

    i2c_Start();
    i2c_SendByte(0x3d);
    i2c_WaitAck();

    for(i=0; i<5; i++)
    {
        XYZ_Data[i]=i2c_ReadByte();
        i2c_Ack();
    }

    XYZ_Data[5] =i2c_ReadByte();
    i2c_NoAck();
    i2c_Stop();

    *x = (int16_t)(XYZ_Data[0]<<8)|XYZ_Data[1];
    *y = (int16_t)(XYZ_Data[4]<<8)|XYZ_Data[5];

}

void HCM5883L_Init(void)
{
    i2c_init();
    HMC5883L_Init();   //磁力计
}
double HCM5883L_Get_Angle(void)
{
    int16_t X_HM,Y_HM;
    double Angle=0;
    delay_ms(67);
    HMC5883L_READ(&X_HM,&Y_HM);
    Angle = (atan2(Y_HM,X_HM) * (180 / 3.14159265) + 180);
    //printf("%.2f\r\n",Angle);
    return Angle;
}

