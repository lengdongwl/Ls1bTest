/*
 * @Description: ���ȿ�������
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
void fan_resistance_io_Config(void);//��ʼ����������ȵ���
void Fan_Control(unsigned char level);//���ȿ��� 0~100
void Resistance_Control(unsigned char on);//���ȵ��迪��

#endif // _FAN_ RESISTANCE_CONTROL_DRV_H

