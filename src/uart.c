/*
 * uart.c
 *
 * created: 2021/4/30
 *  author:
 */
#include "uart.h"
#include "ls1b.h"
#include "ls1b_gpio.h"
#include "ns16550.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "math.h"




char buff[256];

char UART4_Rbuf[Rbuf_size];
TaskHandle_t uart4_task_handle;
unsigned char uart4_task_status = 0;
/*******************************************************************
 **��������UART4_set_IO
 **�������ܣ���ʼ��uart��IO��
 **�βΣ���
 **����ֵ����
 **˵����   UART4_RX:58 -- ���ݽ���
            UART4_TX:59 -- ���ݷ���
 *******************************************************************/
void UART4_Config_Init(void)
{
    unsigned int BaudRate = 115200;
    int r;
    
	//NS16550_SUPPORT_INT   0  �ر�ʹ���ж�
    r=ls1x_uart_init(devUART4,(void *)BaudRate); //��ʼ������
    if(r!=0)
    {
        fb_cons_puts("��ʼ��URAT4ʧ�ܣ�");
    }
    
    
    
    r=ls1x_uart_open(devUART4,NULL); //�򿪴���
 
    if(r!=0)
    {
        fb_cons_puts("��URAT4ʧ�ܣ�");
    }

    
}

//��������4λ���� �ɹ�����true
bool UART4_Deal(void)
{
    int count=0,i;
    //��������
    count = ls1x_uart_read(devUART4,UART4_Rbuf,Rbuf_size,NULL); //arg ����Ϊ�� 0 ʱ��������ѯģʽ
    if(count > 0)
    {
#ifdef BKRC_VOICE //syn7318����ģ��
        if(UART4_Rbuf[0]==0x55 && UART4_Rbuf[1]== 0x02)
        {
            return true;
        }

#else //С������ģ��
        if(UART4_Rbuf[0]==0x55 && UART4_Rbuf[1]== 0x02 && UART4_Rbuf[3]==0)
        {
            return true;
        }
#endif
    }else
    {
        for(i=0;i<Rbuf_size;i++)
        {
            UART4_Rbuf[i]=0;
        }
    }
    return false;
}


//���ڷ��͵��ַ�
void UART4_Send(char Send_Data)
{
	char sbuf[1];
	sbuf[0] = Send_Data;
	ls1x_uart_write(devUART4, sbuf, 1, NULL);
}

//���ڷ����ַ��ַ���
void UART4_Send_String(char *Send_Data, int Size)
{
	ls1x_uart_write(devUART4, Send_Data, Size, NULL);
}





/**********************************************************************
 * �� �� �� ��  С������ 12�볬ʱ�˳�
 * ��    �� ��  ��
 * �� �� ֵ ��  �������
 * ȫ�ֱ��� ��  ��
 * ��    ע ��  ��
*****************************************************************/
uint8_t XiaoChuang_ASR(void)
{
	uint16_t Counter_Fifo = 0;
	uint8_t Temp_flag = 0,ret = 0x00;


	/*��������ʶ��*/
	UART4_Send(0xFA);
	UART4_Send(0xFA);
	UART4_Send(0xFA);
	UART4_Send(0xFA);
	UART4_Send(0xA1);

	while (1)
	{
        if(UART4_Deal())
        {
            ret = UART4_Rbuf[2];
            break;
        }
		delay_ms(100);
		Counter_Fifo += 1;
		if (Counter_Fifo > 30)
		{
			Counter_Fifo = 0;
			if (Temp_flag >= 4)
			{
				break;
			}
			Temp_flag += 1;

			/*���·�������ʶ��*/
			UART4_Send(0xFA);
			UART4_Send(0xFA);
			UART4_Send(0xFA);
			UART4_Send(0xFA);
			UART4_Send(0xA1);
		}
	}

	return ret;
}


/**
 * @description: ����0-9
 * @param {int} number
 * @return {*}
 */
void XiaoChuang_PlayNUMbit(int number)
{
/*
A0::��:5503A000
A1::һ:5503A100
A2::��:5503A200
A3::��:5503A300
A4::��:5503A400
A5::��:5503A500
A6::��:5503A600
A7::��:5503A700
A8::��:5503A800
A9::��:5503A900
*/
	UART4_Send(0xA0+(number));
	/*if(number<0)
	{
		UART4_Send(0xA0+(number*-1));
	}else
	{

	}*/
}
/**********************************************************************
 * �� �� �� ��  С�������������� ֧�ָ���
 * ��    �� ��  ������ֵ -999 �� 999
 * �� �� ֵ ��  ��
 * ȫ�ֱ��� ��  ��
 * ��    ע ��
*****************************************************************/
void XiaoChuang_PlayNUM(int number)
{	int t = 500; //�����ϳɼ��/us û�м�����������
	int buf = number;
	int len = 1;
	int bufARR[20];//�����ݴ��������
	if(number>9 || number<-9)
	{
		while (buf/=10)//��������λ��
		{
			if(len==1)//������λ
			{
				bufARR[len]=number%10;
			}else //�����м�λ
			{
				bufARR[len]=pow(10,(len-1));
				bufARR[len]=number/bufARR[len]%10;
			}

			//printf("bufARR[%d]=%d\n",len,bufARR[len]);
			len++;
		}
		if(len>1)
		{
			//����ĩλ
			bufARR[len]=pow(10,(len-1));
			bufARR[len]=number/bufARR[len];
			//printf("bufARR[%d]=%d\n",len,bufARR[len]);
		}
	}else
	{
		bufARR[len]=number;
	}
	//printf("len=%d\n",len);

	//1 2345 6 7 8 9
	switch(len)
	{
	case 1:
		XiaoChuang_PlayNUMbit(bufARR[1]);
		break;
	case 2:
		if(bufARR[2]>1)//��ֹ����һʮ
		{
			XiaoChuang_PlayNUMbit(bufARR[2]);
			delay_us(t);
		}
		UART4_Send(0xAA);//ʮ
		delay_us(t);
		if(bufARR[1]!=0)//��ֹ����ʮ��
		{
			XiaoChuang_PlayNUMbit(bufARR[1]);
			delay_us(t);
		}
		break;
	case 3://100   320  409    111
		XiaoChuang_PlayNUMbit(bufARR[3]);
		delay_us(t);
		UART4_Send(0xAB);
		delay_us(t);
		if(bufARR[2]!=0)//110 111
		{
			XiaoChuang_PlayNUMbit(bufARR[2]);
			delay_us(t);
			UART4_Send(0xAA);//ʮ
			delay_us(t);
			if(bufARR[1]!=0)//��ֹ����ʮ��
			{
				XiaoChuang_PlayNUMbit(bufARR[1]);
				delay_us(t);
			}
		}else//101
		{
			if(bufARR[2]==0 && bufARR[1]!=0)//x0x
			{
				XiaoChuang_PlayNUMbit(bufARR[2]);
				delay_us(t);
				XiaoChuang_PlayNUMbit(bufARR[1]);
				delay_us(t);
			}
		}
		break;
	/*case 4:
		XiaoChuang_PlayNUMbit(bufARR[4]);
		UART4_Send(0xAC);
		if(bufARR[3]!=0)//ǧ
		{
			XiaoChuang_PlayNUMbit(bufARR[3]);
			UART4_Send(0xAB);
		}//n0nn

		XiaoChuang_PlayNUMbit(bufARR[2]);
		UART4_Send(0xAA);//ʮ
		if(bufARR[1]!=0)//��ֹ����ʮ��
		{
			XiaoChuang_PlayNUMbit(bufARR[1]);
		}
		break;
	case 5:
		XiaoChuang_PlayNUMbit(bufARR[5]);//��
		UART4_Send(0xAD);
		XiaoChuang_PlayNUMbit(bufARR[4]);
		UART4_Send(0xAC);
		XiaoChuang_PlayNUMbit(bufARR[3]);
		UART4_Send(0xAB);
		XiaoChuang_PlayNUMbit(bufARR[2]);
		UART4_Send(0xAA);//ʮ
		if(bufARR[1]!=0)//��ֹ����ʮ��
		{
			XiaoChuang_PlayNUMbit(bufARR[1]);
		}
		break;
	case 6:
		if(bufARR[6]>1)
		{
			XiaoChuang_PlayNUMbit(bufARR[6]);
		}
		UART4_Send(0xAA);//ʮ
		XiaoChuang_PlayNUMbit(bufARR[5]);//��
		UART4_Send(0xAD);
		XiaoChuang_PlayNUMbit(bufARR[4]);
		UART4_Send(0xAC);
		XiaoChuang_PlayNUMbit(bufARR[3]);
		UART4_Send(0xAB);
		XiaoChuang_PlayNUMbit(bufARR[2]);
		UART4_Send(0xAA);//ʮ
		if(bufARR[1]!=0)//��ֹ����ʮ��
		{
			XiaoChuang_PlayNUMbit(bufARR[1]);
		}
		break;*/
	}


}
