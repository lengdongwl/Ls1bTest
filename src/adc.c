/*
 * @Description: 龙芯ADC驱动
 * @Autor: 309 Mushroom
 * @Date: 2022-11-17 19:56:44
 * @LastEditors: 309 Mushroom
 * @LastEditTime: 2022-11-20 19:34:48
 */
/*
 * adc.c
 *
 * created: 2022/11/17
 *  author: 
 */
#include "ls1x_i2c_bus.h"
#include "i2c/mcp4725.h"
#include "ls1b_gpio.h"
#include "adc.h"
void adc_init()
{
    ls1x_ads1015_ioctl(busI2C0,IOCTL_ADS1015_DISP_CONFIG_REG,NULL);
}

/**
 * @description: ADC芯片读取 具体看手册
 * @param {*}
 * @return {*}
 */
unsigned short adc_get(unsigned int ADCx)
{
    //in_v1 = 4.096*2*adc1/4096;//采集电压的转换公式
    return get_ads1015_adc(busI2C0, ADCx);
}


float adc_get_temp(void)
{
        uint16_t adc2=0;
        float temp;
        adc2 = adc_get(ADC3);
        temp = 4.096*2*adc2/4096;//采集电压的转换公式
        temp=temp*100;
        return temp;
}

