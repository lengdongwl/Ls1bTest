/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 *  LS1X CAN driver interface
 *
 *  Author: Bian, 2013/10/09
 */

#ifndef __LS1x_CAN_H__
#define __LS1x_CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"

/*
 * How to use:
 *
 * Init:
 *
 *      ls1x_can_init(devCAN0);
 *
 * Open:
 *
 *      ls1x_can_open(devCAN0, NULL);
 *
 *      Then Start It:
 *
 *      ls1x_can_ioctl(devCAN0, IOCTL_CAN_START, NULL);
 *
 * Read:
 *
 *      int readed;
 *      CANMsg_t msg;
 *
 *      msg.id=x;
 *      ...
 *      readed = ls1x_can_read(devCAN0, &msg, sizeof(msg), NULL);
 *      if ((readed == sizeof(msg)) && msg.len>0)
 *      {
 *          ...
 *      }
 *
 * Write:
 *
 *      int writted;
 *      CANMsg_t msg; 
 *
 *      msg.id=x;
 *      ...
 *      writted = ls1x_can_write(devCAN0, &msg, sizeof(msg), NULL);
 *      if (writted == sizeof(msg))
 *      {
 *          ...
 *      }
 *
 * Close:
 *
 *      ls1x_can_close(devCAN0);
 *
 */

//-------------------------------------------------------------------------------------------------

/*
 * CAN MESSAGE
 */
typedef struct
{
    unsigned int  id;               /* CAN message id */
    char          rtr;              /* RTR - Remote Transmission Request */
    char          extended;         /* whether extended message package */
    unsigned char len;              /* length of data */
    unsigned char data[8];          /* data for transfer */
} CANMsg_t;

/*
 * CAN SPEED
 */
typedef struct
{
    unsigned char btr0;
    unsigned char btr1;
    unsigned char samples;          /* =1: samples 3 times, otherwise once */
} CAN_speed_t;

/*
 * CAN ID AND FILTR
 */
typedef struct
{
    unsigned char code[4];
    unsigned char mask[4];
    int           afmode;           /* =1: single filter, otherwise twice */
} CAN_afilter_t;

/*
 * CAN STATUS
 */
typedef struct
{
    /* tx/rx stats */
    unsigned int rx_msgs;
    unsigned int tx_msgs;

    /* Error Interrupt counters */
    unsigned int err_warn;
    unsigned int err_dovr;
    unsigned int err_errp;
    unsigned int err_arb;
    unsigned int err_bus;

    /* BUS ERRORS
     */
    /* ALC 4-0 */
    unsigned int err_arb_bitnum[32];    /* At what bit arbitration is lost */

    /* ECC 7-6 */
    unsigned int err_bus_bit;           /* Bit error */
    unsigned int err_bus_form;          /* Form Error */
    unsigned int err_bus_stuff;         /* Stuff Error */
    unsigned int err_bus_other;         /* Other Error */

    /* ECC 5 */
    unsigned int err_bus_rx;            /* Errors during Reception */
    unsigned int err_bus_tx;            /* Errors during Transmission */

    /* ECC 4:0 */
    unsigned int err_bus_segs[32];      /* Segment (Where in frame error occured)
                                         * See CAN_SEG_* defines for indexes */
    /* SUMMARY */
    unsigned int ints;                  /* total number of interrupts */
    unsigned int tx_buf_err;            /* software monitoring hw errors */
    unsigned int rx_sw_dovr;            /* software fifo overrun */

} CAN_stats_t;

/*
 * indexes into CAN_stats.err_bus_segs[index]
 */
#define CAN_SEG_ID28            0x02        /* ID field bit 28:21 */
#define CAN_SEG_ID20            0x06        /* ID field bit 20:18 */
#define CAN_SEG_ID17            0x07        /* ID field bit 17:13 */
#define CAN_SEG_ID12            0x0F        /* ID field bit 12:5 */
#define CAN_SEG_ID4             0x0E        /* ID field bit 4:0 */

#define CAN_SEG_START           0x03        /* Start of Frame */
#define CAN_SEG_SRTR            0x04        /* Bit SRTR */
#define CAN_SEG_IDE             0x05        /* Bit IDE */
#define CAN_SEG_RTR             0x0C        /* Bit RTR */
#define CAN_SEG_RSV0            0x09        /* Reserved bit 0 */
#define CAN_SEG_RSV1            0x0D        /* Reserved bit 1 */

#define CAN_SEG_DLEN            0x0B        /* Data Length code */
#define CAN_SEG_DFIELD          0x0A        /* Data Field */

#define CAN_SEG_CRC_SEQ         0x08        /* CRC Sequence */
#define CAN_SEG_CRC_DELIM       0x18        /* CRC Delimiter */

#define CAN_SEG_ACK_SLOT        0x19        /* Acknowledge slot */
#define CAN_SEG_ACK_DELIM       0x1B        /* Acknowledge delimiter */
#define CAN_SEG_EOF             0x1A        /* End Of Frame */
#define CAN_SEG_INTERMISSION    0x12        /* Intermission */
#define CAN_SEG_ACT_ERR         0x11        /* Active error flag */
#define CAN_SEG_PASS_ERR        0x16        /* Passive error flag */
#define CAN_SEG_DOMINANT        0x13        /* Tolerate dominant bits */
#define CAN_SEG_EDELIM          0x17        /* Error delimiter */
#define CAN_SEG_OVERLOAD        0x1C        /* overload flag */

/*
 * Status
 */
#define CAN_STATUS_RESET        0x01
#define CAN_STATUS_OVERRUN      0x02
#define CAN_STATUS_WARN         0x04
#define CAN_STATUS_ERR_PASSIVE  0x08
#define CAN_STATUS_ERR_BUSOFF   0x10
#define CAN_STATUS_QUEUE_ERROR  0x80

/*
 * int ls1x_can_register(amba_confarea_type *bus);
*/
#define CAN_SPEED_500K          500000
#define CAN_SPEED_250K          250000
#define CAN_SPEED_125K          125000
#define CAN_SPEED_100K          100000
#define CAN_SPEED_75K           75000
#define CAN_SPEED_50K           50000
#define CAN_SPEED_25K           25000
#define CAN_SPEED_10K           10000

/*
 * CAN core mode
 */
#define CAN_CORE_20A            0x01        /* CAN core work as CAN2.0A, BasicCan */
#define CAN_CORE_20B            0x02        /* CAN core work as CAN2.0B, PeliCan */

/* CAN2.0B support workmode
 */
#define CAN_STAND_MODE          0x10        /* 正常工作模式 */
#define CAN_SLEEP_MODE          0x20        /* 睡眠模式 - Not implement */
#define CAN_LISTEN_ONLY         0x40        /* 只听模式 - Not implement  */
#define CAN_SELF_RECEIVE        0x80        /* 自接收模式 */

//-----------------------------------------------------------------------------
// CAN devices
//-----------------------------------------------------------------------------

#ifdef BSP_USE_CAN0
extern void *devCAN0;
#endif
#ifdef BSP_USE_CAN1
extern void *devCAN1;
#endif

//-----------------------------------------------------------------------------
// Ioctl command
//-----------------------------------------------------------------------------

#define IOCTL_CAN_START         0x0001      /* start the CAN hardware */
#define IOCTL_CAN_STOP          0x0002      /* stop the CAN hardware */
#define IOCTL_CAN_GET_CONF      0x0004      /* TODO NOT IMPLEMENTED */
#define IOCTL_CAN_GET_STATS     0x0008      /* CAN_stats_t *  - statics */
#define IOCTL_CAN_GET_STATUS    0x0010      /* unsigned int * - see below "status" */
#define IOCTL_CAN_SET_SPEED     0x0020      /* unsigned int   - speed */
#define IOCTL_CAN_SPEED_AUTO    0x0040      /* TODO NOT IMPLEMENTED */
#define IOCTL_CAN_SET_LINK      0x0080      /* TODO NOT IMPLEMENTED */
#define IOCTL_CAN_SET_FILTER    0x0100      /* CAN_afilter_t * - */
#define IOCTL_CAN_SET_BUFLEN    0x0400      /* unsigned int - rx-buf-len = XXXX & 0x0000FFFF
                                                            - tx-buf-len = XXXX >> 16 */
#define IOCTL_CAN_SET_BTRS      0x0800      /* unsigned int - timing.btr1 = XXXX & 0xFF
                                                            - timing.btr0 = (XXXX>>8) & 0xFF  */
#define IOCTL_CAN_SET_CORE      0x1000      /* unsigned int - see below "CAN core mode" */
#define IOCTL_CAN_SET_WORKMODE  0x2000      /* unsigned int - see below "CAN2.0B workmode" */
#define IOCTL_CAN_SET_TIMEOUT   0x4000      /* unsigned int - ms, 0: NO_TIMEOUT=BLOCK MODE */

//-----------------------------------------------------------------------------
// CAN driver operators
//-----------------------------------------------------------------------------

#include "ls1x_io.h"

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_can_drv_ops;

#define ls1x_can_init(can, arg)             ls1x_can_drv_ops->init_entry(can, arg)
#define ls1x_can_open(can, arg)             ls1x_can_drv_ops->open_entry(can, arg)
#define ls1x_can_close(can, arg)            ls1x_can_drv_ops->close_entry(can, arg)
#define ls1x_can_read(can, buf, size, arg)  ls1x_can_drv_ops->read_entry(can, buf, size, arg)
#define ls1x_can_write(can, buf, size, arg) ls1x_can_drv_ops->write_entry(can, buf, size, arg)
#define ls1x_can_ioctl(can, cmd, arg)       ls1x_can_drv_ops->ioctl_entry(can, cmd, arg)

#else

/*
 * 初始化CAN设备
 * 参数:    dev     devCAN0/devCAN1
 *          arg     NULL
 *
 * 返回:    0=成功
 *
 * 默认值:  内核模式: CAN_CORE_20B
 *          工作模式: CAN_STAND_MODE
 *          通信速率: CAN_SPEED_250K
 */
int LS1x_CAN_init(void *dev, void *arg);

/*
 * 打开CAN设备
 * 参数:    dev     devCAN0/devCAN1
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int LS1x_CAN_open(void *dev, void *arg);

/*
 * 关闭CAN设备
 * 参数:    dev     devCAN0/devCAN1
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int LS1x_CAN_close(void *dev, void *arg);

/*
 * 从CAN设备读数据(接收)
 * 参数:    dev     devCAN0/devCAN1
 *          buf     类型: CANMsg_t *, 数组, 用于存放读取数据的缓冲区
 *          size    类型: int, 待读取的字节数, 长度不能超过 buf 的容量, sizeof(CANMsg_t)倍数
 *          arg     NULL
 *
 * 返回:    读取的字节数
 *
 * 说明:    CAN使用中断接收, 接收到的数据存放在驱动内部缓冲区, 读操作总是从缓冲区读取.
 *          必须注意接收数据缓冲区溢出.
 */
int LS1x_CAN_read(void *dev, void *buf, int size, void *arg);

/*
 * 向CAN设备写数据(发送)
 * 参数:    dev     devCAN0/devCAN1
 *          buf     类型: CANMsg_t *, 数组, 用于存放待写数据的缓冲区
 *          size    类型: int, 待写入的字节数, 长度不能超过 buf 的容量, sizeof(CANMsg_t)倍数
 *          arg     NULL
 *
 * 返回:    写入的字节数
 *
 * 说明:    CAN使用中断发送, 待发送的数据直接发送, 或者存放在驱动内部缓冲区待中断发生时继续发送.
 *          必须注意发送数据缓冲区溢出.
 */
int LS1x_CAN_write(void *dev, void *buf, int size, void *arg);

/*
 * 向CAN设备发送控制命令
 * 参数:    dev     devCAN0/devCAN1
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                         |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_CAN_START             |   NULL. 启动CAN进入工作状态, 注意 ls1x_can_open()
 *                                      |   之后必须调用此命令, CAN硬件进入接收或者发送状态.
 *      ---------------------------------------------------------------------------------
 *          IOCTL_CAN_STOP              |   NULL. 停止CAN的工作状态, 在ls1x_can_close()
 *                                      |   之前调用此命令, 结束CAN硬件的接收或者发送状态.
 *      ---------------------------------------------------------------------------------
 *          IOCTL_CAN_GET_STATS         |   类型: CAN_stats_t *
 *                                      |   用途: 读取CAN设备的统计信息
 *      ---------------------------------------------------------------------------------
 *          IOCTL_CAN_GET_STATUS        |   类型: unsigned int *
 *                                      |   用途: 读取CAN设备的当前状态, 见上面"Status"定义
 *      ---------------------------------------------------------------------------------
 *          IOCTL_CAN_SET_SPEED         |   类型: unsigned int
 *                                      |   用途: 设置CAN设备的通信速率, 见"CAN_SPEED_x"定义
 *      ---------------------------------------------------------------------------------
 *          IOCTL_CAN_SET_FILTER        |   类型: CAN_afilter_t *
 *                                      |   用途: 设置CAN设备的硬件过滤器
 *      ---------------------------------------------------------------------------------
 *          IOCTL_CAN_SET_BUFLEN        |   类型: unsigned int
 *                                      |   用途: 更改CAN设备的内部缓存大小.
 *                                      |         低16位: 接收缓冲区个数;
 *                                      |         高16位: 发送缓冲区个数.
 *      ---------------------------------------------------------------------------------
 *          IOCTL_CAN_SET_CORE          |   类型: unsigned int
 *                                      |   用途: 设置CAN设备的内核模式,
 *                                      |         CAN_CORE_20A/CAN_CORE_20B
 *      ---------------------------------------------------------------------------------
 *          IOCTL_CAN_SET_WORKMODE      |   类型: unsigned int
 *                                      |   用途: 设置CAN设备2.0B的工作模式,
 *                                      |         CAN_STAND_MODE/CAN_SLEEP_MODE/
 *                                      |         CAN_LISTEN_ONLY/CAN_SELF_RECEIVE
 *      ---------------------------------------------------------------------------------
 *          IOCTL_CAN_SET_TIMEOUT       |   类型: unsigned int
 *                                      |   用途: 设置CAN设备接收/发送的超时等待毫秒数
 *      ---------------------------------------------------------------------------------
 *
 * 返回:    0=成功
 */
int LS1x_CAN_ioctl(void *dev, int cmd, void *arg);

#define ls1x_can_init(can, arg)             LS1x_CAN_init(can, arg)
#define ls1x_can_open(can, arg)             LS1x_CAN_open(can, arg)
#define ls1x_can_close(can, arg)            LS1x_CAN_close(can, arg)
#define ls1x_can_read(can, buf, size, arg)  LS1x_CAN_read(can, buf, size, arg)
#define ls1x_can_write(can, buf, size, arg) LS1x_CAN_write(can, buf, size, arg)
#define ls1x_can_ioctl(can, cmd, arg)       LS1x_CAN_ioctl(can, cmd, arg)

#endif

/*
 * for RT-Thread
 */
#if defined(OS_RTTHREAD)
const char *ls1x_can_get_device_name(void *pCAN);
#endif

#ifdef __cplusplus
}
#endif

#endif // __LS1x_CAN_H__


