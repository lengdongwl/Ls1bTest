/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_fb_hw.h
 *
 *  LS1x hardware Register definitions
 *  Created on: 2013-10-25
 *      Author: Bian
 */

#ifndef LS1x_FB_HW_H_
#define LS1x_FB_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef bit
#define bit(x)              (1<<x)
#endif

#if defined(LS1B)
#define FB_BURST_SIZE		0xFF
#elif defined(LS1C)
#define FB_BURST_SIZE       0x7F
#else
#error "No Loongson SoC defined."
#endif

/*
 * FrameBuffer Display Control Registers Base
 */
#define LS1x_DC0_BASE		0xBC301240
#define LS1x_DC1_BASE       0xBC301250

typedef struct LS1x_DC_regs
{
	volatile unsigned int config;           /* 0x0000 - 0xbc301240: ���üĴ��� */
	volatile unsigned int rsv01[7];
	volatile unsigned int address0;         /* 0x0020 - 0xbc301260: ֡�����ַ�Ĵ���0, ͼ�������ڴ��׵�ַ */
	volatile unsigned int rsv02[7];
	volatile unsigned int stride;           /* 0x0040 - 0xbc301280: ��ʾ��һ�е��ֽ��� */
#if 1 // defined(LS1B)
	volatile unsigned int rsv03[7];
	volatile unsigned int origin;           /* 0x0060 - 0xbc3012A0: ��ʾ�����ԭ���ֽ���, һ��д 0 */
	volatile unsigned int rsv04[47];
#else
	volatile unsigned int rsv03[55];
#endif
	volatile unsigned int di_config;        /* 0x0120 - 0xbc301360: ��ʾ�������üĴ��� */
	volatile unsigned int rsv05[7];
	volatile unsigned int di_tablelo;       /* 0x0140 - 0xbc301380: ��ʾ������ LOW */
	volatile unsigned int rsv06[7];
	volatile unsigned int di_tablehi;       /* 0x0160 - 0xbc3013A0: ��ʾ������ HIGH */
	volatile unsigned int rsv07[7];
	volatile unsigned int pan_config;       /* 0x0180 - 0xbc3013C0: Һ��������üĴ��� */
#if 1 // defined(LS1B)
	volatile unsigned int rsv08[7];
	volatile unsigned int pan_timing;       /* 0x01A0 - 0xbc3013E0: Һ�����ʱ��Ĵ��� */
	volatile unsigned int rsv09[7];
#else
	volatile unsigned int rsv08[15];
#endif
	volatile unsigned int hdisplay;         /* 0x01C0 - 0xbc301400: ˮƽ��ʾ */
	volatile unsigned int rsv10[7];
	volatile unsigned int hsync;            /* 0x01E0 - 0xbc301420: ˮƽͬ�� */
	volatile unsigned int rsv11[23];
	volatile unsigned int vdisplay;         /* 0x0240 - 0xbc301480: ��ֱ��ʾ */
	volatile unsigned int rsv12[7];
	volatile unsigned int vsync;            /* 0x0260 - 0xbc3014A0: ��ֱͬ�� */
	volatile unsigned int rsv13[15];

	volatile unsigned int gamma_index;      /* 0x02A0 - 0xbc3014E0: ٤��У��Ŀ¼�Ĵ��� */
	volatile unsigned int rsv14[7];
	volatile unsigned int gamma_data;       /* 0x02C0 - 0xbc301500: ٤��У��ֵ�Ĵ��� */
	volatile unsigned int rsv15[7];

#if 1 // defined(LS1B)
	volatile unsigned int cr_config;        /* 0x02E0 - 0xbc301520 */
	volatile unsigned int rsv16[3];
	volatile unsigned int cr_address;       /* 0x02F0 - 0xbc301530 */
	volatile unsigned int rsv17[3];
	volatile unsigned int cr_location;      /* 0x0300 - 0xbc301540 */
	volatile unsigned int rsv18[3];
	volatile unsigned int cr_background;    /* 0x0310 - 0xbc301550 */
	volatile unsigned int rsv19[3];
	volatile unsigned int cr_foreground;    /* 0x0320 - 0xbc301560 */
	volatile unsigned int rsv20[7];

	volatile unsigned int address1;         /* 0x0340 - 0xbc301580: ֡�����ַ�Ĵ���1 */
#endif
} LS1x_DC_regs_t;

/*
 * Config Register
 */
enum
{
	fb_cfg_reset        = bit(20),		// 0: reset
	fb_cfg_gamma_en     = bit(12),		// 1: enable
	fb_cfg_switch_panel = bit(9),		// 1: to switch to another panel. XXX (2 DC)
	fb_cfg_output_en    = bit(8),		// 1: enable
	fb_cfg_format_mask  = 0x07,			// color format
	fb_cfg_format_shift = 0,
	fb_cfg_color_none   = 0,
	fb_cfg_color_r4g4b4 = 1,
	fb_cfg_color_r5g5b5 = 2,
	fb_cfg_color_r5g6b5 = 3,
	fb_cfg_color_r8g8b8 = 4,
};

/*
 * Display Dither Register
 */
enum
{
	fb_dither_enable      = bit(31),	// 1: enable
	fb_dither_red_mask    = 0x0F0000,	// bit19:16
	fb_dither_red_shift   = 16,
	fb_dither_green_mask  = 0x000F00,	// bit11:8
	fb_dither_green_shift = 8,
	fb_dither_blue_mask   = 0x00000F,	// bit3:0
	fb_dither_blue_shift  = 0,
};

/*
 * Panel Configure Register
 */
enum
{
	fb_panel_clock_pol  = bit(9),		// 1: ʱ�Ӽ����÷�, default=0
	fb_panel_clock_en   = bit(8),		// 1: ʱ��ʹ��
	fb_panel_dataen_pol = bit(1),		// 1: ����ʹ�ܼ����÷� , default=0
	fb_panel_dataen_en  = bit(0),		// 1: ����ʹ�����
};

/*
 * HDisplay Register
 */
enum
{
	fb_hdisplay_total_mask  = 0x0FFF0000,	// bit27:16, ��ʾ��һ�е�����������(��������ʾ��)
	fb_hdisplay_total_shift = 16,
	fb_hdisplay_disp_mask   = 0x00000FFF,	// bit11:0, ��ʾ��һ�е���ʾ��������
	fb_hdisplay_disp_shift  = 0,
};

/*
 * HSync Register
 */
enum
{
	fb_hsync_polarity    = bit(31),			// HSync �źŵļ���, 1: ȡ��, default=0
	fb_hsync_pulse       = bit(30),			// HSync �ź�ʹ��, 1: ʹ�����
	fb_hsync_end_mask    = 0x0FFF0000,		// bit27:16, HSync �źŽ�����������
	fb_hsync_end_shift   = 16,
	fb_hsync_start_mask  = 0x00000FFF,		// bit11:0, HSync �źſ�ʼ��������
	fb_hsync_start_shift = 0,
};

/*
 * VDisplay Register
 */
enum
{
	fb_vdisplay_total_mask  = 0x07FF0000,	// bit26:16, ��ʾ������������(����������)
	fb_vdisplay_total_shift = 16,
	fb_vdisplay_disp_mask   = 0x000007FF,	// bit10:0, ��ʾ����ʾ��������
	fb_vdisplay_disp_shift  = 0,
};

/*
 * VSync Register
 */
enum
{
	fb_vsync_polarity    = bit(31),			// VSync �źŵļ���, 1: ȡ��, default=0
	fb_vsync_pulse       = bit(30),			// VSync �ź�ʹ��, 1: ʹ�����
	fb_vsync_end_mask    = 0x0FFF0000,		// bit27:16, VSync �źŽ���������
	fb_vsync_end_shift   = 16,
	fb_vsync_start_mask  = 0x00000FFF,		// bit11:0, VSync �źſ�ʼ������
	fb_vsync_start_shift = 0,
};

/*
 * Gamma Index Register
 */
enum
{
	fb_gamma_index_mask  = 0xFF,		// bit7:0, ��ʾ��0-255��ɫֵ����һ�ʼ����, default=0,
	fb_gamma_index_shift = 0,			//         һ����һ��, �˺��ֵӲ��������
};

/*
 * Gamma Data Register
 */
enum
{
	fb_gamma_data_red_mask    = 0xFF0000, 	// bit23:16, Gamma �����ĺ�ɫ��, ��Gamma Index ��ָʾ��ֵ
	fb_gamma_data_red_shift   = 16,			//			 ����Ϊ��ǰ���ֵ
	fb_gamma_data_green_mask  = 0x00FF00,	// bit15:8
	fb_gamma_data_green_shift = 8,
	fb_gamma_data_blue_mask   = 0x0000FF,	// bit7:0
	fb_gamma_data_blue_shift  = 0,
};


/**********************************************************
 * Cursor Related Configure
 **********************************************************/

/*
 * Cursor Configure Register
 */
enum
{
	fb_cr_cfg_hotX_mask    = 0x1F0000,		// bit20:16, ָ��Ľ���ĺ�����(ָ��32��32ͼ���еĺ�����)
	fb_cr_cfg_hotX_shift   = 16,
	fb_cr_cfg_hotY_mask    = 0x001F00,		// bit12:8, ָ��Ľ����������(ָ��32��32ͼ���е�������)
	fb_cr_cfg_hotY_shift   = 8,
	fb_cr_cfg_display      = bit(4),		// ָ����ʾ���ĸ���ʾ��Ԫ��, 0=0�ŵ�Ԫ, 1=1�ŵ�Ԫ
	fb_cr_cfg_fmt_mask     = 0x03,
	fb_cr_cfg_fmt_shift    = 0,
	fb_cr_cfg_fmt_disable  = 0,
	fb_cr_cfg_fmt_masked   = 1,
	fb_cr_cfg_fmt_a8r8g8b8 = 2,
};

/*
 * Cursor Address Register
 */

/*
 * Cursor Location Register
 */
enum
{
	fb_cr_location_Y_mask  = 0x07FF0000,	// bit26:16, ָ��Ľ�����������ʾ����������
	fb_cr_location_Y_shift = 16,
	fb_cr_location_X_mask   = 0x000007FF,	// bit10:0, ָ��Ľ�����������ʾ���ĺ�����
	fb_cr_location_X_shift  = 0,
};

/*
 * Cursor BackGround Register
 */
enum
{
	fb_cr_bg_red_mask    = 0xFF0000,	// bit23:16, ָ�뵥ɫģʽ�±���ɫ�ĺ�ɫ��
	fb_cr_bg_red_shift   = 16,
	fb_cr_bg_green_mask  = 0x00FF00,	// bit15:8
	fb_cr_bg_green_shift = 8,
	fb_cr_bg_blue_mask   = 0x0000FF,	// bit7:0
	fb_cr_bg_blue_shift  = 0,
};

/*
 * Cursor ForeGround Register
 */
enum
{
	fb_cr_fg_red_mask    = 0xFF0000,	// bit23:16, ָ�뵥ɫģʽ��ǰ��ɫ�ĺ�ɫ��
	fb_cr_fg_red_shift   = 16,
	fb_cr_fg_green_mask  = 0x00FF00,	// bit15:8
	fb_cr_fg_green_shift = 8,
	fb_cr_fg_blue_mask   = 0x0000FF,	// bit7:0
	fb_cr_fg_blue_shift  = 0,
};

#ifdef __cplusplus
}
#endif

#endif /* LS1x_FB_HW_H_ */
