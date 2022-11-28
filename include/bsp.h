/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _BSP_H
#define _BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

//-----------------------------------------------------------------------------

#define BSP_USE_OS          (!defined(OS_NONE))     // Is using RTOS?

#define BSP_USE_LWMEM       (!defined(OS_RTTHREAD)) // Need memory manager?

//-----------------------------------------------------------------------------
// cpu 外部晶振频率: HZ
//-----------------------------------------------------------------------------

#define CPU_XTAL_FREQUENCY  24000000

//-----------------------------------------------------------------------------
// 打印函数
//-----------------------------------------------------------------------------

#ifdef OS_RTTHREAD
extern void rt_kprintf(const char *fmt, ...);
#define printk      	rt_kprintf
#else
extern int printk(const char *fmt, ...);
#endif

#if 1
#define DBG_OUT(...)    printk(__VA_ARGS__)
#else
#define DBG_OUT(...)
#endif

//-----------------------------------------------------------------------------
// 延时函数
//-----------------------------------------------------------------------------

/*
 * in "xxx/port/clock.c"
 */
extern void delay_us(unsigned int us);
extern void delay_ms(unsigned int ms);

extern int ls1x_drv_init(void);
#ifdef OS_RTTHREAD
extern int rt_ls1x_drv_init(void);
#endif
extern int install_3th_libraries(void);

//-----------------------------------------------------------------------------
// 片上设备使用 
//-----------------------------------------------------------------------------

//#define BSP_USE_SPI0
#define BSP_USE_SPI1

#define BSP_USE_I2C0
//#define BSP_USE_I2C1
//#define BSP_USE_I2C2

//#define BSP_USE_UART2
#define BSP_USE_UART3
#define BSP_USE_UART4
#define BSP_USE_UART5           // Console_Port
//#define BSP_USE_UART0
//#define BSP_USE_UART01
//#define BSP_USE_UART02
//#define BSP_USE_UART03
//#define BSP_USE_UART1
//#define BSP_USE_UART11
//#define BSP_USE_UART12
//#define BSP_USE_UART13

//#define BSP_USE_CAN0
//#define BSP_USE_CAN1

//#define BSP_USE_NAND

#define BSP_USE_FB              // framebuffer

//#define BSP_USE_GMAC0
#ifdef LS1B
//#define BSP_USE_GMAC1
#endif

//#define BSP_USE_PWM0
//#define BSP_USE_PWM1
#define BSP_USE_PWM2
//#define BSP_USE_PWM3

#define BSP_USE_RTC

//#define BSP_USE_AC97

//#define BSP_USE_WATCHDOG

//#define BSP_USE_USB
//-----------------------------------------------------------------------------
// 外部设备 
//-----------------------------------------------------------------------------

/*
 * I2C 设备
 */
#ifdef BSP_USE_I2C0
#ifdef BSP_USE_FB
#define GP7101_DRV              // LCD 亮度控制
#define GT1151_DRV              // 竖屏触摸芯片
#endif
#define PCA9557_DRV             // GPIO 芯片
#define ADS1015_DRV             // 4路 12bit ADC
//#define MCP4725_DRV             // 1路 12bit DAC
#define RX8010_DRV              // RTC 芯片
#endif

/*
 * SPI 设备
 */
#ifdef BSP_USE_SPI0
//#define W25X40_DRV
#ifdef BSP_USE_FB
//#define XPT2046_DRV             // 触摸屏芯片
#endif
#endif

//-----------------------------------------------------------------------------
// 第三方软件包
//-----------------------------------------------------------------------------

#ifdef BSP_USE_NAND
#define USE_YAFFS2              // yaffs 文件系统
#endif

#if defined(BSP_USE_GMAC0) || defined(BSP_USE_GMAC1)
//#define USE_LWIP                // lwIP 1.4.1
#if defined(USE_LWIP) && defined(USE_YAFFS2)
//#define USE_FTPD                // ftp 服务器
#endif
#endif

#ifdef BSP_USE_FB
//#define USE_LVGL                // lvgl 7.0.1
#endif

//#define USE_MODBUS              // modbus 协议包

//-----------------------------------------------------------------------------
// 常用函数
//-----------------------------------------------------------------------------

/* in "irq.c"
 */
extern void ls1x_install_irq_handler(int vector, void (*isr)(int, void *), void *arg);
extern void ls1x_remove_irq_handler(int vector);

/* in "aligned_mallo.c" or "lwmem.c"
 */
extern void *aligned_malloc(size_t size, unsigned int align);
extern void aligned_free(void *addr);

/*
 * 获取系统启动以来的 Tick 总数
 */
extern unsigned int get_clock_ticks(void);

#ifdef __cplusplus
}
#endif

#endif // _BSP_H
