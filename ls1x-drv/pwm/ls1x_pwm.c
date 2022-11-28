/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_pwm.c
 *
 * created: 2020/6/26
 *  author: Bian
 *
 */

#include "bsp.h"

#if defined(BSP_USE_PWM0) || defined(BSP_USE_PWM1) || defined(BSP_USE_PWM2) || defined(BSP_USE_PWM3)

#include <stdint.h>

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_irq.h"
#define LS1x_PWM0_BASE  LS1B_PWM0_BASE
#define LS1x_PWM0_IRQ   LS1B_PWM0_IRQ
#define LS1x_PWM1_BASE  LS1B_PWM1_BASE
#define LS1x_PWM1_IRQ   LS1B_PWM1_IRQ
#define LS1x_PWM2_BASE  LS1B_PWM2_BASE
#define LS1x_PWM2_IRQ   LS1B_PWM2_IRQ
#define LS1x_PWM3_BASE  LS1B_PWM3_BASE
#define LS1x_PWM3_IRQ   LS1B_PWM3_IRQ
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_irq.h"
#define LS1x_PWM0_BASE  LS1C_PWM0_BASE
#define LS1x_PWM0_IRQ   LS1C_PWM0_IRQ
#define LS1x_PWM1_BASE  LS1C_PWM1_BASE
#define LS1x_PWM1_IRQ   LS1C_PWM1_IRQ
#define LS1x_PWM2_BASE  LS1C_PWM2_BASE
#define LS1x_PWM2_IRQ   LS1C_PWM2_IRQ
#define LS1x_PWM3_BASE  LS1C_PWM3_BASE
#define LS1x_PWM3_IRQ   LS1C_PWM3_IRQ
#else
#error "No Loongson1x SoC defined."
#endif

#if defined(OS_RTTHREAD)
#include "rtthread.h"
#elif defined(OS_UCOS)
#include "os.h"
#elif defined(OS_FREERTOS)
#include "FreeRTOS.h"
#include "event_groups.h"
#endif

#include "ls1x_io.h"
#include "ls1x_pwm_hw.h"
#include "ls1x_pwm.h"

#include "drv_os_priority.h"

//-------------------------------------------------------------------------------------------------
// PWM device
//-------------------------------------------------------------------------------------------------

typedef struct
{
	LS1x_PWM_regs_t *hwPWM;                 /* pointer to HW registers */
	unsigned int     bus_freq;              /* 总线频率 */

    /* interrupt support */
	unsigned int     irq_num;               /* interrupt num */
	unsigned int     int_ctrlr;             /* interrupt controller */
	unsigned int     int_mask;              /* interrupt mask */

	unsigned int     hi_level_ns;           /* 高电平时间 ns */
	unsigned int     lo_level_ns;           /* 低电平时间 ns */

	int              work_mode;             /* timer or pulse */
    int              single;                /* 单次或者连续 */

    /*
     * 当工作在定时器模式时
     */
    irq_handler_t       isr;                /* user defined interrupt handler */
    pwmtimer_callback_t callback;           /* callback when irq ocurred */

#if defined(OS_RTTHREAD)
	rt_event_t         event;               /* Send the RTOS event when irq ocurred */
#elif defined(OS_UCOS)
	OS_FLAG_GRP       *event;
#elif defined(OS_FREERTOS)
	EventGroupHandle_t event;
#endif

    char dev_name[16];
    int  initialized;
    int  busy;
} PWM_t;

//-------------------------------------------------------------------------------------------------

#define PWM_INDEX(pwm) \
        ((unsigned int)pwm->hwPWM == LS1x_PWM0_BASE ? 0 : \
         (unsigned int)pwm->hwPWM == LS1x_PWM1_BASE ? 1 : \
         (unsigned int)pwm->hwPWM == LS1x_PWM2_BASE ? 2 : 3)

//-------------------------------------------------------------------------------------------------
// 微秒转换为 bus clock 数: (ns * bus_freq) / 1000000000
//-------------------------------------------------------------------------------------------------

#define PWM_AUTO_LIMIT          // 控制取值

static unsigned int NS_2_BUS_CLOCKS(unsigned int bus_freq, unsigned int ns)
{
    unsigned int clocks;

    clocks = (unsigned int)((double)ns * bus_freq / 1000000000.0);

#ifdef PWM_AUTO_LIMIT
    if (clocks >= PWM_REG_VALUE_MAX)
        clocks = PWM_REG_VALUE_MAX - 1;
#endif

    return clocks;
}

//-------------------------------------------------------------------------------------------------
// PWM operator
//-------------------------------------------------------------------------------------------------

#define PWM_RESET(pwm) \
        do { \
            pwm->hwPWM->ctrl = pwm_ctrl_rest | pwm_ctrl_iflag; \
            ls1x_sync(); \
        } while (0)

#if defined(LS1B)
#define PWM_TIMER_START(pwm) \
        do { \
            pwm->hwPWM->ctrl = pwm_ctrl_cntr_en | pwm_ctrl_ien | \
                               pwm_ctrl_single| pwm_ctrl_oe_mask; \
            ls1x_sync(); \
        } while (0)

#elif defined(LS1C)
#define PWM_TIMER_START(pwm) \
        do { \
            pwm->hwPWM->ctrl = pwm_ctrl_cntr_en | pwm_ctrl_ien | \
                               pwm_ctrl_single  | pwm_ctrl_oe_mask | \
                               pwm_ctrl_hrc_ien; \
            ls1x_sync(); \
        } while (0)

#endif

static int LS1x_PWM_start(PWM_t *pwm)
{
    if ((NULL == pwm) || (!pwm->initialized))
        return -1;

    PWM_RESET(pwm);                                             /* Reset PWM */

    switch (pwm->work_mode)
    {
        case PWM_SINGLE_TIMER:
        case PWM_CONTINUE_TIMER:
            LS1x_INTC_CLR(pwm->int_ctrlr)  = pwm->int_mask;     /* Clear PWM Interrupt flag */
            LS1x_INTC_IEN(pwm->int_ctrlr) |= pwm->int_mask;     /* Enable PWM Interrupt */
            PWM_TIMER_START(pwm);                               /* Start the timer */
            break;

        case PWM_SINGLE_PULSE:
        case PWM_CONTINUE_PULSE:
            if (pwm->single)
                pwm->hwPWM->ctrl = pwm_ctrl_cntr_en | pwm_ctrl_single;
            else
                pwm->hwPWM->ctrl = pwm_ctrl_cntr_en;
            break;

        default:
            return -1;
    }

    pwm->busy = 1;

    return 0;
}

static int LS1x_PWM_stop(PWM_t *pwm)
{
    if (NULL == pwm)
        return -1;

    /******************************************************
     * Stop PWM
     */
    PWM_RESET(pwm);

    pwm->hwPWM->ctrl = 0;
    ls1x_sync();
    /*
     * 关中断
     */
    LS1x_INTC_IEN(pwm->int_ctrlr) &= ~pwm->int_mask;
    LS1x_INTC_CLR(pwm->int_ctrlr)  =  pwm->int_mask;

    pwm->busy = 0;

    return 0;
}

/*
 * PWM timer interrupt, only timer mode
 */
static void LS1x_PWM_timer_common_isr(int vector, void *arg)
{
    PWM_t *pwm = (PWM_t *)arg;
    int stopme = 0;

    if (NULL == pwm)
        return;

    /*
     * Continue mode, Restart the timer
     */
    if (!pwm->single)
    {
        PWM_RESET(pwm);             /* Reset PWM */
        PWM_TIMER_START(pwm);
    }

    if (NULL != pwm->callback)      /* Timer callback */
    {
        pwm->callback((void *)pwm, &stopme);
    }

    if (pwm->single || stopme)
    {
        LS1x_PWM_stop(pwm);
    }

#if BSP_USE_OS
    /******************************************************
     * Timer Event of RTOS
     */
    if (pwm->event == NULL)
        return;

  #if defined(OS_RTTHREAD)

    rt_event_send(pwm->event, PWM_TIMER_EVENT);

  #elif defined(OS_UCOS)

    unsigned char err;
    OSFlagPost(pwm->event,
               (OS_FLAGS)PWM_TIMER_EVENT,
               OS_FLAG_SET,
               &err);

  #elif defined(OS_FREERTOS)

    BaseType_t xResult, xHigherPriorityTaskWoken = pdFALSE;
	xResult = xEventGroupSetBitsFromISR(pwm->event,
                                        PWM_TIMER_EVENT,
                                        &xHigherPriorityTaskWoken);
    if (xResult != pdPASS)  /* Was the message posted successfully? */
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  #endif
#endif // #if BSP_USE_OS
}

//-------------------------------------------------------------------------------------------------
// PWM driver implement
//-------------------------------------------------------------------------------------------------

STATIC_DRV int LS1x_PWM_initialize(void *dev, void *arg)
{
    PWM_t *pwm = (PWM_t *)dev;

    if (NULL == pwm)
        return -1;

    if (pwm->initialized)
        return 0;

    pwm->bus_freq = LS1x_BUS_FREQUENCY(CPU_XTAL_FREQUENCY);

    pwm->hi_level_ns = 0;
    pwm->lo_level_ns = 0;
    pwm->work_mode   = 0;
    pwm->single      = 0;
    pwm->isr         = NULL;
    pwm->callback    = NULL;
#if BSP_USE_OS
    pwm->event       = NULL;
#endif

    /******************************************************************
     * Config PWM interrupt
     */
    LS1x_INTC_CLR(pwm->int_ctrlr)   =  pwm->int_mask;
    LS1x_INTC_IEN(pwm->int_ctrlr)  &= ~pwm->int_mask;
    LS1x_INTC_EDGE(pwm->int_ctrlr) &= ~pwm->int_mask;
    LS1x_INTC_POL(pwm->int_ctrlr)  |=  pwm->int_mask;

    pwm->initialized = 1;

    return 0;
}

STATIC_DRV int LS1x_PWM_open(void *dev, void *arg)
{
    PWM_t *pwm = (PWM_t *)dev;
    pwm_cfg_t *cfg = (pwm_cfg_t *)arg;
    unsigned int hrc_clocks, lrc_clocks;

    if ((NULL == pwm) || (!pwm->initialized))
        return -1;

    if (pwm->busy)
        return -2;

    pwm->hi_level_ns = cfg->hi_ns;
    pwm->lo_level_ns = cfg->lo_ns;
    pwm->work_mode   = cfg->mode;

    pwm->single = (cfg->mode == PWM_SINGLE_PULSE) || (cfg->mode == PWM_SINGLE_TIMER);

    /*
     * Configure the PWM
     */
    hrc_clocks = NS_2_BUS_CLOCKS(pwm->bus_freq, pwm->hi_level_ns);
    lrc_clocks = NS_2_BUS_CLOCKS(pwm->bus_freq, pwm->lo_level_ns);

    switch (pwm->work_mode)
    {
        case PWM_SINGLE_PULSE:
        case PWM_CONTINUE_PULSE:
            if ((hrc_clocks == 0) || (lrc_clocks == 0))
            {
                printk("Error: PWM%i register zero, hrc=0x%08x, lrc=0x%08x, too small.\r\n",
                        PWM_INDEX(pwm), hrc_clocks, lrc_clocks);
                return -1;
            }

        #ifndef PWM_AUTO_LIMIT
            if ((hrc_clocks >= PWM_REG_VALUE_MAX) || (lrc_clocks >= PWM_REG_VALUE_MAX))
            {
                printk("Error: PWM%i register exceed, hrc=0x%08x, lrc=0x%08x, too big.\r\n",
                        PWM_INDEX(pwm), hrc_clocks, lrc_clocks);
                return -1;
            }
        #endif

            pwm->hwPWM->counter = 0;
            pwm->hwPWM->hrc = lrc_clocks - 1; // hrc_clocks - 1;
            pwm->hwPWM->lrc = hrc_clocks + lrc_clocks - 1;
            break;

        case PWM_SINGLE_TIMER:
        case PWM_CONTINUE_TIMER:
            if (hrc_clocks == 0)
            {
                printk("Error: PWM%i register zero, hrc=0x%08x, too small.\r\n",
                        PWM_INDEX(pwm), hrc_clocks);
                return -1;
            }

        #ifndef PWM_AUTO_LIMIT
            if (hrc_clocks >= PWM_REG_VALUE_MAX)
            {
                printk("Error: PWM%i register exceed, hrc=0x%08x, too big.\r\n",
                        PWM_INDEX(pwm), hrc_clocks);
                return -1;
            }
        #endif

            pwm->hwPWM->counter = 0;
            pwm->hwPWM->hrc = hrc_clocks;
            pwm->hwPWM->lrc = hrc_clocks;
            break;
    }

    /******************************************************************
     * Install PWM Timer isr
     */
    if ((pwm->work_mode == PWM_SINGLE_TIMER) || (pwm->work_mode == PWM_CONTINUE_TIMER))
    {
        pwm->isr      = cfg->isr;
        pwm->callback = cfg->cb;

#if defined(OS_RTTHREAD)
        pwm->event = (rt_event_t)cfg->event;
#elif defined(OS_UCOS)
        pwm->event = (OS_FLAG_GRP *)cfg->event;
#elif defined(OS_FREERTOS)
        pwm->event = (EventGroupHandle_t)cfg->event;
#endif

        if (pwm->isr == NULL)
            ls1x_install_irq_handler(pwm->irq_num, LS1x_PWM_timer_common_isr, (void *)pwm);
        else
            ls1x_install_irq_handler(pwm->irq_num, pwm->isr, (void *)pwm);
    }
    else
    {
        pwm->isr      = NULL;
        pwm->callback = NULL;
#if BSP_USE_OS
        pwm->event    = NULL;
#endif
    }

    /******************************************************
     * Start the timer/pulsing
     */
    return LS1x_PWM_start(pwm);
}

STATIC_DRV int LS1x_PWM_close(void *dev, void *arg)
{
    int rt;
    PWM_t *pwm = (PWM_t *)dev;

    if (NULL == pwm)
        return -1;

    if (!pwm->busy)
        return 0;

    rt = LS1x_PWM_stop(pwm);

    if ((pwm->work_mode == PWM_SINGLE_TIMER) || (pwm->work_mode == PWM_CONTINUE_TIMER))
    {
        ls1x_remove_irq_handler(pwm->irq_num);      /* uninstall isr? */
    }

    pwm->callback = NULL;
#if BSP_USE_OS
    pwm->event    = NULL;
#endif

    return rt;
}

//-------------------------------------------------------------------------------------------------
// PWM devices
//-------------------------------------------------------------------------------------------------

/* PWM 0 */
#ifdef BSP_USE_PWM0
static PWM_t ls1x_PWM0 =
{
	.hwPWM       = (LS1x_PWM_regs_t *)LS1x_PWM0_BASE,
	.irq_num     = LS1x_PWM0_IRQ,
	.int_ctrlr   = LS1x_INTC0_BASE,
	.int_mask    = INTC0_PWM0_BIT,
	.dev_name    = "pwm0",
	.initialized = 0,
};
void *devPWM0 = (void *)&ls1x_PWM0;
#else
void *devPWM0 = NULL;
#endif

/* PWM 1 */
#ifdef BSP_USE_PWM1
static PWM_t ls1x_PWM1 =
{
	.hwPWM       = (LS1x_PWM_regs_t *)LS1x_PWM1_BASE,
	.irq_num     = LS1x_PWM1_IRQ,
	.int_ctrlr   = LS1x_INTC0_BASE,
	.int_mask    = INTC0_PWM1_BIT,
	.dev_name    = "pwm1",
	.initialized = 0,
};
void *devPWM1 = (void *)&ls1x_PWM1;
#else
void *devPWM1 = NULL;
#endif

/* PWM 2 */
#ifdef BSP_USE_PWM2
static PWM_t ls1x_PWM2 =
{
	.hwPWM       = (LS1x_PWM_regs_t *)LS1x_PWM2_BASE,
	.irq_num     = LS1x_PWM2_IRQ,
	.int_ctrlr   = LS1x_INTC0_BASE,
	.int_mask    = INTC0_PWM2_BIT,
	.dev_name    = "pwm2",
	.initialized = 0,
};
void *devPWM2 = (void *)&ls1x_PWM2;
#else
void *devPWM2 = NULL;
#endif

/* PWM 3 */
#ifdef BSP_USE_PWM3
static PWM_t ls1x_PWM3 =
{
	.hwPWM       = (LS1x_PWM_regs_t *)LS1x_PWM3_BASE,
	.irq_num     = LS1x_PWM3_IRQ,
	.int_ctrlr   = LS1x_INTC0_BASE,
	.int_mask    = INTC0_PWM3_BIT,
	.dev_name    = "pwm3",
	.initialized = 0,
};
void *devPWM3 = (void *)&ls1x_PWM3;
#else
void *devPWM3 = NULL;
#endif

#if (PACK_DRV_OPS)
/******************************************************************************
 * PWM driver operators
 */
static driver_ops_t LS1x_PWM_drv_ops =
{
    .init_entry  = LS1x_PWM_initialize,
    .open_entry  = LS1x_PWM_open,
    .close_entry = LS1x_PWM_close,
    .read_entry  = NULL,
    .write_entry = NULL,
    .ioctl_entry = NULL,
};

driver_ops_t *ls1x_pwm_drv_ops = &LS1x_PWM_drv_ops;
#endif

/******************************************************************************
 * user api
 */
int ls1x_pwm_pulse_start(void *pwm, pwm_cfg_t *cfg)
{
    if ((pwm == NULL) || (cfg == NULL))
        return -1;

    if ((cfg->mode == PWM_SINGLE_PULSE) || (cfg->mode == PWM_CONTINUE_PULSE))
    {
        if (LS1x_PWM_initialize(pwm, NULL) == 0)
        {
            return LS1x_PWM_open(pwm, cfg);
        }
    }

    return -1;
}

int ls1x_pwm_pulse_stop(void *pwm)
{
    if (pwm == NULL)
        return -1;

    if ((((PWM_t *)pwm)->work_mode == PWM_SINGLE_PULSE) ||
        (((PWM_t *)pwm)->work_mode == PWM_CONTINUE_PULSE))
    {
        return LS1x_PWM_close(pwm, NULL);
    }

    return -1;
}

int ls1x_pwm_timer_start(void *pwm, pwm_cfg_t *cfg)
{
    if ((pwm == NULL) || (cfg == NULL))
        return -1;

    if ((cfg->mode == PWM_SINGLE_TIMER) || (cfg->mode == PWM_CONTINUE_TIMER))
    {
        if (LS1x_PWM_initialize(pwm, NULL) == 0)
        {
            return LS1x_PWM_open(pwm, cfg);
        }
    }

    return -1;
}

int ls1x_pwm_timer_stop(void *pwm)
{
    if (pwm == NULL)
        return -1;

    if ((((PWM_t *)pwm)->work_mode == PWM_SINGLE_TIMER) ||
        (((PWM_t *)pwm)->work_mode == PWM_CONTINUE_TIMER))
    {
        return LS1x_PWM_close(pwm, NULL);
    }

    return -1;
}

/*
 * for RT-Thread
 */
#if defined(OS_RTTHREAD)
const char *ls1x_pwm_get_device_name(void *pwm)
{
    return ((PWM_t *)pwm)->dev_name;
}
#endif

#endif // #if defined(BSP_USE_PWM0) || defined(BSP_USE_PWM1) || defined(BSP_USE_PWM2) || defined(BSP_USE_PWM3)

/*
 * @@ END
 */
