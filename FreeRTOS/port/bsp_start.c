/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>

#include "cpu.h"
#include "mips.h"
#include "console.h"

extern void except_common_entry(void);
extern void vPortInitISR(void);
extern void init_tick_step(void);
extern int main(void);

void bsp_start(void)
{
    mips_interrupt_disable();

    /* 
     * install exec vec. Data <==> Instruction must use K1 address
     */
	memcpy((void *)K0_TO_K1(T_VEC), except_common_entry, 40);
	memcpy((void *)K0_TO_K1(C_VEC), except_common_entry, 40);
	memcpy((void *)K0_TO_K1(E_VEC), except_common_entry, 40);

	/*
     * 还没有安装 tick 中断之前, 用于延时
     */
    init_tick_step();

    /* console */
    console_init(115200);

    printk("console initialized.\r\n");
    
	/* Initialise the ISR table */
	vPortInitISR();
	
    mips_interrupt_enable();

    /*
     * 跳转到 c 主程序执行. TODO 汇编跳转, c++ main
     */
    main();
}

