/*
 * ls1x_rtc.c
 *
 * created: 2020/6/29
 * authour: Bian
 */

/******************************************************************************
 * 使用 32.768kHZ 计数器时钟
 */
 
#include "bsp.h"

#if defined(LS1B) && defined(BSP_USE_RTC)

#include <string.h>
#include <time.h>
#include <stdbool.h>

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_irq.h"
#define LS1x_RTC_BASE   LS1B_RTC_BASE
#define LS1x_TOY0_IRQ   LS1B_TOY0_IRQ
#define LS1x_TOY1_IRQ   LS1B_TOY1_IRQ
#define LS1x_TOY2_IRQ   LS1B_TOY2_IRQ
#define LS1x_RTC0_IRQ   LS1B_RTC0_IRQ
#define LS1x_RTC1_IRQ   LS1B_RTC1_IRQ
#define LS1x_RTC2_IRQ   LS1B_RTC2_IRQ
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_irq.h"
#define LS1x_RTC_BASE   LS1C_RTC_BASE
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

#include "ls1x_rtc_hw.h"
#include "ls1x_rtc.h"

#include "drv_os_priority.h"

//-------------------------------------------------------------------------------------------------
// definition
//-------------------------------------------------------------------------------------------------

struct rtc_dev
{
    unsigned int        interval_ms;            /* timer irq trigger gap of ms */
    unsigned int        as_clocks;              /* timer interval as 32768 clocks */
    
    irq_handler_t       isr;                    /* User defined match-isr */
    rtctimer_callback_t callback;               /* callback when match-irq ocurred */
    
#if defined(OS_RTTHREAD)
	rt_event_t          event;                  /* Send the RTOS event when irq ocurred */
#elif defined(OS_UCOS)
	OS_FLAG_GRP        *event;
#elif defined(OS_FREERTOS)
	EventGroupHandle_t  event;
#endif

    int                 busy;
};

typedef struct ls1x_rtc
{
	LS1x_rtc_regs_t *hwRTC;                     /* LS1x_RTC_BASE */

    struct rtc_dev rtcs[3];                     /* rtc devices array */
    struct rtc_dev toys[3];                     /* toy devices array */

    int rtc_active_count;
    int toy_active_count;
    
    int initilized;
} RTC_t;

static RTC_t ls1x_RTC = { .initilized = 0, }, *pRTC = &ls1x_RTC;

//-------------------------------------------------------------------------------------------------
// macros
//-------------------------------------------------------------------------------------------------

#define GET_DEVICE(v)   ((unsigned int)v & 0xFF00)
#define GET_INDEX(v)    ((unsigned int)v & 0x00FF)

/*
 * INTC
 */
#define RTC_INT_INIT(bit) \
    do { \
        LS1x_INTC_EDGE(LS1x_INTC0_BASE) |=  bit; \
	    LS1x_INTC_POL( LS1x_INTC0_BASE) &= ~bit; \
	    LS1x_INTC_CLR( LS1x_INTC0_BASE)  =  bit; \
	    LS1x_INTC_IEN( LS1x_INTC0_BASE) &= ~bit; \
    } while (0)

#define RTC_INT_ENABLE(bit) \
    do { \
	    LS1x_INTC_CLR(LS1x_INTC0_BASE)  = bit; \
	    LS1x_INTC_IEN(LS1x_INTC0_BASE) |= bit; \
    } while (0)

#define RTC_INT_DISABLE(bit) \
    do { \
	    LS1x_INTC_CLR(LS1x_INTC0_BASE)  =  bit; \
	    LS1x_INTC_IEN(LS1x_INTC0_BASE) &= ~bit; \
    } while (0)

//-------------------------------------------------------------------------------------------------
// TOY match format convert
//-------------------------------------------------------------------------------------------------
/*
 * 转换 struct tm 是否使用实际日期
 */
void normalize_tm(struct tm *tm, bool tm_format)
{
    if (tm_format)
    {
        if (tm->tm_year >= 1900)
        {
            tm->tm_year -= 1900;
            tm->tm_mon -= 1;
        }
    }
    else
    {
        if (tm->tm_year <  1900)
        {
            tm->tm_year += 1900;
            tm->tm_mon += 1;
        }
    }
}

/******************************************************************************
 *  TOY_date 格式:
 *   9:4   TOY_SEC      W  秒,   范围 0~59
 *   15:10 TOY_MIN      W  分,   范围 0~59
 *   20:16 TOY_HOUR     W  小时, 范围 0~23
 *   25:21 TOY_DAY      W  日,   范围 1~31
 *   31:26 TOY_MONTH    W  月,   范围 1~12
 */
static void tm_to_toy_datetime(struct tm *dt, unsigned int *hi, unsigned int *lo)
{
    if (dt->tm_year < 1900)
    {
        *lo = ((dt->tm_sec      & 0x3F) <<  4) |
              ((dt->tm_min      & 0x3F) << 10) |
              ((dt->tm_hour     & 0x1F) << 16) |
              ((dt->tm_mday     & 0x1F) << 21) |
             (((dt->tm_mon + 1) & 0x3F) << 26);
        *hi = dt->tm_year + 1900;
    }
    else
    {
        *lo = ((dt->tm_sec  & 0x3F) <<  4) |
              ((dt->tm_min  & 0x3F) << 10) |
              ((dt->tm_hour & 0x1F) << 16) |
              ((dt->tm_mday & 0x1F) << 21) |
              ((dt->tm_mon  & 0x3F) << 26);
        *hi = dt->tm_year;
    }
}

static void toy_datetime_to_tm(struct tm *dt, unsigned int hi, unsigned int lo)
{
    dt->tm_sec  = (lo >>  4) & 0x3F;
    dt->tm_min  = (lo >> 10) & 0x3F;
    dt->tm_hour = (lo >> 16) & 0x1F;
    dt->tm_mday = (lo >> 21) & 0x1F;
    dt->tm_mon  = (lo >> 26) & 0x3F;
    dt->tm_year =  hi;

    dt->tm_year -= 1900;
    dt->tm_mon  -= 1;
}

/******************************************************************************
 *  TOYMATCH 格式:
 *   31:26  YEAR   RW  年,   范围 0~63
 *   25:22  MONTH  RW  月,   范围 1~12
 *   21:17  DAY    RW  日,   范围 1~31
 *   16:12  HOUR   RW  小时, 范围 0~23
 *   11:6   MIN    RW  分,   范围 0~59
 *   5:0    SEC    RW  秒,   范围 0~59
 */
void ls1x_tm_to_toymatch(struct tm *dt, unsigned int *match)
{
    if (dt->tm_year < 1900)
    {
        *match = ((dt->tm_sec          & 0x3F) <<  0) |
                 ((dt->tm_min          & 0x3F) <<  6) |
                 ((dt->tm_hour         & 0x1F) << 12) |
                 ((dt->tm_mday         & 0x1F) << 17) |
                (((dt->tm_mon + 1)     & 0x0F) << 22) |
                (((dt->tm_year + 1900) & 0x3F) << 26);
    }
    else
    {
        *match = ((dt->tm_sec  & 0x3F) <<  0) |
                 ((dt->tm_min  & 0x3F) <<  6) |
                 ((dt->tm_hour & 0x1F) << 12) |
                 ((dt->tm_mday & 0x1F) << 17) |
                 ((dt->tm_mon  & 0x0F) << 22) |
                 ((dt->tm_year & 0x3F) << 26);
    }
}

void ls1x_toymatch_to_tm(struct tm *dt, unsigned int match)
{
    dt->tm_sec  = (match >>  0) & 0x3F;
    dt->tm_min  = (match >>  6) & 0x3F;
    dt->tm_hour = (match >> 12) & 0x1F;
    dt->tm_mday = (match >> 17) & 0x1F;
    dt->tm_mon  = (match >> 22) & 0x0F;
    dt->tm_year = (match >> 26) & 0x3F;

    dt->tm_year += 84;
    dt->tm_mon  -= 1;
}

//-------------------------------------------------------------------------------------------------
// Forward declaration
//-------------------------------------------------------------------------------------------------

static int LS1x_get_system_datetime(struct tm *dt);

static int LS1x_RTC_install_isr(int device, int index, unsigned int *irqNum, unsigned int *irqMask);

//-------------------------------------------------------------------------------------------------
// RTC operator
//-------------------------------------------------------------------------------------------------

/*
 * 毫秒数转换为 rtc_clocks
 */
static int convert_microseconds_to_rtcclocks(unsigned int ms)
{
    unsigned int trim;
    int clocks;

    trim = pRTC->hwRTC->rtctrim;
    if (0 == trim)
        trim = 1;

    clocks = ms * 32768 / 1000 / trim; 

    return clocks;
}

static int LS1x_rtc_set_trim(unsigned int trim)
{
    return 0;
}

/*
 * 启动 rtcmatch
 */
static int LS1x_rtc_start(int index, rtc_cfg_t * cfg)
{
    int clocks;
    unsigned int next_match, irqNum, irqMask;

    if (cfg == NULL)
        return -1;

    if (pRTC->rtcs[index].busy)
        return 1;

    clocks = convert_microseconds_to_rtcclocks(cfg->interval_ms);
    if (clocks <= 0)
        return -1;

    pRTC->rtcs[index].interval_ms = cfg->interval_ms;
    pRTC->rtcs[index].as_clocks   = clocks;
    pRTC->rtcs[index].isr         = cfg->isr;
    pRTC->rtcs[index].callback    = cfg->cb;

#if defined(OS_RTTHREAD)
    pRTC->rtcs[index].event = (rt_event_t)cfg->event;
#elif defined(OS_UCOS)
    pRTC->rtcs[index].event = (OS_FLAG_GRP *)cfg->event;
#elif defined(OS_FREERTOS)
    pRTC->rtcs[index].event = (EventGroupHandle_t)cfg->event;
#endif

    LS1x_RTC_install_isr(LS1X_RTC, index, &irqNum, &irqMask);   /* Install interrupt handler */

    next_match = pRTC->hwRTC->rtcread + clocks;
    pRTC->hwRTC->rtcmatch[index] = next_match;                  /* Set rtc match value */
    ls1x_sync();

    RTC_INT_ENABLE(irqMask);                                    /* 开中断 */
    
    pRTC->rtcs[index].busy = 1;
    pRTC->rtc_active_count++;
    
    return 0;
}

/*
 * 停止 rtcmatch
 */
static int LS1x_rtc_stop(int index)
{
    unsigned int irq_num;

    switch (index)
    {
        case 0: irq_num = LS1x_RTC0_IRQ; RTC_INT_DISABLE(INTC0_RTC0_BIT); break;
        case 1: irq_num = LS1x_RTC1_IRQ; RTC_INT_DISABLE(INTC0_RTC1_BIT); break;
        case 2: irq_num = LS1x_RTC2_IRQ; RTC_INT_DISABLE(INTC0_RTC2_BIT); break;
        default: return -1;
    }

    pRTC->rtcs[index].interval_ms = 0;
    pRTC->rtcs[index].as_clocks   = 0;
    pRTC->rtcs[index].isr         = NULL;
    pRTC->rtcs[index].callback    = NULL;

#if defined(OS_RTTHREAD)
    pRTC->rtcs[index].event = NULL;
#elif defined(OS_UCOS)
    pRTC->rtcs[index].event = NULL;
#elif defined(OS_FREERTOS)
    pRTC->rtcs[index].event = NULL;
#endif

    ls1x_remove_irq_handler(irq_num);               /* Uninstall interrupt handler */

    pRTC->hwRTC->rtcmatch[index] = 0;               /* Set rtc match zero */

    pRTC->rtcs[index].busy = 0;
    pRTC->rtc_active_count--;

    return 0;
}

//-------------------------------------------------------------------------------------------------
// TOY operator
//-------------------------------------------------------------------------------------------------

/*
 * 秒数转换为 toymatch time
 */
unsigned int ls1x_seconds_to_toymatch(unsigned int seconds)
{
    unsigned int match;
    struct tm dt;

    localtime_r((const time_t *)&seconds, &dt);	/* seconds 转换为 struct tm */
    ls1x_tm_to_toymatch(&dt, &match);       	/* struct tm 转换为 toymatch 日期 */

    return match;
}

/*
 * toymatch time 转换为秒数
 */
unsigned int ls1x_toymatch_to_seconds(unsigned int match)
{
    unsigned int seconds;
    struct tm dt;

    ls1x_toymatch_to_tm(&dt, match);        	/* toymatch 日期转换为 struct tm */
    seconds = mktime(&dt);                  	/* struct tm 转换为秒数 */

    return seconds;
}

static unsigned int LS1x_get_future_toymatch(unsigned int after_seconds)
{
    unsigned int next;
    struct tm dt;

    LS1x_get_system_datetime(&dt);          	/* 获取当前日期 struct tm */
    next = mktime(&dt);                     	/* struct tm 转换为秒数 */
    next += after_seconds;                  	/* 加上延迟秒数 */

    localtime_r((const time_t *)&next, &dt);	/* seconds 转换为 struct tm */
    ls1x_tm_to_toymatch(&dt, &next);        	/* struct tm 转换为 toymatch 日期 */

    return next;
}

/*
 * 设置系统日期
 */
static int LS1x_set_system_datetime(struct tm *dt)
{
    unsigned int hi, lo;
    
    if (dt == NULL)
        return -1;

    tm_to_toy_datetime(dt, &hi, &lo);
    pRTC->hwRTC->toywritelo = lo;
    pRTC->hwRTC->toywritehi = hi;
        
    return 0;
}

/*
 * 获取系统日期
 */
static int LS1x_get_system_datetime(struct tm *dt)
{
    unsigned int hi, lo;
    
    if (dt == NULL)
        return -1;
 
    lo = pRTC->hwRTC->toyreadlo;
    hi = pRTC->hwRTC->toyreadhi;
    toy_datetime_to_tm(dt, hi, lo);
    
    return 0;
}

static int LS1x_toy_set_trim(unsigned int trim)
{
    return 0;
}

/*
 * 启动 toymatch
 */
static int LS1x_toy_start(int index, rtc_cfg_t *cfg)
{
    unsigned int next_match, interval_sec, irqNum, irqMask;
    
    if (cfg == NULL)
        return -1;

    if (pRTC->toys[index].busy)
        return 1;

    interval_sec = cfg->interval_ms / 1000;         /* 转换为秒数 */

    if ((interval_sec < 1) && (!cfg->trig_datetime))
    {
        printk("invalid argument when start toy.");
        return -1;
    }
    
    if (cfg->trig_datetime)
    {
        ls1x_tm_to_toymatch(cfg->trig_datetime, &next_match);
        cfg->interval_ms = 0;       /* only once flag*/
    }
    else
    {
        next_match = LS1x_get_future_toymatch(interval_sec);
    }

    if (next_match == 0)
        return -1;

    pRTC->toys[index].interval_ms = cfg->interval_ms;
    pRTC->toys[index].as_clocks   = 0;
    pRTC->toys[index].isr         = cfg->isr;
    pRTC->toys[index].callback    = cfg->cb;

#if defined(OS_RTTHREAD)
    pRTC->toys[index].event = (rt_event_t)cfg->event;
#elif defined(OS_UCOS)
    pRTC->toys[index].event = (OS_FLAG_GRP *)cfg->event;
#elif defined(OS_FREERTOS)
    pRTC->toys[index].event = (EventGroupHandle_t)cfg->event;
#endif

    LS1x_RTC_install_isr(LS1X_TOY, index, &irqNum, &irqMask);   /* Install interrupt handler */

    pRTC->hwRTC->toymatch[index] = next_match;                  /* Set toy match value */
    ls1x_sync();

    RTC_INT_ENABLE(irqMask);                                    /* 开中断 */
    
    pRTC->toys[index].busy = 1;
    pRTC->toy_active_count++;

    return 0;
}

/*
 * 停止 toymatch
 */
static int LS1x_toy_stop(int index)
{
    unsigned int irq_num;

    switch (index)
    {
        case 0: irq_num = LS1x_TOY0_IRQ; RTC_INT_DISABLE(INTC0_TOY0_BIT); break;
        case 1: irq_num = LS1x_TOY1_IRQ; RTC_INT_DISABLE(INTC0_TOY1_BIT); break;
        case 2: irq_num = LS1x_TOY2_IRQ; RTC_INT_DISABLE(INTC0_TOY2_BIT); break;
        default: return -1;
    }

    ls1x_remove_irq_handler(irq_num);               /* Uninstall interrupt handler */

    pRTC->toys[index].interval_ms = 0;
    pRTC->toys[index].as_clocks   = 0;
    pRTC->toys[index].isr         = NULL;
    pRTC->toys[index].callback    = NULL;

#if defined(OS_RTTHREAD)
    pRTC->toys[index].event = NULL;
#elif defined(OS_UCOS)
    pRTC->toys[index].event = NULL;
#elif defined(OS_FREERTOS)
    pRTC->toys[index].event = NULL;
#endif

    pRTC->hwRTC->toymatch[index] = 0;               /* Set toy match zero */

    pRTC->toys[index].busy = 0;
    pRTC->toy_active_count--;

    return 0;
}

//-------------------------------------------------------------------------------------------------
// Interrupt
//-------------------------------------------------------------------------------------------------

/*
 * 中断处理程序
 */
static void LS1x_rtc_common_isr(int vector, void *arg)
{
    int device, index, stop = 0;
    unsigned int cur_match;
#if defined(OS_RTTHREAD)
    rt_event_t event = NULL;
#elif defined(OS_UCOS)
    OS_FLAG_GRP *event = NULL;
#elif defined(OS_FREERTOS)
    EventGroupHandle_t event = NULL;
#endif

    if (arg == NULL)
        return;
        
    device = (int)arg & 0xFF00;
    index  = (int)arg & 0x00FF;
    
    if (!(index & 0x03))
        return;

    index -= 1;
    
    switch (device)
    {
        case LS1X_RTC:
            cur_match = pRTC->hwRTC->rtcmatch[index];

            if (pRTC->rtcs[index].interval_ms > 0)              /* Set next rtc match */
            {
                pRTC->hwRTC->rtcmatch[index] = cur_match + pRTC->rtcs[index].as_clocks;
            }

            if (pRTC->rtcs[index].callback)                     /* 回调函数 */
            {
                (pRTC->rtcs[index].callback)((int)arg, cur_match, &stop);
                
                if ((stop) || (!pRTC->rtcs[index].interval_ms)) /* Stop it */
                {
                    LS1x_rtc_stop(index);
                }

                return;                                         /* ingore event */
            }

        #if BSP_USE_OS
            event = pRTC->rtcs[index].event;
        #endif

            break;

        case LS1X_TOY:
            cur_match = pRTC->hwRTC->toymatch[index];

            if (pRTC->toys[index].interval_ms >= 1000)          /* Set next toy match */
            {
                unsigned int seconds;
                seconds  = ls1x_toymatch_to_seconds(cur_match);
                seconds += pRTC->toys[index].interval_ms / 1000;
                seconds  = ls1x_seconds_to_toymatch(seconds);
                pRTC->hwRTC->toymatch[index] = seconds;
            }

            if (pRTC->toys[index].callback)                     /* 回调函数 */
            {
                (pRTC->toys[index].callback)((int)arg, cur_match, &stop);
                
                if ((stop) || (!pRTC->toys[index].interval_ms)) /* Stop it */
                {
                    LS1x_toy_stop(index);
                }

                return;                                         /* ingore event */
            }

         #if BSP_USE_OS
            event = pRTC->toys[index].event;
        #endif
            
            break;
    }

#if BSP_USE_OS
    /******************************************************
     * Timer Event of RTOS
     */
    if (event == NULL)
        return;

  #if defined(OS_RTTHREAD)

    rt_event_send(event, RTC_TIMER_EVENT);

  #elif defined(OS_UCOS)

    unsigned char err;
    OSFlagPost(event,
               (OS_FLAGS)RTC_TIMER_EVENT,
               OS_FLAG_SET,
               &err);

  #elif defined(OS_FREERTOS)

    BaseType_t xResult, xHigherPriorityTaskWoken = pdFALSE;
	xResult = xEventGroupSetBitsFromISR(event,
                                        RTC_TIMER_EVENT,
                                        &xHigherPriorityTaskWoken);
    if (xResult != pdPASS)  /* Was the message posted successfully? */
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  #endif
#endif // #if BSP_USE_OS
}

/*
 * parameters:
 *
 *  device: DEVICE_RTC or DEVICE_TOY
 *   index: 1~3
 */
static int LS1x_RTC_install_isr(int device, int index, unsigned int *irqNum, unsigned int *irqMask)
{
    unsigned int arg;
    irq_handler_t isr = NULL;

    *irqNum  = 0;
    *irqMask = 0;
    arg = (unsigned int)device | (index + 1);

    switch (device)
    {
        case LS1X_RTC:
            switch (index)
            {
                case 0: *irqNum = LS1x_RTC0_IRQ; *irqMask = INTC0_RTC0_BIT; break;
                case 1: *irqNum = LS1x_RTC1_IRQ; *irqMask = INTC0_RTC1_BIT; break;
                case 2: *irqNum = LS1x_RTC2_IRQ; *irqMask = INTC0_RTC2_BIT; break;
                default: return -1;
            }
            
            isr = pRTC->rtcs[index].isr;
            break;

        case LS1X_TOY:
            switch (index)
            {
                case 0: *irqNum = LS1x_TOY0_IRQ; *irqMask = INTC0_TOY0_BIT; break;
                case 1: *irqNum = LS1x_TOY1_IRQ; *irqMask = INTC0_TOY1_BIT; break;
                case 2: *irqNum = LS1x_TOY2_IRQ; *irqMask = INTC0_TOY2_BIT; break;
                default: return -1;
            }
            
            isr = pRTC->toys[index].isr;
            break;

        default:
            return -1;
    }

    RTC_INT_DISABLE(*irqMask);
    
    if (isr == NULL)
        ls1x_install_irq_handler(*irqNum, LS1x_rtc_common_isr, (void *)arg);
    else
        ls1x_install_irq_handler(*irqNum, isr, (void *)arg);

    // RTC_INT_ENABLE(*irqMask);

    return 0;
}

//-------------------------------------------------------------------------------------------------
// driver implement
//-------------------------------------------------------------------------------------------------

/*
 * parameters:
 *  dev:  NULL
 *  arg:  struct tm *, if not NULL then initialize datetime => toywrite
 */
STATIC_DRV int LS1x_RTC_initialize(void *dev, void *arg)
{
    int i;

    if (pRTC->initilized)
    {
        if (arg)                            /* 有初始化日期参数 */
        {
            LS1x_set_system_datetime((struct tm *)arg);
        }
        
        return 0;
    }
        
    memset((void *)pRTC, 0, sizeof(RTC_t));
    pRTC->hwRTC = (LS1x_rtc_regs_t *)LS1x_RTC_BASE;
    pRTC->rtc_active_count = 0;
    pRTC->toy_active_count = 0;
    
#if BSP_USE_OS

#endif

    pRTC->hwRTC->rtcctrl = 0;               /* 寄存器复位 */
    pRTC->hwRTC->toytrim = 0;
    pRTC->hwRTC->rtctrim = 0;

    for (i=0; i<3; i++)
    {
        pRTC->hwRTC->toymatch[i] = 0;
        pRTC->hwRTC->rtcmatch[i] = 0;
    }

    RTC_INT_INIT(INTC0_RTC0_BIT);           /* 中断置位初始化 */
    RTC_INT_INIT(INTC0_RTC1_BIT);
    RTC_INT_INIT(INTC0_RTC2_BIT);
    RTC_INT_INIT(INTC0_TOY0_BIT);
    RTC_INT_INIT(INTC0_TOY1_BIT);
    RTC_INT_INIT(INTC0_TOY2_BIT);

    if (arg)                                /* 有初始化日期参数 */
    {
        LS1x_set_system_datetime((struct tm *)arg);
    }
    
    pRTC->hwRTC->rtcwrite = 0;

    /**************************************************************************
     * 使能 RTC 和 TOY
     */
    pRTC->hwRTC->rtcctrl |= rtc_ctrl_ten | rtc_ctrl_btt |   /* Enable TOY with 32768 */
                            rtc_ctrl_ren | rtc_ctrl_brt |   /* Enable RTC with 32768 */
                            rtc_ctrl_e0;                    /* Enable OSC 32768 */

    delay_us(100);          

    pRTC->initilized = 1;
    
    return 0;
}

/*
 * parameters:
 *  dev:  DEVICE_XXX
 *  arg:  rtc_cfg_t *
 */
STATIC_DRV int LS1x_RTC_open(void *dev, void *arg)
{
    if (arg && (GET_INDEX(dev) & 0x03))
    {
        switch (GET_DEVICE(dev))
        {
            case LS1X_RTC: return LS1x_rtc_start(GET_INDEX(dev)-1, (rtc_cfg_t *)arg);
            case LS1X_TOY: return LS1x_toy_start(GET_INDEX(dev)-1, (rtc_cfg_t *)arg);
        }
    }

    return -1;
}

/*
 * parameters:
 *  dev:  DEVICE_XXX
 *  arg:  NULL
 */
STATIC_DRV int LS1x_RTC_close(void *dev, void *arg)
{
    if (GET_INDEX(dev) & 0x03)
    {
        switch (GET_DEVICE(dev))
        {
            case LS1X_RTC: return LS1x_rtc_stop(GET_INDEX(dev)-1);
            case LS1X_TOY: return LS1x_toy_stop(GET_INDEX(dev)-1);
        }
    }
    
    return -1;
}

/*
 * get current datetime <= toyread
 *
 * parameters:
 *  dev:  NULL
 *  buf:  struct tm *
 *  size: sizeof(struct tm)
 *  arg:  NULL
 */
STATIC_DRV int LS1x_RTC_read(void *dev, void *buf, int size, void *arg)
{
    if (buf && (size == sizeof(struct tm)))
    {
        if (LS1x_get_system_datetime((struct tm *)buf) == 0)
            return size;
        else
            return 0;
    }
    
    return -1;
}

/*
 * set current datetime => toywrite
 *
 * parameters:
 *  dev:  NULL
 *  buf:  struct tm *
 *  size: sizeof(struct tm)
 *  arg:  NULL
 */
STATIC_DRV int LS1x_RTC_write(void *dev, void *buf, int size, void *arg)
{
    if (buf && (size == sizeof(struct tm)))
    {
        if (LS1x_set_system_datetime((struct tm *)buf) == 0)
            return size;
        else
            return 0;
    }
    
    return -1;
}

/*
 * parameters:
 *  dev:  NULL or DEVICE_XXX
 *  arg:  if dev==NULL then do TRIM access
 *        else is as rtc_cfg_t *
 */
STATIC_DRV int LS1x_RTC_ioctl(void *dev, int cmd, void *arg)
{
    int rt = 0;

    switch (cmd)
    {
    	case IOCTL_SET_SYS_DATETIME:        // struct tm *
    		rt = LS1x_set_system_datetime((struct tm *)arg);
    		break;

    	case IOCTL_GET_SYS_DATETIME:        // struct tm *
    		rt = LS1x_get_system_datetime((struct tm *)arg);
    		break;

        /*
         * control rtc
         */
        case IOCTL_RTC_SET_TRIM:            // unsigned int *
            rt = LS1x_rtc_set_trim(*((unsigned int *)arg));
            break;

        case IOCTL_RTC_GET_TRIM:            // unsigned int *
            if (arg)
                *((unsigned int *)arg) = pRTC->hwRTC->rtctrim;
            else
                rt = -1;
            break;

        case IOCTL_RTCMATCH_START:          // rtc_cfg_t *
            if ((GET_DEVICE(dev) == LS1X_RTC) && (GET_INDEX(dev) & 0x03))
                rt = LS1x_rtc_start(GET_INDEX(dev)-1, (rtc_cfg_t *)arg);
            else
                rt = -1;
            break;

        case IOCTL_RTCMATCH_STOP:
            if ((GET_DEVICE(dev) == LS1X_RTC) && (GET_INDEX(dev) & 0x03))
                rt = LS1x_rtc_stop(GET_INDEX(dev)-1);
            else
                rt = -1;
            break;

        /*
         * control toy
         */
        case IOCTL_TOY_SET_TRIM:            // unsigned int *
            rt = LS1x_toy_set_trim(*((unsigned int *)arg));
            break;

        case IOCTL_TOY_GET_TRIM:            // unsigned int *
            if (arg)
                *((unsigned int *)arg) = pRTC->hwRTC->toytrim;
            else
                rt = -1;
            break;

        case IOCTL_TOYMATCH_START:          // rtc_cfg_t *
            if ((GET_DEVICE(dev) == LS1X_TOY) && (GET_INDEX(dev) & 0x03))
                rt = LS1x_toy_start(GET_INDEX(dev)-1, (rtc_cfg_t *)arg);
            else
                rt = -1;
            break;

        case IOCTL_TOYMATCH_STOP:
            if ((GET_DEVICE(dev) == LS1X_TOY) && (GET_INDEX(dev) & 0x03))
                rt = LS1x_toy_stop(GET_INDEX(dev)-1);
            else
                rt = -1;
            break;

        default:
            rt = -1;
            break;
    }
    
    return rt;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * RTC driver operators
 */
static driver_ops_t LS1x_RTC_drv_ops =
{
    .init_entry  = LS1x_RTC_initialize,
    .open_entry  = LS1x_RTC_open,
    .close_entry = LS1x_RTC_close,
    .read_entry  = LS1x_RTC_read,
    .write_entry = LS1x_RTC_write,
    .ioctl_entry = LS1x_RTC_ioctl,
};

driver_ops_t *ls1x_rtc_drv_ops = &LS1x_RTC_drv_ops;
#endif

/******************************************************************************
 * User API
 */
int ls1x_rtc_set_datetime(struct tm *dt)
{
    return LS1x_set_system_datetime(dt);
}

int ls1x_rtc_get_datetime(struct tm *dt)
{
    return LS1x_get_system_datetime(dt);
}

int ls1x_rtc_timer_start(unsigned device, rtc_cfg_t *cfg)
{
    return LS1x_RTC_open((void *)device, (void *)cfg);
}

int ls1x_rtc_timer_stop(unsigned device)
{
    return LS1x_RTC_close((void *)device, NULL);
}

#endif // #if defined(BSP_USE_RTC)

/*
 * @@ END
 */
 
 
