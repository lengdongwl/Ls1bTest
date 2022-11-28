/*
 * @Description: 
 * @Autor: 309 Mushroom
 * @Date: 2022-11-17 19:59:39
 * @LastEditors: 309 Mushroom
 * @LastEditTime: 2022-11-20 11:22:14
 */
/*
 * adc.h
 *
 * created: 2022/11/17
 *  author: 
 */

#ifndef _ADC_H
#define _ADC_H
#include "i2c/ads1015.h"
#define ADC1 ADS1015_REG_CONFIG_MUX_SINGLE_0
#define ADC2 ADS1015_REG_CONFIG_MUX_SINGLE_1
#define ADC3 ADS1015_REG_CONFIG_MUX_SINGLE_2
#define ADC4 ADS1015_REG_CONFIG_MUX_SINGLE_3

void adc_init();//ADC初始化
unsigned short adc_get(unsigned int ADCx);//读取ADC的值
float adc_get_temp(void);//读取风扇温度
#endif // _ADC_H

