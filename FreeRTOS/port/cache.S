/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * cache.S - memory and cache functions
 *
 * mips32 cache operations
 */

#include "regdef.h"
#include "cpu.h"
#include "asm.h"

    .data

memory_size:
    .word   0
dcache_size:
    .word   0
icache_size:
    .word   0

    .data
icache_linesize:
    .word   0
dcache_linesize:
    .word   0
icache_nway:
    .word   0
dcache_nway:
    .word   0

    .text

/**************************************************************************
 * predefine loop macro, shared mips3 and mips32
 **************************************************************************/

/*
 * cacheop macro to automate cache operations
 * first some helpers...
 */
#define _mincache(size, maxsize) \
    bltu    size, maxsize, 8f ;  \
    move    size, maxsize ;      \
8:

#define _align(tmp, minaddr, maxaddr, linesize) \
    subu    tmp, linesize, 1 ;   \
    not     tmp ;                \
    and     minaddr, tmp ;       \
    addu    maxaddr, -1 ;        \
    and     maxaddr, tmp

/*
 * This is a bit of a hack really because it relies on minaddr=a0
 */
#define _doop1(op1)    \
    cache   op1, 0(a0)

#define _doop2(op1, op2)  \
    cache   op1, 0(a0) ;  \
    cache   op2, 0(a0)

/*
 * specials for cache initialization
 */
#define _doop1lw1(op1)    \
    cache   op1, 0(a0) ;  \
    lw      zero, 0(a0) ; \
    cache   op1, 0(a0)

#define _doop121(op1, op2) \
    cache   op1, 0(a0) ;   \
    nop;                   \
    cache   op2, 0(a0) ;   \
    nop;                   \
    cache   op1, 0(a0)

#define _oploopn(minaddr, maxaddr, linesize, tag, ops) \
    .set    noreorder ;             \
7:  _doop##tag##ops ;               \
    bne     minaddr, maxaddr, 7b ;  \
    addu    minaddr, linesize ;     \
    .set    reorder

/*
 * finally the cache operation macros
 */
#define icacheopn(kva, n, cache_size, cache_linesize, tag, ops) \
    _mincache(n, cache_size);   \
    blez    n, 9f ;             \
    addu    n, kva ;            \
    _align(t1, kva, n, cache_linesize) ; \
    _oploopn(kva, n, cache_linesize, tag, ops) ; \
9:

#define vcacheopn(kva, n, cache_size, cache_linesize, tag, ops) \
    blez    n, 9f ;             \
    addu    n, kva ;            \
    _align(t1, kva, n, cache_linesize) ; \
    _oploopn(kva, n, cache_linesize, tag, ops) ; \
9:

#define icacheop(kva, n, cache_size, cache_linesize, op) \
    icacheopn(kva, n, cache_size, cache_linesize, 1, (op))

#define vcacheop(kva, n, cache_size, cache_linesize, op) \
    vcacheopn(kva, n, cache_size, cache_linesize, 1, (op))
 

/******************************************************************************
 * MIPS32 -- LS232
 ******************************************************************************/

    .text

/*
 * static void _size_cache()    LS232
 *
 * Internal routine to determine cache sizes by looking at LS232 config
 * register.  Sizes are returned in registers, as follows:
 *  t2  icache size
 *  t3  dcache size
 *  t4  icache line size
 *  t5  dcache line size
 *  t6  icache nway
 *  t7  dcache nway
 */
LEAF(_size_cache)
    mfc0    t0, C0_CONFIG, 1

/*
 * bit 24..22   =001    Icache set size equal 64×2^1=128
 * bit 21..19   =100    Icache line size equal 2×2^4=32
 * bit 18..16   =011    Icache associate size equal +1=4
 */
    /*
     * icache config
     */
    srl     t1, t0, 19      // ILINE_SHIFT  // IL: 21..19:  Icache 每行的大小
    andi    t1, 7                           // 行大小是 2 × 2^IL 字节
    li      t2, 2                           // t2 = 2
    sll     t2, t1                          // t2 = t2 × 2^IL
    move    t4, t2                          // ---> icache line size

    srl     t1, t0, 22      // ISIZE_SHIFT  // IS: 24..22:  Icache 每路的组数
    andi    t1, 7                           // 组数大小为 64 × 2^IS
    sll     t2, 6                           // t2 = t2 × 64
    sll     t2, t1                          // t2 = t2 × 2^IS ---> icache set size

    srl     t1, t0, 16      // INWAY_SHIFT  // IA: 18..16:  Icache 组相联数
    andi    t1, 7                           // IA + 1 路组相联
    addi    t6, t1, 1                       // t6 = IA + 1 ---> icache nway

    mul     t2, t2, t6                      // t2 = t2 × (IA + 1) ---> icache size.

/*
 * bit 15..13   =001    Dcache set size equal 64×2^1=128
 * bit 12..10   =100    Dcache line size equal 2×2^4=32
 * bit 9..7     =011    Dcache associate size equal +1=4
 */
    /*
     * dcache config
     */
    srl     t1, t0, 10      // DLINE_SHIFT  // DL: 12..10:  Dcache 每行的大小
    andi    t1, 7                           // 行大小是 2 × 2^DL 字节
    li      t3, 2                           // t3 = t3 × 2
    sll     t3, t1                          // t3 = t3 × 2^DL
    move    t5, t3                          // ---> dcache line size

    srl     t1, t0, 13      // DSIZE_SHIFT  // DS: 15..13:  Dcache 每路的组数 */
    andi    t1, 7                           // 组数大小为 64 × 2^DS
    sll     t3, 6                           // t3 = t3 × 64
    sll     t3, t1                          // t3 = t3 × 2^DS ---> dcache set size

    srl     t1, t0, 7       // DNWAY_SHIFT  // DA: 9..7:    Dcache 组相联数 */
    andi    t1, 7                           // DA + 1 路组相联
    addi    t7, t1, 1                       // t7 = DA + 1 ---> dcahce nway

    mul     t3, t3, t7                      // t3 = t3 × (DA + 1) ---> dcache size.

    j       ra
    nop
ENDFRAME(_size_cache)

/*
 * void config_cache()   LS232
 *
 * Work out size of I & D caches, assuming they are already initialised.
 */
LEAF(config_cache)
    lw      t0, icache_size
    bgtz    t0, 1f                          // already known?
    nop
    move    v0, ra
    bal     _size_cache
    nop                                     // needed nop here?
    move    ra, v0

    sw      t2, icache_size
    sw      t3, dcache_size
    sw      t4, icache_linesize
    sw      t5, dcache_linesize
    sw      t6, icache_nway
    sw      t7, dcache_nway
1:
    j       ra
    nop
ENDFRAME(config_cache)

/*
 * void _init_cache()   LS232
 */
LEAF(_init_cache)
    /*
     * First work out the sizes
     */
    move    v0, ra
    bal     _size_cache
    nop                                     // needed nop here?
    move    ra, v0

    /*
     * The caches may be in an indeterminate state,
     * so we force good parity into them by doing an
     * invalidate, load/fill, invalidate for each line.
     */

    /*
     * disable all i/u and cache exceptions
     */
    mfc0    v0, C0_SR
    and     v1, v0, ~SR_IE
    or      v1, v1, SR_DE                   // disable cache exception, Is LS232 have this bit?
    mtc0    v1, C0_SR

    mtc0    zero, C0_TAGLO
    mtc0    zero, C0_TAGHI

    /*
     * assume bottom of RAM will generate good parity for the cache
     */
    li      a0, PHYS_TO_K0(0)
    move    a2, t2                          // icache_size
    move    a3, t4                          // icache_linesize
    move    a1, a2
    //icacheopn(a0,a1,a2,a3,121,(Index_Store_Tag_I,Fill_I))
    icacheop(a0,a1,a2,a3,Index_Store_Tag_I)

    li      a0, PHYS_TO_K0(0)
    move    a2, t3                          // dcache_size
    move    a3, t5                          // dcache_linesize
    move    a1, a2
    //icacheopn(a0,a1,a2,a3,1lw1,(Index_Store_Tag_D))
    icacheop(a0,a1,a2,a3,Index_Store_Tag_D)

    mtc0    v0, C0_SR
    j       ra
    nop
ENDFRAME(_init_cache)

/*
 * void flush_cache (void)   LS232
 *
 * Flush and invalidate all caches
 */
LEAF(flush_cache)
    lw      a2, icache_size
    blez    a2, 1f
    nop
    lw      a3, icache_linesize
    li      a0, PHYS_TO_K0(0)
    move    a1, a2
    icacheop(a0,a1,a2,a3,Index_Invalidate_I)

    lw      a2, dcache_size
    blez    a2, 1f
    nop
    lw      a3, dcache_linesize
    li      a0, PHYS_TO_K0(0)
    move    a1, a2
    icacheop(a0,a1,a2,a3,Index_Writeback_Inv_D)
1:
    j       ra
    nop
ENDFRAME(flush_cache)

/*
 * FIXME: void flush_dcache (void)   LS232
 *
 * Flush and invalidate data cache
 */
LEAF(flush_dcache)
    lw      a2, dcache_size
    blez    a2, 1f
    nop
    lw      a3, dcache_linesize
    li      a0, PHYS_TO_K0(0)
    move    a1, a2
    icacheop(a0,a1,a2,a3,Index_Writeback_Inv_D)
1:
    j       ra
    nop
ENDFRAME(flush_dcache)

/*
 * FIXME: void flush_icache (void)   LS232
 *
 * Flush and invalidate instruction cache
 */
LEAF(flush_icache)
    lw      a2, icache_size
    blez    a2, 1f
    nop
    lw      a3, icache_linesize
    li      a0, PHYS_TO_K0(0)
    move    a1, a2
    icacheop(a0,a1,a2,a3,Index_Invalidate_I)
1:
    j       ra
    nop
ENDFRAME(flush_icache)

/*
 * void flush_cache_nowrite (void)   LS232
 *
 * Invalidate all caches
 */
LEAF(flush_cache_nowrite)
    mfc0    v0, C0_SR
    and     v1, v0, ~SR_IE
    mtc0    v1, C0_SR

    mtc0    zero, C0_TAGLO
    mtc0    zero, C0_TAGHI

    lw      a2, icache_size
    blez    a2, 1f
    nop
    lw      a3, icache_linesize
    li      a0, PHYS_TO_K0(0)
    move    a1, a2
    icacheop(a0,a1,a2,a3,Index_Invalidate_I)

    lw      a2, dcache_size
    lw      a3, dcache_linesize
    li      a0, PHYS_TO_K0(0)
    move    a1, a2
    icacheop(a0,a1,a2,a3,Index_Store_Tag_D)

1:
    mtc0    v0, C0_SR
    j       ra
    nop
ENDFRAME(flush_cache_nowrite)

/*
 * void clean_cache (unsigned kva, size_t n)   LS232
 *
 * Writeback and invalidate address range in all caches
 */
LEAF(clean_cache)
//XLEAF(clear_cache)
    lw      a2, icache_size
    blez    a2, 1f
    nop
    lw      a3, icache_linesize
    /*
     * save kva & n for subsequent loop
     */
    move    t8, a0
    move    t9, a1
    vcacheop(a0,a1,a2,a3,Hit_Invalidate_I)

    lw      a2, dcache_size
    lw      a3, dcache_linesize
    /*
     * restore kva & n
     */
    move    a0, t8
    move    a1, t9
    vcacheop(a0,a1,a2,a3,Hit_Writeback_Inv_D)
1:
    j       ra
    nop
ENDFRAME(clean_cache)

/*
 * void clean_dcache (unsigned kva, size_t n)   LS232
 *
 * Writeback and invalidate address range in primary data cache
 */
LEAF(clean_dcache)
    lw      a2, dcache_size
    blez    a2, 1f
    nop
    lw      a3, dcache_linesize

    vcacheop(a0,a1,a2,a3,Hit_Writeback_Inv_D)
1:
    j       ra
    nop
ENDFRAME(clean_dcache)

/*
 * void clean_dcache_indexed (unsigned kva, size_t n)   LS232
 *
 * Writeback and invalidate indexed range in primary data cache
 */
LEAF(clean_dcache_indexed)
    lw      a2, dcache_size
    blez    a2, 1f
    nop
    lw      a3, dcache_linesize

    icacheop(a0,a1,a2,a3,Index_Writeback_Inv_D)
1:
    j       ra
    nop
ENDFRAME(clean_dcache_indexed)

/*
 * void clean_dcache_nowrite (unsigned kva, size_t n)   LS232
 *
 * Invalidate an address range in primary data cache
 */
LEAF(clean_dcache_nowrite)
    lw      a2, dcache_size
    blez    a2, 1f
    nop
    lw      a3, dcache_linesize

    vcacheop(a0,a1,a2,a3,Hit_Invalidate_D)
1:
    j       ra
    nop
ENDFRAME(clean_dcache_nowrite)

/*
 * void clean_dcache_nowrite_indexed (unsigned kva, size_t n)   LS232
 *
 * Invalidate indexed range in primary data cache
 */
LEAF(clean_dcache_nowrite_indexed)
    mfc0    v0, C0_SR
    and     v1, v0, ~SR_IE
    mtc0    v1, C0_SR

    mtc0    zero, C0_TAGLO
    mtc0    zero, C0_TAGHI

    lw      a2, dcache_size
    blez    a2, 1f
    nop
    lw      a3, dcache_linesize

    icacheop(a0,a1,a2,a3,Index_Store_Tag_D)
1:
    mtc0    v0, C0_SR
    j       ra
    nop
ENDFRAME(clean_dcache_nowrite_indexed)

/*
 * void clean_icache (unsigned kva, size_t n)   LS232
 *
 * Invalidate address range in primary instruction cache
 */
LEAF(clean_icache)
    lw      a2, icache_size
    blez    a2, 1f
    nop
    lw      a3, icache_linesize

    vcacheop(a0,a1,a2,a3,Hit_Invalidate_I)
1:
    j       ra
    nop
ENDFRAME(clean_icache)

/*
 * void clean_icache_indexed (unsigned kva, size_t n)   LS232
 *
 * Invalidate indexed range in primary instruction cache
 */
LEAF(clean_icache_indexed)
    lw      a2, icache_size
    blez    a2, 1f
    nop
    lw      a3, icache_linesize

    icacheop(a0,a1,a2,a3,Index_Invalidate_I)
1:
    j       ra
    nop
ENDFRAME(clean_icache_indexed)

/**************************************************************************
 * get_memory_conf - get memory configuration  LS232
 **************************************************************************/

LEAF(get_memory_conf)
    lw      t6, memory_size
    sw      t6, 0(a0)
    lw      t7, icache_size
    sw      t7, 4(a0)
    lw      t8, dcache_size
    sw      t8, 8(a0)
    j       ra
    nop
ENDFRAME(get_memory_conf)

//*****************************************************************************
//*****************************************************************************

/*
 * void set_memory_size(memory_size)
 *
 * config_memory()'s memory size gets written into memory_size here.
 * Now we don't need to call config_cache() with memory size - New to IDTC6.0
 */

LEAF(set_memory_size)
    sw      a0, memory_size
    j       ra
    nop
ENDFRAME(set_memory_size)


