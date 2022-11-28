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
// �� interrupt_s.S �е���
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
// ��һ���������
//-----------------------------------------------------------------------------

    .ent    vPortStartFirstTask
    
vPortStartFirstTask:

    la      k0, uxSavedTaskStackPointer     /* ����ջ */
    lw      a0, (k0)

    /*
     * Error while interrupt ocurred here... SP==0
     */
    _load_context                           /* ��ջ */

    la      sp, uxSavedTaskStackPointer     /* sp = ����ջ */
    lw      sp, (sp)

    addiu   sp, sp, CTX_SIZE                /* ��ջ�����ջָ�� */
    eret
    nop

    .end    vPortStartFirstTask

//-----------------------------------------------------------------------------
// tick �ж�
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
// Yield �ж�
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

    addu    k1, zero, sp                    /* k1 ����ջָ��. */

    mfc0    s6, C0_CAUSE                    /* ��� SW0 �жϱ�־ */
    li      s7, 0xfffffeff
    and     s6, s6, s7
    mtc0    s6, C0_CAUSE

#if ISR_STACK
    la      sp, xISRStackTop                /* �л����ж϶�ջ: ��������ж�, ʹ�øö�ջ */
    lw      sp, (sp)
    la		s6, uxInterruptNesting          /* �ж�Ƕ�׼��� =1 */
    addiu	s7, zero, 1
    sw		s7, 0(s6)
#endif

    _save_context                           /* ��ջ, K1 */

    la      k0, pxCurrentTCB                /* ���浱ǰ�����ջָ��. */
    lw      k0, (k0)
    sw      k1, (k0)

    jal     vTaskSwitchContext              /* �����л� */
    nop

    la      k0, pxCurrentTCB                /* ��ȡ��ǰ�����ջָ��. */
    lw      k0, (k0)
    lw      a0, (k0)

    mfc0    k0, C0_STATUS                   /* �ѵ�ǰ status �Ĵ�����ջ, �ٳ�ջ... */
    sw      k0, CTX_OFFSET(R_SR)(a0)

    _load_context                           /* ��ջ */

#if ISR_STACK
    la      k0, uxInterruptNesting          /* �ж�Ƕ�׼��� =0 */
    sw      zero, 0(k0)
#endif

    la      sp, pxCurrentTCB                /* ��ǰ�����ջָ��. */
    lw      sp, (sp)
    lw      sp, (sp)

    addiu   sp, sp, CTX_SIZE
    eret
    nop

    .end    vPortYieldISR

//-----------------------------------------------------------------------------
// �ж�
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

    addu    k1, zero, sp                    /* k1 ���� sp */

#if ISR_STACK
    la      s6, uxInterruptNesting          /* �Ƿ����ж�Ƕ����?  */
    lw      s7, (s6)
    bne     s7, zero, 1f
    nop
    la      sp, xISRStackTop                /* �ж�Ƕ�׼���==0: ��������ж�, ʹ�øö�ջ */
    lw      sp, (sp)
1:
    addiu   s7, s7, 1                       /* �ж�Ƕ�׼��� +=1 */
    sw      s7, (s6)
#endif

    _save_context                           /* ��ջ, k1 */

#if ISR_STACK
    la      k0, uxInterruptNesting          /* �ж�Ƕ�׼���==1, ��Ҫ�����ջֵ */
    lw      k0, (k0)
    addiu   k0, k0, -1
    bne     k0, zero, 2f
    nop
#endif

    la      k0, uxSavedTaskStackPointer     /* �����ַ */
    sw      k1, (k0)                        /* ��ʱ k1 �е��� sp ֵ */
    
2:
    jal     c_interrupt_handler             /* �ж���Ӧ */

#if ISR_STACK
    addu    a0, zero, sp                    /* sp ===> a0 */
    la      k1, uxInterruptNesting          /* �ж�Ƕ�׼���==0, �л���ջ */
    lw      k1, (k1)
    addiu   k1, k1, -1
    bne     k1, zero, 1f
    nop
#endif

    la      k0, uxSavedTaskStackPointer     /* �л�������ջ */
    lw      a0, (k0)

1:
    _load_context                           /* �Ĵ�����ջ */

#if ISR_STACK
    la      k0, uxInterruptNesting          /* �ж�Ƕ�׼��� -=1 */
    lw      k1, (k0)
    addiu   k1, k1, -1
    sw      k1, 0(k0)
    bne     k1, zero, 2f                    /* ��ʱ sp �Ѵ�ջ�з���? */
    nop
#endif

    la      sp, uxSavedTaskStackPointer     /* �ж�Ƕ�׼���==0, ʹ�������ջ */
    lw      sp, (sp)

2:  addiu   sp, sp, CTX_SIZE
    eret
    nop

    .end    vPortInterruptHandler

//-----------------------------------------------------------------------------

/*
 * @@ END
 */
