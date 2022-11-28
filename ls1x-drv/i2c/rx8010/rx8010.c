/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * rx8010.c
 *
 * created: 2020/9/11
 * authour: Bian
 *
 * TODO debug alarm & interrupt
 */

#include "bsp.h"

#ifdef RX8010_DRV

#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>

#if defined(LS1B)
#include "ls1b.h"
#elif defined(LS1C)
#include "ls1c.h"
#else
#error "No Loongson1x SoC defined."
#endif

#include "ls1x_io.h"

#include "ls1x_i2c_bus.h"
#include "../ls1x_i2c_bus_hw.h"

#include "rx8010_hw.h"
#include "i2c/rx8010.h"

//-----------------------------------------------------------------------------

#define RX8010_ADDRESS          0x32

#define RX8010_BAUDRATE         1000000

/*
 * 开发板在硬件设计上把 IRQ1/IRQ2 连接到PCA9557, 只能查询
 */
#define RX8010_USE_INTERRUPT        0
#if (RX8010_USE_INTERRUPT)
//#error "you should confirm IRQ1/IRQ2 use the gpio interrupt."
#endif

//-----------------------------------------------------------------------------

#define CHECK_DONE(rt) \
	do {               \
        if (0 != rt)   \
            goto lbl_done; \
    } while (0);

static unsigned char saved_rx8010_ctrl = 0;

//-----------------------------------------------------------------------------
// local routines
//-----------------------------------------------------------------------------

/*
 * BCD码转为二进制
 */
static inline unsigned bcd2bin(unsigned char val)
{
	return (val & 0x0f) + (val >> 4) * 10;
}

/*
 * 二进制转为BCD码
 */
static inline unsigned char bin2bcd(unsigned val)
{
	return ((val / 10) << 4) + val % 10;
}

//-------------------------------------------------------------------------------------------------
// rx8010_read_regs()
// reads a specified number of rx8010 registers (see Register defines)
// See also rx8010_read_reg() to read single register.
//-------------------------------------------------------------------------------------------------

static int RX8010_read_regs(LS1x_I2C_bus_t *pIIC, int regNum, unsigned char length, unsigned char *regValues)
{
    int rt, rw_cnt;
	unsigned char regAddr;

	/* start transfer */
	rt = ls1x_i2c_send_start(pIIC, RX8010_ADDRESS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls1x_i2c_ioctl(pIIC, IOCTL_SPI_I2C_SET_TFRMODE, (void *)RX8010_BAUDRATE);
	CHECK_DONE(rt);

	/* address device, FALSE = WRITE */
	rt = ls1x_i2c_send_addr(pIIC, RX8010_ADDRESS, false);
	CHECK_DONE(rt);

	/*
	 * first, 1st byte is REGISTER to be accessed.
	 */
	regAddr = (unsigned char)regNum;
	rw_cnt = ls1x_i2c_write_bytes(pIIC, &regAddr, 1);
	if (rw_cnt < 0)
		rt = rw_cnt;
	CHECK_DONE(rt);

	/* restart - address device address device, TRUE = READ
	 */
	rt = ls1x_i2c_send_addr(pIIC, RX8010_ADDRESS, true);
	CHECK_DONE(rt);

	/*
	 * fetch register data
	 */
	rw_cnt = ls1x_i2c_read_bytes(pIIC, regValues, length);
    if (rw_cnt < 0)
	    rt = rw_cnt;

lbl_done:
	/* terminate transfer */
	ls1x_i2c_send_stop(pIIC, RX8010_ADDRESS);

	return rt;
}

//-------------------------------------------------------------------------------------------------
// rx8010_read_reg()
// reads a rx8010 register (see Register defines)
//-------------------------------------------------------------------------------------------------

static inline int RX8010_read_reg(LS1x_I2C_bus_t *pIIC, int regNum, unsigned char *regVal)
{
    return RX8010_read_regs(pIIC, regNum, 1, regVal);
}

//-------------------------------------------------------------------------------------------------
// rx8010_write_regs()
// writes a specified number of rx8010 registers (see Register defines)
// See also rx8010_write_reg() to write a single register.
//-------------------------------------------------------------------------------------------------

static int RX8010_write_regs(LS1x_I2C_bus_t *pIIC, int regNum, unsigned char length, unsigned char *regVals)
{
    int rt, rw_cnt;
	unsigned char regAddr;

	/* start transfer */
	rt = ls1x_i2c_send_start(pIIC, RX8010_ADDRESS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls1x_i2c_ioctl(pIIC, IOCTL_SPI_I2C_SET_TFRMODE, (void *)RX8010_BAUDRATE);
	CHECK_DONE(rt);

	/* address device, FALSE = WRITE */
	rt = ls1x_i2c_send_addr(pIIC, RX8010_ADDRESS, false);
	CHECK_DONE(rt);

	/*
	 * first, 1st byte is REGISTER to be accessed.
	 */
	regAddr = (unsigned char)regNum;
	rw_cnt = ls1x_i2c_write_bytes(pIIC, &regAddr, 1);
	if (rw_cnt < 0)
		rt = rw_cnt;
	CHECK_DONE(rt);

    rw_cnt = ls1x_i2c_write_bytes(pIIC, regVals, length);
	if (rw_cnt < 0)
		rt = rw_cnt;
		
lbl_done:
	/* terminate transfer */
	ls1x_i2c_send_stop(pIIC, RX8010_ADDRESS);

	return rt;
}

//-------------------------------------------------------------------------------------------------
// rx8010_write_reg()
// writes a rx8010 register (see Register defines)
// See also rx8010_write_regs() to write multiple registers.
//-------------------------------------------------------------------------------------------------

static inline int RX8010_write_reg(LS1x_I2C_bus_t *pIIC, int regNum, unsigned char regVal)
{
    return RX8010_write_regs(pIIC, regNum, 1, &regVal);
}

/**************************************************************************************************
 * 驱动程序
 */
 
//-------------------------------------------------------------------------------------------------
// RX8010_initialize()
//-------------------------------------------------------------------------------------------------

STATIC_DRV int RX8010_initialize(void *bus, void *arg)
{
    LS1x_I2C_bus_t *pIIC = (LS1x_I2C_bus_t *)bus;
	int rt, need_reset = 0, need_clear = 0;
	unsigned char ctrl[3];
	
    if (bus == NULL)
        return -1;
        
	// set reserved register 0x17 with specified value of 0xD8
	rt = RX8010_write_reg(pIIC, 0x17, 0xD8);
	CHECK_DONE(rt);

	// set reserved register 0x30 with specified value of 0x00
	rt = RX8010_write_reg(pIIC, 0x30, 0x00);
	CHECK_DONE(rt);

	// set reserved register 0x31 with specified value of 0x08
	rt = RX8010_write_reg(pIIC, 0x31, 0x08);
	CHECK_DONE(rt);

	// set reserved register 0x32 with default value
	rt = RX8010_write_reg(pIIC, RX8010_REG_IRQ, 0x00);
	CHECK_DONE(rt);

	// get current extension, flag, control register values
	rt = RX8010_read_regs(pIIC, RX8010_REG_EXT, 3, ctrl);
	CHECK_DONE(rt);

	// check for VLF Flag (set at power-on)
	if ((ctrl[1] & RX8010_BIT_FLAG_VLF))
	{
		printk("RX8010 Frequency stop was detected, probably due to a supply voltage drop\r\n");
		need_reset = 1;
	}

	// check for Alarm Flag
	if (ctrl[1] & RX8010_BIT_FLAG_AF)
	{
		printk("RX8010 Alarm was detected\r\n");
		need_clear = 1;
	}

	// check for Periodic Timer Flag
	if (ctrl[1] & RX8010_BIT_FLAG_TF)
	{
		printk("RX8010 Periodic timer was detected\r\n");
		need_clear = 1;
	}

	// check for Update Timer Flag
	if (ctrl[1] & RX8010_BIT_FLAG_UF)
	{
		printk("RX8010 Update timer was detected\r\n");
		need_clear = 1;
	}

	// reset or clear needed?
	if (need_reset)
	{
		// clear 1d, 1e, 1f registers
		ctrl[0] = ctrl[1] = ctrl[2] = 0;
		rt = RX8010_write_regs(pIIC, RX8010_REG_EXT, 3, ctrl);
	    CHECK_DONE(rt);
	}
	else if (need_clear)
	{
		// clear flag register
		rt = RX8010_write_reg(pIIC, RX8010_REG_FLAG, 0x00);
	    CHECK_DONE(rt);
	}

	// set "test bit" and reserved bits of control register zero
	saved_rx8010_ctrl = (ctrl[2] & ~RX8010_BIT_CTRL_TEST);

lbl_done:

	return rt;
}

//-------------------------------------------------------------------------------------------------
// rx8010_read()
//   gets the current time from the rx8010 registers
//-------------------------------------------------------------------------------------------------

STATIC_DRV int RX8010_read(void *bus, void *buf, int size, void *arg)
{
    LS1x_I2C_bus_t *pIIC = (LS1x_I2C_bus_t *)bus;
    struct tm *t = (struct tm *)buf;    // should be struct tm *
	unsigned char date[7];
	int rt = 0;

    if ((bus == NULL) || (buf == NULL))
        return -1;
        
	rt = RX8010_read_regs(pIIC, RX8010_REG_SEC, 7, date);
	CHECK_DONE(rt);

	/* Note: need to subtract 0x10 for index as register offset starts at 0x10
     */
	t->tm_sec  = bcd2bin(date[RX8010_REG_SEC   - 0x10] & 0x7f);
	t->tm_min  = bcd2bin(date[RX8010_REG_MIN   - 0x10] & 0x7f);
	t->tm_hour = bcd2bin(date[RX8010_REG_HOUR  - 0x10] & 0x3f); /* only 24-hour clock */
	t->tm_mday = bcd2bin(date[RX8010_REG_MDAY  - 0x10] & 0x3f);
	t->tm_mon  = bcd2bin(date[RX8010_REG_MONTH - 0x10] & 0x1f); 
	t->tm_year = bcd2bin(date[RX8010_REG_YEAR  - 0x10]);
	t->tm_wday = bcd2bin(date[RX8010_REG_WDAY  - 0x10] & 0x7f);

    t->tm_mon -= 1;                 /* read out real world month */
	if (t->tm_year < 70)            /* year range: 1970~2069 */
		t->tm_year += 100;

	rt = sizeof(struct tm);

lbl_done:
    return rt;
}

//-------------------------------------------------------------------------------------------------
// rx8010_write()
//   Sets the current time in the rx8010 registers
//-------------------------------------------------------------------------------------------------

STATIC_DRV int RX8010_write(void *bus, void *buf, int size, void *arg)
{
    LS1x_I2C_bus_t *pIIC = (LS1x_I2C_bus_t *)bus;
    struct tm *t = (struct tm *)buf;    // should be struct tm *
	unsigned char date[7], ctrl;
	int rt = 0;

    if ((bus == NULL) || (buf == NULL))
        return -1;
        
	// set STOP bit before changing clock/calendar
	rt = RX8010_read_reg(pIIC, RX8010_REG_CTRL, &ctrl);
	CHECK_DONE(rt);
	saved_rx8010_ctrl = ctrl | RX8010_BIT_CTRL_STOP;
	rt = RX8010_write_reg(pIIC, RX8010_REG_CTRL, saved_rx8010_ctrl);
	CHECK_DONE(rt);

	// Note: need to subtract 0x10 for index as register offset starts at 0x10
	date[RX8010_REG_SEC   - 0x10] = bin2bcd(t->tm_sec);
	date[RX8010_REG_MIN   - 0x10] = bin2bcd(t->tm_min);
	date[RX8010_REG_HOUR  - 0x10] = bin2bcd(t->tm_hour);        /* only 24hr time */
	date[RX8010_REG_MDAY  - 0x10] = bin2bcd(t->tm_mday);
	if (t->tm_year < 1900)
	    date[RX8010_REG_MONTH - 0x10] = bin2bcd(t->tm_mon + 1);  /* write real world month */
    else
        date[RX8010_REG_MONTH - 0x10] = bin2bcd(t->tm_mon);
	date[RX8010_REG_YEAR  - 0x10] = bin2bcd(t->tm_year % 100);  /* only low 2 digits */
	date[RX8010_REG_WDAY  - 0x10] = bin2bcd(t->tm_wday);

	rt = RX8010_write_regs(pIIC, RX8010_REG_SEC, 7, date);

	// clear STOP bit after changing clock/calendar
	rt = RX8010_read_reg(pIIC, RX8010_REG_CTRL, &ctrl);
	CHECK_DONE(rt);
	saved_rx8010_ctrl = ctrl & ~RX8010_BIT_CTRL_STOP;
	rt = RX8010_write_reg(pIIC, RX8010_REG_CTRL, saved_rx8010_ctrl);
	CHECK_DONE(rt);

	rt = sizeof(struct tm);

lbl_done:
    return rt;
}

//-------------------------------------------------------------------------------------------------
// RX8010_ioctl()
//-------------------------------------------------------------------------------------------------

STATIC_DRV int RX8010_ioctl(void *bus, int cmd, void *arg)
{
    LS1x_I2C_bus_t *pIIC = (LS1x_I2C_bus_t *)bus;
	int tmp, rt = 0;

    if (bus == NULL)
        return -1;
        
    /**************************************************************************
     * TODO arg 参数需要定义成 struct
     */
     
	switch (cmd)
	{
		case IOCTL_RTC_REG_READ:      // arg should be 1 int*
#if 0
			tmp = *((int *)arg);
			if (tmp < RX8010_REG_SEC || tmp > RX8010_REG_IRQ)
				return -EFAULT;
			rt = RX8010_read_reg(pIIC, tmp, (unsigned char *)arg);
			if (!rt)
				return -EFAULT;
#endif
			break;

		case IOCTL_RTC_REG_WRITE:     // arg should be 2 int*
#if 0
			tmp = *((int *)arg);
			if (tmp < RX8010_REG_SEC || tmp > RX8010_REG_IRQ)
				return -EFAULT;
			rt = RX8010_write_reg(pIIC, tmp, (unsigned char)((unsigned int)arg));
#endif
			break;

		case IOCTL_RTC_VL_READ:       // arg should be 1 int*
			rt = RX8010_read_reg(pIIC, RX8010_REG_FLAG, (unsigned char *)&tmp);
			if (!rt)
			{
            	tmp = !!(tmp & RX8010_BIT_FLAG_VLF);
            	*((int *)arg) = tmp;
            }
			break;

		case IOCTL_RTC_VL_CLR:        // arg should be 1 int*
			rt = RX8010_read_reg(pIIC, RX8010_REG_FLAG, (unsigned char *)&tmp);
			if (!rt)
			{
				tmp &= ~RX8010_BIT_FLAG_VLF;
				rt = RX8010_write_reg(pIIC, RX8010_REG_FLAG, (unsigned char)tmp);
			}
			break;

		default:
			return -1;
	}

	return rt;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * RX8010 driver operators
 */
static driver_ops_t LS1x_RX8010_drv_ops =
{
    .init_entry  = RX8010_initialize,
    .open_entry  = NULL,
    .close_entry = NULL,
    .read_entry  = RX8010_read,
    .write_entry = RX8010_write,
    .ioctl_entry = RX8010_ioctl,
};
driver_ops_t *ls1x_rx8010_drv_ops = &LS1x_RX8010_drv_ops;
#endif

//-----------------------------------------------------------------------------
// user api
//-----------------------------------------------------------------------------

int get_rx8010_datetime(void *bus, struct tm *dt)
{
    int rt = RX8010_read(bus, (void *)dt, sizeof(struct tm), NULL);
    return rt == sizeof(struct tm) ? 0 : -1;
}

int set_rx8010_datetime(void *bus, struct tm *dt)
{
    int rt = RX8010_write(bus, (void *)dt, sizeof(struct tm), NULL);
    return rt == sizeof(struct tm) ? 0 : -1;
}

#endif


