/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 *  LS1X AC97 Driver
 *
 *  Author: Bian, 2021/3/20
 */

#ifndef _LS1X_AC97_HW_H
#define _LS1X_AC97_HW_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef bit
#define bit(x)   (1<<x)
#endif

/*
 * AC97 Registers.
 */
typedef struct LS1x_AC97_regs
{
	volatile unsigned int csr;                  /* 0x00 ����״̬�Ĵ��� */
	volatile unsigned int occ0;                 /* 0x04 ���ͨ�����üĴ���0 */
	volatile unsigned int occ1;                 /* 0x08 ���� */
	volatile unsigned int occ2;                 /* 0x0c ���� */
	volatile unsigned int icc;                  /* 0x10 ����ͨ�����üĴ��� */
	volatile unsigned int codec_id;             /* 0x14 Codec ID �Ĵ��� */
	volatile unsigned int crac;                 /* 0x18 Codec�Ĵ����������� */
	volatile unsigned int rsv1;
	volatile unsigned int oc0;                  /* 0x20 �������0 */
	volatile unsigned int oc1;                  /* 0x24 �������1 */
	volatile unsigned int oc2;                  /* 0x28 ���� */
	volatile unsigned int oc3;                  /* 0x2c ���� */
	volatile unsigned int oc4;                  /* 0x30 ���� */
	volatile unsigned int oc5;                  /* 0x34 ���� */
	volatile unsigned int oc6;                  /* 0x38 ���� */
	volatile unsigned int oc7;                  /* 0x3c ���� */
	volatile unsigned int oc8;                  /* 0x40 ���� */
	volatile unsigned int ic0;                  /* 0x44 ���� */
	volatile unsigned int ic1;                  /* 0x48 ���� */
	volatile unsigned int ic2;                  /* 0x4c ��������2 */
	volatile unsigned int rsv2;
	volatile unsigned int intraw;               /* 0x54 �ж�״̬�Ĵ��� */
	volatile unsigned int intmask;              /* 0x58 �ж���Ĥ�Ĵ��� */
	volatile unsigned int intclr;               /* 0x5c �ж�����Ĵ���. ���κ���ж�״̬�Ĵ���,
                                                        �Ա��Ĵ����Ķ�����������Ĵ���0x54�е�
                                                        �����ж�״̬ */

	volatile unsigned int oc_intclr;            /* 0x60 OC�ж�����Ĵ���. �Ա��Ĵ����Ķ�������
                                                        ����Ĵ���0x54�е�����output channel��
                                                        �ж�״̬��Ӧ��bit[7:2] */

	volatile unsigned int ic_intclr;            /* 0x64 IC�ж�����Ĵ���. �Ա��Ĵ����Ķ�������
                                                        ����Ĵ���0x54�е�����input channel����
                                                        ��״̬��Ӧ��bit[31:30] */

	volatile unsigned int wr_intclr;            /* 0x68 CODEC WRITE �ж�����Ĵ���. �Ա��Ĵ���
                                                        �Ķ�����������Ĵ���0x54�е���bit[1] */

	volatile unsigned int rd_intclr;            /* 0x6c CODEC READ �ж�����Ĵ���. �Ա��Ĵ�����
                                                        ������������Ĵ���0x54�е���bit[0] */
} LS1x_AC97_regs_t;

/*
 * 0x00 ����״̬�Ĵ���
 */
#define AC97_CSR_SUSPEND        bit(1)          /* 1��AC97��ϵͳ����; 0����������״̬ */
#define AC97_CSR_RESUME         bit(1)          /* �ڹ���״̬��, д��1����λ, ���Ὺʼ�ָ����� */
#define AC97_CSR_RESET          bit(0)          /* д��1�ᵼ��AC97 Codec������ */

/*
 * ���ͨ�����üĴ���, ��λֵ 0x00004141
 */
#define AC97_OCH1_R_MASK        0xFF00          /* bit[15:8] ���ͨ��1������������ */
#define AC97_OCH1_R_SHIFT       8
#define AC97_OCH0_L_MASK        0x00FF          /* bit[7:0]  ���ͨ��0������������ */
#define AC97_OCH0_L_SHIFT       0

/*
 * ����ͨ�����üĴ���, ��λֵ 0x00410000
 */
#define AC97_ICH2_MIC_MASK      0x00FF0000      /* ����ͨ��2��MIC�������� */
#define AC97_ICH2_MIC_SHIFT     16
#define AC97_ICH1_R_MASK        0x0000FF00      /* bit[15:8] ����ͨ��1������������ */
#define AC97_ICH1_R_SHIFT       8
#define AC97_ICH0_L_MASK        0x000000FF      /* bit[7:0]  ����ͨ��0������������ */
#define AC97_ICH0_L_SHIFT       0

/*
 * ͨ����ʽ
 */
#define CHNL_DMA_EN             bit(6)          /* DMAʹ��. 1��DMA��; 0��DMA�ر� */
#define CHNL_FIFO_THRES_MASK    0x0030          /* bit[5:4], FIFO���� */
#define CHNL_FIFO_THRES_SHIFT   4
#define CHNL_SAMP_WIDTH_MASK    0x000C          /* bit[3:2], ����λ��. 00��8λ; 10��16λ */
#define CHNL_SAMP_WIDTH_SHIFT   2
#define CHNL_SAMP_WIDTH_8       0x00
#define CHNL_SAMP_WIDTH_16      0x08
#define CHNL_VAR_SAMP_RATE      bit(1)          /* ������. 1�������ʿɱ�; 0�������ʹ̶�(48KHz) */
#define CHNL_EN                 bit(0)          /* ͨ��ʹ��. 1:ͨ����; 0:ͨ���ر�(���߽������״̬) */

/*
 * Codec�Ĵ�����������
 */
#define AC97_CODEC_WR           bit(31)         /* 1: ��, ��ȡ����ʱ, ������CODEC_WRΪ����ʽ, ����
                                                      CODEC_ADR���������ʵļĴ�����ַ; �ȵ���������
                                                      ����ж�ʱ�ٶ�CODEC_DAT�Ĵ�����ȡֵ.
                                                   0: д */

#define AC97_CODEC_ADR_MASK     0x7F0000        /* Codec�Ĵ�����ַ */
#define AC97_CODEC_ADR_SHIFT    16

#define AC97_CODEC_DAT_MASK     0x00FFFF        /* Codec�Ĵ������� */

/*
 * �ж�״̬�Ĵ���/�ж���Ĥ�Ĵ���. ��λֵ: 0x00000000
 */
#define AC97_INT_ICH2_FULL      bit(31)         /* ����ͨ��2��FIFO�� */
#define AC97_INT_ICH2_THRES     bit(30)         /* ����ͨ��2: FIFO�ﵽ���� */

#define AC97_INT_OCH1_FULL      bit(7)          /* ���ͨ��1: FIFO�� */
#define AC97_INT_OCH1_EMPTY     bit(6)          /* ���ͨ��1: FIFO�� */
#define AC97_INT_OCH1_THRES     bit(5)          /* ���ͨ��1: FIFO�ﵽ���� */

#define AC97_INT_OCH0_FULL      bit(4)          /* ���ͨ��0: FIFO�� */
#define AC97_INT_OCH0_EMPTY     bit(3)          /* ���ͨ��0: FIFO�� */
#define AC97_INT_OCH0_THRES     bit(2)          /* ���ͨ��0: FIFO�ﵽ���� */

#define AC97_INT_CW_DONE        bit(1)          /* Codec�Ĵ���д��� */
#define AC97_INT_CR_DONE        bit(0)          /* Codec�Ĵ�������� */


#ifdef __cplusplus
}
#endif

#endif // _LS1X_AC97_HW_H

