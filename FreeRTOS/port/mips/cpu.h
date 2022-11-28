/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * cpu.h -- cpu related defines
 *
 * MIPS32 of Loongson1x Core. 2013.7.10 Bian
 */

#ifndef _LS1X_CPU_H
#define _LS1X_CPU_H

/*
 * 龙芯 Loongson 1x
 */

/******************************************************************************
 * LS232 define bellow
 *****************************************************************************/

/*
 * memory configuration and mapping
 */
#define K0BASE  0x80000000
#define K0SIZE  0x20000000
#define K1BASE  0xa0000000
#define K1SIZE  0x20000000

#define KUBASE  0
#define KUSIZE  0x80000000

/*
 * Exception Vectors
 */
#define T_VEC   (K0BASE+0x000)      /* tlbmiss vector */
#if (__mips == 3)
#define X_VEC   (K0BASE+0x080)      /* xtlbmiss vector */
#endif
#define C_VEC   (K0BASE+0x100)      /* cache error vector */
#define E_VEC   (K0BASE+0x180)      /* exception vector */

/*
 * Address conversion macros
 */
#ifdef CLANGUAGE
#define CAST(as) (as)
#else
#define CAST(as)
#endif

#define K0_TO_K1(x)     (CAST(unsigned)(x)|0xA0000000)  /* kseg0 to kseg1 */
#define K1_TO_K0(x)     (CAST(unsigned)(x)&0x9FFFFFFF)  /* kseg1 to kseg0 */
#define K0_TO_PHYS(x)   (CAST(unsigned)(x)&0x1FFFFFFF)  /* kseg0 to physical */
#define K1_TO_PHYS(x)   (CAST(unsigned)(x)&0x1FFFFFFF)  /* kseg1 to physical */
#define PHYS_TO_K0(x)   (CAST(unsigned)(x)|0x80000000)  /* physical to kseg0 */
#define PHYS_TO_K1(x)   (CAST(unsigned)(x)|0xA0000000)  /* physical to kseg1 */

/*
 *  Cache size constants
 */
#define MINCACHE        0x1000      /* 1*4096    4k */
#define MAXCACHE        0x2000      /* 2*4096    8k */

/*
 * LS232 configuration register definitions
 */

/*
 * Config 0 Intialize Data = 0x00030932
 */
#define CFG0_M          0x80000000  /* bit 31:      whether implement config1 */
#define CFG0_BE         0x00008000  /* bit 15:      big endian or little endian */
#define CFG0_AT         0x00006000  /* bit 14..13:  0: MIPS32 Compatiable
                                                    1: Only Can Access 32 bit Address for MIPS64 Compatiable
                                                    2: MIPS64 Compatiable
                                                    3: Reserved */
#define CFG0_AR         0x00001B00  /* bit 12..10:  0: release1
                                                    1: release2
                                                    2-7: Reserved */
#define CFG0_MT         0x00000380  /* bit 9..7:    MMU Type */
#define CFG0_VI         0x00000008  /* bit 3:       if I cache is virtual cache */
#define CFG0_K0         0x00000007  /* bit 2..0:    KSEG0 coherency algorithm
                                                    7 - Uncached Accelerated
                                                    3 - Cachable
                                                    2 –Uncached */
/* LS232 cache mode
 */
#define CFG_C_UNCACHED          2
#define CFG_C_CACHABLE          3
#define CFG_C_ACCELERATED       7

/*
 * Config 1
 */
#define CFG1_M          0x80000000  /* bit 31:      whether implement config2 */
#define CFG1_MMU        0x7E000000  /* bit 30..25:  MMU = TLB Size - 1 */
#define CFG1_IS         0x01C00000  /* bit 24..22:  Icache 每路的组数  */
#define CFG1_IL         0x00380000  /* bit 21..19:  Icache 每行的大小 */
#define CFG1_IA         0x00070000  /* bit 18..16:  Icache 组相联数 */
#define CFG1_DS         0x0000E000  /* bit 15..13:  Dcache 每路的组数 */
#define CFG1_DL         0x00001C00  /* bit 12..10:  Dcache 每行的大小 */
#define CFG1_DA         0x00000380  /* bit 9..7:    Dcache 组相联数 */
#define CFG1_C2         0x00000040  /* bit 6:       Coprocessor2是否实现 */
#define CFG1_PC         0x00000010  /* bit 4:       Performance Counter 是否实现 */
#define CFG1_WR         0x00000008  /* bit 3:       Watch寄存器是否实现 */
#define CFG1_CA         0x00000004  /* bit 2:       代码压缩是否实现 */
#define CFG1_EP         0x00000002  /* bit 1:       EJTAG是否实现 */
#define CFG1_FP         0x00000001  /* bit 0:       浮点功能单元是否实现 */

/*
 * Config 2
 */
#define CFG2_M          0x80000000  /* bit 31:      whether implement config3 */

/*
 * Config 3
 */
#define CFG3_M          0x80000000  /* bit 31:      whether implement config2? */
#define CFG3_DSPP       0x00000600  /* bit 10..9:   DSP是否实现 */
#define CFG3_VEIC       0x00000040  /* bit 6:       是否实现了外部中断 */
#define CFG3_VInt       0x00000020  /* bit 5:       是否实现了向量中断 */
#define CFG3_SP         0x00000010  /* bit 4:       是否支持小物理页 */
#define CFG3_MT         0x00000004  /* bit 2:       是否实现了MTASE */
#define CFG3_SM         0x00000002  /* bit 1:       是否实现了Smart ASE */
#define CFG3_TL         0x00000001  /* bit 0:       是否实现了Trace 逻辑 */

/*
 * Config 6
 */
#define CFG6_RTI        0x00000008  /* bit 3:       是否实现实时中断 */
#define CFG6_BRMASK     0x00000007  /* bit 2..0:    分支预测方式 */
#define CFG6_BR_GSHARE  0x00000000
#define CFG6_BR_PC      0x00000001
#define CFG6_BR_JMP     0x00000002
#define CFG6_BR_NJMP    0x00000003
#define CFG6_BR_JMPF    0x00000004
#define CFG6_BR_JMPB    0x00000005

/*
 * LS232 cache operations
 */
#define Index_Invalidate_I      0x0
#define Index_Store_Tag_I       0x8
#define Hit_Invalidate_I        0x10
#define Hit_Lock_I              0x1C

#define Index_Writeback_Inv_D   0x1
#define Index_Load_Tag_D        0x5
#define Index_Store_Tag_D       0x9
#define Hit_Invalidate_D        0x11
#define Hit_Writeback_Inv_D     0x15
#define Hit_Lock_D              0x1D

/*
 * LS232 TLB resource defines
 */
#define N_TLB_ENTRIES       32

#define TLBHI_VPN2MASK      0xffffe000
#define TLBHI_PIDMASK       0x000000ff
#define TLBHI_NPID          256

#define TLBLO_PFNMASK       0x3fffffc0
#define TLBLO_PFNSHIFT      6
#define TLBLO_D             0x00000004      /* writeable */
#define TLBLO_V             0x00000002      /* valid bit */
#define TLBLO_G             0x00000001      /* global access bit */
#define TLBLO_CMASK         0x00000038      /* cache algorithm mask */
#define TLBLO_CSHIFT        3

#define TLBLO_UNCACHED      (CFG_C_UNCACHED<<TLBLO_CSHIFT)
#define TLBLO_CACHABLE      (CFG_C_CACHABLE<<TLBLO_CSHIFT)
#define TLBLO_ACCELERATED   (CFG_C_ACCELERATED<<TLBLO_CSHIFT)

#define TLBINX_PROBE        0x80000000
#define TLBINX_INXMASK      0x0000003f

#define TLBRAND_RANDMASK    0x0000003f

#define TLBCTXT_BASEMASK    0xff800000
#define TLBCTXT_BASESHIFT   23

#define TLBCTXT_VPN2MASK    0x007ffff0
#define TLBCTXT_VPN2SHIFT   4

#define TLBPGMASK_MASK      0x01ffe000

/*
 * status register definition
 */
#define SR_CUMASK           0xf0000000      /* bit 31..28:  coproc usable bits */
#define SR_CU3              0x80000000      /* Coprocessor 3 usable */
#define SR_CU2              0x40000000      /* Coprocessor 2 usable */
#define SR_CU1              0x20000000      /* Coprocessor 1 usable, XXX FPU is present */
#define SR_CU0              0x10000000      /* Coprocessor 0 usable */

#define SR_RP               0x08000000      /* Reduced power operation ? */
#define SR_FR               0x04000000      /* XXX 0: 32-bits register, FPU is compatible with MIPS-I
                                             *     1: 64-bits register */
#define SR_RE               0x02000000      /* bit 25: Reverse endian in user mode */

#define SR_BEV              0x00400000      /* bit 22: Use boot exception vectors */
#define SR_TS               0x00200000      /* TLB shutdown ? */
#define SR_SR               0x00100000      /* bit 20:  Soft reset */
#define SR_CH               0x00040000      /* Cache hit ? */
#define SR_CE               0x00020000      /* Use cache ECC ? */
#define SR_DE               0x00010000      /* Disable cache exceptions */

/*
 *  status register interrupt masks and bits
 */
#define SR_IMASK            0x0000ff00      /* bit 15..8: Interrupt mask */
#define SR_IMASKSHIFT       8
#define SR_IM0              0x00000100
#define SR_IM1              0x00000200
#define SR_IM2              0x00000400
#define SR_IM3              0x00000800
#define SR_IM4              0x00001000
#define SR_IM5              0x00002000
#define SR_IM6              0x00004000
#define SR_IM7              0x00008000

//#define SR_SINT0            SR_IM0          /* software interrupt 0 */
//#define SR_SINT1            SR_IM1          /* software interrupt 1 */

#define SR_KSMASK           0x00000018      /* bit 4..3: Kernel mode mask */
#define SR_KSUSER           0x00000010      /* User mode */
#define SR_KSSUPER          0x00000008      /* Supervisor mode */
#define SR_KSKERNEL         0x00000000      /* Kernel mode */
#define SR_ERL              0x00000004      /* bit 2: Error level */
#define SR_EXL              0x00000002      /* bit 1: Exception level */
#define SR_IE               0x00000001      /* bit 0: Interrupts enabled */

/*
 * Cause Register
 */
#define CAUSE_BD            0x80000000      /* bit 31:      Branch delay slot */
#define CAUSE_BT            0x40000000      /* bit 30:      Branch Taken */
#define CAUSE_CEMASK        0x30000000      /* bit 29..28:  coprocessor error */
#define CAUSE_CESHIFT       28

#define CAUSE_DC            0x08000000      /* bit 27:      关掉Count寄存器。DC=1时关掉count寄存器 */
#define CAUSE_PCI           0x04000000      /* bit 26:      Performance counter中断,用来指示是否有待处理的PC中断 */
#define CAUSE_IV            0x00800000      /* bit 23:      指示中断向量是否用普通的异常向量（0表示是，1表示用特殊向量）*/

#define CAUSE_IPMASK        0x0000FF00      /* Pending interrupt mask */
#define CAUSE_IPSHIFT       8

#define CAUSE_EXCMASK       0x0000003C      /* Cause code bits */
#define CAUSE_EXCSHIFT      2

/*
 *  Coprocessor 0 registers
 */
#define C0_INX              $0      /* tlb index */
#define C0_RAND             $1      /* tlb random */
#define C0_TLBLO0           $2      /* tlb entry low 0 */
#define C0_TLBLO1           $3      /* tlb entry low 1 */
#define C0_CTXT             $4      /* tlb context */
#define C0_PAGEMASK         $5      /* tlb page mask */
#define C0_WIRED            $6      /* number of wired tlb entries */
#define C0_BADVADDR         $8      /* bad virtual address */
#define C0_COUNT            $9      /* cycle count */
#define C0_TLBHI            $10     /* tlb entry hi */
#define C0_COMPARE          $11     /* cyccle count comparator  */
#define C0_SR               $12     /* status register */
#define C0_STATUS           $12
#define C0_CAUSE            $13     /* exception cause */
#define C0_EPC              $14     /* exception pc */
#define C0_PRID             $15     /* revision identifier */
#define C0_CONFIG           $16     /* configuration register */
#define C0_LLADDR           $17     /* linked load address */
#define C0_WATCHLO          $18     /* watchpoint trap register */
#define C0_WATCHHI          $19     /* watchpoint trap register */
//#define C0_XCTXT          $20     /* extended tlb context ? */
#define C0_DEBUG            $23     /* Debug */
#define C0_DEPC             $24     /* EJTAG debug 例外程序计数器 */
#define C0_PCHI             $25     /* Performance Counter 性能计数器的高半部分 */
#define C0_ECC              $26     /* secondary cache ECC control ? */
#define C0_CACHEERR         $27     /* cache error status ? */
#define C0_TAGLO            $28     /* cache tag lo */
#define C0_TAGHI            $29     /* cache tag hi */
#define C0_ERRPC            $30     /* cache error pc */
#define C0_DESAVE           $31     /* EJTAG debug 例外保存寄存器 */

#define C1_REVISION         $0
#define C1_STATUS           $31

#endif /* _LS1X_CPU_H */


