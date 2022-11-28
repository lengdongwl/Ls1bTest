/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
 
#include "regdef.h"
#include "cpu.h"
#include "mips.h"

static const char *const cause_strings[32] =
{
  /*  0 */ "Int",
  /*  1 */ "TLB Mods",
  /*  2 */ "TLB Load",
  /*  3 */ "TLB Store",
  /*  4 */ "Address Load",
  /*  5 */ "Address Store",
  /*  6 */ "Instruction Bus Error",
  /*  7 */ "Data Bus Error",
  /*  8 */ "Syscall",
  /*  9 */ "Breakpoint",
  /* 10 */ "Reserved Instruction",
  /* 11 */ "Coprocessor Unuseable",
  /* 12 */ "Overflow",
  /* 13 */ "Trap",
  /* 14 */ "Instruction Virtual Coherency Error",
  /* 15 */ "FP Exception",
  /* 16 */ "Reserved 16",
  /* 17 */ "Reserved 17",
  /* 18 */ "Reserved 18",
  /* 19 */ "Reserved 19",
  /* 20 */ "Reserved 20",
  /* 21 */ "Reserved 21",
  /* 22 */ "Reserved 22",
  /* 23 */ "Watch",
  /* 24 */ "Reserved 24",
  /* 25 */ "Reserved 25",
  /* 26 */ "Reserved 26",
  /* 27 */ "Reserved 27",
  /* 28 */ "Reserved 28",
  /* 29 */ "Reserved 29",
  /* 30 */ "Reserved 30",
  /* 31 */ "Data Virtual Coherency Error"
};

struct regdef
{
	int   offset;
	char *name;
};

static const struct regdef dumpregs[] =
{
    { R_AT, "at" }, { R_V0, "v0" }, { R_V1, "v1" }, { R_A0, "a0" }, 
    { R_A1, "a1" }, { R_A2, "a2" }, { R_A3, "a3" }, { R_T0, "t0" }, 
    { R_T1, "t1" }, { R_T2, "t2" }, { R_T3, "t3" }, { R_T4, "t4" },
    { R_T5, "t5" }, { R_T6, "t6" }, { R_T7, "t7" }, { R_S0, "s0" },
    { R_S1, "s1" }, { R_S2, "s2" }, { R_S3, "s3" }, { R_S4, "s4" },
    { R_S5, "s5" }, { R_S6, "s6" }, { R_S7, "s7" }, { R_T8, "t8" },
    { R_T9, "t9" }, { R_K0, "k0" }, { R_K1, "k1" }, { R_GP, "gp" },
    { R_SP, "sp" }, { R_FP, "fp" }, { R_RA, "ra" },
#if 0
    { R_SR, "status" },
    { R_BADVADDR, "badvaddr" },
    { R_EPC, "epc" },
    { R_MDLO, "mdlo" },
    { R_MDHI, "mdhi" },
    { R_BADVADDR, "badvaddr" },
    { R_CAUSE, "cause" },
#endif
    { -1, NULL }
};

void c_exception_handler(unsigned int *sp)
{
    int i, j;
    unsigned sr, cause, epc;
	unsigned *val = (unsigned *)sp;

    mips_interrupt_disable();
    
	for (i=0; dumpregs[i].offset > -1; i++)
	{
		printk("   %s", dumpregs[i].name);
		for (j=0; j<7-strlen(dumpregs[i].name); j++)
           printk(" ");

		printk("  %08X%c", val[dumpregs[i].offset], (i%3) ? '\t' : '\n');
	}

	printk("\n");

	mips_get_sr(sr);
	mips_get_cause(cause);
    mips_mfc0(14, 0, epc);         /* C0_EPC */
	printk("status = %08X, cause = %08X, epc = %08X\r\n", sr, cause, epc);
	
	cause = (cause >> 2) & 0x1f;
	printk("CPU Exception[%i]: %s", cause, cause_strings[cause]);
	
	while (1)           // 产生例外之后... dead loop 
      ;
}

