/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_spi_hw.h
 *
 *  Created on: 2013-11-1
 *      Author: Bian
 */

#ifndef LS1x_SPI_HW_H_
#define LS1x_SPI_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef bit
#define bit(x)  (1<<x)
#endif

/*
 * SPI Flash fast access memory base
 */
#define FLASH_MEMBASE			0xBFC00000

/*
 * spi work at polling mode.
 */
#define LS1x_SPI_FASTREAD	0

/*
 * SPI Registers.
 */
typedef struct LS1x_SPI_regs
{
	volatile unsigned char ctrl;				// 0x00		控制寄存器
	volatile unsigned char sr;					// 0x01		状态寄存器
	union
	{
		volatile unsigned char txfifo;			// 0x02		数据接收寄存器
		volatile unsigned char rxfifo;			// 0x02		数据发送寄存器
	} data;
	volatile unsigned char er;					// 0x03		外部寄存器

	// SPI Flash
	volatile unsigned char param;				// 0x04		SPI Flash 参数控制寄存器
	volatile unsigned char cs;					// 0x05		片选控制寄存器
	volatile unsigned char timing;				// 0x06		SPI Flash 时序控制寄存器
} LS1x_SPI_regs_t;

typedef enum
{
	/* 控制寄存器
	 */
	spi_ctrl_ien			= bit(7),	// SPCR(7)		中断输出使能信号, 高有效
	spi_ctrl_en 			= bit(6),	// SPCR(6)		系统工作使能信号, 高有效
	spi_ctrl_master 		= bit(4),	// SPCR(4)		master模式选择位, 此位一直保持1
	spi_ctrl_cpol 			= bit(3),	// SPCR(3)		时钟极性位
	spi_ctrl_cpha 			= bit(2),	// SPCR(2)		时钟相位位. =1: 相位相反; =0: 相同.
	spi_ctrl_spr_mask 		= 0x03,		// SPCR(1:0)	sclk_o分频设定, 需要与sper的spre一起使用
	spi_ctrl_spr_shift 		= 0,

	/* 状态寄存器
	 */
	spi_sr_iflag 			= bit(7),	// SPSR(7)		中断标志位, =1: 表示有中断申请, 写1则清零.
	spi_sr_txoverflow		= bit(6),	// SPSR(6)		写寄存器溢出标志位, 为1表示已经溢出,写1则清零
	spi_sr_txfull 			= bit(3),	// SPSR(3)		写寄存器满标志, 1表示已经满
	spi_sr_txempty 			= bit(2),	// SPSR(2)		写寄存器空标志, 1表示空
	spi_sr_rxfull 			= bit(1),	// SPSR(1)		读寄存器满标志, 1表示已经满
	spi_sr_rxempty 			= bit(0),	// SPSR(0)		读寄存器空标志, 1表示空

	/* 外部寄存器
	 */
	spi_er_icnt_mask 		= 0xC0,		// SPER(7:6)	在传输完多少个字节后送出中断申请信号
	spi_er_icnt_shift 		= 6,
	spi_er_mode 			= bit(2),	// SPER(2)		spi接口模式控制, =0: 采样与发送时机同时; =1: 采样与发送时机错开半周期.
	spi_er_spre_mask 		= 0x03,		// SPER(1:0)	与spr一起设定分频的比率
	spi_er_spre_shift 		= 0,

	/* SPI Flash参数控制寄存器
	 */
	spi_param_clk_div_mask	= 0xF0,		// SFC_PARAM(7:4)	时钟分频数选择, 分频系数与spre,spr组合相同.
	spi_param_clk_div_shift = 4,
	spi_param_dual_io		= bit(3),	// SFC_PARAM(3)		使用双I/O模式，优先级高于快速读模式
	spi_param_fast_read		= bit(2),	// SFC_PARAM(2)		使用快速读模式
	spi_param_burst_en		= bit(1),	// SFC_PARAM(1)		spi flash 支持连续地址读模式
	spi_param_memory_en		= bit(0),	// SFC_PARAM(0)		spi flash 读使能, 无效时csn[0]可由软件控制.

	/* SPI 片选控制寄存器
	 */
	spi_chip_sel_3			= bit(7),	// csn引脚输出值
	spi_chip_sel_2			= bit(6),
	spi_chip_sel_1			= bit(5),
	spi_chip_sel_0			= bit(4),
	spi_chip_sel_3_en		= bit(3),	// 为1时对应位的cs线由7:4位控制
	spi_chip_sel_2_en		= bit(2),
	spi_chip_sel_1_en		= bit(1),
	spi_chip_sel_0_en		= bit(0),

	/* SPI Flash时序控制寄存器
	 */
	spi_timing_tcsh_mask	= 0x03,		// SPI Flash 的片选信号最短无效时间，以分频后时钟周期T计算
	spi_timing_tcsh_shift	= 0,
	spi_timing_tcsh_1		= 0x00,
	spi_timing_tcsh_2		= 0x01,
	spi_timing_tcsh_4		= 0x02,
	spi_timing_tcsh_8		= 0x03,
} SPI_controller_t;

/*
 * 分频系数设置值
 */
typedef struct LS1x_SPI_clkdiv
{
	unsigned char	spre;
	unsigned char	spr;
	unsigned int    sprate;
} spi_clkdiv_t;

#ifdef __cplusplus
}
#endif

#endif /* LS1x_SPI_HW_H_ */

