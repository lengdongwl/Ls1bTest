/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_pwm.h
 *
 * created: 2020/6/27
 *  author: Bian
 */

/****************************************************************************************
 * PWM 工作模式:
 *
 * 1. PULSE 产生连续的或者单次的脉冲电平.
 *
 * 2. TIMER 当作连续的或者单次的定时器, 定时到达时产生中断.
 *          a. API 安装自定义中断来响应定时器中断;
 *          b. API 使用回调函数来响应定时器中断;
 *          c. API 在RTOS下使用 event 来捕捉定时器中断.
 *
 *    TIMER 中断响应a/b/c之一, 优先级: a->b->c
 *
 *    TODO  当用作定时器时有问题, 有些数值不能使用!!!
 *
 * 3. 芯片的PWM引脚可能被其它功能复用, 这时PWM设备仅可以用作内部定时器(脉冲信号不能从引脚输出).
 *
 ****************************************************************************************/

#ifndef _LS1X_PWM_H
#define _LS1X_PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ls1x_io.h"

//-----------------------------------------------------------------------------

#ifdef BSP_USE_PWM0
extern void *devPWM0;
#endif
#ifdef BSP_USE_PWM1
extern void *devPWM1;
#endif
#ifdef BSP_USE_PWM2
extern void *devPWM2;
#endif
#ifdef BSP_USE_PWM3
extern void *devPWM3;
#endif

//-----------------------------------------------------------------------------

/*
 * PWM 工作模式
 */
#define PWM_SINGLE_PULSE    0x01    // single pulse
#define PWM_SINGLE_TIMER    0x02    // single timer, interval = hi_ns
#define PWM_CONTINUE_PULSE  0x04    // continue pulse
#define PWM_CONTINUE_TIMER  0x08    // continue timer, interval = hi_ns

/*
 * PWM Timer 中断触发回调函数
 */

/*
 * 参数:    dev     devPWM0/devPWM1/devPWM2/devPWM3
 *          stopit  如果给*stopit 赋非零值, 该定时器将停止不再工作, 否则定时器会自动重新
 *                  载入hi_ns值, 等待下一次PWM计数到达阀值产生中断.
 */
typedef void (*pwmtimer_callback_t)(void *pwm, int *stopit);

/*
 * PWM open 的 arg 参数
 */
typedef struct pwm_cfg
{
    unsigned int hi_ns;             /* high level nano seconds,  定时器模式仅用 hi_ns */
    unsigned int lo_ns;             /* low  level nano seconds,  定时器模式没用 lo_ns */
    int          mode;              /* pulse or timer, see above 定时器工作模式 */

    irq_handler_t       isr;        /* User defined interrupt handler */
    pwmtimer_callback_t cb;         /* called by interrupt handler */
#if BSP_USE_OS
    void               *event;      /* RTOS event created by user */
#endif
} pwm_cfg_t;

//-----------------------------------------------------------------------------
// driver operator
//-----------------------------------------------------------------------------

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_pwm_drv_ops;

#define ls1x_pwm_init(pwm, arg)         ls1x_pwm_drv_ops->init_entry(pwm, arg)
#define ls1x_pwm_open(pwm, arg)         ls1x_pwm_drv_ops->open_entry(pwm, arg)
#define ls1x_pwm_close(pwm, arg)        ls1x_pwm_drv_ops->close_entry(pwm, arg)

#else

/*
 * PWM初始化
 * 参数:    dev     devPWM0/devPWM1/devPWM2/devPWM3
 *          arg     总是  NULL
 *
 * 返回:    0=成功
 */
int LS1x_PWM_initialize(void *dev, void *arg);

/*
 * 打开PWM设备
 * 参数:    dev     devPWM0/devPWM1/devPWM2/devPWM3
 *          arg     类型: pwm_cfg_t *, 用于设置PWM设备的工作模式并启动.
 *
 * 返回:    0=成功
 *
 * 说明:    如果PWM工作在脉冲模式, hi_ns是高电平纳秒数, lo_ns是低电平纳秒数.
 *          mode    PWM_SINGLE_PULSE:   产生单次脉冲
 *                  PWM_CONTINUE_PULSE: 产生连续脉冲
 *
 *          如果PWM工作在定时器模式, 定时器时间间隔使用hi_ns纳秒数, (忽略lo_ns). 当PWM计数到达时将触发PWM
 *          定时中断, 这时中断响应:
 *          1. 如果传入参数有用户自定义中断 isr(!=NULL), 则响应isr;
 *          2. 如果自定义中断 isr=NULL, 使用PWM默认中断, 该中断调用cb 回调函数让用户作出定时响应;
 *          3. 如果自定义中断 isr=NULL且cb=NULL, 如果有event参数, PWM默认中断将发出PWM_TIMER_EVENT事件.
 *
 *          mode    PWM_SINGLE_TIMER:   产生单次定时
 *                  PWM_CONTINUE_TIMER: 产生连续定时
 *
 */
int LS1x_PWM_open(void *dev, void *arg);

/*
 * 关闭PWM定时器
 * 参数:    dev     devPWM0/devPWM1/devPWM2/devPWM3
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int LS1x_PWM_close(void *dev, void *arg);

#define ls1x_pwm_init(pwm, arg)         LS1x_PWM_initialize(pwm, arg)
#define ls1x_pwm_open(pwm, arg)         LS1x_PWM_open(pwm, arg)
#define ls1x_pwm_close(pwm, arg)        LS1x_PWM_close(pwm, arg)

#endif

//-----------------------------------------------------------------------------
// user api
//-----------------------------------------------------------------------------

/*
 * 启动PWM设备产生脉冲
 * 参数:    dev     devPWM0/devPWM1/devPWM2/devPWM3
 *          cfg     类型: pwm_cfg_t *, mode==PWM_SINGLE_PULSE/PWM_CONTINUE_PULSE
 *
 * 返回:    0=成功
 */
int ls1x_pwm_pulse_start(void *pwm, pwm_cfg_t *cfg);
/*
 * 停止PWM设备产生脉冲
 * 参数:    dev     devPWM0/devPWM1/devPWM2/devPWM3
 *
 * 返回:    0=成功
 */
int ls1x_pwm_pulse_stop(void *pwm);

/*
 * 启动PWM定时器
 * 参数:    dev     devPWM0/devPWM1/devPWM2/devPWM3
 *          cfg     类型: pwm_cfg_t *, mode==PWM_SINGLE_TIMER/PWM_CONTINUE_TIMER
 *
 * 返回:    0=成功
 */
int ls1x_pwm_timer_start(void *pwm, pwm_cfg_t *cfg);
/*
 * 停止PWM定时器
 * 参数:    dev     devPWM0/devPWM1/devPWM2/devPWM3
 *
 * 返回:    0=成功
 */
int ls1x_pwm_timer_stop(void *pwm);

#if defined(OS_RTTHREAD)
const char *ls1x_pwm_get_device_name(void *pwm);
#endif

#ifdef __cplusplus
}
#endif

#endif // _LS1X_PWM_H

