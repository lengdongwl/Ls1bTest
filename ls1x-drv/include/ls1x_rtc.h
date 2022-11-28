/*
 * ls1x_rtc.h
 *
 * created: 2020/6/30
 * authour: 
 */

/*
 * LS1B: 可以使用
 *       1. 在写 toywritehi/toywritelo后, 一直标记 rtc_ctrl_ts 位. 
 *          在使能后: 可以读出日期, 在正确地计时; toymatch 有时可以使用
 *       2. 在写 rtcwrite后, 一直标记 rtc_ctrl_rs 位. 
 *          在使能后: rtcmatch 可以正常使用
 *
 * LS1C: 不能使用. 表现为: 在写 toytrim/rtctrim 会随机死机, 程序跑飞.
 *
 */

#ifndef _LS1X_RTC_H
#define _LS1X_RTC_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------

#include <time.h>
#include <stdbool.h>

#include "ls1x_io.h"

//-----------------------------------------------------------------------------

#define LS1X_RTC                0x0500
#define LS1X_TOY                0x0A00

/*
 * Virtual Sub-Device. 定时器虚拟子设备
 */
#define DEVICE_RTCMATCH0       (LS1X_RTC | 1)
#define DEVICE_RTCMATCH1       (LS1X_RTC | 2)
#define DEVICE_RTCMATCH2       (LS1X_RTC | 3)

#define DEVICE_TOYMATCH0       (LS1X_TOY | 1)
#define DEVICE_TOYMATCH1       (LS1X_TOY | 2)
#define DEVICE_TOYMATCH2       (LS1X_TOY | 3)

//-----------------------------------------------------------------------------
// 定时器中断回调函数
//-----------------------------------------------------------------------------

/*
 * 参数:    device  RTC虚拟子设备产生的中断
 *          match   当前RTCMATCH或者TOYMATCH的寄存器值
 *          stop    如果给*stop 赋非零值, 该定时器将停止不再工作, 否则定时器会自动重新
 *                  载入interval_ms值, 等待下一次定时器计时阀值产生中断.
 */
typedef void (*rtctimer_callback_t)(int device, unsigned match, int *stop);

//-----------------------------------------------------------------------------
// RTC parameter
//-----------------------------------------------------------------------------

typedef struct rtc_cfg
{
    /*
     * This parameter's unit is millisecond(ms). After interrupt ocurred,
     * this value will auto loaded for next match interrupt.
     * if trig_datetime is NULL, toymatch use this parameter.
     *
     */
    int        interval_ms;

    /*
     * This parameter is used by toymatch. When toymatch arrives this datetime
     * toymatch interrupt will be triggered. This interrupt ocurred only once.
     *
     * if This parameter is NULL, use interval_ms as irq trigger interval
     *
     */
    struct tm *trig_datetime;

    irq_handler_t       isr;            /* User defined match-isr */
    rtctimer_callback_t cb;             /* called by match-isr */
#if BSP_USE_OS
    void               *event;          /* RTOS event created by user */
#endif
} rtc_cfg_t;

//-----------------------------------------------------------------------------
// ioctl command
//-----------------------------------------------------------------------------

#define IOCTL_SET_SYS_DATETIME      0x8001      // struct tm *
#define IOCTL_GET_SYS_DATETIME      0x8002      // struct tm *

/**
 * control rtc
 */
#define IOCTL_RTC_SET_TRIM          0x0811      // unsigned int *
#define IOCTL_RTC_GET_TRIM          0x0812      // unsigned int *

#define IOCTL_RTCMATCH_START        0x0813      // DEVICE_RTCMATCHx & rtc_cfg_t *
#define IOCTL_RTCMATCH_STOP         0x0814      // DEVICE_RTCMATCHx

/**
 * control toy
 */
#define IOCTL_TOY_SET_TRIM          0x0821      // unsigned int *
#define IOCTL_TOY_GET_TRIM          0x0822      // unsigned int *

#define IOCTL_TOYMATCH_START        0x0823      // DEVICE_TOYMATCHx & rtc_cfg_t *
#define IOCTL_TOYMATCH_STOP         0x0824      // DEVICE_TOYMATCHx

//-----------------------------------------------------------------------------
// RTC driver operators
//-----------------------------------------------------------------------------

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_rtc_drv_ops;

#define ls1x_rtc_init(rtc, arg)             ls1x_rtc_drv_ops->init_entry(rtc, arg)
#define ls1x_rtc_open(rtc, arg)             ls1x_rtc_drv_ops->open_entry(rtc, arg)
#define ls1x_rtc_close(rtc, arg)            ls1x_rtc_drv_ops->close_entry(rtc, arg)
#define ls1x_rtc_read(rtc, buf, size, arg)  ls1x_rtc_drv_ops->read_entry(rtc, buf, size, arg)
#define ls1x_rtc_write(rtc, buf, size, arg) ls1x_rtc_drv_ops->write_entry(rtc, buf, size, arg)
#define ls1x_rtc_ioctl(rtc, cmd, arg)       ls1x_rtc_drv_ops->ioctl_entry(rtc, cmd, arg)

#else

/*
 * RTC初始化
 * 参数:    dev     总是 NULL
 *          arg     类型: struct tm *. 如果该参数不是 NULL, 其值用于初始化RTC系统时间.
 *
 * 返回:    0=成功
 */
int LS1x_RTC_initialize(void *dev, void *arg);

/*
 * 打开RTC定时器
 * 参数:    dev     要打开的RTC子设备 DEVICE_XXX
 *          arg     类型: rtc_cfg_t *, 用于设置RTC子设备的工作模式并启动
 *
 * 返回:    0=成功
 *
 * 说明:    如果使用的是RTC子设备, 必须设置参数rtc_cfg_t的interval_ms值, 当RTC计时到达interval_ms阀值时,
 *          将触发RTC定时中断, 这时中断响应:
 *          1. 如果传入参数有用户自定义中断 isr(!=NULL), 则响应isr;
 *          2. 如果自定义中断 isr=NULL, 使用RTC默认中断, 该中断调用cb 回调函数让用户作出定时响应;
 *          3. 如果自定义中断 isr=NULL且cb=NULL, 如果有event参数, RTC默认中断将发出RTC_TIMER_EVENT事件.
 *
 *          如果使用的是TOY子设备, 并且设置有interval_ms参数(>1000), 用法和使用RTC子设备一样;
 *          当interval_ms==0且trig_datetime!=NULL时, 表示TOY子设备将在计时到达这个未来时间点时触发中断,
 *          中断处理流程和上面一致.
 *          使用trig_datetime触发的中断仅发生一次.
 *
 *          interval_ms用于间隔产生中断并且一直产生; trig_datetime用于到时产生中断仅产生一次.
 *
 */
int LS1x_RTC_open(void *dev, void *arg);

/*
 * 关闭RTC定时器
 * 参数:    dev     要关闭的RTC子设备 DEVICE_XXX
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int LS1x_RTC_close(void *dev, void *arg);

/*
 * 读取当前RTC时钟
 * 参数:    dev     NULL
 *          buf     类型: struct tm *, 用于存放读取的时钟值
 *          size    类型: int, 大小=sizeof(struct tm)
 *          arg     NULL
 *
 * 返回:    读取的字节数, 正常为sizeof(struct tm)
 */
int LS1x_RTC_read(void *dev, void *buf, int size, void *arg);

/*
 * 设置RTC时钟
 * 参数:    dev     NULL
 *          buf     类型: struct tm *, 用于存放待写入的时钟值
 *          size    类型: int, 大小=sizeof(struct tm)
 *          arg     NULL
 *
 * 返回:    写入的字节数, 正常为sizeof(struct tm)
 */
int LS1x_RTC_write(void *dev, void *buf, int size, void *arg);

/*
 * 控制RTC时钟设备
 * 参数:    dev     NULL or DEVICE_XXX
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                             |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_SET_SYS_DATETIME          |   类型: truct tm *
 *                                          |   用途: 设置RTC系统时间值
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GET_SYS_DATETIME          |   类型: struct tm *
 *                                          |   用途: 获取当前RTC系统时间值
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTL_RTC_SET_TRIM              |   类型: unsigned int *
 *                                          |   用途: 设置RTC的32768HZ时钟脉冲分频值
 *      ---------------------------------------------------------------------------------
 *          IOCTL_RTC_GET_TRIM              |   类型: unsigned int *
 *                                          |   用途: 获取RTC的32768HZ时钟脉冲分频值
 *      ---------------------------------------------------------------------------------
 *          IOCTL_RTCMATCH_START            |   类型: rtc_cfg_t *, 启动RTC定时器
 *                                          |   dev==DEVICE_RTCMATCHx
 *      ---------------------------------------------------------------------------------
 *          IOCTL_RTCMATCH_STOP             |   类型: NULL, 停止RTC定时器
 *                                          |   dev==DEVICE_RTCMATCHx
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTL_TOY_SET_TRIM              |   类型: unsigned int *
 *                                          |   用途: 设置TOY的32768HZ时钟脉冲分频值
 *      ---------------------------------------------------------------------------------
 *          IOCTL_TOY_GET_TRIM              |   类型: unsigned int *
 *                                          |   用途: 获取TOY的32768HZ时钟脉冲分频值
 *      ---------------------------------------------------------------------------------
 *          IOCTL_TOYMATCH_START            |   类型: rtc_cfg_t *, 启动TOY定时器
 *                                          |   dev==DEVICE_TOYMATCHx
 *      ---------------------------------------------------------------------------------
 *          IOCTL_TOYMATCH_STOP             |   类型: NULL, 停止TOY定时器
 *                                          |   dev==DEVICE_TOYMATCHx
 *      ---------------------------------------------------------------------------------
 *
 *
 * 返回:    0=成功
 */
int LS1x_RTC_ioctl(void *dev, int cmd, void *arg);                 

#define ls1x_rtc_init(rtc, arg)             LS1x_RTC_initialize(rtc, arg)
#define ls1x_rtc_open(rtc, arg)             LS1x_RTC_open(rtc, arg)
#define ls1x_rtc_close(rtc, arg)            LS1x_RTC_close(rtc, arg)
#define ls1x_rtc_read(rtc, buf, size, arg)  LS1x_RTC_read(rtc, buf, size, arg)
#define ls1x_rtc_write(rtc, buf, size, arg) LS1x_RTC_write(rtc, buf, size, arg)
#define ls1x_rtc_ioctl(rtc, cmd, arg)       LS1x_RTC_ioctl(rtc, cmd, arg)

#endif

//-----------------------------------------------------------------------------
// User API
//-----------------------------------------------------------------------------

/*
 * 设置RTC时钟值, 参见ls1x_rtc_read()
 */
int ls1x_rtc_set_datetime(struct tm *dt);
/*
 * 获取当前RTC时间, 参见ls1x_rtc_write()
 */
int ls1x_rtc_get_datetime(struct tm *dt);

/*
 * device is Virtual Sub Device
 */
/*
 * 开启定时器, 参见ls1x_rtc_open()
 */
int ls1x_rtc_timer_start(unsigned device, rtc_cfg_t *cfg);
/*
 * 关闭定时器, 参见ls1x_rtc_close()
 */
int ls1x_rtc_timer_stop(unsigned device);

/*
 * LS1x toymatch 日期格式转换
 */
void ls1x_tm_to_toymatch(struct tm *dt, unsigned int *match);
void ls1x_toymatch_to_tm(struct tm *dt, unsigned int match);

unsigned int ls1x_seconds_to_toymatch(unsigned int seconds);
unsigned int ls1x_toymatch_to_seconds(unsigned int match);

/*
 * struct tm 日期格式转换, +1900/-1900
 */
void normalize_tm(struct tm *tm, bool tm_format);

#ifdef __cplusplus
}
#endif

#endif // _LS1X_RTC_H

