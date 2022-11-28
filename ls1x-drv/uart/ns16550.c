/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ns16550.c, 2013/04/09 Bian
 *
 * NOTE: console usage
 */

#include "bsp.h"

#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "mips.h"

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_irq.h"
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_irq.h"
#else
#error "No Loongson1x SoC defined."
#endif

#include "ls1x_io.h"
#include "termios.h"

#include "ns16550_p.h"
#include "ns16550.h"

//-----------------------------------------------------------------------------

#define NS16550_SUPPORT_INT   0         /* use interrupt or not */

#if (NS16550_SUPPORT_INT)

#define UART_BUF_SIZE   512

typedef struct
{
    char  Buf[UART_BUF_SIZE];
    int   Count;
    char *pHead;
    char *pTail;
} NS16550_buf_t;

#endif

//-----------------------------------------------------------------------------

typedef struct NS16550
{
    unsigned int   BusClock;
    unsigned int   BaudRate;
    unsigned int   CtrlPort;
    unsigned int   DataPort;
    bool           bFlowCtrl;
    unsigned char  ModemCtrl;

    /* Interrupt Support */
    bool           bIntrrupt;
    unsigned int   IntrNum;
    unsigned int   IntrCtrl;
    unsigned int   IntrMask;

#if (NS16550_SUPPORT_INT)
    /* RX/TX Buffer */
    NS16550_buf_t  RxData;
    NS16550_buf_t  TxData;
#endif

    int            initialized;
    int            opened;
    char           dev_name[16];
} NS16550_t;

//-----------------------------------------------------------------------------

#if (NS16550_SUPPORT_INT)

static int NS16550_write_string_int(NS16550_t *pUART, char *buf, int len);

//-----------------------------------------------------------------------------
// buffer: cycle mode, drop the most oldest data when add
//-----------------------------------------------------------------------------

/*
 * 保存 UART_BUF_SIZE 个字符.
 *
 * 字节: 0  1  2  3  4  5        ...
 *       __ __ xx xx xx xx __ __ ...
 *             ^           ^
 *             pHead       |
 *                         pTail
 *
 * if full or empty: pHead==pTail;
 */

static void initialize_buffer(NS16550_buf_t *data)
{
    data->Count = 0;
    data->pHead = data->pTail = data->Buf;
}

/*
 * If oveflow, overwrite always
 */
static int enqueue_to_buffer(NS16550_t *pUART, NS16550_buf_t *data, char *buf, int len)
{
    int i;

    for (i=0; i<len; i++)
    {
        *data->pTail = buf[i];
        data->Count++;
        data->pTail++;
        if (data->pTail >= data->Buf + UART_BUF_SIZE)
            data->pTail = data->Buf;
    }

    /*
     * if overflow, override the lastest data
     */
    if (data->Count > UART_BUF_SIZE)    // overflow
    {
        data->Count = UART_BUF_SIZE;
        data->pHead = data->pTail;
    }

    return len;
}

static int dequeue_from_buffer(NS16550_t *pUART, NS16550_buf_t *data, char *buf, int len)
{
    int i, count;

    count = len < data->Count ? len : data->Count;

    for (i=0; i<count; i++)
    {
        buf[i] = *data->pHead;
        data->Count--;
        data->pHead++;
        if (data->pHead >= data->Buf + UART_BUF_SIZE)
            data->pHead = data->Buf;
    }

    return count;
}

#endif

//-----------------------------------------------------------------------------
// access ns16550 register
//-----------------------------------------------------------------------------

typedef struct uart_reg
{
	volatile unsigned char reg;
} uartReg;

static unsigned char NS16550_get_r(unsigned CtrlPort, unsigned char RegNum)
{
	struct uart_reg *p = (struct uart_reg *)CtrlPort;
	unsigned char ch = p[RegNum].reg;

	return ch;
}

static void NS16550_set_r(unsigned CtrlPort, unsigned char RegNum, unsigned char ch)
{
	struct uart_reg *p = (struct uart_reg *)CtrlPort;
	p[RegNum].reg = ch;
}

//-----------------------------------------------------------------------------
// interrupt
//-----------------------------------------------------------------------------

/*
 * This routine initializes the port to have the specified interrupts masked.
 */
static void NS16550_set_interrupt(NS16550_t *pUART, int mask)
{
#if (NS16550_SUPPORT_INT)
    NS16550_set_r(pUART->CtrlPort, NS16550_INTERRUPT_ENABLE, mask);
#else
    NS16550_set_r(pUART->CtrlPort, NS16550_INTERRUPT_ENABLE, 0);
#endif
}

#if (NS16550_SUPPORT_INT)

/******************************************************************************
 * Process interrupt.
 */
static void NS16550_interrupt_process(NS16550_t *pUART)
{
    int i, status, count = 0;
    char buf[SP_FIFO_SIZE+1];

    /*
     * Iterate until no more interrupts are pending
     */
    do
    {
        status = (int)NS16550_get_r(pUART->CtrlPort, NS16550_INTERRUPT_ID);

        /*
         * 收到数据
         */
        if (status & SP_INTID_RXRDY)
        {
            /* Fetch received characters */
            for (i=0; i<SP_FIFO_SIZE; ++i)
            {
                if ((NS16550_get_r(pUART->CtrlPort, NS16550_LINE_STATUS) & SP_LSR_RDY) != 0)
                {
                    buf[i] = (char)NS16550_get_r(pUART->CtrlPort, NS16550_RECEIVE_BUFFER);
                }
                else
                    break;
            }

            /*
             * Enqueue fetched characters to buffer
             */
            enqueue_to_buffer(pUART, &pUART->RxData, buf, i);
        }

        /* check if we need transmit characters go on
         */
        if ((pUART->TxData.Count > 0) &&
            (NS16550_get_r(pUART->CtrlPort, NS16550_LINE_STATUS) & SP_LSR_THOLD))
        {
            /* Dequeue transmitted characters from buffer
             */
            count = dequeue_from_buffer(pUART, &pUART->TxData, buf, SP_FIFO_SIZE);

            for (i=0; i<count; ++i)
            {
                NS16550_set_r(pUART->CtrlPort, NS16550_TRANSMIT_BUFFER, buf[i]);
            }
        }

        if (count > 0)
            NS16550_set_interrupt(pUART, NS16550_ENABLE_ALL_INTR);
        else
            NS16550_set_interrupt(pUART, NS16550_ENABLE_ALL_INTR_EXCEPT_TX);

    } while ((status & SP_INTID_NOPEND) == 0);
}

static void NS16550_interrupt_handler(int vector, void *arg)
{
    if (NULL != arg)
    {
        NS16550_t *pUART = (NS16550_t *)arg;

        LS1x_INTC_IEN(pUART->IntrCtrl) &= ~pUART->IntrMask;     /* 关中断 */

        NS16550_interrupt_process(pUART);                       /* 中断处理 */

        LS1x_INTC_CLR(pUART->IntrCtrl)  =  pUART->IntrMask;
        LS1x_INTC_IEN(pUART->IntrCtrl) |=  pUART->IntrMask;     /* 开中断 */
    }
}

/*
 *  This routine install the interrupt handler.
 */
static void NS16550_install_irq(NS16550_t *pUART)
{
    if (pUART->bIntrrupt)
    {
        ls1x_install_irq_handler(pUART->IntrNum, NS16550_interrupt_handler, pUART);

        LS1x_INTC_EDGE(pUART->IntrCtrl) &= ~pUART->IntrMask;
        LS1x_INTC_POL( pUART->IntrCtrl) |=  pUART->IntrMask;
        LS1x_INTC_CLR( pUART->IntrCtrl)  =  pUART->IntrMask;
        LS1x_INTC_IEN( pUART->IntrCtrl) |=  pUART->IntrMask;

        NS16550_set_interrupt(pUART, NS16550_DISABLE_ALL_INTR);
    }
}

static void NS16550_remove_irq(NS16550_t *pUART)
{
    if (pUART->bIntrrupt)
    {
        NS16550_set_interrupt(pUART, NS16550_DISABLE_ALL_INTR);

        LS1x_INTC_IEN(pUART->IntrCtrl) &= ~pUART->IntrMask;
        LS1x_INTC_CLR(pUART->IntrCtrl)  =  pUART->IntrMask;

        ls1x_remove_irq_handler(pUART->IntrNum);
    }
}

#endif

/******************************************************************************
 * These routines provide control of the RTS and DTR lines
 ******************************************************************************/

/*
 * NS16550_assert_RTS
 */
static int NS16550_assert_RTS(NS16550_t *pUART)
{
    mips_interrupt_disable();
    pUART->ModemCtrl |= SP_MODEM_RTS;
    NS16550_set_r(pUART->CtrlPort, NS16550_MODEM_CONTROL, pUART->ModemCtrl);
    mips_interrupt_enable();
    return 0;
}

/*
 * NS16550_negate_RTS
 */
static int NS16550_negate_RTS(NS16550_t *pUART)
{
    mips_interrupt_disable();
    pUART->ModemCtrl &= ~SP_MODEM_RTS;
    NS16550_set_r(pUART->CtrlPort, NS16550_MODEM_CONTROL, pUART->ModemCtrl);
    mips_interrupt_enable();
    return 0;
}

/******************************************************************************
 * These flow control routines utilise a connection from the local DTR
 * line to the remote CTS line
 ******************************************************************************/

/*
 * NS16550_assert_DTR
 */
static int NS16550_assert_DTR(NS16550_t *pUART)
{
    mips_interrupt_disable();
    pUART->ModemCtrl |= SP_MODEM_DTR;
    NS16550_set_r(pUART->CtrlPort, NS16550_MODEM_CONTROL, pUART->ModemCtrl);
    mips_interrupt_enable();
    return 0;
}

/*
 * NS16550_negate_DTR
 */
static int NS16550_negate_DTR(NS16550_t *pUART)
{
    mips_interrupt_disable();
    pUART->ModemCtrl &= ~SP_MODEM_DTR;
    NS16550_set_r(pUART->CtrlPort, NS16550_MODEM_CONTROL, pUART->ModemCtrl);
    mips_interrupt_enable();
    return 0;
}

/******************************************************************************
 * NS16550_set_attributes
 */
static int NS16550_set_attributes(NS16550_t *pUART, const struct termios *t)
{
    unsigned int divisor, baud_requested, irq_mask;
    unsigned char lineCtrl, trash;

    /* Calculate the baud rate divisor */
    baud_requested = t->c_cflag & CBAUD;
    if (!baud_requested)
        baud_requested = B9600;              /* default to 9600 baud */

    baud_requested = CFLAG_TO_BAUDRATE(baud_requested);
    divisor = NS16550_BAUD_DIV(pUART->BusClock, baud_requested);
    lineCtrl = 0;

    /* Parity */
    if (t->c_cflag & PARENB)
    {
        lineCtrl |= SP_LINE_PAR;
        if (!(t->c_cflag & PARODD))
            lineCtrl |= SP_LINE_ODD;
    }

    /* Character Size */
    if (t->c_cflag & CSIZE)
    {
        switch (t->c_cflag & CSIZE)
        {
            case CS5:  lineCtrl |= CFCR_5_BITS; break;
            case CS6:  lineCtrl |= CFCR_6_BITS; break;
            case CS7:  lineCtrl |= CFCR_7_BITS; break;
            case CS8:  lineCtrl |= CFCR_8_BITS; break;
        }
    }
    else
    {
        lineCtrl |= CFCR_8_BITS;               /* default to 9600,8,N,1 */
    }

    /* Stop Bits */
    if (t->c_cflag & CSTOPB)
        lineCtrl |= SP_LINE_STOP;              /* 2 stop bits */

    /*
     * Now actually set the chip
     */
    // mips_interrupt_disable();

    /* Save port interrupt mask */
    irq_mask = NS16550_get_r(pUART->CtrlPort, NS16550_INTERRUPT_ENABLE);

    /* Set the baud rate */
    NS16550_set_r(pUART->CtrlPort, NS16550_LINE_CONTROL, SP_LINE_DLAB);

    /* XXX are these registers right? */
    NS16550_set_r(pUART->CtrlPort, NS16550_DIVISIORLATCH_LSB, divisor & 0xFF);
    NS16550_set_r(pUART->CtrlPort, NS16550_DIVISIORLATCH_MSB, (divisor >> 8) & 0xFF);

    /* Now write the line control */
    NS16550_set_r(pUART->CtrlPort, NS16550_LINE_CONTROL, lineCtrl);

    /* Restore port interrupt mask */
    NS16550_set_r(pUART->CtrlPort, NS16550_INTERRUPT_ENABLE, irq_mask);

    trash = NS16550_get_r(pUART->CtrlPort, NS16550_LINE_STATUS);
    trash = NS16550_get_r(pUART->CtrlPort, NS16550_RECEIVE_BUFFER);
    trash = NS16550_get_r(pUART->CtrlPort, NS16550_MODEM_STATUS);

    // mips_interrupt_enable();

    return 0;
}

/******************************************************************************
 * NS16550_init
 */
STATIC_DRV int NS16550_init(void *dev, void *arg)
{
    unsigned int divisor;
    unsigned char trash;
    NS16550_t *pUART = (NS16550_t *)dev;

    if (dev == NULL)
        return -1;

    if (pUART->initialized)
        return 0;

    pUART->BusClock = LS1x_BUS_FREQUENCY(CPU_XTAL_FREQUENCY);
    if (arg != NULL)
    {
        pUART->BaudRate = (unsigned int)arg;
    }

    divisor = NS16550_BAUD_DIV(pUART->BusClock, pUART->BaudRate);

    /* Clear the divisor latch, clear all interrupt enables,
     * and reset and disable the FIFO's. */
    NS16550_set_r(pUART->CtrlPort, NS16550_LINE_CONTROL, 0);
    NS16550_set_interrupt(pUART, NS16550_DISABLE_ALL_INTR);

    /* Set the divisor latch and set the baud rate. */
    NS16550_set_r(pUART->CtrlPort, NS16550_LINE_CONTROL, SP_LINE_DLAB);

    NS16550_set_r(pUART->CtrlPort, NS16550_DIVISIORLATCH_LSB, (unsigned char)(divisor & 0xFFU));
    NS16550_set_r(pUART->CtrlPort, NS16550_DIVISIORLATCH_MSB, (unsigned char)((divisor >> 8) & 0xFFU));

    /* Clear the divisor latch and set the character size to eight bits
     * with one stop bit and no parity checking. 8.N.1 */
    NS16550_set_r(pUART->CtrlPort, NS16550_LINE_CONTROL, CFCR_8_BITS);

    /* Enable and reset transmit and receive FIFOs. */
    NS16550_set_r(pUART->CtrlPort, NS16550_FIFO_CONTROL, SP_FIFO_ENABLE);
    NS16550_set_r(pUART->CtrlPort, NS16550_FIFO_CONTROL,
                  SP_FIFO_ENABLE | SP_FIFO_RXRST | SP_FIFO_TXRST /*| SP_FIFO_TRIGGER_1*/);
    NS16550_set_interrupt(pUART, NS16550_DISABLE_ALL_INTR);

    /* Set data terminal ready. */
    /* And open interrupt tristate line */
    pUART->ModemCtrl = SP_MODEM_IRQ;
    NS16550_set_r(pUART->CtrlPort, NS16550_MODEM_CONTROL, SP_MODEM_IRQ);

    trash = NS16550_get_r(pUART->CtrlPort, NS16550_LINE_STATUS);
    trash = NS16550_get_r(pUART->CtrlPort, NS16550_RECEIVE_BUFFER);
    trash = NS16550_get_r(pUART->CtrlPort, NS16550_MODEM_STATUS);

#if (NS16550_SUPPORT_INT)
    /* Disable All Port Interrupt Mode */
    /*
     * XXX 控制台中断模式会影响调试
     */
    if (pUART == ConsolePort)
        pUART->bIntrrupt = false;
#endif

    pUART->initialized = 1;
    return 0;
}

/******************************************************************************
 * NS16550_open
 */
STATIC_DRV int NS16550_open(void *dev, void *arg)
{
    NS16550_t *pUART = (NS16550_t *)dev;
    struct termios *t = (struct termios *)arg;

    if (dev == NULL)
        return -1;

    if (pUART->opened)
        return 0;

    /* Assert DTR */
    if (pUART->bFlowCtrl)
        NS16550_assert_DTR(pUART);

    /* Set initial baud */
    if (t != NULL)
        NS16550_set_attributes(pUART, t);

#if (NS16550_SUPPORT_INT)
    initialize_buffer(&pUART->RxData);
    initialize_buffer(&pUART->TxData);

    if (pUART->bIntrrupt)
    {
        NS16550_install_irq(pUART);
        NS16550_set_interrupt(pUART, NS16550_ENABLE_ALL_INTR_EXCEPT_TX);
    }
#endif

    pUART->opened = 1;
    return 0;
}

/******************************************************************************
 * NS16550_close
 */
STATIC_DRV int NS16550_close(void *dev, void *arg)
{
    NS16550_t *pUART = (NS16550_t *)dev;

    if (dev == NULL)
        return -1;

    /* Negate DTR */
    if (pUART->bFlowCtrl)
        NS16550_negate_DTR(pUART);

    NS16550_set_interrupt(pUART, NS16550_DISABLE_ALL_INTR);

#if (NS16550_SUPPORT_INT)
    if (pUART->bIntrrupt)
    {
        NS16550_remove_irq(pUART);
    }
#endif

    pUART->opened = 0;
    return 0;
}

/*
 * Polled write for NS16550.
 */
static void NS16550_output_char_polled(NS16550_t *pUART, char ch)
{
    unsigned char status = 0;

    /* Save port interrupt mask */
    unsigned char irq_mask = NS16550_get_r(pUART->CtrlPort, NS16550_INTERRUPT_ENABLE);

    /* Disable port interrupts */
    NS16550_set_interrupt(pUART, NS16550_DISABLE_ALL_INTR);

    while (true)
    {
        /* Try to transmit the character in a critical section */
        mips_interrupt_disable();

        /* Read the transmitter holding register and check it */
        status = NS16550_get_r(pUART->CtrlPort, NS16550_LINE_STATUS);
        if ((status & SP_LSR_THOLD) != 0)
        {
            /* Transmit character */
            NS16550_set_r(pUART->CtrlPort, NS16550_TRANSMIT_BUFFER, ch);
            /* Finished */
            mips_interrupt_enable();
            break;
        }
        else
        {
            mips_interrupt_enable();
        }

        /* Wait for transmitter holding register to be empty
         * FIXME add timeout about 2ms, one byte transfer at 4800bps
         */

        do
        {
            status = NS16550_get_r(pUART->CtrlPort, NS16550_LINE_STATUS);
        } while ((status & SP_LSR_THOLD) == 0);
    }

    /* Restore port interrupt mask */
    NS16550_set_r(pUART->CtrlPort, NS16550_INTERRUPT_ENABLE, irq_mask);
}

#if (NS16550_SUPPORT_INT)
/*
 * Interrupt write for NS16550.
 */
static int NS16550_write_string_int(NS16550_t *pUART, char *buf, int len)
{
    int i, sent = 0;

    /* if idle, send immediately
     */
    if (NS16550_get_r(pUART->CtrlPort, NS16550_LINE_STATUS) & SP_LSR_THOLD)
    {
        sent = len <= SP_FIFO_SIZE ? len : SP_FIFO_SIZE;

        /* write data to transmit buffer */
        for (i=0; i<sent; ++i)
        {
            NS16550_set_r(pUART->CtrlPort, NS16550_TRANSMIT_BUFFER, buf[i]);
        }

        if (sent > 0)
            NS16550_set_interrupt(pUART, NS16550_ENABLE_ALL_INTR);
        else
            NS16550_set_interrupt(pUART, NS16550_ENABLE_ALL_INTR_EXCEPT_TX);
    }

    /* add remain data to transmit cached buffer
     */
    if (sent < len)
    {
        mips_interrupt_disable();
        sent += enqueue_to_buffer(pUART, &pUART->TxData, buf + sent, len - sent);
        mips_interrupt_enable();
    }

    return sent;
}
#endif

/*
 * Polled write string
 */
static int NS16550_write_string_polled(NS16550_t *pUART, char *buf, int len)
{
    int nwrite = 0;

    /*
     * poll each byte in the string out of the port.
     */
    while (nwrite < len)
    {
        /* transmit character */
        NS16550_output_char_polled(pUART, *buf++);
        nwrite++;
    }

    /* return the number of bytes written. */
    return nwrite;
}

/*
 * Polled get char nonblocking
 */
static int NS16550_inbyte_nonblocking_polled(NS16550_t *pUART)
{
    unsigned char status, ch;

    status = NS16550_get_r(pUART->CtrlPort, NS16550_LINE_STATUS);

    if (status & SP_LSR_RDY)
    {
        ch = NS16550_get_r(pUART->CtrlPort, NS16550_RECEIVE_BUFFER);
        return (int)ch;
    }

    return -1;
}

/*
 * Polled get char blocking
 */
static int NS16550_inbyte_blocking_polled(NS16550_t *pUART)
{
    unsigned char status, ch = 0xFF;

    for ( ; ; )
    {
        status = NS16550_get_r(pUART->CtrlPort, NS16550_LINE_STATUS);
        if (status & SP_LSR_RDY)
        {
            ch = NS16550_get_r(pUART->CtrlPort, NS16550_RECEIVE_BUFFER);
            break;
        }

        /* XXX if use RTOS, delay with RTOS func to throw-out time? */
#if BSP_USE_OS
        delay_ms(1);
#else
        delay_us(100);
#endif
    }

    return (int)ch;
}

/******************************************************************************
 * NS16550 read
 */
STATIC_DRV int NS16550_read(void *dev, void *buf, int size, void *arg)
{
    NS16550_t *pUART = (NS16550_t *)dev;
    char *pchbuf = (char *)buf;
    int timeout = (int)arg;

    if ((dev == NULL) || (buf == NULL))
        return -1;

#if (NS16550_SUPPORT_INT)
    if (pUART->bIntrrupt)                   /* read from buffer */
    {
        int count;
        pchbuf[0] = 0xFF;                   /* avoid read nothing */

        mips_interrupt_disable();
        count = dequeue_from_buffer(pUART, &pUART->RxData, (char *)pchbuf, size);
        mips_interrupt_enable();

        /*
         * time out read
         */
        while ((count < size) && (timeout-- > 0))
        {
            delay_ms(1);                    /* RTOS will give-up time */
            pchbuf = (char *)buf + count;

            mips_interrupt_disable();
            count += dequeue_from_buffer(pUART, &pUART->RxData, (char *)pchbuf, size - count);
            mips_interrupt_enable();
        }

        return count;
    }
    else
#endif
    if (timeout == 0)                       /* return immediately */
    {
        int i=0, val;
        for (i=0; i<size; i++)
        {
            val = NS16550_inbyte_nonblocking_polled(pUART);
            pchbuf[i] = (char)val;
            if (val == -1)
            {
                return i;                   /* can return 0 bytes */
            }
        }
    }
    else // if (timeout != 0)               /* blocking read */
    {
        int i=0, val;
        for (i=0; i<size; i++)
        {
            val = NS16550_inbyte_blocking_polled(pUART);
            pchbuf[i] = (char)val;
        }
    }

    return size;
}

/******************************************************************************
 * NS16550 write
 */
STATIC_DRV int NS16550_write(void *dev, void *buf, int size, void *arg)
{
    NS16550_t *pUART = (NS16550_t *)dev;
    int count;

    if ((dev == NULL) || (buf == NULL))
        return -1;

#if (NS16550_SUPPORT_INT)
    if (pUART->bIntrrupt)
        count = NS16550_write_string_int(pUART, (char *)buf, size);
    else
#endif
        count = NS16550_write_string_polled(pUART, (char *)buf, size);

    return count;
}

/******************************************************************************
 * NS16550 control
 */
STATIC_DRV int NS16550_ioctl(void *dev, int cmd, void *arg)
{
    NS16550_t *pUART = (NS16550_t *)dev;

    if (dev == NULL)
        return -1;

    switch (cmd)
    {
        case IOCTL_NS16550_SET_MODE:
            /* Set initial baud */
            if (arg != NULL)
                NS16550_set_attributes(pUART, (struct termios *)arg);
            break;

        case IOCTL_NS16550_USE_IRQ:
            if (arg != NULL)
                *(int *)arg = (int)pUART->bIntrrupt;
            break;

        default:
            break;
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Console Support
//-----------------------------------------------------------------------------

char Console_get_char(void *pUART)
{
    return (char)NS16550_inbyte_nonblocking_polled((NS16550_t *)pUART);
}

void Console_output_char(void *pUART, char ch)
{
    NS16550_output_char_polled((NS16550_t *)pUART, ch);
}

//-----------------------------------------------------------------------------
// UART devices
//-----------------------------------------------------------------------------

/***********************************************************************
 * Loongson 1B chip
 */
#if defined(LS1B)

/* UART 2 */
#ifdef BSP_USE_UART2
static NS16550_t ls1b_UART2 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1B_UART2_BASE,
    .DataPort  = LS1B_UART2_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1B_UART2_IRQ,
    .IntrCtrl  = LS1B_INTC0_BASE,
    .IntrMask  = INTC0_UART2_BIT,
    .dev_name  = "uart2",
};
void *devUART2 = &ls1b_UART2;
#endif

/* UART 3 */
#ifdef BSP_USE_UART3
static NS16550_t ls1b_UART3 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1B_UART3_BASE,
    .DataPort  = LS1B_UART3_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1B_UART3_IRQ,
    .IntrCtrl  = LS1B_INTC0_BASE,
    .IntrMask  = INTC0_UART3_BIT,
    .dev_name  = "uart3",
};
void *devUART3 = &ls1b_UART3;
#endif

/* UART 4 */
#ifdef BSP_USE_UART4
static NS16550_t ls1b_UART4 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1B_UART4_BASE,
    .DataPort  = LS1B_UART4_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1B_UART4_IRQ,
    .IntrCtrl  = LS1B_INTC0_BASE,
    .IntrMask  = INTC0_UART4_BIT,
    .dev_name  = "uart4",
};
void *devUART4 = &ls1b_UART4;
#endif

/* UART 5 */
#ifdef BSP_USE_UART5
static NS16550_t ls1b_UART5 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1B_UART5_BASE,
    .DataPort  = LS1B_UART5_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1B_UART5_IRQ,
    .IntrCtrl  = LS1B_INTC0_BASE,
    .IntrMask  = INTC0_UART5_BIT,
    .dev_name  = "uart5",
};
void *devUART5 = &ls1b_UART5;
#endif

/* UART 0 */
#ifdef BSP_USE_UART0
static NS16550_t ls1b_UART0 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1B_UART0_BASE,
    .DataPort  = LS1B_UART0_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1B_UART0_IRQ,
    .IntrCtrl  = LS1B_INTC0_BASE,
    .IntrMask  = INTC0_UART0_BIT,
    .dev_name  = "uart0",
};
void *devUART0 = &ls1b_UART0;
#endif

/* UART 01 */
#ifdef BSP_USE_UART01
static NS16550_t ls1b_UART01 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1B_UART0_BASE+0x1000,
    .DataPort  = LS1B_UART0_BASE+0x1000,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = false,             // No interrupt entry
    .IntrNum   = 0,
    .IntrCtrl  = 0,
    .IntrMask  = 0,
    .dev_name  = "uart01",
};
void *devUART01 = &ls1b_UART01;
#endif

/* UART 02 */
#ifdef BSP_USE_UART02
static NS16550_t ls1b_UART02 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1B_UART0_BASE+0x2000,
    .DataPort  = LS1B_UART0_BASE+0x2000,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = false,             // No interrupt entry
    .IntrNum   = 0,
    .IntrCtrl  = 0,
    .IntrMask  = 0,
    .dev_name  = "uart02",
};
void *devUART02 = &ls1b_UART02;
#endif

/* UART 03 */
#ifdef BSP_USE_UART03
static NS16550_t ls1b_UART03 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1B_UART0_BASE+0x3000,
    .DataPort  = LS1B_UART0_BASE+0x3000,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = false,             // No interrupt entry
    .IntrNum   = 0,
    .IntrCtrl  = 0,
    .IntrMask  = 0,
    .dev_name  = "uart03",
};
void *devUART03 = &ls1b_UART03;
#endif

/* UART 1 */
#ifdef BSP_USE_UART1
static NS16550_t ls1b_UART1 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1B_UART1_BASE,
    .DataPort  = LS1B_UART1_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1B_UART1_IRQ,
    .IntrCtrl  = LS1B_INTC0_BASE,
    .IntrMask  = INTC0_UART1_BIT,
    .dev_name  = "uart1",
};
void *devUART1 = &ls1b_UART1;
#endif

/* UART 11 */
#ifdef BSP_USE_UART11
static NS16550_t ls1b_UART11 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1B_UART1_BASE+0x1000,
    .DataPort  = LS1B_UART1_BASE+0x1000,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = false,             // No interrupt entry
    .IntrNum   = 0,
    .IntrCtrl  = 0,
    .IntrMask  = 0,
    .dev_name  = "uart11",
};
void *devUART11 = &ls1b_UART11;
#endif

/* UART 12 */
#ifdef BSP_USE_UART12
static NS16550_t ls1b_UART12 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1B_UART1_BASE+0x2000,
    .DataPort  = LS1B_UART1_BASE+0x2000,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = false,             // No interrupt entry
    .IntrNum   = 0,
    .IntrCtrl  = 0,
    .IntrMask  = 0,
    .dev_name  = "uart12",
};
void *devUART12 = &ls1b_UART12;
#endif

/* UART 13 */
#ifdef BSP_USE_UART13
static NS16550_t ls1b_UART13 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1B_UART1_BASE+0x3000,
    .DataPort  = LS1B_UART1_BASE+0x3000,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = false,             // No interrupt entry
    .IntrNum   = 0,
    .IntrCtrl  = 0,
    .IntrMask  = 0,
    .dev_name  = "uart13",
};
void *devUART13 = &ls1b_UART13;
#endif

/***********************************************************************
 * Loongson 1C chip
 */
#elif defined(LS1C)

/* UART 0 */
#ifdef BSP_USE_UART0
static NS16550_t ls1c_UART0 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1C_UART0_BASE,
    .DataPort  = LS1C_UART0_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1C_IRQ_UART0,
    .IntrCtrl  = LS1C_INTC0_BASE,
    .IntrMask  = INTC0_IRQ_UART0,
    .dev_name  = "uart0",
};
void *devUART0 = &ls1c_UART0;
#endif

/* UART 1 */
#ifdef BSP_USE_UART1
static NS16550_t ls1c_UART1 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1C_UART1_BASE,
    .DataPort  = LS1C_UART1_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1C_UART1_IRQ,
    .IntrCtrl  = LS1C_INTC0_BASE,
    .IntrMask  = INTC0_UART1_BIT,
    .dev_name  = "uart1",
};
void *devUART1 = &ls1c_UART1;
#endif

/* UART 2 */
#ifdef BSP_USE_UART2
static NS16550_t ls1c_UART2 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1C_UART2_BASE,
    .DataPort  = LS1C_UART2_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1C_UART2_IRQ,
    .IntrCtrl  = LS1C_INTC0_BASE,
    .IntrMask  = INTC0_UART2_BIT,
    .dev_name  = "uart2",
};
void *devUART2 = &ls1c_UART2;
#endif

/* UART 3 */
#ifdef BSP_USE_UART3
static NS16550_t ls1c_UART3 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1C_UART3_BASE,
    .DataPort  = LS1C_UART3_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1C_UART3_IRQ,
    .IntrCtrl  = LS1C_INTC0_BASE,
    .IntrMask  = INTC0_UART3_BIT,
    .dev_name  = "uart3",
};
void *devUART3 = &ls1c_UART3;
#endif

/* UART 4 */
#ifdef BSP_USE_UART4
static NS16550_t ls1c_UART4 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1C_UART4_BASE,
    .DataPort  = LS1C_UART4_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1C_UART4_IRQ,
    .IntrCtrl  = LS1C_INTC1_BASE,
    .IntrMask  = INTC1_UART4_BIT,
    .dev_name  = "uart4",
};
void *devUART4 = &ls1c_UART4;
#endif

/* UART 5 */
#ifdef BSP_USE_UART5
static NS16550_t ls1c_UART5 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1C_UART5_BASE,
    .DataPort  = LS1C_UART5_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1C_UART5_IRQ,
    .IntrCtrl  = LS1C_INTC1_BASE,
    .IntrMask  = INTC1_UART5_BIT,
    .dev_name  = "uart5",
};
void *devUART5 = &ls1c_UART5;
#endif

/* UART 6 */
#ifdef BSP_USE_UART6
static NS16550_t ls1c_UART6 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1C_UART6_BASE,
    .DataPort  = LS1C_UART6_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1C_UART6_IRQ,
    .IntrCtrl  = LS1C_INTC1_BASE,
    .IntrMask  = INTC1_UART6_BIT,
    .dev_name  = "uart6",
};
void *devUART6 = &ls1c_UART6;
#endif

/* UART 7 */
#ifdef BSP_USE_UART7
static NS16550_t ls1c_UART7 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1C_UART7_BASE,
    .DataPort  = LS1C_UART7_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1C_UART7_IRQ,
    .IntrCtrl  = LS1C_INTC1_BASE,
    .IntrMask  = INTC1_UART7_BIT,
    .dev_name  = "uart7",
};
void *devUART7 = &ls1c_UART7;
#endif

/* UART 8 */
#ifdef BSP_USE_UART8
static NS16550_t ls1c_UART8 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1C_UART8_BASE,
    .DataPort  = LS1C_UART8_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1C_UART8_IRQ,
    .IntrCtrl  = LS1C_INTC1_BASE,
    .IntrMask  = INTC1_UART8_BIT,
    .dev_name  = "uart8",
};
void *devUART8 = &ls1c_UART8;
#endif

/* UART 9 */
#ifdef BSP_USE_UART9
static NS16550_t ls1c_UART9 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1C_UART9_BASE,
    .DataPort  = LS1C_UART9_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1C_UART9_IRQ,
    .IntrCtrl  = LS1C_INTC1_BASE,
    .IntrMask  = INTC1_UART9_BIT,
    .dev_name  = "uart9",
};
void *devUART9 = &ls1c_UART9;
#endif

/* UART 10 */
#ifdef BSP_USE_UART10
static NS16550_t ls1c_UART10 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1C_UART10_BASE,
    .DataPort  = LS1C_UART10_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1C_UART10_IRQ,
    .IntrCtrl  = LS1C_INTC1_BASE,
    .IntrMask  = INTC1_UART10_BIT,
    .dev_name  = "uart10",
};
void *devUART10 = &ls1c_UART10;
#endif

/* UART 11 */
#ifdef BSP_USE_UART11
static NS16550_t ls1c_UART11 =
{
    .BusClock  = 0,                 // to do initialize
    .BaudRate  = 115200,
    .CtrlPort  = LS1C_UART11_BASE,
    .DataPort  = LS1C_UART11_BASE,
    .bFlowCtrl = false,             // enable on hardware support
    .ModemCtrl = 0,
    .bIntrrupt = true,              // by user needed
    .IntrNum   = LS1C_UART11_IRQ,
    .IntrCtrl  = LS1C_INTC1_BASE,
    .IntrMask  = INTC1_UART11_BIT,
    .dev_name  = "uart11",
};
void *devUART11 = &ls1c_UART11;
#endif

#endif

#if (PACK_DRV_OPS)
/******************************************************************************
 * CAN driver operators
 */
static driver_ops_t LS1x_NS16550_drv_ops =
{
    .init_entry  = NS16550_init,
    .open_entry  = NS16550_open,
    .close_entry = NS16550_close,
    .read_entry  = NS16550_read,
    .write_entry = NS16550_write,
    .ioctl_entry = NS16550_ioctl,
};
driver_ops_t *ls1x_uart_drv_ops = &LS1x_NS16550_drv_ops;
#endif

/******************************************************************************
 * for RT-Thread
 */
const char *ls1x_uart_get_device_name(void *pUART)
{
    return ((NS16550_t *)pUART)->dev_name;
}

