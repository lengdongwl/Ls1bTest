/*
 * bh1750_iic.c
 *
 * created: 2022/3/2
 *  author: 
 */
#include "ls1b.h"
#include "ls1b_gpio.h"

#include "bsp.h"
#include "bh1750_iic.h"
/*********************************************************
功  能：初始化IIC
参  数: 无
返回值: 无
**********************************************************/
void IIC_Init(void)
{
    gpio_enable( 32, DIR_OUT );
	gpio_enable( 33, DIR_OUT );
    gpio_write(32,1);
    gpio_write(33,1);
}

/**************************************************************
*功  能：配置输出端口
*参  数: 无
*返回值: 无
**************************************************************/
void SDA_OUT(void)
{
	gpio_enable( 33, DIR_OUT );
}

/**************************************************************
*功  能：配置输入端口
*参  数: 无
*返回值: 无
**************************************************************/
void SDA_IN(void)
{
    gpio_enable( 33, DIR_IN );

}

/*********************************************************
功  能：产生IIC起始信号
参  数: 无
返回值: 无
**********************************************************/
void IIC_Start(void)
{
    SDA_OUT();     //sda线输出
    gpio_write(33,1);
    gpio_write(32,1);
    delay_us(2);
    gpio_write(33,0);//START:when CLK is high,DATA change form high to low
    delay_us(2);
    gpio_write(32,0);//钳住I2C总线，准备发送或接收数据
}

/*********************************************************
功  能：产生IIC停止信号
参  数: 无
返回值: 无
**********************************************************/
void IIC_Stop(void)
{
    SDA_OUT();//sda线输出
    gpio_write(32,0);
    gpio_write(33,0);//STOP:when CLK is high DATA change form low to high
    delay_us(2);
    gpio_write(32,1);
    gpio_write(33,1);//发送I2C总线结束信号
    delay_us(2);
}

/*********************************************************
功  能：等待应答信号到来
参  数: 无
返回值: 1，接收应答失败
        0，接收应答成功
**********************************************************/
unsigned char IIC_Wait_Ack(void)
{
    unsigned char ucErrTime=0;
    SDA_IN();      //SDA设置为输入
    gpio_write(33,1);
    delay_us(1);
    gpio_write(32,1);
    delay_us(1);
    while(gpio_read( 33 ))
    {
        ucErrTime++;
        if(ucErrTime>250)
        {
            IIC_Stop();
            return 1;
        }
    }
    gpio_write(32,0);//时钟输出0
    return 0;
}

/*********************************************************
功  能：产生ACK应答
参  数: 无
返回值: 无
**********************************************************/
void IIC_Ack(void)
{
    gpio_write(32,0);
    SDA_OUT();
    gpio_write(33,0);
    delay_us(2);
    gpio_write(32,1);
    delay_us(2);
    gpio_write(32,0);
}

/*********************************************************
功  能：不产生ACK应答
参  数: 无
返回值: 无
**********************************************************/
void IIC_NAck(void)
{
    gpio_write(32,0);
    SDA_OUT();
    gpio_write(33,1);
    delay_us(2);
    gpio_write(32,1);
    delay_us(2);
    gpio_write(32,0);
}

/*********************************************************
功  能：IIC发送一个字节
参  数: 无
返回值: 从机有无应答
		1，有应答
		0，无应答
**********************************************************/
void IIC_Send_Byte(unsigned char txd)
{
    unsigned char t;
    SDA_OUT();
    gpio_write(32,0);//拉低时钟开始数据传输
    for(t=0; t<8; t++)
    {
        gpio_write(33,(txd&0x80)>>7);
        txd<<=1;
        delay_us(1);   //对TEA5767这三个延时都是必须的
        gpio_write(32,1);
        delay_us(1);
        gpio_write(32,0);
        delay_us(1);
    }
}

/*********************************************************
功  能：读1个字节
参  数: ack=1时，ack=0时
返回值: 发送ACK，发送nACK
**********************************************************/
unsigned char IIC_Read_Byte(unsigned char ack)
{
    unsigned char i,receive=0;
    SDA_IN();//SDA设置为输入
    for(i=0; i<8; i++ )
    {
        gpio_write(32,0);
        delay_us(1);
        gpio_write(32,1);
        receive<<=1;
        if(gpio_read( 33 ))
            receive++;
        delay_us(1);
    }
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK
    return receive;
}
