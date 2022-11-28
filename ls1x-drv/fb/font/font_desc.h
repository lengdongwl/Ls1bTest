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
 *  系统字符集
 *
 */

#ifndef _FONT_DESC_H_
#define _FONT_DESC_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include "bsp.h"

/*
 * 轮廓和点阵两种字符集
 */
#if (BSP_NEED_FONTX > 0)
#define HAS_CHARSET_X       1
#else
#define HAS_CHARSET_X       0
#endif

#define HAS_CHARSET_16      1

/*
 * 中文和西文字库
 */
#define HAS_CHINESE_FONT    1
#define HAS_ASCII_FONT      1

typedef struct tag_font_desc
{
	char  			name[16];            	/* 字体名称 */
	unsigned char  *data;                	/* 字体 */
    int       		is_asc;
    int       		intialized;          	/* 是否初始化 */
    unsigned  		num_faces;           	/* 字体总数 */
    int       		cur_face;            	/* 当前字体 */
	unsigned   		width;               	/* 字符宽度 */
	unsigned  		height;              	/* 字符高度 */
    unsigned 		scalable;            	/* 轮廓字体, 可缩放 */

    void  (*load_font)(const unsigned char *filename);				/* 字库装入内存 */
    void  (*release_font)(void);									/* 释放内存 */
	int   (*char_in_this)(const unsigned char *str);				/* 字符是否在这个字库 */
	int   (*get_char_size)(const unsigned char *str);				/* 字符大小, ASCII=1, 汉字=2 */
	int   (*get_font_size)(const unsigned char *str);				/* 字模的缓冲区大小 */
	unsigned char *(*get_font_data)(const unsigned char *str, int *length);	/* 字模数据, 缓冲区地址和大小 */
	void  (*draw_font)(int x, int y, const unsigned char *str);		/* 直接输出点阵字符到framebuffer */
} font_desc_t;

/**********************************************************************
 * outline font application interface
 **********************************************************************/

#if (HAS_CHARSET_X > 0)
#define SONG_OUTLINE_FILE		"/ndd/font/HZKPSSTJ"
#define ASC_OUTLINE_FILE		"/ndd/font/ASCPS"
/*
 * 初始化字体定义
 */
int initialize_fontx_desc(void);
/*
 * 终止字体定义
 */
int finalize_fontx_desc(void);
#endif

/*
 * 获取字符串的字字库描述
 * outlinefont:	true:  轮廓字库;
 * 				false: 点阵字库
 */
font_desc_t *get_font_desc(const unsigned char *str, int outlinefont);

#ifdef	__cplusplus
}
#endif

#endif /* _FONT_DESC_H_ */


