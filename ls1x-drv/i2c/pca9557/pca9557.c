/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * pca9557.c
 *
 * created: 2020/7/17
 *  author: Bian
 */

#include "bsp.h"

#ifdef PCA9557_DRV

#include <stdio.h>
#include <errno.h>

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

#include "i2c/pca9557.h"

//-----------------------------------------------------------------------------

#define PCA9557_ADDRESS         0x1C                // pca9557 地址 0b0011 100

#define PCA9557_BAUDRATE        100000              // pca9557 通信速率

//-----------------------------------------------------------------------------

#define PCA9557_IN_REG          0x00                // 输入寄存器
#define PCA9557_OUT_REG         0x01                // 输出寄存器
#define PCA9557_POL_REG         0x02                // 极性寄存器
#define PCA9557_CFG_REG         0x03                // 配置寄存器

/*
 * 输入输出引脚
 */
#if defined(LS1C)
#define PCA9557_CFG_PINS        0b11100100          // pca9557 配置寄存器
#define USB_OTG_VBUS            (1<<1)              // PIN[ 7] = IO[1], out : IO_OTG_1
#define USB_OTG_ID              (1<<2)              // PIN[ 9] = IO[2], in  : IO_OTG_2
#define CAMREA_RESET            (1<<3)              // PIN[10] = IO[3], out : reset
#define CAMERA_PWND             (1<<4)              // PIN[11] = IO[4], out : power down
#define RS8010_IRQ1             (1<<5)              // in
#define RS8010_IRQ2             (1<<6)              // in
#define TOUCH_DOWN              (1<<7)              // in
#elif defined(LS1B)
#define PCA9557_CFG_PINS        0b01110000          // pca9557 配置寄存器
#define RS8010_IRQ1             (1<<4)              // in
#define RS8010_IRQ2             (1<<5)              // in
#define ADS1015_READY           (1<<6)              // in
#endif

#define PCA9557_IN_MASK         PCA9557_CFG_PINS
#define PCA9557_OUT_MASK        (~PCA9557_CFG_PINS)

//-----------------------------------------------------------------------------

#define CHECK_DONE(rt) \
	do {               \
        if (0 != rt)   \
            goto lbl_done; \
    } while (0);

//-----------------------------------------------------------------------------

static unsigned char saved_out_reg = 0;      // 输出寄存器缓存值

/******************************************************************************
 * local function
 ******************************************************************************/
 
static int PCA9557_write_register(LS1x_I2C_bus_t *pIIC, unsigned char reg, unsigned char val)
{
	int rt = 0, rw_cnt = 0;
	unsigned char cmd[2];

	/* start transfer */
	rt = ls1x_i2c_send_start(pIIC, PCA9557_ADDRESS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls1x_i2c_ioctl(pIIC, IOCTL_SPI_I2C_SET_TFRMODE, (void *)PCA9557_BAUDRATE);
	CHECK_DONE(rt);

	/* address device */
	rt = ls1x_i2c_send_addr(pIIC, PCA9557_ADDRESS, false);
	CHECK_DONE(rt);

	//-----------------------------------------------------
	// access register
	//-----------------------------------------------------
    cmd[0] = reg;
    cmd[1] = val;
	rw_cnt = ls1x_i2c_write_bytes(pIIC, cmd, 2);
	if (rw_cnt < 0)
		rt = rw_cnt;
	CHECK_DONE(rt);

	rt = rw_cnt;

lbl_done:

	/* terminate transfer */
	ls1x_i2c_send_stop(pIIC, PCA9557_ADDRESS);

	return rt;
}

static int PCA9557_read_register(LS1x_I2C_bus_t *pIIC, unsigned char reg, unsigned char *val)
{
	int rt = 0, rw_cnt = 0;

	/* start transfer */
	rt = ls1x_i2c_send_start(pIIC, PCA9557_ADDRESS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls1x_i2c_ioctl(pIIC, IOCTL_SPI_I2C_SET_TFRMODE, (void *)PCA9557_BAUDRATE);
	CHECK_DONE(rt);

	/* address device, FALSE(0) for WRTIE */
	rt = ls1x_i2c_send_addr(pIIC, PCA9557_ADDRESS, false);
	CHECK_DONE(rt);

	//-----------------------------------------------------
	// access register
	//-----------------------------------------------------
	rw_cnt = ls1x_i2c_write_bytes(pIIC, &reg, 1);
	if (rw_cnt < 0)
		rt = rw_cnt;
	CHECK_DONE(rt);

	//-----------------------------------------------------
	// restart - address device, TRUE(1) for READ
	//-----------------------------------------------------
	rt = ls1x_i2c_send_addr(pIIC, PCA9557_ADDRESS, true);
	CHECK_DONE(rt);

	//-----------------------------------------------------
	// read out data
	//-----------------------------------------------------
	rw_cnt = ls1x_i2c_read_bytes(pIIC, val, 1);
	if (rw_cnt < 0)
		rt = rw_cnt;
	CHECK_DONE(rt);

	rt = rw_cnt;

lbl_done:

	/* terminate transfer */
	ls1x_i2c_send_stop(pIIC, PCA9557_ADDRESS);

	return rt;
}

/******************************************************************************
 * driver implement
 ******************************************************************************/

STATIC_DRV int PCA9557_init(void *bus, void *arg)
{
	int rt = 0;
	
    if (bus == NULL)
        return -1;
        
    /* polarity register */
    rt = PCA9557_write_register((LS1x_I2C_bus_t *)bus, PCA9557_POL_REG, 0);
    if (rt)
    {
        /* config register */
        rt = PCA9557_write_register((LS1x_I2C_bus_t *)bus,
                                    PCA9557_CFG_REG,
                                    PCA9557_CFG_PINS);
    }

    return rt;
}

STATIC_DRV int PCA9557_read(void *bus, void *buf, int size, void *arg)
{
    if ((bus == NULL) || (buf == NULL))
        return -1;
        
    /* input register */
    return PCA9557_read_register((LS1x_I2C_bus_t *)bus,
                                 PCA9557_IN_REG,
                                 (unsigned char *)buf);
}

STATIC_DRV int PCA9557_write(void *bus, void *buf, int size, void *arg)
{
    if ((bus == NULL) || (buf == NULL))
        return -1;
        
    if (saved_out_reg == *((unsigned char *)buf))   // NO change
        return 0;

    saved_out_reg = *((unsigned char *)buf);        // Save It
    
    /* output register */
    return PCA9557_write_register((LS1x_I2C_bus_t *)bus,
                                  PCA9557_OUT_REG,
                                  *((unsigned char *)buf));
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * PCA9557 driver operators
 */
static driver_ops_t LS1x_PCA9557_drv_ops =
{
    .init_entry  = PCA9557_init,
    .open_entry  = NULL,
    .close_entry = NULL,
    .read_entry  = PCA9557_read,
    .write_entry = PCA9557_write,
    .ioctl_entry = NULL,
};
driver_ops_t *ls1x_pca9557_drv_ops = &LS1x_PCA9557_drv_ops;
#endif

//-----------------------------------------------------------------------------
// user api
//-----------------------------------------------------------------------------

/******************************************************************************
 * get
 */

#if defined(LS1B)

bool get_ads1015_ready(void *bus)
{
    unsigned char tmp;
    if (PCA9557_read(bus, (void *)&tmp, 1, NULL) > 0)
        return (tmp & ADS1015_READY) ? 1 : 0;
    return 0;
}

#endif

bool get_rx8010_irq1(void *bus)
{
    unsigned char tmp;
    if (PCA9557_read(bus, (void *)&tmp, 1, NULL) > 0)
        return (tmp & RS8010_IRQ1) ? 1 : 0;
    return 0;
}

bool get_rx8010_irq2(void *bus)
{
    unsigned char tmp;
    if (PCA9557_read(bus, (void *)&tmp, 1, NULL) > 0)
        return (tmp & RS8010_IRQ2) ? 1 : 0;
    return 0;
}

#if defined(LS1C)

bool get_usb_otg_id(void *bus)
{
    unsigned char tmp;
    if (PCA9557_read(bus, (void *)&tmp, 1, NULL) > 0)
        return (tmp & USB_OTG_ID) ? 1 : 0;
    return 0;
}

bool get_touch_down(void *bus)
{
    unsigned char tmp;
    if (PCA9557_read(bus, (void *)&tmp, 1, NULL) > 0)
        return (tmp & TOUCH_DOWN) ? 0 : 1;      /* XPT2046's PENIRQ is negative... */
    return 0;
}

#endif

/******************************************************************************
 * set
 */

#if defined(LS1C)

bool set_usb_otg_vbus(void *bus, bool en)
{
    unsigned char tmp;
    tmp  = en ? (saved_out_reg | USB_OTG_VBUS) : (saved_out_reg & ~USB_OTG_VBUS);
    tmp &= PCA9557_OUT_MASK;
    if (PCA9557_write(bus, (void *)&tmp, 1, NULL) > 0)
        return 1;
    return 0;
}

bool set_camera_reset(void *bus, bool en)
{
    unsigned char tmp;
    tmp  = en ? (saved_out_reg | CAMREA_RESET) : (saved_out_reg & ~CAMREA_RESET);
    tmp &= PCA9557_OUT_MASK;
    if (PCA9557_write(bus, (void *)&tmp, 1, NULL) > 0)
        return 1;
    return 0;
}

bool set_camera_powerdown(void *bus, bool en)
{
    unsigned char tmp;
    tmp  = en ? (saved_out_reg | CAMERA_PWND) : (saved_out_reg & ~CAMERA_PWND);
    tmp &= PCA9557_OUT_MASK;
    if (PCA9557_write(bus, (void *)&tmp, 1, NULL) > 0)
        return 1;
    return 0;
}

#endif

#endif // #ifdef PCA9557_DRV



