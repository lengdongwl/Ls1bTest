/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Implement task switch for FreeRTOS V8.1.2
 */

#include "cpu.h"
#include "regdef.h"
#include "ISR_Support.h"

#define ISR_STACK       1

//-----------------------------------------------------------------------------
// 在 interrupt_s.S 中调用
//-----------------------------------------------------------------------------

    .text
    .set    noreorder

    .extern pxCurrentTCB
    .extern vTaskSwitchContext
    .extern vPortIncrementTick
    .extern xISRStackTop
    .extern uxInterruptNesting
    
    .global vPortStartFirstTask
    .global vPortYieldISR

//-----------------------------------------------------------------------------
// 第一次任务入口
//-----------------------------------------------------------------------------

    .ent    vPortStartFirstTask
    
vPortStartFirstTask:

    la      k0, uxSavedTaskStackPointer     /* 任务栈 */
    lw      a0, (k0)

    /*
     * Error while interrupt ocurred here... SP==0
     */
    _load_context                           /* 出栈 */

    la      sp, uxSavedTaskStackPointer     /* sp = 任务栈 */
    lw      sp, (sp)

    addiu   sp, sp, CTX_SIZE                /* 出栈后调整栈指针 */
    eret
    nop

    .end    vPortStartFirstTask

//-----------------------------------------------------------------------------
// tick 中断
//-----------------------------------------------------------------------------

#if 0

    .ent    vPortTickInterruptHandler
    .global vPortTickInterruptHandler

vPortTickInterruptHandler:

    portSAVE_CONTEXT

    jal     vPortIncrementTick
    nop

    portRESTORE_CONTEXT

    .end    vPortTickInterruptHandler
    
#endif

//-----------------------------------------------------------------------------
// Yield 中断
//-----------------------------------------------------------------------------

    .ent    vPortYieldISR
    
vPortYieldISR:

    addiu   sp, sp, -CTX_SIZE
/*
    sw      k0, CTX_OFFSET(R_K0)(sp)
    sw      k1, CTX_OFFSET(R_K1)(sp)
 */
    sw      s6, CTX_OFFSET(R_S6)(sp)
    sw      s7, CTX_OFFSET(R_S7)(sp)
    sw      sp, CTX_OFFSET(R_SP)(sp)

    addu    k1, zero, sp                    /* k1 用作栈指针. */

    mfc0    s6, C0_CAUSE                    /* 清除 SW0 中断标志 */
    li      s7, 0xfffffeff
    and     s6, s6, s7
    mtc0    s6, C0_CAUSE

#if ISR_STACK
    la      sp, xISRStackTop                /* 切换到中断堆栈: 如果发生中断, 使用该堆栈 */
    lw      sp, (sp)
    la		s6, uxInterruptNesting          /* 中断嵌套计数 =1 */
    addiu	s7, zero, 1
    sw		s7, 0(s6)
#endif

    _save_context                           /* 入栈, K1 */

    la      k0, pxCurrentTCB                /* 保存当前任务的栈指针. */
    lw      k0, (k0)
    sw      k1, (k0)

    jal     vTaskSwitchContext              /* 任务切换 */
    nop

    la      k0, pxCurrentTCB                /* 获取当前任务的栈指针. */
    lw      k0, (k0)
    lw      a0, (k0)

    mfc0    k0, C0_STATUS                   /* 把当前 status 寄存器入栈, 再出栈... */
    sw      k0, CTX_OFFSET(R_SR)(a0)

    _load_context                           /* 出栈 */

#if ISR_STACK
    la      k0, uxInterruptNesting          /* 中断嵌套计数 =0 */
    sw      zero, 0(k0)
#endif

    la      sp, pxCurrentTCB                /* 当前任务的栈指针. */
    lw      sp, (sp)
    lw      sp, (sp)

    addiu   sp, sp, CTX_SIZE
    eret
    nop

    .end    vPortYieldISR

//-----------------------------------------------------------------------------
// 中断
//-----------------------------------------------------------------------------

    .extern c_interrupt_handler
    .ent    vPortInterruptHandler
    .global vPortInterruptHandler

vPortInterruptHandler:

    addiu   sp, sp, -CTX_SIZE
/*
    sw      k0, CTX_OFFSET(R_K0)(sp)
    sw      k1, CTX_OFFSET(R_K1)(sp)
 */
    sw      s6, CTX_OFFSET(R_S6)(sp)
    sw      s7, CTX_OFFSET(R_S7)(sp)
    sw      sp, CTX_OFFSET(R_SP)(sp)

    addu    k1, zero, sp                    /* k1 用作 sp */

#if ISR_STACK
    la      s6, uxInterruptNesting          /* 是否在中断嵌套中?  */
    lw      s7, (s6)
    bne     s7, zero, 1f
    nop
    la      sp, xISRStackTop                /* 中断嵌套计数==0: 如果发生中断, 使用该堆栈 */
    lw      sp, (sp)
1:
    addiu   s7, s7, 1                       /* 中断嵌套计数 +=1 */
    sw      s7, (s6)
#endif

    _save_context                           /* 入栈, k1 */

#if ISR_STACK
    la      k0, uxInterruptNesting          /* 中断嵌套计数==1, 需要保存堆栈值 */
    lw      k0, (k0)
    addiu   k0, k0, -1
    bne     k0, zero, 2f
    nop
#endif

    la      k0, uxSavedTaskStackPointer     /* 保存地址 */
    sw      k1, (k0)                        /* 这时 k1 中的是 sp 值 */
    
2:
    jal     c_interrupt_handler             /* 中断响应 */

#if ISR_STACK
    addu    a0, zero, sp                    /* sp ===> a0 */
    la      k1, uxInterruptNesting          /* 中断嵌套计数==0, 切换堆栈 */
    lw      k1, (k1)
    addiu   k1, k1, -1
    bne     k1, zero, 1f
    nop
#endif

    la      k0, uxSavedTaskStackPointer     /* 切换到任务栈 */
    lw      a0, (k0)

1:
    _load_context                           /* 寄存器出栈 */

#if ISR_STACK
    la      k0, uxInterruptNesting          /* 中断嵌套计数 -=1 */
    lw      k1, (k0)
    addiu   k1, k1, -1
    sw      k1, 0(k0)
    bne     k1, zero, 2f                    /* 这时 sp 已从栈中返回? */
    nop
#endif

    la      sp, uxSavedTaskStackPointer     /* 中断嵌套计数==0, 使用任务堆栈 */
    lw      sp, (sp)

2:  addiu   sp, sp, CTX_SIZE
    eret
    nop

    .end    vPortInterruptHandler

//-----------------------------------------------------------------------------

/*
 * @@ END
 */

