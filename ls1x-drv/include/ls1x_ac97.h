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
 * AC97 ������֧�ֲ�������48000��wave��ʽ�ļ�
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
 * ����оƬ����
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
 * ����оƬ����
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
 * ��ʼ��AC97�豸
 * ����:    dev     NULL
 *          arg     NULL
 *
 * ����:    0=�ɹ�
 */
int LS1x_AC97_initialize(void *dev, void *arg);

/*
 * ��AC97�豸, ����ʼ����
 * ����:    dev     NULL
 *          arg     ����: ac97_param_t *, ���Ķ�ac97_param_t�����˵��
 *
 * ����:    0=�ɹ�
 */
int LS1x_AC97_open(void *dev, void *arg);

/*
 * �ر�AC97�豸, ��ֹͣ����
 * ����:    dev     NULL
 *          arg     ����: unsigned int, ��ֵ����ʱǿ��ֹͣAC97
 *
 * ����:    0=�ɹ�
 */
int LS1x_AC97_close(void *dev, void *arg);

/*
 * ��CAN�豸���Ϳ�������
 * ����:    dev     devCAN0/devCAN1
 *
 *      ---------------------------------------------------------------------------------
 *          cmd                         |   arg
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_POWER            |   NULL. ��AC97 ��ΧоƬ��Դ
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_RESET            |   NULL. ��λAC97�� ��ΧоƬ
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_SET_VOLUME       |   ����: unsigned int, �μ�unpack_vol_arg()
 *                                      |   ��;: ����AC97��CODECоƬ����
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_RECORD_GAIN      |   ����: unsigned int (& 0x1F1F)
 *                                      |   ��;: ����AC97��CODECоƬ¼������
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_RECORD_SEL       |   ����: unsigned int (& 0x0707)
 *                                      |   ��;: ����AC97��CODECоƬ¼��Դ
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_SET_REGISTER     |   ����: unsigned int, �μ�unpack_reg_arg()
 *                                      |   ��;: ����AC97��CODEC�Ĵ���ֵ
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_GET_REGISTER     |   ����: unsigned int *, �μ�unpack_reg_arg()
 *                                      |   ��;: ��ȡAC97��CODEC�Ĵ���ֵ
 *      ---------------------------------------------------------------------------------
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_START            |   NULL. ����AC97��ʼ����
 *      ---------------------------------------------------------------------------------
 *          IOCTL_AC97_STOP             |   NULL. ֹͣAC97����
 *      ---------------------------------------------------------------------------------
 *
 * ����:    0=�ɹ�
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
 * ��ʼAC97����
 */
int ls1x_ac97_play(const ac97_param_t *param);
/*
 * ��ʼAC97¼��
 */
int ls1x_ac97_record(const ac97_param_t *param);

/*
 * AC97�豸�Ƿ�æ. �û�����������������ǰ�����ж�AC97�Ƿ����ڹ���.
 */
int ls1x_ac97_busy(void);

#ifdef __cplusplus
}
#endif

#endif // _LS1X_AC97_H


