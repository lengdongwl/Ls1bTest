/*
 * gt1151q.h
 *
 * created: 2021/5/19
 *  author: 
 */

#ifndef _GT1151Q_H
#define _GT1151Q_H
#include "touch.h"
#define GT_RST 2
#define GT_INT 3
#define GT1151Q_BAUDRATE 100000

//I2C读写命令
#define GT_CMD_WR 		0x14     //写命令
#define GT_CMD_RD 		0x14	 //读命令

//GT1151 部分寄存器定义
#define GT_CTRL_REG 	0X8040   	//GT1151控制寄存器
#define GT_CFGS_REG 	0X8050   	//GT1151配置起始地址寄存器
#define GT_CHECK_REG 	0X813C  	//GT1151校验和寄存器
#define GT_PID_REG 		0X8140   	//GT1151产品ID寄存器

#define GT_GSTID_REG 	0X814E   	//GT1151当前检测到的触摸情况
#define GT_TP1_REG 		0X8150  	//第一个触摸点数据地址
#define GT_TP2_REG 		0X8158		//第二个触摸点数据地址
#define GT_TP3_REG 		0X8160		//第三个触摸点数据地址
#define GT_TP4_REG 		0X8168		//第四个触摸点数据地址
#define GT_TP5_REG 		0X8170		//第五个触摸点数据地址

#define TP_PRES_DOWN 0x80  //触屏被按下
#define TP_CATH_PRES 0x40  //有按键按下了
#define CT_MAX_TOUCH  5    //电容屏支持的点数,固定为5点
/*
//触摸屏控制器
typedef struct
{
	unsigned int x[CT_MAX_TOUCH]; 	//当前坐标
	unsigned int y[CT_MAX_TOUCH];		//电容屏有最多5组坐标,电阻屏则用x[0],y[0]代表:此次扫描时,触屏的坐标,用
								            //x[4],y[4]存储第一次按下时的坐标.
	//寄存器0x814E
	unsigned char  sta;					    //笔的状态
								            //b7:按下1/松开0;
	                                        //b6:0,没有按键按下;1,有按键按下.
								            //b5:保留
								            //b4~b0:电容触摸屏按下的点数(0,表示未按下,1表示按下)
								            
//新增的参数,当触摸屏的左右上下完全颠倒时需要用到.
//b0:0,竖屏(适合左右为X坐标,上下为Y坐标的TP)
//   1,横屏(适合左右为Y坐标,上下为X坐标的TP)
//b1~6:保留.
//b7:0,电阻屏
//   1,电容屏
	unsigned char touchtype;
}_m_tp_dev;
*/
extern _m_tp_dev tp_dev;	 	//触屏控制器在touch.c里面定义

int GT1151_WR_Reg(int reg,unsigned char *buf, int len);
int GT1151_RD_Reg(int reg,unsigned char *buf,int len);
unsigned char GT1151_Init(void);
unsigned char GT1151_Scan(unsigned char mode);
void GT1151_Test(void);



//I2C读写命令
//#define GT_CMD_WR 		0X28     	//写命令
//#define GT_CMD_RD 		0X29		//读命令

//GT1151 部分寄存器定义
//#define GT_CTRL_REG 	0X8040   	//GT1151控制寄存器
//#define GT_CFGS_REG 	0X8050   	//GT1151配置起始地址寄存器
//#define GT_CHECK_REG 	0X813C   	//GT1151校验和寄存器
//#define GT_PID_REG 		0X8140   	//GT1151产品ID寄存器
#define GT_FW_REG 		0X8145   	//GT1151 IC FW寄存器

//#define GT_GSTID_REG 	0X814E   	//GT1151当前检测到的触摸情况
//#define GT_TP1_REG 		0X8150  	//第一个触摸点数据地址
//#define GT_TP2_REG 		0X8158		//第二个触摸点数据地址
//#define GT_TP3_REG 		0X8160		//第三个触摸点数据地址
//#define GT_TP4_REG 		0X8168		//第四个触摸点数据地址
//#define GT_TP5_REG 		0X8170		//第五个触摸点数据地址


unsigned char GT1151_Send_Cfg(unsigned char  mode);

void checksum(void);



#endif // _GT1151Q_H

