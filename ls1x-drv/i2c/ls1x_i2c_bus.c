/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_i2c_bus.c
 *
 * this file contains the ls1x I2C driver
 *
 *  Created on: 2013-11-1
 *      Author: Bian
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "bsp.h"

#if defined(BSP_USE_I2C0) || defined(BSP_USE_I2C1) || defined(BSP_USE_I2C2)

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_irq.h"
#define LS1x_I2C0_BASE    LS1B_I2C0_BASE
#define LS1x_I2C1_BASE    LS1B_I2C1_BASE
#define LS1x_I2C2_BASE    LS1B_I2C2_BASE
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_irq.h"
#define LS1x_I2C0_BASE    LS1C_I2C0_BASE
#define LS1x_I2C1_BASE    LS1C_I2C1_BASE
#define LS1x_I2C2_BASE    LS1C_I2C2_BASE
#else
#error "No Loongson1x SoC defined."
#endif

#include "ls1x_io.h"
#include "drv_os_priority.h"

#include "ls1x_i2c_bus_hw.h"
#include "ls1x_i2c_bus.h"

/*
 * time-out, xxx how long is fit ?
 */
#define DEFAULT_TIMEOUT 10000

//-----------------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------------

#if defined(OS_RTTHREAD)
#define LOCK(p)    do { rt_mutex_take(p->i2c_mutex, RT_WAITING_FOREVER); } while (0);
#define UNLOCK(p)  do { rt_mutex_release(p->i2c_mutex); } while (0);
#elif defined(OS_UCOS)
#define LOCK(p)    do { unsigned char err; if (OSRunning == OS_TRUE) OSMutexPend(p->i2c_mutex, ~0, &err); } while (0);
#define UNLOCK(p)  do { if (OSRunning == OS_TRUE) OSMutexPost(p->i2c_mutex); } while (0);
#elif defined(OS_FREERTOS)
#define LOCK(p)    do { xSemaphoreTake(p->i2c_mutex, portMAX_DELAY); } while (0);
#define UNLOCK(p)  do { xSemaphoreGive(p->i2c_mutex); } while (0);
#else // defined(OS_NONE)
#define LOCK(p)
#define UNLOCK(p)
#endif

//-----------------------------------------------------------------------------

/******************************************************************************
 * I2C hardware
 ******************************************************************************/

static int LS1x_I2C_wait_done(LS1x_I2C_bus_t *pIIC)
{
	register unsigned int tmo = 0;

	/* wait for i2c to terminate */
	while (pIIC->hwI2C->cmd_sr.sr & i2c_sr_xferflag)
	{
		if (tmo++ > DEFAULT_TIMEOUT)
		{
			printk("I2C tmo!\r\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

static int LS1x_I2C_set_baudrate(LS1x_I2C_bus_t *pIIC, unsigned int baudrate)
{
	unsigned int fdr_val;
	unsigned char ctrl;

	/* set frequency divider, default to 100kHz */
	fdr_val = baudrate < pIIC->baudrate ? baudrate : pIIC->baudrate;
	fdr_val = fdr_val > 0 ? fdr_val : 100000;
	fdr_val = pIIC->base_frq / (5 * fdr_val) - 1;

	/* write the clock div register */
	ctrl  = pIIC->hwI2C->ctrl;
	ctrl &= ~(i2c_ctrl_en | i2c_ctrl_ien);
	pIIC->hwI2C->ctrl   = ctrl;
	pIIC->hwI2C->prerlo = fdr_val & 0xFF;
	pIIC->hwI2C->prerhi = (fdr_val >> 8) & 0xFF;

	/* set control register to module enable */
	pIIC->hwI2C->cmd_sr.cmd = 0x00;
	ctrl |= i2c_ctrl_en;
	pIIC->hwI2C->ctrl = ctrl;

	return 0;
}

/******************************************************************************
 * I2C driver Implement
 ******************************************************************************/

STATIC_DRV int LS1x_I2C_initialize(void *bus)
{
	LS1x_I2C_bus_t *pIIC = (LS1x_I2C_bus_t *)bus;

    if (bus == NULL)
        return -1;
        
    if (pIIC->initialized)
        return 0;
        
#if BSP_USE_OS
  #if defined(OS_RTTHREAD)
    pIIC->i2c_mutex = rt_mutex_create(pIIC->dev_name, RT_IPC_FLAG_FIFO);
  #elif defined(OS_UCOS)
    unsigned char err;
    if ((unsigned)pIIC->hwI2C == LS1x_I2C0_BASE)
        pIIC->i2c_mutex = OSMutexCreate(I2C0_MUTEX_PRIO, &err);
    else if ((unsigned)pIIC->hwI2C == LS1x_I2C1_BASE)
        pIIC->i2c_mutex = OSMutexCreate(I2C1_MUTEX_PRIO, &err);
    else if ((unsigned)pIIC->hwI2C == LS1x_I2C2_BASE)
        pIIC->i2c_mutex = OSMutexCreate(I2C2_MUTEX_PRIO, &err);
    else
        return -1;
  #elif defined(OS_FREERTOS)
    pIIC->i2c_mutex = xSemaphoreCreateMutex();  /* 创建设备锁 */
  #endif

    if (pIIC->i2c_mutex == NULL)
    	return -1;
#endif // #if BSP_USE_OS

	/* initialize with bsp bus frequency */
	pIIC->base_frq = LS1x_BUS_FREQUENCY(CPU_XTAL_FREQUENCY);
#if defined(LS1C)
	pIIC->base_frq >>= 1;
#endif

	/* initialize the baudrate */
	LS1x_I2C_set_baudrate(pIIC, pIIC->baudrate);

    pIIC->initialized = 1;

    printk("I2C%i controller initialized.\r\n", \
           ((unsigned)pIIC->hwI2C == LS1x_I2C0_BASE) ? 0 : \
           ((unsigned)pIIC->hwI2C == LS1x_I2C1_BASE) ? 1 : 2);

	return 0;
}

STATIC_DRV int LS1x_I2C_send_start(void *bus, unsigned int Addr)
{
	unsigned int tmo = 0;
	LS1x_I2C_bus_t *pIIC = (LS1x_I2C_bus_t *)bus;
	
    if (bus == NULL)
        return -1;
        
	LOCK(pIIC);
	
	/* wait for bus idle */
	while (pIIC->hwI2C->cmd_sr.sr & i2c_sr_busy)
	{
		if (tmo++ > DEFAULT_TIMEOUT)
			return -ETIMEDOUT;
		delay_us(1);
	}

	/* "start" command to handle the bus */
	pIIC->hwI2C->cmd_sr.cmd = i2c_cmd_start;

	return 0;
}

STATIC_DRV int LS1x_I2C_send_stop(void *bus, unsigned int Addr)
{
	LS1x_I2C_bus_t *pIIC = (LS1x_I2C_bus_t *)bus;
	
    if (bus == NULL)
        return -1;
        
	/* "stop" command to release the bus */
	pIIC->hwI2C->cmd_sr.cmd = i2c_cmd_stop;

	delay_us(1);

    UNLOCK(pIIC);
	return 0;
}

STATIC_DRV int LS1x_I2C_send_addr(void *bus, unsigned int Addr, int rw)
{
	int rt;
	LS1x_I2C_bus_t *pIIC = (LS1x_I2C_bus_t *)bus;
	
    if (bus == NULL)
        return -1;
        
	/* set then slave address, 1: read, 0: write. */
	pIIC->hwI2C->data.txreg = ((unsigned char)Addr << 1) | ((rw) ? 1 : 0);

	/* "start" "write" command to send addr */
	pIIC->hwI2C->cmd_sr.cmd = i2c_cmd_write | i2c_cmd_start;

	/* wait for successful transfer */
	rt = LS1x_I2C_wait_done(pIIC);

	/* slave is no ack */
	if (0 == rt)
	{
		if (pIIC->hwI2C->cmd_sr.sr & i2c_sr_rxnack)
		{
			LS1x_I2C_send_stop(pIIC, 0);
			rt = -2;
		}
	}
	else
	{
		LS1x_I2C_send_stop(pIIC, 0);
	}

	return rt;
}

STATIC_DRV int LS1x_I2C_read_bytes(void *bus, unsigned char *buf, int len)
{
	int rt;
	unsigned char *p = buf;
	LS1x_I2C_bus_t *pIIC = (LS1x_I2C_bus_t *)bus;
	
    if ((bus == NULL) || (buf == NULL))
        return -1;
        
	while (len-- > 0)
	{
		/* Read command */
		if (len == 0)
		{
			/* last byte is not acknowledged */
			pIIC->hwI2C->cmd_sr.cmd = i2c_cmd_read | i2c_cmd_nack;
		}
		else
		{
			pIIC->hwI2C->cmd_sr.cmd = i2c_cmd_read | i2c_cmd_ack;
		}

		/* wait until end of transfer */
		rt = LS1x_I2C_wait_done(pIIC);
		if (0 != rt)
		{
			LS1x_I2C_send_stop(pIIC, 0);
			return -rt;
		}

		/* Read data */
		*p++ = pIIC->hwI2C->data.rxreg;
	}

	return p - buf;
}

STATIC_DRV int LS1x_I2C_write_bytes(void *bus, unsigned char *buf, int len)
{
	int rt;
	unsigned char *p = buf;
	LS1x_I2C_bus_t *pIIC = (LS1x_I2C_bus_t *)bus;
	
    if ((bus == NULL) || (buf == NULL))
        return -1;
        
	while (len-- > 0)
	{
		/* Write data */
		pIIC->hwI2C->data.txreg = *p++;

		/* Write command */
		pIIC->hwI2C->cmd_sr.cmd = i2c_cmd_write; // XXX | i2c_cmd_ack

		/* Wait until end of transfer */
		rt = LS1x_I2C_wait_done(pIIC);
		if (0 != rt)
		{
			LS1x_I2C_send_stop(pIIC, 0);
			return -rt;
		}

		/* Slave is no ack */
		if (pIIC->hwI2C->cmd_sr.sr & i2c_sr_rxnack)
		{
			LS1x_I2C_send_stop(pIIC, 0);
			return p - buf;
		}
	}

	return p - buf;
}

STATIC_DRV int LS1x_I2C_ioctl(void *bus, int cmd, void *arg)
{
	int rt = -1;
	LS1x_I2C_bus_t *pIIC = (LS1x_I2C_bus_t *)bus;
	
    if (bus == NULL)
        return -1;
        
	switch (cmd)
	{
		case IOCTL_SPI_I2C_SET_TFRMODE:
			rt = -LS1x_I2C_set_baudrate(pIIC, (unsigned int)arg);
			break;

		default:
			rt = -1;
			break;
	}

	return rt;
}

//-----------------------------------------------------------------------------
// IIC bus driver ops
//-----------------------------------------------------------------------------

#if (PACK_DRV_OPS)
static libi2c_ops_t LS1x_I2C_ops =
{
    .init        = LS1x_I2C_initialize,
    .send_start  = LS1x_I2C_send_start,
    .send_stop   = LS1x_I2C_send_stop,
    .send_addr   = LS1x_I2C_send_addr,
    .read_bytes  = LS1x_I2C_read_bytes,
    .write_bytes = LS1x_I2C_write_bytes,
    .ioctl       = LS1x_I2C_ioctl,
};
#endif

//-----------------------------------------------------------------------------
// IIC bus device table
//-----------------------------------------------------------------------------

#ifdef BSP_USE_I2C0
static LS1x_I2C_bus_t ls1x_I2C0 =
{
	.hwI2C       = (struct LS1x_I2C_regs *)LS1x_I2C0_BASE,  /* pointer to HW registers */
	.base_frq    = 0,    		                            /* input frq for baud rate divider */
	.baudrate    = 100000,                                  /* work baud rate */
	.dummy_char  = 0,                                       /* dummy char */
	.i2c_mutex   = 0,                                       /* thread-safe */
    .initialized = 0,
    .dev_name    = "i2c0",
#if (PACK_DRV_OPS)
    .ops         = &LS1x_I2C_ops,
#endif
};
LS1x_I2C_bus_t *busI2C0 = &ls1x_I2C0;
#endif

#ifdef BSP_USE_I2C1
static LS1x_I2C_bus_t ls1x_I2C1 =
{
	.hwI2C       = (struct LS1x_I2C_regs *)LS1x_I2C1_BASE,  /* pointer to HW registers */
	.base_frq    = 0,    		                            /* input frq for baud rate divider */
	.baudrate    = 100000,                                  /* work baud rate */
	.dummy_char  = 0,                                       /* dummy char */
	.i2c_mutex   = 0,                                       /* thread-safe */
    .initialized = 0,
    .dev_name    = "i2c1",
#if (PACK_DRV_OPS)
    .ops         = &LS1x_I2C_ops,
#endif
};
LS1x_I2C_bus_t *busI2C1 = &ls1x_I2C1;
#endif

#ifdef BSP_USE_I2C2
static LS1x_I2C_bus_t ls1x_I2C2 =
{
	.hwI2C       = (struct LS1x_I2C_regs *)LS1x_I2C2_BASE,  /* pointer to HW registers */
	.base_frq    = 0,    		                            /* input frq for baud rate divider */
	.baudrate    = 100000,                                  /* work baud rate */
	.dummy_char  = 0,                                       /* dummy char */
	.i2c_mutex   = 0,                                       /* thread-safe */
    .initialized = 0,
    .dev_name    = "i2c2",
#if (PACK_DRV_OPS)
    .ops         = &LS1x_I2C_ops,
#endif
};
LS1x_I2C_bus_t *busI2C2 = &ls1x_I2C2;
#endif

#endif


