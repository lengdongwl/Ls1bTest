/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _LS1X_MIPS_H
#define _LS1X_MIPS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * SR bits that enable/disable interrupts
 */
#if (__mips == 3) || (__mips == 32)
#ifdef ASM
#define SR_INTERRUPT_ENABLE_BITS 0x01
#else
#define SR_INTERRUPT_ENABLE_BITS SR_IE
#endif
#elif __mips == 1
#define SR_INTERRUPT_ENABLE_BITS SR_IEC
#else
#error "mips interrupt enable bits: unknown architecture level!"
#endif

#if defined(__mips_soft_float)
#define MIPS_HAS_FPU 0
#else
#define MIPS_HAS_FPU 1
#endif

#if (__mips == 1)
#define CPU_MODEL_NAME  "ISA Level 1 or 2"
#elif (__mips == 3) || (__mips == 32)
#if defined(__mips64)
#define CPU_MODEL_NAME  "ISA Level 4"
#else
#define CPU_MODEL_NAME  "ISA Level 3"
#endif
#else
#error "Unknown MIPS ISA level"
#endif

/*
 * Define the name of the CPU family.
 */
#define CPU_NAME "MIPS"

/*
 * MIPS Vector numbers for exception conditions. This is a direct map to the causes.
 */
#define MIPS_EXCEPTION_BASE             0

#define MIPS_EXCEPTION_INT              MIPS_EXCEPTION_BASE+0
#define MIPS_EXCEPTION_MOD              MIPS_EXCEPTION_BASE+1
#define MIPS_EXCEPTION_TLBL             MIPS_EXCEPTION_BASE+2
#define MIPS_EXCEPTION_TLBS             MIPS_EXCEPTION_BASE+3
#define MIPS_EXCEPTION_ADEL             MIPS_EXCEPTION_BASE+4
#define MIPS_EXCEPTION_ADES             MIPS_EXCEPTION_BASE+5
#define MIPS_EXCEPTION_IBE              MIPS_EXCEPTION_BASE+6
#define MIPS_EXCEPTION_DBE              MIPS_EXCEPTION_BASE+7
#define MIPS_EXCEPTION_SYSCALL          MIPS_EXCEPTION_BASE+8
#define MIPS_EXCEPTION_BREAK            MIPS_EXCEPTION_BASE+9
#define MIPS_EXCEPTION_RI               MIPS_EXCEPTION_BASE+10
#define MIPS_EXCEPTION_CPU              MIPS_EXCEPTION_BASE+11
#define MIPS_EXCEPTION_OVERFLOW         MIPS_EXCEPTION_BASE+12
#define MIPS_EXCEPTION_TRAP             MIPS_EXCEPTION_BASE+13
#define MIPS_EXCEPTION_VCEI             MIPS_EXCEPTION_BASE+14
/* FPE only on mips2 and higher */
#define MIPS_EXCEPTION_FPE              MIPS_EXCEPTION_BASE+15
#define MIPS_EXCEPTION_C2E              MIPS_EXCEPTION_BASE+16
/* 17-22 reserved */
#define MIPS_EXCEPTION_WATCH            MIPS_EXCEPTION_BASE+23
/* 24-30 reserved */
#define MIPS_EXCEPTION_VCED             MIPS_EXCEPTION_BASE+31

#define MIPS_INTERRUPT_BASE             MIPS_EXCEPTION_BASE+32

/*
 * Some macros to access registers
 */
#define mips_get_sr(_x) \
    do { \
        __asm__ volatile("mfc0 %0, $12; nop" : "=r" (_x) :); \
    } while (0)

#define mips_set_sr(_x) \
    do { \
        register unsigned int __x = (_x); \
        __asm__ volatile("mtc0 %0, $12; nop" : : "r" (__x)); \
    } while (0)

/*
 * Access the Cause register
 */
#define mips_get_cause(_x) \
    do { \
        __asm__ volatile("mfc0 %0, $13; nop" : "=r" (_x) :); \
    } while (0)

#define mips_set_cause(_x) \
    do { \
        register unsigned int __x = (_x); \
        __asm__ volatile("mtc0 %0, $13; nop" : : "r" (__x)); \
    } while (0)

/*
 * Access the Debug Cache Invalidate Control register
 */
#define mips_get_dcic(_x) \
    do { \
        __asm__ volatile("mfc0 %0, $7; nop" : "=r" (_x) :); \
    } while (0)

#define mips_set_dcic(_x) \
    do { \
        register unsigned int __x = (_x); \
        __asm__ volatile("mtc0 %0, $7; nop" : : "r" (__x)); \
    } while (0)

/*
 * Access the Breakpoint Program Counter & Mask registers
 * (_x for BPC, _y for mask)
 */
#define mips_get_bpcrm(_x, _y) \
    do { \
        __asm__ volatile("mfc0 %0, $3;  nop" : "=r" (_x) :); \
        __asm__ volatile("mfc0 %0, $11; nop" : "=r" (_y) :); \
    } while (0)

#define mips_set_bpcrm(_x, _y) \
    do { \
        register unsigned int __x = (_x); \
        register unsigned int __y = (_y); \
        __asm__ volatile("mtc0 %0, $11; nop" : : "r" (__y)); \
        __asm__ volatile("mtc0 %0, $3;  nop" : : "r" (__x)); \
    } while (0)

/*
 * Access the Breakpoint Data Address & Mask registers
 * (_x for BDA, _y for mask)
 */

#define mips_get_bdarm(_x, _y) \
    do { \
        __asm__ volatile("mfc0 %0, $5; nop" : "=r" (_x) :); \
        __asm__ volatile("mfc0 %0, $9; nop" : "=r" (_y) :); \
    } while (0)

#define mips_set_bdarm(_x, _y) \
    do { \
        register unsigned int __x = (_x); \
        register unsigned int __y = (_y); \
        __asm__ volatile("mtc0 %0, $9; nop" : : "r" (__y)); \
        __asm__ volatile("mtc0 %0, $5; nop" : : "r" (__x)); \
    } while (0)

/*
 * Access FCR31
 */
#if (MIPS_HAS_FPU == 1)

#define mips_get_fcr31(_x) \
    do { \
        __asm__ volatile("cfc1 %0, $31; nop" : "=r" (_x) :); \
    } while(0)

#define mips_set_fcr31(_x) \
    do { \
        register unsigned int __x = (_x); \
        __asm__ volatile("ctc1 %0, $31; nop" : : "r" (__x)); \
    } while(0)

#else
#define mips_get_fcr31(_x)
#define mips_set_fcr31(_x)
#endif

/*
 *  Manipulate interrupt mask
 *
 *  mips_unmask_interrupt(_mask)
 *    enables interrupts - mask is positioned so it only needs to be or'ed
 *    into the status reg. This also does some other things !!!! Caution
 *    should be used if invoking this while in the middle of a debugging
 *    session where the client may have nested interrupts.
 *
 *  mips_mask_interrupt(_mask)
 *    disable the interrupt - mask is the complement of the bits to be
 *    cleared - i.e. to clear ext int 5 the mask would be - 0xffff7fff
 *
 *  NOTE: mips_mask_interrupt() used to be disable_int().
 *        mips_unmask_interrupt() used to be enable_int().
 */
#define mips_unmask_interrupt(_mask) \
    do { \
        unsigned int _sr; \
        mips_get_sr(_sr); \
        _sr |= (_mask);   \
        mips_set_sr(_sr); \
    } while (0)

#define mips_mask_interrupt(_mask) \
    do { \
        unsigned int _sr; \
        mips_get_sr(_sr); \
        _sr &= ~(_mask);  \
        mips_set_sr(_sr); \
    } while (0)

#define mips_interrupt_enable()     mips_unmask_interrupt(0x01) // SR_INTERRUPT_ENABLE_BITS
#define mips_interrupt_disable()    mips_mask_interrupt(0x01)   // SR_INTERRUPT_ENABLE_BITS

/******************************************************************************
 * mips access general register
 */
#define _m32c0_mfc0(reg, sel) \
    ({                        \
        register unsigned int __r;           \
        __asm__ volatile ("mfc0 %0, $%1, %2" \
                          : "=d" (__r)       \
                          : "JK" (reg),      \
                            "JK" (sel));     \
        __r; \
    })

#if __mips_isa_rev >= 2
#define _m32c0_mtc0(reg, sel, val) \
    do {                           \
        __asm__ __volatile ("%(mtc0 %z0, $%1, %2; ehb%)" \
                            : \
                            : "dJ" ((unsigned int)(val)), "JK" (reg), "JK" (sel) \
                            : "memory"); \
    } while (0)
#else
#define _m32c0_mtc0(reg, sel, val) \
    do {                           \
        __asm__ __volatile ("%(mtc0 %z0, $%1, %2; ssnop; ssnop; ehb%)" \
                            : \
                            : "dJ" ((unsigned int)(val)), \
                              "JK" (reg), \
                              "JK" (sel)  \
                            : "memory");  \
    } while (0)
#endif

#define mips_mfc0(reg, sel, val)    { val =_m32c0_mfc0(reg, sel); }
#define mips_mtc0(reg, sel, val)    { _m32c0_mtc0(reg, sel, val); }

/******************************************************************************
 * Enable/Disable COUNT/COMPARE of cause bit[27]
 */
#define mips_enable_dc() \
    do { \
        unsigned int _cause;    \
        mips_get_cause(_cause); \
        _cause &= ~(1<<27);     \
        mips_set_cause(_cause); \
    } while (0)

#define mips_disable_dc() \
    do { \
        unsigned int _cause;    \
        mips_get_cause(_cause); \
        _cause |= (1<<27);      \
        mips_set_cause(_cause); \
    } while (0)


#ifdef __cplusplus
}
#endif

#endif /* _LS1X_MIPS_H */


