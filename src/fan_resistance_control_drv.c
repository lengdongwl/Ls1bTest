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
    gpio_enable(GPIO_R,DIR_OUT);// ���ȵ���
    gpio_enable(GPIO_FUN,DIR_OUT);//�綯����
    gpio_write(GPIO_R,0);//���ȵ���Ĭ�Ϲر�
    gpio_write(GPIO_FUN,0);//���ȿ�����Ĭ�Ϲر�
    Fan_Control(0);
}


/**
 * @description: ���ȵ��迪��
 * @param {unsigned char} on
 * @return {*}
 */
void Resistance_Control(unsigned char on)
{
    gpio_write(GPIO_R,on);
}

/**
 * @description: ����ת��
 * @param {unsigned char} level 0-100
 * @return {*}
 */
void Fan_Control(unsigned char level)
{
    if (level==0)
    {
        gpio_write(GPIO_FUN,0);//���ȿ������ر�
    }else
    {
        gpio_write(GPIO_FUN,1);//���ȿ���������
    }
    
    set_lcd_brightness(level);
}


