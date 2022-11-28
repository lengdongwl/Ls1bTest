/*
 * ls1x_rtc.h
 *
 * created: 2020/6/30
 * authour: 
 */

/*
 * LS1B: ����ʹ��
 *       1. ��д toywritehi/toywritelo��, һֱ��� rtc_ctrl_ts λ. 
 *          ��ʹ�ܺ�: ���Զ�������, ����ȷ�ؼ�ʱ; toymatch ��ʱ����ʹ��
 *       2. ��д rtcwrite��, һֱ��� rtc_ctrl_rs λ. 
 *          ��ʹ�ܺ�: rtcmatch ��������ʹ��
 *
 * LS1C: ����ʹ��. ����Ϊ: ��д toytrim/rtctrim ���������, �����ܷ�.
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
 * Virtual Sub-Device. ��ʱ���������豸
 */
#define DEVICE_RTCMATCH0       (LS1X_RTC | 1)
#define DEVICE_RTCMATCH1       (LS1X_RTC | 2)
#define DEVICE_RTCMATCH2       (LS1X_RTC | 3)

#define DEVICE_TOYMATCH0       (LS1X_TOY | 1)
#define DEVICE_TOYMATCH1       (LS1X_TOY | 2)
#define DEVICE_TOYMATCH2       (LS1X_TOY | 3)

//-----------------------------------------------------------------------------
// ��ʱ���жϻص�����
//-----------------------------------------------------------------------------

/*
 * ����:    device  RTC�������豸�������ж�
 *          match   ��ǰRTCMATCH����TOYMATCH�ļĴ���ֵ
 *          stop    �����*stop ������ֵ, �ö�ʱ����ֹͣ���ٹ���, ����ʱ�����Զ�����
 *                  ����interval_msֵ, �ȴ���һ�ζ�ʱ����ʱ��ֵ�����ж�.
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
 * RTC��ʼ��
 * ����:    dev     ���� NULL
 *          arg     ����: struct tm *. ����ò������� NULL, ��ֵ���ڳ�ʼ��RTCϵͳʱ��.
 *
 * ����:    0=�ɹ�
 */
int LS1x_RTC_initialize(void *dev, void *arg);

/*
 * ��RTC��ʱ��
 * ����:    dev     Ҫ�򿪵�RTC���豸 DEVICE_XXX
 *          arg     ����: rtc_cfg_t *, ��������RTC���豸�Ĺ���ģʽ������
 *
 * ����:    0=�ɹ�
 *
 * ˵��:    ���ʹ�õ���RTC���豸, �������ò���rtc_cfg_t��interval_msֵ, ��RTC��ʱ����interval_ms��ֵʱ,
 *          ������RTC��ʱ�ж�, ��ʱ�ж���Ӧ:
 *          1. �������������û��Զ����ж� isr(!=NULL), ����Ӧisr;
 *          2. ����Զ����ж� isr=NULL, ʹ��RTCĬ���ж�, ���жϵ���cb �ص��������û�������ʱ��Ӧ;
 *          3. ����Զ����ж� isr=NULL��cb=NULL, �����event����, RTCĬ���жϽ�����RTC_TIMER_EVENT�¼�.
 *
 *          ���ʹ�õ���TOY���豸, ����������interval_ms����(>1000), �÷���ʹ��RTC���豸һ��;
 *          ��interval_ms==0��trig_datetime!=NULLʱ, ��ʾTOY���豸���ڼ�ʱ�������δ��ʱ���ʱ�����ж�,
 *          �жϴ������̺�����һ��.
 *          ʹ��trig_datetime�������жϽ�����һ��.
 *
 *          interval_ms���ڼ�������жϲ���һֱ����; trig_datetime���ڵ�ʱ�����жϽ�����һ��.
 *
 */
int LS1x_RTC_open(void *dev, void *arg);

/*
 * �ر�RTC��ʱ��
 * ����:    dev     Ҫ�رյ�RTC���豸 DEVICE_XXX
 *          arg     NULL
 *
 * ����:    0=�ɹ�
 */
int LS1x_RTC_close(void *dev, void *arg);

/*
 * ��ȡ��ǰRTCʱ��
 * ����:    dev     NULL
 *          buf     ����: struct tm *, ���ڴ�Ŷ�ȡ��ʱ��ֵ
 *          size    ����: int, ��С=sizeof(struct tm)
 *          arg     NULL
 *
 * ����:    ��ȡ���ֽ���, ����Ϊsizeof(struct tm)
 */
int LS1x_RTC_read(void *dev, void *buf, int size, void *arg);

/*
 * ����RTCʱ��
 * ����:    dev     NULL
 *          buf     ����: struct tm *, ���ڴ�Ŵ�д���ʱ��ֵ
 *          size    ����: int, ��С=sizeof(struct tm)
 *          arg     NULL
 *
 * ����:    д����ֽ���, ����Ϊsizeof(struct tm)
 */
int LS1x_RTC_write(void *dev, void *buf, int size, void *arg);

/*
 * ����RTCʱ���豸
 * ����:    dev     NULL or DEVICE_XXX
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                             |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_SET_SYS_DATETIME          |   ����: truct tm *
 *                                          |   ��;: ����RTCϵͳʱ��ֵ
 *      ---------------------------------------------------------------------------------
 *          IOCTL_GET_SYS_DATETIME          |   ����: struct tm *
 *                                          |   ��;: ��ȡ��ǰRTCϵͳʱ��ֵ
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTL_RTC_SET_TRIM              |   ����: unsigned int *
 *                                          |   ��;: ����RTC��32768HZʱ�������Ƶֵ
 *      ---------------------------------------------------------------------------------
 *          IOCTL_RTC_GET_TRIM              |   ����: unsigned int *
 *                                          |   ��;: ��ȡRTC��32768HZʱ�������Ƶֵ
 *      ---------------------------------------------------------------------------------
 *          IOCTL_RTCMATCH_START            |   ����: rtc_cfg_t *, ����RTC��ʱ��
 *                                          |   dev==DEVICE_RTCMATCHx
 *      ---------------------------------------------------------------------------------
 *          IOCTL_RTCMATCH_STOP             |   ����: NULL, ֹͣRTC��ʱ��
 *                                          |   dev==DEVICE_RTCMATCHx
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTL_TOY_SET_TRIM              |   ����: unsigned int *
 *                                          |   ��;: ����TOY��32768HZʱ�������Ƶֵ
 *      ---------------------------------------------------------------------------------
 *          IOCTL_TOY_GET_TRIM              |   ����: unsigned int *
 *                                          |   ��;: ��ȡTOY��32768HZʱ�������Ƶֵ
 *      ---------------------------------------------------------------------------------
 *          IOCTL_TOYMATCH_START            |   ����: rtc_cfg_t *, ����TOY��ʱ��
 *                                          |   dev==DEVICE_TOYMATCHx
 *      ---------------------------------------------------------------------------------
 *          IOCTL_TOYMATCH_STOP             |   ����: NULL, ֹͣTOY��ʱ��
 *                                          |   dev==DEVICE_TOYMATCHx
 *      ---------------------------------------------------------------------------------
 *
 *
 * ����:    0=�ɹ�
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
 * ����RTCʱ��ֵ, �μ�ls1x_rtc_read()
 */
int ls1x_rtc_set_datetime(struct tm *dt);
/*
 * ��ȡ��ǰRTCʱ��, �μ�ls1x_rtc_write()
 */
int ls1x_rtc_get_datetime(struct tm *dt);

/*
 * device is Virtual Sub Device
 */
/*
 * ������ʱ��, �μ�ls1x_rtc_open()
 */
int ls1x_rtc_timer_start(unsigned device, rtc_cfg_t *cfg);
/*
 * �رն�ʱ��, �μ�ls1x_rtc_close()
 */
int ls1x_rtc_timer_stop(unsigned device);

/*
 * LS1x toymatch ���ڸ�ʽת��
 */
void ls1x_tm_to_toymatch(struct tm *dt, unsigned int *match);
void ls1x_toymatch_to_tm(struct tm *dt, unsigned int match);

unsigned int ls1x_seconds_to_toymatch(unsigned int seconds);
unsigned int ls1x_toymatch_to_seconds(unsigned int match);

/*
 * struct tm ���ڸ�ʽת��, +1900/-1900
 */
void normalize_tm(struct tm *tm, bool tm_format);

#ifdef __cplusplus
}
#endif

#endif // _LS1X_RTC_H

