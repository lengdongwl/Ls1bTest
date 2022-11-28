/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*-----------------------------------------------------------
 * Implementation of functions defined in port_timer.h
 *----------------------------------------------------------*/
 
#include <stdint.h>

#include "FreeRTOSConfig.h"
#include "port_timer.h"
#include "interrupt.h"

#include "bsp.h"

#include "cpu.h"

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_irq.h"
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_irq.h"
#else
#error "No Loongson1x Soc defined."
#endif

/*
 * In "mips_timer.S"
 */
extern void mips_set_timer(unsigned int timer_clock_interval);
extern unsigned int mips_get_timer(void);

static unsigned int mips_timer_step = 0;        /* tick ÿ���ۼӼ����� */

static int is_tick_started = 0;                 /* tick �Ƿ����� */

//-----------------------------------------------------------------------------

/* Clear the timer interrupt */
void portClearTickTimerInterrupt( void )
{
	mips_set_timer(mips_timer_step);
}

extern void vPortTickInterruptHandler( void );
extern void vPortIncrementTick(int vector, void *arg);

/* This is defined as weak, as it can (theoretically) be redefined by the
   application that we are being built into. */
void vApplicationSetupTickTimerInterrupt( void )
{
    mips_timer_step = LS1x_CPU_FREQUENCY(CPU_XTAL_FREQUENCY) / 2 / configTIMER_RATE_HZ;

	/* Install interrupt handler */
	pvPortInstallISR(LS1x_IRQ_CNT, vPortIncrementTick); // vPortTickInterruptHandler

    /*
     * This will start when schedule
     */
    mips_set_timer(mips_timer_step * 25);

    /*
     * Can't disable DC of LS1B?
     */
    mips_enable_dc();
    
    is_tick_started = 1;
}

/****************************************************************************** 
 * ��ʱ���� 
 */
 
/*
 * ��û�а�װ�ж�֮ǰ, Ҫʹ����ʱ, ��Ҫ��ʼ�� mips_timer_step ��ֵ
 */
void init_tick_step(void)
{
    mips_timer_step = LS1x_CPU_FREQUENCY(CPU_XTAL_FREQUENCY) / 2 / configTIMER_RATE_HZ;
}

void delay_us(unsigned int us)
{
	register unsigned int startVal;
	register unsigned int endVal;
	register unsigned int curVal;
	asm volatile ("mfc0 %0, $9;" : "=&r"(startVal));
	endVal = startVal + us * (mips_timer_step / 1000);
	while (1) 
    {
		asm volatile ("mfc0 %0, $9;" : "=&r"(curVal));
		if (((endVal > startVal) && (curVal >= endVal)) ||
		    ((endVal < startVal) && (curVal < startVal) && (curVal >= endVal)))
			break;
	}
}

extern unsigned int get_clock_ticks(void);

void delay_ms(unsigned int ms)
{
    if (is_tick_started)
    {
    	vTaskDelay(ms);
    }
    else
    {
    	delay_us(ms * 1000);
    }
}


