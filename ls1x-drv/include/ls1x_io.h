/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef LS1X_IO_H_
#define LS1X_IO_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PACK_DRV_OPS    1

#if (PACK_DRV_OPS)
#define STATIC_DRV static
#else
#define STATIC_DRV
#endif

//-----------------------------------------------------------------------------
// 中断句柄的函数原型
//-----------------------------------------------------------------------------

/*
 * 参数:    vector  中断编号
 *          arg     安装中断向量时传入的参数
 */
typedef void (*irq_handler_t)(int vector, void *arg);

//-----------------------------------------------------------------------------
// IOCTL of spi_flash read engine mode, special for Loongson 1x
//-----------------------------------------------------------------------------

#define IOCTL_SPI_I2C_SET_TFRMODE       0x1000
#define IOCTL_FLASH_FAST_READ_ENABLE    0x2000  // none.          - set spi flash param to memory-en
#define IOCTL_FLASH_FAST_READ_DISABLE   0x4000  // none.          - unset spi flash param to memory-en
#define IOCTL_FLASH_GET_FAST_READ_MODE  0x8000  // unsigned int * - get spi flash is set to memory-en

//-----------------------------------------------------------------------------
// 用于 SPI/I2C 总线驱动的函数原型
//-----------------------------------------------------------------------------

typedef int (*I2C_init_t)(void *bus);
typedef int (*I2C_send_start_t)(void *bus, unsigned Addr);
typedef int (*I2C_send_stop_t)(void *bus, unsigned Addr);
typedef int (*I2C_send_addr_t)(void *bus, unsigned Addr, int rw);
typedef int (*I2C_read_bytes_t)(void *bus, unsigned char *bytes, int nbytes);
typedef int (*I2C_write_bytes_t)(void *bus, unsigned char *bytes, int nbytes);
typedef int (*I2C_ioctl_t)(void *bus, int cmd, void *arg);

typedef I2C_init_t        SPI_init_t;
typedef I2C_send_start_t  SPI_send_start_t;
typedef I2C_send_stop_t   SPI_send_stop_t;
typedef I2C_send_addr_t   SPI_send_addr_t;
typedef I2C_read_bytes_t  SPI_read_bytes_t;
typedef I2C_write_bytes_t SPI_write_bytes_t;
typedef I2C_ioctl_t       SPI_ioctl_t;

#if (PACK_DRV_OPS)
typedef struct libi2c_ops
{
    I2C_init_t          init;
    I2C_send_start_t    send_start;
    I2C_send_stop_t     send_stop;
    I2C_send_addr_t     send_addr;
    I2C_read_bytes_t    read_bytes;
    I2C_write_bytes_t   write_bytes;
    I2C_ioctl_t         ioctl;
} libi2c_ops_t;

typedef libi2c_ops_t    libspi_ops_t;
#endif

//-----------------------------------------------------------------------------
// 用于设备驱动的函数原型
//-----------------------------------------------------------------------------

typedef int (*driver_init_t)(void *dev, void *arg);
typedef int (*driver_open_t)(void *dev, void *arg);
typedef int (*driver_close_t)(void *dev, void *arg);
typedef int (*driver_read_t)(void *dev, void *buf, int size, void *arg);
typedef int (*driver_write_t)(void *dev, void *buf, int size, void *arg);
typedef int (*driver_ioctl_t)(void *dev, int cmd, void *arg);

#if (PACK_DRV_OPS)
typedef struct driver_ops
{
    driver_init_t       init_entry;     /* 设备初始化 */
    driver_open_t       open_entry;     /* 打开设备 */
    driver_close_t      close_entry;    /* 关闭设备 */
    driver_read_t       read_entry;     /* 读设备操作 */
    driver_write_t      write_entry;    /* 写设备操作 */
    driver_ioctl_t      ioctl_entry;    /* 设备控制 */
} driver_ops_t;
#endif

//-----------------------------------------------------------------------------

extern int fls(int x);

#ifdef __cplusplus
}
#endif

#endif /* LS1X_IO_H_ */


