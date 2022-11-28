/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ads1015.h
 *
 *  Created on: 2015-11-11
 *      Author: Bian's
 */
/*
 * The ADS1013/4/5 respond to the I2C general call address (0000000)
 * if the eighth bit is '0'. The devices acknowledge the general call
 * address and respond to commands in the second byte. If the second
 * byte is 00000110 (06h), the ADS1013/4/5 reset the internal registers
 * and enter power-down mode.
 */

#ifndef ADS1015_H_
#define ADS1015_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>

/*
 * POINTER REGISTER
 * 1 byte, bit 1:0 indicate to access which register.
 */
#define ADS1015_REG_POINTER_MASK        0x03
#define ADS1015_REG_POINTER_CONVERT     0x00
#define ADS1015_REG_POINTER_CONFIG      0x01
#define ADS1015_REG_POINTER_LOWTHRESH   0x02
#define ADS1015_REG_POINTER_HITHRESH    0x03
#define ADS1015_REGISTER_COUNT    		4

/*
 * CONVERT REGISTER
 * 2 bytes, bit 15:4 is the convert data D11:0; bit 3:0 not used.
 * so read out data must shift right 4 bits.
 */

/*
 * CONFIG REGISTER
 * 2 bytes, bit defination are as below.
 */
/* Bit[15] This bit15 can only be written when in power-down mode. */
#define ADS1015_REG_CONFIG_OS_MASK      0x8000
#define ADS1015_REG_CONFIG_OS_SINGLE    0x8000  // Write: Set to start a single-conversion
#define ADS1015_REG_CONFIG_OS_BUSY      0x0000  // Read: Bit = 0 when conversion is in progress
#define ADS1015_REG_CONFIG_OS_NOTBUSY   0x8000  // Read: Bit = 1 when device is not performing a conversion

/* Bits[14:12] */
#define ADS1015_REG_CONFIG_MUX_MASK     0x7000
#define ADS1015_REG_CONFIG_MUX_DIFF_0_1 0x0000  // Differential P = AIN0, N = AIN1 (default)
#define ADS1015_REG_CONFIG_MUX_DIFF_0_3 0x1000  // Differential P = AIN0, N = AIN3
#define ADS1015_REG_CONFIG_MUX_DIFF_1_3 0x2000  // Differential P = AIN1, N = AIN3
#define ADS1015_REG_CONFIG_MUX_DIFF_2_3 0x3000  // Differential P = AIN2, N = AIN3
#define ADS1015_REG_CONFIG_MUX_SINGLE_0 0x4000  // Single-ended AIN0
#define ADS1015_REG_CONFIG_MUX_SINGLE_1 0x5000  // Single-ended AIN1
#define ADS1015_REG_CONFIG_MUX_SINGLE_2 0x6000  // Single-ended AIN2
#define ADS1015_REG_CONFIG_MUX_SINGLE_3 0x7000  // Single-ended AIN3
#define ADS1015_REG_CONFIG_MUX_SHIFT    12
#define ADS1015_CHANNEL_COUNT			8

/* Bits[11:9] */
#define ADS1015_REG_CONFIG_PGA_MASK     0x0E00
#define ADS1015_REG_CONFIG_PGA_6_144V   0x0000  // +/-6.144V range = Gain 2/3
#define ADS1015_REG_CONFIG_PGA_4_096V   0x0200  // +/-4.096V range = Gain 1
#define ADS1015_REG_CONFIG_PGA_2_048V   0x0400  // +/-2.048V range = Gain 2 (default)
#define ADS1015_REG_CONFIG_PGA_1_024V   0x0600  // +/-1.024V range = Gain 4
#define ADS1015_REG_CONFIG_PGA_0_512V   0x0800  // +/-0.512V range = Gain 8
#define ADS1015_REG_CONFIG_PGA_0_256V   0x0A00  // +/-0.256V range = Gain 16

/* Bit[8] */
#define ADS1015_REG_CONFIG_MODE_MASK    0x0100
#define ADS1015_REG_CONFIG_MODE_CONTIN  0x0000  // Continuous conversion mode
#define ADS1015_REG_CONFIG_MODE_SINGLE  0x0100  // Power-down single-shot mode (default)

/* Bits[7:5] Data rate */
#define ADS1015_REG_CONFIG_DR_MASK      0x00E0
#define ADS1015_REG_CONFIG_DR_128SPS    0x0000  // 128  samples per second
#define ADS1015_REG_CONFIG_DR_250SPS    0x0020  // 250  samples per second
#define ADS1015_REG_CONFIG_DR_490SPS    0x0040  // 490  samples per second
#define ADS1015_REG_CONFIG_DR_920SPS    0x0060  // 920  samples per second
#define ADS1015_REG_CONFIG_DR_1600SPS   0x0080  // 1600 samples per second (default)
#define ADS1015_REG_CONFIG_DR_2400SPS   0x00A0  // 2400 samples per second
#define ADS1015_REG_CONFIG_DR_3300SPS   0x00C0  // 3300 samples per second

/* Bit[4] */
#define ADS1015_REG_CONFIG_CMODE_MASK   0x0010
#define ADS1015_REG_CONFIG_CMODE_TRAD   0x0000  // Traditional comparator with hysteresis (default)
#define ADS1015_REG_CONFIG_CMODE_WINDOW 0x0010  // Window comparator

/* Bit[3] */
#define ADS1015_REG_CONFIG_CPOL_MASK    0x0008
#define ADS1015_REG_CONFIG_CPOL_ACTVLOW 0x0000  // ALERT/RDY pin is low when active (default)
#define ADS1015_REG_CONFIG_CPOL_ACTVHI  0x0008  // ALERT/RDY pin is high when active

/* Bit[2] */
#define ADS1015_REG_CONFIG_CLAT_MASK    0x0004  // Determines if ALERT/RDY pin latches once asserted
#define ADS1015_REG_CONFIG_CLAT_NONLAT  0x0000  // Non-latching comparator (default)
#define ADS1015_REG_CONFIG_CLAT_LATCH   0x0004  // Latching comparator

/* Bit[1:0] */
#define ADS1015_REG_CONFIG_CQUE_MASK    0x0003
#define ADS1015_REG_CONFIG_CQUE_1CONV   0x0000  // Assert ALERT/RDY after one conversions
#define ADS1015_REG_CONFIG_CQUE_2CONV   0x0001  // Assert ALERT/RDY after two conversions
#define ADS1015_REG_CONFIG_CQUE_4CONV   0x0002  // Assert ALERT/RDY after four conversions
#define ADS1015_REG_CONFIG_CQUE_NONE    0x0003  // Disable the comparator and put ALERT/RDY in high state (default)

/*
 * CONVERSION READY PIN (ADS1014/5 ONLY)
 * The ALERT/RDY pin can also be configured as a conversion ready pin.
 * This mode of operation can be realized if the MSB of the high threshold
 * register is set to '1' and the MSB of the low threshold register is
 * set to '0'. The COMP_POL bit continues to function and the COMP_QUE bits
 * can disable the pin; however, the COMP_MODE and COMP_LAT bits no longer
 * control any function. When configured as a conversion ready pin, ALERT/
 * RDY continues to require a pull-up resistor.
 * When in continuous conversion mode, the ADS1013/4/5 provide a brief (~8¦Ìs)
 * pulse on the ALERT/RDY pin at the end of each conversion.
 * When in single-shot shutdown mode, the ALERT/RDY pin asserts low at the
 * end of a conversion if the COMP_POL bit is set to '0'.
 */

/*
 * OR ADS1015_REG_CONFIG_MUX_MASK when use
 */
#define ADS1015_CFG_DEFAULT		ADS1015_REG_CONFIG_CQUE_NONE |		\
								ADS1015_REG_CONFIG_CLAT_NONLAT | 	\
								ADS1015_REG_CONFIG_CPOL_ACTVLOW | 	\
								ADS1015_REG_CONFIG_CMODE_TRAD | 	\
								ADS1015_REG_CONFIG_DR_1600SPS |		\
								ADS1015_REG_CONFIG_MODE_CONTIN |	\
								ADS1015_REG_CONFIG_PGA_4_096V
                        //    | ADS1015_REG_CONFIG_MUX_SINGLE_0

//-----------------------------------------------------------------------------
// read convert channel
//-----------------------------------------------------------------------------

#define ADS1015_CHANNEL_D0	    0xD0	// ADS1015_REG_CONFIG_MUX_DIFF_0_1,
#define ADS1015_CHANNEL_D1	    0xD1	// ADS1015_REG_CONFIG_MUX_DIFF_0_3,
#define ADS1015_CHANNEL_D2	    0xD2	// ADS1015_REG_CONFIG_MUX_DIFF_1_3,
#define ADS1015_CHANNEL_D3	    0xD3	// ADS1015_REG_CONFIG_MUX_DIFF_2_3,

#define ADS1015_CHANNEL_S0	    0x10	// ADS1015_REG_CONFIG_MUX_SINGLE_0,
#define ADS1015_CHANNEL_S1	    0x11	// ADS1015_REG_CONFIG_MUX_SINGLE_1,
#define ADS1015_CHANNEL_S2	    0x12	// ADS1015_REG_CONFIG_MUX_SINGLE_2,
#define ADS1015_CHANNEL_S3	    0x13	// ADS1015_REG_CONFIG_MUX_SINGLE_3,

//-----------------------------------------------------------------------------
// ioctl command
//-----------------------------------------------------------------------------

#define IOCTL_ADS1015_SET_CONV_CTRL		0x01
#define IOCTL_ADS1015_GET_CONV_CTRL		0x02
#define IOCTL_ADS1015_SET_LOW_THRESH	0x04
#define IOCTL_ADS1015_GET_LOW_THRESH	0x08
#define IOCTL_ADS1015_SET_HIGH_THRESH	0x10
#define IOCTL_ADS1015_GET_HIGH_THRESH	0x20

#define IOCTL_ADS1015_DISP_CONFIG_REG	0x80

//-----------------------------------------------------------------------------
// I2C0-ADS1015 driver operators
//-----------------------------------------------------------------------------

#include "ls1x_io.h"

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_ads1015_drv_ops;

#define ls1x_ads1015_read(iic, buf, size, arg)  ls1x_ads1015_drv_ops->read_entry(iic, buf, size, arg)
#define ls1x_ads1015_ioctl(iic, cmd, arg)       ls1x_ads1015_drv_ops->ioctl_entry(iic, cmd, arg)

#else

/*
 * ADS1015_read()
 *
 *   parameter:
 *     bus		busI2C0
 *     buf      is as uint16_t *
 *     arg      ADS1015_CHANNEL_D0: Differential P=AIN0, N=AIN1
 *              ADS1015_CHANNEL_D1: Differential P=AIN0, N=AIN3
 *              ADS1015_CHANNEL_D2: Differential P=AIN1, N=AIN3
 *              ADS1015_CHANNEL_D3: Differential P=AIN2, N=AIN3
 *              ADS1015_CHANNEL_S0: Single-ended AIN0
 *              ADS1015_CHANNEL_S1: Single-ended AIN1
 *              ADS1015_CHANNEL_S2: Single-ended AIN2
 *              ADS1015_CHANNEL_S3: Single-ended AIN3
 *
 *  return 		0=successful
 */
int ADS1015_read(void *bus, void *buf, int size, void *arg);

/*
 * ADS1015_ioctl()
 *
 *   parameter:
 *     	bus		busI2C0
 *
 *   ----------------------------------------------------------------------------------------------
 * 		cmd									| arg
 *   ----------------------------------------------------------------------------------------------
 * 		IOCTL_ADS1015_SET_CONV_CTRL			| uint16_t, set convert config register
 *   ----------------------------------------------------------------------------------------------
 * 		IOCTL_ADS1015_GET_CONV_CTRL			| uint16_t *, get convert config register
 *   ----------------------------------------------------------------------------------------------
 * 		IOCTL_ADS1015_SET_LOW_THRESH		| uint16_t, set low thresh register
 *   ----------------------------------------------------------------------------------------------
 * 		IOCTL_ADS1015_GET_LOW_THRESH		| uint16_t *, get low thresh register
 *   ----------------------------------------------------------------------------------------------
 * 		IOCTL_ADS1015_SET_HIGH_THRESH		| uint16_t, set high thresh register
 *   ----------------------------------------------------------------------------------------------
 * 		IOCTL_ADS1015_GET_HIGH_THRESH		| uint16_t *, get high thresh register
 *   ----------------------------------------------------------------------------------------------
 * 		IOCTL_ADS1015_DISP_CONFIG_REG		| NULL, display config register
 *   ----------------------------------------------------------------------------------------------
 *
 *  return 		0=successful
 */
int ADS1015_ioctl(void *bus, int cmd, void *arg);

#define ls1x_ads1015_read(iic, buf, size, arg)  ADS1015_read(iic, buf, size, arg)
#define ls1x_ads1015_ioctl(iic, cmd, arg)       ADS1015_ioctl(iic, cmd, arg)

#endif

//-----------------------------------------------------------------------------
// user api
//-----------------------------------------------------------------------------

/*
 *  parameter:
 *     bus		busI2C0
 *     channel	see ADS1015_read()'s arg
 */
uint16_t get_ads1015_adc(void *bus, int channel);

#ifdef	__cplusplus
}
#endif

#endif /* ADS1015_H_ */


