/*
 * bh1750_iic.h
 *
 * created: 2022/3/2
 *  author: 
 */

#ifndef _BH1750_IIC_H
#define _BH1750_IIC_H


//IIC���в�������
void IIC_Init(void);                		//��ʼ��IIC��IO��
void SDA_OUT(void);						
void SDA_IN(void);  						
void IIC_Start(void);					//����IIC��ʼ�ź�
void IIC_Stop(void);	  					//����IICֹͣ�ź�
void IIC_Send_Byte(unsigned char txd);			//IIC����һ���ֽ�
unsigned char IIC_Read_Byte(unsigned char ack);	//IIC��ȡһ���ֽ�
unsigned char IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);						//IIC����ACK�ź�
void IIC_NAck(void);						//IIC������ACK�ź�

void IIC_Write_One_Byte(unsigned char daddr,unsigned char addr,unsigned char data);
unsigned char IIC_Read_One_Byte(unsigned char daddr,unsigned char addr);

#endif // _BH1750_IIC_H

