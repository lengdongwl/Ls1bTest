/*
 * @Description: 
 * @Autor: 309 Mushroom
 * @Date: 2022-11-17 19:34:27
 * @LastEditors: 309 Mushroom
 * @LastEditTime: 2022-11-20 12:35:23
 */
/*
 * motor_drv.h
 *
 * created: 2022/7/15
 *  author: 
 */

#ifndef _MOTOR_DRV_H
#define _MOTOR_DRV_H
#include "stdint.h"
#define GPIO_Motor 53

void Motor_set_pwm(int n);//���õ��pwmռ�ձ�0-100
unsigned int Motor_get_count(void);//��ȡ����
void Motor_count_init();//��ʼ��count����
void Motor_GPIO_Config(void);//��ʼ�����
void Motor_Interrupt_Init(void);//�ⲿ�жϳ�ʼ�� ע����Ӱ�췢freeRTOS���� ʹ��ʱ��ʱ����
/*
uint16_t Motor_Speed_Check(void);
void Inc_MOTOR_PIDInit(void);
unsigned int Loc_MOTOR_PIDCalc(int NextPoint);
uint16_t Motor_Speed_Check2(void);
*/
#endif // _MOTOR_DRV_H

