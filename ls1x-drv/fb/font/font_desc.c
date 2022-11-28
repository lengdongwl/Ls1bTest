/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * font_desc.c
 *
 *  Created on: 2015-1-2
 *      Author: Bian
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp.h"

#ifdef BSP_USE_FB

#include "font_desc.h"

/******************************************************************************
 * 16 点阵字库
 ******************************************************************************/

/**********************************************************************
 * 西文字库 8x16点阵 operate functions
 **********************************************************************/

#if (HAS_CHARSET_16 > 0)

#if (HAS_ASCII_FONT > 0)

#define HAS_ASC_SIMPLE_8X16     0
#define HAS_ASC_ISO8859_8X16    1

#if (HAS_ASC_SIMPLE_8X16 > 0)

#include "asc-simple-8x16.inl"

static int asc_simple_char_in_this(const unsigned char *str);
static int asc_simple_get_char_size(const unsigned char *str);
static int asc_simple_get_font_size(const unsigned char *str);
static unsigned char *asc_simple_get_font_data(const unsigned char *str, int *length);

extern void fb_draw_ascii_char(int x, int y, const unsigned char *str);

static font_desc_t font_asc_simple_8x16 =
{
    "asc-16",                       	/* name[16]        字体名称 */
    (unsigned char *)asc_simple_8x16, 	/* data            字体点阵字库 */
    1,                              	/* is_asc */
    1,                              	/* intialized      是否初始化 */
    1,                              	/* num_faces       字体总数 */
    0,                              	/* cur_face        当前字体 */
	8,              			    	/* width           字符宽度 */
	16,                      	    	/* height          字符高度 */
    0,                              	/* scalable        轮廓字体, 可缩放 */
    NULL,                           	/* load_font()     字库装入内存 */
    NULL,                           	/* release_font()  释放内存 */
    asc_simple_char_in_this,        	/* char_in_this()  字符是否在这个字库 */
	asc_simple_get_char_size,       	/* get_char_size() 字符大小, ASCII=1, 汉字=2 */
	asc_simple_get_font_size,	    	/* get_font_size() 字模的缓冲区大小 */
	asc_simple_get_font_data,       	/* get_font_data() 字模的缓冲区 地址*/
	fb_draw_ascii_char,             	/* draw_font()     输出字符到framebuffer */
};

static int asc_simple_char_in_this(const unsigned char *str)
{
	if ((str == NULL) || ((*str) < 0x20) || ((*str) > 0x7F))
		return -1;

	return 0;
}

static int asc_simple_get_char_size(const unsigned char *str)
{
	return 1;
}

static int asc_simple_get_font_size(const unsigned char *str)
{
	return 16;          // 8*16/8;
}

static unsigned char *asc_simple_get_font_data(const unsigned char *str, int *length)
{
    int offset;
	if ((str == NULL) || ((*str) < 0x20) || ((*str) > 0x7F))
		return NULL;

    *length = 16;       // 8*16/8;
    
	/* 16 bytes per font */
	offset = ((int)(*str) - 0x20) * 16;
	return (unsigned char *)(asc_simple_8x16 + (unsigned)offset);
}

#endif /* HAS_ASC_SIMPLE_8X16 */

#if (HAS_ASC_ISO8859_8X16 > 0)

#include "asc-iso8859-8x16.inl"

static int asc_iso8859_char_in_this(const unsigned char *str);
static int asc_iso8859_get_char_size(const unsigned char *str);
static int asc_iso8859_get_font_size(const unsigned char *str);
static unsigned char *asc_iso8859_get_font_data(const unsigned char *str, int *length);

extern void fb_draw_ascii_char(int x, int y, const unsigned char *str);

static font_desc_t font_asc_iso8859_8x16 =
{
    "asc-16",                       	/* name[16]        字体名称 */
	(unsigned char *)asc_iso8859_8x16, 			/* data            字体点阵字库 */
    1,                              	/* is_asc */
    1,                              	/* intialized      是否初始化 */
    1,                              	/* num_faces       字体总数 */
    0,                              	/* cur_face        当前字体 */
	8,              			    	/* width           字符宽度 */
	16,                      	    	/* height          字符高度 */
    0,                              	/* scalable        轮廓字体, 可缩放 */
    NULL,                           	/* load_font()     字库装入内存 */
    NULL,                           	/* release_font()  释放内存 */
    asc_iso8859_char_in_this,       	/* char_in_this()  字符是否在这个字库 */
	asc_iso8859_get_char_size,      	/* get_char_size() 字符大小, ASCII=1, 汉字=2 */
	asc_iso8859_get_font_size,	    	/* get_font_size() 字模的缓冲区大小 */
	asc_iso8859_get_font_data,      	/* get_font_data() 字模的缓冲区 地址*/
	fb_draw_ascii_char,             	/* draw_font()     输出字符到framebuffer */
};

static int asc_iso8859_char_in_this(const unsigned char *str)
{
	if ((str == NULL) || ((*str) > 0x7F))
		return -1;

	return 0;
}

static int asc_iso8859_get_char_size(const unsigned char *str)
{
	return 1;
}

static int asc_iso8859_get_font_size(const unsigned char *str)
{
	return 16;          // 8*16/8;
}

static unsigned char *asc_iso8859_get_font_data(const unsigned char *str, int *length)
{
    int offset;
	if ((str == NULL) || ((*str) > 0x7F))
		return NULL;

    *length = 16;       // 8*16/8;
    
	/* 16 bytes per font */
	offset = (int)(*str) * 16;
	return (unsigned char *)(asc_iso8859_8x16 + (unsigned)offset);
}

#endif /* HAS_ASC_ISO8859_8X16 */

#endif /* HAS_ASCII_FONT */

/**********************************************************************
 * 中文字库 16x16点阵 operate functions
 **********************************************************************/

#if (HAS_CHINESE_FONT > 0)

#include "song-gb2312-16x16.inl"

static int song_16x16_char_in_this(const unsigned char *str);
static int song_16x16_get_char_size(const unsigned char *str);
static int song_16x16_get_font_size(const unsigned char *str);
static unsigned char *song_16x16_get_font_data(const unsigned char *str, int *length);

extern void fb_draw_gb2312_char(int x, int y, const unsigned char *str);

static font_desc_t font_song_16x16 =
{
    "song-16",                      	/* name[16]        字体名称 */
    (unsigned char *)gb2312_song_16x16,			/* data            字体点阵字库 */
    0,                              	/* is_asc */
    1,                              	/* intialized      是否初始化 */
    1,                              	/* num_faces       字体总数 */
    0,                              	/* cur_face        当前字体 */
	16,              			    	/* width           字符宽度 */
	16,                      	    	/* height          字符高度 */
    0,                              	/* scalable        轮廓字体, 可缩放 */
    NULL,                           	/* load_font()     字库装入内存 */
    NULL,                           	/* release_font()  释放内存 */
    song_16x16_char_in_this,        	/* char_in_this()  字符是否在这个字库 */
	song_16x16_get_char_size,       	/* get_char_size() 字符大小, ASCII=1, 汉字=2 */
	song_16x16_get_font_size,	    	/* get_font_size() 字模的缓冲区大小 */
	song_16x16_get_font_data,       	/* get_font_data() 字模的缓冲区 地址*/
	fb_draw_gb2312_char,            	/* draw_font()     输出字符到framebuffer */
};

#define IS_GB2312_CHAR(ch1, ch2) \
        if (((ch1 >= 0xA1 && ch1 <= 0xA9) || (ch1 >= 0xB0 && ch1 <= 0xF7)) \
        	&& ch2 >= 0xA1 && ch2 <= 0xFE)

static int song_16x16_char_in_this(const unsigned char *str)
{
    unsigned char ch1, ch2;
	if (str == NULL)
		return -1;

	ch1 = str[0];
	ch2 = str[1];

	IS_GB2312_CHAR(ch1, ch2)
	{
		return 0;
	}

    return -1;
}

static int song_16x16_get_char_size(const unsigned char *str)
{
	return 2;
}

static int song_16x16_get_font_size(const unsigned char *str)
{
	return 32;          // 16*16/8;
}

static unsigned char *song_16x16_get_font_data(const unsigned char *str, int *length)
{
    unsigned char ch1, ch2;

	if (str == NULL)
		return NULL;

    ch1 = str[0];
    ch2 = str[1];

	IS_GB2312_CHAR(ch1, ch2)
    {
        int area   = (int)ch1 - 0xA1;
        int offset = 0;

        *length = 32;   // 16*16/8;

        if (area < 9)
            offset = area * 94 + ch2 - 0xA1;
        else if (area >= 15)
        	offset = (area - 6) * 94 + ch2 - 0xA1;

        /* 32 bytes per font */
        return (unsigned char *)(gb2312_song_16x16 + (unsigned)offset * 32);
    }

    return NULL;
}

#endif /* HAS_CHINESE_FONT */

/*
 * font 16x16 desc table
 */
static font_desc_t *font16_descs[] =
{
#if (HAS_ASCII_FONT > 0)
  #if (HAS_ASC_SIMPLE_8X16 > 0)
    &font_asc_simple_8x16,
  #endif
  #if (HAS_ASC_ISO8859_8X16 > 0)
    &font_asc_iso8859_8x16,
  #endif
#endif
#if (HAS_CHINESE_FONT > 0)
    &font_song_16x16,
#endif
};

static int font16_descs_count = sizeof(font16_descs) / sizeof(font_desc_t *);

/*
 * 检查字符串是否属于一个字库
 */
static font_desc_t *get_font16_desc(const unsigned char *str)
{
	int i;
	for (i=0; i<font16_descs_count; i++)
        if (font16_descs[i]->char_in_this(str) == 0)
			return font16_descs[i];
	return NULL;
}

#endif /* HAS_CHARSET_16 */


/******************************************************************************
 * 轮廓字库
 ******************************************************************************/

#if (HAS_CHARSET_X > 0)

#define DYN_MALLOC_BUFFER_X		1

#if 0
/*
 * 曲线字库中汉字信息索引结构体
 */
typedef struct tag_font_idx
{
    unsigned int   fontoffset;      /* 字模偏移量 */
    unsigned short fontlength;      /* 字模信息长度 */
} font_idx_t;
#endif

/**********************************************************************
 * 西文字库 ASCPS operate functions
 **********************************************************************/

#if (HAS_ASCII_FONT > 0)

#if (DYN_MALLOC_BUFFER_X == 0)
static unsigned char ascps_buf[60*1024];		/* 字库缓冲区 */
#endif

static void asc_outline_load_font(const unsigned char *filename);
static void asc_outline_release_font(void);
static int  asc_outline_char_in_this(const unsigned char *str);
static int  asc_outline_get_char_size(const unsigned char *str);
static unsigned char *asc_outline_get_font_data(const unsigned char *str, int *length);

static font_desc_t font_asc_outline =
{
    "asc-x",                        /* name[16]        字体名称 */
	NULL,               			/* data            字体点阵字库 */
    1,                              /* is_asc */
    0,                              /* intialized      是否初始化 */
    10,                             /* num_faces       字体总数 */
    0,                              /* cur_face        当前字体 */
	100,              			    /* width           字符宽度 */
	100,                      	    /* height          字符高度 */
    1,                              /* scalable        轮廓字体, 可缩放 */
    asc_outline_load_font,          /* load_font()     字库装入内存 */
    asc_outline_release_font,       /* release_font()  释放内存 */
    asc_outline_char_in_this,       /* char_in_this()  字符是否在这个字库 */
	asc_outline_get_char_size,      /* get_char_size() 字符大小, ASCII=1, 汉字=2 */
	NULL,	                        /* get_font_size() 字模的缓冲区大小 */
	asc_outline_get_font_data,      /* get_font_data() 字模的缓冲区 地址*/
	NULL,                           /* draw_font()     输出字符到framebuffer */
};

/*
 * 字库装入内存
 */
static void asc_outline_load_font(const unsigned char *filename)
{
    font_desc_t *ff;
	FILE        *fd;
    int          fsize, n;

    ff = &font_asc_outline;
    if (ff->intialized)
        return;

    /* 打开曲线西文字库文件名 */
	if ((fd = fopen(ASC_OUTLINE_FILE, "rb")) == NULL)
	{
		printk("ascii outline font file ASCPS open fail!\r\n");
		return;
	}

    /* 根据文件大小申请内存 */
    fseek(fd, 0, SEEK_END);
    fsize = ftell(fd);

#if (DYN_MALLOC_BUFFER_X == 0)
    ff->data = ascps_buf;
#else
    ff->data = (unsigned char *)malloc(fsize);
#endif

    if (ff->data == NULL)
    {
        fclose(fd);
        return;
    }

    /* 字库数据读入内存 */
    fseek(fd, 0, SEEK_SET);
    n = fread(ff->data, 1, fsize, fd);
    if (n == fsize)
        ff->intialized = 1;
    else
        ff->intialized = 0;

	fclose(fd);
}

/*
 * 释放内存
 */
static void asc_outline_release_font(void)
{
    font_desc_t *ff;

    ff = &font_asc_outline;
    if ((!ff->intialized) || (ff->data == NULL))
        return;

#if (DYN_MALLOC_BUFFER_X == 1)
    free(ff->data);
#endif

    ff->data = NULL;
    ff->intialized = 0;
}

/*
 * 字符是否在这个字库
 */
static int asc_outline_char_in_this(const unsigned char *str)
{
	if ((str == NULL) || ((*str) <= 32) || ((*str) > 126))
		return -1;

	return 0;
}

/*
 * 字符大小, ASCII=1, 汉字=2
 */
static int asc_outline_get_char_size(const unsigned char *str)
{
	return 1;
}

/*
 * 字模数据, 缓冲区地址和大小
 */
static unsigned char *asc_outline_get_font_data(const unsigned char *str, int *length)
{
    int      offset;
    uint32_t fontoffset;
    uint16_t fontlength;
    unsigned char *p;
    font_desc_t   *ff;


    ff = &font_asc_outline;
	if ((!ff->intialized) || (ff->data == NULL) ||
        (str == NULL) || ((*str) <= 32) || ((*str) > 126))
		return NULL;

    /* 共有1-10种字体 */
    if (strlen((const char *)str) > 1)
    {
        int n = (int)(*(str + 1));
        if ((n >= 1) && (n < ff->num_faces))
            ff->cur_face = n;
    }

	/* font offset */
	offset = ((ff->cur_face) * 94 + (*str - 33)) * 6;

    p = (unsigned char *)((unsigned)ff->data + offset);

    /* 字模偏移量 */
    fontoffset = (p[0] << 0) | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);

    /* 字模信息长度 */
    fontlength  = (p[4] << 0) | (p[5] << 8);

    /* font length */
    *length = fontlength;

    /* font data */
    p = (unsigned char *)((unsigned)ff->data + fontoffset);

    return p;
}

#endif /* HAS_ASCII_FONT */

/**********************************************************************
 * 中文字库 HZKPSSTJ operate functions
 **********************************************************************/

#if (HAS_CHINESE_FONT > 0)

#if (DYN_MALLOC_BUFFER_X == 0)
extern unsigned char *alloc_mem_of_large_partition(int *size, unsigned char *addr);
extern void free_mem_of_large_partition(unsigned char *addr);
#endif

static void song_outline_load_font(const unsigned char *filename);
static void song_outline_release_font(void);
static int  song_outline_char_in_this(const unsigned char *str);
static int  song_outline_get_char_size(const unsigned char *str);
static unsigned char *song_outline_get_font_data(const unsigned char *str, int *length);

static font_desc_t font_song_outline =
{
    "song-x",                       /* name[16]        字体名称 */
	NULL,               			/* data            字体 */
    0,                              /* is_asc */
    0,                              /* intialized      是否初始化 */
    1,                              /* num_faces       字体总数 */
    1,                              /* cur_face        当前字体 */
	168,              			    /* width           字符宽度 */
	168,                      	    /* height          字符高度 */
    1,                              /* scalable        轮廓字体, 可缩放 */
    song_outline_load_font,         /* load_font()     字库装入内存 */
    song_outline_release_font,      /* release_font()  释放内存 */
    song_outline_char_in_this,      /* char_in_this()  字符是否在这个字库 */
	song_outline_get_char_size,     /* get_char_size() 字符大小, ASCII=1, 汉字=2 */
	NULL,                           /* get_font_size() 字模的缓冲区大小 */
	song_outline_get_font_data,     /* get_font_data() 字模的缓冲区地址*/
	NULL,
};

/*
 * 字库装入内存
 */
static void song_outline_load_font(const unsigned char *filename)
{
    font_desc_t *ff;
	FILE        *fd;
    int          fsize, n;

    ff = &font_song_outline;
    if (ff->intialized)
        return;

    /* 打开曲线汉字库文件(宋体简) */
	if ((fd = fopen(SONG_OUTLINE_FILE, "rb")) == NULL)
	{
		printk("chinese song-j outline font file HZKPSSTJ open fail!\r\n");
		return;
	}

    /* 根据文件大小申请内存 */
    fseek(fd, 0, SEEK_END);
    n = fsize = ftell(fd);
    ff->data = (unsigned char *)malloc(fsize);

    if ((ff->data == NULL) || (n < fsize))
    {
        fclose(fd);
        free(ff->data);
        ff->data = NULL;
        return;
    }

    /* 字库数据读入内存 */
    fseek(fd, 0, SEEK_SET);
    n = fread(ff->data, 1, fsize, fd);
    if (n == fsize)
        ff->intialized = 1;
    else
        ff->intialized = 0;

	fclose(fd);
}

/*
 * 释放内存
 */
static void song_outline_release_font(void)
{
    font_desc_t *ff;

    ff = &font_song_outline;
    if ((!ff->intialized) || (ff->data == NULL))
        return;

    free(ff->data);
    ff->data = NULL;
    ff->intialized = 0;
}

#define IS_GB2312_CHAR(ch1, ch2) \
        if (((ch1 >= 0xA1 && ch1 <= 0xA9) || (ch1 >= 0xB0 && ch1 <= 0xF7)) \
        	&& ch2 >= 0xA1 && ch2 <= 0xFE)

/*
 * 字符是否在这个字库
 */
static int song_outline_char_in_this(const unsigned char *str)
{
    unsigned char ch1, ch2;

	if (str == NULL)
		return -1;

    ch1 = str[0];
    ch2 = str[1];

	IS_GB2312_CHAR(ch1, ch2)
	{
		return 0;
	}

    return -1;
}

/*
 * 字符大小, ASCII=1, 汉字=2
 */
static int song_outline_get_char_size(const unsigned char *str)
{
	return 2;
}

/*
 * 字模数据, 缓冲区地址和大小
 */
static unsigned char *song_outline_get_font_data(const unsigned char *str, int *length)
{
    unsigned char ch1, ch2, *p;
    font_desc_t *ff;

    ff = &font_song_outline;
	if ((!ff->intialized) || (ff->data == NULL) ||(str == NULL))
		return NULL;

    ch1 = str[0];
    ch2 = str[1];

	IS_GB2312_CHAR(ch1, ch2)
    {
        int QM, WM, offset;
        uint32_t fontoffset;
        uint16_t fontlength;

        /* 计算汉字区码 */
        QM = (ch1 - 0xA0) & 0x7F;

        /*
         * FIXME 仅一级(16至55区)和二级(56至87区)汉字库
         */
        if ((QM < 16) || (QM > 87))
            return NULL;

        /* 计算汉字位码 */
        WM = (ch2 - 0xA0) & 0x7F;

        /* 计算汉字在字库中的索引 */
        offset = ((QM - 16) * 94 + (WM - 1)) * 6;

        p = (unsigned char *)((unsigned)ff->data + offset);

        /* 字模偏移量 */
        fontoffset = (p[0] << 0) | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);

        /* 字模信息长度 */
        fontlength  = (p[4] << 0) | (p[5] << 8);

        /* font length */
        *length = fontlength;

        /* font data */
        fontoffset &= ~0x10000000;	/* UCDOS 6.0/7.0 offset adjust */
        p = (unsigned char *)((unsigned)ff->data + fontoffset);

    return p;
    }

    return NULL;
}

#endif /* HAS_CHINESE_FONT */

/*
 * font outline desc table
 */
static font_desc_t *fontx_descs[] =
{
#if (HAS_ASCII_FONT > 0)
    &font_asc_outline,
#endif
#if (HAS_CHINESE_FONT > 0)
    &font_song_outline,
#endif
};

static int fontx_descs_count = sizeof(fontx_descs) / sizeof(font_desc_t *);

/*
 * 初始化字体定义
 */
int initialize_fontx_desc(void)
{
	int i;
	for (i=0; i<fontx_descs_count; i++)
		if (fontx_descs[i]->load_font != NULL)
			fontx_descs[i]->load_font(NULL);
	return 0;
}

/*
 * 终止字体定义
 */
int finalize_fontx_desc(void)
{
	int i;
	for (i=0; i<fontx_descs_count; i++)
		if ((fontx_descs[i]->release_font != NULL) &&
            (fontx_descs[i]->intialized) && (fontx_descs[i]->data != NULL))
			fontx_descs[i]->release_font();
	return 0;
}

/*
 * 检查字符串是否属于一个字库
 */
static font_desc_t *get_fontx_desc(const unsigned char *str)
{
	int i;
	for (i=0; i<fontx_descs_count; i++)
        if (fontx_descs[i]->char_in_this(str) == 0)
        	return fontx_descs[i];
	return NULL;
}

#endif /* HAS_CHARSET_X */


/******************************************************************************
 * 字符集操作
 ******************************************************************************/

font_desc_t *get_font_desc(const unsigned char *str, int outlinefont)
{
    if (outlinefont != 0)
#if (HAS_CHARSET_X > 0)
    	return get_fontx_desc(str);
#else
    	return NULL;
#endif
    else
#if (HAS_CHARSET_16 > 0)
        return get_font16_desc(str);
#else
    	return NULL;
#endif
}

#endif // #ifdef BSP_USE_FB


