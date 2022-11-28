/*
 * ls1x_dog.h
 *
 * created: 2021/3/12
 *  author: 
 */

#ifndef _LS1X_WATCHDOG_H
#define _LS1X_WATCHDOG_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// driver operator
//-----------------------------------------------------------------------------

#include "ls1x_io.h"

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_dog_drv_ops;

#define ls1x_dog_open(dog, arg)             ls1x_dog_drv_ops->open_entry(dog, arg)
#define ls1x_dog_close(dog, arg)            ls1x_dog_drv_ops->close_entry(dog, arg)
#define ls1x_dog_write(dog, buf, size, arg) ls1x_dog_drv_ops->write_entry(dog, buf, size, arg)

#else

/*
 * �������Ź�
 * ����:    dev     NULL. оƬֻ��һ��dog�豸
 *          arg     ����: unsigned int *. ������, dog�������������ֵʱϵͳ��λ
 *
 * ����:    0=�ɹ�
 */
int LS1x_DOG_open(void *dev, void *arg);

/*
 * �رտ��Ź�
 * ����:    dev     NULL. оƬֻ��һ��dog�豸
 *          arg     NULL
 *
 * ����:    ���� 0
 */
int LS1x_DOG_close(void *dev, void *arg);

/*
 * ���Ź�д������
 * ����:    dev     NULL. оƬֻ��һ��dog�豸
 *          buf     ����: unsigned int *. ������, dog�������������ֵʱϵͳ��λ
 *          size    =4, sizeof(unsigned int)
 *          arg     NULL
 *
 * ����:    4=�ɹ�
 */
int LS1x_DOG_write(void *dev, void *buf, int size, void *arg);

#define ls1x_dog_open(dog, arg)             LS1x_DOG_open(dog, arg)
#define ls1x_dog_close(dog, arg)            LS1x_DOG_close(dog, arg)
#define ls1x_dog_write(dog, buf, size, arg) LS1x_DOG_write(dog, buf, size, arg)

#endif

//-----------------------------------------------------------------------------
// user api
//-----------------------------------------------------------------------------

/*
 * �������Ź�
 * ����:    ms      ����: unsigned int. ������, dog�������������ֵʱϵͳ��λ
 *
 * ����:    0=�ɹ�
 */
int ls1x_watchdog_start(unsigned int ms);

/*
 * ι��
 * ����:    ms      ����: unsigned int. ������, dog�������������ֵʱϵͳ��λ
 *
 * ����:    4=�ɹ�
 */
int ls1x_watchdog_feed(unsigned int ms);

/*
 * �رտ��Ź�
 * ����:    (none)
 *
 * ����:    ���� 0
 */
int ls1x_watchdog_stop(void);

#ifdef __cplusplus
}
#endif

#endif // _LS1X_WATCHDOG_H

