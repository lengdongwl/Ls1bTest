/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_pwm_hw.h
 *
 * Created on: 2013-3-12
 *     Author: Bian
 */

#ifndef _LS1X_PWM_HW_H
#define _LS1X_PWM_HW_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * PWM hardware Registers.
 */
typedef struct LS1x_PWM_regs
{
	volatile unsigned int counter;      /* 0x00 �������� */
	volatile unsigned int hrc;          /* 0x04 �����嶨ʱ�ο��Ĵ��� */
	volatile unsigned int lrc;          /* 0x08 �����嶨ʱ�ο��Ĵ��� */
	volatile unsigned int ctrl;         /* 0x0C ���ƼĴ��� */
} LS1x_PWM_regs_t;

/******************************************************************************
 * 1. ��������
 * 2. �����嶨ʱ�ο��Ĵ���
 * 3. �����嶨ʱ�ο��Ĵ���
 */
#if defined(LS1B)
#define PWM_REG_VALUE_MAX       0x007FFFFF      // XXX ʵ��ֵ 0x003FF100
#elif defined(LS1C)
#define PWM_REG_VALUE_MAX       0x007FFFFF      // TODO
#endif

/******************************************************************************
 * ���ƼĴ���
 */
enum
{
#if defined(LS1C)
    //-----------------------------------------------------
    // TODO �� LS1B �������ֲ���, δ��������˵��.
    //      LS1B/LS1C �Ĵ������ =8 �����Ǵ����.
    //-----------------------------------------------------
    pwm_ctrl_lrc_ien = (1<<11),     /* =1: ��INTENΪ1ʱ��CNTR������LRC������ж� */
    pwm_ctrl_hrc_ien = (1<<10),     /* =1: ��INTENΪ1ʱ��CNTR������HRC������ж� */
#endif

	pwm_ctrl_rest    = (1<<7),      /* =1: counter����������, =0: �������������� */
	pwm_ctrl_iflag   = (1<<6),      /* =1: ���жϲ���. д�� 1: ���ж� */
	pwm_ctrl_ien     = (1<<5),      /* =1: ��counter������lrc��hrc������ж� */
	pwm_ctrl_single  = (1<<4),      /* =1: ���������һ��, =0: ����������� */
	pwm_ctrl_oe_mask = (1<<3),      /* =1: �����������, =0: �������ʹ�� */
	pwm_ctrl_cntr_en = (1<<0),      /* =1: counterʹ�ܼ���, =0: counterֹͣ���� */
};

#ifdef __cplusplus
}
#endif

#endif // _LS1X_PWM_HW_H


