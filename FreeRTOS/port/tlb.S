/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * fetch the registers associated with and the contents of the tlb.
 */
 
#include "regdef.h"
#include "cpu.h"
#include "asm.h"

	.text

/*
 * ret_tlblo[01] -- returns the 'entrylo' contents for the TLB
 * 'c' callable - as ret_tlblo(index) - where index is the
 * tlb entry to return the lo value for - if called from assembly
 * language then index should be in register a0.
 */
FRAME(ret_tlblo0, sp, 0, ra)
    mfc0    t0, C0_SR               # save sr
    mtc0    zero, C0_SR             # clear interrupts
    mfc0    t1, C0_TLBHI            # save pid
    mtc0    a0, C0_INX              # write to index register
    .set    noreorder
    nop; nop; nop; nop; nop; nop; nop; nop
    .set    reorder
    tlbr                            # put tlb entry in entrylo and hi
    .set    noreorder
    nop; nop; nop; nop; nop; nop; nop; nop
    .set    reorder
    mfc0    v0, C0_TLBLO0           # get the requested entry lo
    mtc0    t1, C0_TLBHI            # restore pid
    mtc0    t0, C0_SR               # restore status register
    j       ra
    nop
ENDFRAME(ret_tlblo0)

FRAME(ret_tlblo1, sp, 0, ra)
    mfc0    t0, C0_SR               # save sr
    mtc0    zero, C0_SR             # clear interrupts
    mfc0    t1, C0_TLBHI            # save pid
    mtc0    a0, C0_INX              # write to index register
    .set    noreorder
    nop; nop; nop; nop; nop; nop; nop; nop
    .set    reorder
    tlbr                            # put tlb entry in entrylo and hi
    .set    noreorder
    nop; nop; nop; nop; nop; nop; nop; nop
    .set    reorder
    mfc0    v0, C0_TLBLO1           # get the requested entry lo
    mtc0    t1, C0_TLBHI            # restore pid
    mtc0    t0, C0_SR               # restore status register
    j       ra
    nop
ENDFRAME(ret_tlblo1)

/*
 * ret_pagemask(index) -- return pagemask contents of tlb entry "index"
 */
FRAME(ret_pagemask, sp, 0, ra)
    mfc0    t0, C0_SR               # save sr
    mtc0    zero, C0_SR             # disable interrupts
    mfc0    t1, C0_TLBHI            # save current pid
    mtc0    a0, C0_INX              # drop it in C0 register
    .set    noreorder
    nop; nop; nop; nop; nop; nop; nop; nop
    .set    reorder
    tlbr                            # read entry to entry hi/lo
    .set    noreorder
    nop; nop; nop; nop; nop; nop; nop; nop
    .set    reorder
    mfc0    v0, C0_PAGEMASK         # to return value
    mtc0    t1, C0_TLBHI            # restore current pid
    mtc0    t0, C0_SR               # restore sr
    j       ra
    nop
ENDFRAME(ret_pagemask)

/*
 * ret_tlbwired(void) -- return wired register
 */
FRAME(ret_tlbwired, sp, 0, ra)
    mfc0    v0, C0_WIRED
    j       ra
    nop
ENDFRAME(ret_tlbwired)

/*
 * ret_tlbhi -- return the tlb entry high content for tlb entry index
 */
FRAME(ret_tlbhi, sp, 0, ra)
    mfc0    t0, C0_SR               # save sr
    mtc0    zero, C0_SR             # disable interrupts
    mfc0    t1, C0_TLBHI            # save current pid
    mtc0    a0, C0_INX              # drop it in C0 register
    .set    noreorder
    nop; nop; nop; nop; nop; nop; nop; nop
    .set    reorder
    tlbr                            # read entry to entry hi/lo0/lo1/mask
    .set    noreorder
    nop; nop; nop; nop; nop; nop; nop; nop
    .set    reorder
    mfc0    v0, C0_TLBHI            # to return value
    mtc0    t1, C0_TLBHI            # restore current pid
    mtc0    t0, C0_SR               # restore sr
    j       ra
    nop
ENDFRAME(ret_tlbhi)

/*
 * ret_tlbpid() -- return tlb pid contained in the current entry hi
 */
FRAME(ret_tlbpid, sp, 0, ra)
    mfc0    v0, C0_TLBHI            # to return value
    nop
    and     v0, TLBHI_PIDMASK
    j       ra
    nop
ENDFRAME(ret_tlbpid)

/*
 * tlbprobe(address, pid) -- probe the tlb to see if address is currently mapped
 * a0 = vpn  - virtual page numbers are 0=0 1=0x1000, 2=0x2000...
 *			   virtual page numbers for the r3000 are in
 *			   entry hi bits 31-12
 * a1 = pid  - this is a process id ranging from 0 to 63
 *		       this process id is shifted left 6 bits and or'ed into
 *		       the entry hi register
 * returns an index value (0-63) if successful -1 -f not
 */
FRAME(tlbprobe, sp, 0, ra)
    mfc0    t0, C0_SR               # save sr
    mtc0    zero, C0_SR             # disable interrupts
    mfc0    t1, C0_TLBHI            # save current pid
    and     a0, TLBHI_VPN2MASK      # construct tlbhi for probe
    and     a1, TLBHI_PIDMASK
    or      a0, a1
    mtc0    a0, C0_TLBHI
    .set    noreorder
    nop; nop; nop; nop; nop; nop; nop; nop
    .set    reorder
    tlbp                            # probe entry to entry hi/lo0/lo1/mask
    .set    noreorder
    nop; nop; nop; nop; nop; nop; nop; nop
    .set    reorder
    mfc0    v1, C0_INX
    li      v0, -1
    bltz    v1, 1f
    move    v0, v1
1:
    mtc0    t1, C0_TLBHI            # restore current pid
    mtc0    t0, C0_SR               # restore sr
    j       ra
    nop
ENDFRAME(tlbprobe)

/*
 * resettlb(index) Invalidate the  TLB entry specified by index
 */
FRAME(resettlb, sp, 0, ra)
    li      t2, K0BASE&TLBHI_VPN2MASK
    mfc0    t0, C0_TLBHI            # save current TLBHI
    mfc0    v0, C0_SR               # save SR and disable interrupts
    mtc0    zero, C0_SR
    mtc0    t2, C0_TLBHI            # invalidate entry
    mtc0    zero, C0_TLBLO0
    mtc0    zero, C0_TLBLO1
    mtc0    a0, C0_INX
    .set    noreorder
    nop; nop; nop; nop; nop; nop; nop; nop
    .set    reorder
    tlbwi
    .set    noreorder
    nop; nop; nop; nop; nop; nop; nop; nop
    .set    reorder
    mtc0    t0, C0_TLBHI
    mtc0    v0, C0_SR
    j       ra
    nop
ENDFRAME(resettlb)

/*
 * Set current TLBPID. This assumes PID is positioned correctly in reg a0.
 */
FRAME(set_tlbpid, sp, 0, ra)
    .set    noreorder
    mtc0    a0, C0_TLBHI
    j       ra
    nop
    .set    reorder
ENDFRAME(set_tlbpid)


