/*
 * mylib.h
 *
 * created: 2022/11/16
 *  author: 
 */

#ifndef _MYLIB_H
#define _MYLIB_H
#include "stdint.h"
#include "stdbool.h"
//IO口操作宏定义
/*
#define BITBAND(addr, bitnum) LS1B_GPIO_OUT(addr) = bitnum
//(bitnum? LS1B_GPIO_OUT(((int register)(ioNum / 32))) |= ((int register)(1 << (ioNum % 32))):LS1B_GPIO_OUT(((int register)(ioNum / 32))) &= ~((int register)(1 << (ioNum % 32)))) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
#define PXout(val) BIT_ADDR(1,val)
*/

#endif // _MYLIB_H

