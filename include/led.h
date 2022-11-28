/*
 * led.h
 *
 * created: 2022/7/6
 * authour: 
 */

#ifndef _LED_H
#define _LED_H
#include "mylib.h"
#define LED1 34//RGB红
#define LED2 37 //RGB 绿
#define LED3 35 //RGB 蓝


extern uint8_t RGB_COLOR_RED[];//红色
extern uint8_t  RGB_COLOR_GREEN [];//绿色
extern uint8_t  RGB_COLOR_BLUE [];//蓝色
extern uint8_t  RGB_COLOR_YELLOW [];//黄色
extern uint8_t  RGB_COLOR_SORT [];//品色
extern uint8_t  RGB_COLOR_SYAN [];//青色
extern uint8_t  RGB_COLOR_WHITE [];//白色
extern uint8_t  RGB_COLOR_BLACK [];//黑色 无色

void RGB_Init(void);		//LED初始化
void RGB_DeInit(void);		//删除初始化
void RGB(	uint8_t R_Data,
					uint8_t G_Data,
					uint8_t B_Data);//RGB颜色控制
void RGB_Set(uint8_t *RGB_COLOR_x);

#endif // _LED_H

