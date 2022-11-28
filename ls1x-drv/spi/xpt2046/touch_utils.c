/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * touchs creen_task.c
 *
 *  Created on: 2014-9-1
 *      Author: Bian
 */

/*
 * MENTION：XPT2046's PENIRQ is negative
 */

#include "bsp.h"

#if defined(BSP_USE_FB) && defined(XPT2046_DRV)

#if defined(LS1B)
#include "ls1b_gpio.h"
#define GET_TOUCH_DOWN()    (gpio_read(XPT2046_USE_GPIO_NUM) ? 0 : 1)
#elif defined(LS1C)
#include "ls1c_gpio.h"
#include "i2c/pca9557.h"
#define GET_TOUCH_DOWN()    get_touch_down(busI2C0)
#endif

#include "ls1x_i2c_bus.h"
#include "ls1x_spi_bus.h"
#include "ls1x_fb.h"
#include "spi/xpt2046.h"

#include "drv_os_priority.h"

/*
 * save calibrated value to SPI flash
 */
#if defined(W25X40_DRV)
#include "spi/w25x40.h"
#endif

/*
 * file to save calibrated value
 */
#if defined(USE_YAFFS2)
#include "../../yaffs2/port/ls1x_yaffs.h"
#include "../../yaffs2/direct/yaffsfs.h"
#elif defined(HAVE_FILESYSTEM)
#include "configini.h"
#endif

/*
 * touch screen click message quene
 */
#define TS_MESSAGE_MAX  16

#ifndef TOUCH_STK_SIZE
//#define TOUCH_STK_SIZE  4096
#endif

#if defined(OS_RTTHREAD)
#include "rtthread.h"
#define SLEEP           rt_thread_sleep
static  rt_thread_t     touch_thread;
#if (XPT2046_USE_GPIO_INT)
rt_event_t              touch_event;        // extern for "xpt2046.c"
#endif
#if (TOUCHSCREEN_USE_MESSAGE)
rt_mq_t                 touch_msg_queue;    // extern
#endif

#elif defined(OS_UCOS)
#include "os.h"
#define SLEEP           OSTimeDly
static  OS_STK          touch_stack[TOUCH_STK_SIZE];
#if (XPT2046_USE_GPIO_INT)
OS_FLAG_GRP            *touch_event;        // extern for "xpt2046.c"
#endif
#if (TOUCHSCREEN_USE_MESSAGE)
static void            *touch_msg_storage[TS_MESSAGE_MAX];
OS_EVENT               *touch_msg_queue;    // extern
#endif

#elif defined(OS_FREERTOS)
#include "FreeRTOS.h"
#include "event_groups.h"
#define SLEEP           vTaskDelay
#if (XPT2046_USE_GPIO_INT)
EventGroupHandle_t      touch_event;        // extern for "xpt2046.c"
#endif
#if (TOUCHSCREEN_USE_MESSAGE)
QueueHandle_t           touch_msg_queue;    // extern
#endif

#else // OS_NONE

#define SLEEP           delay_ms

#endif

/*
 * touch screen calibrate
 */
static int ts_cal_coords[7];    /* calibrate value from config file */

static int __pixelsx = 0;
static int __pixelsy = 0;

/**********************************************************
 * sort function
 **********************************************************/

static int sort_by_x(const void* a, const void *b)
{
	return (((ts_message_t *)a)->x - ((ts_message_t *)b)->x);
}

static int sort_by_y(const void* a, const void *b)
{
	return (((ts_message_t *)a)->y - ((ts_message_t *)b)->y);
}

/******************************************************************************
 * 触摸屏事件处理任务
 ******************************************************************************/

/*
 * read samples
 */
#define MAX_SAMPLES		16
#define MIN_SAMPLES		12

/*
 * 采样点
 */
static ts_message_t samp[MAX_SAMPLES];

static int read_xyz_coords(int *click_down)
{
	int readed = 0;

	/* clear the samp array */
	memset((void *)samp, 0, sizeof(ts_message_t)*MAX_SAMPLES);

	while (readed < MAX_SAMPLES)
	{
        *click_down = GET_TOUCH_DOWN();
		if (!(*click_down))		        /* check any click */
			break;

        ls1x_xpt2046_read((void *)busSPI0, (unsigned char *)samp, sizeof(samp), NULL);
 
		if ((samp[readed].x < 100) || (samp[readed].x > 4000) ||
			(samp[readed].y < 100) || (samp[readed].y > 4000) ||
			(samp[readed].z < 100) || (samp[readed].z > 4000))
		{
			break;
		}
		
		readed++;
	}

	return readed;
}

/*
 * 载入触摸屏校正参数
 */
int load_touch_calibrate_values(void)
{
	int	_xres=0, _yres=0;

	__pixelsx = fb_get_pixelsx();
	__pixelsy = fb_get_pixelsy();

#if defined(W25X40_DRV)
    {
        int i, savedcoords[9];

        load_touchscreen_calibrate_values_from_w25x40(savedcoords, 9);
        for (i=0; i<7; i++)
            ts_cal_coords[i] = savedcoords[i];
        _xres = savedcoords[7];
        _yres = savedcoords[8];
    }
#endif

#if defined(USE_YAFFS2)
    if ((ts_cal_coords[6] <= 0) && yaffs_is_running())
    {
        int fd = yaffs_open(TOUCH_CALIBRATE_DATFILE, O_RDONLY, 0777);

        if (fd >= 0)
        {
            yaffs_read(fd, (void *)ts_cal_coords, 7 * sizeof(int));
            yaffs_read(fd, (char *)&_xres, 4);
            yaffs_read(fd, (char *)&_yres, 4);
            yaffs_close(fd);
        }
    }
#endif

    /*
     * TODO 判断分辨率是否一致...
     */
     
    return 0;
}

#define CALIBRATE_TOUCH_VALUES(_x, _y) \
	if (ts_cal_coords[6] > 0) { \
        _x = ((int64_t)ts_cal_coords[0] + (int64_t)ts_cal_coords[1]*(_x) + \
		      (int64_t)ts_cal_coords[2]*(_y)) / ts_cal_coords[6]; \
	    _y = ((int64_t)ts_cal_coords[3] + (int64_t)ts_cal_coords[4]*(_x) + \
		      (int64_t)ts_cal_coords[5]*(_y)) / ts_cal_coords[6]; \
    }

/*
 * 只有 RTOS 下才可以使用任务
 */
#if BSP_USE_OS

/******************************************************************************
 * task to wait touch screen click
 */
static void touchscreen_click_task(void *arg)
{
    SLEEP(300);

    load_touch_calibrate_values();

    // SLEEP(200);

    /**************************************************************************
     * 任务开始, 200 毫秒检测一次
     */
    DBG_OUT("touchscreen_click_task started.\r\n");
    
	while (1)
	{
		int x=0, y=0, z=0, click_down = 0;

		SLEEP(200);             /* check click event per 200 ms */

#if (XPT2046_USE_GPIO_INT)
    #if defined(OS_RTTHREAD)
    
		unsigned int recv = 0;
        rt_event_recv(touch_event,
                      TOUCH_CLICK_EVENT,
                      RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER,
                      &recv);

    #elif defined(OS_UCOS)

        unsigned char err;
        unsigned short recv;
        recv = OSFlagPend(touch_event,
                          (OS_FLAGS)TOUCH_CLICK_EVENT,  /* 触摸事件 */
                          OS_FLAG_WAIT_SET_ALL |        /* 触摸事件置1时有效，否则任务挂在这里 */
                          OS_FLAG_CONSUME,              /* 清除指定事件标志位 */
                          0,                            /* 0=无限等待，直到收到信号为止 */
                          &err);

    #elif defined(OS_FREERTOS)

        unsigned int recv;
        recv = xEventGroupWaitBits(touch_event,         /* 事件对象句柄 */
                                   TOUCH_CLICK_EVENT,   /* 接收事件 */
                                   pdTRUE,              /* 退出时清除事件位 */
                                   pdTRUE,              /* 满足感兴趣的所有事件 */
                                   portMAX_DELAY);      /* 指定超时事件, 一直等 */

    #endif

		if (recv != TOUCH_CLICK_EVENT)
		{
			ls1x_enable_gpio_interrupt(XPT2046_USE_GPIO_NUM);
			continue;
		}

		ls1x_disable_gpio_interrupt(XPT2046_USE_GPIO_NUM);

#else

        click_down = GET_TOUCH_DOWN();
		if (!click_down)		        /* check any click */
			continue;

#endif // #if (XPT2046_USE_GPIO_INT)

		SLEEP(100);		                /* wait ad-circuit be stable */
		
        click_down = GET_TOUCH_DOWN();
		if (!click_down)
		{
#if (XPT2046_USE_GPIO_INT)
			ls1x_enable_gpio_interrupt(XPT2046_USE_GPIO_NUM);
#endif
			continue;
		}

		/*
		 * 每个100ms读一次数据, 检测 MOVE 或者 CLICKUP
		 */
		while (click_down)
		{
			int readed;

			/* read xyz coords from xpt2046
			 */
			readed = read_xyz_coords(&click_down);

			/* calculate readed coords
			 */
			if (readed >= MIN_SAMPLES)
			{
				int middle = readed / 2;

				/* z: the last one */
				z = samp[readed-1].z;

				/* sort x coordinates */
				qsort(samp, readed, sizeof(ts_message_t), sort_by_x);
				if (readed & 1)
					x = samp[middle].x;
				else
					x = (samp[middle-1].x + samp[middle].x) / 2;

				/* sort y coordinates */
				qsort(samp, readed, sizeof(ts_message_t), sort_by_y);
				if (readed & 1)
					y = samp[middle].y;
				else
					y = (samp[middle-1].y + samp[middle].y) / 2;

                /* calibrate the touch coordinates */
				CALIBRATE_TOUCH_VALUES(x, y);
				
				DBG_OUT("touch raw at: x=%i, y=%i, z=%i\r\n", x, y, z);
			}

            if ((x > 0) && (x < __pixelsx) && (y > 0) && (y < __pixelsy))
            {
#if (TOUCHSCREEN_USE_MESSAGE)
			    if (touch_msg_queue)
                {
				    ts_message_t msg, *pmsg = &msg;

				    msg.z = z;
				    msg.x = (uint16_t)x;
				    msg.y = (uint16_t)y;
				
    #if defined(OS_RTTHREAD)
    
				    if (rt_mq_send(touch_msg_queue, (const void *)&msg, sizeof(ts_message_t)))
				    {
				        /* if send fail, clear then resend again
                         */
                        rt_mq_control(touch_msg_queue, RT_IPC_CMD_RESET, NULL);
                        rt_mq_send(touch_msg_queue, (const void *)&msg, sizeof(ts_message_t));
				    }
				    
    #elif defined(OS_UCOS)

                    if (OSQPost(touch_msg_queue, (void *)&msg) == OS_ERR_MBOX_FULL)
                    {
                        unsigned char err;
                        OSQDel(touch_msg_queue, OS_DEL_NO_PEND, &err);
                        OSQPost(touch_msg_queue, (void *)&msg);
                    }

    #elif defined(OS_FREERTOS)

                    if (xQueueSend(touch_msg_queue, (void *)&pmsg, (TickType_t)0) == errQUEUE_FULL)
                    {
                        xQueueReset(touch_msg_queue);
                        xQueueSend(touch_msg_queue, (void *)&pmsg, (TickType_t)0);
                    }
    #endif // RTOS
			    }
			    
#endif // #if (TOUCHSCREEN_USE_MESSAGE)

                /*
                 * touch_callback_t 触摸回调函数
                 */
                if (arg)
                {
                    touch_callback_t callback = (touch_callback_t)arg;
                    callback(x, y);
                }

            }

            break;
		}

#if (XPT2046_USE_GPIO_INT)
		ls1x_enable_gpio_interrupt(XPT2046_USE_GPIO_NUM);
#endif
	}
}

/******************************************************************************
 * 启动触摸屏任务
 */
static int is_touchscreen_task_started = 0;

int start_touchscreen_task(touch_callback_t cb)
{
    if (is_touchscreen_task_started)
        return 0;

#if (TOUCHSCREEN_USE_MESSAGE)
	/*
     * 创建消息队列
     */
  #if defined(OS_RTTHREAD)
    touch_msg_queue = rt_mq_create("touchmsgqueue",
                                   sizeof(ts_message_t),
                                   TS_MESSAGE_MAX,
                                   RT_IPC_FLAG_FIFO);

  #elif defined(OS_UCOS)
    touch_msg_queue = OSQCreate(touch_msg_storage, TS_MESSAGE_MAX);
    
  #elif defined(OS_FREERTOS)
    touch_msg_queue = xQueueCreate(TS_MESSAGE_MAX, sizeof(ts_message_t));
    
  #endif

	if (touch_msg_queue == NULL)
	{
		printk("create touchscreen message quene fail!\r\n");
		return -1;
	}

#endif

	/*
     * 创建触摸事件
     */
#if (XPT2046_USE_GPIO_INT)
  #if defined(OS_RTTHREAD)

    touch_event = rt_event_create("touchevent", RT_IPC_FLAG_FIFO);

  #elif defined(OS_UCOS)
    unsigned char err;
    touch_event = OSFlagCreate(0, &err);

  #elif defined(OS_FREERTOS)
    touch_event = xEventGroupCreate();

  #endif
  
    if (touch_event == NULL)
    {
		printk("create touchscreen click event fail!\r\n");
    #if (TOUCHSCREEN_USE_MESSAGE) && defined(OS_RTTHREAD)
		rt_mq_delete(touch_msg_queue);
    #endif
		return -1;
    }
    
#endif

    /*
     * 建触摸屏任务
     */
#if defined(OS_RTTHREAD)

    touch_thread = rt_thread_create("touchthread",
                                    touchscreen_click_task,
                                    (void *)cb,         // arg
                                    TOUCH_STK_SIZE*4,   // statck size
                                    TOUCH_TASK_PRIO,    // priority
                                    TOUCH_TASK_SLICE);  // slice ticks

    if (touch_thread == NULL)
    {
        printk("create touchscreen thread fail!\r\n");
#if (TOUCHSCREEN_USE_MESSAGE)
		rt_mq_delete(touch_msg_queue);
#endif
#if (XPT2046_USE_GPIO_INT)
        rt_event_delete(touch_event);
#endif
		return -1;
	}

	rt_thread_startup(touch_thread);

#elif defined(OS_UCOS)

    OSTaskCreate(touchscreen_click_task,
    			 (void *)cb,         // arg
        #if OS_STK_GROWTH == 1
                 (void *)&touch_stack[TOUCH_STK_SIZE - 1],
        #else
                 (void *)&touch_stack[0],
        #endif
                 TOUCH_TASK_PRIO);

#elif defined(OS_FREERTOS)

    xTaskCreate(touchscreen_click_task,
                "touchtask",
                TOUCH_STK_SIZE,
				(void *)cb,         // arg
                TOUCH_TASK_PRIO,
                NULL);

#endif

    is_touchscreen_task_started = 1;
	return 0;
}

/******************************************************************************
 * 停止触摸屏任务
 */
int stop_touchscreen_task(void)
{
	if (!is_touchscreen_task_started)
		return 0;

#if defined(OS_RTTHREAD)
	if (touch_thread)
	{
	    rt_thread_delete(touch_thread);
		touch_thread = NULL;
	}

  #if (TOUCHSCREEN_USE_MESSAGE)
    if (touch_msg_queue)
    {
		rt_mq_delete(touch_msg_queue);
		touch_msg_queue = NULL;
    }
  #endif

  #if (XPT2046_USE_GPIO_INT)
	if (touch_event)
    {
        rt_event_delete(touch_event);
        touch_event = NULL;
    }
  #endif

#elif defined(OS_UCOS)

#elif defined(OS_FREERTOS)

#endif

	return 0;
}

#else // #if BSP_USE_OS

/*
 * 裸机模式下读取触摸时间
 */
static int getxy(int *x, int *y, int blocking);

int bare_get_touch_point(int *x, int *y)
{
    static int bfirst = 1;
    int X, Y;
 
    if ((ts_cal_coords[6] == 0) && bfirst)  /* 还没有载入初始化数据 */
    {
        load_touch_calibrate_values();
        bfirst = 0;
    }

    if (getxy(&X, &Y, 0) != 0)              /* 读取触摸坐标, nonblocking */
        return -1;
    
    CALIBRATE_TOUCH_VALUES(X, Y);           /* calibrate the touch coordinates */

    DBG_OUT("touch at: x=%i, y=%i\r\n", X, Y);

    if ((X > 0) && (X < __pixelsx) && (Y > 0) && (Y < __pixelsy))
    {
        *x = X;
        *y = Y;
        return 0;
    }

    return -1;
}

#endif // #if BSP_USE_OS

/******************************************************************************
 * 触摸屏校正
 ******************************************************************************/

static int cal_palette[] =
{
	0x000000, 0xffe080, 0xffffff, 0xe0c0a0
};

#define CAL_NR_COLORS (sizeof(cal_palette)/sizeof(cal_palette[0]))

typedef struct
{
	int x[5], xfb[5];
	int y[5], yfb[5];
	int a[7];
} ts_calibration_t;

static int getxy(int *x, int *y, int blocking)
{
	int readed, middle, click_down = 0;

	while (1)
	{
	    click_down = GET_TOUCH_DOWN();  
	    
        if (!click_down)                /* exists click ? */
		{
		    if (!blocking)              /* return immediately */
                break;
        }
        else
        {
            delay_ms(100);              /* wait adc be stable */
            
		    /*
             * read xyz coords from xpt2046
		     */
		    readed = read_xyz_coords(&click_down);
		    if ((!click_down) || (readed >= MIN_SAMPLES))
			    break;
		}
		
		delay_ms(100);
	}

    if (!click_down)
        return -1;
        
	middle = readed/2;

	if (x)
	{
		qsort((void *)samp, readed, sizeof(ts_message_t), sort_by_x);

		if (readed & 1)
			*x = samp[middle].x;
		else
			*x = (samp[middle-1].x + samp[middle].x) / 2;
	}

	if (y)
	{
		qsort((void *)samp, readed, sizeof(ts_message_t), sort_by_y);

		if (readed & 1)
			*y = samp[middle].y;
		else
			*y = (samp[middle-1].y + samp[middle].y) / 2;
	}
	
	return 0;
}

#define NR_STEPS	10

static void get_sample(ts_calibration_t *cal, int idx, int x, int y, char *name)
{
	static int last_x = -1, last_y;

#if (XPT2046_USE_GPIO_INT)
	ls1x_disable_gpio_interrupt(XPT2046_USE_GPIO_NUM);	    /* 关中断 */
#endif

	if (last_x != -1)
	{
		int dx = ((x - last_x) << 16) / NR_STEPS;
		int dy = ((y - last_y) << 16) / NR_STEPS;
		int i;
		last_x <<= 16;
		last_y <<= 16;
		for (i = 0; i < NR_STEPS; i++)
		{
			fb_put_cross(last_x >> 16, last_y >> 16, 2 | XORMODE);
			//usleep(1000);
			delay_us(1000);
			fb_put_cross(last_x >> 16, last_y >> 16, 2 | XORMODE);
			last_x += dx;
			last_y += dy;
		}
	}

	fb_put_cross(x, y, 2 | XORMODE);
	getxy(&cal->x[idx], &cal->y[idx], 1);
	fb_put_cross(x, y, 2 | XORMODE);

	last_x = cal->xfb[idx] = x;
	last_y = cal->yfb[idx] = y;

	printk("%s : X = %4d Y = %4d\n", name, cal->x[idx], cal->y[idx]);

#if (XPT2046_USE_GPIO_INT)
	ls1x_enable_gpio_interrupt(XPT2046_USE_GPIO_NUM);   /* 开中断 */
#endif

    delay_ms(500);       /* delay invoid last click */
}

static int perform_calibration(ts_calibration_t *cal)
{
	int   j;
	float n, x, y, x2, y2, xy, z, zx, zy;
	float det, a, b, c, e, f, i;
	float f0, f1, f2, scaling = 65535.0;

	/* Get sums for matrix */
	n = x = y = x2 = y2 = xy = 0;
	for (j=0; j<5; j++)
	{
		n  += 1.0;
		x  += (float)(cal->x[j]);
		y  += (float)(cal->y[j]);
		x2 += (float)(cal->x[j]*cal->x[j]);
		y2 += (float)(cal->y[j]*cal->y[j]);
		xy += (float)(cal->x[j]*cal->y[j]);
	}

	/*  Get determinant of matrix -- check if determinant is too small */
	det = n*(x2*y2 - xy*xy) + x*(xy*y - x*y2) + y*(x*xy - y*x2);
	if (det < 0.1 && det > -0.1)
	{
		printk("ts_calibrate: determinant is too small -- %f\n", det);
		return 0;
	}

	/* Get elements of inverse matrix */
	a = (x2 * y2 - xy * xy) / det;
	b = (xy *  y -  x * y2) / det;
	c = ( x * xy -  y * x2) / det;
	e = ( n * y2 -  y *  y) / det;
	f = ( x *  y -  n * xy) / det;
	i = ( n * x2 -  x *  x) / det;

	/* Get sums for x calibration */
	z = zx = zy = 0;
	for (j=0; j<5; j++)
	{
		z  += (float)(cal->xfb[j]);
		zx += (float)(cal->xfb[j]*cal->x[j]);
		zy += (float)(cal->xfb[j]*cal->y[j]);
	}

	/* Now multiply out to get the calibration for framebuffer x coord */
	f0 = a*z + b*zx + c*zy;
    f1 = b*z + e*zx + f*zy;
    f2 = c*z + f*zx + i*zy;
	cal->a[0] = (int)(f0 * scaling);
	cal->a[1] = (int)(f1 * scaling);
	cal->a[2] = (int)(f2 * scaling);

	printk("x: %f %f %f\n", f0, f1, f2);

	/* Get sums for y calibration */
	z = zx = zy = 0;
	for (j=0; j<5; j++)
	{
		z  += (float)(cal->yfb[j]);
		zx += (float)(cal->yfb[j]*cal->x[j]);
		zy += (float)(cal->yfb[j]*cal->y[j]);
	}

	/* Now multiply out to get the calibration for framebuffer y coord */
	f0 = a*z + b*zx + c*zy;
    f1 = b*z + e*zx + f*zy;
    f2 = c*z + f*zx + i*zy;
	cal->a[3] = (int)(f0 * scaling);
	cal->a[4] = (int)(f1 * scaling);
	cal->a[5] = (int)(f2 * scaling);

	printk("y: %f %f %f\n", f0, f1, f2);

	/* If we got here, we're OK, so assign scaling to a[6] and return */
	cal->a[6] = (int)scaling;

	for (j=0; j<5; j++)
	{
		x = (cal->a[1]*cal->x[j] + cal->a[2]*cal->y[j] + cal->a[0]) / cal->a[6];
		y = (cal->a[4]*cal->x[j] + cal->a[5]*cal->y[j] + cal->a[3]) / cal->a[6];

		printk("after calibrate, x=%i, y=%i\r\n", (int)x, (int)y);
	}

	return 1;
}

/*
 * 触摸屏校准
 */
extern int fb_is_opened(void);

int do_touchscreen_calibrate(void)
{
	ts_calibration_t cal;
	int i, bas, opened;

    opened = fb_is_opened();
    if (!opened)
    {
	   if (fb_open() != 0)
		  return -1;
    }

	for (i=0; i<CAL_NR_COLORS; i++)
		fb_set_color(i, cal_palette[i]);

	/*
	 * FIXME initialize __pixelsx & __pixelsy
	 */
	__pixelsx = fb_get_pixelsx();
	__pixelsy = fb_get_pixelsy();

	fb_put_string_center(__pixelsx/2, __pixelsy/4, "TSLIB calibration utility", 1);
	fb_put_string_center(__pixelsx/2, __pixelsy/4+20, "Touch crosshair to calibrate", 2);

	printk("xres = %u, yres = %u\n", (int)__pixelsx, (int)__pixelsy);

	bas = __pixelsx < __pixelsy ? __pixelsx : __pixelsy;
	bas /= 10;
	get_sample(&cal, 0, bas,           bas,           "Top left ");
	get_sample(&cal, 1, __pixelsx-bas, bas,           "Top right");
	get_sample(&cal, 2, __pixelsx-bas, __pixelsy-bas, "Bot right");
	get_sample(&cal, 3, bas,           __pixelsy-bas, "Bot left ");
	get_sample(&cal, 4, __pixelsx/2,   __pixelsy/2,   "Center   ");

	if (perform_calibration(&cal))
	{
	    int saved_ok = 0;

#if defined(W25X40_DRV)
        {
            int savedcoords[9];

            for (i=0; i<7; i++)
                savedcoords[i] = cal.a[i];
            savedcoords[7] = __pixelsx;
            savedcoords[8] = __pixelsy;

            save_touchscreen_calibrate_values_to_w25x40(savedcoords, 9);

            saved_ok = 1;
        }
#endif

#if defined(USE_YAFFS2)
        if (yaffs_is_running())
        {
            yaffs_DIR *cal_dir = yaffs_opendir(TOUCH_CALIBRATE_DIR);

            if (!cal_dir)
                yaffs_mkdir(TOUCH_CALIBRATE_DIR, S_IFDIR);
            else
                yaffs_closedir(cal_dir);

            int fd = yaffs_open(TOUCH_CALIBRATE_DATFILE, 0100 /*O_CREAT*/ | O_RDWR, 0777);

            if (fd >= 0)
            {
                yaffs_ftruncate(fd, 0);
                yaffs_write(fd, (const void *)cal.a, 7 * sizeof(int));
                yaffs_write(fd, (const void *)&__pixelsx, 4);
                yaffs_write(fd, (const void *)&__pixelsy, 4);
                yaffs_flush(fd);
                yaffs_close(fd);
                
                saved_ok = 1;
            }
        }
#endif

        if (saved_ok)
        {
            printk ("\r\nCalibration constants: \r\n");
	        for (i=0; i<7; i++)
		        printk("%i \r\n", cal.a[i]);
            printk("\r\nCalibration saved successful.\r\n");
        }
        else
            printk("\r\nCalibration saved fail!\r\n");
            
        /*
         * copy to ts_cal_coords
         */
        for (i=0; i<7; i++)
        {
            ts_cal_coords[i] = cal.a[i];
        }

        i = 0;
	}
	else
	{
		printk("Calibration failed.\n");
		i = -1;
	}

    if (!opened)
        fb_close();

	return i;
}

#endif

