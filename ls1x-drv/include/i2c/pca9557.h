/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * pca9557.h
 *
 * created: 2020/7/17
 *  author: Bian
 */

#ifndef _PCA9557_H
#define _PCA9557_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

//-----------------------------------------------------------------------------
// I2C0-PCA9557 driver operators
//-----------------------------------------------------------------------------

#include "ls1x_io.h"

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_pca9557_drv_ops;

#define ls1x_pca9557_init(iic, arg)             ls1x_pca9557_drv_ops->init_entry(iic, arg)
#define ls1x_pca9557_read(iic, buf, size, arg)  ls1x_pca9557_drv_ops->read_entry(iic, buf, size, arg)
#define ls1x_pca9557_write(iic, buf, size, arg) ls1x_pca9557_drv_ops->write_entry(iic, buf, size, arg)

#else

/*
 * PCA9557_init()
 *   parameter:
 *     	bus			busI2C0
 *     	arg			NULL
 *
 *   return 		0=successful
 *
 *   note			initialize the pca9557 gpio in/out port according to hardware.
 */
int PCA9557_init(void *bus, void *arg);

/*
 * PCA9557_read()
 *   parameter:
 *     	bus			busI2C0
 *     	buf			unsigned char *, read 1 byte of pca9557 in-register
 *     	size        1
 *     	arg         NULL
 *
 *   return 		0=successful
 */
int PCA9557_read(void *bus, void *buf, int size, void *arg);

/*
 * PCA9557_write()
 *   parameter:
 *     	bus			busI2C0
 *     	buf			unsigned char *, write 1 byte to pca9557 out-register
 *     	size        1
 *     	arg         NULL
 *
 *   return 		0=successful
 */
int PCA9557_write(void *bus, void *buf, int size, void *arg);

#define ls1x_pca9557_init(iic, arg)             PCA9557_init(iic, arg)
#define ls1x_pca9557_read(iic, buf, size, arg)  PCA9557_read(iic, buf, size, arg)
#define ls1x_pca9557_write(iic, buf, size, arg) PCA9557_write(iic, buf, size, arg)

#endif

//-----------------------------------------------------------------------------
// user api
//-----------------------------------------------------------------------------

/*
 * All these function according to hardware design of PCA9557.
 *
 * Parametr "bus" is busI2C0 currently
 */

bool get_ads1015_ready(void *bus);

bool get_rx8010_irq1(void *bus);
bool get_rx8010_irq2(void *bus);

#if defined(LS1C)
bool get_usb_otg_id(void *bus);
bool set_usb_otg_vbus(void *bus, bool en);

bool set_camera_reset(void *bus, bool en);
bool set_camera_powerdown(void *bus, bool en);

bool get_touch_down(void *bus);

#endif

#ifdef __cplusplus
}
#endif

#endif // _PCA9557_H

