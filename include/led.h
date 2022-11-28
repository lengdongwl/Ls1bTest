/*
 * led.h
 *
 * created: 2022/7/6
 * authour: 
 */

#ifndef _LED_H
#define _LED_H
#include "mylib.h"
#define LED1 34//RGB��
#define LED2 37 //RGB ��
#define LED3 35 //RGB ��


extern uint8_t RGB_COLOR_RED[];//��ɫ
extern uint8_t  RGB_COLOR_GREEN [];//��ɫ
extern uint8_t  RGB_COLOR_BLUE [];//��ɫ
extern uint8_t  RGB_COLOR_YELLOW [];//��ɫ
extern uint8_t  RGB_COLOR_SORT [];//Ʒɫ
extern uint8_t  RGB_COLOR_SYAN [];//��ɫ
extern uint8_t  RGB_COLOR_WHITE [];//��ɫ
extern uint8_t  RGB_COLOR_BLACK [];//��ɫ ��ɫ

void RGB_Init(void);		//LED��ʼ��
void RGB_DeInit(void);		//ɾ����ʼ��
void RGB(	uint8_t R_Data,
					uint8_t G_Data,
					uint8_t B_Data);//RGB��ɫ����
void RGB_Set(uint8_t *RGB_COLOR_x);

#endif // _LED_H

