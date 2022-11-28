/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * font_desc.h
 *
 *  Created on: 2015-1-19
 *      Author: Bian
 *
 *  ϵͳ�ַ���
 *
 */

#ifndef _FONT_DESC_H_
#define _FONT_DESC_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include "bsp.h"

/*
 * �����͵��������ַ���
 */
#if (BSP_NEED_FONTX > 0)
#define HAS_CHARSET_X       1
#else
#define HAS_CHARSET_X       0
#endif

#define HAS_CHARSET_16      1

/*
 * ���ĺ������ֿ�
 */
#define HAS_CHINESE_FONT    1
#define HAS_ASCII_FONT      1

typedef struct tag_font_desc
{
	char  			name[16];            	/* �������� */
	unsigned char  *data;                	/* ���� */
    int       		is_asc;
    int       		intialized;          	/* �Ƿ��ʼ�� */
    unsigned  		num_faces;           	/* �������� */
    int       		cur_face;            	/* ��ǰ���� */
	unsigned   		width;               	/* �ַ���� */
	unsigned  		height;              	/* �ַ��߶� */
    unsigned 		scalable;            	/* ��������, ������ */

    void  (*load_font)(const unsigned char *filename);				/* �ֿ�װ���ڴ� */
    void  (*release_font)(void);									/* �ͷ��ڴ� */
	int   (*char_in_this)(const unsigned char *str);				/* �ַ��Ƿ�������ֿ� */
	int   (*get_char_size)(const unsigned char *str);				/* �ַ���С, ASCII=1, ����=2 */
	int   (*get_font_size)(const unsigned char *str);				/* ��ģ�Ļ�������С */
	unsigned char *(*get_font_data)(const unsigned char *str, int *length);	/* ��ģ����, ��������ַ�ʹ�С */
	void  (*draw_font)(int x, int y, const unsigned char *str);		/* ֱ����������ַ���framebuffer */
} font_desc_t;

/**********************************************************************
 * outline font application interface
 **********************************************************************/

#if (HAS_CHARSET_X > 0)
#define SONG_OUTLINE_FILE		"/ndd/font/HZKPSSTJ"
#define ASC_OUTLINE_FILE		"/ndd/font/ASCPS"
/*
 * ��ʼ�����嶨��
 */
int initialize_fontx_desc(void);
/*
 * ��ֹ���嶨��
 */
int finalize_fontx_desc(void);
#endif

/*
 * ��ȡ�ַ��������ֿ�����
 * outlinefont:	true:  �����ֿ�;
 * 				false: �����ֿ�
 */
font_desc_t *get_font_desc(const unsigned char *str, int outlinefont);

#ifdef	__cplusplus
}
#endif

#endif /* _FONT_DESC_H_ */


