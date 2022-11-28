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

    int initialized;                            /* 是否初始化 */
    int started;                                /* 是否启动 */
    
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
 * 初始化DC设备
 * 参数:    dev     devDC
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int LS1x_DC_initialize(void *dev, void *arg);

/*
 * 打开DC设备
 * 参数:    dev     devDC
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int LS1x_DC_open(void *dev, void *arg);

/*
 * 关闭DC设备
 * 参数:    dev     devDC
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int LS1x_DC_close(void *dev, void *arg);

/*
 * 读显示缓冲区
 * 参数:    dev     devDC
 *          buf     类型: char *, 用于存放读取数据的缓冲区
 *          size    类型: int, 待读取的字节数, 长度不能超过 buf 的容量
 *          arg     类型: unsigned int, framebuffer显示缓冲区内地址偏移量.
 *
 * 返回:    读取的字节数
 */
int LS1x_DC_read(void *dev, void *buf, int size, void *arg);

/*
 * 写显示缓冲区
 * 参数:    dev     devDC
 *          buf     类型: char *, 用于存放待写数据的缓冲区
 *          size    类型: int, 待写入的字节数, 长度不能超过 buf 的容量
 *          arg     类型: unsigned int, framebuffer显示缓冲区内地址偏移量.
 *
 * 返回:    写入的字节数
 */
int LS1x_DC_write(void *dev, void *buf, int size, void *arg);

/*
 * 向DC设备发送控制命令
 * 参数:    dev     devDC
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                         |   arg
 *      ---------------------------------------------------------------------------------
 *          FBIOGET_FSCREENINFO         |   类型: struct fb_fix_screeninfo *
 *                                      |   用途: 读取framebuffer固定显示信息
 *      ---------------------------------------------------------------------------------
 *          FBIOGET_VSCREENINFO         |   类型: struct fb_var_screeninfo *
 *                                      |   用途: 读取framebuffer可视显示信息
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          FBIOGETCMAP                 |   类型: struct fb_cmap *
 *                                      |   用途: 读取framebuffer调色板
 *      ---------------------------------------------------------------------------------
 *          FBIOPUTCMAP                 |   类型: struct fb_cmap *
 *                                      |   用途: 设置framebuffer调色板
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTRL_FB_CLEAR_BUFFER      |   类型: unsigned int
 *                                      |   用途: 用指定值填充显示缓冲区(颜色值)
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTRL_LCD_POWERON          |   TODO 如果硬件设计有LCD电源控制电路, 补充实现
 *      ---------------------------------------------------------------------------------
 *          IOCTRL_LCD_POWEROFF         |   TODO 如果硬件设计有LCD电源控制电路, 补充实现
 *      ---------------------------------------------------------------------------------
 *
 * 返回:    0=成功
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
 * 显示控制器是否初始化
 * 返回:    1=已初始化; 0=未初始化
 */
int ls1x_dc_initialized(void);

/*
 * 显示控制器是否启动(open)
 * 返回:    1=已启动; 0=未启动
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
#define LCD_480x800     "480x800-16@60"     /* Fit: 4 inch 竖屏 */
#define LCD_800x480     "800x480-16@60"     /* Fit: 7 inch LCD */

/*
 * 全局变量: 显示模式. 打开前调用.
 */
extern char LCD_display_mode[];             /* likely LCD_480x272 */

/******************************************************************************
 *
 * Framebuffer Applicaton Interface Functions, in "lx1x_fb_utils.c"
 *
 ******************************************************************************/

//-----------------------------------------------------------------------------
// 通用函数
//-----------------------------------------------------------------------------

extern int fb_open(void);                   /* 初始化并打开framebuffer驱动 */
extern void fb_close(void);                 /* 关闭framebuffer驱动  */

extern int fb_get_pixelsx(void);            /* 返回LCD的X分辨率 */
extern int fb_get_pixelsy(void);            /* 返回LCD的Y分辨率 */

//-----------------------------------------------------------------------------
// 设置颜色索引表
//-----------------------------------------------------------------------------

extern void fb_set_color(unsigned coloridx, unsigned value);    /* 设定颜色索引表colidx处的颜色 */
extern unsigned fb_get_color(unsigned coloridx);                /* 读取颜色索引表colidx处的颜色 */

//-----------------------------------------------------------------------------
// 设置字符前后背景色
//-----------------------------------------------------------------------------

extern void fb_set_bgcolor(unsigned coloridx, unsigned value);  /* 设置字符输出使用的背景色 */
extern void fb_set_fgcolor(unsigned coloridx, unsigned value);  /* 设置字符输出使用的前景色 */

//-----------------------------------------------------------------------------
// LCD控制台打印
//-----------------------------------------------------------------------------

extern void fb_cons_putc(char chr);                             /* 在LCD控制台输出一个字符 */
extern void fb_cons_puts(char *str);                            /* 在LCD控制台输出一个字符串 */
extern void fb_cons_clear(void);                                /* 执行LCD控制台清屏 */

//-----------------------------------------------------------------------------
// 文本输出
//-----------------------------------------------------------------------------

extern void fb_textout(int x, int y, char *str);                /* 在LCD[x,y]处打印字符串 */

//-----------------------------------------------------------------------------
// 显示 BMP图像
//-----------------------------------------------------------------------------

extern int fb_showbmp(int x, int y, char *bmpfilename);         /* 在LCD[x,y]处显示bmp图像 */

extern void display_pic(unsigned int xpos,unsigned int ypos,unsigned int x1,unsigned int y1,unsigned char *ptrs);

//-----------------------------------------------------------------------------
// LCD 绘图函数
//-----------------------------------------------------------------------------

/* 在LCD[x,y]处画”＋”符号
 */
extern void fb_put_cross(int x, int y, unsigned coloridx);

/* 在LCD[x,y]处用指定颜色打印字符串
 */
extern void fb_put_string(int x, int y, char *str, unsigned coloridx);

/* 在LCD上以[x,y]为中心用指定颜色打印字符串
 */
extern void fb_put_string_center(int x, int y, char *str, unsigned coloridx);

/* 在LCD[x,y]处用指定颜色画像素
 */
extern void fb_drawpixel(int x, int y, unsigned coloridx);

/* 在LCD[x,y]处用指定颜色、宽度画点
 */
extern void fb_drawpoint(int x, int y, int thickness, unsigned coloridx);

/* 在LCD[x1,y1]~[x2,y2]处用指定颜色画线
 */
extern void fb_drawline(int x1, int y1, int x2, int y2, unsigned coloridx);

/* 在LCD[x1,y1]~[x2,y2]处用指定颜色画矩形框
 */
extern void fb_drawrect(int x1, int y1, int x2, int y2, unsigned coloridx);

/* 在LCD[x1,y1]~[x2,y2]处用指定颜色填充矩形框
 */
extern void fb_fillrect(int x1, int y1, int x2, int y2, unsigned coloridx);

/* 把LCD[x1,y1]~[x2,y2]处的图像从[x1,y1]移动到[px, py]的位置
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
 * 字库和显示
 ******************************************************************************/

#define XORMODE             0x80000000

/*
 * 颜色表 RGB888, 使用 set_color() 设置
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
 * 颜色索引 RGB565, 可通过 get_color() 获取.
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

