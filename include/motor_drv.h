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

void Motor_set_pwm(int n);//设置电机pwm占空比0-100
unsigned int Motor_get_count(void);//获取次数
void Motor_count_init();//初始化count参数
void Motor_GPIO_Config(void);//初始化电机
void Motor_Interrupt_Init(void);//外部中断初始化 注：会影响发freeRTOS运行 使用时及时开关
/*
uint16_t Motor_Speed_Check(void);
void Inc_MOTOR_PIDInit(void);
unsigned int Loc_MOTOR_PIDCalc(int NextPoint);
uint16_t Motor_Speed_Check2(void);
*/
#endif // _MOTOR_DRV_H

