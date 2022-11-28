/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * LS1B irq.h
 *
 * Interrupt related API declarations.
 *
 * This file contains the API prototypes for configuring LS232 INTC
 */

#ifndef _LS1B_IRQ_H
#define _LS1B_IRQ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mips.h"

/******************************************************************************
 * Interrupt Vector Numbers
 * MIPS_INTERRUPT_BASE should be 32 (0x20)
 ******************************************************************************/

/*
 * CP0 Cause ($12)  IP bit(15:8)=IP[7:0], IP[1:0] is soft-interrupt
 *     Status($13)  IM bit(15:8) if mask interrupts
 *
 */
#define LS1B_IRQ_SW0            (MIPS_INTERRUPT_BASE + 0)
#define LS1B_IRQ_SW1            (MIPS_INTERRUPT_BASE + 1)
#define LS1B_IRQ0_REQ           (MIPS_INTERRUPT_BASE + 2)
#define LS1B_IRQ1_REQ           (MIPS_INTERRUPT_BASE + 3)
#define LS1B_IRQ2_REQ           (MIPS_INTERRUPT_BASE + 4)
#define LS1B_IRQ3_REQ           (MIPS_INTERRUPT_BASE + 5)
#define LS1B_IRQ_PERF           (MIPS_INTERRUPT_BASE + 6)
#define LS1B_IRQ_CNT            (MIPS_INTERRUPT_BASE + 7)

/*
 * Interrupt Control 0 Interrupts
 */
#define LS1B_IRQ0_BASE          (MIPS_INTERRUPT_BASE + 8)

#define LS1B_UART0_IRQ          (LS1B_IRQ0_BASE + 2)
#define LS1B_UART1_IRQ          (LS1B_IRQ0_BASE + 3)
#define LS1B_UART2_IRQ          (LS1B_IRQ0_BASE + 4)
#define LS1B_UART3_IRQ          (LS1B_IRQ0_BASE + 5)
#define LS1B_CAN0_IRQ           (LS1B_IRQ0_BASE + 6)
#define LS1B_CAN1_IRQ           (LS1B_IRQ0_BASE + 7)
#define LS1B_SPI0_IRQ           (LS1B_IRQ0_BASE + 8)
#define LS1B_SPI1_IRQ           (LS1B_IRQ0_BASE + 9)
#define LS1B_AC97_IRQ           (LS1B_IRQ0_BASE + 10)
#define LS1B_DMA0_IRQ           (LS1B_IRQ0_BASE + 13)
#define LS1B_DMA1_IRQ           (LS1B_IRQ0_BASE + 14)
#define LS1B_DMA2_IRQ           (LS1B_IRQ0_BASE + 15)
#define LS1B_PWM0_IRQ           (LS1B_IRQ0_BASE + 17)
#define LS1B_PWM1_IRQ           (LS1B_IRQ0_BASE + 18)
#define LS1B_PWM2_IRQ           (LS1B_IRQ0_BASE + 19)
#define LS1B_PWM3_IRQ           (LS1B_IRQ0_BASE + 20)
#define LS1B_RTC0_IRQ           (LS1B_IRQ0_BASE + 21)
#define LS1B_RTC1_IRQ           (LS1B_IRQ0_BASE + 22)
#define LS1B_RTC2_IRQ           (LS1B_IRQ0_BASE + 23)
#define LS1B_TOY0_IRQ           (LS1B_IRQ0_BASE + 24)
#define LS1B_TOY1_IRQ           (LS1B_IRQ0_BASE + 25)
#define LS1B_TOY2_IRQ           (LS1B_IRQ0_BASE + 26)
#define LS1B_RTC_IRQ            (LS1B_IRQ0_BASE + 27)
#define LS1B_TOY_IRQ            (LS1B_IRQ0_BASE + 28)
#define LS1B_UART4_IRQ          (LS1B_IRQ0_BASE + 29)
#define LS1B_UART5_IRQ          (LS1B_IRQ0_BASE + 30)

/*
 * Interrupt Control 1 interrupts
 */
#define LS1B_IRQ1_BASE          (MIPS_INTERRUPT_BASE + 40)

#define LS1B_EHCI_IRQ           (LS1B_IRQ1_BASE + 0)
#define LS1B_OHCI_IRQ           (LS1B_IRQ1_BASE + 1)
#define LS1B_GMAC0_IRQ          (LS1B_IRQ1_BASE + 2)
#define LS1B_GMAC1_IRQ          (LS1B_IRQ1_BASE + 3)

/*
 * Interrupt Control 2 interrupts (GPIO)
 */
#define LS1B_IRQ2_BASE          (MIPS_INTERRUPT_BASE + 72)

#define LS1B_GPIO0_IRQ          (LS1B_IRQ2_BASE + 0)
#define LS1B_GPIO1_IRQ          (LS1B_IRQ2_BASE + 1)
#define LS1B_GPIO2_IRQ          (LS1B_IRQ2_BASE + 2)
#define LS1B_GPIO3_IRQ          (LS1B_IRQ2_BASE + 3)
#define LS1B_GPIO4_IRQ          (LS1B_IRQ2_BASE + 4)
#define LS1B_GPIO5_IRQ          (LS1B_IRQ2_BASE + 5)
#define LS1B_GPIO6_IRQ          (LS1B_IRQ2_BASE + 6)
#define LS1B_GPIO7_IRQ          (LS1B_IRQ2_BASE + 7)
#define LS1B_GPIO8_IRQ          (LS1B_IRQ2_BASE + 8)
#define LS1B_GPIO9_IRQ          (LS1B_IRQ2_BASE + 9)
#define LS1B_GPIO10_IRQ         (LS1B_IRQ2_BASE + 10)
#define LS1B_GPIO11_IRQ         (LS1B_IRQ2_BASE + 11)
#define LS1B_GPIO12_IRQ         (LS1B_IRQ2_BASE + 12)
#define LS1B_GPIO13_IRQ         (LS1B_IRQ2_BASE + 13)
#define LS1B_GPIO14_IRQ         (LS1B_IRQ2_BASE + 14)
#define LS1B_GPIO15_IRQ         (LS1B_IRQ2_BASE + 15)
#define LS1B_GPIO16_IRQ         (LS1B_IRQ2_BASE + 16)
#define LS1B_GPIO17_IRQ         (LS1B_IRQ2_BASE + 17)
#define LS1B_GPIO18_IRQ         (LS1B_IRQ2_BASE + 18)
#define LS1B_GPIO19_IRQ         (LS1B_IRQ2_BASE + 19)
#define LS1B_GPIO20_IRQ         (LS1B_IRQ2_BASE + 20)
#define LS1B_GPIO21_IRQ         (LS1B_IRQ2_BASE + 21)
#define LS1B_GPIO22_IRQ         (LS1B_IRQ2_BASE + 22)
#define LS1B_GPIO23_IRQ         (LS1B_IRQ2_BASE + 23)
#define LS1B_GPIO24_IRQ         (LS1B_IRQ2_BASE + 24)
#define LS1B_GPIO25_IRQ         (LS1B_IRQ2_BASE + 25)
#define LS1B_GPIO26_IRQ         (LS1B_IRQ2_BASE + 26)
#define LS1B_GPIO27_IRQ         (LS1B_IRQ2_BASE + 27)
#define LS1B_GPIO28_IRQ         (LS1B_IRQ2_BASE + 28)
#define LS1B_GPIO29_IRQ         (LS1B_IRQ2_BASE + 29)
#define LS1B_GPIO30_IRQ         (LS1B_IRQ2_BASE + 30)
 
/*
 * Interrupt Control 3 source bit (GPIO)
 */
#define LS1B_IRQ3_BASE          (MIPS_INTERRUPT_BASE + 104)

#define LS1B_GPIO32_IRQ         (LS1B_IRQ3_BASE + 0)
#define LS1B_GPIO33_IRQ         (LS1B_IRQ3_BASE + 1)
#define LS1B_GPIO34_IRQ         (LS1B_IRQ3_BASE + 2)
#define LS1B_GPIO35_IRQ         (LS1B_IRQ3_BASE + 3)
#define LS1B_GPIO36_IRQ         (LS1B_IRQ3_BASE + 4)
#define LS1B_GPIO37_IRQ         (LS1B_IRQ3_BASE + 5)
#define LS1B_GPIO38_IRQ         (LS1B_IRQ3_BASE + 6)
#define LS1B_GPIO39_IRQ         (LS1B_IRQ3_BASE + 7)
#define LS1B_GPIO40_IRQ         (LS1B_IRQ3_BASE + 8)
#define LS1B_GPIO41_IRQ         (LS1B_IRQ3_BASE + 9)
#define LS1B_GPIO42_IRQ         (LS1B_IRQ3_BASE + 10)
#define LS1B_GPIO43_IRQ         (LS1B_IRQ3_BASE + 11)
#define LS1B_GPIO44_IRQ         (LS1B_IRQ3_BASE + 12)
#define LS1B_GPIO45_IRQ         (LS1B_IRQ3_BASE + 13)
#define LS1B_GPIO46_IRQ         (LS1B_IRQ3_BASE + 14)
#define LS1B_GPIO47_IRQ         (LS1B_IRQ3_BASE + 15)
#define LS1B_GPIO48_IRQ         (LS1B_IRQ3_BASE + 16)
#define LS1B_GPIO49_IRQ         (LS1B_IRQ3_BASE + 17)
#define LS1B_GPIO50_IRQ         (LS1B_IRQ3_BASE + 18)
#define LS1B_GPIO51_IRQ         (LS1B_IRQ3_BASE + 19)
#define LS1B_GPIO52_IRQ         (LS1B_IRQ3_BASE + 20)
#define LS1B_GPIO53_IRQ         (LS1B_IRQ3_BASE + 21)
#define LS1B_GPIO54_IRQ         (LS1B_IRQ3_BASE + 22)
#define LS1B_GPIO55_IRQ         (LS1B_IRQ3_BASE + 23)
#define LS1B_GPIO56_IRQ         (LS1B_IRQ3_BASE + 24)
#define LS1B_GPIO57_IRQ         (LS1B_IRQ3_BASE + 25)
#define LS1B_GPIO58_IRQ         (LS1B_IRQ3_BASE + 26)
#define LS1B_GPIO59_IRQ         (LS1B_IRQ3_BASE + 27)
#define LS1B_GPIO60_IRQ         (LS1B_IRQ3_BASE + 28)
#define LS1B_GPIO61_IRQ         (LS1B_IRQ3_BASE + 29)

#define LS1B_MAXIMUM_VECTORS        (LS1B_GPIO61_IRQ+1)

#define BSP_INTERRUPT_VECTOR_MIN    0
#define BSP_INTERRUPT_VECTOR_MAX    LS1B_MAXIMUM_VECTORS

/******************************************************************************
 * use for "vectorirs.c"
 */
#define LS1x_IRQ_SW0            LS1B_IRQ_SW0
#define LS1x_IRQ_SW1            LS1B_IRQ_SW1
#define LS1x_IRQ_PERF           LS1B_IRQ_PERF
#define LS1x_IRQ_CNT            LS1B_IRQ_CNT

#define LS1x_IRQ0_BASE          LS1B_IRQ0_BASE
#define LS1x_IRQ1_BASE          LS1B_IRQ1_BASE
#define LS1x_IRQ2_BASE          LS1B_IRQ2_BASE
#define LS1x_IRQ3_BASE          LS1B_IRQ3_BASE

#ifdef __cplusplus
}
#endif

#endif /* _LS1B_IRQ_H */

