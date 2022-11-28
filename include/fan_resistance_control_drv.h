/*
 * @Description: 风扇控制驱动
 * @Autor: 309 Mushroom
 * @Date: 2022-11-12 20:26:26
 * @LastEditors: 309 Mushroom
 * @LastEditTime: 2022-11-17 19:30:02
 */


#ifndef _FAN_RESISTANCE_CONTROL_DRV_H
#define _FAN_RESISTANCE_CONTROL_DRV_H
#include "mylib.h"


#define GPIO_FUN 36
#define GPIO_R 38
void fan_resistance_io_Config(void);//初始化风扇与加热电阻
void Fan_Control(unsigned char level);//风扇控制 0~100
void Resistance_Control(unsigned char on);//加热电阻开关

#endif // _FAN_ RESISTANCE_CONTROL_DRV_H

