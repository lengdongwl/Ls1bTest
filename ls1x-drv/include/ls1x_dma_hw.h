/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * LS1x_dma_hw.h
 *
 *  Created on: 2013-11-21
 *      Author: Bian
 */

#ifndef _LS1x_DMA_HW_H_
#define _LS1x_DMA_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef bit
#define bit(x)	   (1<<x)
#endif

#define LOCK_DMA
#define UNLOCK_DMA

//-----------------------------------------------------------------------------

/*
 * DMA direction
 */
#define DMA_READ	0x01					/* DMA 读操作 */
#define DMA_WRITE	0x02					/* DMA 写操作 */

/*
 * DMA Control register Base Address
 */
#define LS1x_DMA_CTRL_ADDR		0xBFD01160
#define LS1x_DMA_CTRL			(*(volatile unsigned int*)(LS1x_DMA_CTRL_ADDR))

/*
 * DMA access address
 */
#define LS1x_DMA_ACCESS_ADDR	0xBFE78040	/* DMA读写NAND flash数据(ID/STATUS除外)时候的访问地址,
 	 	 	 	 	 	 	 	 	 	 	 * 读/写地址相同, 读写方向通过DMA配置实现
 	 	 	 	 	 	 	 	 	 	 	 */

#define LS1x_AC97_TX_DMA_ADDR	0xBFE72420	/* AC97 DMA 发送操作地址 */
#define LS1x_AC97_RX_DMA_ADDR	0xBFE74C4C	/* AC97 DMA 接收操作地址 */

/*
 * DMA Control(ORDER_ADDR_IN) Register
 */
typedef enum
{
	dma_desc_addr_mask	= 0xFFFFFFC0,	// 被选中DMA第一个描述符地址的高26位,低6位为0;
	dma_desc_addr_shift	= 6,			// 实际描述符地址?: 相当于26位的 Ask_addr左移6位.
										/* 第一个描述符: 必须 64 字节对齐 ? (1<<6==64) */

	dma_stop			= bit(4),		// 用户请求停止DMA操作; 完成当前数据读写操作后,停止操作.

	dma_start			= bit(3),		// 可以开始读描述符链的第一个DMA描述符;
										// 当第一个描述符相关的寄存器读回后,该位清零.
	ask_valid			= bit(2),		// 用户请求将当前DMA操作的相关信息写回到指定的内存地址;
										// 当用户写回DMA操作相关信息后,该位清零.
	dma_dev_num_mask	= 0x03,			// bit 1:0, 选择DMA操作的设备.
	dma_dev_nand		= 0x00,			// nand flash
	dma_dev_ac97_write	= 0x01,			// AC97 write device (codec)
	dma_dev_ac97_read	= 0x02,			// AC97 read device (codec)
} LS1x_dma_order_t;

/*
 * DMA Descriptor
 *
 * 第一个描述符: 地址为 ORDER_ADDR_IN.ask_addr << 6, 就是要求 64 字节对齐.
 *
 */
typedef struct LS1x_dma_desc
{
	unsigned int next_desc;				// 0x00	下一个描述符地址寄存器 - 要求 16 字节对齐
	unsigned int mem_addr;				// 0x04	DMA操作的内存地址
	unsigned int dev_addr;				// 0x08	设备地址寄存器
	unsigned int length;				// 0x0C	传输数据长度寄存器 - 单位是 4 字节
	unsigned int step_length;			// 0x10	数据传输间隔长度寄存器
										//      说明: 间隔长度说明两块被搬运内存数据块之间的长度,
										//      前一个step的结束地址与后一个step的开始地址之间的间隔.
	unsigned int step_times;			// 0x14	数据传输循环次数寄存器
										//      说明: 循环次数说明在一次DMA操作中需要搬运的块的数目;
										//      如果只想搬运一个连续的数据块,循环次数寄存器的值可以赋值为1.
	unsigned int command;				// 0x18	控制寄存器
	unsigned int _blank;                // 0x1C this is a gap of 64 bytes aligned
} LS1x_dma_desc_t;

/*
 * DMA Descriptor's ORDERADDR
 */
typedef enum
{
	next_desc_addr_mask  	= 0xFFFFFFFE,	// bit 31:1, 存储器内部下一个描述符地址寄存器 - 要求 16 字节对齐
	next_desc_addr_shift 	= 1,
	next_desc_enable		= bit(0),		// 下一个描述符是否有效, 1: 有效.
} LS1x_dma_desc2_t;

/*
 * DMA Descriptor's DADDR
 */
typedef enum
{
	desc_daddr_ac97_wren		 = bit(31),		// AC97写使能，“1”表示是写操作
	desc_daddr_ac97_stereo		 = bit(30),		// =0: mono; =1: 2 stereo
	desc_daddr_ac97_wrmode_mask	 = (0x03<<28),	// AC97写模式, =0: 1byte, =1: 2byte, =2: 4byte
	desc_daddr_ac97_wrmode_shift = 28,
	desc_daddr_ac97_wrmode_1	 = (0x00<<28),
	desc_daddr_ac97_wrmode_2	 = (0x01<<28),
	desc_daddr_ac97_wrmode_4 	 = (0x02<<28),

	desc_daddr_dev_mask		     = 0x0FFFFFFF,	// DMA操作的APB设备地址

} LS1x_dma_daddr_t;

/*
 * DMA Descriptor's CMD
 */
typedef enum
{
	/* bit defination
	 */
	desc_cmd_addr_gen_mask	    = (0x03<<13),		// 源/目的地址生成方式
	desc_cmd_addr_gen_shift	    = 13,
	desc_cmd_r_w			    = bit(12),			// DMA操作类型, =1: 为读ddr2写设备; =0为读设备写ddr2.

	/* write status
	 */
	desc_cmd_wr_status	   	    = (0x0F<<8),		// DMA写数据状态
	desc_cmd_wr_shift           = 8,
	
	desc_cmd_wr_idle			= (0x00<<8),		// 写状态正处于空闲状态
	desc_cmd_wr_ddr_wait		= (0x01<<8),		// DMA判断需要执行读设备写内存操作,并发起写内存请求,
													// 但是内存还没准备好响应请求,因此DMA一直在等待内存的响应
	desc_cmd_wr_ddr			    = (0x02<<8),		// 内存接收了DMA写请求,但是还没有执行完写操作
	desc_cmd_wr_ddr_end		    = (0x03<<8),		// 内存接收了DMA写请求,并完成写操作;此时DMA处于写内存操作完成状态
	desc_cmd_wr_dma_wait		= (0x04<<8),		// DMA发出将DMA状态寄存器写回内存的请求,等待内存接收请求
	desc_cmd_wr_dma			    = (0x05<<8),		// 内存接收写DMA状态请求,但是操作还未完成
	desc_cmd_wr_dma_end		    = (0x06<<8),		// 内存完成写DMA状态操作
	desc_cmd_wr_step_end		= (0x07<<8),		// DMA完成一次length长度的操作(也就是说完成一个step)

	/* read status
	 */
	desc_cmd_rd_status	   	    = (0x0F<<4),		// DMA读数据状态
	desc_cmd_rd_shift           = 4,

	desc_cmd_rd_idle			 = (0x00<<4),		// 读状态正处于空闲状态
	desc_cmd_rd_ready			 = (0x01<<4),		// 接收到开始DMA操作的start信号后,进入准备好状态,开始读描述符
	desc_cmd_rd_get_order		 = (0x02<<4),		// 向内存发出读描述符请求,等待内存应答
	desc_cmd_rd_order		     = (0x03<<4),		// 内存接收读描述符请求,正在执行读操作
	desc_cmd_rd_finish_order_end = (0x04<<4),		// 内存读完DMA描述符
	desc_cmd_rd_ddr_wait		 = (0x05<<4),		// DMA向内存发出读数据请求,等待内存应答
	desc_cmd_rd_ddr			     = (0x06<<4),		// 内存接收DMA读数据请求,正在执行读数据操作
	desc_cmd_rd_ddr_end		     = (0x07<<4),		// 内存完成DMA的一次读数据请求
	desc_cmd_rd_dev			     = (0x08<<4),		// DMA进入读设备状态
	desc_cmd_rd_dev_end		     = (0x09<<4),		// 设备返回读数据,结束此次读设备请求
	desc_cmd_rd_step_end		 = (0x0A<<4),		// 结束一次step操作, step times减1
	
	desc_cmd_all_trans_over	    = bit(3),			// DMA执行完被配置的所有描述符操作
	desc_cmd_single_trans_over  = bit(2),			// DMA执行完一次描述符操作
	desc_cmd_int			    = bit(1),			// DMA中断信号
	desc_cmd_int_enable         = bit(0),			// DMA中断是否被允许
} LS1x_dma_cmd_t;

#ifdef __cplusplus
}
#endif

#endif /* LS1x_DMA_HW_H_ */

