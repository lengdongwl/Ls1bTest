/*
 * bh1750.c
 *
 * created: 2022/3/2
 *  author:
 */

#include "ls1b.h"
#include "ls1b_gpio.h"
#include "bsp.h"
#include "bh1750.h"
#include "ls1x_i2c_bus.h"
  #include "ls1x_fb.h"
float result_lx=0;
unsigned char BUF[2]= {0};
unsigned int result1750=0;
extern unsigned char tmp_buf[16];

/**************************************************************
*��  �ܣ��˿ڳ�ʼ��
*��  ��: ��
*����ֵ: ��
**************************************************************/
void BH1750_Init(void)
{

}

/**************************************************************
*��  �ܣ������豸��ַ
*��  ��: ��
*����ֵ: ��
**************************************************************/
#define BH1750_BAUDRATE        1000000
void Cmd_Write_BH1750(unsigned char cmd)
{
    unsigned char data[2]= {0};
    data[0]=cmd;

    ls1x_i2c_send_start(busI2C0,BH1750_Addr);//��ʼ�ź�
    ls1x_i2c_send_addr(busI2C0,BH1750_Addr,0);	//�����豸��ַ+д�ź�
    ls1x_i2c_write_bytes(busI2C0, data, 1);	//�ڲ��Ĵ�����ַ
    ls1x_i2c_send_stop(busI2C0,BH1750_Addr);	//����ֹͣ�ź�
    delay_ms(5);
}


/**************************************************************
*��  �ܣ�����һ��H�ֱ���ģʽ
*��  ��: ��
*����ֵ: ��
**************************************************************/
void Start_BH1750(void)
{
    Cmd_Write_BH1750(BH1750_ON);	 		//power on
    Cmd_Write_BH1750(BH1750_RSET);		//clear
    Cmd_Write_BH1750(BH1750_ONE);  		//һ��H�ֱ���ģʽ������120ms��֮���Զ��ϵ�ģʽ
}

/**************************************************************
*��  �ܣ��������ź�
*��  ��: ��
*����ֵ: ��
**************************************************************/
void Read_BH1750(void)
{
    ls1x_i2c_send_start(busI2C0, BH1750_Addr);//��ʼ�ź�
    ls1x_i2c_send_addr(busI2C0,BH1750_Addr,1);//�����豸��ַ�Ӷ��ź�
    ls1x_i2c_read_bytes(busI2C0, BUF, 2);//��ȡ�����ֽ�����
    ls1x_i2c_send_stop(busI2C0,BH1750_Addr);//����ֹͣ�ź�
}

/**************************************************************
*��  �ܣ��ϳɹ�������
*��  ��: ��
*����ֵ: ��
**************************************************************/
void Convert_BH1750(void)
{
    result1750=BUF[0];
    result1750=(result1750<<8)+BUF[1];		//�ϳ����ݣ�����������
    result1750=(float)result1750/1.2;

}

/**************************************************************
*��  �ܣ���ʾ��������
*��  ��: ��
*����ֵ: ��
**************************************************************/
uint16_t BH1750_Test(void)
{
    unsigned char buf[20] = {0};
    unsigned int lx = 0;
    Start_BH1750();						//power on
    delay_ms(120);						//��ʱ120ms
    Read_BH1750();						//������ȡ��� ��BUF����
    Convert_BH1750();					//ת�������result_lx
    //sprintf((char *)buf,"%d lux", result1750);
    //fb_textout(160, 70, buf);
    delay_ms(200);
    //fb_fillrect(160, 70, 480, 106, cidxBLACK);
    return result1750;
}


