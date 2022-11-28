/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 *  LS1X CAN driver
 *
 *  Author: Bian, 2013/10/09
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "bsp.h"

#if defined(BSP_USE_CAN0) || defined(BSP_USE_CAN1)

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_irq.h"
#include "ls1b_gpio.h"
#define LS1x_CAN0_BASE 		LS1B_CAN0_BASE
#define LS1x_CAN0_IRQ 		LS1B_CAN0_IRQ
#define LS1x_CAN1_BASE 		LS1B_CAN1_BASE
#define LS1x_CAN1_IRQ 		LS1B_CAN1_IRQ
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_irq.h"
#include "ls1c_gpio.h"
#define LS1x_CAN0_BASE 		LS1C_CAN0_BASE
#define LS1x_CAN0_IRQ 		LS1C_CAN0_IRQ
#define LS1x_CAN1_BASE 		LS1C_CAN1_BASE
#define LS1x_CAN1_IRQ 		LS1C_CAN1_IRQ
#else
#error "No Loongson1x SoC defined."
#endif

#if defined(OS_RTTHREAD)
#include "rtthread.h"
#elif defined(OS_UCOS)
#include "os.h"
#elif defined(OS_FREERTOS)
#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"
#endif

#include "ls1x_io.h"
#include "ls1x_can.h"
#include "ls1x_can_hw.h"
#include "drv_os_priority.h"

#if defined(OS_RTTHREAD)
#define MALLOC  rt_malloc
#define FREE    rt_free
#else
#define MALLOC  malloc
#define FREE    free
#endif

#define CAN_DEBUG       0

#define RX_FIFO_LEN		64
#define TX_FIFO_LEN		64

#define RESET_TIMEOUT 	100
#define MAX_TSEG2 		7
#define MAX_TSEG1 		15

//-------------------------------------------------------------------------------------------------

/******************************************************************************
 * fifo interface
 */
typedef struct
{
	int			count;
	int			ovcount;			/* overwrite count */
	int			full;				/* 1 = base contain cnt CANMsgs, tail==head */
	CANMsg_t   *tail, *head;
	CANMsg_t   *base;
	char		fifoarea[0];
} CAN_fifo_t;

/******************************************************************************
 * ls1x can priv defination
 */
typedef struct CAN
{
	/* hardware shortcuts
	 */
	LS1x_CAN_regs_t *hwCAN;			/* CAN 硬件 */
	unsigned int 	irqNum;			/* 中断号 */
	unsigned int	int_ctrlr;
	unsigned int	int_mask;

	CAN_speed_t	    timing;			/* btr0/btr1 */
	unsigned int 	afmode;			/* 单/双过滤模式 */
	unsigned int	coremode;		/* CAN core: 标准CAN2.0A、扩展CAN2.0B */
	unsigned int	workmode;		/* normal, selftest, listenonly */

#if defined(OS_RTTHREAD)
	rt_event_t         can_event;
#elif defined(OS_UCOS)
	OS_FLAG_GRP       *can_event;
#elif defined(OS_FREERTOS)
	EventGroupHandle_t can_event;
#endif

    int             timeout;        /* 收发超时 */
	int 		    started;		/* can device started */
	unsigned int 	status;
	CAN_stats_t		stats;

	/* rx and tx fifos
	 */
	CAN_fifo_t		*rxfifo;
	CAN_fifo_t		*txfifo;

	/* Config
	 */
	unsigned int	speed; 			/* speed in HZ */
	unsigned char	acode[4];
	unsigned char	amask[4];

    int             initialized;
    int             opened;
    char            dev_name[16];
} CAN_t;

//-------------------------------------------------------------------------------------------------
// Here is CAN interface defination
//-------------------------------------------------------------------------------------------------

#ifdef BSP_USE_CAN0
static CAN_t ls1x_CAN0 =
{
	.hwCAN     = (LS1x_CAN_regs_t *)LS1x_CAN0_BASE,
    .irqNum    = LS1x_CAN0_IRQ,
    .int_ctrlr = LS1x_INTC0_BASE,
    .int_mask  = INTC0_CAN0_BIT,
    .timeout     = -1,              /* wait forever */
    .rxfifo      = NULL,
	.txfifo      = NULL,
	.initialized = 0,
	.dev_name    = "can0",
};
void *devCAN0 = (void *)&ls1x_CAN0;
#endif

#ifdef BSP_USE_CAN1
static CAN_t ls1x_CAN1 =
{
	.hwCAN     = (LS1x_CAN_regs_t *)LS1x_CAN1_BASE,
    .irqNum    = LS1x_CAN1_IRQ,
    .int_ctrlr = LS1x_INTC0_BASE,
    .int_mask  = INTC0_CAN1_BIT,
    .timeout     = -1,              /* wait forever */
    .rxfifo      = NULL,
	.txfifo      = NULL,
	.initialized = 0,
	.dev_name    = "can1",
};
void *devCAN1 = (void *)&ls1x_CAN1;
#endif

/******************************************************************************
 * FIFO IMPLEMENTATION
 ******************************************************************************/

static CAN_fifo_t *can_fifo_create(int count)
{
	CAN_fifo_t *fifo;

	fifo = MALLOC(sizeof(CAN_fifo_t) + count * sizeof(CANMsg_t));
	if (fifo)
	{
		fifo->count = count;
		fifo->full = 0;
		fifo->ovcount = 0;
		fifo->base = (CANMsg_t *)&fifo->fifoarea[0];
		fifo->tail = fifo->head = fifo->base;

		/* clear CAN Messages
		 */
		memset(fifo->base, 0, count * sizeof(CANMsg_t));
	}

	return fifo;
}

static void can_fifo_free(CAN_fifo_t *fifo)
{
	if (fifo)
	{
		FREE(fifo);
		fifo = NULL;
	}
}

static inline int can_fifo_full(CAN_fifo_t *fifo)
{
	return fifo->full;
}

static inline int can_fifo_empty(CAN_fifo_t *fifo)
{
	return (!fifo->full) && (fifo->head == fifo->tail);
}

static void can_fifo_get(CAN_fifo_t *fifo)
{
	if (!fifo)
		return;

	if (can_fifo_empty(fifo))
		return;

	/*
     * increment indexes
	 */
	fifo->tail = (fifo->tail >= &fifo->base[fifo->count - 1]) ? fifo->base : fifo->tail + 1;
	fifo->full = 0;
}

/* Stage 1 - get buffer to fill (never fails if force!=0)
 */
static CANMsg_t *can_fifo_put_claim(CAN_fifo_t *fifo, int force)
{
	if (!fifo)
		return NULL;

	if (can_fifo_full(fifo))
	{
		if (!force)
			return NULL;

		/* all buffers already used ==> overwrite the oldest
		 */
		fifo->ovcount++;
		can_fifo_get(fifo);
	}

	return fifo->head;
}

/* Stage 2 - increment indexes
 */
static void can_fifo_put(CAN_fifo_t *fifo)
{
	if (can_fifo_full(fifo))
		return;

	/*
     * wrap around the indexes
	 */
	fifo->head = (fifo->head >= &fifo->base[fifo->count - 1]) ? fifo->base : fifo->head + 1;
	if (fifo->head == fifo->tail)
		fifo->full = 1;
}

static CANMsg_t *can_fifo_claim_get(CAN_fifo_t *fifo)
{
	if (can_fifo_empty(fifo))
		return NULL;

	/* return oldest message
	 */
	return fifo->tail;
}

static void can_fifo_clear(CAN_fifo_t *fifo)
{
	fifo->full = 0;
	fifo->ovcount = 0;
	fifo->head = fifo->tail = fifo->base;
}

/******************************************************************************
 * Predefined Interface
 ******************************************************************************/

/* interrupt routine, use for PeliCAN
 */
static void inline LS1x_CAN_interrupt_enable(CAN_t *pCAN);
static void inline LS1x_CAN_interrupt_disable(CAN_t *pCAN);
static void LS1x_CAN_interrupt_handler(int vector, void *arg);

#if (PACK_DRV_OPS)
static int LS1x_CAN_init(void *dev, void *arg);
static int LS1x_CAN_open(void *dev, void *arg);
static int LS1x_CAN_close(void *dev, void *arg);
static int LS1x_CAN_read(void *dev, void *buf, int size, void *arg);
static int LS1x_CAN_write(void *dev, void *buf, int size, void *arg);
static int LS1x_CAN_ioctl(void *dev, int cmd, void *arg);
#endif

/******************************************************************************
 * Hardware Operating
 ******************************************************************************/

/* This function calculates BTR0 and BTR1 values for a given bitrate.
 *
 * Set communication parameters.
 * param: rate Requested baud rate in bits/second.
 * param: result Pointer to where resulting BTRs will be stored.
 * return: zero if successful to calculate a baud rate.
 */
static int LS1x_CAN_calc_speedregs(unsigned int rate, CAN_speed_t *result)
{
	int best_error = 1000000000;
	int error;
	int best_tseg=0, best_brp=0, best_rate=0, brp=0;
	int tseg=0, tseg1=0, tseg2=0;
	int sjw = 0;
	int sampl_pt = 65;

	int clock_hz = LS1x_BUS_FREQUENCY(CPU_XTAL_FREQUENCY);

	if ((rate < 5000) || (rate > 1000000))
		return -1;

	/****************************************************************
	 * find best match, return -2 if no good reg combination
	 * is available for this frequency
	 ****************************************************************/

	/* tseg even = round down, odd = round up
	 */
	for (tseg = (0 + 0 + 2) * 2;
	     tseg <= (MAX_TSEG2 + MAX_TSEG1 + 2) * 2 + 1;
	     tseg++)
	{
		brp = clock_hz / ((1 + tseg / 2) * rate) + tseg % 2;
		if ((brp == 0) || (brp > 64))
			continue;

		error = rate - clock_hz / (brp * (1 + tseg / 2));
		if (error < 0)
		{
			error = -error;
		}

		if (error <= best_error)
		{
			best_error = error;
			best_tseg = tseg / 2;
			best_brp = brp - 1;
			best_rate = clock_hz / (brp * (1 + tseg / 2));
		}
	}

	if (best_error && (rate / best_error < 10))
	{
		printk("CAN Error: rate %i is not possible with %iHZ clock\r\n", (int)rate, (int)clock_hz);
		return -2;
	}
	else if (!result)
	{
		/* nothing to store result in, but a valid bitrate can be calculated
		 */
		return 0;
	}

	/* some heuristic specials
	 */
	if (best_tseg <= 10)
		sampl_pt = 80;
	else if (best_tseg <= 15)
		sampl_pt = 72;

	tseg2 = best_tseg - (sampl_pt * (best_tseg + 1)) / 100;

	if (tseg2 < 0)
		tseg2 = 0;

	if (tseg2 > MAX_TSEG2)
		tseg2 = MAX_TSEG2;

	tseg1 = best_tseg - tseg2 - 2;

	if (tseg1 > MAX_TSEG1)
	{
		tseg1 = MAX_TSEG1;
		tseg2 = best_tseg - tseg1 - 2;
	}

	if (tseg2 >= 3)
		sjw = 1;

	if (rate > 250000)
		result->samples = 0;		/* 1 次采样 */
	else
		result->samples = 1;		/* 3 次采样 */

	result->btr0 = (sjw << can_btr0_sjw_shift) | (best_brp & can_btr0_brp_mask);

	if (result->samples)
		result->btr1 = (1 << 7) | (tseg2 << can_btr1_tseg2_shift) | tseg1;
	else
		result->btr1 = (0 << 7) | (tseg2 << can_btr1_tseg2_shift) | tseg1;

	return 0;
}

static int LS1x_CAN_set_speedregs(CAN_t *pCAN, CAN_speed_t *timing)
{
	if (!timing || !pCAN || !pCAN->hwCAN)
		return -1;

	pCAN->hwCAN->btr0 = timing->btr0;
	pCAN->hwCAN->btr1 = timing->btr1;

	return 0;
}

static void LS1x_CAN_set_accept(CAN_t *pCAN, unsigned char *acode, unsigned char *amask)
{
	if (pCAN->coremode == CAN_CORE_20A)
	{
		pCAN->hwCAN->reg.std.accode = acode[0];
		pCAN->hwCAN->reg.std.acmask = amask[0];
	}
	else
	{
		pCAN->hwCAN->msg.ext.msg.accept.accode[0] = acode[0];
		pCAN->hwCAN->msg.ext.msg.accept.accode[1] = acode[1];
		pCAN->hwCAN->msg.ext.msg.accept.accode[2] = acode[2];
		pCAN->hwCAN->msg.ext.msg.accept.accode[3] = acode[3];
		pCAN->hwCAN->msg.ext.msg.accept.acmask[0] = amask[0];
		pCAN->hwCAN->msg.ext.msg.accept.acmask[1] = amask[1];
		pCAN->hwCAN->msg.ext.msg.accept.acmask[2] = amask[2];
		pCAN->hwCAN->msg.ext.msg.accept.acmask[3] = amask[3];
	}
}

/*
 * put the CAN-Control to reset mode
 */
static void LS1x_CAN_hw_stop(CAN_t *pCAN)
{
	int i;
	unsigned char tmp;

	/* Disable CAN interrupt
	 */
	LS1x_CAN_interrupt_disable(pCAN);

	/* disable can interrupts
	 */
	if (pCAN->coremode == CAN_CORE_20B)
		pCAN->hwCAN->reg.ext.inten = 0;

	tmp = pCAN->hwCAN->intflags;				/* Read for clear */

	tmp = pCAN->hwCAN->ctrl;

	for (i=0; i<RESET_TIMEOUT; i++)
	{
		if (tmp & can_ctrl_reset)				/* Check reset bit */
		{
		    pCAN->started = false;
			return;
		}

		pCAN->hwCAN->ctrl = can_ctrl_reset;		/* Reset chip */

		delay_us(10);

		tmp = pCAN->hwCAN->ctrl;
	}

   	printk("Error: CAN Device reset timeout.\r\n");

}

/*
 * put the CAN-Control into workmode
 */
static int LS1x_CAN_hw_start(CAN_t *pCAN)
{
	unsigned char ctrl, tmp;
	int i;

	if (!pCAN->rxfifo || !pCAN->txfifo)
		return -1;

	if (pCAN->started)
		LS1x_CAN_hw_stop(pCAN);

	/* empty the TX fifo
	 */
	can_fifo_clear(pCAN->txfifo);

	pCAN->status = 0;

	LS1x_CAN_set_speedregs(pCAN, &pCAN->timing);
	LS1x_CAN_set_accept(pCAN, pCAN->acode, pCAN->amask);

	/*
	 * 必须在进入工作模式前先设置 CAN 模式
	 */
	if (pCAN->coremode == CAN_CORE_20A)
	{
		pCAN->hwCAN->cmd = can_cmd_standard;		/* CAN2.0A */
		delay_us(10);

		/* Enable all CAN interrupt, and set core in workmode
		 */
		ctrl = can_std_interrupts;
	}
	else
	{
		pCAN->hwCAN->cmd = can_cmd_extended;		/* CAN2.0B */
		delay_us(10);

		ctrl = can_ctrl_standwork_ext;				/* XXX CAN_STAND_MODE, Care of here! */

//		if (sc->afmode)
//			ctrl |= can_ctrl_afilter_ext;

		pCAN->hwCAN->msg.ext.rxerrcnt = 0;
		pCAN->hwCAN->msg.ext.txerrcnt = 0;

		tmp = pCAN->hwCAN->msg.ext.errcode;
	}

	/* clear CAN pending interrupts
	 */
	tmp = pCAN->hwCAN->intflags;

	tmp = pCAN->hwCAN->ctrl;

	for (i=0; i<RESET_TIMEOUT; i++)
	{
		if ((tmp & can_ctrl_reset) == 0)		/* Exit reset mode, begin working. */
		{
			pCAN->started = true;

			if (pCAN->coremode == CAN_CORE_20B)
			{
				/* Enable all CAN interrupt
				 */
				pCAN->hwCAN->reg.ext.inten = can_inten_all_ext;
			}

			/* Enable CAN interrupt
			 */
			LS1x_CAN_interrupt_enable(pCAN);

			return 0;
		}

		pCAN->hwCAN->ctrl = ctrl;

		delay_us(10);

		tmp = pCAN->hwCAN->ctrl;
	}

	printk("Error: CAN Device set work timeout.\n");

	return -1;
}

/* Try to send message "msg", if hardware txfifo is full, then -1 is returned.
 * Be sure to have disabled CAN interrupts when entering this function.
 */
static int LS1x_CAN_send_msg(CAN_t *pCAN, CANMsg_t *msg)
{
	unsigned char ch, cmd;

	/* is there room in send buffer?
	 */
	if (!(pCAN->hwCAN->status & can_status_txbuf))
		return -1;

	/* Transmit command
	 */
	cmd = can_cmd_txrequest;

	ch = msg->len & 0x0F;

	if (pCAN->coremode == CAN_CORE_20A)		/* Standard Frame */
	{
		if (msg->rtr)
			ch |= 0x10;

		pCAN->hwCAN->msg.std.tx.id[0] = ((unsigned char)(msg->id >> 3) & 0xFF);
		pCAN->hwCAN->msg.std.tx.id[1] = ((unsigned char)(msg->id << 5) & 0xE0) | ch;
		ch = msg->len & 0x0F;
		while (ch--)
		{
			pCAN->hwCAN->msg.std.tx.data[ch] = msg->data[ch];
		}
	}
	else
	{
		if (msg->rtr)
			ch |= 0x40;

		if (msg->extended)					/* Extended Frame */
		{
			pCAN->hwCAN->msg.ext.msg.tx.frameinfo = ch | 0x80;
			pCAN->hwCAN->msg.ext.msg.tx.ext.id[0] = (unsigned char)(msg->id >> (5+8+8)) & 0xFF;
			pCAN->hwCAN->msg.ext.msg.tx.ext.id[1] = (unsigned char)(msg->id >> (5+8)) & 0xFF;
			pCAN->hwCAN->msg.ext.msg.tx.ext.id[2] = (unsigned char)(msg->id >> (5)) & 0xFF;
			pCAN->hwCAN->msg.ext.msg.tx.ext.id[3] = (unsigned char)(msg->id << 3) & 0xF8;
			ch = msg->len & 0x0F;
			while (ch--)
			{
				pCAN->hwCAN->msg.ext.msg.tx.ext.data[ch] = msg->data[ch];
			}
		}
		else								/* Standard Frame */
		{
			pCAN->hwCAN->msg.ext.msg.tx.frameinfo = ch;
			pCAN->hwCAN->msg.ext.msg.tx.std.id[0] = (unsigned char)(msg->id >> 3) & 0xFF;
			pCAN->hwCAN->msg.ext.msg.tx.std.id[1] = (unsigned char)(msg->id << 5) & 0xE0;
			ch = msg->len & 0x0F;
			while (ch--)
			{
				pCAN->hwCAN->msg.ext.msg.tx.std.data[ch] = msg->data[ch];
			}
		}

		/* TODO Please care of here!
		 */
		if (pCAN->workmode == CAN_SELF_RECEIVE)
			cmd |= can_cmd_selfrxrequest_ext;
	}

#if 0
	/* using with "can_cmd_txrequest" send once
	 */
	if (msg->sshot)
		cmd |= can_cmd_txabort;
#endif

	pCAN->hwCAN->cmd = cmd;

	return 0;
}

/******************************************************************************
 * CAN Driver Implement
 ******************************************************************************/

static void LS1x_CAN_clear_softc(CAN_t *pCAN)
{
    int i;
  
    memset((void *)&pCAN->stats,  0, sizeof(CAN_stats_t));
    
    pCAN->timing.btr0 = 0;
    pCAN->timing.btr1 = 0;
    pCAN->timing.samples = 0;
    
    pCAN->afmode   = 0;
    pCAN->coremode = 0;
    pCAN->workmode = 0;
    pCAN->timeout = 0;
    pCAN->started = 0;
    pCAN->status  = 0;
    pCAN->speed   = 0;

	can_fifo_free(pCAN->rxfifo);
	can_fifo_free(pCAN->txfifo);
	
    for (i=0; i<4; i++)
    {
	    pCAN->acode[i] = 0;
	    pCAN->amask[i] = 0;
    }
}

/******************************************************************************
 * LS1x_CAN_init
 */
STATIC_DRV int LS1x_CAN_init(void *dev, void *arg)
{
    CAN_t *pCAN = (CAN_t *)dev;

	if (dev == NULL)
		return -1;
		
    if (pCAN->initialized)
        return 0;
        
#if defined(LS1C)

    if ((unsigned int)pCAN->hwCAN == LS1x_CAN1_BASE)
    {
        /* Set device CAN1 */
        gpio_set_pinmux(31, 4);    // Pin: 144
        gpio_set_pinmux(32, 4);    // Pin: 145

        /* Enable CAN1 */
        LS1C_SHUT_CTRL &= ~SHUT_CTRL_CAN1_SHUT;
    }
    
    delay_ms(1);
    
#endif

    LS1x_CAN_clear_softc(pCAN);

#if defined(OS_RTTHREAD)
    pCAN->can_event = rt_event_create(pCAN->dev_name, 0);
#elif defined(OS_UCOS)
    unsigned char err;
    pCAN->can_event = OSFlagCreate(0, &err);
#elif defined(OS_FREERTOS)
    pCAN->can_event = xEventGroupCreate();
#endif

#if BSP_USE_OS
    if (NULL == pCAN->can_event)
    {
        printk("create CAN event fail.\r\n");
        return -1;
    }
#endif
	/*
	 * 设置 CAN 默认工作模式
	 */
	pCAN->coremode = CAN_CORE_20B;			/* Default is CAN2.0B */
	pCAN->workmode = CAN_STAND_MODE;		/* Normal */
	pCAN->speed    = CAN_SPEED_250K;
	pCAN->amask[0] = 0xFF;
	pCAN->amask[1] = 0xFF;
	pCAN->amask[2] = 0xFF;
	pCAN->amask[3] = 0xFF;
	pCAN->afmode   = 1; 					/* single acceptance filter */

	if (LS1x_CAN_calc_speedregs(pCAN->speed, &pCAN->timing))
	{
		/* calculate rate error
		 */
		pCAN->speed = CAN_SPEED_50K;
		LS1x_CAN_calc_speedregs(pCAN->speed, &pCAN->timing);
	}

	/* stop CAN, put core in reset mode
	 */
	LS1x_CAN_hw_stop(pCAN);

	/* set CAN with poll trigger mode
	 */
	LS1x_INTC_EDGE(pCAN->int_ctrlr) &= ~pCAN->int_mask;
	LS1x_INTC_POL(pCAN->int_ctrlr)  |=  pCAN->int_mask;

	/* Setup interrupt handler
	 */
    ls1x_install_irq_handler(pCAN->irqNum, LS1x_CAN_interrupt_handler, (void *)pCAN);

    pCAN->initialized = 1;
    
    printk("CAN%i controller initialized.\r\n",
           ((unsigned)pCAN->hwCAN == LS1x_CAN0_BASE) ? 0 : 1);
    
	return 0;
}

/******************************************************************************
 * LS1x_CAN_open
 */
STATIC_DRV int LS1x_CAN_open(void *dev, void *arg)
{
    CAN_t *pCAN = (CAN_t *)dev;
    
	if (dev == NULL)
		return -1;

    if (pCAN->opened)
        return 0;
        
	/* already open
	 */
	if (pCAN->rxfifo && pCAN->txfifo)
		return 0;

	/* allocate fifos
	 */
	pCAN->rxfifo = can_fifo_create(RX_FIFO_LEN);
	if (!pCAN->rxfifo)
    	return -1;

    pCAN->txfifo = can_fifo_create(TX_FIFO_LEN);
	if (!pCAN->txfifo)
	{
		can_fifo_free(pCAN->rxfifo);
		return -1;
	}

	/* clear stat counters
	 */
	memset(&pCAN->stats, 0, sizeof(CAN_stats_t));

	/* HW must be in reset-mode here (close and initializes resets core...)
	 * set default modes/speeds
	 */
	LS1x_CAN_hw_stop(pCAN);

    pCAN->opened = 1;
	return 0;
}

/******************************************************************************
 * LS1x_CAN_close
 */
STATIC_DRV int LS1x_CAN_close(void *dev, void *arg)
{
    CAN_t *pCAN = (CAN_t *)dev;
    
	if (dev == NULL)
		return -1;

	/* stop CAN device, and set to reset mode
	 */
	LS1x_CAN_hw_stop(pCAN);

    LS1x_CAN_clear_softc(pCAN);

    pCAN->opened = 0;
	return 0;
}

/******************************************************************************
 * LS1x_CAN read
 */
STATIC_DRV int LS1x_CAN_read(void *dev, void *buf, int size, void *arg)
{
    int left = size;
    CAN_t *pCAN = (CAN_t *)dev;
    CANMsg_t *srcmsg, *dstmsg = (CANMsg_t *)buf;

	/* does at least one message fit ?
	 */
	if ((dev == NULL) || (buf == NULL) || (left < sizeof(CANMsg_t)))
		return -1;

	if (!pCAN->started)
	{
		LS1x_CAN_hw_start(pCAN);
	}

	while (left >= sizeof(CANMsg_t))
	{
		/* A bus off interrupt may have occured after read
		 */
		if (pCAN->status & (CAN_STATUS_ERR_BUSOFF | CAN_STATUS_RESET))
		{
        	return -2;
        }

		srcmsg = can_fifo_claim_get(pCAN->rxfifo);

		if (!srcmsg)
		{
		    int tmo = pCAN->timeout;

			/* No more messages in reception fifo. Wait for incoming packets
			 * return if no wait OR readed some messages.
			 */
			if ((tmo == 0) || (left != size))
				break;

			/*
             * wait for incomming messages...
			 */
        #if BSP_USE_OS
          #if defined(OS_RTTHREAD)
          
		    unsigned int recv = 0;
		    if (tmo < 0) tmo = RT_WAITING_FOREVER;
            rt_event_recv(pCAN->can_event,
                          CAN_RX_EVENT,
                          RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                          tmo,
                          &recv);

          #elif defined(OS_UCOS)
          
            unsigned char err;
            unsigned short recv;
            if (tmo < 0) tmo = 0;                       /* 0=无限等待 */
            recv = OSFlagPend(pCAN->can_event,
                              (OS_FLAGS)CAN_RX_EVENT,   /* 接收事件 */
                              OS_FLAG_WAIT_SET_ALL |    /* 接收事件标志位置1时有效，否则任务挂在这里 */
                              OS_FLAG_CONSUME,          /* 清除指定事件标志位 */
                              tmo,
                              &err);

          #elif defined(OS_FREERTOS)
          
            unsigned int recv;
            if (tmo < 0) tmo = portMAX_DELAY;
            recv = xEventGroupWaitBits(pCAN->can_event, /* 事件对象句柄 */
                                       CAN_RX_EVENT,    /* 接收事件 */
                                       pdTRUE,          /* 退出时清除事件位 */
                                       pdTRUE,          /* 满足感兴趣的所有事件 */
                                       tmo);            /* 指定超时事件, 一直等 */

          #endif
          
            if (recv != CAN_RX_EVENT)
	            break;
          
        #else 
        
            while (tmo-- > 0)
            {
                delay_ms(1);
                if (!can_fifo_empty(pCAN->rxfifo))
                    break;
            }
                
            if (can_fifo_empty(pCAN->rxfifo))
                break;
                
        #endif // #if BSP_USE_OS
        
			/* did we get woken up by a BUS OFF error?
			 */
			if (pCAN->status & (CAN_STATUS_ERR_BUSOFF | CAN_STATUS_RESET))
			{
				/* At this point it should not matter how many messages we handled
				 */
                return -2;
			}

			/* no errors detected, it must be a message
			 */
			continue;
		}

		/* got message, copy it to userspace buffer
		 */
		*dstmsg = *srcmsg;

		/* Return borrowed message, RX interrupt can use it again
		 */
		can_fifo_get(pCAN->rxfifo);

		left -= sizeof(CANMsg_t);
		dstmsg++;
	}

    return size - left;
}

/******************************************************************************
 * LS1x_CAN write
 */
STATIC_DRV int LS1x_CAN_write(void *dev, void *buf, int size, void *arg)
{
    int left = size;
    CAN_t *pCAN = (CAN_t *)dev;
    CANMsg_t *msg = (CANMsg_t *)buf, *fifo_msg;

	if ((dev == NULL) || (buf == NULL) || (left < sizeof(CANMsg_t)))
		return -1;

	msg->len = (msg->len > 8) ? 8 : msg->len;

	/* A bus off interrupt may have occured before being send
	 */
	if (pCAN->status & (CAN_STATUS_ERR_BUSOFF | CAN_STATUS_RESET))
		return -2;

	if (!pCAN->started)
	{
		LS1x_CAN_hw_start(pCAN);
	}

	/* If no messages in software tx fifo, we will try to send first message
	 * by putting it directly into the HW TX fifo.
	 */
	if (can_fifo_empty(pCAN->txfifo))
	{
		if (LS1x_CAN_send_msg(pCAN, msg) == 0)
		{
			/* First message put directly into HW TX fifo, This will turn TX interrupt on.
			 */
			left -= sizeof(CANMsg_t);
			msg++;

			pCAN->stats.tx_msgs++;
		}
	}

	/* Put messages into software fifo
	 */
	while (left >= sizeof(CANMsg_t))
	{
		msg->len = (msg->len > 8) ? 8 : msg->len;

		fifo_msg = can_fifo_put_claim(pCAN->txfifo, 0);

		if (!fifo_msg)
		{
		    int tmo = pCAN->timeout;

            /* Waiting only if no messages previously sent.
             * return if no wait OR written some messages.
			 */
			if ((tmo == 0) || (left != size))
				break;

			/*
             * wait for messages sent...
			 */
        #if BSP_USE_OS
          #if defined(OS_RTTHREAD)
          
		    unsigned int recv = 0;
		    if (tmo < 0) tmo = RT_WAITING_FOREVER;
            rt_event_recv(pCAN->can_event,
                          CAN_TX_EVENT,
                          RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                          tmo,
                          &recv);

          #elif defined(OS_UCOS)
          
            unsigned char err;
            unsigned short recv;
            if (tmo < 0) tmo = 0;                       /* 0=无限等待 */
            recv = OSFlagPend(pCAN->can_event,
                              (OS_FLAGS)CAN_TX_EVENT,   /* 发送事件 */
                              OS_FLAG_WAIT_SET_ALL |    /* 接收事件标志位置1时有效，否则任务挂在这里 */
                              OS_FLAG_CONSUME,          /* 清除指定事件标志位 */
                              tmo,                      /* 直到收到信号为止 */
                              &err);

        #elif defined(OS_FREERTOS)
        
            unsigned int recv;
            if (tmo < 0) tmo = portMAX_DELAY;
            recv = xEventGroupWaitBits(pCAN->can_event, /* 事件对象句柄 */
                                       CAN_TX_EVENT,    /* 发送事件 */
                                       pdTRUE,          /* 退出时清除事件位 */
                                       pdTRUE,          /* 满足感兴趣的所有事件 */
                                       tmo);            /* 指定超时事件, 一直等 */

        #endif
        
            if (recv != CAN_TX_EVENT)
                break;

        #else
        
            while (tmo-- > 0)
            {
                delay_ms(1);
                if (!can_fifo_full(pCAN->txfifo))
                    break;
            }

            if (can_fifo_full(pCAN->txfifo))
                break;

        #endif // #if BSP_USE_OS

			/* did we get woken up by a BUS OFF error?
			 */
			if (pCAN->status & (CAN_STATUS_ERR_BUSOFF | CAN_STATUS_RESET))
			{
				/* At this point it should not matter how many messages we handled
				 */
				return -2;
			}

			if (can_fifo_empty(pCAN->txfifo))
			{
				if (!LS1x_CAN_send_msg(pCAN, msg))
				{
					/* First message put directly into HW TX fifo
					 * This will turn TX interrupt on.
					 */
					left -= sizeof(CANMsg_t);
					msg++;

					pCAN->stats.tx_msgs++;
				}
			}

			continue;
		}

		/* copy message into fifo area
		 */
		*fifo_msg = *msg;

		/* tell interrupt handler about the message
		 */
		can_fifo_put(pCAN->txfifo);

		/* Prepare insert of next message
		 */
		msg++;
		left -= sizeof(CANMsg_t);
	}
	
    return size - left;
}

/******************************************************************************
 * LS1x_CAN control
 */
#define CAN_STARTED_BREAK   if (pCAN->started) { rt = -1; break; }
#define PTR_NULL_BREAK(ptr) if (ptr == NULL)   { rt = -1; break; }
 
STATIC_DRV int LS1x_CAN_ioctl(void *dev, int cmd, void *arg)
{
    int rt = 0;
    CAN_t *pCAN = (CAN_t *)dev;
	CAN_speed_t timing;
	CAN_afilter_t *afilter;
	CAN_stats_t *dststats;
	unsigned int speed, rxcnt, txcnt, val;

	if (dev == NULL)
		return -1;

	switch (cmd)
	{
		case IOCTL_CAN_SET_SPEED:
			CAN_STARTED_BREAK;
			speed = (unsigned int)arg; 
			rt = LS1x_CAN_calc_speedregs(speed, &timing);
			if (rt)
			{
				rt = -2; 
				break;
			}
			LS1x_CAN_set_speedregs(pCAN, &timing);
			pCAN->speed = speed;
			pCAN->timing = timing;
			break;

		case IOCTL_CAN_SET_BTRS:
			CAN_STARTED_BREAK;
			pCAN->speed = 0;
			val = (unsigned int)arg; 
			pCAN->timing.btr1 = val & 0xFF;
			pCAN->timing.btr0 = (val>>8) & 0xFF;
			LS1x_CAN_set_speedregs(pCAN, &pCAN->timing);
			break;

		case IOCTL_CAN_SPEED_AUTO:
			rt = 0; 
			break;

		case IOCTL_CAN_SET_BUFLEN:
			CAN_STARTED_BREAK;
			val = (unsigned int)arg; 
			rxcnt = val & 0x0000FFFF;
			txcnt = val >> 16;
			can_fifo_free(pCAN->rxfifo);
			can_fifo_free(pCAN->txfifo);
			pCAN->rxfifo = can_fifo_create(rxcnt);
			pCAN->txfifo = can_fifo_create(txcnt);
			if (!pCAN->rxfifo || !pCAN->txfifo)
			{
				rt = -2; 
				break;
			}
			break;

		case IOCTL_CAN_GET_CONF:
			rt = 0; 
			break;

		case IOCTL_CAN_GET_STATS:
			dststats = (CAN_stats_t *)arg; 
			PTR_NULL_BREAK(dststats);
			if (pCAN->rxfifo)
				pCAN->stats.rx_sw_dovr = pCAN->rxfifo->ovcount;
			*dststats = pCAN->stats;
			break;

		case IOCTL_CAN_GET_STATUS:
			if (!arg)
			{
				rt = -1; 
				break;
			}
			*(unsigned int *)arg = pCAN->status;
			break;

		case IOCTL_CAN_SET_LINK:
			rt = 0; 
			break;

		case IOCTL_CAN_SET_FILTER:
			CAN_STARTED_BREAK;
			afilter = (CAN_afilter_t *)arg; 
			if (!afilter)
			{
				rt = -2; 
				break;
			}
			pCAN->acode[0] = afilter->code[0];
			pCAN->acode[1] = afilter->code[1];
			pCAN->acode[2] = afilter->code[2];
			pCAN->acode[3] = afilter->code[3];
			pCAN->amask[0] = afilter->mask[0];
			pCAN->amask[1] = afilter->mask[1];
			pCAN->amask[2] = afilter->mask[2];
			pCAN->amask[3] = afilter->mask[3];
			pCAN->afmode   = afilter->afmode;
			LS1x_CAN_set_accept(pCAN, pCAN->acode, pCAN->amask);
			break;

		case IOCTL_CAN_START:
			if (pCAN->started)
			{
				rt = -2; 
				break;
			}
			if (LS1x_CAN_hw_start(pCAN))
			{
				rt = -2; 
				break;
			}
			break;

		case IOCTL_CAN_STOP:
			if (!pCAN->started)
			{
				rt = -2; 
				break;
			}
			LS1x_CAN_hw_stop(pCAN);
			break;

		case IOCTL_CAN_SET_CORE:
			CAN_STARTED_BREAK;
			val = (unsigned int)arg; 
			pCAN->coremode = val;
			break;

		case IOCTL_CAN_SET_WORKMODE:
			CAN_STARTED_BREAK;
			val = (unsigned int)arg; 
			pCAN->workmode = val;
			break;

		case IOCTL_CAN_SET_TIMEOUT:
			CAN_STARTED_BREAK;
			val = (unsigned int)arg; 
			pCAN->timeout = val;
			break;

		default:
			rt = 0;
			break;
	}

	return rt;
}

/******************************************************************************
 * Interrupt Implement
 ******************************************************************************/

static void inline LS1x_CAN_interrupt_enable(CAN_t *pCAN)
{
    LS1x_INTC_CLR(pCAN->int_ctrlr)  = pCAN->int_mask;
    LS1x_INTC_IEN(pCAN->int_ctrlr) |= pCAN->int_mask;
}

static void inline LS1x_CAN_interrupt_disable(CAN_t *pCAN)
{
	LS1x_INTC_CLR(pCAN->int_ctrlr)  = pCAN->int_mask;
    LS1x_INTC_IEN(pCAN->int_ctrlr) &= ~(pCAN->int_mask);
}

/*
 * XXX ls1x CAN interrupt only support extended - CAN2.0B
 */
static void LS1x_CAN_interrupt_handler(int vector, void *arg)
{
	unsigned char iflags;
	unsigned char ch, errcode, arbcode;
	int tx_error_cnt, rx_error_cnt, rx_flag=0, tx_flag=0;
	CANMsg_t *msg;
    CAN_t *pCAN = (CAN_t *)arg;

    if (NULL == pCAN)
        return;

#if defined(OS_UCOS)
    unsigned char err;
#elif defined(OS_FREERTOS)
	BaseType_t xResult, xHigherPriorityTaskWoken = pdFALSE; 
#endif

	LS1x_CAN_interrupt_disable(pCAN);

	pCAN->stats.ints++;

	iflags = pCAN->hwCAN->intflags;
	if (pCAN->coremode == CAN_CORE_20A)
		iflags &= 0x1F;

	while (iflags != 0)
	{
		/*
         * still interrupts to handle
		 */
		if (iflags & can_intflags_rx)
		{
			/* the rx fifo is not empty put 1 message into rxfifo for later use.
			 * get empty (or make room) message
			 */
			msg = can_fifo_put_claim(pCAN->rxfifo, 1);

			if (pCAN->coremode & CAN_CORE_20A)
			{
				/* Standard frame */
				ch = pCAN->hwCAN->msg.std.rx.id[1];
				msg->extended = 0;
				msg->rtr = (ch >> 4) & 0x01;
				msg->len = ch = ch & 0x0F;

				msg->id = (pCAN->hwCAN->msg.std.rx.id[0] << 3) |
						  (pCAN->hwCAN->msg.std.rx.id[1] >> 5);
				while (ch--)
				{
					msg->data[ch] = pCAN->hwCAN->msg.std.rx.data[ch];
				}
			}
			else
			{
				ch = pCAN->hwCAN->msg.ext.msg.rx.frameinfo;
				msg->extended = ch >> 7;

				if (msg->extended)
				{
					/* Extended Frame */
					msg->rtr = (ch >> 6) & 0x01;
					msg->len = ch = ch & 0x0F;
					msg->id = (pCAN->hwCAN->msg.ext.msg.rx.ext.id[0] << (5+8+8)) |
							  (pCAN->hwCAN->msg.ext.msg.rx.ext.id[1] << (5+8)) |
							  (pCAN->hwCAN->msg.ext.msg.rx.ext.id[2] << 5) |
							  (pCAN->hwCAN->msg.ext.msg.rx.ext.id[3] >> 3);
					while (ch--)
					{
						msg->data[ch] = pCAN->hwCAN->msg.ext.msg.rx.ext.data[ch];
					}
				}
				else
				{
					/* Standard frame */
					msg->rtr = (ch >> 4) & 0x01;
					msg->len = ch = ch & 0x0F;
					msg->id = (pCAN->hwCAN->msg.ext.msg.rx.std.id[0] << 3) |
							  (pCAN->hwCAN->msg.ext.msg.rx.std.id[1] >> 5);
					while (ch--)
					{
						msg->data[ch] = pCAN->hwCAN->msg.ext.msg.rx.std.data[ch];
					}
				}
			}

			/* Re-Enable RX buffer for a new message
			 */
			pCAN->hwCAN->cmd = can_cmd_releaserxbuf;

			/* make message available to the user
			 */
			can_fifo_put(pCAN->rxfifo);

			pCAN->stats.rx_msgs++;

			/* signal the semaphore only once
			 */
            rx_flag = 1;
		}

		if (iflags & can_intflags_tx)
		{
			/*
             * there is room in tx fifo of HW
			 */
			if (!can_fifo_empty(pCAN->txfifo))
			{
				/* send 1 more messages
				 */
				msg = can_fifo_claim_get(pCAN->txfifo);

				if (LS1x_CAN_send_msg(pCAN, msg))
				{
					/* ERROR! We got an TX interrupt telling us tx fifo is empty,
					 * yet it is not. Complain about this max 10 times
					 */
					if (pCAN->stats.tx_buf_err < 10)
					{
						DBG_OUT("CAN: got TX interrupt but TX fifo in not empty\r\n");
					}

					pCAN->status |= CAN_STATUS_QUEUE_ERROR;
					pCAN->stats.tx_buf_err++;
				}

				/* free software-fifo space taken by sent message
				 */
				can_fifo_get(pCAN->txfifo);

				pCAN->stats.tx_msgs++;

				/* wake any sleeping thread waiting for "fifo not full"
				 */
                tx_flag = 1;
			}
		}

		if (iflags & can_intflags_error)
		{
			tx_error_cnt = pCAN->hwCAN->msg.ext.txerrcnt;
			rx_error_cnt = pCAN->hwCAN->msg.ext.rxerrcnt;

			/* if bus off tx error counter = 127
			 */
			if ((tx_error_cnt > 96) || (rx_error_cnt > 96))
			{
				/* in Error Active Warning area or BUS OFF
				 */
				pCAN->status |= CAN_STATUS_WARN;

				if (pCAN->hwCAN->ctrl & can_ctrl_reset)
				{
					DBG_OUT("CAN stop\r\n");
					pCAN->status |= CAN_STATUS_ERR_BUSOFF | CAN_STATUS_RESET;

					/* stop CAN. turn off interrupts, enter reset mode.
					 */
					pCAN->hwCAN->reg.ext.inten = 0;

					/* User must issue a ioctl(START) to get going again.
					 */
					pCAN->started = false;

					/* signal any waiting read/write threads, so that they
					 * can handle the bus error.
					 */
					rx_flag = 1;
					tx_flag = 1;

					break;
				}
			}
			else
				/* not in Upper Error Active area any more
				 */
				pCAN->status &= ~(CAN_STATUS_WARN);

			pCAN->stats.err_warn++;
		}

		if (iflags & can_intflags_dataoverflow)
		{
			pCAN->status |= CAN_STATUS_OVERRUN;
			pCAN->stats.err_dovr++;
			DBG_OUT("DOVR\r\n");
		}

		if (iflags & can_intflags_errorpassive_ext)
		{
			/* Let the error counters decide what kind of
			 * interrupt it was. In/Out of EPassive area.
			 */
			tx_error_cnt = pCAN->hwCAN->msg.ext.txerrcnt;
			rx_error_cnt = pCAN->hwCAN->msg.ext.rxerrcnt;

			if ((tx_error_cnt > 127) || (rx_error_cnt > 127))
				pCAN->status |= CAN_STATUS_ERR_PASSIVE;
			else
				pCAN->status &= ~(CAN_STATUS_ERR_PASSIVE);

			pCAN->stats.err_errp++;
		}

		if (iflags & can_intflags_arbitratelost_ext)
		{
			arbcode = pCAN->hwCAN->msg.ext.arblost;
			pCAN->stats.err_arb_bitnum[arbcode & can_arblost_mask_ext]++;
			pCAN->stats.err_arb++;
			DBG_OUT("ARB (0x%x)\r\n", arbcode & can_arblost_mask_ext);
		}

		if (iflags & can_intflags_buserror_ext)
		{
			errcode = pCAN->hwCAN->msg.ext.errcode;
			DBG_OUT("buserr %02X\n", errcode);

			/* Some kind of BUS error, only used for statistics.
			 * Error Register is decoded and put into can->stats.
			 */
			switch (errcode & can_errcode_mask_ext)
			{
				case can_errcode_bit_ext:
					pCAN->stats.err_bus_bit++;
					break;
				case can_errcode_form_ext:
					pCAN->stats.err_bus_form++;
					break;
				case can_errcode_stuff_ext:
					pCAN->stats.err_bus_stuff++;
					break;
				case can_errcode_other_ext:
					pCAN->stats.err_bus_other++;
					break;
			}

			/* Get Direction (TX/RX)
			 */
			if (errcode & can_errcode_dir_ext)
				pCAN->stats.err_bus_rx++;
			else
				pCAN->stats.err_bus_tx++;

			pCAN->stats.err_bus_segs[errcode & can_errcode_seg_mask_ext]++;

			pCAN->stats.err_bus++;
		}

		iflags = pCAN->hwCAN->intflags;

		if (pCAN->coremode == CAN_CORE_20A)
			iflags &= 0x1F;

	}	/* End of While. */

	/* signal Binary semaphore, messages available!
	 */
	if (rx_flag)
	{
#if defined(OS_RTTHREAD)
        rt_event_send(pCAN->can_event, CAN_RX_EVENT);
#elif defined(OS_UCOS)
        OSFlagPost(pCAN->can_event,
                   (OS_FLAGS)CAN_RX_EVENT,
                   OS_FLAG_SET,
                   &err);
#elif defined(OS_FREERTOS)
		xResult = xEventGroupSetBitsFromISR(pCAN->can_event,
                                            CAN_RX_EVENT,
                                            &xHigherPriorityTaskWoken);
        if (xResult != pdPASS)  /* Was the message posted successfully? */
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
#endif
	}

	if (tx_flag)
	{
#if defined(OS_RTTHREAD)
        rt_event_send(pCAN->can_event, CAN_TX_EVENT);
#elif defined(OS_UCOS)
        OSFlagPost(pCAN->can_event,
                   (OS_FLAGS)CAN_TX_EVENT,
                   OS_FLAG_SET,
                   &err);
#elif defined(OS_FREERTOS)
		xResult = xEventGroupSetBitsFromISR(pCAN->can_event,
                                            CAN_TX_EVENT,
                                            &xHigherPriorityTaskWoken);
        if (xResult != pdPASS)  /* Was the message posted successfully? */
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
#endif
	}

	LS1x_CAN_interrupt_enable(pCAN);
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * CAN driver operators
 */
static driver_ops_t LS1x_CAN_drv_ops =
{
    .init_entry  = LS1x_CAN_init,
    .open_entry  = LS1x_CAN_open,
    .close_entry = LS1x_CAN_close,
    .read_entry  = LS1x_CAN_read,
    .write_entry = LS1x_CAN_write,
    .ioctl_entry = LS1x_CAN_ioctl,
};

driver_ops_t *ls1x_can_drv_ops = &LS1x_CAN_drv_ops;
#endif

/******************************************************************************
 * for RT-Thread
 */
#if defined(OS_RTTHREAD)
const char *ls1x_can_get_device_name(void *pCAN)
{
    return ((CAN_t *)pCAN)->dev_name;
}
#endif

#endif

/*
 * @@ End
 */
 
 
