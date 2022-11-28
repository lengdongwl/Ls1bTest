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

//I2C��д����
#define GT_CMD_WR 		0x14     //д����
#define GT_CMD_RD 		0x14	 //������

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

#define TP_PRES_DOWN 0x80  //����������
#define TP_CATH_PRES 0x40  //�а���������
#define CT_MAX_TOUCH  5    //������֧�ֵĵ���,�̶�Ϊ5��
/*
//������������
typedef struct
{
	unsigned int x[CT_MAX_TOUCH]; 	//��ǰ����
	unsigned int y[CT_MAX_TOUCH];		//�����������5������,����������x[0],y[0]����:�˴�ɨ��ʱ,����������,��
								            //x[4],y[4]�洢��һ�ΰ���ʱ������.
	//�Ĵ���0x814E
	unsigned char  sta;					    //�ʵ�״̬
								            //b7:����1/�ɿ�0;
	                                        //b6:0,û�а�������;1,�а�������.
								            //b5:����
								            //b4~b0:���ݴ��������µĵ���(0,��ʾδ����,1��ʾ����)
								            
//�����Ĳ���,��������������������ȫ�ߵ�ʱ��Ҫ�õ�.
//b0:0,����(�ʺ�����ΪX����,����ΪY�����TP)
//   1,����(�ʺ�����ΪY����,����ΪX�����TP)
//b1~6:����.
//b7:0,������
//   1,������
	unsigned char touchtype;
}_m_tp_dev;
*/
extern _m_tp_dev tp_dev;	 	//������������touch.c���涨��

int GT1151_WR_Reg(int reg,unsigned char *buf, int len);
int GT1151_RD_Reg(int reg,unsigned char *buf,int len);
unsigned char GT1151_Init(void);
unsigned char GT1151_Scan(unsigned char mode);
void GT1151_Test(void);



//I2C��д����
//#define GT_CMD_WR 		0X28     	//д����
//#define GT_CMD_RD 		0X29		//������

//GT1151 ���ּĴ�������
//#define GT_CTRL_REG 	0X8040   	//GT1151���ƼĴ���
//#define GT_CFGS_REG 	0X8050   	//GT1151������ʼ��ַ�Ĵ���
//#define GT_CHECK_REG 	0X813C   	//GT1151У��ͼĴ���
//#define GT_PID_REG 		0X8140   	//GT1151��ƷID�Ĵ���
#define GT_FW_REG 		0X8145   	//GT1151 IC FW�Ĵ���

//#define GT_GSTID_REG 	0X814E   	//GT1151��ǰ��⵽�Ĵ������
//#define GT_TP1_REG 		0X8150  	//��һ�����������ݵ�ַ
//#define GT_TP2_REG 		0X8158		//�ڶ������������ݵ�ַ
//#define GT_TP3_REG 		0X8160		//���������������ݵ�ַ
//#define GT_TP4_REG 		0X8168		//���ĸ����������ݵ�ַ
//#define GT_TP5_REG 		0X8170		//��������������ݵ�ַ


unsigned char GT1151_Send_Cfg(unsigned char  mode);

void checksum(void);



#endif // _GT1151Q_H

