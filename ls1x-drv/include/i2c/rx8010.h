/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * rx8010.h
 *
 * created: 2020/9/11
 *  author: Bian
 */

/*
 * Because of we have some problem with RTC of Loongson 1B/1C,
 * we have to use RX8010 on the development board.
 */

#ifndef _RX8010_H
#define _RX8010_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <time.h>

typedef struct
{
    struct tm at;
    int enabled;
    int pending;
} rtc_alarm_t;

/******************************************************************************
 * ×¢Òâ: Äê·Ý·¶Î§ 1970~2069
 */
 
//-----------------------------------------------------------------------------
// ioctl command
//-----------------------------------------------------------------------------

#define IOCTL_RTC_REG_READ    0x01
#define IOCTL_RTC_REG_WRITE   0x02
#define IOCTL_RTC_VL_READ     0x03
#define IOCTL_RTC_VL_CLR      0x04

//-----------------------------------------------------------------------------
// I2C0-RX8010 driver operators
//-----------------------------------------------------------------------------

#include "ls1x_io.h"

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_rx8010_drv_ops;

#define ls1x_rx8010_init(iic, arg)             ls1x_rx8010_drv_ops->init_entry(iic, arg)
#define ls1x_rx8010_read(iic, buf, size, arg)  ls1x_rx8010_drv_ops->read_entry(iic, buf, size, arg)
#define ls1x_rx8010_write(iic, buf, size, arg) ls1x_rx8010_drv_ops->write_entry(iic, buf, size, arg)
#define ls1x_rx8010_ioctl(iic, cmd, arg)       ls1x_rx8010_drv_ops->ioctl_entry(iic, cmd, arg)

#else

int RX8010_initialize(void *bus, void *arg);
int RX8010_read(void *bus, void *buf, int size, void *arg);    // buf: struct tm *
int RX8010_write(void *bus, void *buf, int size, void *arg);   // buf: struct tm *
int RX8010_ioctl(void *bus, int cmd, void *arg);

#define ls1x_rx8010_init(iic, arg)             RX8010_initialize(iic, arg)
#define ls1x_rx8010_read(iic, buf, size, arg)  RX8010_read(iic, buf, size, arg)
#define ls1x_rx8010_write(iic, buf, size, arg) RX8010_write(iic, buf, size, arg)
#define ls1x_rx8010_ioctl(iic, cmd, arg)       RX8010_ioctl(iic, cmd, arg)

#endif

//-----------------------------------------------------------------------------
// user api
//-----------------------------------------------------------------------------

/*
 * write system datetime to RX8010
 */
int set_rx8010_datetime(void *bus, struct tm *dt);

/*
 * read system datetime from RX8010
 */
int get_rx8010_datetime(void *bus, struct tm *dt);

#ifdef	__cplusplus
}
#endif

#endif // _RX8010_H

