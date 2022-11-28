/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * gp7101.c
 *
 * created: 2020/9/11
 *  author: Bian
 */

#include "bsp.h"

#ifdef GP7101_DRV

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

#include "i2c/gp7101.h"

//-----------------------------------------------------------------------------

#define GP7101_ADDRESS          0x58

#define GP7101_BAUDRATE         1000000

//-----------------------------------------------------------------------------

#define CHECK_DONE(rt) \
	do {               \
        if (0 != rt)   \
            goto lbl_done; \
    } while (0);

//-----------------------------------------------------------------------------
// driver routine
//-----------------------------------------------------------------------------

#define WR_8BIT_CMD     0x03

static int GP7101_write(void *bus, void *buf, int size, void *arg)
{
	int rt = 0;
	unsigned char data[2];
    LS1x_I2C_bus_t *pIIC = (LS1x_I2C_bus_t *)bus;

    if ((bus == NULL) || (buf == NULL))
        return -1;
        
	data[0] = WR_8BIT_CMD;
	data[1] = *((unsigned char *)buf);

	/* start transfer */
	rt = ls1x_i2c_send_start(pIIC, GP7101_ADDRESS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls1x_i2c_ioctl(pIIC, IOCTL_SPI_I2C_SET_TFRMODE, (void *)GP7101_BAUDRATE);
	CHECK_DONE(rt);

	/* address device, FALSE(0) for write */
	rt = ls1x_i2c_send_addr(pIIC, GP7101_ADDRESS, false);
	CHECK_DONE(rt);

	rt = ls1x_i2c_write_bytes(pIIC, data, 2);

lbl_done:
	/* terminate transfer */
	ls1x_i2c_send_stop(pIIC, GP7101_ADDRESS);

	return rt;
}

//-----------------------------------------------------------------------------
// user api
//-----------------------------------------------------------------------------

/*
 * brightness: 0~100
 */
int set_lcd_brightness(int brightpercent)
{
    unsigned char brightness = (unsigned char)(brightpercent * 255 / 100);
    
    return GP7101_write(busI2C0, (void *)&brightness, 1, NULL);
}

#endif


