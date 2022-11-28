/*
 * touch.h
 *
 * created: 2021/3/15
 *  author: 
 */

/******************************************************************************
 * 用于 GT1151 竖屏触摸
 ******************************************************************************/

#ifndef _TOUCH_H
#define _TOUCH_H

#ifdef __cplusplus
extern "C" {
#endif

#define TP_PRES_DOWN 	0x80   	// 触屏被按下
#define TP_CATH_PRES 	0x40   	// 有按键按下
#define CT_MAX_TOUCH  	5     	// 电容屏支持的点数，固定为5点

// 触摸屏控制器（电容屏不需要校准）

typedef struct
{
    void (*init)(void);						// 初始化触摸屏控制器
	unsigned char (*scan)(unsigned char);	// 扫描触摸屏
	unsigned x[CT_MAX_TOUCH]; 				// 当前坐标
	unsigned y[CT_MAX_TOUCH];				// 电容屏有最多5组坐标
	unsigned char sta;						// 笔的状态
											// b7:按下1/松开0;
	                            			// b6:0-没有按键按下;1-有按键按下
											// b5:保留
											// b4~b0:电容触摸屏按下的点数(0-表示未按下;1-表示按下)
} _m_tp_dev;

extern _m_tp_dev tp_dev;	 				// 触屏控制器在touch.c里面定义

void TP_Init(void);     // 初始化

#ifdef __cplusplus
}
#endif

#endif // _TOUCH_H

