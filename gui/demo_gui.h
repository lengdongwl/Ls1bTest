/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * demo_gui.h
 *
 *  Created on: 2015-1-26
 *      Author: Bian
 */

#ifndef _DEMO_GUI_H_
#define _DEMO_GUI_H_
#include ".\simple-gui\simple_gui.h"
#ifdef	__cplusplus
extern "C" {
#endif
extern char func_task_status[20][10];//����״̬��־



int clear_screen(void);//�����Ļ
void start_my_gui(void);//����ui
void set_objects_active_group_mainUI(); //������ui
int get_objects_active_group(void);//��ȡ���������id
//void gui_drawtext_in_grid(int row, int col, const char *str);

#ifdef	__cplusplus
}
#endif

#endif /* _DEMO_GUI_H_ */
