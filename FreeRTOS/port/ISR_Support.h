/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Stack support for FreeRTOS V8.1.2
 */

#ifndef _ISR_SUPPORT_H
#define _ISR_SUPPORT_H

#include "regdef.h"
#include "asm.h"
#include "cpu.h"

//-----------------------------------------------------------------------------

#define CTX_OFFSET(n)   ((n)*4)

#if __mips_hard_float
#define CTX_SIZE        (72*4)
#else
#define CTX_SIZE        (38*4)
#endif

//-----------------------------------------------------------------------------

#ifdef __ASSEMBLER__

#if __mips_hard_float

.macro _save_fpu

    swc1    $f0,  CTX_OFFSET(R_F0)(k1)
    swc1    $f1,  CTX_OFFSET(R_F1)(k1)
    swc1    $f2,  CTX_OFFSET(R_F2)(k1)
    swc1    $f3,  CTX_OFFSET(R_F3)(k1)
    swc1    $f4,  CTX_OFFSET(R_F4)(k1)
    swc1    $f5,  CTX_OFFSET(R_F5)(k1)
    swc1    $f6,  CTX_OFFSET(R_F6)(k1)
    swc1    $f7,  CTX_OFFSET(R_F7)(k1)
    swc1    $f8,  CTX_OFFSET(R_F8)(k1)
    swc1    $f9,  CTX_OFFSET(R_F9)(k1)
    swc1    $f10, CTX_OFFSET(R_F10)(k1)
    swc1    $f11, CTX_OFFSET(R_F11)(k1)
    swc1    $f12, CTX_OFFSET(R_F12)(k1)
    swc1    $f13, CTX_OFFSET(R_F13)(k1)
    swc1    $f14, CTX_OFFSET(R_F14)(k1)
    swc1    $f15, CTX_OFFSET(R_F15)(k1)
    swc1    $f16, CTX_OFFSET(R_F16)(k1)
    swc1    $f17, CTX_OFFSET(R_F17)(k1)
    swc1    $f18, CTX_OFFSET(R_F18)(k1)
    swc1    $f19, CTX_OFFSET(R_F19)(k1)
    swc1    $f20, CTX_OFFSET(R_F20)(k1)
    swc1    $f21, CTX_OFFSET(R_F21)(k1)
    swc1    $f22, CTX_OFFSET(R_F22)(k1)
    swc1    $f23, CTX_OFFSET(R_F23)(k1)
    swc1    $f24, CTX_OFFSET(R_F24)(k1)
    swc1    $f25, CTX_OFFSET(R_F25)(k1)
    swc1    $f26, CTX_OFFSET(R_F26)(k1)
    swc1    $f27, CTX_OFFSET(R_F27)(k1)
    swc1    $f28, CTX_OFFSET(R_F28)(k1)
    swc1    $f29, CTX_OFFSET(R_F29)(k1)
    swc1    $f30, CTX_OFFSET(R_F30)(k1)
    swc1    $f31, CTX_OFFSET(R_F31)(k1)
    cfc1    k0, fcr31                       /* Read FP status/conrol reg */
    cfc1    k0, fcr31                       /* Two reads clear pipeline */
    nop
    nop
    sw 		k0, CTX_OFFSET(R_FCSR)(k1);     /* Store value to FPCS location */
    nop
    
.endm

//-----------------------------------------------------------------------------

.macro _load_fpu

    lwc1    $f0,  CTX_OFFSET(R_F0)(a0)
    lwc1    $f1,  CTX_OFFSET(R_F1)(a0)
    lwc1    $f2,  CTX_OFFSET(R_F2)(a0)
    lwc1    $f3,  CTX_OFFSET(R_F3)(a0)
    lwc1    $f4,  CTX_OFFSET(R_F4)(a0)
    lwc1    $f5,  CTX_OFFSET(R_F5)(a0)
    lwc1    $f6,  CTX_OFFSET(R_F6)(a0)
    lwc1    $f7,  CTX_OFFSET(R_F7)(a0)
    lwc1    $f8,  CTX_OFFSET(R_F8)(a0)
    lwc1    $f9,  CTX_OFFSET(R_F9)(a0)
    lwc1    $f10, CTX_OFFSET(R_F10)(a0)
    lwc1    $f11, CTX_OFFSET(R_F11)(a0)
    lwc1    $f12, CTX_OFFSET(R_F12)(a0)
    lwc1    $f13, CTX_OFFSET(R_F13)(a0)
    lwc1    $f14, CTX_OFFSET(R_F14)(a0)
    lwc1    $f15, CTX_OFFSET(R_F15)(a0)
    lwc1    $f16, CTX_OFFSET(R_F16)(a0)
    lwc1    $f17, CTX_OFFSET(R_F17)(a0)
    lwc1    $f18, CTX_OFFSET(R_F18)(a0)
    lwc1    $f19, CTX_OFFSET(R_F19)(a0)
    lwc1    $f20, CTX_OFFSET(R_F20)(a0)
    lwc1    $f21, CTX_OFFSET(R_F21)(a0)
    lwc1    $f22, CTX_OFFSET(R_F22)(a0)
    lwc1    $f23, CTX_OFFSET(R_F23)(a0)
    lwc1    $f24, CTX_OFFSET(R_F24)(a0)
    lwc1    $f25, CTX_OFFSET(R_F25)(a0)
    lwc1    $f26, CTX_OFFSET(R_F26)(a0)
    lwc1    $f27, CTX_OFFSET(R_F27)(a0)
    lwc1    $f28, CTX_OFFSET(R_F28)(a0)
    lwc1    $f29, CTX_OFFSET(R_F29)(a0)
    lwc1    $f30, CTX_OFFSET(R_F30)(a0)
    lwc1    $f31, CTX_OFFSET(R_F31)(a0)
    cfc1    k0, fcr31                      /* Read from FP status/control reg */
    cfc1    k0, fcr31                      /* Two reads clear pipeline */
    nop
    nop                                    /* nops ensure execution */
    lw      k0, CTX_OFFSET(R_FCSR)(a0)     /* Load saved FPCS value */
    nop
    ctc1    k0, fcr31                      /* Restore FPCS register */
    nop
    
.endm

#endif

//-----------------------------------------------------------------------------

/* Save the hard context to a gp_ctx pointed to by k1.
   Leave the value of C0_STATUS in v0/$2.
   Leave the value of k1 unchanged. */

.macro _save_context

    .set    noreorder
    .set    noat
    
	/* Save general registers.  */
	sw      AT, CTX_OFFSET(R_AT)(k1)
	sw      v0, CTX_OFFSET(R_V0)(k1)
	sw      v1, CTX_OFFSET(R_V1)(k1)
	sw      a0, CTX_OFFSET(R_A0)(k1)
	sw      a1, CTX_OFFSET(R_A1)(k1)
	sw      a2, CTX_OFFSET(R_A2)(k1)
	sw      a3, CTX_OFFSET(R_A3)(k1)
	sw      t0, CTX_OFFSET(R_T0)(k1)
	sw      t1, CTX_OFFSET(R_T1)(k1)
	sw      t2, CTX_OFFSET(R_T2)(k1)
	sw      t3, CTX_OFFSET(R_T3)(k1)
	sw      t4, CTX_OFFSET(R_T4)(k1)
	sw      t5, CTX_OFFSET(R_T5)(k1)
	sw      t6, CTX_OFFSET(R_T6)(k1)
	sw      t7, CTX_OFFSET(R_T7)(k1)
	sw      s0, CTX_OFFSET(R_S0)(k1)
	sw      s1, CTX_OFFSET(R_S1)(k1)
	sw      s2, CTX_OFFSET(R_S2)(k1)
	sw      s3, CTX_OFFSET(R_S3)(k1)
	sw      s4, CTX_OFFSET(R_S4)(k1)
	sw      s5, CTX_OFFSET(R_S5)(k1)
//	sw      s6, CTX_OFFSET(R_S6)(k1)  /* has been saved yet */
//	sw      s7, CTX_OFFSET(R_S7)(k1)  /* has been saved yet */
	sw      t8, CTX_OFFSET(R_T8)(k1)
	sw      t9, CTX_OFFSET(R_T9)(k1)
    /* $26/k0 has been saved prior before using this macro. */
    /* $27/k1 has been saved prior before using this macro. */
	sw      gp, CTX_OFFSET(R_GP)(k1)
//	sw      sp, CTX_OFFSET(R_SP)(k1)  /* has been saved yet */
	sw      fp, CTX_OFFSET(R_FP)(k1)
	sw      ra, CTX_OFFSET(R_RA)(k1)

#if __mips_hard_float
    mfc0	s5, C0_STATUS
    li      k0, SR_CU1
    and     k0, k0, s5
    beqz    k0, 1f
    nop

    _save_fpu           /* save the FPU registers, can use any but K1 */

1:
#endif

    /* Save CP0 registers. */
	mflo	k0
	sw	    k0, CTX_OFFSET(R_MDLO)(k1)
	mfhi	k0
	sw	    k0, CTX_OFFSET(R_MDHI)(k1)
/*
    mfc0	k0, C0_BADVADDR
    sw	    k0, CTX_OFFSET(R_BADVADDR)(k1)
    mfc0	k0, C0_CAUSE
    sw	    k0, CTX_OFFSET(R_CAUSE)(k1)
 */
	mfc0	k0, C0_STATUS
	sw	    k0, CTX_OFFSET(R_SR)(k1)
    mfc0    ra, C0_EPC
	sw   	ra, CTX_OFFSET(R_EPC)(k1)
    nop
    
.endm

//-----------------------------------------------------------------------------

/* Restores a gp_ctx pointed to by a0.  Leaves interrupts disabled and
   C0_EPC ready to eret.  */

.macro _load_context

    .set    noreorder
    .set    noat
    
	/* Restore the general registers.  */
	lw   	AT, CTX_OFFSET(R_AT)(a0)
	lw   	v0, CTX_OFFSET(R_V0)(a0)
	lw   	v1, CTX_OFFSET(R_V1)(a0)
	/* Do not restore $4/a0 until the end.  */
	lw   	a1, CTX_OFFSET(R_A1)(a0)
	lw   	a2, CTX_OFFSET(R_A2)(a0)
	lw   	a3, CTX_OFFSET(R_A3)(a0)
	lw   	t0, CTX_OFFSET(R_T0)(a0)
	lw   	t1, CTX_OFFSET(R_T1)(a0)
	lw   	t2, CTX_OFFSET(R_T2)(a0)
	lw   	t3, CTX_OFFSET(R_T3)(a0)
	lw   	t4, CTX_OFFSET(R_T4)(a0)
	lw   	t5, CTX_OFFSET(R_T5)(a0)
	lw   	t6, CTX_OFFSET(R_T6)(a0)
	lw   	t7, CTX_OFFSET(R_T7)(a0)
	lw   	s0, CTX_OFFSET(R_S0)(a0)
	lw   	s1, CTX_OFFSET(R_S1)(a0)
	lw   	s2, CTX_OFFSET(R_S2)(a0)
	lw   	s3, CTX_OFFSET(R_S3)(a0)
	lw   	s4, CTX_OFFSET(R_S4)(a0)
	lw   	s5, CTX_OFFSET(R_S5)(a0)
	lw   	s6, CTX_OFFSET(R_S6)(a0)
	lw   	s7, CTX_OFFSET(R_S7)(a0)
	lw   	t8, CTX_OFFSET(R_T8)(a0)
	lw   	t9, CTX_OFFSET(R_T9)(a0)

#if __mips_hard_float
	lw	    k1, CTX_OFFSET(R_SR)(a0)    /* Check the saved SR */
    li      k0, SR_CU1
    and     k0, k0, k1
    beqz    k0, 1f
    nop

    _load_fpu           /* load the FPU registers, can use K0/K1 in it */

1:
#endif

	/* Restore CP0 registers, kernel registers and stack with
	   interrupts disabled.  */
	lw	    k0, CTX_OFFSET(R_MDLO)(a0)
	lw	    k1, CTX_OFFSET(R_MDHI)(a0)
	mtlo	k0
	mthi	k1
/*
    lw	    k0, CTX_OFFSET(R_BADVADDR)(a0)
    lw	    k1, CTX_OFFSET(R_CAUSE)(a0)
    mtc0    k0, C0_BADVADDR
    mtc0    k1, C0_CAUSE
 */
	lw   	gp, CTX_OFFSET(R_GP)(a0)
	lw   	sp, CTX_OFFSET(R_SP)(a0)
	lw   	fp, CTX_OFFSET(R_FP)(a0)
	lw   	ra, CTX_OFFSET(R_RA)(a0)

    .set    mips32r2
	di
	.set    mips32

	lw   	k0, CTX_OFFSET(R_EPC)(a0)
	lw	    k1, CTX_OFFSET(R_SR)(a0)
	lw   	a0, CTX_OFFSET(R_A0)(a0)	    /* restore a0/$4.  */

	mtc0	k1, C0_STATUS
    mtc0    k0, C0_EPC

	.set    mips32r2
    ehb
	.set    mips32

/*
	lw   	k0, CTX_OFFSET(R_K0)(a0)        // restore k0
	lw   	k1, CTX_OFFSET(R_K1)(a0)        // restore k1
 */

.endm

#endif // #ifdef __ASSEMBLER__

#endif // #ifndef _ISR_SUPPORT_H

//-----------------------------------------------------------------------------

/*
 * @@ END
 */


