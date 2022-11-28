/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_gmac.h
 *
 * Created on: 2013/07/01
 *     Author: Bian
 */

#ifndef _LS1X_GMAC_H
#define _LS1X_GMAC_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// GMAC devices
//-----------------------------------------------------------------------------

#ifdef BSP_USE_GMAC0
extern void *devGMAC0;
#endif
#ifdef BSP_USE_GMAC1
extern void *devGMAC1;
#endif

//-----------------------------------------------------------------------------
// GMAC Interrupt Callback function
//-----------------------------------------------------------------------------

typedef int (*net_rx_callback_t)(void);
typedef int (*net_tx_callback_t)(void);

//-----------------------------------------------------------------------------
// Ioctl command
//-----------------------------------------------------------------------------

#define IOCTL_GMAC_START            0x0101      /* start GMAC hardware */
#define IOCTL_GMAC_STOP             0x0102      /* stop GMAC hardware */
#define IOCTL_GMAC_RESET            0x0103      /* reset GMAC hardware */

#define IOCTL_GMAC_SET_MACADDR      0x0105      /* set mac address */
#define IOCTL_GMAC_SET_TIMEOUT      0x0106      /* wait transmit/receive time out */

#define IOCTL_GMAC_IS_RUNNING       0x0107      /* GMAC is running? started or not */
#define IOCTL_GMAC_SHOW_STATS       0x0108

/*
 * TODO GMAC ��·״̬
 */

//-----------------------------------------------------------------------------
// GMAC driver operators
//-----------------------------------------------------------------------------

#include "ls1x_io.h"

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_gmac_drv_ops;

#define ls1x_gmac_init(gmac, arg)             ls1x_gmac_drv_ops->init_entry(gmac, arg)
#define ls1x_gmac_read(gmac, buf, size, arg)  ls1x_gmac_drv_ops->read_entry(gmac, buf, size, arg)
#define ls1x_gmac_write(gmac, buf, size, arg) ls1x_gmac_drv_ops->write_entry(gmac, buf, size, arg)
#define ls1x_gmac_ioctl(gmac, cmd, arg)       ls1x_gmac_drv_ops->ioctl_entry(gmac, cmd, arg)

#else

/*
 * GMAC��ʼ��
 * ����:    dev     devGMAC0/devGMAC1
 *          arg     NULL
 *
 * ����:    0=�ɹ�
 */
int LS1x_GMAC_initialize(void *dev, void *arg);

/*
 * ��GMAC��ȡ���յ�����������
 * ����:    dev     devGMAC0/devGMAC1
 *          buf     ����: char *, ���ڴ�Ŷ�ȡ���ݵĻ�����
 *          size    ����: int, ����ȡ���ֽ���, ���Ȳ��ܳ��� buf ������
 *          arg     NULL
 *
 * ����:    ���ζ�ȡ���ֽ���
 */
int LS1x_GMAC_read(void *dev, void *buf, int size, void *arg);

/*
 * ��GMACд������͵���������
 * ����:    dev     devGMAC0/devGMAC1
 *          buf     ����: char *, ���ڴ�Ŵ��������ݵĻ�����
 *          size    ����: int, �����͵��ֽ���, ���Ȳ��ܳ��� buf ������
 *          arg     NULL
 *
 * ����:    ���η��͵��ֽ���
 */
int LS1x_GMAC_write(void *dev, void *buf, int size, void *arg);

/*
 * GMAC ��������
 * ����:    dev     devGMAC0/devGMAC1
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                         |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GMAC_START            |   NULL, ����GMAC�豸
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GMAC_STOP             |   NULL, ֹͣGMAC�豸
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GMAC_RESET            |   NULL, ��λGMAC�豸
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GMAC_SET_TIMEOUT      |   ����: unsigned int
 *                                      |   ��;: ���ý���/���͵ĳ�ʱ�ȴ�ʱ��(ms)
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GMAC_IS_RUNNING       |   NULL, GMAC�豸�Ƿ�����
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GMAC_SHOW_STATS       |   NULL, ��ӡGMAC�豸ͳ����Ϣ
 *      ---------------------------------------------------------------------------------
 *
 * ����:    0=�ɹ�
 */
int LS1x_GMAC_ioctl(void *dev, int cmd, void *arg);

#define ls1x_gmac_init(gmac, arg)             LS1x_GMAC_initialize(gmac, arg)
#define ls1x_gmac_read(gmac, buf, size, arg)  LS1x_GMAC_read(gmac, buf, size, arg)
#define ls1x_gmac_write(gmac, buf, size, arg) LS1x_GMAC_write(gmac, buf, size, arg)
#define ls1x_gmac_ioctl(gmac, cmd, arg)       LS1x_GMAC_ioctl(gmac, cmd, arg)

#endif

//-----------------------------------------------------------------------------
// GMAC driver api
//-----------------------------------------------------------------------------

/*
 * �ȴ�GMAC���յ���������
 * ����:    dev     devGMAC0/devGMAC1
 *          bufptr  ����: unsigned char **, ����GMAC�����ڲ����ջ�������ַ
 *
 * ����:    0=�ɹ�
 *
 * ˵��:    1. RTOS�µ��øú���ʱ, ʹ��RTOS�¼�ʵ�����޵ȴ�;
 *          2. ����µ��øú���ʱ, �ȴ�ʱ��ΪIOCTL_GMAC_SET_TIMEOUT���õĳ�ʱ������
 */
int LS1x_GMAC_wait_receive_packet(void *dev, unsigned char **bufptr);

/*
 * �ȴ�GMAC����ʱ��������
 * ����:    dev     devGMAC0/devGMAC1
 *          bufptr  ����: unsigned char **, ����GMAC�����ڲ����ջ�������ַ
 *
 * ����:    0=�ɹ�
 *
 * ˵��:    1. RTOS�µ��øú���ʱ, ʹ��RTOS�¼�ʵ�����޵ȴ�;
 *          2. ����µ��øú���ʱ, �ȴ�ʱ��ΪIOCTL_GMAC_SET_TIMEOUT���õĳ�ʱ������
 */
int LS1x_GMAC_wait_transmit_idle(void *dev, unsigned char **bufptr);

#define ls1x_gmac_wait_rx_packet(gmac, pbuf)  LS1x_GMAC_wait_receive_packet(gmac, pbuf)
#define ls1x_gmac_wait_tx_idle(gmac, pbuf)    LS1x_GMAC_wait_transmit_idle(gmac, pbuf)

/*
 * for RT-Thread
 */
#if defined(OS_RTTHREAD)
const char *ls1x_gmac_get_device_name(void *pMAC);
#endif

#ifdef __cplusplus
}
#endif

#endif // _LS1X_GMAC_H

