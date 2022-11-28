/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * mcp4725.h
 *
 *  Created on: 2015-12-8
 *      Author: Bian
 */

#ifndef MCP4725_H_
#define MCP4725_H_

#ifdef	__cplusplus
extern "C" {
#endif

//---------------------------------------------------------
// mcp4725 芯片标号 |  对于地址      |  I2C 地址
//---------------------------------------------------------
//           AJXX   |  A2/A1 = 0b00  |  0x60
//---------------------------------------------------------
//           APXX   |  A2/A1 = 0b01  |  0x62
//---------------------------------------------------------
//           AQXX   |  A2/A1 = 0b10  |  0x64
//---------------------------------------------------------
//           ARXX   |  A2/A1 = 0b11  |  0x66
//---------------------------------------------------------

/*
 * fast write DAC register with power-down mode
 */
#define MCP4725_PD_MASK				0x3000
#define MCP4725_PD_SHIFT			11
#define MCP4725_PD_0				0x0000		/* Normal Mode */
#define MCP4725_PD_1K				0x1000		/* 1   kΩ resistor to ground */
#define MCP4725_PD_100K				0x2000		/* 100 kΩ resistor to ground */
#define MCP4725_PD_500K				0x3000		/* 500 kΩ resistor to ground */

//-----------------------------------------------------------------------------
// ioctl command
//-----------------------------------------------------------------------------

#define IOCTL_MCP4725_WRTIE_EEPROM	    0x01    /* uint16_t: X X PD1 PD0 D11-0 */
#define IOCTL_MCP4725_DISP_CONFIG_REG	0x02
#define IOCTL_MCP4725_READ_DAC		    0x04
#define IOCTL_MCP4725_READ_EEPROM		0x08

//-----------------------------------------------------------------------------
// I2C0-MCP4725 driver operators
//-----------------------------------------------------------------------------

#include "ls1x_io.h"

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_mcp4725_drv_ops;

#define ls1x_mcp4725_write(iic, buf, size, arg) ls1x_mcp4725_drv_ops->write_entry(iic, buf, size, arg)
#define ls1x_mcp4725_ioctl(iic, cmd, arg)       ls1x_mcp4725_drv_ops->ioctl_entry(iic, cmd, arg)

#else

/*
 * MCP4725_write()
 *
 *   parameter:
 *     bus		busI2C0
 *     buf      uint16_t *, value to be convert out
 *     arg      NULL
 *
 *  return 		0=successful
 */
int MCP4725_write(void *bus, void *buf, int size, void *arg);

/*
 * MCP4725_ioctl()
 *
 *   parameter:
 *     	bus		busI2C0
 *
 *   ----------------------------------------------------------------------------------------------
 * 		cmd									| arg
 *   ----------------------------------------------------------------------------------------------
 * 		IOCTL_MCP4725_WRTIE_EEPROM			| uint16_t, set default value to eeprom of mcp4725
 * 											| to convert out after poweron
 *   ----------------------------------------------------------------------------------------------
 * 		IOCTL_MCP4725_READ_EEPROM			| uint16_t *, get default value of mcp4725's eeprom
 *   ----------------------------------------------------------------------------------------------
 * 		IOCTL_MCP4725_DISP_CONFIG_REG		| NULL, display all config register
 *   ----------------------------------------------------------------------------------------------
 * 		IOCTL_MCP4725_READ_DAC				| uint16_t *, get current value in convert register
 *   ----------------------------------------------------------------------------------------------
 *
 *  return 		0=successful
 */
int MCP4725_ioctl(void *bus, int cmd, void *arg);

#define ls1x_mcp4725_write(iic, buf, size, arg) MCP4725_write(iic, buf, size, arg)
#define ls1x_mcp4725_ioctl(iic, cmd, arg)       MCP4725_ioctl(iic, cmd, arg)

#endif

/******************************************************************************
 * user api
 */
/*
 * parameter
 *     	bus		busI2C0
 *     	dacVal  value to be convert out
 */
int set_mcp4725_dac(void *bus, unsigned short dacVal);

#ifdef	__cplusplus
}
#endif

#endif /* MCP4725_H_ */
