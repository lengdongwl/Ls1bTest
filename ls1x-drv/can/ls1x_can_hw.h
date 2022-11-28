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
	volatile unsigned char ctrl;							/* 0x00 ���ƼĴ��� */
	volatile unsigned char cmd;								/* 0x01 ����Ĵ��� */
	volatile unsigned char status;							/* 0x02 ״̬�Ĵ��� */
	volatile unsigned char intflags;						/* 0x03 �жϼĴ��� */

	union
	{
		struct		/* ��׼ģʽ */
		{
			volatile unsigned char accode;					/* 0x04 ���մ���Ĵ��� */
			volatile unsigned char acmask;					/* 0x05 �������μĴ��� */
		} std;

		struct		/* ��չģʽ */
		{
			volatile unsigned char inten;					/* 0x04 �ж�ʹ�ܼĴ��� */
			volatile unsigned char res5;					/* 0x05 */
		} ext;

	} reg;

	volatile unsigned char btr0;							/* 0x06 */
	volatile unsigned char btr1;							/* 0x07 */
	volatile unsigned char res8;							/* 0x08 */
	volatile unsigned char res9;							/* 0x09 */

	union
	{
		struct		/* ��׼ģʽ */
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

		struct		/* ��չģʽ */
		{
			volatile unsigned char resA;					/* 0x0A */
			volatile unsigned char arblost;					/* 0x0B �ٲö�ʧ��׽�Ĵ��� */
			volatile unsigned char errcode;					/* 0x0C ������벶׽�Ĵ��� */
			volatile unsigned char emlimit;					/* 0x0D ���󾯱����ƼĴ��� */
			volatile unsigned char rxerrcnt;				/* 0x0E RX��������Ĵ��� */
			volatile unsigned char txerrcnt;				/* 0x0F TX��������Ĵ��� */

			union
			{
				struct			/* ����ģʽ�½����� */
				{
					volatile unsigned char frameinfo;		/* 0x10 RX֡��Ϣ */
					union
					{
						struct	/* ��չ֡ */
						{
							volatile unsigned char id[4];	/* 0x11~0x14, RXʶ���� 1-4 */
							volatile unsigned char data[8];	/* 0x15~0x1C, RX���� 1-8 */
						} ext;

						struct	/* ��׼֡ */
						{
							volatile unsigned char id[2];	/* 0x11~0x12, RXʶ���� 1-2 */
							volatile unsigned char data[8];	/* 0x13~0x1A, RX���� 1-8 */
							volatile unsigned char res1B;	/* 0x1B */
							volatile unsigned char res1C;	/* 0x1C */
						} std;
					};
				} rx;

				struct			/* ����ģʽ�·����� */
				{
					volatile unsigned char frameinfo;		/* 0x10 TX֡��Ϣ */
					union
					{
						struct	/* ��չ֡ */
						{
							volatile unsigned char id[4];	/* 0x11~0x14, TXʶ���� 1-4 */
							volatile unsigned char data[8];	/* 0x15~0x1C, TX���� 1-8 */
						} ext;

						struct	/* ��׼֡ */
						{
							volatile unsigned char id[2];	/* 0x11~0x12, TXʶ���� 1-2 */
							volatile unsigned char data[8];	/* 0x13~0x1A, TX���� 1-8 */
							volatile unsigned char res1B;	/* 0x1B */
							volatile unsigned char res1C;	/* 0x1C */
						} std;
					};
				} tx;

				struct			/* ��λģʽ�� */
				{
					volatile unsigned char accode[4];		/* 0x10~0x13  ���մ���1-4 */
					volatile unsigned char acmask[4];		/* 0x14~0x17  ��������1-4 */
				} accept;
			} msg;

			volatile unsigned char rxframecnt;				/* 0x1D RX��Ϣ�����Ĵ��� */

		} ext;

	} msg;

} LS1x_CAN_regs_t;

/*
 * CAN Control Register
 */
typedef enum
{
	can_ctrl_overflowie_std	= bit(4),				/* ����ж�ʹ�� */
	can_ctrl_errorie_std	= bit(3),				/* �����ж�ʹ�� */
	can_ctrl_txie_std		= bit(2),				/* �����ж�ʹ�� */
	can_ctrl_rxie_std		= bit(1),				/* �����ж�ʹ�� */

	can_ctrl_sleep_ext		= bit(4),				/* ˯��ģʽ */
	can_ctrl_afilter_ext	= bit(3),				/* �˲���ģʽ.	=1: ��; =0: ˫. */
	can_ctrl_standwork_ext	= bit(2),				/* ��������ģʽ.	=1: ����. */
	can_ctrl_listenonly_ext	= bit(1),				/* ֻ��ģʽ.		=1: ֻ��. */

	can_ctrl_reset 			= bit(0),				/* ��λ���� */

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
	can_cmd_extended			= bit(7),			/* ��չģʽ -> CAN 2.0 A/B, 0=standard */
	can_cmd_sleep_std			= bit(4),			/* Standard: ˯�� */
	can_cmd_selfrxrequest_ext	= bit(4),			/* �Խ�������, =1: ��ǰ��Ϣ�ɱ�ͬʱ���ͺͽ��� */
	can_cmd_cleardataoverflow	= bit(3),			/* ���������� */
	can_cmd_releaserxbuf		= bit(2),			/* �ͷŽ��ջ����� */
	can_cmd_txabort				= bit(1),			/* ��ֹ����, =1: ��ǰ, ����������ڴ���, �ȴ��еķ�������ȡ�� */
	can_cmd_txrequest 			= bit(0),			/* �������� */
} LS1x_CAN_cmd_t;

/*
 * CAN Status Register
 */
typedef enum
{
	can_status_bus				= bit(7),			/* ����״̬. =1: ���߹ر�; =0: ���߿���. */
	can_status_error			= bit(6),			/* ����״̬. 	=1: ���ٳ���һ��������������򳬹�CPU��������. */
	can_status_tx				= bit(5),			/* ����״̬. 	=1: ����; =0: ����. */
	can_status_rx				= bit(4),			/* ����״̬. 	=1: ����; =0: ����. */
	can_status_txcomplete		= bit(3),			/* �������״̬. =1: ���; =0: δ���. */
	can_status_txbuf			= bit(2),			/* ���ͻ�����״̬. =1: �ͷ�; =0: ����. */
	can_status_dataoverflow		= bit(1),			/* �������״̬. =1: ���, ��Ϣ��ʧ, ��ΪRXFIFO��û���㹻�Ŀռ����洢��. */
	can_status_rxbuf 			= bit(0),			/* ���ջ�����״̬. =1: ��, RXFIFO���п�����Ϣ. */
} LS1x_CAN_status_t;

/*
 * CAN Interrupt Register
 */
typedef enum
{
	can_intflags_buserror_ext		= bit(7),		/* ���ߴ����ж� */
	can_intflags_arbitratelost_ext	= bit(6),		/* �ٲö�ʧ�ж� */
	can_intflags_errorpassive_ext	= bit(5),		/* ���������ж� */
	can_intflags_wakeup				= bit(4),		/* �����ж� */
	can_intflags_dataoverflow		= bit(3),		/* ��������ж� */
	can_intflags_error				= bit(2),		/* �����ж� */
	can_intflags_tx					= bit(1),		/* �����ж� */
	can_intflags_rx 				= bit(0),		/* �����ж� */
} LS1x_CAN_iflags_t;

/*
 * CAN Interrupt Enable Register
 */
typedef enum
{
	can_inten_buserror_ext		= bit(7),			/* ���ߴ����ж� */
	can_inten_arbitratelost_ext	= bit(6),			/* �ٲö�ʧ�ж� */
	can_inten_errorpassive_ext	= bit(5),			/* ���������ж� */
	can_inten_wakeup_ext		= bit(4),			/* �����ж� */
	can_inten_dataoverflow_ext	= bit(3),			/* ��������ж� */
	can_inten_error_ext			= bit(2),			/* �����ж� */
	can_inten_tx_ext			= bit(1),			/* �����ж� */
	can_inten_rx_ext 			= bit(0),			/* �����ж� */
	can_inten_all_ext			= 0xFF,				/* Enable All */
} LS1x_CAN_ien_t;

/*
 * CAN Bus Timer Register 0
 */
typedef enum
{
	can_btr0_sjw_mask		= 0xC0,					/* bit(7:6), ͬ����ת��� */
	can_btr0_sjw_shift		= 6,
	can_btr0_brp_mask		= 0x3F,					/* bit(5:0), �����ʷ�Ƶϵ�� */
	can_btr0_brp_shift		= 0,
} LS1x_CAN_btr_brp_t;

/*
 * CAN Bus Timer Register 1
 */
typedef enum
{
	can_btr1_sample_mask	= 0x80,					/* bit(7), =1 ʱ���β���, ������һ�β��� */
	can_btr1_sample_shift	= 7,
	can_btr1_tseg2_mask		= 0x70,					/* bit(6:4), һ��bit�е�ʱ��� 2 �ļ���ֵ */
	can_btr1_tseg2_shift	= 4,
	can_btr1_tseg1_mask		= 0x0F,					/* bit(3:0), һ��bit�е�ʱ��� 1 �ļ���ֵ */
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
	can_errcode_bit_ext			= 0x00,				/* 00: λ�� */
	can_errcode_form_ext		= 0x40,				/* 01: ��ʽ�� */
	can_errcode_stuff_ext	 	= 0x80,				/* 10: ���� */
	can_errcode_other_ext	 	= 0xC0,				/* 11: �������� */
	can_errcode_dir_ext		 	= 0x20,				/* bit(5), =1: RX Error; =0: TX Error */
	can_errcode_seg_mask_ext 	= 0x1F,
} LS1x_CAN_errcode_t;


#ifdef __cplusplus
}
#endif

#endif /* _LS1x_CAN_HW_H_ */
