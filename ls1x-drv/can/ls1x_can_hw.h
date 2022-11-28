/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_can_hw.h
 *
 *  Created on: 2013-10-23
 *      Author: Bian
 */

#ifndef _LS1x_CAN_HW_H_
#define _LS1x_CAN_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef bit
#define bit(x)   (1<<x)
#endif

/*
 * CAN Registers.
 */
typedef struct LS1x_CAN_regs
{
	volatile unsigned char ctrl;							/* 0x00 控制寄存器 */
	volatile unsigned char cmd;								/* 0x01 命令寄存器 */
	volatile unsigned char status;							/* 0x02 状态寄存器 */
	volatile unsigned char intflags;						/* 0x03 中断寄存器 */

	union
	{
		struct		/* 标准模式 */
		{
			volatile unsigned char accode;					/* 0x04 验收代码寄存器 */
			volatile unsigned char acmask;					/* 0x05 验收屏蔽寄存器 */
		} std;

		struct		/* 扩展模式 */
		{
			volatile unsigned char inten;					/* 0x04 中断使能寄存器 */
			volatile unsigned char res5;					/* 0x05 */
		} ext;

	} reg;

	volatile unsigned char btr0;							/* 0x06 */
	volatile unsigned char btr1;							/* 0x07 */
	volatile unsigned char res8;							/* 0x08 */
	volatile unsigned char res9;							/* 0x09 */

	union
	{
		struct		/* 标准模式 */
		{
			struct
			{
				volatile unsigned char id[2];				/* 0x0A~0x0B:	tx ID(10-3); ID(2-0),RTR(4),DLC(3:0) */
				volatile unsigned char data[8];				/* 0x0C~0x13	tx data(1~8) */
			} tx;

			struct
			{
				volatile unsigned char id[2];				/* 0x14~0x15:	rx ID(10-3); ID(2-0),RTR(4),DLC(3:0) */
				volatile unsigned char data[8];				/* 0x16~0x1D	rx data(1~8) */
			} rx;
		} std;

		struct		/* 扩展模式 */
		{
			volatile unsigned char resA;					/* 0x0A */
			volatile unsigned char arblost;					/* 0x0B 仲裁丢失捕捉寄存器 */
			volatile unsigned char errcode;					/* 0x0C 错误代码捕捉寄存器 */
			volatile unsigned char emlimit;					/* 0x0D 错误警报限制寄存器 */
			volatile unsigned char rxerrcnt;				/* 0x0E RX错误计数寄存器 */
			volatile unsigned char txerrcnt;				/* 0x0F TX错误计数寄存器 */

			union
			{
				struct			/* 工作模式下接收区 */
				{
					volatile unsigned char frameinfo;		/* 0x10 RX帧信息 */
					union
					{
						struct	/* 扩展帧 */
						{
							volatile unsigned char id[4];	/* 0x11~0x14, RX识别码 1-4 */
							volatile unsigned char data[8];	/* 0x15~0x1C, RX数据 1-8 */
						} ext;

						struct	/* 标准帧 */
						{
							volatile unsigned char id[2];	/* 0x11~0x12, RX识别码 1-2 */
							volatile unsigned char data[8];	/* 0x13~0x1A, RX数据 1-8 */
							volatile unsigned char res1B;	/* 0x1B */
							volatile unsigned char res1C;	/* 0x1C */
						} std;
					};
				} rx;

				struct			/* 工作模式下发送区 */
				{
					volatile unsigned char frameinfo;		/* 0x10 TX帧信息 */
					union
					{
						struct	/* 扩展帧 */
						{
							volatile unsigned char id[4];	/* 0x11~0x14, TX识别码 1-4 */
							volatile unsigned char data[8];	/* 0x15~0x1C, TX数据 1-8 */
						} ext;

						struct	/* 标准帧 */
						{
							volatile unsigned char id[2];	/* 0x11~0x12, TX识别码 1-2 */
							volatile unsigned char data[8];	/* 0x13~0x1A, TX数据 1-8 */
							volatile unsigned char res1B;	/* 0x1B */
							volatile unsigned char res1C;	/* 0x1C */
						} std;
					};
				} tx;

				struct			/* 复位模式下 */
				{
					volatile unsigned char accode[4];		/* 0x10~0x13  验收代码1-4 */
					volatile unsigned char acmask[4];		/* 0x14~0x17  验收屏蔽1-4 */
				} accept;
			} msg;

			volatile unsigned char rxframecnt;				/* 0x1D RX信息计数寄存器 */

		} ext;

	} msg;

} LS1x_CAN_regs_t;

/*
 * CAN Control Register
 */
typedef enum
{
	can_ctrl_overflowie_std	= bit(4),				/* 溢出中断使能 */
	can_ctrl_errorie_std	= bit(3),				/* 错误中断使能 */
	can_ctrl_txie_std		= bit(2),				/* 发送中断使能 */
	can_ctrl_rxie_std		= bit(1),				/* 接收中断使能 */

	can_ctrl_sleep_ext		= bit(4),				/* 睡眠模式 */
	can_ctrl_afilter_ext	= bit(3),				/* 滤波器模式.	=1: 单; =0: 双. */
	can_ctrl_standwork_ext	= bit(2),				/* 正常工作模式.	=1: 正常. */
	can_ctrl_listenonly_ext	= bit(1),				/* 只听模式.		=1: 只听. */

	can_ctrl_reset 			= bit(0),				/* 复位请求 */

	can_std_interrupts = can_ctrl_overflowie_std |
						 can_ctrl_errorie_std |
						 can_ctrl_txie_std |
						 can_ctrl_rxie_std,
} LS1x_CAN_ctrl_t;

/*
 * CAN Command Register.
 */
typedef enum
{
	can_cmd_standard			= 0,
	can_cmd_extended			= bit(7),			/* 扩展模式 -> CAN 2.0 A/B, 0=standard */
	can_cmd_sleep_std			= bit(4),			/* Standard: 睡眠 */
	can_cmd_selfrxrequest_ext	= bit(4),			/* 自接收请求, =1: 当前信息可被同时发送和接收 */
	can_cmd_cleardataoverflow	= bit(3),			/* 清除数据溢出 */
	can_cmd_releaserxbuf		= bit(2),			/* 释放接收缓冲器 */
	can_cmd_txabort				= bit(1),			/* 中止发送, =1: 当前, 如果不是正在处理, 等待中的发送请求被取消 */
	can_cmd_txrequest 			= bit(0),			/* 发送请求 */
} LS1x_CAN_cmd_t;

/*
 * CAN Status Register
 */
typedef enum
{
	can_status_bus				= bit(7),			/* 总线状态. =1: 总线关闭; =0: 总线开启. */
	can_status_error			= bit(6),			/* 出错状态. 	=1: 至少出现一个错误计数器满或超过CPU报警限制. */
	can_status_tx				= bit(5),			/* 发送状态. 	=1: 发送; =0: 空闲. */
	can_status_rx				= bit(4),			/* 接收状态. 	=1: 接收; =0: 空闲. */
	can_status_txcomplete		= bit(3),			/* 发送完毕状态. =1: 完毕; =0: 未完毕. */
	can_status_txbuf			= bit(2),			/* 发送缓存器状态. =1: 释放; =0: 锁定. */
	can_status_dataoverflow		= bit(1),			/* 数据溢出状态. =1: 溢出, 信息丢失, 因为RXFIFO中没有足够的空间来存储它. */
	can_status_rxbuf 			= bit(0),			/* 接收缓存器状态. =1: 满, RXFIFO中有可用信息. */
} LS1x_CAN_status_t;

/*
 * CAN Interrupt Register
 */
typedef enum
{
	can_intflags_buserror_ext		= bit(7),		/* 总线错误中断 */
	can_intflags_arbitratelost_ext	= bit(6),		/* 仲裁丢失中断 */
	can_intflags_errorpassive_ext	= bit(5),		/* 错误消极中断 */
	can_intflags_wakeup				= bit(4),		/* 唤醒中断 */
	can_intflags_dataoverflow		= bit(3),		/* 数据溢出中断 */
	can_intflags_error				= bit(2),		/* 错误中断 */
	can_intflags_tx					= bit(1),		/* 发送中断 */
	can_intflags_rx 				= bit(0),		/* 接收中断 */
} LS1x_CAN_iflags_t;

/*
 * CAN Interrupt Enable Register
 */
typedef enum
{
	can_inten_buserror_ext		= bit(7),			/* 总线错误中断 */
	can_inten_arbitratelost_ext	= bit(6),			/* 仲裁丢失中断 */
	can_inten_errorpassive_ext	= bit(5),			/* 错误消极中断 */
	can_inten_wakeup_ext		= bit(4),			/* 唤醒中断 */
	can_inten_dataoverflow_ext	= bit(3),			/* 数据溢出中断 */
	can_inten_error_ext			= bit(2),			/* 错误中断 */
	can_inten_tx_ext			= bit(1),			/* 发送中断 */
	can_inten_rx_ext 			= bit(0),			/* 接收中断 */
	can_inten_all_ext			= 0xFF,				/* Enable All */
} LS1x_CAN_ien_t;

/*
 * CAN Bus Timer Register 0
 */
typedef enum
{
	can_btr0_sjw_mask		= 0xC0,					/* bit(7:6), 同步跳转宽度 */
	can_btr0_sjw_shift		= 6,
	can_btr0_brp_mask		= 0x3F,					/* bit(5:0), 波特率分频系数 */
	can_btr0_brp_shift		= 0,
} LS1x_CAN_btr_brp_t;

/*
 * CAN Bus Timer Register 1
 */
typedef enum
{
	can_btr1_sample_mask	= 0x80,					/* bit(7), =1 时三次采样, 否则是一次采样 */
	can_btr1_sample_shift	= 7,
	can_btr1_tseg2_mask		= 0x70,					/* bit(6:4), 一个bit中的时间段 2 的计数值 */
	can_btr1_tseg2_shift	= 4,
	can_btr1_tseg1_mask		= 0x0F,					/* bit(3:0), 一个bit中的时间段 1 的计数值 */
	can_btr1_tseg1_shift	= 0,
} LS1x_CAN_btr_tseg_t;

/*
 * CAN Arbitrate Lost Register
 */
typedef enum
{
	can_arblost_mask_ext 	= 0x1F,
	can_arblost_shift_ext 	= 0,
} LS1x_CAN_arblost_t;

/*
 * CAN Error Code Register
 */
typedef enum
{
	can_errcode_mask_ext	 	= 0xC0,				/* bit(7:6), */
	can_errcode_bit_ext			= 0x00,				/* 00: 位错 */
	can_errcode_form_ext		= 0x40,				/* 01: 格式错 */
	can_errcode_stuff_ext	 	= 0x80,				/* 10: 填充错 */
	can_errcode_other_ext	 	= 0xC0,				/* 11: 其它错误 */
	can_errcode_dir_ext		 	= 0x20,				/* bit(5), =1: RX Error; =0: TX Error */
	can_errcode_seg_mask_ext 	= 0x1F,
} LS1x_CAN_errcode_t;


#ifdef __cplusplus
}
#endif

#endif /* _LS1x_CAN_HW_H_ */
