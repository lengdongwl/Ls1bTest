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
	volatile unsigned int toytrim;          /* 0x20 32 RW �� 32.768kHZ �ķ�Ƶϵ��(������ʱ��) */
#else
    volatile unsigned int rsv1[9];
#endif

/*
 * Only These Registers LS1C Have?
 */
    volatile unsigned int toywritelo;       /* 0x24 32 W  TOY ��32λ��ֵд�� */
	volatile unsigned int toywritehi;       /* 0x28 32 W  TOY ��32λ��ֵд�� */
	volatile unsigned int toyreadlo;        /* 0x2C 32 R  TOY ��32λ��ֵ���� */
	volatile unsigned int toyreadhi;        /* 0x30 32 R  TOY ��32λ��ֵ���� */

#if defined(LS1B)
    volatile unsigned int toymatch[3];      /* 0x34~0x3C 32 RW TOY ��ʱ�ж�0 */

	volatile unsigned int rtcctrl;          /* 0x40 32 RW TOY �� RTC ���ƼĴ��� */

	volatile unsigned int rsv2[7];
	volatile unsigned int rtctrim;          /* 0x60 32 RW �� 32.768kHZ �ķ�Ƶϵ��(��ʱ��) */
	volatile unsigned int rtcwrite;         /* 0x64 32 R  RTC ��ʱ����д�� */
	volatile unsigned int rtcread;          /* 0x68 32 W  RTC ��ʱ�������� */

    volatile unsigned int rtcmatch[3];      /* 0x6C~0x74 32 RW RTC ʱ�Ӷ�ʱ�ж�0 */
#endif

} LS1x_rtc_regs_t;

#if defined(LS1B)
/*
 * RTC control bits
 */
enum
{
	rtc_ctrl_e0			= (1<<8),		/* RW 0: 32.768k�����ֹ;   1: 32.768k����ʹ�� */
	rtc_ctrl_32s		= (1<<5),		/* R  0��32.768k���񲻹���; 1��32.768k������������. */
    rtc_ctrl_ers		= (1<<23),		/* R  RTCʹ��(bit13)д״̬ */
    rtc_ctrl_rts		= (1<<20),		/* R  rtctrim д״̬ */
    rtc_ctrl_rm2		= (1<<19),		/* R  rtcmatch2 д״̬ */
    rtc_ctrl_rm1		= (1<<18),		/* R  rtcmatch1 д״̬ */
    rtc_ctrl_rm0		= (1<<17),		/* R  rtcmatch0 д״̬ */
    rtc_ctrl_rs			= (1<<16),		/* R  rtcwrite д״̬ */
	rtc_ctrl_bp			= (1<<14),		/* RW ��·32.768k����  0:ѡ��������; 1: GPIO8������������. */
	rtc_ctrl_ren		= (1<<13),		/* RW 0��RTC��ֹ; 1��RTCʹ��*/
	rtc_ctrl_brt		= (1<<12),		/* RW ��·RTC��Ƶ. 0:��������; 1:RTCֱ�ӱ�32.768k�������� */

	rtc_ctrl_ten		= (1<<11),		/* RW 0��TOY ��ֹ; 1��TOYʹ�� */
	rtc_ctrl_btt		= (1<<10),		/* RW ��·TOY��Ƶ. 0:��������; 1:TOYֱ�ӱ�32.768k�������� */
    rtc_ctrl_ets		= (1<<7),		/* R  TOYʹ��(bit11)д״̬ */
    rtc_ctrl_tts		= (1<<4),		/* R  toytrim д״̬ */
    rtc_ctrl_tm2		= (1<<3),		/* R  toymatch2 д״̬ */
    rtc_ctrl_tm1		= (1<<2),		/* R  toymatch1 д״̬ */
    rtc_ctrl_tm0		= (1<<1),		/* R  toymatch0 д״̬ */
    rtc_ctrl_ts			= (1<<0),		/* R  toywrite д״̬ */

} LS1x_rtc_reg_bits;

#endif

#ifdef __cplusplus
}
#endif

#endif /* LS1X_RTC_HW_H_ */

