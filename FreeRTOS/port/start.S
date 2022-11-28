/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * Typical standalone start up code required for MIPS32
 */

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

	.extern _fbss, 4                   /* The name in ld.script */
	.extern _end, 4                    /* The name in ld.script */
	.extern _stack_start, 4            /* The name in ld.script */
	.extern _stack_end, 4              /* The name in ld.script */
	
	.lcomm  memory_cfg_struct, 12      /* 1st: 4 bytes, memory size
	                                    * 2st: 4 bytes, icache size
	                                    * 3th: 4 bytes, dcache size
	                                    */

/******************************************************************************
 * 注意: 1. 没有加入 rom 启动的代码
 *       2. ld.script中调整大小 _StackSize
 */

	.text

//-----------------------------------------------------------------------------

FRAME(start, sp, 0, ra)
    .set    noreorder

#if (__mips_hard_float)
    li      v0, SR_CU1 
    mtc0    v0, C0_SR                       /* set CU1 for test */               
#else
    mtc0    zero, C0_SR                     /* clr IntMsks/ kernel/disabled mode */    
#endif
    nop
    li      v0, CAUSE_DC
    mtc0    v0, C0_CAUSE                    /* clear software interrupts */
    nop

    lui     v0, 0xFFFF
    mtc0    v0, C0_COMPARE                  /* maybe COUNT/COMPARE interrupt, clear It. */
    nop
    
/******************************************************************************
 * Because of we are start form pmon, boot-initialization things has done.
 * If we start form FLASH, we must do these thing here.
 ******************************************************************************/

/*
 * TODO 片上设备初始化代码 
 */

#if (__mips_hard_float)
/* check to see if a fpu is really plugged in
 */
    li      t3,   0xAAAA5555                /* put a's and 5's in t3	*/
    mtc1    t3,   fp0                       /* try to write them into fp0   */
    mtc1    zero, fp1                       /* try to write zero in fp	*/
    mfc1    t0,   fp0
    mfc1    t1,   fp1
    nop
    bne     t0, t3, 1f                      /* branch if no match  */
    nop
    bne     t1, zero, 1f                    /* double check for positive id   */
    nop
/* We have a FPU. clear fcsr
 */
    ctc1    zero, fcr31
    j       2f                              /* status register already correct  */
    nop
1:
    li      v0, 0x0                         /* clear ERL and disable FPA */
    mtc0    v0, C0_SR                       /* reset status register */
    nop
2:
#endif

    la      gp, _gp                         /* Initialize gp register (pointer to "small" data)*/

/* clear bss before using it
 */
    la      v0, _fbss                       /* start of bss */
    la      v1, _end                        /* end of bss */
3:
    sw      zero, 0(v0)
    bltu    v0, v1, 3b
    add     v0, 4

#if 1
/* clear stack before using it. XXX in .bss section, needn't clear alone
 */
    la      v0, _stack_start                /* start of stack */
    la      v1, _stack_end                  /* end of stack */
4:
    sw      zero, 0(v0)
    bltu    v0, v1, 4b
    add     v0, 4
#endif
    
/* Set system stack
 */
    la      t0, _stack_end            
    sub     t0, t0, (4 * 4)                 /* XXX overhead  */
    move    sp, t0  
    nop

/* Set memory size global
 */
    la      a0, _RamSize 
    jal     set_memory_size
	nop
	
/* determine size of D & I caches, In "Idtmem.S"
 */
    jal     config_cache
    nop
    
/* fill memory config struct
 */
    la      a0, memory_cfg_struct
    jal     get_memory_conf
    nop
    
/* Initialize cahce
 */
    jal     flush_cache
    nop
    
/* Enable cache
 */
    mfc0    v0, C0_CONFIG, 0                /* set K0 "cacheable noncoherent" mode */
    and     v0, v0, ~CFG0_K0
    or      v0, v0, CFG_C_CACHABLE          /* = 0x03 */
    mtc0    v0, C0_CONFIG, 0
    nop

/* Clear Translation Lookaside Buffer (TLB)
 */
    jal     init_tlb                        /* clear the tlb */
    nop

/* End of CPU initialization, ready to start kernel
 */
    move    a0, zero                        /* Set argc passed to main */
    jal     bsp_start
    nop

/* Kernel has been shutdown, jump to the "exit" routine
 */
    jal     _sys_exit
    move    a0, v0                          # pass through the exit code

1:
    beq     zero, zero, 1b
    nop

    .set    reorder
    .set    mips32
ENDFRAME(start)

//-----------------------------------------------------------------------------

/*
 * _sys_exit -- Exit from the application. Normally we cause a user trap
 *          to return to the ROM monitor for another run. NOTE: This is
 *          the only other routine we provide in the crt0.o object, since
 *          it may be tied to the "_start" routine. It also allows
 *          executables that contain a complete world to be linked with
 *          just the crt0.o object.
 */

FRAME(_sys_exit,sp,0,ra)
    break   1023
    nop
13:
    b       13b                     /* but loop back just in-case */
    nop
ENDFRAME(_sys_exit)

//-----------------------------------------------------------------------------

/*
 * function: get_memory_size()
 * return:   memory size.
 */
FRAME(get_memory_size,sp,0,ra)
    .set    noreorder
    la      a0, memory_cfg_struct
    lw      a0, 0(a0)               /* Get memory size from struct */
    move    v0, a0                  /* memory_size */
    j       ra
    nop
    .set    reorder
ENDFRAME(get_memory_size)

//-----------------------------------------------------------------------------
// end of start.S
//-----------------------------------------------------------------------------



