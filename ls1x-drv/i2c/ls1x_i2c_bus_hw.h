/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * LS1x_I2C_hw.h
 *
 *  Created on: 2013-11-1
 *      Author: Bian
 */

#ifndef LS1x_I2C_HW_H_
#define LS1x_I2C_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef bit
#define bit(x)  (1<<x)
#endif

/*
 * I2C Registers.
 */
typedef struct LS1x_I2C_regs
{
	volatile unsigned char prerlo;			// 0x00		��Ƶ���������ֽڼĴ���  Prcescale =
	volatile unsigned char prerhi;			// 0x01		��Ƶ���������ֽڼĴ���  DDR_clk/(10*clock_s)-1
	volatile unsigned char ctrl;			// 0x02		���ƼĴ���
	union
	{
		volatile unsigned char txreg;		// 0x03		�������ݼĴ���
		volatile unsigned char rxreg;		// 0x03		�������ݼĴ���
	} data;

	union
	{
		volatile unsigned char cmd;			// 0x04		������ƼĴ��� - дʱ
		volatile unsigned char sr;			// 0x04		״̬�Ĵ��� - ��ʱ
	} cmd_sr;
} LS1x_I2C_regs_t;

typedef enum
{
	/* ���ƼĴ���
	 */
	i2c_ctrl_en		 = bit(7),			// CTR(7)	=1: ��������ģʽ; =0: �Է�Ƶ�Ĵ������в���.
	i2c_ctrl_ien	 = bit(6),			// CTR(6)	=1: ���ж�.

	/* �������ݼĴ���
	 */
	i2c_txreg_rwstat = bit(0),			// TXR(0)	����ַ����ʱ, ��λָʾ��д״̬.

	/* ������ƼĴ���
	 */
	i2c_cmd_start	 = bit(7),			// CMD(7)	����START�ź�
	i2c_cmd_stop	 = bit(6),			// CMD(6)	����STOP�ź�
	i2c_cmd_read	 = bit(5),			// CMD(5)	�������ź�
	i2c_cmd_write	 = bit(4),			// CMD(4)	����д�ź�
	i2c_cmd_nack	 = bit(3),			// CMD(3)	����Ӧ���ź�, =1: ������Ӧ���ź�
	i2c_cmd_ack		 = 0,
	i2c_cmd_iack	 = bit(0),			// CMD(0)	�����ж�Ӧ���ź�, =1: ����

	/* ״̬�Ĵ���
	 */
	i2c_sr_rxnack	 = bit(7),			// SR(7)	�յ�Ӧ��λ, =1:û�յ�Ӧ��λ; =0:�յ�Ӧ��λ.
	i2c_sr_busy	 	 = bit(6),			// SR(6)	I2C����æ��־λ, =1:������æ; =0:���߿���.
	i2c_sr_arblost	 = bit(5),			// SR(5)	��I2C��ʧȥI2C���߿���Ȩʱ����λ��1.
	i2c_sr_xferflag	 = bit(1),			// SR(1)	ָʾ����Ĺ���, =1:��ʾ���ڴ�������; =0:��ʾ���ݴ������.
	i2c_sr_iflag	 = bit(0),			// SR(0)	�жϱ�־λ.һ�����ݴ�����,������һ�������������ݴ���,��λ��1.
} IIC_controller_t;

#ifdef __cplusplus
}
#endif

#endif /* LS1x_I2C_HW_H_ */

