/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_spi_bus.c
 *
 * this file contains the ls1x SPI driver
 *
 *  Created on: 2013-11-1
 *      Author: Bian
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "bsp.h"

#if defined(BSP_USE_SPI0) || defined(BSP_USE_SPI1)

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_irq.h"
#define LS1x_SPI0_IRQ       LS1B_SPI0_IRQ
#define LS1x_SPI1_IRQ       LS1B_SPI1_IRQ
#define LS1x_INTC0_BASE     LS1B_INTC0_BASE
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_irq.h"
#define LS1x_SPI0_IRQ       LS1C_SPI0_IRQ
#define LS1x_SPI1_IRQ       LS1C_SPI1_IRQ
#define LS1x_INTC0_BASE     LS1C_INTC0_BASE
#else
#error "No Loongson1x SoC defined."
#endif

#include "ls1x_io.h"
#include "drv_os_priority.h"

#include "ls1x_spi_bus_hw.h"
#include "ls1x_spi_bus.h"

/*
 * time-out, xxx how long is fit ?
 */
#define DEFAULT_TIMEOUT 	1000

//-----------------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------------

#if defined(OS_RTTHREAD)
#define LOCK(p)    do { rt_mutex_take(p->spi_mutex, RT_WAITING_FOREVER); } while (0);
#define UNLOCK(p)  do { rt_mutex_release(p->spi_mutex); } while (0);
#elif defined(OS_UCOS)
#define LOCK(p)    do { unsigned char err; if (OSRunning == OS_TRUE) OSMutexPend(p->spi_mutex, ~0, &err); } while (0);
#define UNLOCK(p)  do { if (OSRunning == OS_TRUE) OSMutexPost(p->spi_mutex); } while (0);
#elif defined(OS_FREERTOS)
#define LOCK(p)    do { xSemaphoreTake(p->spi_mutex, portMAX_DELAY); } while (0);
#define UNLOCK(p)  do { xSemaphoreGive(p->spi_mutex); } while (0);
#else // defined(OS_NONE)
#define LOCK(p)    
#define UNLOCK(p)  
#endif

/******************************************************************************
 * SPI hardware
 ******************************************************************************/

static int LS1x_SPI_baud_to_mode(unsigned int baudrate,
                                 unsigned int base_freq,
					             struct LS1x_SPI_clkdiv *clkdiv)
{
	unsigned int divider;
	unsigned char bit;

	/* determine clock divider
	 */
	divider= base_freq / baudrate;

	if (divider < 2)
		divider = 2;

	if (divider > 4096)
		divider = 4096;

	bit = fls(divider) - 1;

	switch (2 << bit)
	{
		case 16:
			divider = 2;
			break;
		case 32:
			divider = 3;
			break;
		case 8:
			divider = 4;
			break;
		default:
			divider = bit - 1;
			break;
	}

	clkdiv->spr = divider & spi_ctrl_spr_mask;
	clkdiv->spre = (divider >> 2) & spi_er_spre_mask;

	return 0;
}

static int LS1x_SPI_wait_tx_done(LS1x_SPI_bus_t *pSPI)
{
	register unsigned int tmo = 0;

	/*
	 * Wait for SPI to terminate
	 */
	while ((pSPI->hwSPI->sr & spi_sr_iflag) == 0)
	{
		if (tmo++ > DEFAULT_TIMEOUT)
		{
			printk("SPI tmo!\r\n");
			return -ETIMEDOUT;
		}
	}

	return 0;
}

/******************************************************************************
 * SPI driver Implement
 ******************************************************************************/

STATIC_DRV int LS1x_SPI_initialize(void *bus)
{
    LS1x_SPI_bus_t *pSPI = (LS1x_SPI_bus_t *)bus;
    
    if (bus == NULL)
        return -1;

    if (pSPI->initialized)
        return 0;

#if BSP_USE_OS
  #if defined(OS_RTTHREAD)
    pSPI->spi_mutex = rt_mutex_create(pSPI->dev_name, RT_IPC_FLAG_FIFO);
  #elif defined(OS_UCOS)
    unsigned char err;
    if ((unsigned)pSPI->hwSPI == LS1x_SPI0_BASE)
        pSPI->spi_mutex = OSMutexCreate(SPI0_MUTEX_PRIO, &err);
    else if ((unsigned)pSPI->hwSPI == LS1x_SPI1_BASE)
        pSPI->spi_mutex = OSMutexCreate(SPI1_MUTEX_PRIO, &err);
    else
        return -1;
  #elif defined(OS_FREERTOS)
    pSPI->spi_mutex = xSemaphoreCreateMutex();  /* 创建设备锁 */
  #endif

    if (pSPI->spi_mutex == NULL)
    	return -1;
#endif // #if BSP_USE_OS

	/* initialize with bsp bus frequency
	 */
	pSPI->base_frq = LS1x_BUS_FREQUENCY(CPU_XTAL_FREQUENCY);

	/*
	 * init HW registers:
	 */

	/* disable interrupt output */
	pSPI->hwSPI->ctrl &= ~spi_ctrl_ien;

	/* set spi-sr register, clear flag */
	pSPI->hwSPI->sr = spi_sr_iflag | spi_sr_txoverflow;

	/* disable SPI interrupt
	 */
	LS1x_INTC_CLR(pSPI->int_ctrlr) = pSPI->int_mask;
    LS1x_INTC_IEN(pSPI->int_ctrlr) &= ~(pSPI->int_mask);

    pSPI->initialized = 1;
    
    printk("SPI%i controller initialized.\r\n", \
           ((unsigned)pSPI->hwSPI == LS1x_SPI0_BASE) ? 0 : 1);
    
	return 0;
}

static int LS1x_SPI_read_write_bytes(LS1x_SPI_bus_t *pSPI,
						             unsigned char *rxbuf,
						             const unsigned char *txbuf,
						             int len)
{
	int rt, rw_cnt = 0;
	unsigned char rx_val;

	/* Clear interrupt and txoverflow flag */
	pSPI->hwSPI->sr |= (spi_sr_iflag | spi_sr_txoverflow);

	/* Allow interrupt */
	pSPI->hwSPI->ctrl |= spi_ctrl_ien;

	while (len-- > 0)
	{
		if (txbuf == NULL)
		{
			/* write dummy char while read. */
			pSPI->hwSPI->data.txfifo = pSPI->dummy_char;
		}
		else
		{
			pSPI->hwSPI->data.txfifo = *(unsigned char *)txbuf;
			txbuf++;
		}

		/*
		 * wait until end of transfer
		 */
		rt = LS1x_SPI_wait_tx_done(pSPI);

		if (0 != rt)
		{
			printk("SPI rw tmo.\r\n");
			return -rt;
		}

		rx_val = pSPI->hwSPI->data.rxfifo;
		if (rxbuf != NULL)
		{
			(*(unsigned char *)rxbuf) = rx_val;
			rxbuf++;
		}

		/*
		 * Clear Interrupt and txoverflow flag
		 */
		pSPI->hwSPI->sr |= (spi_sr_iflag | spi_sr_txoverflow);

		rw_cnt++;
	}

	/* Disable interrupt */
	pSPI->hwSPI->ctrl &= ~spi_ctrl_ien;

	return rw_cnt;
}

STATIC_DRV int LS1x_SPI_read_bytes(void *bus, unsigned char *buf, int len)
{
    LS1x_SPI_bus_t *pSPI = (LS1x_SPI_bus_t *)bus;
    
    if ((bus == NULL) || (buf == NULL))
        return -1;
        
	return LS1x_SPI_read_write_bytes(pSPI, buf, NULL, len);
}

/*
 * send some bytes to SPI device
 * Input Parameters:	bus specifier structure
 * 						buffer to send
 * 						number of bytes to send
 */
STATIC_DRV int LS1x_SPI_write_bytes(void *bus, unsigned char *buf, int len)
{
    LS1x_SPI_bus_t *pSPI = (LS1x_SPI_bus_t *)bus;
    
    if ((bus == NULL) || (buf == NULL))
        return -1;
        
	return LS1x_SPI_read_write_bytes(pSPI, NULL, buf, len);
}

static int LS1x_SPI_set_tfr_mode(LS1x_SPI_bus_t *pSPI, LS1x_SPI_mode_t *pMODE)
{
	struct LS1x_SPI_clkdiv clkdiv;
	int rt;
	unsigned char val;

	/*
	 * according baudrate, calculate the proper frequency
	 */
	rt = LS1x_SPI_baud_to_mode(pMODE->baudrate, pSPI->base_frq, &clkdiv);
	if (0 != rt)
	{
		return rt;
	}

	pSPI->hwSPI->ctrl &= ~spi_ctrl_en;				/* disable spi */

	/* set spi-er register */
	val = clkdiv.spre & spi_er_spre_mask;
	if (!pMODE->clock_phs)							/* = true: 同步发送 */
	{
		val |= spi_er_mode;
	}
	pSPI->hwSPI->er = val;

	/* set spi-param register */
	val = pSPI->hwSPI->param;
	val &= ~spi_param_clk_div_mask;
	val |= (clkdiv.spre << 6) | (clkdiv.spr << 4);
	pSPI->hwSPI->param = val;

	/* set spi-control register - spi mode */
	val = spi_ctrl_master | (clkdiv.spr & spi_ctrl_spr_mask);
	if (pMODE->clock_pha)
	{
		val |= spi_ctrl_cpha;
	}
	if (pMODE->clock_pol)
	{
		val |= spi_ctrl_cpol;
	}

	/* set new control value and re-enable spi */
	pSPI->hwSPI->ctrl = val | spi_ctrl_en;

	/* set spi-sr register, clear flag */
	pSPI->hwSPI->sr = spi_sr_iflag | spi_sr_txoverflow;

	/* set idle character */
	pSPI->dummy_char = pSPI->dummy_char & 0xFF;

	/* chip select mode */
	pSPI->chipsel_high = !pMODE->clock_inv;

	return 0;
}

STATIC_DRV int LS1x_SPI_ioctl(void *bus, int cmd, void *arg)
{
	LS1x_SPI_bus_t *pSPI = (LS1x_SPI_bus_t *)bus;
    LS1x_SPI_mode_t *pMODE;
    unsigned int val;
	int rt = 0;
	
    if (bus == NULL)
        return -1;

	switch (cmd)
	{
		case IOCTL_SPI_I2C_SET_TFRMODE:
		    pMODE = (LS1x_SPI_mode_t *)arg;
			rt = -LS1x_SPI_set_tfr_mode(pSPI, pMODE);
			break;

		case IOCTL_FLASH_FAST_READ_ENABLE:
        /*
			pSPI->hwSPI->timing = spi_timing_tcsh_2;
			pSPI->hwSPI->param |= spi_param_fast_read |
					              spi_param_burst_en |
								  spi_param_memory_en;
         */
            pSPI->hwSPI->param |= spi_param_memory_en;
			break;

		case IOCTL_FLASH_FAST_READ_DISABLE:
			pSPI->hwSPI->param &= ~spi_param_memory_en;
			break;

		case IOCTL_FLASH_GET_FAST_READ_MODE:
		    val  = (unsigned)pSPI->hwSPI->param;
            val &= spi_param_memory_en;
			*(unsigned int *)arg = val;
			break;

		default:
			rt = -1;
			break;
	}

	return rt;
}

/*
 * according chip-select to send begin r/w op
 */
STATIC_DRV int LS1x_SPI_send_start(void *bus, unsigned int Addr)
{
	unsigned char chip_sel;
    LS1x_SPI_bus_t *pSPI = (LS1x_SPI_bus_t *)bus;
    
    if (bus == NULL)
        return -1;
        
	LOCK(pSPI);
	Addr &= 0x03;							/* (sc_ptr->chipsel_nums - 1); */
	chip_sel = 1 << Addr;					/* set chip_sel enable */

	if (!pSPI->chipsel_high)				/* XXX de-select chip */
	{
    	chip_sel |= 1 << (Addr + 4);
    }

	pSPI->hwSPI->cs = chip_sel;

    return 0;
}

/*
 * spi chip select
 */
STATIC_DRV int LS1x_SPI_send_addr(void *bus, unsigned int Addr, int rw)
{
	unsigned char chip_sel;
    LS1x_SPI_bus_t *pSPI = (LS1x_SPI_bus_t *)bus;

    if (bus == NULL)
        return -1;
        
	Addr &= 0x03;						/* address range 0~3 */
	chip_sel = pSPI->hwSPI->cs & 0x0F;

	if (pSPI->chipsel_high)				/* select chip */
	{
    	chip_sel |= 1 << (Addr + 4);
    }
	else
	{
    	chip_sel &= ~(1 << (Addr + 4));
    }

	pSPI->hwSPI->cs = chip_sel;
	
	return 0;
}

/*
 * according chip-select to send finish r/w op
 */
STATIC_DRV int LS1x_SPI_send_stop(void *bus, unsigned int Addr)
{
	unsigned char chip_sel;
    LS1x_SPI_bus_t *pSPI = (LS1x_SPI_bus_t *)bus;
    
    if (bus == NULL)
        return -1;
        
	Addr &= 0x03;							/* (sc_ptr->chipsel_nums - 1); */
	chip_sel = pSPI->hwSPI->cs & 0x0F;

	if (pSPI->chipsel_high)				/* XXX de-select chip */
	{
		chip_sel &= ~(1 << (Addr + 4));
	}
	else
	{
		chip_sel |= 1 << (Addr + 4);
	}

	pSPI->hwSPI->cs = chip_sel;
	
    UNLOCK(pSPI);
	return 0;
}

//-----------------------------------------------------------------------------
// SPI bus driver ops
//-----------------------------------------------------------------------------

#if (PACK_DRV_OPS)
static libspi_ops_t LS1x_SPI_ops =
{
    .init        = LS1x_SPI_initialize,
    .send_start  = LS1x_SPI_send_start,
    .send_stop   = LS1x_SPI_send_stop,
    .send_addr   = LS1x_SPI_send_addr,
    .read_bytes  = LS1x_SPI_read_bytes,
    .write_bytes = LS1x_SPI_write_bytes,
    .ioctl       = LS1x_SPI_ioctl,
};
#endif

//-----------------------------------------------------------------------------
// SPI bus device table
//-----------------------------------------------------------------------------

#ifdef BSP_USE_SPI0
static LS1x_SPI_bus_t ls1x_SPI0 =
{
    .hwSPI        = (struct LS1x_SPI_regs *)LS1x_SPI0_BASE, /* pointer to HW registers */
    .base_frq     = 0,                                      /* input frq for baud rate divider */
    .chipsel_nums = 4,                                      /* total chip select numbers */
	.chipsel_high = 0,                                      /* cs high level - XXX: value from tfr-mode */
    .dummy_char   = 0,                                      /* This character will be continuously transmitted in read only functions */
    .irqNum       = LS1x_SPI0_IRQ,                          /* interrupt num */
    .int_ctrlr    = LS1x_INTC0_BASE,                        /* interrupt controller */
    .int_mask     = INTC0_SPI0_BIT,                         /* interrupt mask */
	.spi_mutex    = 0,                                      /* thread-safe */
    .initialized  = 0,
    .dev_name     = "spi0",
#if (PACK_DRV_OPS)
    .ops          = &LS1x_SPI_ops,
#endif
};
LS1x_SPI_bus_t *busSPI0 = &ls1x_SPI0;
#endif

#ifdef BSP_USE_SPI1
static LS1x_SPI_bus_t ls1x_SPI1 =
{
    .hwSPI        = (struct LS1x_SPI_regs *)LS1x_SPI1_BASE, /* pointer to HW registers */
    .base_frq     = 0,                                      /* input frq for baud rate divider */
    .chipsel_nums = 4,                                      /* total chip select numbers */
	.chipsel_high = 0,                                      /* cs high level - XXX: value from tfr-mode */
    .dummy_char   = 0,                                      /* This character will be continuously transmitted in read only functions */
    .irqNum       = LS1x_SPI1_IRQ,                          /* interrupt num */
    .int_ctrlr    = LS1x_INTC0_BASE,                        /* interrupt controller */
    .int_mask     = INTC0_SPI1_BIT,                         /* interrupt mask */
	.spi_mutex    = 0,                                      /* thread-safe */
    .initialized  = 0,
    .dev_name     = "spi1",
#if (PACK_DRV_OPS)
    .ops          = &LS1x_SPI_ops,
#endif
};
LS1x_SPI_bus_t *busSPI1 = &ls1x_SPI1;
#endif

/******************************************************************************
 * for external enable/disable spiflash fastread mode.
 */
int LS1x_enable_spiflash_fastread(LS1x_SPI_bus_t *pSPI)
{
    if (NULL == pSPI)
		return -1;

    LOCK(pSPI);
	pSPI->hwSPI->timing = spi_timing_tcsh_2;
	pSPI->hwSPI->param |= spi_param_fast_read | spi_param_burst_en | spi_param_memory_en;
    UNLOCK(pSPI);
    
	return 0;
}

int LS1x_disable_spiflash_fastread(LS1x_SPI_bus_t *pSPI)
{
    if (NULL == pSPI)
		return -1;
		
    LOCK(pSPI);
	pSPI->hwSPI->param &= ~spi_param_memory_en;
    UNLOCK(pSPI);
    
	return 0;
}

#endif

