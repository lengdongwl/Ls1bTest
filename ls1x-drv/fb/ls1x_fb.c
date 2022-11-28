/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * LS1x_fb.c
 *
 *  LS1x LCD driver
 *  Created on: 2013-10-28
 *      Author: Bian
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "bsp.h"

#ifdef BSP_USE_FB

#if defined(LS1B)
#include "ls1b.h"
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_gpio.h"
#else
#error "No Loongson1x SoC defined."
#endif

#include "cpu.h"
#include "fb.h"

#include "ls1x_io.h"
#include "drv_os_priority.h"

#include "LS1x_fb_hw.h"
#include "LS1x_fb.h"

#if defined(OS_RTTHREAD)
#define MALLOC  rt_malloc
#define FREE    rt_free
#else
#define MALLOC  malloc
#define FREE    free
#endif

/*
 * 使用指定内存地址
 */
#ifdef MALLOC
#define FIXED_FB_MEMADDR    0                   // !BSP_USE_LWMEM
#else
#define FIXED_FB_MEMADDR    1
#define FB_MEMORY_ADDRESS	0x80100000			/* low memory 1M-2M used for fb */
#endif

#define VA_TO_PHYS	        K0_TO_PHYS

/******************************************************************************
 * framebuffer device
 ******************************************************************************/

static LS1x_DC_dev_t  ls1x_DC;                  /* soft control of Display-Control */

LS1x_DC_dev_t *devDC = &ls1x_DC;                // global variable

//-----------------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------------

#if defined(OS_RTTHREAD)
#define LOCK(p)    do { rt_mutex_take(p->dc_mutex, RT_WAITING_FOREVER); } while (0);
#define UNLOCK(p)  do { rt_mutex_release(p->dc_mutex); } while (0);
#elif defined(OS_UCOS)
#define LOCK(p)    do { unsigned char err; if (OSRunning == OS_TRUE) OSMutexPend(p->dc_mutex, ~0, &err); } while (0);
#define UNLOCK(p)  do { if (OSRunning == OS_TRUE) OSMutexPost(p->dc_mutex); } while (0);
#elif defined(OS_FREERTOS)
#define LOCK(p)    do { xSemaphoreTake(p->dc_mutex, portMAX_DELAY); } while (0);
#define UNLOCK(p)  do { xSemaphoreGive(p->dc_mutex); } while (0);
#else // defined(OS_NONE)
#define LOCK(p)
#define UNLOCK(p)
#endif

/******************************************************************************
 * framebuffer display control routine
 ******************************************************************************/

typedef struct vga_struc
{
	unsigned int pclk, refresh;
	unsigned int hr, hss, hse, hfl;
	unsigned int vr, vss, vse, vfl;
	unsigned int pan_config;
	unsigned int hvsync_polarity;
} vga_struc_t;

static struct vga_struc vga_modes[] =
{
/****************************************************************************************
 *     pclk        hr   hss   hse   hfl    vr   vss   vse   vfl  pan_config
 *      |  refresh |     |     |     |     |     |     |     |       |   hvsync_polarity
 *      |     |    |     |     |     |     |     |     |     |       |           |
 *      V     V    V     V     V     V     V     V     V     V       V           V
 */
#if 0
	{  6429, 70,  240,  250,  260,  280,  320,  324,  326,  328, 0x00000301, 0x40000000}, /*"240x320_70.00"*/ /* ili9341 DE mode */
	{  7154, 60,  320,  332,  364,  432,  240,  248,  254,  276, 0x00000103, 0xc0000000}, /*"320x240_60.00"*/ /* HX8238-D */
	{ 12908, 60,  320,  360,  364,  432,  480,  488,  490,  498, 0x00000101, 0xc0000000}, /*"320x480_60.00"*/ /* NT35310 */
	{  9009, 60,  480,  488,  489,  531,  272,  276,  282,  288, 0x00000101, 0xc0000000}, /*"480x272_60.00"*/ /* LT043A-02AT */
	{ 20217, 60,  480,  488,  496,  520,  640,  642,  644,  648, 0x00000101, 0xc0000000}, /*"480x640_60.00"*/ /* jbt6k74 */
	{ 25200, 60,  640,  656,  666,  800,  480,  512,  514,  525, 0x00000301, 0xc0000000}, /*"640x480_60.00"*/ /* AT056TN53 */
	{ 33100, 60,  640,  672,  736,  832,  640,  641,  644,  663, 0x00000101, 0xc0000000}, /*"640x640_60.00"*/
	{ 39690, 60,  640,  672,  736,  832,  768,  769,  772,  795, 0x00000101, 0xc0000000}, /*"640x768_60.00"*/
	{ 42130, 60,  640,  680,  744,  848,  800,  801,  804,  828, 0x00000101, 0xc0000000}, /*"640x800_60.00"*/
#endif

	/* Test result: 10K~15K */
	{ 12500, 60,  480,  488,  490,  498,  800,  832,  912, 1024, 0x00000301, 0xc0000000}, /*"480x800_60.00"*/ /* WKS43178 */

	{ 33000, 60,  800,  840,  888,  928,  480,  509,  512,  525, 0x00000101, 0x00000000}, /*"800x480_60.00"*/ /* AT070TN83 V1 */
//  { 29232, 60,  800,    0,    0,  928,  480,    0,    0,  525, 0x00000101, 0x00000000}, /*"800x480_60.00"*/ /* AT070TN92 */

#if 0
    { 49500, 75,  800,  816,  896, 1056,  600,  601,  604,  625, 0x00000101, 0xc0000000}, /*"800x600_75.00"*/
	{ 40730, 60,  800,  832,  912, 1024,  640,  641,  644,  663, 0x00000101, 0xc0000000}, /*"800x640_60.00"*/
	{ 40010, 60,  832,  864,  952, 1072,  600,  601,  604,  622, 0x00000101, 0xc0000000}, /*"832x600_60.00"*/
	{ 40520, 60,  832,  864,  952, 1072,  608,  609,  612,  630, 0x00000101, 0xc0000000}, /*"832x608_60.00"*/
	{ 38170, 60, 1024, 1048, 1152, 1280,  480,  481,  484,  497, 0x00000101, 0xc0000000}, /*"1024x480_60.00"*/
	{ 51206, 60, 1024,    0,    0, 1344,  600,    0,    0,  635, 0x00000101, 0x00000000}, /*"1024x600_60.00"*/
	{ 52830, 60, 1024, 1072, 1176, 1328,  640,  641,  644,  663, 0x00000101, 0xc0000000}, /*"1024x640_60.00"*/
	{ 65000, 60, 1024, 1048, 1184, 1344,  768,  771,  777,  806, 0x00000101, 0xc0000000}, /*"1024x768_60.00"*/
	{ 71380, 60, 1152, 1208, 1328, 1504,  764,  765,  768,  791, 0x00000101, 0xc0000000}, /*"1152x764_60.00"*/
	{ 83460, 60, 1280, 1344, 1480, 1680,  800,  801,  804,  828, 0x00000101, 0xc0000000}, /*"1280x800_60.00"*/
	{135000, 75, 1280, 1296, 1440, 1688, 1024, 1025, 1028, 1066, 0x00000101, 0xc0000000}, /*"1280x1024_75.00"*/
	{ 85500, 60, 1360, 1424, 1536, 1792,  768,  771,  777,  795, 0x00000101, 0xc0000000}, /*"1360x768_60.00"*/
	{121750, 60, 1440, 1528, 1672, 1904, 1050, 1053, 1057, 1089, 0x00000101, 0xc0000000}, /*"1440x1050_60.00"*/
	{136750, 75, 1440, 1536, 1688, 1936,  900,  903,  909,  942, 0x00000101, 0xc0000000}, /*"1440x900_75.00"*/
	{148500, 60, 1920, 2008, 2052, 2200, 1080, 1084, 1089, 1125, 0x00000101, 0xc0000000}, /*"1920x1080_60.00"*/
#endif
};

static int vgamode_count = sizeof(vga_modes) / sizeof(struct vga_struc);

/*******************************************************************************
 * parse the string display mode
 */
extern char LCD_display_mode[];

static int LS1x_fb_parse_vgamode(char *vgamode,
                                 int  *xres,
                                 int  *yres,
                                 int  *refreshrate,
                                 int  *colordepth)
{
	int   i;
	char *p, *end, *pmode;

	if (NULL == vgamode)
		return -EINVAL;

	pmode = vgamode;

	/* find first digit charactor */
	for (i=0; i<20; i++)
		if (isdigit((int)*((pmode+i))))
			break;

	if (i >= 20)
		return -EINVAL;

	/* x-y resolution */
	*xres =	strtol(pmode+i, &end, 10);
	*yres = strtol(end+1, NULL, 10);

	if ((*xres<=0 || *xres>2000)||(*yres<=0 || *yres>2000))
		return -EINVAL;

	/* find the display mode is supported */
	for (i=0; i<vgamode_count; i++)
		if (vga_modes[i].hr == *xres && vga_modes[i].vr == *yres)
			break;

	if (i >= vgamode_count)
		return -ENOTSUP;

	/* refresh rate */
	p = strchr(pmode, '@');
	if (p != NULL)
	{
    	*refreshrate = strtol(p+1, NULL, 0);
    }
    
	/* color depth */
	p = strchr(pmode, '-');
	if (p != NULL)
	{
    	*colordepth = strtol(p+1, NULL, 0);
    }

    return 0;
}

/*******************************************************************************
 * Display Control wait enable done
 */
static int LS1x_DC_wait_enable(LS1x_DC_dev_t *dc)
{
	unsigned int val;
	int timeout = 204800;

	val = dc->hwDC->config;
	do
	{
		dc->hwDC->config = val | fb_cfg_output_en;
		ls1x_sync();
		val = dc->hwDC->config;
	} while (((val & fb_cfg_output_en) == 0) && (timeout-- > 0));

	if (timeout <= 0)
	{
		printk("Enable framebuffer timeout!\r\n");
		return -1;
	}

	return 0;
}

/*******************************************************************************
 * initialize framebuffer hardware
 */
#define DC_DELAY_US     200     // 100 /* LS1B 会出现不明原因 dead!!! */

static int LS1x_DC_hw_initialize(LS1x_DC_dev_t *dc)
{
	int i, mode = -1;

#if defined(LS1B)
	/*
	 * XXX: PAD is set to use UART0/UART1/UART5
	 */
	LS1B_MUX_CTRL0 &= ~(MUX_CTRL0_LCD_USE_UART0_DAT |
					    MUX_CTRL0_LCD_USE_UART15 | MUX_CTRL0_LCD_USE_UART0);
    ls1x_sync();
    
#elif defined(LS1C)

	LS1C_SHUT_CTRL &= ~SHUT_CTRL_LCD_SHUT;
	
	/*
	 * 在 QFP176 封装下, RGB 16 色 使用 PIN3~25 的默认输出.
	 *
	 * TODO 需要将复用寄存器和GPIO寄存器全部 set=0 (GPIO Number: 58~76)
	 *
	 */
    #if 0
    {
        int i;
        for (i=58; i<=76; i++)
            gpio_disable(i);
    }
    #endif
#endif

	/*
	 * framebuffer disable output
	 */
	dc->hwDC->config &= ~fb_cfg_output_en;
	ls1x_sync();
	delay_us(DC_DELAY_US);

	/* find the fit vgamode - whether supported
	 */
	for (i=0; i<vgamode_count; i++)
	{
		mode = i;
		unsigned int divider_int, div_reg;

		if ((vga_modes[i].hr != dc->fb_var.xres) ||
			(vga_modes[i].vr != dc->fb_var.yres))
			continue;
			
    #if defined(LS1B)
		divider_int  = LS1B_PLL_FREQUENCY(CPU_XTAL_FREQUENCY) / 1000 / 4;
    #elif defined(LS1C)
		divider_int  = LS1C_PLL_FREQUENCY(CPU_XTAL_FREQUENCY) / 1000;
    #endif

		divider_int /= vga_modes[mode].pclk;

	#if defined(LS1B)
		/* check whether divisor is too small. */
		if (divider_int < 1)
		{
			printk("Warning: clock source is too slow. Try bigger resolution\n");
			divider_int = 1;
		}
		else if (divider_int > 15)
		{
			printk("Warning: clock source is too fast. Try smaller resolution\n");
			divider_int = 15;
		}

		/* Set DC Clock */
		div_reg = LS1B_CLK_PLL_DIV;
		div_reg |= PLL_DC_BYPASS_EN | PLL_DC_BYPASS;
		div_reg &= ~(PLL_DC_RST_EN | PLL_DC_RST | PLL_DC_DIV_MASK);
		div_reg |= (divider_int << PLL_DC_DIV_SHIFT) & PLL_DC_DIV_MASK;
		LS1B_CLK_PLL_DIV = div_reg;
		div_reg &= ~PLL_DC_BYPASS;
		LS1B_CLK_PLL_DIV = div_reg;
		
    #elif defined(LS1C)
        div_reg = LS1C_CLK_PLL_DIV;
        LS1C_CLK_PLL_DIV &= ~CLK_DC_DIV_EN;     /* 注意：首先需要把分频使能位清零 */
        div_reg |=  CLK_DC_DIV_EN | CLK_DC_DIV_VALID | CLK_DC_SEL;
        div_reg &= ~CLK_DC_DIV_MASK;
		div_reg |= (divider_int << CLK_DC_DIV_SHIFT) & CLK_DC_DIV_MASK;
		LS1C_CLK_PLL_DIV = div_reg;
      #if 0
		div_reg = LS1C_DC_FREQUENCY(CPU_XTAL_FREQUENCY);
		printk("\r\nset dc frequency: %iMHZ\r\n", div_reg / 1000000);
      #endif
      
    #endif
    
		ls1x_sync();
		delay_us(DC_DELAY_US);    	/* if necessary? */

		break;
	}

	if (mode < 0)
	{
		printk("\n\n\nunsupported framebuffer resolution, choose from bellow:\n");
		for (i=0; i<vgamode_count; i++)
			printk("%dx%d, ", vga_modes[i].hr, vga_modes[i].vr);
		printk("\n");
		return -1;
	}

	/* Frame Buffer Memory Address.
	 */
	dc->hwDC->address0 = VA_TO_PHYS((unsigned int)dc->fb_fix.smem_start);
	dc->hwDC->address1 = VA_TO_PHYS((unsigned int)dc->fb_fix.smem_start);

	/* panel_config
	 */
	dc->hwDC->pan_config = 0x80001111 | vga_modes[mode].pan_config;

	dc->hwDC->hdisplay = (vga_modes[mode].hfl << fb_hdisplay_total_shift) | vga_modes[mode].hr;
	dc->hwDC->hsync    = (vga_modes[mode].hse << fb_hsync_end_shift) |
						  vga_modes[mode].hvsync_polarity | vga_modes[mode].hss;

	dc->hwDC->vdisplay = (vga_modes[mode].vfl << fb_vdisplay_total_shift) | vga_modes[mode].vr;
	dc->hwDC->vsync    = (vga_modes[mode].vse << fb_vsync_end_shift) |
						  vga_modes[mode].hvsync_polarity | vga_modes[mode].vss;

	/* set configure register 16bpp
	 */
	dc->hwDC->config = fb_cfg_reset | fb_cfg_color_r5g6b5; // fb_cfg_output_en |
	ls1x_sync();
	delay_us(DC_DELAY_US);
	
	dc->hwDC->stride = (dc->fb_var.xres * 2 + FB_BURST_SIZE) & ~FB_BURST_SIZE;
	dc->hwDC->origin = 0;
    ls1x_sync();
	delay_us(100);

	/* wait for enable done.
	 */
//	if (LS1x_DC_wait_enable(dc) < 0)
//		return -1;

	/* flag hardware has initialized
	 */
	dc->initialized = 1;

	return 0;
}

/******************************************************************************
 * start framebuffer
 */
static int LS1x_DC_start(LS1x_DC_dev_t *dc)
{
	/*
	 * wait for framebuffer enable done
	 */
	if (LS1x_DC_wait_enable(dc) < 0)
		return -1;

#if defined(LS1C)
    //
#endif

	dc->started = 1;

	return 0;
}

/******************************************************************************
 * stop famebuffer
 */
static void LS1x_DC_stop(LS1x_DC_dev_t *dc)
{
	dc->hwDC->config &= ~fb_cfg_output_en;
	delay_us(100);

#if defined(LS1C)
	LS1C_SHUT_CTRL |= SHUT_CTRL_LCD_SHUT;
#endif

	dc->started = 0;
}

/******************************************************************************
 * clear the memory buffer
 */
#if (FB_BUF_K0_ADDR)
extern void flush_cache(void);
#endif

static void LS1x_clear_fb_buffer(LS1x_DC_dev_t *dc, unsigned int color)
{
	unsigned int *addr, size;
	int i;

	/*
	 * 清除内存 - XXX: 四字节对齐!
	 */
	addr = (unsigned int *)dc->fb_fix.smem_start;
	size = (dc->fb_var.xres*dc->fb_var.yres*((dc->fb_var.bits_per_pixel+7)/8)+3)/4;
	for (i=0; i<size; i++)
		*addr++ = color;

#if (FB_BUF_K0_ADDR)
	flush_cache();
#endif
}

/******************************************************************************
 * framebuffer driver routine
 ******************************************************************************/

static unsigned short red16[] =
{
	0x0000, 0x0000, 0x0000, 0x0000, 0xaaaa, 0xaaaa, 0xaaaa, 0xaaaa,
	0x5555, 0x5555, 0x5555, 0x5555, 0xffff, 0xffff, 0xffff, 0xffff
};

static unsigned short green16[] =
{
	0x0000, 0x0000, 0xaaaa, 0xaaaa, 0x0000, 0x0000, 0x5555, 0xaaaa,
	0x5555, 0x5555, 0xffff, 0xffff, 0x5555, 0x5555, 0xffff, 0xffff
};

static unsigned short blue16[] =
{
	0x0000, 0xaaaa, 0x0000, 0xaaaa, 0x0000, 0xaaaa, 0x0000, 0xaaaa,
	0x5555, 0xffff, 0x5555, 0xffff, 0x5555, 0xffff, 0x5555, 0xffff
};

static int get_fix_screen_info(LS1x_DC_dev_t *dc, struct fb_fix_screeninfo *info)
{
	if (NULL == info)
		return -1;

	info->smem_start  = dc->fb_fix.smem_start;
	info->smem_len    = dc->fb_fix.smem_len;
	info->type        = dc->fb_fix.type;
	info->visual      = dc->fb_fix.visual;
	info->line_length = dc->fb_fix.line_length;

	return 0;
}

static int get_var_screen_info(LS1x_DC_dev_t *dc, struct fb_var_screeninfo *info)
{
	if (NULL == info)
		return -1;

	info->xres             = dc->fb_var.xres;
	info->yres             = dc->fb_var.yres;
	info->bits_per_pixel   = dc->fb_var.bits_per_pixel;

	info->red.offset       = dc->fb_var.red.offset;
	info->red.length       = dc->fb_var.red.length;
	info->red.msb_right    = dc->fb_var.red.msb_right;

	info->green.offset     = dc->fb_var.green.offset;
	info->green.length     = dc->fb_var.green.length;
	info->green.msb_right  = dc->fb_var.green.msb_right;

	info->blue.offset      = dc->fb_var.blue.offset;
	info->blue.length      = dc->fb_var.blue.length;
	info->blue.msb_right   = dc->fb_var.blue.msb_right;

	info->transp.offset    = dc->fb_var.transp.offset;
	info->transp.length    = dc->fb_var.transp.length;
	info->transp.msb_right = dc->fb_var.transp.msb_right;

	return 0;
}

/******************************************************************************
 * framebuffer driver implement
 ******************************************************************************/

STATIC_DRV int LS1x_DC_initialize(void *dev, void *arg)
{
	LS1x_DC_dev_t *pDC = (LS1x_DC_dev_t *)dev;
	int xres, yres, refreshrate=60, colordepth=16;
	unsigned int mem_len;
	
#if (!FIXED_FB_MEMADDR)
	char *fb_buffer = NULL;
#endif

    if (dev == NULL)
        return -1;
        
    if (pDC->initialized)
        return 0;

    memset(pDC, 0, sizeof(LS1x_DC_dev_t));    // clear all

#if BSP_USE_OS
  #if defined(OS_RTTHREAD)
    pDC->dc_mutex = rt_mutex_create(LS1x_FB_DEVICE_NAME, RT_IPC_FLAG_FIFO);
  #elif defined(OS_UCOS)
    unsigned char err;
    pDC->dc_mutex = OSMutexCreate(FB_MUTEX_PRIO, &err);
  #elif defined(OS_FREERTOS)
    pDC->dc_mutex = xSemaphoreCreateMutex();
  #endif

    if (pDC->dc_mutex == NULL)
    {
        printk("DC create mutex fail!\n");
    	return -1;
    }
#endif // #if BSP_USE_OS

    /*
     * XXX use DC0 as framebuffer controller
     */
    pDC->hwDC = (LS1x_DC_regs_t *)LS1x_DC0_BASE;

	pDC->fb_fix.type   = FB_TYPE_PACKED_PIXELS;
	pDC->fb_fix.visual = FB_VISUAL_TRUECOLOR;

	if (LS1x_fb_parse_vgamode(LCD_display_mode, &xres, &yres, &refreshrate, &colordepth) < 0)
	{
		printk("DC vga mode %s is not supported!\n", LCD_display_mode);
		return -1;
	}

	mem_len = xres * yres * colordepth / 8;
	pDC->fb_var.xres = xres;
	pDC->fb_var.yres = yres;
	pDC->fb_var.bits_per_pixel = colordepth;

	pDC->fb_fix.line_length = xres * colordepth / 8;
	pDC->fb_fix.smem_len = mem_len;

#if (FIXED_FB_MEMADDR)
	/*
	 * 使用指定内存地址
	 */
	pDC->fb_fix.smem_start = (char *)FB_MEMORY_ADDRESS;

#else
	/*
	 * FIXME alloc framebuffer memory dynamic.
	 */
	fb_buffer = (char *)MALLOC(pDC->fb_fix.smem_len + 0x400);
	if (fb_buffer == NULL)
	{
	    printk("DC alloc memory fail!\n");
		return -1;
	}

	/*
	 * 256 byte aligned
	 */
  #if (FB_BUF_K0_ADDR)
	pDC->fb_fix.smem_start = (char *)((unsigned int)(fb_buffer + 0x400) & ~0x3FF);
  #else
    /* use K1 address */
	pDC->fb_fix.smem_start = (char *)K0_TO_K1((unsigned int)(fb_buffer + 0x400) & ~0x3FF);
  #endif

#endif

	/*
	 * clear the memory buffer
	 */
	memset((void *)pDC->fb_fix.smem_start, 0, pDC->fb_fix.smem_len);

	/*
	 * TODO only 16 color mode of R5G6B5
	 */
	pDC->fb_var.red.length   = 5;
	pDC->fb_var.red.offset   = 11;
	pDC->fb_var.green.length = 5;
	pDC->fb_var.green.offset = 6;
	pDC->fb_var.blue.length  = 5;
	pDC->fb_var.blue.offset  = 0;

	if (LS1x_DC_hw_initialize(pDC) < 0)
	{
		printk("DC initialize fail!\n");
		return -1;
	}

	pDC->initialized = 1;

    DBG_OUT("DC controller initialized.\r\n");
    
	return 0;
}

STATIC_DRV int LS1x_DC_open(void *dev, void *arg)
{
	LS1x_DC_dev_t *pDC = (LS1x_DC_dev_t *)dev;
	
    if (dev == NULL)
        return -1;
        
	if (!pDC->initialized)
        if (LS1x_DC_initialize(dev, arg) < 0)
            return -1;

	if (!pDC->started)
    	if (LS1x_DC_start(pDC) < 0)
			return -1;

	return 0;
}

STATIC_DRV int LS1x_DC_close(void *dev, void *arg)
{
	LS1x_DC_dev_t *pDC = (LS1x_DC_dev_t *)dev;
	
    if (dev == NULL)
        return -1;

	LS1x_DC_stop(pDC);

	return 0;
}

/*
 * arg is/as offset
 */
STATIC_DRV int LS1x_DC_read(void *dev, void *buf, int size, void *arg)
{
    int rdBytes;
	LS1x_DC_dev_t *pDC = (LS1x_DC_dev_t *)dev;
	unsigned int offset = (unsigned int)arg;

    if ((dev == NULL) || (buf == NULL))
        return -1;
        
    LOCK(pDC);
    
	rdBytes = ((offset + size) > pDC->fb_fix.smem_len) ? (pDC->fb_fix.smem_len - offset) : size;

	memcpy(buf, (const void *)(pDC->fb_fix.smem_start + offset), rdBytes);

    UNLOCK(pDC);
    
	return rdBytes;
}

/*
 * arg is/as offset
 */
STATIC_DRV int LS1x_DC_write(void *dev, void *buf, int size, void *arg)
{
    int wrBytes;
	LS1x_DC_dev_t *pDC = (LS1x_DC_dev_t *)dev;
	unsigned int offset = (unsigned int)arg;
	
    if ((dev == NULL) || (buf == NULL))
        return -1;

    LOCK(pDC);
    
	wrBytes = ((offset + size) > pDC->fb_fix.smem_len) ? (pDC->fb_fix.smem_len - offset) : size;

	memcpy((void *)(pDC->fb_fix.smem_start + offset), buf, wrBytes);

#if (FB_BUF_K0_ADDR)
	clean_dcache((unsigned)(pDC->fb_fix.smem_start + offset), wrBytes);
#endif

    UNLOCK(pDC);
    
	return wrBytes;
}

static int get_palette(struct fb_cmap *cmap)
{
	unsigned int i;

	if (cmap->start + cmap->len >= 16)
		return 1;

	for (i = 0; i < cmap->len; i++)
	{
		cmap->red[cmap->start + i]   = red16[cmap->start + i];
		cmap->green[cmap->start + i] = green16[cmap->start + i];
		cmap->blue[cmap->start + i]  = blue16[cmap->start + i];
	}

	return 0;
}

static int set_palette(struct fb_cmap *cmap)
{
	unsigned int i;

	if (cmap->start + cmap->len >= 16)
		return 1;

	for (i = 0; i < cmap->len; i++)
	{
		red16[cmap->start + i]   = cmap->red[cmap->start + i];
		green16[cmap->start + i] = cmap->green[cmap->start + i];
		blue16[cmap->start + i]  = cmap->blue[cmap->start + i];
	}

	return 0;
}

STATIC_DRV int LS1x_DC_ioctl(void *dev, int cmd, void *arg)
{
    int rt = 0;
	LS1x_DC_dev_t *pDC = (LS1x_DC_dev_t *)dev;
	
    if (dev == NULL)
        return -1;

	switch (cmd)
    {
		case FBIOGET_FSCREENINFO:
		    rt = get_fix_screen_info(pDC, (struct fb_fix_screeninfo *)arg);
			break;

		case FBIOGET_VSCREENINFO:
			rt = get_var_screen_info(pDC, (struct fb_var_screeninfo *)arg);
			break;

		case FBIOPUT_VSCREENINFO:    /* not implemented yet */
			rt = -1;
		    break;

		case FBIOGETCMAP:
			rt = get_palette((struct fb_cmap *)arg);
			break;

		case FBIOPUTCMAP:
			rt = set_palette((struct fb_cmap *)arg);
			break;

		case IOCTRL_FB_CLEAR_BUFFER:
		{
			unsigned int clr = (unsigned int)arg;
			clr = (clr << 16) | clr;
			LS1x_clear_fb_buffer(pDC, clr);
			break;
		}

		case IOCTRL_LCD_POWERON:
			break;

		case IOCTRL_LCD_POWEROFF:
			break;

		default:
			break;
    }

	return rt;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * DC driver operators
 */
static driver_ops_t LS1x_DC_drv_ops =
{
    .init_entry  = LS1x_DC_initialize,
    .open_entry  = LS1x_DC_open,
    .close_entry = LS1x_DC_close,
    .read_entry  = LS1x_DC_read,
    .write_entry = LS1x_DC_write,
    .ioctl_entry = LS1x_DC_ioctl,
};
driver_ops_t *ls1x_dc_drv_ops = &LS1x_DC_drv_ops;
#endif

//-----------------------------------------------------------------------------
// 是否初始化
//-----------------------------------------------------------------------------

int ls1x_dc_initialized(void)
{
	return devDC->initialized;
}

//-----------------------------------------------------------------------------
// 是否启动
//-----------------------------------------------------------------------------

int ls1x_dc_started(void)
{
	return devDC->started;
}

#endif // #ifdef BSP_USE_FB


