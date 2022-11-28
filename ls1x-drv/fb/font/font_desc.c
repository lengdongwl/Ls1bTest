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
 * 16 �����ֿ�
 ******************************************************************************/

/**********************************************************************
 * �����ֿ� 8x16���� operate functions
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
    "asc-16",                       	/* name[16]        �������� */
    (unsigned char *)asc_simple_8x16, 	/* data            ��������ֿ� */
    1,                              	/* is_asc */
    1,                              	/* intialized      �Ƿ��ʼ�� */
    1,                              	/* num_faces       �������� */
    0,                              	/* cur_face        ��ǰ���� */
	8,              			    	/* width           �ַ���� */
	16,                      	    	/* height          �ַ��߶� */
    0,                              	/* scalable        ��������, ������ */
    NULL,                           	/* load_font()     �ֿ�װ���ڴ� */
    NULL,                           	/* release_font()  �ͷ��ڴ� */
    asc_simple_char_in_this,        	/* char_in_this()  �ַ��Ƿ�������ֿ� */
	asc_simple_get_char_size,       	/* get_char_size() �ַ���С, ASCII=1, ����=2 */
	asc_simple_get_font_size,	    	/* get_font_size() ��ģ�Ļ�������С */
	asc_simple_get_font_data,       	/* get_font_data() ��ģ�Ļ����� ��ַ*/
	fb_draw_ascii_char,             	/* draw_font()     ����ַ���framebuffer */
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
    "asc-16",                       	/* name[16]        �������� */
	(unsigned char *)asc_iso8859_8x16, 			/* data            ��������ֿ� */
    1,                              	/* is_asc */
    1,                              	/* intialized      �Ƿ��ʼ�� */
    1,                              	/* num_faces       �������� */
    0,                              	/* cur_face        ��ǰ���� */
	8,              			    	/* width           �ַ���� */
	16,                      	    	/* height          �ַ��߶� */
    0,                              	/* scalable        ��������, ������ */
    NULL,                           	/* load_font()     �ֿ�װ���ڴ� */
    NULL,                           	/* release_font()  �ͷ��ڴ� */
    asc_iso8859_char_in_this,       	/* char_in_this()  �ַ��Ƿ�������ֿ� */
	asc_iso8859_get_char_size,      	/* get_char_size() �ַ���С, ASCII=1, ����=2 */
	asc_iso8859_get_font_size,	    	/* get_font_size() ��ģ�Ļ�������С */
	asc_iso8859_get_font_data,      	/* get_font_data() ��ģ�Ļ����� ��ַ*/
	fb_draw_ascii_char,             	/* draw_font()     ����ַ���framebuffer */
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
 * �����ֿ� 16x16���� operate functions
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
    "song-16",                      	/* name[16]        �������� */
    (unsigned char *)gb2312_song_16x16,			/* data            ��������ֿ� */
    0,                              	/* is_asc */
    1,                              	/* intialized      �Ƿ��ʼ�� */
    1,                              	/* num_faces       �������� */
    0,                              	/* cur_face        ��ǰ���� */
	16,              			    	/* width           �ַ���� */
	16,                      	    	/* height          �ַ��߶� */
    0,                              	/* scalable        ��������, ������ */
    NULL,                           	/* load_font()     �ֿ�װ���ڴ� */
    NULL,                           	/* release_font()  �ͷ��ڴ� */
    song_16x16_char_in_this,        	/* char_in_this()  �ַ��Ƿ�������ֿ� */
	song_16x16_get_char_size,       	/* get_char_size() �ַ���С, ASCII=1, ����=2 */
	song_16x16_get_font_size,	    	/* get_font_size() ��ģ�Ļ�������С */
	song_16x16_get_font_data,       	/* get_font_data() ��ģ�Ļ����� ��ַ*/
	fb_draw_gb2312_char,            	/* draw_font()     ����ַ���framebuffer */
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
 * ����ַ����Ƿ�����һ���ֿ�
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
 * �����ֿ�
 ******************************************************************************/

#if (HAS_CHARSET_X > 0)

#define DYN_MALLOC_BUFFER_X		1

#if 0
/*
 * �����ֿ��к�����Ϣ�����ṹ��
 */
typedef struct tag_font_idx
{
    unsigned int   fontoffset;      /* ��ģƫ���� */
    unsigned short fontlength;      /* ��ģ��Ϣ���� */
} font_idx_t;
#endif

/**********************************************************************
 * �����ֿ� ASCPS operate functions
 **********************************************************************/

#if (HAS_ASCII_FONT > 0)

#if (DYN_MALLOC_BUFFER_X == 0)
static unsigned char ascps_buf[60*1024];		/* �ֿ⻺���� */
#endif

static void asc_outline_load_font(const unsigned char *filename);
static void asc_outline_release_font(void);
static int  asc_outline_char_in_this(const unsigned char *str);
static int  asc_outline_get_char_size(const unsigned char *str);
static unsigned char *asc_outline_get_font_data(const unsigned char *str, int *length);

static font_desc_t font_asc_outline =
{
    "asc-x",                        /* name[16]        �������� */
	NULL,               			/* data            ��������ֿ� */
    1,                              /* is_asc */
    0,                              /* intialized      �Ƿ��ʼ�� */
    10,                             /* num_faces       �������� */
    0,                              /* cur_face        ��ǰ���� */
	100,              			    /* width           �ַ���� */
	100,                      	    /* height          �ַ��߶� */
    1,                              /* scalable        ��������, ������ */
    asc_outline_load_font,          /* load_font()     �ֿ�װ���ڴ� */
    asc_outline_release_font,       /* release_font()  �ͷ��ڴ� */
    asc_outline_char_in_this,       /* char_in_this()  �ַ��Ƿ�������ֿ� */
	asc_outline_get_char_size,      /* get_char_size() �ַ���С, ASCII=1, ����=2 */
	NULL,	                        /* get_font_size() ��ģ�Ļ�������С */
	asc_outline_get_font_data,      /* get_font_data() ��ģ�Ļ����� ��ַ*/
	NULL,                           /* draw_font()     ����ַ���framebuffer */
};

/*
 * �ֿ�װ���ڴ�
 */
static void asc_outline_load_font(const unsigned char *filename)
{
    font_desc_t *ff;
	FILE        *fd;
    int          fsize, n;

    ff = &font_asc_outline;
    if (ff->intialized)
        return;

    /* �����������ֿ��ļ��� */
	if ((fd = fopen(ASC_OUTLINE_FILE, "rb")) == NULL)
	{
		printk("ascii outline font file ASCPS open fail!\r\n");
		return;
	}

    /* �����ļ���С�����ڴ� */
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

    /* �ֿ����ݶ����ڴ� */
    fseek(fd, 0, SEEK_SET);
    n = fread(ff->data, 1, fsize, fd);
    if (n == fsize)
        ff->intialized = 1;
    else
        ff->intialized = 0;

	fclose(fd);
}

/*
 * �ͷ��ڴ�
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
 * �ַ��Ƿ�������ֿ�
 */
static int asc_outline_char_in_this(const unsigned char *str)
{
	if ((str == NULL) || ((*str) <= 32) || ((*str) > 126))
		return -1;

	return 0;
}

/*
 * �ַ���С, ASCII=1, ����=2
 */
static int asc_outline_get_char_size(const unsigned char *str)
{
	return 1;
}

/*
 * ��ģ����, ��������ַ�ʹ�С
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

    /* ����1-10������ */
    if (strlen((const char *)str) > 1)
    {
        int n = (int)(*(str + 1));
        if ((n >= 1) && (n < ff->num_faces))
            ff->cur_face = n;
    }

	/* font offset */
	offset = ((ff->cur_face) * 94 + (*str - 33)) * 6;

    p = (unsigned char *)((unsigned)ff->data + offset);

    /* ��ģƫ���� */
    fontoffset = (p[0] << 0) | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);

    /* ��ģ��Ϣ���� */
    fontlength  = (p[4] << 0) | (p[5] << 8);

    /* font length */
    *length = fontlength;

    /* font data */
    p = (unsigned char *)((unsigned)ff->data + fontoffset);

    return p;
}

#endif /* HAS_ASCII_FONT */

/**********************************************************************
 * �����ֿ� HZKPSSTJ operate functions
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
    "song-x",                       /* name[16]        �������� */
	NULL,               			/* data            ���� */
    0,                              /* is_asc */
    0,                              /* intialized      �Ƿ��ʼ�� */
    1,                              /* num_faces       �������� */
    1,                              /* cur_face        ��ǰ���� */
	168,              			    /* width           �ַ���� */
	168,                      	    /* height          �ַ��߶� */
    1,                              /* scalable        ��������, ������ */
    song_outline_load_font,         /* load_font()     �ֿ�װ���ڴ� */
    song_outline_release_font,      /* release_font()  �ͷ��ڴ� */
    song_outline_char_in_this,      /* char_in_this()  �ַ��Ƿ�������ֿ� */
	song_outline_get_char_size,     /* get_char_size() �ַ���С, ASCII=1, ����=2 */
	NULL,                           /* get_font_size() ��ģ�Ļ�������С */
	song_outline_get_font_data,     /* get_font_data() ��ģ�Ļ�������ַ*/
	NULL,
};

/*
 * �ֿ�װ���ڴ�
 */
static void song_outline_load_font(const unsigned char *filename)
{
    font_desc_t *ff;
	FILE        *fd;
    int          fsize, n;

    ff = &font_song_outline;
    if (ff->intialized)
        return;

    /* �����ߺ��ֿ��ļ�(�����) */
	if ((fd = fopen(SONG_OUTLINE_FILE, "rb")) == NULL)
	{
		printk("chinese song-j outline font file HZKPSSTJ open fail!\r\n");
		return;
	}

    /* �����ļ���С�����ڴ� */
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

    /* �ֿ����ݶ����ڴ� */
    fseek(fd, 0, SEEK_SET);
    n = fread(ff->data, 1, fsize, fd);
    if (n == fsize)
        ff->intialized = 1;
    else
        ff->intialized = 0;

	fclose(fd);
}

/*
 * �ͷ��ڴ�
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
 * �ַ��Ƿ�������ֿ�
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
 * �ַ���С, ASCII=1, ����=2
 */
static int song_outline_get_char_size(const unsigned char *str)
{
	return 2;
}

/*
 * ��ģ����, ��������ַ�ʹ�С
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

        /* ���㺺������ */
        QM = (ch1 - 0xA0) & 0x7F;

        /*
         * FIXME ��һ��(16��55��)�Ͷ���(56��87��)���ֿ�
         */
        if ((QM < 16) || (QM > 87))
            return NULL;

        /* ���㺺��λ�� */
        WM = (ch2 - 0xA0) & 0x7F;

        /* ���㺺�����ֿ��е����� */
        offset = ((QM - 16) * 94 + (WM - 1)) * 6;

        p = (unsigned char *)((unsigned)ff->data + offset);

        /* ��ģƫ���� */
        fontoffset = (p[0] << 0) | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);

        /* ��ģ��Ϣ���� */
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
 * ��ʼ�����嶨��
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
 * ��ֹ���嶨��
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
 * ����ַ����Ƿ�����һ���ֿ�
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
 * �ַ�������
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


