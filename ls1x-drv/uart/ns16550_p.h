/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ns16550_p.h, 2013/04/09 Bian
 */

#ifndef _NS16550_P_H_
#define _NS16550_P_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ns16550 hardware register, the same as ns16550
 */
#define NS16550_RECEIVE_BUFFER		0	/* DLAB=0; read */
#define NS16550_TRANSMIT_BUFFER		0	/* DLAB=0; write */
#define NS16550_INTERRUPT_ENABLE	1	/* DLAB=0; */
#define NS16550_INTERRUPT_ID		2	/* DLAB=X; read */
#define NS16550_FIFO_CONTROL		2	/* DLAB=X; write */
#define NS16550_LINE_CONTROL		3	/* DLAB=X; */
#define NS16550_MODEM_CONTROL		4	/* DLAB=X; */
#define NS16550_LINE_STATUS			5	/* DLAB=X; */
#define NS16550_MODEM_STATUS		6	/* DLAB=X; */
#define NS16550_SCRATCH_PAD			7	/* DLAB=X; */
#define NS16550_DIVISIORLATCH_LSB	0	/* DLAB=1; */
#define NS16550_DIVISIORLATCH_MSB	1	/* DLAB=1; */
#define NS16550_ALTERNET_FUNCTION	2	/* DLAB=1; */

/*
 * Define "Interrupt Enable" register structure.
 */
#define SP_INT_RX_ENABLE	0x01		/* int on rx ready */
#define SP_INT_TX_ENABLE	0x02		/* int on tx ready */
#define SP_INT_LS_ENABLE	0x04		/* int on line status change */
#define SP_INT_MS_ENABLE	0x08		/* int on modem status change */

#define NS16550_ENABLE_ALL_INTR           (SP_INT_RX_ENABLE | SP_INT_TX_ENABLE)
#define NS16550_DISABLE_ALL_INTR          0x00
#define NS16550_ENABLE_ALL_INTR_EXCEPT_TX (SP_INT_RX_ENABLE)

/*
 * Define "Interrupt ID" register structure.
 */
#define SP_INTID_IMASK		0x0f	/* mask */
#define SP_INTID_MLSC		0x00	/* modem status */
#define SP_INTID_NOPEND		0x01	/* nothing */
#define SP_INTID_TXRDY		0x02	/* transmit ready */
#define SP_INTID_RXRDY		0x04	/* receive ready */
#define SP_INTID_RLS		0x06	/* receive line status */
#define SP_INTID_RXTOUT		0x0c	/* receive timeout */
#define SP_INTID_FIFO_MASK	0xc0	/* set if FIFOs are enabled */

/*
 * Define "FIFO Control" register structure.
 */
#define SP_FIFO_ENABLE  	0x01	/* enable fifo */
#define SP_FIFO_RXRST 		0x02	/* reset receive fifo */
#define SP_FIFO_TXRST 		0x04	/* reset transmit fifo */
#define SP_FIFO_DMA   		0x08	/* enable dma mode, 0=8250 mode */
#define SP_FIFO_TRIGGER_1	0x00	/* trigger at 1 char */
#define SP_FIFO_TRIGGER_4	0x40	/* trigger at 4 chars */
#define SP_FIFO_TRIGGER_8	0x80	/* trigger at 8 chars */
#define SP_FIFO_TRIGGER_12 	0xc0	/* trigger at 12 chars */

#define SP_FIFO_SIZE 16

/*
 * Define "Line Control" register structure.
 */
#define CFCR_5_BITS 	0x00 	/* five bits per character */
#define CFCR_6_BITS 	0x01  	/* six bits per character */
#define CFCR_7_BITS 	0x02  	/* seven bits per character */
#define CFCR_8_BITS 	0x03   	/* eight bits per character */
#define SP_LINE_SZMASK	0x03	/* Size Mask */

#define SP_LINE_STOP	0x04	/* Stop bits: 0=1 bit, 1=2 bits */
#define SP_LINE_PAR		0x08	/* parity enable */
#define	SP_LINE_EVEN	0x10	/* Even Parity Select, otherwise Odd */
#define	SP_LINE_ODD		0x00
#define	SP_LINE_ONE		0x20	/* Stick Parity ? is one parity? */
#define SP_LINE_BREAK	0x40	/* send break */
#define SP_LINE_DLAB	0x80	/* Enable Divisor Latch Access */

/*
 * Define "Line Status" register structure.
 */
#define SP_LSR_RDY		0x01	/* receiver ready */
#define SP_LSR_EOVRUN 	0x02	/* overrun error */
#define SP_LSR_EPAR   	0x04	/* parity error */
#define SP_LSR_EFRAME 	0x08	/* framing error */
#define SP_LSR_BREAK  	0x10	/* break detected */
#define SP_LSR_THOLD  	0x20	/* transmitter ready */
#define SP_LSR_TX     	0x40	/* transmitter empty */
#define SP_LSR_EFIFO  	0x80	/* error in receive fifo */

/*
 * Define "Modem Control" register structure.
 */
#define SP_MODEM_DTR	0x01	/* Data Terminal Ready */
#define SP_MODEM_RTS	0x02	/* Request To Send */
#define	SP_MODEM_DRS	0x04	/* Output 1, (reserved on COMPAQ I/O Board) */
#define SP_MODEM_IRQ	0x08	/* Output 2, Enable Asynchronous Port Interrupts */
#define SP_MODEM_LOOP	0x10	/* Enable Internal Loop Back */

/*
 * Define "Modem Status" register structure.
 */
#define SP_MSR_DCTS 	0x01 	/* Delta Clear To Send */
#define SP_MSR_DDSR 	0x02 	/* Delta Data Set Ready */
#define SP_MSR_TERI 	0x04 	/* Trailing Edge Ring Indicator */
#define SP_MSR_DDCD 	0x08 	/* Delta Carrier Detect Indicator */
#define SP_MSR_CTS  	0x10 	/* Clear To Send (when loop back is active) */
#define SP_MSR_DSR  	0x20 	/* Data Set Ready (when loop back is active) */
#define SP_MSR_RI   	0x40 	/* Ring Indicator (when loop back is active) */
#define SP_MSR_DCD  	0x80 	/* Data Carrier Detect (when loop back is active) */

/*
 * Line speed divisor definition.
 */
#define NS16550_BAUD_DIV(_clock, _baud_rate) \
		((_clock) / ((_baud_rate == 0) ? 115200 : (_baud_rate)) / 16)

#ifdef __cplusplus
}
#endif

#endif /* _NS16550_P_H_ */

