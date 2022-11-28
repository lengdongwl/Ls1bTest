/*
 * ls1x_rtc_hw.h
 *
 *  Created on: 2014-3-12
 *      Author: Bian
 */

#ifndef LS1X_RTC_HW_H_
#define LS1X_RTC_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * RTC Registers.
 */
typedef struct LS1x_rtc_regs
{
#if defined(LS1B)
    volatile unsigned int rsv1[8];
	volatile unsigned int toytrim;          /* 0x20 32 RW 对 32.768kHZ 的分频系数(计数器时钟) */
#else
    volatile unsigned int rsv1[9];
#endif

/*
 * Only These Registers LS1C Have?
 */
    volatile unsigned int toywritelo;       /* 0x24 32 W  TOY 低32位数值写入 */
	volatile unsigned int toywritehi;       /* 0x28 32 W  TOY 高32位数值写入 */
	volatile unsigned int toyreadlo;        /* 0x2C 32 R  TOY 低32位数值读出 */
	volatile unsigned int toyreadhi;        /* 0x30 32 R  TOY 高32位数值读出 */

#if defined(LS1B)
    volatile unsigned int toymatch[3];      /* 0x34~0x3C 32 RW TOY 定时中断0 */

	volatile unsigned int rtcctrl;          /* 0x40 32 RW TOY 和 RTC 控制寄存器 */

	volatile unsigned int rsv2[7];
	volatile unsigned int rtctrim;          /* 0x60 32 RW 对 32.768kHZ 的分频系数(定时器) */
	volatile unsigned int rtcwrite;         /* 0x64 32 R  RTC 定时计数写入 */
	volatile unsigned int rtcread;          /* 0x68 32 W  RTC 定时计数读出 */

    volatile unsigned int rtcmatch[3];      /* 0x6C~0x74 32 RW RTC 时钟定时中断0 */
#endif

} LS1x_rtc_regs_t;

#if defined(LS1B)
/*
 * RTC control bits
 */
enum
{
	rtc_ctrl_e0			= (1<<8),		/* RW 0: 32.768k晶振禁止;   1: 32.768k晶振使能 */
	rtc_ctrl_32s		= (1<<5),		/* R  0：32.768k晶振不工作; 1：32.768k晶振正常工作. */
    rtc_ctrl_ers		= (1<<23),		/* R  RTC使能(bit13)写状态 */
    rtc_ctrl_rts		= (1<<20),		/* R  rtctrim 写状态 */
    rtc_ctrl_rm2		= (1<<19),		/* R  rtcmatch2 写状态 */
    rtc_ctrl_rm1		= (1<<18),		/* R  rtcmatch1 写状态 */
    rtc_ctrl_rm0		= (1<<17),		/* R  rtcmatch0 写状态 */
    rtc_ctrl_rs			= (1<<16),		/* R  rtcwrite 写状态 */
	rtc_ctrl_bp			= (1<<14),		/* RW 旁路32.768k晶振  0:选择晶振输入; 1: GPIO8来驱动计数器. */
	rtc_ctrl_ren		= (1<<13),		/* RW 0：RTC禁止; 1：RTC使能*/
	rtc_ctrl_brt		= (1<<12),		/* RW 旁路RTC分频. 0:正常操作; 1:RTC直接被32.768k晶振驱动 */

	rtc_ctrl_ten		= (1<<11),		/* RW 0：TOY 禁止; 1：TOY使能 */
	rtc_ctrl_btt		= (1<<10),		/* RW 旁路TOY分频. 0:正常操作; 1:TOY直接被32.768k晶振驱动 */
    rtc_ctrl_ets		= (1<<7),		/* R  TOY使能(bit11)写状态 */
    rtc_ctrl_tts		= (1<<4),		/* R  toytrim 写状态 */
    rtc_ctrl_tm2		= (1<<3),		/* R  toymatch2 写状态 */
    rtc_ctrl_tm1		= (1<<2),		/* R  toymatch1 写状态 */
    rtc_ctrl_tm0		= (1<<1),		/* R  toymatch0 写状态 */
    rtc_ctrl_ts			= (1<<0),		/* R  toywrite 写状态 */

} LS1x_rtc_reg_bits;

#endif

#ifdef __cplusplus
}
#endif

#endif /* LS1X_RTC_HW_H_ */

