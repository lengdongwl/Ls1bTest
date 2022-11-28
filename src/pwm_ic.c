/*
 * pwm_ic.c
 *
 * created: 2022/3/2
 *  author: 
 */

 #include "pwm_ic.h"
 #include "ls1x_i2c_bus.h"

 #define WR_8BIT_CMD            0x03
 #define WR_16BIT_CMD           0X02
 #define GP7101_ADDRESS         0x58
 #define GP7101_BAUDRATE        1000000

 //MUX基地址和寄存器地址
 #define GPIO_MUX_BASE    0XBFD00420
 /*
  * i=0: GPIO_MUX_CTRL0; i=1: GPIO_MUX_CTRL1
  */
 #define LS1B_MUX_CFG(i)        (*(volatile unsigned int*)(GPIO_MUX_BASE+i*4))

/***************************************************************************************************
 ** 函数功能：初始化I2C1
 ** 参数：无
 ** 说明：
****************************************************************************************************/
 void IIC0_init(void)
 {
    /* i=0: GPIO30:GPIO0; i=1: GPIO61:GPIO32*/
    //LS1B_GPIO_CFG(1)  &= ~(3 << 6);    	  //设置对应PAD为普通功能
    //LS1B_MUX_CFG(0)   |= 1<< 24;          //开启IIC1的复用

    //初始化
    ls1x_i2c_initialize(busI2C0);
    //设置速率
    //ls1x_i2c_ioctl(busI2C0, IOCTL_SPI_I2C_SET_TFRMODE, (void *)GP7101_BAUDRATE);

 }

//#if 1
 //周期20ms
 //脉冲宽度范围900~2100us -- 0.9~2.1ms -->11~27 -->8位
 void Set_PWM(unsigned int brightpercent)
 {
    unsigned char brightness = brightpercent * 255.0 / 10000 ;
    unsigned char data[2] = {0};

    //8位PWM模式
    data[0] = WR_8BIT_CMD;
    //数据
    data[1] = brightness;

    //起始信号
    ls1x_i2c_send_start(busI2C0, GP7101_ADDRESS);

    //发送地址
    ls1x_i2c_send_addr(busI2C0, GP7101_ADDRESS, false);

    //写入数据
    ls1x_i2c_write_bytes(busI2C0, data, 2);

    //停止信号
    ls1x_i2c_send_stop(busI2C0, GP7101_ADDRESS);
    
 }
//#endif

//脉冲宽度范围500~1500~2500us -- 0.5~1.5~2.5ms -->1638~8192 -->16位
//8350 -- 4720 -- 1680
/***************************************************************************************************
 ** 函数功能：控制舵机的偏转角度
 ** 参数：unsigned char brightpercent：填入的数值为：0 ~ 120
 ** 说明：周期20ms
          FS90偏转角度0~120度
          脉冲宽度范围900~1500~2100us -- 0.9~2.1ms -->2949~6881 -->16位
          0刻度的数值--6650
          90刻度的数值--4915
          120刻度的数值--2940
****************************************************************************************************/
/*
void Set_PWM(unsigned short brightpercent)
{
    unsigned char data[3] = {0};
    unsigned short brightness = brightpercent;

#if 0
    if(brightpercent >= 95)
    {
        brightness = 3325 - (brightpercent - 95) * 10;//每减10，指针偏转一度
    }
    else
    {
        brightness = 6650 - brightpercent * 35;//每减35，指针偏转一度
    }
#endif

#if 1
    if(brightpercent >= 140)
    {
        brightness = 2880 - (brightpercent - 140) * 30; //每减30，指针偏转一度
    }
    else
    {
        brightness = 8350 - brightpercent * 40;         //每减40，指针偏转一度
    }
#endif

    //16位PWM模式
    data[0] = WR_16BIT_CMD;
    //数据
    data[1] = brightness;
    data[2] = brightness >> 8;

    //起始信号
    ls1x_i2c_send_start(busI2C1, GP7101_ADDRESS);

    //发送地址
    ls1x_i2c_send_addr(busI2C1, GP7101_ADDRESS, false);

    //写入数据
    ls1x_i2c_write_bytes(busI2C1, data, 3);

    //停止信号
    ls1x_i2c_send_stop(busI2C1, GP7101_ADDRESS);
}

*/


