/*
 * gt1151.h
 *
 * created: 2021/3/15
 *  author: 
 */

#ifndef _GT1151_H
#define _GT1151_H
#include "bsp.h"
#ifdef __cplusplus
extern "C" {
#endif

#define GT1151_ADDRESS      0x14        // 7λ��ַ
#define GT1151_BAUDRATE     1000000

//IO�����Ĵ���
#define GT_RST  2 	// GT115��λ����
#define GT_INT  3  	// GT1151�ж�����

//GT1151 ���ּĴ�������
#define GT_CTRL_REG 	0X8040   	//GT1151���ƼĴ���
#define GT_CFGS_REG 	0X8050   	//GT1151������ʼ��ַ�Ĵ���
#define GT_CHECK_REG 	0X813C  	//GT1151У��ͼĴ���
#define GT_PID_REG 		0X8140   	//GT1151��ƷID�Ĵ���

#define GT_GSTID_REG 	0X814E   	//GT1151��ǰ��⵽�Ĵ������
#define GT_TP1_REG 		0X8150  	//��һ�����������ݵ�ַ
#define GT_TP2_REG 		0X8158		//�ڶ������������ݵ�ַ
#define GT_TP3_REG 		0X8160		//���������������ݵ�ַ
#define GT_TP4_REG 		0X8168		//���ĸ����������ݵ�ַ
#define GT_TP5_REG 		0X8170		//��������������ݵ�ַ

//-----------------------------------------------------------------------------
// I2C0-GT1151 driver operators
//-----------------------------------------------------------------------------

#include "ls1x_io.h"

#if (PACK_DRV_OPS)

extern driver_ops_t *ls1x_gt1151_drv_ops;

#define ls1x_gt1151_init(iic, arg)                  ls1x_gt1151_drv_ops->init_entry(iic, arg)
#define ls1x_gt1151_read(iic, buf, size, arg)       ls1x_gt1151_drv_ops->read_entry(iic, buf, size, arg)
#define ls1x_gt1151_write(iic, buf, size, arg)      ls1x_gt1151_drv_ops->write_entry(iic, buf, size, arg)

#else
int GT1151_init(void *bus, void *arg);
int GT1151_read(void *bus, void *buf, int size, void *arg);
int GT1151_write(void *bus, void *buf, int size, void *arg);

#define ls1x_gt1151_init(iic, arg)             GT1151_init(iic, arg)
#define ls1x_gt1151_read(iic, buf, size, arg)  GT1151_read(iic, buf, size, arg)
#define ls1x_gt1151_write(iic, buf, size, arg) GT1151_write(iic, buf, size, arg)

#endif

//-----------------------------------------------------------------------------
// user api
//-----------------------------------------------------------------------------

/*
 * ɨ�败��״̬, ��д�� tp_dev ��(touch.c/touch.h)
 */
unsigned char GT1151_Scan(unsigned char mode);

#ifdef __cplusplus
}
#endif

#endif // _GT1151_H

