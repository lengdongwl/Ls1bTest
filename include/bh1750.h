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

void BH1750_Init(void);					//IO��ʼ����
void Start_BH1750(void);				//�ϵ磬����������ݼĴ���
void Read_BH1750(void);					//�����Ķ�ȡ�ڲ��Ĵ�������
void Convert_BH1750(void);
uint16_t BH1750_Test(void);

#endif // _BH1750_H


