/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * simple_gui.h
 *
 *  Created on: 2015-1-19
 *      Author: Bian
 */

#ifndef _SIMPLE_GUI_H_
#define _SIMPLE_GUI_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "queue_list.h"

/*
 * fixed length to decrease memory alloc times
 */
#define BUTTON_TEXT_MAX		16

/*
 * mouse/touchscreen click event handle
 */
typedef void (*OnClick)(unsigned, void *);

/**********************************************************************
 * Point & Rect
 **********************************************************************/

typedef struct Point
{
	int  x;
	int  y;
} TPoint;

typedef struct Rect
{
	int  left;
	int  top;
	int  right;
	int  bottom;
} TRect;

#define MAKE_POINT(pt, x1, y1)	\
		{						\
			(pt)->x = x1;		\
			(pt)->y = y1;		\
		}

#define MAKE_RECT(rect, x1, y1, x2, y2)	\
		{								\
			(rect)->left   = x1;		\
			(rect)->top    = y1;		\
			(rect)->right  = x2;		\
			(rect)->bottom = y2;		\
		}

extern void normalize_rect(TRect *rt);

/**********************************************************************
 * button
 **********************************************************************/

typedef struct Button
{
	TRect 	 rect;								/* button draw area */
	char  	 caption[BUTTON_TEXT_MAX+1];		/* button caption */
	int   	 guid;								/* unique id in all buttons */
	int   	 group;								/* we draw buttons in one group */
	bool   	 focused;							/* button is current focused */
	unsigned color;								/* button color */
	unsigned fontsize;							/* XXX: now only 16x16 font supported */
	unsigned fontcolor;							/* caption font color */
	bool     layout_changed;					/* need repaint */
    int      tag;                               /* for user usage */
    
	void  (*ownerdraw)(void *param);
	void  (*mousedown)(unsigned msg, void *param);
	void  (*mouseup)(unsigned msg, void *param);
	void  (*mousemove)(unsigned msg, void *param);
	void  (*onclick)(unsigned msg, void *param);

	TAILQ_ENTRY(Button) next;
} TButton;

/*
 * implement TButton
 */
TButton *new_button(TRect *rt, int guid, int group, const char *caption, OnClick click);
TButton *find_button(int guid);
TButton *find_button_first(int group);
int delete_button_by_guid(int guid);
int delete_button(TButton *p_btn);
int delete_button_by_group(int group);
void delete_button_all(void);
//TButton *get_button_by_guid(int guid);
int *find_button_by_group();
int get_buttons_count(int group);
TButton *focused_button_by_guid(int guid,unsigned char status);

/**********************************************************************
 * grid
 **********************************************************************/

#define GRID_TITLE_MAX		32
#define GRID_FORMAT_MAX		16
#define CELL_TEXT_MAX		100

#ifdef __cplusplus
#define border_left		0x01
#define border_top		0x02
#define border_right	0x04
#define border_bottom	0x08
#else
typedef enum
{
	border_left   = 0x01,
	border_top    = 0x02,
	border_right  = 0x04,
	border_bottom = 0x08,
} border_pos_t;
#endif

typedef struct Cell
{
	TRect 	 rect;						/* rectangle */
	int      index;						/* index of queue */
	unsigned border;					/* border to draw */
	bool	 visible;
	char     text[CELL_TEXT_MAX+1];		/* text */
	TAILQ_ENTRY(Cell) next;				/* queue of cell */
} TCell;

#ifdef __cplusplus
#define align_left   		0x00
#define align_center   		0x01
#define align_right   		0x02
#else
typedef enum
{
	align_left   = 0x00,
	align_center = 0x01,
	align_right  = 0x02,
} align_style_t;
#endif

typedef struct Column
{
	TRect 	 rect;						/* column rectangle */
	int      index;						/* column index of grid */
	int      visible;					/* column is visible */
	char     title[GRID_TITLE_MAX+1];	/* title */
	char     format[GRID_FORMAT_MAX+1];	/* data display format */
	int		 align;						/* data display align */
	int      cellcount;					/* number of cell, equel grid's rows+1 */
	unsigned fillcolor;					/* column fill color */
	unsigned fontcolor;					/* font color */
	TAILQ_HEAD(_cell_q, Cell) cells;	/* cell-queue header */
	TAILQ_ENTRY(Column) next;			/* queue of column */
} TColumn;

typedef struct Grid
{
	TRect	 rect;								/* rectangle */
	int 	 rows;								/* row numbers */
	int 	 cols;								/* col numbers */
	int		 borderwidth;						/* outerborder width */
	unsigned bordercolor;						/* outerborder color */
	unsigned bgcolor;							/* background color */
	unsigned linecolor;							/* line color */
	int 	 titleheight;						/* title height */
	unsigned titlecolor;						/* title fill color */
	int		 group;
	int   	 guid;								/* unique id in all buttons */
	bool     layout_changed;					/* need repaint */
	
//	rtems_id mtx;							/* semaphore for synchronize cell text */
	
	TAILQ_HEAD(_column_q, Column) columns;		/* column-queue header */
	TAILQ_ENTRY(Grid) next;
} TGrid;

/*
 * implement TGrid
 */
TGrid *create_grid(TRect *rt, int rows, int cols, int guid, int group);
TGrid *find_grid(int guid);
int    get_grids_count(int group);
void   destroy_grid(TGrid *grid);
void   grid_set_bgcolor(TGrid *grid, unsigned bgcolor);
void   grid_scroll_rows(TGrid *grid, int rows);
void   grid_blank_row(TGrid *grid, int row);

TColumn *grid_get_column(TGrid *grid, int index);
void   grid_set_column_index(TGrid *grid, TColumn *column, int index);
void   grid_set_column_title(TColumn *column, const char *title);
void   grid_set_column_format(TColumn *column, const char *format);
void   grid_set_column_width(TGrid *grid, int index, int width);
TColumn *grid_new_column(TGrid *grid, int index, int width);
int    grid_delete_column(TGrid *grid, int index);

void   grid_set_cell_text(TGrid *grid, int row, int col, const char *str);

/**********************************************************************
 * simple gui minotor task
 **********************************************************************/

void init_simple_gui_env(void);

#if BSP_USE_OS

int start_gui_monitor_task(void);
/*
 * only active group object is visible
 */
int set_gui_active_group(int group);
int get_gui_active_group(void);
bool simple_gui_busy(void);

#else
/*
 * just test drawing
 */
void paint_my_simple_gui(void);
void set_focused_button(int x, int y);

#endif

#ifdef	__cplusplus
}
#endif

#endif /* _SIMPLE_GUI_H_ */


