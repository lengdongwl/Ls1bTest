/*
 * bh1750_iic.h
 *
 * created: 2022/3/2
 *  author: 
 */

#ifndef _BH1750_IIC_H
#define _BH1750_IIC_H


//IIC所有操作函数
void IIC_Init(void);                		//初始化IIC的IO口
void SDA_OUT(void);						
void SDA_IN(void);  						
void IIC_Start(void);					//发送IIC开始信号
void IIC_Stop(void);	  					//发送IIC停止信号
void IIC_Send_Byte(unsigned char txd);			//IIC发送一个字节
unsigned char IIC_Read_Byte(unsigned char ack);	//IIC读取一个字节
unsigned char IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);						//IIC发送ACK信号
void IIC_NAck(void);						//IIC不发送ACK信号

void IIC_Write_One_Byte(unsigned char daddr,unsigned char addr,unsigned char data);
unsigned char IIC_Read_One_Byte(unsigned char daddr,unsigned char addr);

#endif // _BH1750_IIC_H

