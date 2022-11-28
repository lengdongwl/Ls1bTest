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
	volatile unsigned char prerlo;			// 0x00		分频锁存器低字节寄存器  Prcescale =
	volatile unsigned char prerhi;			// 0x01		分频锁存器高字节寄存器  DDR_clk/(10*clock_s)-1
	volatile unsigned char ctrl;			// 0x02		控制寄存器
	union
	{
		volatile unsigned char txreg;		// 0x03		发送数据寄存器
		volatile unsigned char rxreg;		// 0x03		接收数据寄存器
	} data;

	union
	{
		volatile unsigned char cmd;			// 0x04		命令控制寄存器 - 写时
		volatile unsigned char sr;			// 0x04		状态寄存器 - 读时
	} cmd_sr;
} LS1x_I2C_regs_t;

typedef enum
{
	/* 控制寄存器
	 */
	i2c_ctrl_en		 = bit(7),			// CTR(7)	=1: 正常工作模式; =0: 对分频寄存器进行操作.
	i2c_ctrl_ien	 = bit(6),			// CTR(6)	=1: 打开中断.

	/* 发送数据寄存器
	 */
	i2c_txreg_rwstat = bit(0),			// TXR(0)	当地址传送时, 该位指示读写状态.

	/* 命令控制寄存器
	 */
	i2c_cmd_start	 = bit(7),			// CMD(7)	产生START信号
	i2c_cmd_stop	 = bit(6),			// CMD(6)	产生STOP信号
	i2c_cmd_read	 = bit(5),			// CMD(5)	产生读信号
	i2c_cmd_write	 = bit(4),			// CMD(4)	产生写信号
	i2c_cmd_nack	 = bit(3),			// CMD(3)	产生应答信号, =1: 不产生应答信号
	i2c_cmd_ack		 = 0,
	i2c_cmd_iack	 = bit(0),			// CMD(0)	产生中断应答信号, =1: 产生

	/* 状态寄存器
	 */
	i2c_sr_rxnack	 = bit(7),			// SR(7)	收到应答位, =1:没收到应答位; =0:收到应答位.
	i2c_sr_busy	 	 = bit(6),			// SR(6)	I2C总线忙标志位, =1:总线在忙; =0:总线空闲.
	i2c_sr_arblost	 = bit(5),			// SR(5)	当I2C核失去I2C总线控制权时，该位置1.
	i2c_sr_xferflag	 = bit(1),			// SR(1)	指示传输的过程, =1:表示正在传输数据; =0:表示数据传输完毕.
	i2c_sr_iflag	 = bit(0),			// SR(0)	中断标志位.一个数据传输完,或另外一个器件发起数据传输,该位置1.
} IIC_controller_t;

#ifdef __cplusplus
}
#endif

#endif /* LS1x_I2C_HW_H_ */

