/*
 * @Description: 
 * @Autor: 309 Mushroom
 * @Date: 2022-11-12 20:26:26
 * @LastEditors: 309 Mushroom
 * @LastEditTime: 2022-11-20 11:42:03
 */

#include "ls1b.h"
#include "mips.h"
#include "ls1b_gpio.h"
#include "fan_resistance_control_drv.h"
#include "i2c/gp7101.h"
void fan_resistance_io_Config(void)
{
    gpio_enable(GPIO_R,DIR_OUT);// 加热电阻
    gpio_enable(GPIO_FUN,DIR_OUT);//电动风扇
    gpio_write(GPIO_R,0);//加热电阻默认关闭
    gpio_write(GPIO_FUN,0);//风扇控制引默认关闭
    Fan_Control(0);
}


/**
 * @description: 加热电阻开关
 * @param {unsigned char} on
 * @return {*}
 */
void Resistance_Control(unsigned char on)
{
    gpio_write(GPIO_R,on);
}

/**
 * @description: 风扇转速
 * @param {unsigned char} level 0-100
 * @return {*}
 */
void Fan_Control(unsigned char level)
{
    if (level==0)
    {
        gpio_write(GPIO_FUN,0);//风扇控制引关闭
    }else
    {
        gpio_write(GPIO_FUN,1);//风扇控制引开启
    }
    
    set_lcd_brightness(level);
}


