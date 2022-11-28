/*
 * @Description: 数码管驱动库
 * @Autor: 309 Mushroom
 * @Date: 2022-11-12 20:18:47
 * @LastEditors: 309 Mushroom
 * @LastEditTime: 2022-11-17 09:55:15
 */
#ifndef __HC595_H
#define __HC595_H
#include "mylib.h"
#include "ls1b_gpio.h"
#include "bsp.h"
#include "FreeRTOS.h"
#include "task.h"

#define GPIO_SMG_SI 39
#define GPIO_SMG_SCK 49
#define GPIO_SMG_RCK 48
#define GPIO_SMG_COM1 45
#define GPIO_SMG_COM2 44
#define GPIO_SMG_COM3 43
#define GPIO_SMG_COM4 42

#define GPIO_x 

#define SER_H gpio_write(GPIO_SMG_SI, 1); //75HC595数据输入段
#define SER_L gpio_write(GPIO_SMG_SI, 0); //75HC595数据输入段
#define SCK_H gpio_write(GPIO_SMG_SCK, 1); //75HC595串行时钟
#define SCK_L gpio_write(GPIO_SMG_SCK, 0); //75HC595串行时钟
#define RCK_H gpio_write(GPIO_SMG_RCK, 1); //75HC595并行时钟
#define RCK_L gpio_write(GPIO_SMG_RCK, 0); //75HC595并行时钟
//#define OE_H gpio_write(GPIO_x, 1);  //75HC595输出使能控制端
//#define OE_L gpio_write(GPIO_x, 0);  //75HC595并行时钟

#define SMG_1_H gpio_write(GPIO_SMG_COM1, 1); //数码管位选1
#define SMG_1_L gpio_write(GPIO_SMG_COM1, 0); //数码管位选1
#define SMG_2_H gpio_write(GPIO_SMG_COM2, 1); //数码管位选2
#define SMG_2_L gpio_write(GPIO_SMG_COM2, 0); //数码管位选2
#define SMG_3_H gpio_write(GPIO_SMG_COM3, 1); //数码管位选3
#define SMG_3_L gpio_write(GPIO_SMG_COM3, 0); //数码管位选3
#define SMG_4_H gpio_write(GPIO_SMG_COM4, 1); //数码管位选4
#define SMG_4_L gpio_write(GPIO_SMG_COM4, 0); //数码管位选4

extern uint8_t SMGSWAP; //数码管位选选择 便于调节

void SMG_Init(void); //数码管初始化
void SMG_DeInit(void);
void SMG_Display_Bit(uint8_t Input_Data, uint8_t Bit_Data); //数码管单个位显示
void SMG_Display(uint16_t Input_Data, uint8_t Ctrl_Bit);    //数码管显示
void SMG_DisplayP(int16_t Input_Data4, int16_t Input_Data3, int16_t Input_Data2, int16_t Input_Data1, uint8_t Ctrl_Bit);
void SMG_Position(uint8_t Bit_Data);//软件修改数码管顺序
#endif
