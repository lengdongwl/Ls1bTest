/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * LS1x_nand_hw.h
 *
 *  Created on: 2013-11-21
 *      Author: Bian
 */

#ifndef _LS1x_NAND_HW_H_
#define _LS1x_NAND_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef bit
#define bit(x)	(1<<x)
#endif

/*
 * NAND Registers
 */
typedef struct LS1x_NAND_regs
{
	volatile unsigned int cmd;				// 0xBFE78000: NAND_CMD, ����Ĵ���
	volatile unsigned int addr_l;			// 0xBFE78004: ADDR_C, ҳ��ƫ�Ƶ�ַ�Ĵ���
	volatile unsigned int addr_h;			// 0xBFE78008: ADDR_R, ҳ��ַ�Ĵ���
	volatile unsigned int timing;			// 0xBFE7800C: NAND_TIMING, ʱ��Ĵ���
	volatile unsigned int id_l;				// 0xBFE78010: ID_L, ID�Ĵ���
	volatile unsigned int id_h;				// 0xBFE78014: STATUS & ID_H, ID��״̬�Ĵ���
	volatile unsigned int param;			// 0xBFE78018: NAND_PARAMETER, �������üĴ���
	volatile unsigned int opnum;			// 0xBFE7801C: NAND_OP_NUM, ���������Ĵ��� - NAND��д����Byte��; ����Ϊ����
	volatile unsigned int cs_rdy_map;		// 0xBFE78020: CS_RDY_MAP, ӳ��Ĵ���
	volatile unsigned int rsv[7];
	volatile unsigned int dma_addr;			// 0xBFE78040: DMA_ADDRESS, DMA��д���ݼĴ���
} LS1x_NAND_regs_t;

/*
 * NAND cmd Register
 */
typedef enum
{
#ifdef LS1C
	nand_cmd_dma_req		= bit(31),		// R/- ��ECCģʽ��NAND����DMA����
	nand_cmd_ecc_dma_req	= bit(30),		// R/- ECCģʽ��NAND����DMA����
#endif

	nand_cmd_cs_mask		= (0x0F<<20),	// R/- �ⲿNANDоƬƬѡ���, ��λ�ֱ��ӦƬ���ĸ�Ƭѡ, XXX =0: ��ʾѡ��
	nand_cmd_cs_shift		= 20,
    nand_cmd_cs_0           = (1<<20),
    nand_cmd_cs_1           = (1<<21),
    nand_cmd_cs_2           = (1<<22),
    nand_cmd_cs_3           = (1<<23),

	nand_cmd_rdy_mask		= (0x0F<<16),	// R/- �ⲿNANDоƬRDY���, ��Ӧ��ϵ��NAND_CEһ��, =1: ��ʾ׼����
	nand_cmd_rdy_shift		= 16,
    nand_cmd_rdy_0          = (1<<16),
    nand_cmd_rdy_1          = (1<<17),
    nand_cmd_rdy_2          = (1<<18),
    nand_cmd_rdy_3          = (1<<19),
#ifdef LS1C
	nand_cmd_wait_rd_done	= bit(14), 		// R/W =1: ��ʾ�ȴ�ECC�����(����ECC��)
	nand_cmd_int_en			= bit(13),		// R/W NAND�ж�ʹ���ź�, =1: ��ʾʹ���ж�
	nand_cmd_ecc_wr			= bit(12),		// R/W =1: ��ʾд����ʱ��ECC���ܿ���

	nand_cmd_ecc_rd			= bit(11),		// R/W =1: ��ʾ������ʱ��ECC���ܿ���
#endif
	nand_cmd_done			= bit(10),		// R/W =1: ��ʾ�������, ��Ҫ�������
	nand_cmd_spare			= bit(9),		// R/W =1: ��ʾ����������NAND��SPARE��
	nand_cmd_main			= bit(8),		// R/W =1: ��ʾ����������NAND��MAIN��

	nand_cmd_rd_sr			= bit(7),		// R/W =1: ��ʾ��NAND��״̬����
	nand_cmd_reset			= bit(6),		// R/W =1: ��ʾNand��λ����
	nand_cmd_rd_id			= bit(5),		// R/W =1: ��ʾ��ID����
	nand_cmd_erase_blocks	= bit(4),		// R/W =1: ��������, �������Ŀ��nand_op_num����

	nand_cmd_erase_1		= bit(3),		// R/W =1: ��ʾ��������
	nand_cmd_write			= bit(2),		// R/W =1: ��ʾд����
	nand_cmd_read			= bit(1),		// R/W =1: ��ʾ������
	nand_cmd_start			= bit(0),		// R/W =1: ��ʾ������Ч, ������ɺ�Ӳ���Զ�����
} LS1x_NAND_cmd_t;

/*
 * NAND addr_c Register
 */
typedef enum
{
	/* TODO
	 * 13:0 Nand_Col R/W ����д������������ʼ��ַҳ�ڵ�ַ(�������ֶ���, Ϊ4�ı���), ��ҳ��С��Ӧ��ϵ����:
	 * 512Bytes: ֻ��Ҫ���[8:0]
	 *       2K: ��Ҫ���[11:0], [11]��ʾspare��, [10:0]��ʾҳ��ƫ�Ƶ�ַ
	 *       4K: ��Ҫ���[12:0], [12]��ʾspare��, [11:0]��ʾҳ��ƫ�Ƶ�ַ
	 *       8K: ��Ҫ���[13:0], [13]��ʾspare��, [12:0]��ʾҳ��ƫ�Ƶ�ַ
	 */
	nand_A11_mask		= bit(11),			/* ==1 & nand_cmd_spare==1:  only op spare
											 * ==0 & nand_cmd_spare==0:  only op main
											 * ==X & nand_cmd_spare==X:  op main & spare
											 */
} LS1x_NAND_addrc_t;

/*
 * NAND addr_r Register
 */
typedef enum
{
	/* TODO
	 * 24:0 Nand_Row R/W ����д������������ʼ��ַҳ��ַ, ��ַ�������:
	 * { Ƭѡ, ҳ�� }
	 * ����Ƭѡ�̶�Ϊ2λ, ҳ������ʵ�ʵĵ�Ƭ��������ȷ��, ��1Mҳ��Ϊ[19:0], [21:20]����ѡ��4Ƭ�еĵڼ�Ƭ
	 */
	nand_addr_h_mask	= 0xFF,
	nand_addr_h_shift	= 0,
} LS1x_NAND_addrh_t;

/*
 * NAND timing Register
 */
typedef enum
{
	nand_timing_hold_cycle_mask  = 0xFF00,	// NAND������Ч��ȴ���������, ȱʡ4
	nand_timing_hold_cycle_shift = 8,
	nand_timing_wait_cycle_mask  = 0xFF,	// NANDһ�ζ�д������ʱ��������, ȱʡ18
	nand_timing_wait_cycle_shift = 0,
} LS1x_NAND_timing_t;

/*
 * NAND id_h Register, XXX ��LS1B��ͬ?
 */
typedef enum
{
#if defined(LS1B)
	nand_status_mask	= 0xFF00,			// bit[15:8], NAND�豸��ǰ�Ķ�д���״̬
	nand_status_shift	= 8,
	nand_id_h_mask		= 0xFF,			    // bit[7:0], ID��8λ
	nand_id_h_shift		= 0,
#elif defined(LS1C)
	nand_status_mask	= 0xFF0000,			// bit[23:16], NAND�豸��ǰ�Ķ�д���״̬
	nand_status_shift	= 16,
	nand_id_h_mask		= 0xFFFF,			// bit[15:0], ID��16λ
	nand_id_h_shift		= 0,
#endif
} LS1x_NAND_idh_t;

/*
 * NAND param Register
 *
 * PMON �����������Ϊ: 0x0800 5 000: 2K+5byte+1GB
 */
typedef enum
{
#ifdef LS1C
	/* TODO �������ԭ��������, =main+spare-Colomn. Ӧ�ý�����1Cʹ����! 2018.7.22
	 */
	nand_param_opscope_mask	 = (0x3FFF<<16),	/* bit[29:16] R/W ÿ���ܲ����ķ�Χ, ��������:
												   1. ����main��, ����Ϊһҳ��main����С
												   2. ����spare��, ����Ϊһҳ��spare����С
												   3. ����main��spare��, ����Ϊһҳ��main������spare����С */
	nand_param_opscope_shift = 16,

	nand_param_id_mask		 = (0x07<<12),		/* bit[14:12] R/W ID�ŵ��ֽ��� */
	nand_param_id_shift		 = 12,
#endif

	nand_param_size_mask	 = (0x0F<<8),		// bit[11:8] �ⲿ����������С
	nand_param_size_shift	 = 8,
	nand_size_1Gb			 = (0x00<<8),		// 0: 1Gb(2Kҳ)
	nand_size_2Gb			 = (0x01<<8),		// 1: 2Gb(2Kҳ)
	nand_size_4Gb			 = (0x02<<8),		// 2: 4Gb(2Kҳ)
	nand_size_8Gb			 = (0x03<<8),		// 3: 8Gb(2Kҳ)
#if 0
	nand_size_16Gb			 = (0x04<<8),		// 4: 16Gb(4Kҳ)
	nand_size_32Gb			 = (0x05<<8),		// 5: 32Gb(8Kҳ)
	nand_size_64Gb			 = (0x06<<8),		// 6: 64Gb(8Kҳ)
	nand_size_128Gb			 = (0x07<<8),		// 7: 128Gb(8Kҳ)
	nand_size_64Mb			 = (0x09<<8),		// 9: 64Mb(512Bҳ)
	nand_size_128Mb			 = (0x0A<<8),		// a:128Mb(512Bҳ)
	nand_size_256Mb			 = (0x0B<<8),		// b:256Mb(512Bҳ)
	nand_size_512Mb			 = (0x0C<<8),		// c:512Mb(512Bҳ)
	nand_size_1Gb		 	 = (0x0D<<8),		// d:1Gb(512Bҳ)
#endif
} LS1x_NAND_param_t;

/*
 * NAND cs_rdy_map Register
 */
typedef enum
{
	nand_rdy3_mask	= (0x0F<<28),
	nand_rdy3_shift = 28,
	nand_cs3_mask	= (0x0F<<24),
	nand_cs3_shift	= 24,

	nand_rdy2_mask  = (0x0F<<20),
	nand_rdy2_shift = 20,
	nand_cs2_mask	= (0x0F<<16),
	nand_cs2_shift	= 16,

	nand_rdy1_mask  = (0x0F<<12),
	nand_rdy1_shift = 12,
	nand_cs1_mask	= (0x0F<<8),
	nand_cs1_shift	= 8,

	nand_rdy_0		= bit(0),
	nand_rdy_1		= bit(1),
	nand_rdy_2		= bit(2),
	nand_rdy_3		= bit(3),

	nand_cs_0		= bit(0),
	nand_cs_1		= bit(1),
	nand_cs_2		= bit(2),
	nand_cs_3		= bit(3),
} LS1x_NAND_ready_t;

#ifdef __cplusplus
}
#endif


#endif /* LS1x_NAND_HW_H_ */

