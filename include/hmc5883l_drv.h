/*
 * @Description: 
 * @Autor: 309 Mushroom
 * @Date: 2022-11-12 20:25:03
 * @LastEditors: 309 Mushroom
 * @LastEditTime: 2022-11-20 10:58:22
 */
/*
 * hmc5883l_drv.h
 *
 * created: 2022/7/14
 *  author: 
 */

#ifndef _HMC5883L_DRV_H
#define _HMC5883L_DRV_H


#include "stdio.h"

#include "ls1b.h"
#include "mips.h"
#include "stdint.h"
#include "ls1b_gpio.h"

#define HMC_SCL_H gpio_write(28,1)
#define HMC_SCL_L gpio_write(28,0)

#define HMC_SDA_H gpio_write(29,1)
#define HMC_SDA_L gpio_write(29,0)

#define HMC_SDA gpio_read(29)

void HCM5883L_Init(void);
double HCM5883L_Get_Angle(void);

#endif // _HMC5883L_DRV_H

