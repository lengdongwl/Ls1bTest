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
	volatile unsigned int cmd;				// 0xBFE78000: NAND_CMD, 命令寄存器
	volatile unsigned int addr_l;			// 0xBFE78004: ADDR_C, 页内偏移地址寄存器
	volatile unsigned int addr_h;			// 0xBFE78008: ADDR_R, 页地址寄存器
	volatile unsigned int timing;			// 0xBFE7800C: NAND_TIMING, 时序寄存器
	volatile unsigned int id_l;				// 0xBFE78010: ID_L, ID寄存器
	volatile unsigned int id_h;				// 0xBFE78014: STATUS & ID_H, ID和状态寄存器
	volatile unsigned int param;			// 0xBFE78018: NAND_PARAMETER, 参数配置寄存器
	volatile unsigned int opnum;			// 0xBFE7801C: NAND_OP_NUM, 操作数量寄存器 - NAND读写操作Byte数; 擦除为块数
	volatile unsigned int cs_rdy_map;		// 0xBFE78020: CS_RDY_MAP, 映射寄存器
	volatile unsigned int rsv[7];
	volatile unsigned int dma_addr;			// 0xBFE78040: DMA_ADDRESS, DMA读写数据寄存器
} LS1x_NAND_regs_t;

/*
 * NAND cmd Register
 */
typedef enum
{
#ifdef LS1C
	nand_cmd_dma_req		= bit(31),		// R/- 非ECC模式下NAND发出DMA请求
	nand_cmd_ecc_dma_req	= bit(30),		// R/- ECC模式下NAND发出DMA请求
#endif

	nand_cmd_cs_mask		= (0x0F<<20),	// R/- 外部NAND芯片片选情况, 四位分别对应片外四个片选, XXX =0: 表示选中
	nand_cmd_cs_shift		= 20,
    nand_cmd_cs_0           = (1<<20),
    nand_cmd_cs_1           = (1<<21),
    nand_cmd_cs_2           = (1<<22),
    nand_cmd_cs_3           = (1<<23),

	nand_cmd_rdy_mask		= (0x0F<<16),	// R/- 外部NAND芯片RDY情况, 对应关系和NAND_CE一致, =1: 表示准备好
	nand_cmd_rdy_shift		= 16,
    nand_cmd_rdy_0          = (1<<16),
    nand_cmd_rdy_1          = (1<<17),
    nand_cmd_rdy_2          = (1<<18),
    nand_cmd_rdy_3          = (1<<19),
#ifdef LS1C
	nand_cmd_wait_rd_done	= bit(14), 		// R/W =1: 表示等待ECC读完成(用于ECC读)
	nand_cmd_int_en			= bit(13),		// R/W NAND中断使能信号, =1: 表示使能中断
	nand_cmd_ecc_wr			= bit(12),		// R/W =1: 表示写操作时候ECC功能开启

	nand_cmd_ecc_rd			= bit(11),		// R/W =1: 表示读操作时候ECC功能开启
#endif
	nand_cmd_done			= bit(10),		// R/W =1: 表示操作完成, 需要软件清零
	nand_cmd_spare			= bit(9),		// R/W =1: 表示操作发生在NAND的SPARE区
	nand_cmd_main			= bit(8),		// R/W =1: 表示操作发生在NAND的MAIN区

	nand_cmd_rd_sr			= bit(7),		// R/W =1: 表示读NAND的状态操作
	nand_cmd_reset			= bit(6),		// R/W =1: 表示Nand复位操作
	nand_cmd_rd_id			= bit(5),		// R/W =1: 表示读ID操作
	nand_cmd_erase_blocks	= bit(4),		// R/W =1: 连续擦除, 擦块的数目由nand_op_num决定

	nand_cmd_erase_1		= bit(3),		// R/W =1: 表示擦除操作
	nand_cmd_write			= bit(2),		// R/W =1: 表示写操作
	nand_cmd_read			= bit(1),		// R/W =1: 表示读操作
	nand_cmd_start			= bit(0),		// R/W =1: 表示命令有效, 操作完成后硬件自动清零
} LS1x_NAND_cmd_t;

/*
 * NAND addr_c Register
 */
typedef enum
{
	/* TODO
	 * 13:0 Nand_Col R/W 读、写、擦除操作起始地址页内地址(必须以字对齐, 为4的倍数), 和页大小对应关系如下:
	 * 512Bytes: 只需要填充[8:0]
	 *       2K: 需要填充[11:0], [11]表示spare区, [10:0]表示页内偏移地址
	 *       4K: 需要填充[12:0], [12]表示spare区, [11:0]表示页内偏移地址
	 *       8K: 需要填充[13:0], [13]表示spare区, [12:0]表示页内偏移地址
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
	 * 24:0 Nand_Row R/W 读、写、擦除操作起始地址页地址, 地址组成如下:
	 * { 片选, 页数 }
	 * 其中片选固定为2位, 页数根据实际的单片颗粒容量确定, 如1M页则为[19:0], [21:20]用于选择4片中的第几片
	 */
	nand_addr_h_mask	= 0xFF,
	nand_addr_h_shift	= 0,
} LS1x_NAND_addrh_t;

/*
 * NAND timing Register
 */
typedef enum
{
	nand_timing_hold_cycle_mask  = 0xFF00,	// NAND命令有效需等待的周期数, 缺省4
	nand_timing_hold_cycle_shift = 8,
	nand_timing_wait_cycle_mask  = 0xFF,	// NAND一次读写所需总时钟周期数, 缺省18
	nand_timing_wait_cycle_shift = 0,
} LS1x_NAND_timing_t;

/*
 * NAND id_h Register, XXX 与LS1B不同?
 */
typedef enum
{
#if defined(LS1B)
	nand_status_mask	= 0xFF00,			// bit[15:8], NAND设备当前的读写完成状态
	nand_status_shift	= 8,
	nand_id_h_mask		= 0xFF,			    // bit[7:0], ID高8位
	nand_id_h_shift		= 0,
#elif defined(LS1C)
	nand_status_mask	= 0xFF0000,			// bit[23:16], NAND设备当前的读写完成状态
	nand_status_shift	= 16,
	nand_id_h_mask		= 0xFFFF,			// bit[15:0], ID高16位
	nand_id_h_shift		= 0,
#endif
} LS1x_NAND_idh_t;

/*
 * NAND param Register
 *
 * PMON 配置这个参数为: 0x0800 5 000: 2K+5byte+1GB
 */
typedef enum
{
#ifdef LS1C
	/* TODO 这个参数原来理解错误, =main+spare-Colomn. 应该仅仅是1C使用了! 2018.7.22
	 */
	nand_param_opscope_mask	 = (0x3FFF<<16),	/* bit[29:16] R/W 每次能操作的范围, 配置如下:
												   1. 操作main区, 配置为一页的main区大小
												   2. 操作spare区, 配置为一页的spare区大小
												   3. 操作main加spare区, 配置为一页的main区加上spare区大小 */
	nand_param_opscope_shift = 16,

	nand_param_id_mask		 = (0x07<<12),		/* bit[14:12] R/W ID号的字节数 */
	nand_param_id_shift		 = 12,
#endif

	nand_param_size_mask	 = (0x0F<<8),		// bit[11:8] 外部颗粒容量大小
	nand_param_size_shift	 = 8,
	nand_size_1Gb			 = (0x00<<8),		// 0: 1Gb(2K页)
	nand_size_2Gb			 = (0x01<<8),		// 1: 2Gb(2K页)
	nand_size_4Gb			 = (0x02<<8),		// 2: 4Gb(2K页)
	nand_size_8Gb			 = (0x03<<8),		// 3: 8Gb(2K页)
#if 0
	nand_size_16Gb			 = (0x04<<8),		// 4: 16Gb(4K页)
	nand_size_32Gb			 = (0x05<<8),		// 5: 32Gb(8K页)
	nand_size_64Gb			 = (0x06<<8),		// 6: 64Gb(8K页)
	nand_size_128Gb			 = (0x07<<8),		// 7: 128Gb(8K页)
	nand_size_64Mb			 = (0x09<<8),		// 9: 64Mb(512B页)
	nand_size_128Mb			 = (0x0A<<8),		// a:128Mb(512B页)
	nand_size_256Mb			 = (0x0B<<8),		// b:256Mb(512B页)
	nand_size_512Mb			 = (0x0C<<8),		// c:512Mb(512B页)
	nand_size_1Gb		 	 = (0x0D<<8),		// d:1Gb(512B页)
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

