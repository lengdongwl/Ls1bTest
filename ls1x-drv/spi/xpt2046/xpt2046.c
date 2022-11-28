/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * xpt2046.c
 *
 *  communicate with spi0 at cs1; pending-irq with UART5_RX/GPIO37.
 *
 *  Created on: 2014-8-30
 *      Author: Bian
 */

/*
 * MENTION：XPT2046's PENIRQ is negative
 */

#include "bsp.h"

#ifdef XPT2046_DRV

#include <stdio.h>
#include <errno.h>

#include "../ls1x_spi_bus_hw.h"
#include "ls1x_io.h"
#include "ls1x_spi_bus.h"

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_irq.h"
#include "ls1b_gpio.h"
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_irq.h"
#include "ls1c_gpio.h"
#else
#error "No Loongson1x SoC defined."
#endif

#if defined(OS_RTTHREAD)
#include "rtthread.h"
#elif defined(OS_UCOS)
#include "os.h"
#elif defined(OS_FREERTOS)
#include "FreeRTOS.h"
#include "event_groups.h"
#endif

#include "spi/xpt2046.h"
#include "drv_os_priority.h"

//-----------------------------------------------------------------------------

#define CHECK_DONE(rt) \
	do {               \
        if (0 != rt)   \
            goto lbl_done; \
    } while (0);

//-----------------------------------------------------------------------------
// device
//-----------------------------------------------------------------------------

#define XPT2046_CS      1   // xpt2046 片选

//-----------------------------------------------------------------------------

#define XPT2046_12BIT	1

#if XPT2046_12BIT > 0
/**************************//*    S A2-0 MODE SER/DFR PD1-0 *********************/
#define CMD_READ_X	0xD0	/* 0B 1  101   0     0      00	即用差分方式读X坐标	*/
#define CMD_READ_Y	0x90	/* 0B 1  001   0     0      00	即用差分方式读Y坐标	*/
#define CMD_READ_Z1	0xB0	/* 0B 1  011   0     0      00	即用差分方式读Z1坐标	*/
#define CMD_READ_Z2	0xC0	/* 0B 1  100   0     0      00	即用差分方式读Z2坐标	*/
#else
#define CMD_READ_X	0xD8	/* 0B 1  101   1     0      00	即用差分方式读X坐标	*/
#define CMD_READ_Y	0x98	/* 0B 1  001   1     0      00	即用差分方式读Y坐标	*/
#define CMD_READ_Z1	0xB8	/* 0B 1  011   1     0      00	即用差分方式读Z1坐标	*/
#define CMD_READ_Z2	0xC8	/* 0B 1  100   1     0      00	即用差分方式读Z2坐标	*/
#endif

/*
 * x/y 坐标交换
 */
#define XPT2046_SWAP_XY		0
#define XPT2046_READ_Z		1		/* whether read z value */

/**********************************************************
 * gpio37 interrupt handle
 **********************************************************/

#if (XPT2046_USE_GPIO_INT)
static void XPT2046_touch_irq_handler(int vector, void *arg)
{
    int click_down = gpio_read(XPT2046_USE_GPIO_NUM) ? 0 : 1;
    
    /* send the touch-screen CLICK_DOWN message
     */
    if (click_down)
    {
        /* disable gpio interrupt */
        ls1x_disable_gpio_interrupt(XPT2046_USE_GPIO_NUM);
    
#if defined(OS_RTTHREAD)

        extern rt_event_t touch_event;          // defined in "touch_utils.c"
        rt_event_send(touch_event, TOUCH_CLICK_EVENT);

#elif defined(OS_UCOS)

        extern OS_FLAG_GRP *touch_event;
        unsigned char err;
        OSFlagPost(touch_event,
                   (OS_FLAGS)TOUCH_CLICK_EVENT,
                   OS_FLAG_SET,
                   &err);
        
#elif defined(OS_FREERTOS)

        extern EventGroupHandle_t touch_event;
        BaseType_t xResult, xHigherPriorityTaskWoken = pdFALSE;
		xResult = xEventGroupSetBitsFromISR(touch_event,
                                            TOUCH_CLICK_EVENT,
                                            &xHigherPriorityTaskWoken);
        if (xResult != pdPASS)  /* Was the message posted successfully? */
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }

#endif

        /* enable gpio interrupt */
        ls1x_enable_gpio_interrupt(XPT2046_USE_GPIO_NUM);
    }
}
#endif

/******************************************************************************
 * touchscreen driver implement
 ******************************************************************************/

STATIC_DRV int XPT2046_initialize(void *bus, void *arg)
{
#if defined(LS1B)
    /*
     * 1B200开发板, 触摸中断信号通过 GPIO54(UART2_RX) 查询读取.
     */
    gpio_enable(XPT2046_USE_GPIO_NUM, DIR_IN);

  #if (XPT2046_USE_GPIO_INT)
    /* disable gpio interrupt */
    ls1x_disable_gpio_interrupt(XPT2046_USE_GPIO_NUM);

    /*
     * install interrupt handler
     */
    ls1x_install_gpio_isr(XPT2046_USE_GPIO_NUM,
                          INT_TRIG_EDGE_DOWN, 
                          XPT2046_touch_irq_handler,
                          NULL);

    /* enable gpio interrupt */
    ls1x_enable_gpio_interrupt(XPT2046_USE_GPIO_NUM);

  #endif

#elif defined(LS1C)
    /*
     * 1C300B开发板, 触摸中断信号通过 pca9557 查询读取.
     */
#endif

	return 0;
}

static int XPT2046_do_cmd(LS1x_SPI_bus_t *pSPI, unsigned char cmd, unsigned short *val)
{
	int rt = 0, ret_cnt;
	unsigned char buf[2];

	/*
	 * send command -> delay 6us -> fetch read data
	 */
	buf[0] = cmd;
	ret_cnt = ls1x_spi_write_bytes(pSPI, buf, 1);
	if (ret_cnt < 0)
		rt = ret_cnt;
	CHECK_DONE(rt);

	/* wait for convert done, max 150k */
	delay_us(12);

	ret_cnt = ls1x_spi_read_bytes(pSPI, buf, 2);
	if (ret_cnt < 0)
		rt = ret_cnt;
	CHECK_DONE(rt);

	/*
	 * first 1 clock period is BUSY singal, so drop first bit.
	 */
#if XPT2046_12BIT > 0
	*val = (unsigned short)(buf[0] << 5) | (buf[1] >> 3);
	*val &= 0xFFF;
#else
	*val = (unsigned short)(buf[0] << 1) | (buf[1] >> 7);
	*val &= 0xFF;
#endif

lbl_done:
	return rt;
}

STATIC_DRV int XPT2046_read(void *bus, void *buf, int size, void *arg)
{
	int rt = 0, readed;
    LS1x_SPI_bus_t *pSPI = (LS1x_SPI_bus_t *)bus;
	ts_message_t *points = (ts_message_t *)buf;     //\\ 注意类型转换 
    unsigned int ptcount = size / sizeof(ts_message_t);
	unsigned short val;

    if ((bus == NULL) || (buf == NULL))
        return -1;
    
	LS1x_SPI_mode_t tfr_mode =
	{
		baudrate:      2000000,			/* maximum bits per second                   */
		bits_per_char: 8,				/* how many bits per byte/word/longword?     */
		lsb_first:     false,			/* FALSE: send MSB first                     */
		clock_pha:	   true,			/* clock phase    - spi mode 				 */
		clock_pol:	   true,			/* clock polarity - spi mode 				 */
		clock_inv:     true,			/* TRUE:  inverted clock (low active)		 */
		clock_phs:     false,			/* FALSE: clock starts in middle of data tfr */
	};

	if (points == NULL)
		return -EADDRNOTAVAIL;

	if (ptcount == 0)
		return 0;

	/* start transfer */
	rt = ls1x_spi_send_start(pSPI, XPT2046_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls1x_spi_ioctl(pSPI, IOCTL_SPI_I2C_SET_TFRMODE, &tfr_mode);
	CHECK_DONE(rt);

	/* address device */
	rt = ls1x_spi_send_addr(pSPI, XPT2046_CS, true);
	CHECK_DONE(rt);

	readed = 0;
	while (readed < ptcount)
	{
		/* read x */
		rt = XPT2046_do_cmd(pSPI, CMD_READ_X, &val);
		CHECK_DONE(rt);
    #if (XPT2046_SWAP_XY)
		points[readed].y = val;
    #else
		points[readed].x = val;
    #endif

		/* read y */
		rt = XPT2046_do_cmd(pSPI, CMD_READ_Y, &val);
		CHECK_DONE(rt);
    #if (XPT2046_SWAP_XY)
		points[readed].x = val;
    #else
		points[readed].y = val;
    #endif

    #if XPT2046_READ_Z > 0
		/* read z1 */
		rt = XPT2046_do_cmd(pSPI, CMD_READ_Z1, &val);
		CHECK_DONE(rt);
		points[readed].z = val;
    #else
		points[readed].z = 1;
    #endif
		readed++;
	}

	/* return the readed count */
	rt = ptcount * sizeof(ts_message_t);

lbl_done:
	/* terminate transfer */
	ls1x_spi_send_stop(pSPI, XPT2046_CS);

	return rt;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * SPI0-XPT2046 driver operators
 */
static driver_ops_t LS1x_XPT2046_drv_ops =
{
    .init_entry  = XPT2046_initialize,
    .open_entry  = NULL,
    .close_entry = NULL,
    .read_entry  = XPT2046_read,
    .write_entry = NULL,
    .ioctl_entry = NULL,
};
driver_ops_t *ls1x_xpt2046_drv_ops = &LS1x_XPT2046_drv_ops;
#endif

#endif

