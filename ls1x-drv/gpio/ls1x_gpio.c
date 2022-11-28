/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_gpio.c
 *
 * created: 2020/11/2
 *  author: Bian
 */

#include "bsp.h"

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_irq.h"
#include "ls1b_gpio.h"
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_irq.h"
#include "ls1c_gpio.h"
#else
#error "No Loongson SoC defined."
#endif

//-------------------------------------------------------------------------------------------------

static unsigned int ls1x_gpio_intcbase(int gpio)
{
#if defined(LS1B)
	if ((gpio >= 0) && (gpio <= 30))
		return LS1B_INTC2_BASE;
	else if ((gpio >= 32) && (gpio <= 61))
		return LS1B_INTC3_BASE;
#elif defined(LS1C)
	if ((gpio >= 0) && (gpio <= 31))
		return LS1C_INTC2_BASE;
	else if ((gpio >= 32) && (gpio <= 63))
		return LS1C_INTC3_BASE;
	else if ((gpio >= 64) && (gpio <= 95))
		return LS1C_INTC4_BASE;
	else if ((gpio >= 96) && (gpio <= 105))
		return LS1C_INTC1_BASE;
#endif
    else
        return 0;
}

static unsigned int ls1x_gpio_intcbit(int gpio)
{
#if defined(LS1B)
	if ((gpio >= 0) && (gpio <= 30))
		return (1 << gpio);
	else if ((gpio >= 32) && (gpio <= 61))
		return (1 << (gpio - 32));
#elif defined(LS1C)
	if ((gpio >= 0) && (gpio <= 31))
		return (1 << gpio);
	else if ((gpio >= 32) && (gpio <= 63))
		return (1 << (gpio - 32));
	else if ((gpio >= 64) && (gpio <= 95))
		return (1 << (gpio - 64));
	else if ((gpio >= 96) && (gpio <= 105))
		return (1 << (gpio - 96 + 22));
#endif
    else
        return 0;
}

static unsigned int ls1x_gpio_irqnum(int gpio)
{
#if defined(LS1B)
	if ((gpio >= 0) && (gpio <= 30))
        return LS1B_IRQ2_BASE + gpio;
	else if ((gpio >= 32) && (gpio <= 61))
        return LS1B_IRQ3_BASE + gpio - 32;
#elif defined(LS1C)
	if ((gpio >= 0) && (gpio <= 31))
        return LS1C_IRQ2_BASE + gpio;
	else if ((gpio >= 32) && (gpio <= 63))
        return LS1C_IRQ3_BASE + gpio - 32;
	else if ((gpio >= 64) && (gpio <= 95))
        return LS1C_IRQ4_BASE + gpio - 64;
	else if ((gpio >= 96) && (gpio <= 105))
        return LS1C_IRQ1_BASE + gpio - 96 + 22;
#endif
    else
        return 0;
}

//-------------------------------------------------------------------------------------------------

int ls1x_enable_gpio_interrupt(int gpio)
{
    unsigned int intc_base, intc_bit;
    
    intc_base = ls1x_gpio_intcbase(gpio);
    intc_bit  = ls1x_gpio_intcbit(gpio);
    
    if ((intc_base > 0) && (intc_bit > 0))
    {
		LS1x_INTC_CLR(intc_base)  = intc_bit;
		LS1x_INTC_IEN(intc_base) |= intc_bit;
		return 0;
    }
    
    return -1;
}

int ls1x_disable_gpio_interrupt(int gpio)
{
    unsigned int intc_base, intc_bit;

    intc_base = ls1x_gpio_intcbase(gpio);
    intc_bit  = ls1x_gpio_intcbit(gpio);

    if ((intc_base > 0) && (intc_bit > 0))
    {
        LS1x_INTC_IEN(intc_base) &= ~intc_bit;
		LS1x_INTC_CLR(intc_base)  =  intc_bit;
		return 0;
    }
    
    return -1;
}

int ls1x_install_gpio_isr(int gpio, int trigger_mode, void (*isr)(int, void *), void *arg)
{
    unsigned int intc_base, intc_bit, irq_num;

    intc_base = ls1x_gpio_intcbase(gpio);
    intc_bit  = ls1x_gpio_intcbit(gpio);
    irq_num   = ls1x_gpio_irqnum(gpio);

    if ((intc_base > 0) && (intc_bit > 0) && (irq_num > 0))
    {
        /* set as gpio in */
        gpio_enable(gpio, DIR_IN);
        
        /* disable interrupt first */
        LS1x_INTC_IEN(intc_base) &= ~intc_bit;
		LS1x_INTC_CLR(intc_base)  =  intc_bit;
		
		/* set interrupt trigger mode */
		switch (trigger_mode)
		{
		    case INT_TRIG_LEVEL_LOW:
	            LS1x_INTC_EDGE(intc_base) &= ~intc_bit;     // level
	            LS1x_INTC_POL(intc_base)  &= ~intc_bit;     // low
                break;

		    case INT_TRIG_LEVEL_HIGH:
	            LS1x_INTC_EDGE(intc_base) &= ~intc_bit;     // level
	            LS1x_INTC_POL(intc_base)  |=  intc_bit;     // high
                break;

		    case INT_TRIG_EDGE_DOWN:
	            LS1x_INTC_EDGE(intc_base) |=  intc_bit;     // edge
	            LS1x_INTC_POL(intc_base)  &= ~intc_bit;     // down
                break;

		    case INT_TRIG_EDGE_UP:
		    default:
	            LS1x_INTC_EDGE(intc_base) |= intc_bit;      // edge
	            LS1x_INTC_POL(intc_base)  |= intc_bit;      // up
                break;
	    }

	    /*
         * install the isr
         */
        ls1x_install_irq_handler(irq_num, isr, arg);

        /* enable interrupt finally */
		LS1x_INTC_CLR(intc_base)  = intc_bit;
		LS1x_INTC_IEN(intc_base) |= intc_bit;
		
		return 0;
    }
    
    return -1;
}

int ls1x_remove_gpio_isr(int gpio)
{
    unsigned intc_base, intc_bit, irq_num;

    intc_base = ls1x_gpio_intcbase(gpio);
    intc_bit  = ls1x_gpio_intcbit(gpio);
    irq_num   = ls1x_gpio_irqnum(gpio);

    if ((intc_base > 0) && (intc_bit > 0) && (irq_num > 0))
    {
        /* disable interrupt */
        LS1x_INTC_IEN(intc_base) &= ~intc_bit;
		LS1x_INTC_CLR(intc_base)  =  intc_bit;
		
	    /*
         * uninstall the isr
         */
        ls1x_remove_irq_handler(irq_num);

		return 0;
    }

    return -1;
}


