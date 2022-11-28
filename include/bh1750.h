/*
 * bh1750.h
 *
 * created: 2022/3/2
 *  author:
 */

#ifndef _BH1750_H
#define _BH1750_H
#include "stdint.h"
#define BH1750_Addr 0x23				//0100 011
#define BH1750_ON   0x01
#define BH1750_CON  0x10
#define BH1750_ONE  0x20
#define BH1750_RSET 0x07

void BH1750_Init(void);					//IO初始化，
void Start_BH1750(void);				//上电，设置清除数据寄存器
void Read_BH1750(void);					//连续的读取内部寄存器数据
void Convert_BH1750(void);
uint16_t BH1750_Test(void);

#endif // _BH1750_H


