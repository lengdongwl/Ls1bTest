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
	volatile unsigned char ctrl;				// 0x00		���ƼĴ���
	volatile unsigned char sr;					// 0x01		״̬�Ĵ���
	union
	{
		volatile unsigned char txfifo;			// 0x02		���ݽ��ռĴ���
		volatile unsigned char rxfifo;			// 0x02		���ݷ��ͼĴ���
	} data;
	volatile unsigned char er;					// 0x03		�ⲿ�Ĵ���

	// SPI Flash
	volatile unsigned char param;				// 0x04		SPI Flash �������ƼĴ���
	volatile unsigned char cs;					// 0x05		Ƭѡ���ƼĴ���
	volatile unsigned char timing;				// 0x06		SPI Flash ʱ����ƼĴ���
} LS1x_SPI_regs_t;

typedef enum
{
	/* ���ƼĴ���
	 */
	spi_ctrl_ien			= bit(7),	// SPCR(7)		�ж����ʹ���ź�, ����Ч
	spi_ctrl_en 			= bit(6),	// SPCR(6)		ϵͳ����ʹ���ź�, ����Ч
	spi_ctrl_master 		= bit(4),	// SPCR(4)		masterģʽѡ��λ, ��λһֱ����1
	spi_ctrl_cpol 			= bit(3),	// SPCR(3)		ʱ�Ӽ���λ
	spi_ctrl_cpha 			= bit(2),	// SPCR(2)		ʱ����λλ. =1: ��λ�෴; =0: ��ͬ.
	spi_ctrl_spr_mask 		= 0x03,		// SPCR(1:0)	sclk_o��Ƶ�趨, ��Ҫ��sper��spreһ��ʹ��
	spi_ctrl_spr_shift 		= 0,

	/* ״̬�Ĵ���
	 */
	spi_sr_iflag 			= bit(7),	// SPSR(7)		�жϱ�־λ, =1: ��ʾ���ж�����, д1������.
	spi_sr_txoverflow		= bit(6),	// SPSR(6)		д�Ĵ��������־λ, Ϊ1��ʾ�Ѿ����,д1������
	spi_sr_txfull 			= bit(3),	// SPSR(3)		д�Ĵ�������־, 1��ʾ�Ѿ���
	spi_sr_txempty 			= bit(2),	// SPSR(2)		д�Ĵ����ձ�־, 1��ʾ��
	spi_sr_rxfull 			= bit(1),	// SPSR(1)		���Ĵ�������־, 1��ʾ�Ѿ���
	spi_sr_rxempty 			= bit(0),	// SPSR(0)		���Ĵ����ձ�־, 1��ʾ��

	/* �ⲿ�Ĵ���
	 */
	spi_er_icnt_mask 		= 0xC0,		// SPER(7:6)	�ڴ�������ٸ��ֽں��ͳ��ж������ź�
	spi_er_icnt_shift 		= 6,
	spi_er_mode 			= bit(2),	// SPER(2)		spi�ӿ�ģʽ����, =0: �����뷢��ʱ��ͬʱ; =1: �����뷢��ʱ����������.
	spi_er_spre_mask 		= 0x03,		// SPER(1:0)	��sprһ���趨��Ƶ�ı���
	spi_er_spre_shift 		= 0,

	/* SPI Flash�������ƼĴ���
	 */
	spi_param_clk_div_mask	= 0xF0,		// SFC_PARAM(7:4)	ʱ�ӷ�Ƶ��ѡ��, ��Ƶϵ����spre,spr�����ͬ.
	spi_param_clk_div_shift = 4,
	spi_param_dual_io		= bit(3),	// SFC_PARAM(3)		ʹ��˫I/Oģʽ�����ȼ����ڿ��ٶ�ģʽ
	spi_param_fast_read		= bit(2),	// SFC_PARAM(2)		ʹ�ÿ��ٶ�ģʽ
	spi_param_burst_en		= bit(1),	// SFC_PARAM(1)		spi flash ֧��������ַ��ģʽ
	spi_param_memory_en		= bit(0),	// SFC_PARAM(0)		spi flash ��ʹ��, ��Чʱcsn[0]�����������.

	/* SPI Ƭѡ���ƼĴ���
	 */
	spi_chip_sel_3			= bit(7),	// csn�������ֵ
	spi_chip_sel_2			= bit(6),
	spi_chip_sel_1			= bit(5),
	spi_chip_sel_0			= bit(4),
	spi_chip_sel_3_en		= bit(3),	// Ϊ1ʱ��Ӧλ��cs����7:4λ����
	spi_chip_sel_2_en		= bit(2),
	spi_chip_sel_1_en		= bit(1),
	spi_chip_sel_0_en		= bit(0),

	/* SPI Flashʱ����ƼĴ���
	 */
	spi_timing_tcsh_mask	= 0x03,		// SPI Flash ��Ƭѡ�ź������Чʱ�䣬�Է�Ƶ��ʱ������T����
	spi_timing_tcsh_shift	= 0,
	spi_timing_tcsh_1		= 0x00,
	spi_timing_tcsh_2		= 0x01,
	spi_timing_tcsh_4		= 0x02,
	spi_timing_tcsh_8		= 0x03,
} SPI_controller_t;

/*
 * ��Ƶϵ������ֵ
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

