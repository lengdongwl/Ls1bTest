/*
 * @Description: 
 * @Autor: 309 Mushroom
 * @Date: 2022-11-17 19:34:37
 * @LastEditors: 309 Mushroom
 * @LastEditTime: 2022-11-20 19:43:28
 */
/*
 * motor_drv.c
 *
 * created: 2022/7/15
 *  author:
 */
#include <stdio.h>

#include "ls1b.h"
#include "mips.h"
#include "bsp.h"
#include "motor_drv.h"
#include "ls1b_gpio.h"
#include "i2c/gp7101.h"
#include "ls1b_irq.h"
#include "stdint.h"

unsigned int count=0; //编码器中断触发次数

void Motor_GPIO_Config(void)
{

    Motor_set_pwm(50);
    Motor_count_init();
}


//外部中断服务函数


static void gpio_interrupt_isr(int vector, void * param)
{
    count++;
    return;
}

//外部中断初始化
void Motor_Interrupt_Init(void)
{
    //ls1x_install_gpio_isr(KEY_UP, INT_TRIG_EDGE_UP, gpio_interrupt_isr, 0);    /* 上升沿触发 */
    ls1x_install_gpio_isr(GPIO_Motor, INT_TRIG_EDGE_DOWN, gpio_interrupt_isr, 0);  /* 下降沿触发 */
    //ls1x_install_gpio_isr(GPIO_Motor, INT_TRIG_LEVEL_HIGH, gpio_interrupt_isr, 0);  /* 高电平触发 */
    //ls1x_install_gpio_isr(KEY_UP, INT_TRIG_LEVEL_LOW, gpio_interrupt_isr, 0);  /* 低电平触发 */
    ls1x_enable_gpio_interrupt(GPIO_Motor);    //使能中断

    return ;
}

void Motor_Interrupt_DeInit(void)
{
    ls1x_remove_gpio_isr(GPIO_Motor);  
    ls1x_disable_gpio_interrupt(GPIO_Motor);    
    return ;
}


void Motor_count_init()
{
    count=0;
}
unsigned int Motor_get_count(void)
{
    return count;
}

void Motor_set_pwm(int n)
{
    set_lcd_brightness(n);//设置PMW值 GP7101挂接在I2C0
}
