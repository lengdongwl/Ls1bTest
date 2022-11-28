/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * simple-gui.c
 *
 *  Created on: 2015-1-19
 *      Author: Bian
 */

#include "bsp.h"

#ifdef BSP_USE_FB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(OS_RTTHREAD)
#include "rtthread.h"
#elif defined(OS_UCOS)
#include "os.h"
#elif defined(OS_FREERTOS)
#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"
#endif

#if defined(OS_RTTHREAD)
#define MALLOC  rt_malloc
#define FREE    rt_free
#else
#define MALLOC  malloc
#define FREE    free
#endif

#include "ls1x_fb.h"
#ifdef XPT2046_DRV
#include "spi/xpt2046.h"
#elif defined(GT1151_DRV)
#include "touch.h"
#include "gt1151q.h"
#endif

#include "../../ls1x-drv/fb/font/font_desc.h"
#include "simple_gui.h"

#include "app_os_priority.h"

/******************************************************************************
 * simple gui common
 ******************************************************************************/

static volatile int gui_active_group = -1;  /* current buttons to show, need semphore ? */

#ifndef GUI_STK_SIZE
//#define GUI_STK_SIZE    4096
#endif

#if defined(OS_RTTHREAD)
static  rt_thread_t     gui_thread;		    /* task for change button click events */

#elif defined(OS_UCOS)
static  OS_STK          gui_stack[GUI_STK_SIZE];

#elif defined(OS_FREERTOS)

#endif

//-------------------------------------------------------------------------------------------------

static volatile int m_simple_gui_busy = 0;	/* objects is painting now */

int set_gui_active_group(int group);
int get_gui_active_group(void);

/*
 * ASC font size
 */
static int m_asc_font_width  = 8;
static int m_asc_font_height = 16;

/*
 * color index to draw
 */
#define DEFAULT_COLOR_INDEX		0xF0

static inline unsigned fade_color(unsigned color, float factor)
{
	unsigned char r = ((unsigned char)(color >> 16)) * factor;
	unsigned char g = ((unsigned char)(color >> 8)) * factor;
	unsigned char b = ((unsigned char)(color)) * factor;
	return (unsigned)((r << 16) | (g << 8) | (b));
}

/******************************************************************************
 * normalize rect coordinate
 */
void normalize_rect(TRect *rect)
{
	int tmp;
	if (rect->right < rect->left)
	{
		tmp = rect->right;
		rect->right = rect->left;
		rect->left = tmp;
	}
	if (rect->bottom < rect->top)
	{
		tmp = rect->bottom;
		rect->bottom = rect->top;
		rect->top = tmp;
	}
}

/******************************************************************************
 * TButton implement
 ******************************************************************************/

static TAILQ_HEAD(_button_q, Button)    gui_buttons;	/* buttons queue */

#define BTN_BORDER_THICKNESS	2

#define FADED_1			0.75
#define FADED_2			0.625
#define FADED_3			0.875

static void draw_button(TButton *p_btn)
{
    TRect   *rt;
    int      w, h, j, cy1, cy2;
    unsigned color;
    float    step, factor;

	if ((!p_btn->layout_changed) || (p_btn == NULL))
    	return;

    rt = &p_btn->rect;
    normalize_rect(rt);

    if (p_btn->ownerdraw != NULL)
    {
    	p_btn->ownerdraw((void *)p_btn);
    	return;
    }

    w = rt->right  - rt->left;			/* width */
    h = rt->bottom - rt->top;			/* height */

    /* outer border
     */
    for (j=0; j<BTN_BORDER_THICKNESS; j++)
    {
        if (!p_btn->focused)
        {
        	/* half brightness */
        	color = (((unsigned char)(p_btn->color >> (16+BTN_BORDER_THICKNESS-j))) << 16) |
        			(((unsigned char)(p_btn->color >> (8 +BTN_BORDER_THICKNESS-j))) <<  8) |
        			(((unsigned char)(p_btn->color >> (0 +BTN_BORDER_THICKNESS-j))) <<  0);
        	SetColor(DEFAULT_COLOR_INDEX, color);
        }
        else if (p_btn->color != clBLUE)
        	SetColor(DEFAULT_COLOR_INDEX, clBLUE);
        else
        	SetColor(DEFAULT_COLOR_INDEX, clBLACK);
    	DrawRect(rt->left+j, rt->top+j, rt->right-j, rt->bottom-j, DEFAULT_COLOR_INDEX);
    }

    /* center line, desalinate 0.625
     */
    color = fade_color(p_btn->color, FADED_2);
    SetColor(DEFAULT_COLOR_INDEX, color);
    cy1 = rt->top + h / 2 + 0.5;
    DrawLine(rt->left  + BTN_BORDER_THICKNESS, cy1,
    		 rt->right - BTN_BORDER_THICKNESS, cy1, DEFAULT_COLOR_INDEX);

    /* upper area
     */
    step = (1.0 - FADED_1) / (cy1 - (rt->top+BTN_BORDER_THICKNESS));
    factor = 1.0;
    for (j=rt->top+BTN_BORDER_THICKNESS; j<cy1; j++)
    {
    	factor -= step;
        color = fade_color(p_btn->color, factor);
        SetColor(DEFAULT_COLOR_INDEX, color);
        DrawLine(rt->left  + BTN_BORDER_THICKNESS, j,
        		 rt->right - BTN_BORDER_THICKNESS, j, DEFAULT_COLOR_INDEX);
    }

    /* lower area
     */
    cy2 = ((rt->bottom - BTN_BORDER_THICKNESS) - cy1) * 2 / 3 + cy1 + 0.5;

    step = (FADED_3 - FADED_2) / (cy2 - cy1);
    factor  = FADED_3;
    for (j=cy2; j>cy1; j--)
    {
    	factor -= step;
        color = fade_color(p_btn->color, factor);
        SetColor(DEFAULT_COLOR_INDEX, color);
        DrawLine(rt->left  + BTN_BORDER_THICKNESS, j,
        		 rt->right - BTN_BORDER_THICKNESS, j, DEFAULT_COLOR_INDEX);
    }

    step = (FADED_3 - FADED_1) / ((rt->bottom - BTN_BORDER_THICKNESS) - cy2);
    factor  = FADED_3;
    for (j=cy2+1; j<=rt->bottom-BTN_BORDER_THICKNESS; j++)
    {
    	factor -= step;
        color = fade_color(p_btn->color, factor);
        SetColor(DEFAULT_COLOR_INDEX, color);
        DrawLine(rt->left  + BTN_BORDER_THICKNESS, j,
        		 rt->right - BTN_BORDER_THICKNESS, j, DEFAULT_COLOR_INDEX);
    }

    /* draw text last
     */
    h -= 2*BTN_BORDER_THICKNESS;
    w -= 2*BTN_BORDER_THICKNESS;
    if ((h > m_asc_font_height) &&
    	(w > m_asc_font_width) && (p_btn->caption != NULL))
    {
    	int  n_temp;
    	char temp[BUTTON_TEXT_MAX + 1];

    	n_temp = w / m_asc_font_width;
    	n_temp = n_temp < BUTTON_TEXT_MAX ? n_temp : BUTTON_TEXT_MAX;
    	strncpy(temp, p_btn->caption, n_temp);
    	temp[n_temp] = 0;
        SetFGColor(DEFAULT_COLOR_INDEX, p_btn->fontcolor);
        PutStringCenter(rt->left + (rt->right - rt->left) / 2 + 0.5,
        				rt->top + (rt->bottom - rt->top) / 2 + 0.5,
        				temp, DEFAULT_COLOR_INDEX);
    }

    p_btn->layout_changed = false;
}

static void paint_active_buttons(void)
{
	TButton *btn;

	TAILQ_FOREACH(btn, &gui_buttons, next)
	{
		if (btn->group == gui_active_group)
			draw_button(btn);
	}
}

/*
 * find button by guid
 */
TButton *find_button(int guid)
{
	TButton *btn;

	TAILQ_FOREACH(btn, &gui_buttons, next)
	{
		if (btn->guid == guid)
			return btn;
	}

	return NULL;
}

/*
 * find first button by group
 */
TButton *find_button_first(int group)
{
	TButton *btn;

	TAILQ_FOREACH(btn, &gui_buttons, next)
	{
		if (btn->group == group)
			return btn;
	}

	return NULL;
}

/*
 * find button arr by group
 */
int *find_button_by_group(int group)
{
	static int buf[100];
    int i=0;
    TButton *btn;
    
	TAILQ_FOREACH(btn, &gui_buttons, next)
	{
		if (btn->group == group)
		{
			buf[i++]=btn->guid;
		}
	}

	return buf;
}

/*
 * dynamic add one button
 */
TButton *new_button(TRect *rt, int guid, int group, const char *caption, OnClick click)
{
	TButton *p_btn;

	/* check to invoid duplicated guid
	 */
	if (find_button(guid) != NULL)
	{
		errno = EEXIST;
		return NULL;
	}

	/* create new button
	 */
	p_btn = (TButton *)MALLOC(sizeof(TButton));
	if (p_btn == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}
	memset((void *)p_btn, 0, sizeof(TButton));

	p_btn->guid        = guid;
	p_btn->group       = group;
	p_btn->color       = clSILVER;
	p_btn->fontcolor   = clBLACK;
	p_btn->rect.left   = rt->left;
	p_btn->rect.top    = rt->top;
	p_btn->rect.right  = rt->right;
	p_btn->rect.bottom = rt->bottom;
	p_btn->layout_changed = true;
    p_btn->tag            = 0;
    
	if (caption != NULL)
	{
		strncpy(p_btn->caption, caption, BUTTON_TEXT_MAX);
	}

	/* button click event handle */
	if (click != NULL)
		p_btn->onclick = click;

	TAILQ_INSERT_TAIL(&gui_buttons, p_btn, next);

	return p_btn;
}

/*
 * dynamic delete one button
 */
int delete_button_by_guid(int guid)
{
	TButton *btn;

	btn = find_button(guid);
	if (btn != NULL)
	{
		TAILQ_REMOVE(&gui_buttons, btn, next);
		FREE(btn);

		return 0;
	}

	return -1;
}

int delete_button(TButton *p_btn)
{
	if (p_btn == NULL)
		return 0;

	return delete_button_by_guid(p_btn->guid);
}

int delete_button_by_group(int group)
{
	TButton *btn;
	TAILQ_FOREACH(btn, &gui_buttons, next)
	{
		if (btn->group == group)
			delete_button_by_guid(btn->guid);
	}
	return 0;
}

void delete_button_all(void)
{
	while (TAILQ_LAST(&gui_buttons, _button_q) != NULL)
	{
		TButton *btn = TAILQ_LAST(&gui_buttons, _button_q);
		TAILQ_REMOVE(&gui_buttons, btn, next);
		FREE(btn);
	}
}

/*
 * set focused button and return
 */
static TButton *focused_button(TPoint *pt)
{
	TButton *btn, *focused_btn=NULL;

	TAILQ_FOREACH(btn, &gui_buttons, next)
	{
		if ((btn->group == gui_active_group) &&
			(pt->x >= btn->rect.left) && (pt->x <= btn->rect.right) &&
			(pt->y >= btn->rect.top) && (pt->y <= btn->rect.bottom))
		{
			focused_btn = btn;
			btn->layout_changed = !btn->focused;
			btn->focused = true;
		}
		else
		{
			btn->layout_changed = btn->focused;
			btn->focused = false;
		}
	}

	return focused_btn;
}

TButton *focused_button_by_guid(int guid,unsigned char status)
{
    TButton *btn, *focused_btn=NULL;

	TAILQ_FOREACH(btn, &gui_buttons, next)
	{
		if (btn->guid == guid)
        {
            if(status)
            {
                focused_btn = btn;
                btn->layout_changed = true;
			    btn->focused = true;
		    }else
		    {
		        btn->layout_changed = true;
			    btn->focused = false;
            }
            break;
		}

	}
	return focused_btn;
}



int get_buttons_count(int group)
{
	int count = 0;
	TButton *btn;

	TAILQ_FOREACH(btn, &gui_buttons, next)
	{
		if ((btn->group == group) || (group <= 0))
			count++;
	}

	return count;
}
/******************************************************************************
 * TGrid implement
 ******************************************************************************/

static TAILQ_HEAD(_grid_q, Grid) gui_grids;			/* grids queue */

/*
 * XXX pre-delcared
 */
static void grid_calcurate_cell_rect_internal(TGrid *grid);
static void grid_clear_cell_rect_internal(TGrid *grid, TColumn *column, TCell *cell);
static void grid_draw_cell_text_internal(TGrid *grid, TColumn *column, TCell *cell);
static TColumn *grid_new_column_internal(TGrid *grid, const int width);
static int grid_delete_column_internal(TColumn *column);
static TCell *grid_get_cell(TColumn *column, const int index);

TGrid *create_grid(TRect *rect, int rows, int cols, int guid, int group)
{
	TGrid *p_grid;
	int    i, cw;
	TRect  temp;

	/* check parameter is too small or too large
	 */
	temp.left   = 0;
	temp.top    = 0;
	temp.right  = fb_get_pixelsx();
	temp.bottom = fb_get_pixelsy();

	normalize_rect(rect);

	if ((rect->left < temp.left) || (rect->top < temp.top) ||
		(rect->right > temp.right) || (rect->bottom > temp.bottom) ||
		(rows <= 0) || (cols <= 0))
		return NULL;

	/*
	 * create mutex first
	 */

	cw = (rect->right - rect->left) / cols;
	p_grid = (TGrid *)MALLOC(sizeof(TGrid));
	if (p_grid == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}

	memset((void *)p_grid, 0, sizeof(TGrid));

	normalize_rect(rect);
	p_grid->rect.left      = rect->left;
	p_grid->rect.top       = rect->top;
	p_grid->rect.right     = rect->right;
	p_grid->rect.bottom    = rect->bottom;
	p_grid->borderwidth    = 2;
	p_grid->bordercolor    = clBLACK;
	p_grid->linecolor      = clGREY;
	p_grid->bgcolor		   = clHINT;
	p_grid->titlecolor     = clSILVER;
	p_grid->rows		   = rows;
	p_grid->cols		   = cols;
	p_grid->guid		   = guid;
	p_grid->group          = group;
	p_grid->layout_changed = true;

	/* intialize the column queue
	 */
	TAILQ_INIT(&p_grid->columns);

	/* create new columns of this grid
	 */
	for (i=0; i<cols; i++)
	{
		TColumn *p_column = grid_new_column_internal(p_grid, cw);
		if (p_column != NULL)
			p_column->index = i;
	}

	grid_calcurate_cell_rect_internal(p_grid);

	TAILQ_INSERT_TAIL(&gui_grids, p_grid, next);

	return p_grid;
}

TGrid *find_grid(int guid)
{
	TGrid *grid;

	TAILQ_FOREACH(grid, &gui_grids, next)
	{
		if (grid->guid == guid)
			return grid;
	}

	return NULL;
}

int get_grids_count(int group)
{
	int count = 0;
	TGrid *grid;

	TAILQ_FOREACH(grid, &gui_grids, next)
	{
		if ((grid->guid == group) || (group <= 0))
			count++;
	}

	return count;
}

void destroy_grid(TGrid *grid)
{
	while (TAILQ_LAST(&grid->columns, _column_q) != NULL)
	{
		TColumn *col = TAILQ_LAST(&grid->columns, _column_q);
		TAILQ_REMOVE(&grid->columns, col, next);
		grid_delete_column_internal(col);
	}

	TAILQ_REMOVE(&gui_grids, grid, next);

    /*
     * free mutex
     */
     
	FREE(grid);
	grid = NULL;
}

static void paint_grid(TGrid *grid)
{
	TRect   *grid_r;
	TColumn *p_column;
	int      i;

	if ((!grid->layout_changed) || (grid == NULL))
		return;

	/* grid border
	 */
	grid_r = &grid->rect;
    SetColor(DEFAULT_COLOR_INDEX, grid->bordercolor);
    for (i=0; i<grid->borderwidth; i++)
    {
    	DrawRect(grid_r->left+i, grid_r->top+i,
    			 grid_r->right-i, grid_r->bottom-i, DEFAULT_COLOR_INDEX);
    }

	/* grid body - loop column
	 */
	TAILQ_FOREACH(p_column, &grid->columns, next)
	{
		TCell *p_cell;

		if (!p_column->visible)
			continue;

		/* loop cell
		 */
		TAILQ_FOREACH(p_cell, &p_column->cells, next)
		{
			TRect *cell_r = &p_cell->rect;

			if (!p_cell->visible)
				continue;

			/* cell border
			 */
			if (p_cell->border != 0)
			{
				SetColor(DEFAULT_COLOR_INDEX, grid->linecolor);
				if (p_cell->border & border_left)
				{
					DrawLine(cell_r->left, cell_r->top,
							 cell_r->left, cell_r->bottom, DEFAULT_COLOR_INDEX);
				}
				if (p_cell->border & border_top)
				{
					DrawLine(cell_r->left, cell_r->top,
							 cell_r->right, cell_r->top, DEFAULT_COLOR_INDEX);
				}
				if (p_cell->border & border_right)
				{
					DrawLine(cell_r->right, cell_r->top,
							 cell_r->right, cell_r->bottom, DEFAULT_COLOR_INDEX);
				}
				if (p_cell->border & border_bottom)
				{
					DrawLine(cell_r->left, cell_r->bottom,
							 cell_r->right, cell_r->bottom, DEFAULT_COLOR_INDEX);
				}
			}

			/* column title
			 */
			if (p_cell->index == 0)
			{
				/* shadow effect
				 */
				SetColor(DEFAULT_COLOR_INDEX, p_column->fillcolor);
				if (p_column->index == 0)
					DrawLine(cell_r->left, cell_r->top,
							 cell_r->left, cell_r->bottom-1, DEFAULT_COLOR_INDEX);
				DrawLine(cell_r->left, cell_r->top,
						 cell_r->right-1, cell_r->top, DEFAULT_COLOR_INDEX);

				/* title color
				 */
				SetColor(DEFAULT_COLOR_INDEX, grid->titlecolor);
				FillRect(cell_r->left+1, cell_r->top+1,
						 cell_r->right-1, cell_r->bottom-1, DEFAULT_COLOR_INDEX);

				/* title caption
				 */
				if (p_column->title[0] != 0)
				{
			    	int  n_temp;
			    	char temp[GRID_TITLE_MAX + 1];

			    	n_temp = (cell_r->right - cell_r->left - 2) / m_asc_font_width;
			    	n_temp = n_temp < GRID_TITLE_MAX ? n_temp : GRID_TITLE_MAX;
			    	strncpy(temp, p_column->title, n_temp);
			    	temp[n_temp] = 0;

					SetFGColor(DEFAULT_COLOR_INDEX, p_column->fontcolor);
					PutStringCenter(cell_r->left + (cell_r->right - cell_r->left) / 2 + 0.5,
									cell_r->top + (cell_r->bottom - cell_r->top) / 2 + 0.5,
									temp, DEFAULT_COLOR_INDEX);
				}
			}

			/* cell text
			 */
			else
			{
				/* fill back ground
				 */
				SetColor(DEFAULT_COLOR_INDEX, p_column->fillcolor);
				FillRect(cell_r->left+1, cell_r->top+1,
						 cell_r->right-1, cell_r->bottom-1, DEFAULT_COLOR_INDEX);
				grid_draw_cell_text_internal(grid, p_column, p_cell);
			}
		}
	}

    /* grid background color - remainder
     */
	p_column = TAILQ_LAST(&grid->columns, _column_q);
	if (p_column != NULL)
	{
		/* right part */
		if (p_column->visible &&
			(p_column->rect.right < grid_r->right - grid->borderwidth))
		{
		    SetColor(DEFAULT_COLOR_INDEX, grid->bgcolor);
		    FillRect(p_column->rect.right + 1,
		    		 p_column->rect.top,
		    		 grid_r->right  - grid->borderwidth,
		    		 grid_r->bottom - grid->borderwidth, DEFAULT_COLOR_INDEX);
		}

		/* bottom part */
		if (p_column->rect.bottom < grid_r->bottom - grid->borderwidth)
		{
		    SetColor(DEFAULT_COLOR_INDEX, grid->bgcolor);
		    FillRect(grid_r->left   + grid->borderwidth,
					 p_column->rect.bottom + 1,
					 grid_r->right  - grid->borderwidth,
					 grid_r->bottom - grid->borderwidth, DEFAULT_COLOR_INDEX);
		}
	}

	grid->layout_changed = false;
}

static void paint_active_grids(void)
{
	TGrid *grid;

	TAILQ_FOREACH(grid, &gui_grids, next)
	{
		if (grid->group == gui_active_group)
			paint_grid(grid);
	}
}

void grid_set_color(TGrid *grid, unsigned bgcolor)
{
	TColumn *p_column;

	TAILQ_FOREACH(p_column, &grid->columns, next)
	{
		p_column->fillcolor = bgcolor;
	}

	grid->bgcolor = bgcolor;
}

void grid_scroll_rows(TGrid *grid, int rows)
{
	TColumn *p_column;

	if (grid == NULL)
		return;

	/* loop column
	 */
	TAILQ_FOREACH(p_column, &grid->columns, next)
	{
		TCell *p_cell;

		/* loop cell
		 */
		TAILQ_FOREACH(p_cell, &p_column->cells, next)
		{
			TCell *cell;
			int    index = p_cell->index + rows;

			if (p_cell->text[0] != 0)
			{
				grid_clear_cell_rect_internal(grid, p_column, p_cell);
				p_cell->text[0] = 0;
			}

			cell = grid_get_cell(p_column, index);
			if ((cell != NULL) && (cell->text[0] != 0))
			{
				strcpy(p_cell->text, cell->text);
				p_cell->text[strlen(cell->text)] = 0;
				cell->text[0] = 0;
				grid_draw_cell_text_internal(grid, p_column, p_cell);
			}
		}
	}
}

void grid_blank_row(TGrid *grid, int row)
{
	TColumn *p_column;

	if (grid == NULL)
		return;

	/* loop column
	 */
	TAILQ_FOREACH(p_column, &grid->columns, next)
	{
		TCell *p_cell;

		/* loop cell
		 */
		TAILQ_FOREACH(p_cell, &p_column->cells, next)
		{
			if (p_cell->index == row)
			{
				if (p_cell->text[0] != 0)
				{
					grid_clear_cell_rect_internal(grid, p_column, p_cell);
					p_cell->text[0] = 0;
				}
			}
		}
	}
}

/*
 * while colomn/cell's right/bottom is out of grid's right/bottom,
 * set it as grid's right/bottom. cell paint by it's visible.
 */
static void grid_calcurate_cell_rect_internal(TGrid *grid)
{
	TColumn *p_column;
	int      i=0, th, ch, _left, _top, b_max, r_max;
	TRect   *g_rect;

	if ((!grid->layout_changed) || (grid->rows <= 0) || (grid->cols <= 0))
		return;

	g_rect = &grid->rect;
	b_max  = g_rect->bottom - grid->borderwidth;	/* bottom margin */
	r_max  = g_rect->right  - grid->borderwidth;	/* right  margin */
	_left  = g_rect->left   + grid->borderwidth;    /* left   margin */

	/* title height */
	th = grid->titleheight;

	/* cell height */
	if (th <= 0)
	{
		ch = (g_rect->bottom - g_rect->top - grid->borderwidth*2) / (grid->rows + 1);
		th = ch;
	}
	else
		ch = (g_rect->bottom - g_rect->top - grid->borderwidth*2 - th) / grid->rows;

	/* loop column
	 */
	TAILQ_FOREACH(p_column, &grid->columns, next)
	{
		TCell *p_cell;
		int    j=0, dx;

		_top = g_rect->top + grid->borderwidth;
		dx = p_column->rect.right - p_column->rect.left;

		p_column->rect.left   = _left;
		p_column->rect.right  = _left + dx;
		p_column->rect.top    = _top;
		p_column->visible     = _left < r_max;

		/* loop cell
		 */
		TAILQ_FOREACH(p_cell, &p_column->cells, next)
		{
			int dy;

			if (j == 0) dy = th;	/* it is title */
			else dy = ch;			/* it is cell  */

			p_cell->border = 0;
			p_cell->visible = p_column->visible;

			p_cell->rect.left = _left;
			if ((_left + dx) < r_max)
			{
				p_cell->rect.right = _left + dx;
				p_cell->border |= border_right;
			}
			else
				p_cell->rect.right = r_max;

			p_cell->rect.top = _top;
			if (_top + dy < b_max)
			{
				p_cell->rect.bottom = _top + dy;
				p_cell->border |= border_bottom;
			}
			else
				p_cell->rect.bottom = b_max;

			p_cell->index = j;
			j++;
			_top += dy;
		}

		/* column's bottom equel last cell's bottom
		 */
		p_cell = TAILQ_LAST(&p_column->cells, _cell_q);
		if (p_cell != NULL)
			p_column->rect.bottom = p_cell->rect.bottom;
		else
			p_column->rect.bottom = b_max;

		p_column->index = i;
		i++;
		_left += dx;
	}

	grid->layout_changed = false;
}

/**********************************************************************
 * TCell implement
 **********************************************************************/

static TCell *grid_get_cell(TColumn *column, const int index)
{
	TCell *tmp;

	if ((index < 1) || (index >= column->cellcount))
		return NULL;

	TAILQ_FOREACH(tmp, &column->cells, next)
	{
		if (tmp->index == index)
			return tmp;
	}

	return NULL;
}

/*
 * create new cell of column
 */
static TCell *grid_new_cell_internal(TColumn *column, const int width)
{
	TCell *cell;

	cell = (TCell *)MALLOC(sizeof(TCell));
	if (cell == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}
	memset((void *)cell, 0, sizeof(TCell));

	cell->rect.right = width;

	/* append to cell-queue
	 */
	TAILQ_INSERT_TAIL(&column->cells, cell, next);

	return cell;
}

static void grid_clear_cell_rect_internal(TGrid *grid, TColumn *column, TCell *cell)
{
	TRect *rect = &cell->rect;

	if ((grid->group != gui_active_group) || (!cell->visible) ||
		(rect->right - rect->left < m_asc_font_width) ||
		(rect->bottom - rect->top < m_asc_font_height))
		return;

	/*
	 * clear back ground
	 */
	if (cell->text[0] != 0)
	{
		int nlen = strlen(cell->text);
		int x1, y1, x2, y2;

		if (column->align == align_center)
		{
			x1  = rect->left + (rect->right - rect->left) / 2 + 0.5;
			x1 -= nlen * m_asc_font_width  / 2;
			y1  = rect->top + (rect->bottom - rect->top) / 2 + 0.5;
			y1 -= m_asc_font_height / 2;
		}
		else
		{
			x1 = rect->left + 1;
			y1 = rect->top + (rect->bottom - rect->top - m_asc_font_height) / 2 + 0.5;
		}

		x2 = x1 + nlen * m_asc_font_width - 1;
		x2 = x2 < cell->rect.right - 1 ? x2 : cell->rect.right - 1;
		y2 = y1 + m_asc_font_height - 1;
		SetColor(DEFAULT_COLOR_INDEX, column->fillcolor);
		FillRect(x1, y1, x2, y2, DEFAULT_COLOR_INDEX);
	}
}

static void grid_draw_cell_text_internal(TGrid *grid, TColumn *column, TCell *cell)
{
	TRect *rect = &cell->rect;

	if ((grid->group != gui_active_group) || (!cell->visible) ||
		(rect->right - rect->left < m_asc_font_width) ||
		(rect->bottom - rect->top < m_asc_font_height))
		return;

	/*
	 * draw text
	 */
	if (cell->text[0] != 0)
	{
		char temp[CELL_TEXT_MAX + 1];
    	int  nlen;

    	nlen = (rect->right - rect->left - 2) / m_asc_font_width;
    	nlen = nlen < strlen(cell->text) ? nlen : strlen(cell->text);
    	nlen = nlen < CELL_TEXT_MAX ? nlen : CELL_TEXT_MAX;

    	strncpy(temp, cell->text, nlen);
    	temp[nlen] = 0;

		SetFGColor(DEFAULT_COLOR_INDEX, column->fontcolor);
		if (column->align == align_center)
		{
			PutStringCenter(rect->left + (rect->right - rect->left) / 2 + 0.5,
							rect->top + (rect->bottom - rect->top) / 2 + 0.5,
							temp, DEFAULT_COLOR_INDEX);
		}
		else
		{
			PutString(rect->left + 1,
					  rect->top + (rect->bottom - rect->top - m_asc_font_height) / 2 + 0.5,
					  temp, DEFAULT_COLOR_INDEX);
		}
	}
}

void grid_set_cell_text(TGrid *grid, int row, int col, const char *str)
{
	TColumn *p_column;

	if ((row < 0) || (row >= grid->rows) ||
		(col < 0) || (col >= grid->cols) || (grid == NULL))
		return;

	/* loop column
	 */
	TAILQ_FOREACH(p_column, &grid->columns, next)
	{
		TCell *p_cell;

		if (p_column->index != col)
			continue;

		/* loop cell
		 */
		TAILQ_FOREACH(p_cell, &p_column->cells, next)
		{
			TRect *rect;

			if (p_cell->index != row + 1)	/* first is title */
				continue;

			rect = &p_cell->rect;
			if (p_cell->text[0] != 0)
			{
				grid_clear_cell_rect_internal(grid, p_column, p_cell);
				p_cell->text[0] = 0;
			}

			if (str != NULL)
			{
				int nlen = strlen(str);
				nlen = nlen < CELL_TEXT_MAX ? nlen : CELL_TEXT_MAX;
				strncpy(p_cell->text, str, CELL_TEXT_MAX);
				p_cell->text[nlen] = 0;
				grid_draw_cell_text_internal(grid, p_column, p_cell);
			}

			break;
		}	/* loop cell */
		break;
	}	/* loop column */
}

/**********************************************************************
 * TColumn implement
 * cell[0] is title
 **********************************************************************/

static TColumn *grid_new_column_internal(TGrid *grid, const int width)
{
	TColumn *col;
	int      i;

	col = (TColumn *)MALLOC(sizeof(TColumn));
	if (col == NULL)
	{
		errno = ENOMEM;
		return NULL;
	}
	memset((void *)col, 0, sizeof(TColumn));

	//col->rect.left  = 0;
	col->rect.right = width;
	col->visible    = true;
	col->align      = align_left;
	col->cellcount  = grid->rows + 1;

	col->fillcolor = clHINT;
	col->fontcolor = clBLACK;

	/* append to column-queue
	 */
	TAILQ_INSERT_TAIL(&grid->columns, col, next);

	/* intialize the cell queue
	 */
	TAILQ_INIT(&col->cells);

	/* create new cells of this column
	 */
	for (i=0; i<=grid->rows; i++)
	{
		TCell *p_cell = grid_new_cell_internal(col, width);
		if (p_cell != NULL)
			p_cell->index = i;
	}

	grid->layout_changed = true;

	return col;
}

static int grid_delete_column_internal(TColumn *column)
{
	while (TAILQ_LAST(&column->cells, _cell_q) != NULL)
	{
		TCell *cell = TAILQ_LAST(&column->cells, _cell_q);
		TAILQ_REMOVE(&column->cells, cell, next);
		FREE(cell);
	}

	FREE(column);

	return 0;
}

TColumn *grid_get_column(TGrid *grid, int index)
{
	TColumn *col;

	TAILQ_FOREACH(col, &grid->columns, next)
	{
		if (col->index == index)
			return col;
	}

	return NULL;
}

void grid_set_column_index(TGrid *grid, TColumn *column, int index)
{
	TColumn *col = grid_get_column(grid, index);
	if (col == NULL)
		return;

	grid->layout_changed = true;
	/* 1st remove
	 */
	TAILQ_REMOVE(&grid->columns, column, next);
	/* 2st insert before
	 */
	TAILQ_INSERT_BEFORE(col, column, next);
	/* 3st calcurate cell
	 */
	grid_calcurate_cell_rect_internal(grid);
}

void grid_set_column_title(TColumn *column, const char *title)
{
	column->title[0] = 0;

	if (title != NULL)
	{
		int nlen = strlen(title);
		nlen = nlen < GRID_TITLE_MAX ? nlen : GRID_TITLE_MAX;
		strncpy(column->title, title, GRID_TITLE_MAX);
		column->title[nlen] = 0;
	}
}

void grid_set_column_format(TColumn *column, const char *format)
{
	column->format[0] = 0;

	if (format != NULL)
	{
		int nlen = strlen(format);
		nlen = nlen < GRID_FORMAT_MAX ? nlen : GRID_FORMAT_MAX;
		strncpy(column->format, format, GRID_FORMAT_MAX);
		column->format[nlen] = 0;
	}
}

void grid_set_column_width(TGrid *grid, int index, int width)
{
	TColumn *col = grid_get_column(grid, index);
	if ((col == NULL) || (col->rect.right - col->rect.left == width))
		return;

	grid->layout_changed = true;
	col->rect.right = col->rect.left + width;
	grid_calcurate_cell_rect_internal(grid);
}

TColumn *grid_new_column(TGrid *grid, int index, int width)
{
	TRect rect;
	TColumn *col = TAILQ_FIRST(&grid->columns);
	if (col == NULL)
		return NULL;

	rect.left   = col->rect.left;
	rect.top    = col->rect.top;
	rect.right  = rect.left + width;
	rect.bottom = col->rect.bottom;

	/* 1st create a new column
	 */
	col = grid_new_column_internal(grid, width);
	if (col == NULL)
		return NULL;

	/* 2st correct column index
	 */
	grid_set_column_index(grid, col, index);

	return col;
}

int grid_delete_column(TGrid *grid, int index)
{
	TColumn *col = grid_get_column(grid, index);
	if (col == NULL)
		return -1;

	grid->layout_changed = true;
	/* 1st remove it
	 */
	TAILQ_REMOVE(&grid->columns, col, next);

	/* 2st calcurate cell
	 */
	grid_calcurate_cell_rect_internal(grid);
	/* 3st delete column
	 */
	return grid_delete_column_internal(col);
}

/******************************************************************************
 * XXX simple gui active group - only button use this
 ******************************************************************************/

int set_gui_active_group(int group)
{
	TButton *p_btn;
	TGrid   *p_grid;

	TAILQ_FOREACH(p_btn, &gui_buttons, next)
	{
		p_btn->layout_changed = 1; // p_btn->group != group;
	}

	TAILQ_FOREACH(p_grid, &gui_grids, next)
	{
		p_grid->layout_changed = 1; // p_grid->group != group;
	}

	gui_active_group = group;

	return gui_active_group;
}

int get_gui_active_group(void)
{
	return gui_active_group;
}

/*
 * XXX must be first call before new_button().
 */
static void init_simple_gui_queue(void)
{
	font_desc_t *font_desc = get_font_desc((const unsigned char *)"A", 0);
	if (font_desc != NULL)
	{
		m_asc_font_width  = font_desc->width;
		m_asc_font_height = font_desc->height;
	}

	TAILQ_INIT(&gui_buttons);
	TAILQ_INIT(&gui_grids);
}

static void paint_objects(void)
{
	m_simple_gui_busy = true;

	paint_active_buttons();
	paint_active_grids();

	m_simple_gui_busy = false;
}

bool simple_gui_busy(void)
{
	return (bool)m_simple_gui_busy;
}

/******************************************************************************
 * objects queue
 */
void init_simple_gui_env(void)
{
    init_simple_gui_queue();
}

/******************************************************************************
 * task for simple gui, paint active group object,
 * and process touchscreen click events
 ******************************************************************************/

#if BSP_USE_OS

#if defined(OS_RTTHREAD)
extern  rt_mq_t      touch_msg_queue;
#define SLEEP        rt_thread_delay
#elif defined(OS_UCOS)
extern  OS_EVENT    *touch_msg_queue;
#define SLEEP        OSTimeDly
#elif defined(OS_FREERTOS)
extern QueueHandle_t touch_msg_queue;
#define SLEEP        vTaskDelay
#endif

static void simple_gui_task(void *arg)
{
    TPoint pt;
#if defined(XPT2046_DRV) && (TOUCHSCREEN_USE_MESSAGE)
	ts_message_t msg, *pmsg;
#endif

    SLEEP(20);
    paint_objects();
    DBG_OUT("simple_gui_task started.\r\n");
    
	for ( ; ; )
	{
		paint_objects();            /* paint buttons on screen */

		if (gui_active_group < 0)
		{
		    SLEEP(100);             /* sleep 100ms */
        	continue;
        }

#if defined(XPT2046_DRV) && (TOUCHSCREEN_USE_MESSAGE)

	    int rt;

		/*
         * 等待触摸屏消息
		 */
    #if defined(OS_RTTHREAD)
        rt = rt_mq_recv(touch_msg_queue,
                        (void *)&msg,
                        sizeof(ts_message_t),
                        50 /* RT_WAITING_FOREVER */);   // equal SLEEP(50)
        if (rt != 0)
            continue;

        pt.x = msg.x;
		pt.y = msg.y;
		
    #elif defined(OS_UCOS)

        unsigned char err;
        pmsg = (ts_message_t *)OSQPend(touch_msg_queue, 50, &err);
        if (pmsg == NULL)
            continue;

        pt.x = pmsg->x;
		pt.y = pmsg->y;

    #elif defined(OS_FREERTOS)

        rt = xQueueReceive(touch_msg_queue, &pmsg, 50);
        if (rt == errQUEUE_EMPTY)
            continue;

        pt.x = pmsg->x;
		pt.y = pmsg->y;
       
    #else // OS_NONE

    #endif
    
#elif defined(GT1151_DRV)

        SLEEP(100);

        if (!GT1151_Scan(0))
            continue;

        pt.x = tp_dev.x[0];
        pt.y = tp_dev.y[0];

        SLEEP(300);

#endif

		/* handle button click event
		 */
		TButton *btn = focused_button(&pt);
		if (btn != NULL)
		{
			draw_button(btn);

			if (btn->onclick != NULL)
			{
            	btn->onclick(0, (void *)btn);
            	// draw_button(btn);
            }
		}

	}
}

/**********************************************************************
 * start gui monitor task
 **********************************************************************/

int start_gui_monitor_task(void)
{
#if defined(OS_RTTHREAD)

    gui_thread = rt_thread_create("guithread",
                                   simple_gui_task,
                                   NULL,                // arg
                                   GUI_STK_SIZE*4,      // statck size
                                   GUI_TASK_PRIO,       // priority
                                   GUI_TASK_SLICE);     // slice ticks

    if (gui_thread == NULL)
    {
        printk("create simple gui monitor thread fail!\r\n");
		return -1;
	}

	rt_thread_startup(gui_thread);

#elif defined(OS_UCOS)

    unsigned char err;
    OSTaskCreate(simple_gui_task,
                 NULL,
        #if OS_STK_GROWTH == 1
                 (void *)&gui_stack[GUI_STK_SIZE-1],
        #else
                 (void *)&gui_stack[0],
        #endif
                 GUI_TASK_PRIO);

#elif defined(OS_FREERTOS)

    xTaskCreate(simple_gui_task,
                "guitask",
                GUI_STK_SIZE, NULL,
                GUI_TASK_PRIO, NULL);

#else

#endif

    printk("simple gui monitor thread started!\r\n");
	return 0;
}

#else // #if BSP_USE_OS

/*
 * drawing without RTOS
 */
void paint_my_simple_gui(void)
{
    paint_objects();
}

/*
 * 按钮回调函数
 */
void set_focused_button(int x, int y)
{
    TPoint p;
    TButton *btn;

    p.x = x; p.y = y;
    btn = focused_button(&p);
    
    if (btn != NULL)
    {
		if (btn->onclick != NULL)
			btn->onclick(0, (void *)btn);

        paint_active_buttons();
    }
}

#endif // #if BSP_USE_OS

#endif


