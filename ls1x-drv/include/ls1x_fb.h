/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_fb.h
 *
 *  LS1x framebuffer definitions
 *  Created on: 2013-10-25
 *      Author: Bian
 */

#ifndef LS1x_FB_H_
#define LS1x_FB_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(OS_RTTHREAD)
#include "rtthread.h"
#elif defined(OS_UCOS)
#include "os.h"
#elif defined(OS_FREERTOS)
#include "FreeRTOS.h"
#include "semphr.h"
#endif

#include "bsp.h"

/******************************************************************************
 * framebuffer device
 ******************************************************************************/

#include "fb.h"
#include "ls1x_io.h"
#include "../fb/ls1x_fb_hw.h"

#define FB_BUF_K0_ADDR      1                   /* use K0 or K1 */

typedef struct LS1x_DC_dev
{
    LS1x_DC_regs_t *hwDC;                       /* DC control */

    struct fb_fix_screeninfo fb_fix;            /* framebuffer standard device */
    struct fb_var_screeninfo fb_var;            /* framebuffer standard device */

    int initialized;                            /* �Ƿ��ʼ�� */
    int started;                                /* �Ƿ����� */
    
    /* mutex */
#if defined(OS_RTTHREAD)
    rt_mutex_t dc_mutex;
#elif defined(OS_UCOS)
    OS_EVENT  *dc_mutex;
#elif defined(OS_FREERTOS)
    SemaphoreHandle_t dc_mutex;
#else // defined(OS_NONE)
    int dc_mutex;
#endif
} LS1x_DC_dev_t;

//-----------------------------------------------------------------------------
// FrameBuffer devices
//-----------------------------------------------------------------------------

#ifdef BSP_USE_FB
extern LS1x_DC_dev_t *devDC;
#endif

//-----------------------------------------------------------------------------
// ioctl command
//-----------------------------------------------------------------------------

#define IOCTRL_FB_CLEAR_BUFFER  0x46B1
#define IOCTRL_LCD_POWERON      0x46B2
#define IOCTRL_LCD_POWEROFF     0x46B3

//-----------------------------------------------------------------------------
// FrameBuffer driver operators
//-----------------------------------------------------------------------------

#include "ls1x_io.h"

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_dc_drv_ops;

#define ls1x_dc_init(dc, arg)             ls1x_dc_drv_ops->init_entry(dc, arg)
#define ls1x_dc_open(dc, arg)             ls1x_dc_drv_ops->open_entry(dc, arg)
#define ls1x_dc_close(dc, arg)            ls1x_dc_drv_ops->close_entry(dc, arg)
#define ls1x_dc_read(dc, buf, size, arg)  ls1x_dc_drv_ops->read_entry(dc, buf, size, arg)
#define ls1x_dc_write(dc, buf, size, arg) ls1x_dc_drv_ops->write_entry(dc, buf, size, arg)
#define ls1x_dc_ioctl(dc, cmd, arg)       ls1x_dc_drv_ops->ioctl_entry(dc, cmd, arg)

#else

/*
 * ��ʼ��DC�豸
 * ����:    dev     devDC
 *          arg     NULL
 *
 * ����:    0=�ɹ�
 */
int LS1x_DC_initialize(void *dev, void *arg);

/*
 * ��DC�豸
 * ����:    dev     devDC
 *          arg     NULL
 *
 * ����:    0=�ɹ�
 */
int LS1x_DC_open(void *dev, void *arg);

/*
 * �ر�DC�豸
 * ����:    dev     devDC
 *          arg     NULL
 *
 * ����:    0=�ɹ�
 */
int LS1x_DC_close(void *dev, void *arg);

/*
 * ����ʾ������
 * ����:    dev     devDC
 *          buf     ����: char *, ���ڴ�Ŷ�ȡ���ݵĻ�����
 *          size    ����: int, ����ȡ���ֽ���, ���Ȳ��ܳ��� buf ������
 *          arg     ����: unsigned int, framebuffer��ʾ�������ڵ�ַƫ����.
 *
 * ����:    ��ȡ���ֽ���
 */
int LS1x_DC_read(void *dev, void *buf, int size, void *arg);

/*
 * д��ʾ������
 * ����:    dev     devDC
 *          buf     ����: char *, ���ڴ�Ŵ�д���ݵĻ�����
 *          size    ����: int, ��д����ֽ���, ���Ȳ��ܳ��� buf ������
 *          arg     ����: unsigned int, framebuffer��ʾ�������ڵ�ַƫ����.
 *
 * ����:    д����ֽ���
 */
int LS1x_DC_write(void *dev, void *buf, int size, void *arg);

/*
 * ��DC�豸���Ϳ�������
 * ����:    dev     devDC
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                         |   arg
 *      ---------------------------------------------------------------------------------
 *          FBIOGET_FSCREENINFO         |   ����: struct fb_fix_screeninfo *
 *                                      |   ��;: ��ȡframebuffer�̶���ʾ��Ϣ
 *      ---------------------------------------------------------------------------------
 *          FBIOGET_VSCREENINFO         |   ����: struct fb_var_screeninfo *
 *                                      |   ��;: ��ȡframebuffer������ʾ��Ϣ
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          FBIOGETCMAP                 |   ����: struct fb_cmap *
 *                                      |   ��;: ��ȡframebuffer��ɫ��
 *      ---------------------------------------------------------------------------------
 *          FBIOPUTCMAP                 |   ����: struct fb_cmap *
 *                                      |   ��;: ����framebuffer��ɫ��
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTRL_FB_CLEAR_BUFFER      |   ����: unsigned int
 *                                      |   ��;: ��ָ��ֵ�����ʾ������(��ɫֵ)
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTRL_LCD_POWERON          |   TODO ���Ӳ�������LCD��Դ���Ƶ�·, ����ʵ��
 *      ---------------------------------------------------------------------------------
 *          IOCTRL_LCD_POWEROFF         |   TODO ���Ӳ�������LCD��Դ���Ƶ�·, ����ʵ��
 *      ---------------------------------------------------------------------------------
 *
 * ����:    0=�ɹ�
 */
int LS1x_DC_ioctl(void *dev, int cmd, void *arg);

#define ls1x_dc_init(dc, arg)             LS1x_DC_initialize(dc, arg)
#define ls1x_dc_open(dc, arg)             LS1x_DC_open(dc, arg)
#define ls1x_dc_close(dc, arg)            LS1x_DC_close(dc, arg)
#define ls1x_dc_read(dc, buf, size, arg)  LS1x_DC_read(dc, buf, size, arg)
#define ls1x_dc_write(dc, buf, size, arg) LS1x_DC_write(dc, buf, size, arg)
#define ls1x_dc_ioctl(dc, cmd, arg)       LS1x_DC_ioctl(dc, cmd, arg)

#endif

/******************************************************************************
 * user api
 ******************************************************************************/

/*
 * ��ʾ�������Ƿ��ʼ��
 * ����:    1=�ѳ�ʼ��; 0=δ��ʼ��
 */
int ls1x_dc_initialized(void);

/*
 * ��ʾ�������Ƿ�����(open)
 * ����:    1=������; 0=δ����
 */
int ls1x_dc_started(void);

/*
 * for RT-Thread
 */
#if defined(OS_RTTHREAD)
#define LS1x_FB_DEVICE_NAME     "fb0"
#endif

/******************************************************************************
 * LCD resolution
 ******************************************************************************/

/*
 * LCD suported vgamode
 */
#define LCD_480x272     "480x272-16@60"     /* Fit: 4 inch LCD */
#define LCD_480x800     "480x800-16@60"     /* Fit: 4 inch ���� */
#define LCD_800x480     "800x480-16@60"     /* Fit: 7 inch LCD */

/*
 * ȫ�ֱ���: ��ʾģʽ. ��ǰ����.
 */
extern char LCD_display_mode[];             /* likely LCD_480x272 */

/******************************************************************************
 *
 * Framebuffer Applicaton Interface Functions, in "lx1x_fb_utils.c"
 *
 ******************************************************************************/

//-----------------------------------------------------------------------------
// ͨ�ú���
//-----------------------------------------------------------------------------

extern int fb_open(void);                   /* ��ʼ������framebuffer���� */
extern void fb_close(void);                 /* �ر�framebuffer����  */

extern int fb_get_pixelsx(void);            /* ����LCD��X�ֱ��� */
extern int fb_get_pixelsy(void);            /* ����LCD��Y�ֱ��� */

//-----------------------------------------------------------------------------
// ������ɫ������
//-----------------------------------------------------------------------------

extern void fb_set_color(unsigned coloridx, unsigned value);    /* �趨��ɫ������colidx������ɫ */
extern unsigned fb_get_color(unsigned coloridx);                /* ��ȡ��ɫ������colidx������ɫ */

//-----------------------------------------------------------------------------
// �����ַ�ǰ�󱳾�ɫ
//-----------------------------------------------------------------------------

extern void fb_set_bgcolor(unsigned coloridx, unsigned value);  /* �����ַ����ʹ�õı���ɫ */
extern void fb_set_fgcolor(unsigned coloridx, unsigned value);  /* �����ַ����ʹ�õ�ǰ��ɫ */

//-----------------------------------------------------------------------------
// LCD����̨��ӡ
//-----------------------------------------------------------------------------

extern void fb_cons_putc(char chr);                             /* ��LCD����̨���һ���ַ� */
extern void fb_cons_puts(char *str);                            /* ��LCD����̨���һ���ַ��� */
extern void fb_cons_clear(void);                                /* ִ��LCD����̨���� */

//-----------------------------------------------------------------------------
// �ı����
//-----------------------------------------------------------------------------

extern void fb_textout(int x, int y, char *str);                /* ��LCD[x,y]����ӡ�ַ��� */

//-----------------------------------------------------------------------------
// ��ʾ BMPͼ��
//-----------------------------------------------------------------------------

extern int fb_showbmp(int x, int y, char *bmpfilename);         /* ��LCD[x,y]����ʾbmpͼ�� */

extern void display_pic(unsigned int xpos,unsigned int ypos,unsigned int x1,unsigned int y1,unsigned char *ptrs);

//-----------------------------------------------------------------------------
// LCD ��ͼ����
//-----------------------------------------------------------------------------

/* ��LCD[x,y]��������������
 */
extern void fb_put_cross(int x, int y, unsigned coloridx);

/* ��LCD[x,y]����ָ����ɫ��ӡ�ַ���
 */
extern void fb_put_string(int x, int y, char *str, unsigned coloridx);

/* ��LCD����[x,y]Ϊ������ָ����ɫ��ӡ�ַ���
 */
extern void fb_put_string_center(int x, int y, char *str, unsigned coloridx);

/* ��LCD[x,y]����ָ����ɫ������
 */
extern void fb_drawpixel(int x, int y, unsigned coloridx);

/* ��LCD[x,y]����ָ����ɫ����Ȼ���
 */
extern void fb_drawpoint(int x, int y, int thickness, unsigned coloridx);

/* ��LCD[x1,y1]~[x2,y2]����ָ����ɫ����
 */
extern void fb_drawline(int x1, int y1, int x2, int y2, unsigned coloridx);

/* ��LCD[x1,y1]~[x2,y2]����ָ����ɫ�����ο�
 */
extern void fb_drawrect(int x1, int y1, int x2, int y2, unsigned coloridx);

/* ��LCD[x1,y1]~[x2,y2]����ָ����ɫ�����ο�
 */
extern void fb_fillrect(int x1, int y1, int x2, int y2, unsigned coloridx);

/* ��LCD[x1,y1]~[x2,y2]����ͼ���[x1,y1]�ƶ���[px, py]��λ��
 */
extern void fb_copyrect(int x1, int y1, int x2, int y2, int px, int py);

/******************************************************************************
 * shorten name
 ******************************************************************************/

#define SetColor        fb_set_color
#define GetColor        fb_get_color
#define SetBGColor      fb_set_bgcolor
#define SetFGColor      fb_set_fgcolor

#define TextOut         fb_textout
#define PutString       fb_put_string
#define PutStringCenter fb_put_string_center

#define DrawPixel       fb_drawpixel
#define DrawPoint       fb_drawpoint
#define DrawLine        fb_drawline
#define DrawRect        fb_drawrect
#define FillRect        fb_fillrect
#define CopyRect        fb_copyrect

/******************************************************************************
 * �ֿ����ʾ
 ******************************************************************************/

#define XORMODE             0x80000000

/*
 * ��ɫ�� RGB888, ʹ�� set_color() ����
 */
#define clBLACK             0x00

#define clRED               (0xA0 << 16)
#define clGREEN             (0xA0 << 8)
#define clBLUE              (0xA0 << 0)

#define clCYAN              (clBLUE | clGREEN)
#define clVIOLET            (clRED  | clBLUE)
#define clYELLOW            (clRED  | clGREEN)
#define clWHITE             (clRED  | clGREEN | clBLUE)

/* half brightness */
#define clhRED              (0x50 << 16)
#define clhGREEN            (0x50 << 8)
#define clhBLUE             (0x50 << 0)
/* more brightness */
#define clbRED              (0xF0 << 16)
#define clbGREEN            (0xF0 << 8)
#define clbBLUE             (0xF0 << 0)

#define clGREY              (clhRED | clhGREEN | clhBLUE)
#define clBRTBLUE           (clhRED | clhGREEN | clbBLUE)
#define clBRTGREEN          (clhRED | clbGREEN | clhBLUE)
#define clBRTCYAN           (clhRED | clbGREEN | clbBLUE)
#define clBRTRED            (clbRED | clhGREEN | clhBLUE)
#define clBRTVIOLET         (clbRED | clhGREEN | clbBLUE)
#define clBRTYELLOW         (clbRED | clbGREEN | clhBLUE)
#define clBRTWHITE          (clhRED | clhGREEN | clhBLUE)

#define clBTNFACE           0x00808080
#define clSILVER            0x00C0C0C0
#define clHINT              0x00E4F0F0

/*
 * ��ɫ���� RGB565, ��ͨ�� get_color() ��ȡ.
 */
#define cidxBLACK           0
#define cidxBLUE            1
#define cidxGREEN           2
#define cidxCYAN            3
#define cidxRED             4
#define cidxVIOLET          5
#define cidxYELLOW          6
#define cidxWHITE           7
#define cidxGREY            8
#define cidxBRTBLUE         9
#define cidxBRTGREEN        10
#define cidxBRTCYAN         11
#define cidxBRTRED          12
#define cidxBRTVIOLET       13
#define cidxBRTYELLOW       14
#define cidxBRTWHITE        15

#define cidxBTNFACE         16
#define cidxSILVER          17

/******************************************************************************
 * XXX how to use font, please reference "font_desc.h"
 ******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* LS1x_FB_H_ */

