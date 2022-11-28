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
��  �ܣ���ʼ��IIC
��  ��: ��
����ֵ: ��
**********************************************************/
void IIC_Init(void)
{
    gpio_enable( 32, DIR_OUT );
	gpio_enable( 33, DIR_OUT );
    gpio_write(32,1);
    gpio_write(33,1);
}

/**************************************************************
*��  �ܣ���������˿�
*��  ��: ��
*����ֵ: ��
**************************************************************/
void SDA_OUT(void)
{
	gpio_enable( 33, DIR_OUT );
}

/**************************************************************
*��  �ܣ���������˿�
*��  ��: ��
*����ֵ: ��
**************************************************************/
void SDA_IN(void)
{
    gpio_enable( 33, DIR_IN );

}

/*********************************************************
��  �ܣ�����IIC��ʼ�ź�
��  ��: ��
����ֵ: ��
**********************************************************/
void IIC_Start(void)
{
    SDA_OUT();     //sda�����
    gpio_write(33,1);
    gpio_write(32,1);
    delay_us(2);
    gpio_write(33,0);//START:when CLK is high,DATA change form high to low
    delay_us(2);
    gpio_write(32,0);//ǯסI2C���ߣ�׼�����ͻ��������
}

/*********************************************************
��  �ܣ�����IICֹͣ�ź�
��  ��: ��
����ֵ: ��
**********************************************************/
void IIC_Stop(void)
{
    SDA_OUT();//sda�����
    gpio_write(32,0);
    gpio_write(33,0);//STOP:when CLK is high DATA change form low to high
    delay_us(2);
    gpio_write(32,1);
    gpio_write(33,1);//����I2C���߽����ź�
    delay_us(2);
}

/*********************************************************
��  �ܣ��ȴ�Ӧ���źŵ���
��  ��: ��
����ֵ: 1������Ӧ��ʧ��
        0������Ӧ��ɹ�
**********************************************************/
unsigned char IIC_Wait_Ack(void)
{
    unsigned char ucErrTime=0;
    SDA_IN();      //SDA����Ϊ����
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
    gpio_write(32,0);//ʱ�����0
    return 0;
}

/*********************************************************
��  �ܣ�����ACKӦ��
��  ��: ��
����ֵ: ��
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
��  �ܣ�������ACKӦ��
��  ��: ��
����ֵ: ��
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
��  �ܣ�IIC����һ���ֽ�
��  ��: ��
����ֵ: �ӻ�����Ӧ��
		1����Ӧ��
		0����Ӧ��
**********************************************************/
void IIC_Send_Byte(unsigned char txd)
{
    unsigned char t;
    SDA_OUT();
    gpio_write(32,0);//����ʱ�ӿ�ʼ���ݴ���
    for(t=0; t<8; t++)
    {
        gpio_write(33,(txd&0x80)>>7);
        txd<<=1;
        delay_us(1);   //��TEA5767��������ʱ���Ǳ����
        gpio_write(32,1);
        delay_us(1);
        gpio_write(32,0);
        delay_us(1);
    }
}

/*********************************************************
��  �ܣ���1���ֽ�
��  ��: ack=1ʱ��ack=0ʱ
����ֵ: ����ACK������nACK
**********************************************************/
unsigned char IIC_Read_Byte(unsigned char ack)
{
    unsigned char i,receive=0;
    SDA_IN();//SDA����Ϊ����
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
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK
    return receive;
}
