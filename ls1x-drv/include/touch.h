/*
 * touch.h
 *
 * created: 2021/3/15
 *  author: 
 */

/******************************************************************************
 * ���� GT1151 ��������
 ******************************************************************************/

#ifndef _TOUCH_H
#define _TOUCH_H

#ifdef __cplusplus
extern "C" {
#endif

#define TP_PRES_DOWN 	0x80   	// ����������
#define TP_CATH_PRES 	0x40   	// �а�������
#define CT_MAX_TOUCH  	5     	// ������֧�ֵĵ������̶�Ϊ5��

// ������������������������ҪУ׼��

typedef struct
{
    void (*init)(void);						// ��ʼ��������������
	unsigned char (*scan)(unsigned char);	// ɨ�败����
	unsigned x[CT_MAX_TOUCH]; 				// ��ǰ����
	unsigned y[CT_MAX_TOUCH];				// �����������5������
	unsigned char sta;						// �ʵ�״̬
											// b7:����1/�ɿ�0;
	                            			// b6:0-û�а�������;1-�а�������
											// b5:����
											// b4~b0:���ݴ��������µĵ���(0-��ʾδ����;1-��ʾ����)
} _m_tp_dev;

extern _m_tp_dev tp_dev;	 				// ������������touch.c���涨��

void TP_Init(void);     // ��ʼ��

#ifdef __cplusplus
}
#endif

#endif // _TOUCH_H

