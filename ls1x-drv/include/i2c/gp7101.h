/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * gp7101.h
 *
 * created: 2020/9/11
 *  author: Bian
 */

#ifndef _GP7101_H
#define _GP7101_H

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * brightness: 0~100
 */
int set_lcd_brightness(int brightpercent);

#ifdef	__cplusplus
}
#endif

#endif // _GP7101_H

