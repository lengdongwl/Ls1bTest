/*
 * @Description: 
 * @Autor: 309 Mushroom
 * @Date: 2022-11-17 11:15:47
 * @LastEditors: 309 Mushroom
 * @LastEditTime: 2022-11-21 09:43:20
 */
/*
 * uart.h
 *
 * created: 2021/4/30
 *  author:
 */

#ifndef _UART_H
#define _UART_H
#include "mylib.h"


#ifdef BKRC_VOICE //syn7318语音模块
#define Rbuf_size 5
#else //小创语音模块
#define Rbuf_size 4
#endif


extern char UART4_Rbuf[Rbuf_size];

void UART4_Config_Init(void);
bool UART4_Deal(void);
void UART4_Send_String(char *Send_Data, int Size);
void UART4_Send(char Send_Data);
uint8_t XiaoChuang_ASR(void);
void XiaoChuang_PlayNUM(int number);
#endif // _UART_H


