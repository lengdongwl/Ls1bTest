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
#define DMA_READ	0x01					/* DMA ������ */
#define DMA_WRITE	0x02					/* DMA д���� */

/*
 * DMA Control register Base Address
 */
#define LS1x_DMA_CTRL_ADDR		0xBFD01160
#define LS1x_DMA_CTRL			(*(volatile unsigned int*)(LS1x_DMA_CTRL_ADDR))

/*
 * DMA access address
 */
#define LS1x_DMA_ACCESS_ADDR	0xBFE78040	/* DMA��дNAND flash����(ID/STATUS����)ʱ��ķ��ʵ�ַ,
 	 	 	 	 	 	 	 	 	 	 	 * ��/д��ַ��ͬ, ��д����ͨ��DMA����ʵ��
 	 	 	 	 	 	 	 	 	 	 	 */

#define LS1x_AC97_TX_DMA_ADDR	0xBFE72420	/* AC97 DMA ���Ͳ�����ַ */
#define LS1x_AC97_RX_DMA_ADDR	0xBFE74C4C	/* AC97 DMA ���ղ�����ַ */

/*
 * DMA Control(ORDER_ADDR_IN) Register
 */
typedef enum
{
	dma_desc_addr_mask	= 0xFFFFFFC0,	// ��ѡ��DMA��һ����������ַ�ĸ�26λ,��6λΪ0;
	dma_desc_addr_shift	= 6,			// ʵ����������ַ?: �൱��26λ�� Ask_addr����6λ.
										/* ��һ��������: ���� 64 �ֽڶ��� ? (1<<6==64) */

	dma_stop			= bit(4),		// �û�����ֹͣDMA����; ��ɵ�ǰ���ݶ�д������,ֹͣ����.

	dma_start			= bit(3),		// ���Կ�ʼ�����������ĵ�һ��DMA������;
										// ����һ����������صļĴ������غ�,��λ����.
	ask_valid			= bit(2),		// �û����󽫵�ǰDMA�����������Ϣд�ص�ָ�����ڴ��ַ;
										// ���û�д��DMA���������Ϣ��,��λ����.
	dma_dev_num_mask	= 0x03,			// bit 1:0, ѡ��DMA�������豸.
	dma_dev_nand		= 0x00,			// nand flash
	dma_dev_ac97_write	= 0x01,			// AC97 write device (codec)
	dma_dev_ac97_read	= 0x02,			// AC97 read device (codec)
} LS1x_dma_order_t;

/*
 * DMA Descriptor
 *
 * ��һ��������: ��ַΪ ORDER_ADDR_IN.ask_addr << 6, ����Ҫ�� 64 �ֽڶ���.
 *
 */
typedef struct LS1x_dma_desc
{
	unsigned int next_desc;				// 0x00	��һ����������ַ�Ĵ��� - Ҫ�� 16 �ֽڶ���
	unsigned int mem_addr;				// 0x04	DMA�������ڴ��ַ
	unsigned int dev_addr;				// 0x08	�豸��ַ�Ĵ���
	unsigned int length;				// 0x0C	�������ݳ��ȼĴ��� - ��λ�� 4 �ֽ�
	unsigned int step_length;			// 0x10	���ݴ��������ȼĴ���
										//      ˵��: �������˵�����鱻�����ڴ����ݿ�֮��ĳ���,
										//      ǰһ��step�Ľ�����ַ���һ��step�Ŀ�ʼ��ַ֮��ļ��.
	unsigned int step_times;			// 0x14	���ݴ���ѭ�������Ĵ���
										//      ˵��: ѭ������˵����һ��DMA��������Ҫ���˵Ŀ����Ŀ;
										//      ���ֻ�����һ�����������ݿ�,ѭ�������Ĵ�����ֵ���Ը�ֵΪ1.
	unsigned int command;				// 0x18	���ƼĴ���
	unsigned int _blank;                // 0x1C this is a gap of 64 bytes aligned
} LS1x_dma_desc_t;

/*
 * DMA Descriptor's ORDERADDR
 */
typedef enum
{
	next_desc_addr_mask  	= 0xFFFFFFFE,	// bit 31:1, �洢���ڲ���һ����������ַ�Ĵ��� - Ҫ�� 16 �ֽڶ���
	next_desc_addr_shift 	= 1,
	next_desc_enable		= bit(0),		// ��һ���������Ƿ���Ч, 1: ��Ч.
} LS1x_dma_desc2_t;

/*
 * DMA Descriptor's DADDR
 */
typedef enum
{
	desc_daddr_ac97_wren		 = bit(31),		// AC97дʹ�ܣ���1����ʾ��д����
	desc_daddr_ac97_stereo		 = bit(30),		// =0: mono; =1: 2 stereo
	desc_daddr_ac97_wrmode_mask	 = (0x03<<28),	// AC97дģʽ, =0: 1byte, =1: 2byte, =2: 4byte
	desc_daddr_ac97_wrmode_shift = 28,
	desc_daddr_ac97_wrmode_1	 = (0x00<<28),
	desc_daddr_ac97_wrmode_2	 = (0x01<<28),
	desc_daddr_ac97_wrmode_4 	 = (0x02<<28),

	desc_daddr_dev_mask		     = 0x0FFFFFFF,	// DMA������APB�豸��ַ

} LS1x_dma_daddr_t;

/*
 * DMA Descriptor's CMD
 */
typedef enum
{
	/* bit defination
	 */
	desc_cmd_addr_gen_mask	    = (0x03<<13),		// Դ/Ŀ�ĵ�ַ���ɷ�ʽ
	desc_cmd_addr_gen_shift	    = 13,
	desc_cmd_r_w			    = bit(12),			// DMA��������, =1: Ϊ��ddr2д�豸; =0Ϊ���豸дddr2.

	/* write status
	 */
	desc_cmd_wr_status	   	    = (0x0F<<8),		// DMAд����״̬
	desc_cmd_wr_shift           = 8,
	
	desc_cmd_wr_idle			= (0x00<<8),		// д״̬�����ڿ���״̬
	desc_cmd_wr_ddr_wait		= (0x01<<8),		// DMA�ж���Ҫִ�ж��豸д�ڴ����,������д�ڴ�����,
													// �����ڴ滹û׼������Ӧ����,���DMAһֱ�ڵȴ��ڴ����Ӧ
	desc_cmd_wr_ddr			    = (0x02<<8),		// �ڴ������DMAд����,���ǻ�û��ִ����д����
	desc_cmd_wr_ddr_end		    = (0x03<<8),		// �ڴ������DMAд����,�����д����;��ʱDMA����д�ڴ�������״̬
	desc_cmd_wr_dma_wait		= (0x04<<8),		// DMA������DMA״̬�Ĵ���д���ڴ������,�ȴ��ڴ��������
	desc_cmd_wr_dma			    = (0x05<<8),		// �ڴ����дDMA״̬����,���ǲ�����δ���
	desc_cmd_wr_dma_end		    = (0x06<<8),		// �ڴ����дDMA״̬����
	desc_cmd_wr_step_end		= (0x07<<8),		// DMA���һ��length���ȵĲ���(Ҳ����˵���һ��step)

	/* read status
	 */
	desc_cmd_rd_status	   	    = (0x0F<<4),		// DMA������״̬
	desc_cmd_rd_shift           = 4,

	desc_cmd_rd_idle			 = (0x00<<4),		// ��״̬�����ڿ���״̬
	desc_cmd_rd_ready			 = (0x01<<4),		// ���յ���ʼDMA������start�źź�,����׼����״̬,��ʼ��������
	desc_cmd_rd_get_order		 = (0x02<<4),		// ���ڴ淢��������������,�ȴ��ڴ�Ӧ��
	desc_cmd_rd_order		     = (0x03<<4),		// �ڴ���ն�����������,����ִ�ж�����
	desc_cmd_rd_finish_order_end = (0x04<<4),		// �ڴ����DMA������
	desc_cmd_rd_ddr_wait		 = (0x05<<4),		// DMA���ڴ淢������������,�ȴ��ڴ�Ӧ��
	desc_cmd_rd_ddr			     = (0x06<<4),		// �ڴ����DMA����������,����ִ�ж����ݲ���
	desc_cmd_rd_ddr_end		     = (0x07<<4),		// �ڴ����DMA��һ�ζ���������
	desc_cmd_rd_dev			     = (0x08<<4),		// DMA������豸״̬
	desc_cmd_rd_dev_end		     = (0x09<<4),		// �豸���ض�����,�����˴ζ��豸����
	desc_cmd_rd_step_end		 = (0x0A<<4),		// ����һ��step����, step times��1
	
	desc_cmd_all_trans_over	    = bit(3),			// DMAִ���걻���õ���������������
	desc_cmd_single_trans_over  = bit(2),			// DMAִ����һ������������
	desc_cmd_int			    = bit(1),			// DMA�ж��ź�
	desc_cmd_int_enable         = bit(0),			// DMA�ж��Ƿ�����
} LS1x_dma_cmd_t;

#ifdef __cplusplus
}
#endif

#endif /* LS1x_DMA_HW_H_ */

