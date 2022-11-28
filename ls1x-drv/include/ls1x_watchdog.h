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
 * 开启看门狗
 * 参数:    dev     NULL. 芯片只有一个dog设备
 *          arg     类型: unsigned int *. 毫秒数, dog计数到达这个数值时系统复位
 *
 * 返回:    0=成功
 */
int LS1x_DOG_open(void *dev, void *arg);

/*
 * 关闭看门狗
 * 参数:    dev     NULL. 芯片只有一个dog设备
 *          arg     NULL
 *
 * 返回:    总是 0
 */
int LS1x_DOG_close(void *dev, void *arg);

/*
 * 向看门狗写毫秒数
 * 参数:    dev     NULL. 芯片只有一个dog设备
 *          buf     类型: unsigned int *. 毫秒数, dog计数到达这个数值时系统复位
 *          size    =4, sizeof(unsigned int)
 *          arg     NULL
 *
 * 返回:    4=成功
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
 * 开启看门狗
 * 参数:    ms      类型: unsigned int. 毫秒数, dog计数到达这个数值时系统复位
 *
 * 返回:    0=成功
 */
int ls1x_watchdog_start(unsigned int ms);

/*
 * 喂狗
 * 参数:    ms      类型: unsigned int. 毫秒数, dog计数到达这个数值时系统复位
 *
 * 返回:    4=成功
 */
int ls1x_watchdog_feed(unsigned int ms);

/*
 * 关闭看门狗
 * 参数:    (none)
 *
 * 返回:    总是 0
 */
int ls1x_watchdog_stop(void);

#ifdef __cplusplus
}
#endif

#endif // _LS1X_WATCHDOG_H

