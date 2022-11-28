/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

    .text

FRAME(mips_set_timer,sp,0,ra)
	.set noreorder
    mfc0    t0, C0_COUNT
    nop
    addu    t0, a0, t0
    mtc0    t0, C0_COMPARE
    j       ra
	nop
	.set reorder
ENDFRAME(mips_set_timer)

FRAME(mips_get_timer,sp,0,ra)
	.set noreorder
    mfc0    v0, C0_COUNT
    j       ra
	nop
	.set reorder
ENDFRAME(mips_get_timer)


