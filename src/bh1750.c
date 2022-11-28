/*
 * bh1750.c
 *
 * created: 2022/3/2
 *  author:
 */

#include "ls1b.h"
#include "ls1b_gpio.h"
#include "bsp.h"
#include "bh1750.h"
#include "ls1x_i2c_bus.h"
  #include "ls1x_fb.h"
float result_lx=0;
unsigned char BUF[2]= {0};
unsigned int result1750=0;
extern unsigned char tmp_buf[16];

/**************************************************************
*功  能：端口初始化
*参  数: 无
*返回值: 无
**************************************************************/
void BH1750_Init(void)
{

}

/**************************************************************
*功  能：发送设备地址
*参  数: 无
*返回值: 无
**************************************************************/
#define BH1750_BAUDRATE        1000000
void Cmd_Write_BH1750(unsigned char cmd)
{
    unsigned char data[2]= {0};
    data[0]=cmd;

    ls1x_i2c_send_start(busI2C0,BH1750_Addr);//起始信号
    ls1x_i2c_send_addr(busI2C0,BH1750_Addr,0);	//发送设备地址+写信号
    ls1x_i2c_write_bytes(busI2C0, data, 1);	//内部寄存器地址
    ls1x_i2c_send_stop(busI2C0,BH1750_Addr);	//发送停止信号
    delay_ms(5);
}


/**************************************************************
*功  能：开启一次H分辨率模式
*参  数: 无
*返回值: 无
**************************************************************/
void Start_BH1750(void)
{
    Cmd_Write_BH1750(BH1750_ON);	 		//power on
    Cmd_Write_BH1750(BH1750_RSET);		//clear
    Cmd_Write_BH1750(BH1750_ONE);  		//一次H分辨率模式，至少120ms，之后自动断电模式
}

/**************************************************************
*功  能：读光照信号
*参  数: 无
*返回值: 无
**************************************************************/
void Read_BH1750(void)
{
    ls1x_i2c_send_start(busI2C0, BH1750_Addr);//开始信号
    ls1x_i2c_send_addr(busI2C0,BH1750_Addr,1);//发送设备地址加读信号
    ls1x_i2c_read_bytes(busI2C0, BUF, 2);//读取两个字节数据
    ls1x_i2c_send_stop(busI2C0,BH1750_Addr);//发送停止信号
}

/**************************************************************
*功  能：合成光照数据
*参  数: 无
*返回值: 无
**************************************************************/
void Convert_BH1750(void)
{
    result1750=BUF[0];
    result1750=(result1750<<8)+BUF[1];		//合成数据，即光照数据
    result1750=(float)result1750/1.2;

}

/**************************************************************
*功  能：显示光照数据
*参  数: 无
*返回值: 无
**************************************************************/
uint16_t BH1750_Test(void)
{
    unsigned char buf[20] = {0};
    unsigned int lx = 0;
    Start_BH1750();						//power on
    delay_ms(120);						//延时120ms
    Read_BH1750();						//连续读取结果 到BUF里面
    Convert_BH1750();					//转换结果到result_lx
    //sprintf((char *)buf,"%d lux", result1750);
    //fb_textout(160, 70, buf);
    delay_ms(200);
    //fb_fillrect(160, 70, 480, 106, cidxBLACK);
    return result1750;
}


