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
	volatile unsigned int csr;                  /* 0x00 配置状态寄存器 */
	volatile unsigned int occ0;                 /* 0x04 输出通道配置寄存器0 */
	volatile unsigned int occ1;                 /* 0x08 保留 */
	volatile unsigned int occ2;                 /* 0x0c 保留 */
	volatile unsigned int icc;                  /* 0x10 输入通道配置寄存器 */
	volatile unsigned int codec_id;             /* 0x14 Codec ID 寄存器 */
	volatile unsigned int crac;                 /* 0x18 Codec寄存器访问命令 */
	volatile unsigned int rsv1;
	volatile unsigned int oc0;                  /* 0x20 输出声道0 */
	volatile unsigned int oc1;                  /* 0x24 输出声道1 */
	volatile unsigned int oc2;                  /* 0x28 保留 */
	volatile unsigned int oc3;                  /* 0x2c 保留 */
	volatile unsigned int oc4;                  /* 0x30 保留 */
	volatile unsigned int oc5;                  /* 0x34 保留 */
	volatile unsigned int oc6;                  /* 0x38 保留 */
	volatile unsigned int oc7;                  /* 0x3c 保留 */
	volatile unsigned int oc8;                  /* 0x40 保留 */
	volatile unsigned int ic0;                  /* 0x44 保留 */
	volatile unsigned int ic1;                  /* 0x48 保留 */
	volatile unsigned int ic2;                  /* 0x4c 输入声道2 */
	volatile unsigned int rsv2;
	volatile unsigned int intraw;               /* 0x54 中断状态寄存器 */
	volatile unsigned int intmask;              /* 0x58 中断掩膜寄存器 */
	volatile unsigned int intclr;               /* 0x5c 中断清除寄存器. 屏蔽后的中断状态寄存器,
                                                        对本寄存器的读操作将清除寄存器0x54中的
                                                        所有中断状态 */

	volatile unsigned int oc_intclr;            /* 0x60 OC中断清除寄存器. 对本寄存器的读操作将
                                                        清除寄存器0x54中的所有output channel的
                                                        中断状态对应的bit[7:2] */

	volatile unsigned int ic_intclr;            /* 0x64 IC中断清除寄存器. 对本寄存器的读操作将
                                                        清除寄存器0x54中的所有input channel的中
                                                        断状态对应的bit[31:30] */

	volatile unsigned int wr_intclr;            /* 0x68 CODEC WRITE 中断清除寄存器. 对本寄存器
                                                        的读操作将清除寄存器0x54中的中bit[1] */

	volatile unsigned int rd_intclr;            /* 0x6c CODEC READ 中断清除寄存器. 对本寄存器的
                                                        读操作将清除寄存器0x54中的中bit[0] */
} LS1x_AC97_regs_t;

/*
 * 0x00 配置状态寄存器
 */
#define AC97_CSR_SUSPEND        bit(1)          /* 1：AC97子系统挂起; 0：正常工作状态 */
#define AC97_CSR_RESUME         bit(1)          /* 在挂起状态下, 写入1到该位, 将会开始恢复操作 */
#define AC97_CSR_RESET          bit(0)          /* 写入1会导致AC97 Codec冷启动 */

/*
 * 输出通道配置寄存器, 复位值 0x00004141
 */
#define AC97_OCH1_R_MASK        0xFF00          /* bit[15:8] 输出通道1：右声道配置 */
#define AC97_OCH1_R_SHIFT       8
#define AC97_OCH0_L_MASK        0x00FF          /* bit[7:0]  输出通道0：左声道配置 */
#define AC97_OCH0_L_SHIFT       0

/*
 * 输入通道配置寄存器, 复位值 0x00410000
 */
#define AC97_ICH2_MIC_MASK      0x00FF0000      /* 输入通道2：MIC声道配置 */
#define AC97_ICH2_MIC_SHIFT     16
#define AC97_ICH1_R_MASK        0x0000FF00      /* bit[15:8] 输入通道1：右声道配置 */
#define AC97_ICH1_R_SHIFT       8
#define AC97_ICH0_L_MASK        0x000000FF      /* bit[7:0]  输入通道0：左声道配置 */
#define AC97_ICH0_L_SHIFT       0

/*
 * 通道格式
 */
#define CHNL_DMA_EN             bit(6)          /* DMA使能. 1：DMA打开; 0：DMA关闭 */
#define CHNL_FIFO_THRES_MASK    0x0030          /* bit[5:4], FIFO门限 */
#define CHNL_FIFO_THRES_SHIFT   4
#define CHNL_SAMP_WIDTH_MASK    0x000C          /* bit[3:2], 采样位数. 00：8位; 10：16位 */
#define CHNL_SAMP_WIDTH_SHIFT   2
#define CHNL_SAMP_WIDTH_8       0x00
#define CHNL_SAMP_WIDTH_16      0x08
#define CHNL_VAR_SAMP_RATE      bit(1)          /* 采样率. 1：采样率可变; 0：采样率固定(48KHz) */
#define CHNL_EN                 bit(0)          /* 通道使能. 1:通道打开; 0:通道关闭(或者进入节能状态) */

/*
 * Codec寄存器访问命令
 */
#define AC97_CODEC_WR           bit(31)         /* 1: 读, 读取数据时, 先设置CODEC_WR为读方式, 并在
                                                      CODEC_ADR设置欲访问的寄存器地址; 等到返回数据
                                                      完成中断时再读CODEC_DAT寄存器读取值.
                                                   0: 写 */

#define AC97_CODEC_ADR_MASK     0x7F0000        /* Codec寄存器地址 */
#define AC97_CODEC_ADR_SHIFT    16

#define AC97_CODEC_DAT_MASK     0x00FFFF        /* Codec寄存器数据 */

/*
 * 中断状态寄存器/中断掩膜寄存器. 复位值: 0x00000000
 */
#define AC97_INT_ICH2_FULL      bit(31)         /* 输入通道2：FIFO满 */
#define AC97_INT_ICH2_THRES     bit(30)         /* 输入通道2: FIFO达到门限 */

#define AC97_INT_OCH1_FULL      bit(7)          /* 输出通道1: FIFO满 */
#define AC97_INT_OCH1_EMPTY     bit(6)          /* 输出通道1: FIFO空 */
#define AC97_INT_OCH1_THRES     bit(5)          /* 输出通道1: FIFO达到门限 */

#define AC97_INT_OCH0_FULL      bit(4)          /* 输出通道0: FIFO满 */
#define AC97_INT_OCH0_EMPTY     bit(3)          /* 输出通道0: FIFO空 */
#define AC97_INT_OCH0_THRES     bit(2)          /* 输出通道0: FIFO达到门限 */

#define AC97_INT_CW_DONE        bit(1)          /* Codec寄存器写完成 */
#define AC97_INT_CR_DONE        bit(0)          /* Codec寄存器读完成 */


#ifdef __cplusplus
}
#endif

#endif // _LS1X_AC97_HW_H

