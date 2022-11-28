/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ads1015.c
 *
 *  Created on: 2015-11-11
 *      Author: Bian
 */

#include "bsp.h"

#ifdef ADS1015_DRV

#include <stdio.h>
#include <errno.h>
#include <stdint.h>

#include "ls1x_io.h"
#include "ls1x_i2c_bus.h"
#include "../ls1x_i2c_bus_hw.h"

#if defined(LS1B)
#include "ls1b.h"
#elif defined(LS1C)
#include "ls1c.h"
#else
#error "No Loongson1x SoC defined."
#endif

#include "i2c/ads1015.h"

//-----------------------------------------------------------------------------

/*
 * I2C ADDRESS/BITS,
 * 	ADDR:	Ground	1001000
 *			VDD		1001001
 *			SDA		1001010
 *			SCL		1001011
 */
#define ADS1015_ADDRESS         0x48	    /* Ground 1001 000 */

#define ADS1015_BAUDRATE        1000000

//-----------------------------------------------------------------------------

#define CHECK_DONE(rt) \
	do {               \
        if (0 != rt)   \
            goto lbl_done; \
    } while (0);

//-----------------------------------------------------------------------------

static volatile uint16_t saved_config_reg = 0;

/******************************************************************************
 * common functions
 ******************************************************************************/

static void print_ads1015_config_reg(uint16_t val)
{
	printk("ADS1015 Config-Convert Register:\r\n");
	printk("OS = %s\r\n",
			(val & ADS1015_REG_CONFIG_OS_MASK)==ADS1015_REG_CONFIG_OS_NOTBUSY ? "device is not performing a conversion" :
			(val & ADS1015_REG_CONFIG_OS_MASK)==ADS1015_REG_CONFIG_OS_BUSY    ? "conversion is in progress" : "ERROR");
	printk("MUX = %s\r\n",
			(val & ADS1015_REG_CONFIG_MUX_MASK)==ADS1015_REG_CONFIG_MUX_DIFF_0_1 ? "Differential P = AIN0, N = AIN1" :
			(val & ADS1015_REG_CONFIG_MUX_MASK)==ADS1015_REG_CONFIG_MUX_DIFF_0_3 ? "Differential P = AIN0, N = AIN3" :
			(val & ADS1015_REG_CONFIG_MUX_MASK)==ADS1015_REG_CONFIG_MUX_DIFF_1_3 ? "Differential P = AIN1, N = AIN3" :
			(val & ADS1015_REG_CONFIG_MUX_MASK)==ADS1015_REG_CONFIG_MUX_DIFF_2_3 ? "Differential P = AIN2, N = AIN3" :
			(val & ADS1015_REG_CONFIG_MUX_MASK)==ADS1015_REG_CONFIG_MUX_SINGLE_0 ? "Single-ended AIN0" :
			(val & ADS1015_REG_CONFIG_MUX_MASK)==ADS1015_REG_CONFIG_MUX_SINGLE_1 ? "Single-ended AIN1" :
			(val & ADS1015_REG_CONFIG_MUX_MASK)==ADS1015_REG_CONFIG_MUX_SINGLE_2 ? "Single-ended AIN2" :
			(val & ADS1015_REG_CONFIG_MUX_MASK)==ADS1015_REG_CONFIG_MUX_SINGLE_3 ? "Single-ended AIN3" : "ERROR");
	printk("PGA = %s\r\n",
			(val & ADS1015_REG_CONFIG_PGA_MASK)==ADS1015_REG_CONFIG_PGA_6_144V ? "+/-6.144V range, Gain 2/3" :
			(val & ADS1015_REG_CONFIG_PGA_MASK)==ADS1015_REG_CONFIG_PGA_4_096V ? "+/-4.096V range, Gain 1" :
			(val & ADS1015_REG_CONFIG_PGA_MASK)==ADS1015_REG_CONFIG_PGA_2_048V ? "+/-2.048V range, Gain 2" :
			(val & ADS1015_REG_CONFIG_PGA_MASK)==ADS1015_REG_CONFIG_PGA_1_024V ? "+/-1.024V range, Gain 4" :
			(val & ADS1015_REG_CONFIG_PGA_MASK)==ADS1015_REG_CONFIG_PGA_0_512V ? "+/-0.512V range, Gain 8" :
			(val & ADS1015_REG_CONFIG_PGA_MASK)==ADS1015_REG_CONFIG_PGA_0_256V ? "+/-0.256V range, Gain 16" : "ERROR");
	printk("MODE = %s\r\n",
			(val & ADS1015_REG_CONFIG_MODE_MASK)==ADS1015_REG_CONFIG_MODE_CONTIN ? "Continuous conversion mode" :
			(val & ADS1015_REG_CONFIG_MODE_MASK)==ADS1015_REG_CONFIG_MODE_SINGLE ? "Power-down single-shot mode" : "ERROR");
	printk("RATE = %s\r\n",
			(val & ADS1015_REG_CONFIG_DR_MASK)==ADS1015_REG_CONFIG_DR_128SPS  ? "128  samples per second" :
			(val & ADS1015_REG_CONFIG_DR_MASK)==ADS1015_REG_CONFIG_DR_250SPS  ? "250  samples per second" :
			(val & ADS1015_REG_CONFIG_DR_MASK)==ADS1015_REG_CONFIG_DR_490SPS  ? "490  samples per second" :
			(val & ADS1015_REG_CONFIG_DR_MASK)==ADS1015_REG_CONFIG_DR_920SPS  ? "920  samples per second" :
			(val & ADS1015_REG_CONFIG_DR_MASK)==ADS1015_REG_CONFIG_DR_1600SPS ? "1600 samples per second" :
			(val & ADS1015_REG_CONFIG_DR_MASK)==ADS1015_REG_CONFIG_DR_2400SPS ? "2400 samples per second" :
			(val & ADS1015_REG_CONFIG_DR_MASK)==ADS1015_REG_CONFIG_DR_3300SPS ? "3300 samples per second" : "ERROR");
	printk("CMODE = %s\r\n",
			(val & ADS1015_REG_CONFIG_CMODE_MASK)==ADS1015_REG_CONFIG_CMODE_TRAD   ? "Traditional comparator with hysteresis" :
			(val & ADS1015_REG_CONFIG_CMODE_MASK)==ADS1015_REG_CONFIG_CMODE_WINDOW ? "Window comparator" : "ERROR");
	printk("CPOL = %s\r\n",
			(val & ADS1015_REG_CONFIG_CPOL_MASK)==ADS1015_REG_CONFIG_CPOL_ACTVLOW ? "ALERT/RDY pin is low when active" :
			(val & ADS1015_REG_CONFIG_CPOL_MASK)==ADS1015_REG_CONFIG_CPOL_ACTVHI  ? "ALERT/RDY pin is high when active" : "ERROR");
	printk("CPOL = %s\r\n",
			(val & ADS1015_REG_CONFIG_CLAT_MASK)==ADS1015_REG_CONFIG_CLAT_NONLAT ? "Non-latching comparator (default)" :
			(val & ADS1015_REG_CONFIG_CLAT_MASK)==ADS1015_REG_CONFIG_CLAT_LATCH  ? "Latching comparator" : "ERROR");
	printk("CPOL = %s\r\n",
			(val & ADS1015_REG_CONFIG_CQUE_MASK) == ADS1015_REG_CONFIG_CQUE_1CONV ? "Assert ALERT/RDY after one conversions" :
			(val & ADS1015_REG_CONFIG_CQUE_MASK) == ADS1015_REG_CONFIG_CQUE_2CONV ? "Assert ALERT/RDY after two conversions" :
			(val & ADS1015_REG_CONFIG_CQUE_MASK) == ADS1015_REG_CONFIG_CQUE_4CONV ? "Assert ALERT/RDY after four conversions" :
			(val & ADS1015_REG_CONFIG_CQUE_MASK) == ADS1015_REG_CONFIG_CQUE_NONE ? "Disable the comparator and put ALERT/RDY in high state" : "ERROR");
}

/*
 * read or write config/lowthresh/highthresh register
 * rw: FALSE=write TRUE=read
 */
static int ADS1015_reg_access(LS1x_I2C_bus_t *pIIC, uint8_t regNum, uint16_t *regVal, bool rw)
{
    int rt, rw_cnt;
	unsigned char buf[4] = { 0 };

	if (regNum >= ADS1015_REGISTER_COUNT)
		return -1;

	/* start transfer */
	rt = ls1x_i2c_send_start(pIIC, ADS1015_ADDRESS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls1x_i2c_ioctl(pIIC, IOCTL_SPI_I2C_SET_TFRMODE, (void *)ADS1015_BAUDRATE);
	CHECK_DONE(rt);

	/* address device, FALSE = WRITE */
	rt = ls1x_i2c_send_addr(pIIC, ADS1015_ADDRESS, false);
	CHECK_DONE(rt);

	/*
	 * first, 1st byte is REGISTER to be accessed.
	 */
	buf[0] = regNum;
	rw_cnt = ls1x_i2c_write_bytes(pIIC, buf, 1);
	if (rw_cnt < 0)
		rt = rw_cnt;
	CHECK_DONE(rt);

	if (rw)		/* read */
	{
		/* restart - address device address device, TRUE = READ
		 */
		rt = ls1x_i2c_send_addr(pIIC, ADS1015_ADDRESS, true);
		CHECK_DONE(rt);

		/*
		 * fetch register data
		 */
	    rw_cnt = ls1x_i2c_read_bytes(pIIC, buf+1, 2);
        if (rw_cnt < 0)
		    rt = rw_cnt;
		CHECK_DONE(rt);

		*regVal = ((uint16_t)buf[1] << 8) + buf[2];
	}
	else		/* write */
	{
		buf[1] = *regVal >> 8;
		buf[2] = *regVal;

		rt = ls1x_i2c_write_bytes(pIIC, buf+1, 2);
	}

lbl_done:
	/* terminate transfer */
	ls1x_i2c_send_stop(pIIC, ADS1015_ADDRESS);

	return rt;
}

/******************************************************************************
 * driver implement
 ******************************************************************************/

/*
 * read convert result of ads1015, ONLY CONTINUOUS MODE
 */
STATIC_DRV int ADS1015_read(void *bus, void *buf, int size, void *arg)
{
    int rt = 0, channel = (int)arg;
	uint16_t *pVal = (uint16_t *)buf, cfgVal;

    if ((bus == NULL) || (buf == NULL))
        return -1;

    if (saved_config_reg == 0)    /* The first time read */
    {
        saved_config_reg = ADS1015_CFG_DEFAULT; // use deault config
    }

    if (channel > 0)            /* Parameter has channel to read */
    {
        cfgVal = saved_config_reg & ~ADS1015_REG_CONFIG_MUX_MASK;

        switch (channel)
        {
            case ADS1015_CHANNEL_D0: cfgVal |= ADS1015_REG_CONFIG_MUX_DIFF_0_1; break;
            case ADS1015_CHANNEL_D1: cfgVal |= ADS1015_REG_CONFIG_MUX_DIFF_0_3; break;
            case ADS1015_CHANNEL_D2: cfgVal |= ADS1015_REG_CONFIG_MUX_DIFF_1_3; break;
            case ADS1015_CHANNEL_D3: cfgVal |= ADS1015_REG_CONFIG_MUX_DIFF_2_3; break;

            case ADS1015_CHANNEL_S0: cfgVal |= ADS1015_REG_CONFIG_MUX_SINGLE_0; break;
            case ADS1015_CHANNEL_S1: cfgVal |= ADS1015_REG_CONFIG_MUX_SINGLE_1; break;
            case ADS1015_CHANNEL_S2: cfgVal |= ADS1015_REG_CONFIG_MUX_SINGLE_2; break;
            case ADS1015_CHANNEL_S3: cfgVal |= ADS1015_REG_CONFIG_MUX_SINGLE_3; break;
        
            default: cfgVal = 0; break;
        }
        
        if (cfgVal != 0)        /* Channel is ok */
        {
            rt = ADS1015_reg_access((LS1x_I2C_bus_t *)bus,
                                    ADS1015_REG_POINTER_CONFIG,
                                    &cfgVal,
                                    false); // write
            if (rt != 2)
                return rt;
                
            delay_ms(2);
        }
    }

	/******************************************************
	 * read convert register
	 ******************************************************/

	rt = ADS1015_reg_access((LS1x_I2C_bus_t *)bus,
                            ADS1015_REG_POINTER_CONVERT,
                            pVal,
                            true); // read
	if (rt == 0)
		*pVal >>= 4;

	return rt;
}

STATIC_DRV int ADS1015_ioctl(void *bus, int cmd, void *arg)
{
	int rt = 0;
	uint16_t val = 0;
    LS1x_I2C_bus_t *pIIC = (LS1x_I2C_bus_t *)bus;

    if (bus == NULL)
        return -1;
        
	switch (cmd)
	{
		/* access full-config register */
		case IOCTL_ADS1015_SET_CONV_CTRL:
			val = (uint16_t)((unsigned)arg);
			rt = ADS1015_reg_access(pIIC, ADS1015_REG_POINTER_CONFIG, &val, false);
			saved_config_reg = val;
			break;

		case IOCTL_ADS1015_GET_CONV_CTRL:
			rt = ADS1015_reg_access(pIIC, ADS1015_REG_POINTER_CONFIG, &val, true);
			*(uint16_t *)arg = val;
            saved_config_reg = val;
			break;

		/* access low thresh register */
		case IOCTL_ADS1015_SET_LOW_THRESH:
			val = (uint16_t)((unsigned)arg);
			rt = ADS1015_reg_access(pIIC, ADS1015_REG_POINTER_LOWTHRESH, &val, false);
			break;

		case IOCTL_ADS1015_GET_LOW_THRESH:
			rt = ADS1015_reg_access(pIIC, ADS1015_REG_POINTER_LOWTHRESH, &val, true);
			*(uint16_t *)arg = val;
			break;

		/* access high thresh register */
		case IOCTL_ADS1015_SET_HIGH_THRESH:
			val = (uint16_t)((unsigned)arg);
			rt = ADS1015_reg_access(pIIC, ADS1015_REG_POINTER_HITHRESH, &val, false);
			break;

		case IOCTL_ADS1015_GET_HIGH_THRESH:
			rt = ADS1015_reg_access(pIIC, ADS1015_REG_POINTER_HITHRESH, &val, true);
			*(uint16_t *)arg = val;
			break;

		/* display config register */
		case IOCTL_ADS1015_DISP_CONFIG_REG:
		{
			rt = ADS1015_reg_access(pIIC, ADS1015_REG_POINTER_CONFIG, &val, true);
			if (rt == 0)
				print_ads1015_config_reg(val);
			break;
		}

		default:
			break;
	}

	return rt;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * ADS1015 driver operators
 */
static driver_ops_t LS1x_ADS1015_drv_ops =
{
    .init_entry  = NULL,
    .open_entry  = NULL,
    .close_entry = NULL,
    .read_entry  = ADS1015_read,
    .write_entry = NULL,
    .ioctl_entry = ADS1015_ioctl,
};
driver_ops_t *ls1x_ads1015_drv_ops = &LS1x_ADS1015_drv_ops;
#endif

/******************************************************************************
 * user api
 */
uint16_t get_ads1015_adc(void *bus, int channel)
{
    if (ADS1015_ioctl(bus,
                      IOCTL_ADS1015_SET_CONV_CTRL,
                      (void *)(channel | ADS1015_CFG_DEFAULT)) >= 0)
    {
        uint16_t val;
        delay_ms(2);
        if (ADS1015_read(bus, (void*)&val, 2, NULL) >= 0)
            return val;
    }

    return 0;
}

#endif



