/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 *  LS1X AC97 Driver
 *
 *  Author: Bian, 2021/3/20
 */
 
/*
 * AC97 驱动仅支持采样速率48000的wave格式文件
 */
 
#ifndef _LS1X_AC97_H
#define _LS1X_AC97_H

#ifdef __cplusplus
extern "C" {
#endif

//-----------------------------------------------------------------------------
// AC97 open() arg
//-----------------------------------------------------------------------------

typedef struct ac97_param
{
    unsigned char *buffer;          /* raw music data buffer */
    int buf_length;                 /* buffer length */
    int num_channels;               /* 1=mono;   2=stereo */
    int sample_bits;                /* 8=1byte;  16=2bytes */
    int flag;                       /* 1=record; 2=play */
} ac97_param_t;

//-----------------------------------------------------------------------------
// AC97 ioctl() command & arg
//-----------------------------------------------------------------------------

/******************************************************************************
 * 控制芯片参数
 */
/*
 * See "codec_alc655.h"
 */
#define IOCTL_AC97_POWER            0x01    /* arg !=0 then power on, else power off */
#define IOCTL_AC97_RESET            0x02    /* ac97 & codec chip reset */

/*
 * Set volume for play or record. Arg is as unsigned int.
 * Low 2 bytes is CODEC_XXX_VOL register, high 2 bytes is volume to be set.
 * Zero should be set mute
 */
#define IOCTL_AC97_SET_VOLUME       0x05

/*
 * Set gain for record. Arg is as unsigned int to be set
 */
#define IOCTL_AC97_RECORD_GAIN      0x06

/*
 * Set source for record. Arg is as unsigned int to be set
 */
#define IOCTL_AC97_RECORD_SEL       0x07

/*
 * Set codec chip register generally. Arg is as unsigned int.
 * Low 2 bytes is CODEC_XXX register, High 2 bytes is volue to be set.
 */
#define IOCTL_AC97_SET_REGISTER     0x0A
#define IOCTL_AC97_GET_REGISTER     0x0B    /* arg is unsigned int* */

/******************************************************************************
 * 控制芯片工作
 */
/*
 * Prepare AC97 to play or record
 */
#define IOCTL_AC97_START            0x10
#define IOCTL_AC97_STOP             0x40

//-----------------------------------------------------------------------------
// AC97 driver operators
//-----------------------------------------------------------------------------

#include "ls1x_io.h"

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_ac97_drv_ops;

#define ls1x_ac97_init(ac97, arg)             ls1x_ac97_drv_ops->init_entry(ac97, arg)
#define ls1x_ac97_open(ac97, arg)             ls1x_ac97_drv_ops->open_entry(ac97, arg)
#define ls1x_ac97_close(ac97, arg)            ls1x_ac97_drv_ops->close_entry(ac97, arg)
#define ls1x_ac97_ioctl(ac97, cmd, arg)       ls1x_ac97_drv_ops->ioctl_entry(ac97, cmd, arg)

#else

/*
 * 初始化AC97设备
 * 参数:    dev     NULL
 *          arg     NULL
 *
 * 返回:    0=成功
 */
int LS1x_AC97_initialize(void *dev, void *arg);

/*
 * 打开AC97设备, 并开始工作
 * 参数:    dev     NULL
 *          arg     类型: ac97_param_t *, 请阅读ac97_param_t域变量说明
 *
 * 返回:    0=成功
 */
int LS1x_AC97_open(void *dev, void *arg);

/*
 * 关闭AC97设备, 并停止工作
 * 参数:    dev     NULL
 *          arg     类型: unsigned int, 该值非零时强制停止AC97
 *
 * 返回:    0=成功
 */
int LS1x_AC97_close(void *dev, void *arg);

/*
 * 向CAN设备发送控制命令
 * 参数:    dev     devCAN0/devCAN1
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                         |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_POWER            |   NULL. 打开AC97 外围芯片电源
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_RESET            |   NULL. 复位AC97及 外围芯片
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_SET_VOLUME       |   类型: unsigned int, 参见unpack_vol_arg()
 *                                      |   用途: 设置AC97的CODEC芯片音量
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_RECORD_GAIN      |   类型: unsigned int (& 0x1F1F)
 *                                      |   用途: 设置AC97的CODEC芯片录音增益
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_RECORD_SEL       |   类型: unsigned int (& 0x0707)
 *                                      |   用途: 设置AC97的CODEC芯片录音源
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_SET_REGISTER     |   类型: unsigned int, 参见unpack_reg_arg()
 *                                      |   用途: 设置AC97的CODEC寄存器值
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_GET_REGISTER     |   类型: unsigned int *, 参见unpack_reg_arg()
 *                                      |   用途: 读取AC97的CODEC寄存器值
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_START            |   NULL. 启动AC97开始工作
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_STOP             |   NULL. 停止AC97工作
 *      ---------------------------------------------------------------------------------
 *
 * 返回:    0=成功
 */
int LS1x_AC97_ioctl(void *dev, int cmd, void *arg);

#define ls1x_ac97_init(ac97, arg)             LS1x_AC97_initialize(ac97, arg)
#define ls1x_ac97_open(ac97, arg)             LS1x_AC97_open(ac97, arg)
#define ls1x_ac97_close(ac97, arg)            LS1x_AC97_close(ac97, arg)
#define ls1x_ac97_ioctl(ac97, cmd, arg)       LS1x_AC97_ioctl(ac97, cmd, arg)

#endif

/*
 * for RT-Thread
 */
#if defined(OS_RTTHREAD)
#define LS1x_AC97_DEVICE_NAME   "ac97"
#endif

//-----------------------------------------------------------------------------
// User API
//-----------------------------------------------------------------------------

/*
 * 开始AC97放音
 */
int ls1x_ac97_play(const ac97_param_t *param);
/*
 * 开始AC97录音
 */
int ls1x_ac97_record(const ac97_param_t *param);

/*
 * AC97设备是否忙. 用户调用上面两个函数前必须判断AC97是否正在工作.
 */
int ls1x_ac97_busy(void);

#ifdef __cplusplus
}
#endif

#endif // _LS1X_AC97_H


