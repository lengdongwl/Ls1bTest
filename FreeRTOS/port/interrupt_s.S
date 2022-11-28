/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

    .text
	.align	2

//-----------------------------------------------------------------------------
// 安装到: K0BASE+0x00, K0BASE+0x80, K0BASE+0x180 处 
//-----------------------------------------------------------------------------

    .extern real_exception_entry
    .global except_common_entry
    .type   except_common_entry, @function
except_common_entry:
    la      k0, real_exception_entry
    jr      k0
    nop
    nop
    nop

//-----------------------------------------------------------------------------
// 中断处理入口 
//-----------------------------------------------------------------------------

    .extern vPortYieldISR
    .extern vPortTickInterruptHandler
    .extern vPortInterruptHandler
    .extern exception_handler

    .global real_exception_entry
    .type   real_exception_entry, @function
    .set    noreorder
real_exception_entry:
    mfc0    k0, C0_CAUSE
    andi    k1, k0, (0x1F << 2)
    beq     zero, k1, 1f                    /* 中断 */
    nop
    la      k0, exception_handler           /* 例外 */
    jr      k0
    nop
1:
#if 0
    andi    k1, k0, 0x8000                  /* Is count/compare interrupt? */
    beq     zero, k1, 2f
    nop
    la      k0, vPortTickInterruptHandler
    jr      k0
    nop
#endif
2:  andi    k1, k0, 0x0100                  /* Is software0 interrupt? */
    beq     zero, k1, 3f
    nop
    la      k0, vPortYieldISR 
    jr      k0
    nop

3:  la      k0, vPortInterruptHandler       /* General hardware interrupt */
    jr      k0
    nop
    .set reorder

//-----------------------------------------------------------------------------
// 例外处理入口 
//-----------------------------------------------------------------------------

    .global exception_handler
    .type   exception_handler, @function
    .set    noreorder
exception_handler:
    mfc0    t0, C0_STATUS           //disable interrupt
    and     t0, t0, 0xfffffffe
    mtc0    t0, C0_STATUS

    sw      $0, (4*0)(sp)
    sw      $1, (4*1)(sp)
    sw      $2, (4*2)(sp)
    sw      $3, (4*3)(sp)
    sw      $4, (4*4)(sp)
    sw      $5, (4*5)(sp)
    sw      $6, (4*6)(sp)
    sw      $7, (4*7)(sp)
    sw      $8, (4*8)(sp)
    sw      $9, (4*9)(sp)
    sw      $10, (4*10)(sp)
    sw      $11, (4*11)(sp)
    sw      $12, (4*12)(sp)
    sw      $13, (4*13)(sp)
    sw      $14, (4*14)(sp)
    sw      $15, (4*15)(sp)
    sw      $16, (4*16)(sp)
    sw      $17, (4*17)(sp)
    sw      $18, (4*18)(sp)
    sw      $19, (4*19)(sp)
    sw      $20, (4*20)(sp)
    sw      $21, (4*21)(sp)
    sw      $22, (4*22)(sp)
    sw      $23, (4*23)(sp)
    sw      $24, (4*24)(sp)
    sw      $25, (4*25)(sp)
    sw      $26, (4*26)(sp)
    sw      $27, (4*27)(sp)
    sw      $28, (4*28)(sp)
    sw      $29, (4*29)(sp)
    sw      $30, (4*30)(sp)
    sw      $31, (4*31)(sp)
    move    a0, sp
    la      k0, c_exception_handler
    jr      k0
    nop
    .set reorder

//-----------------------------------------------------------------------------
// end of interrupt.S 
//-----------------------------------------------------------------------------


