/*
 * bkrc_voice.c
 *
 * created: 2022/2/28
 *  author:
 */
#include "bkrc_voice.h"
#include "uart.h"
#include "ls1b.h"
#include "ls1b_gpio.h"
#include "ns16550.h"
#include "stdio.h"
#include "string.h"

unsigned char voice_falg = 0;		    // ����ģ�鷵��״̬
//�����ַ�������ʹ������BKRCspeak_TTS_Data("���");
void BKRCspeak_TTS(char *dat)//ȫ��������
{
    while(*dat)
    {
        if((unsigned char)*dat<0x80)
        {
            //USART2_Send_Byte(*dat);
            ls1x_uart_write(devUART4,*dat,8,NULL);
            dat++;
            delay_ms(10);
        }
        else
        {
            //USART2_Send_Byte(*dat);
            ls1x_uart_write(devUART4,*dat,8,NULL);
            //USART2_Send_Byte(*(dat+1));
            ls1x_uart_write(devUART4,*(dat+1),8,NULL);
            dat+=2;
            delay_ms(10);
        }
    }
}
//�������ֺ�����ʹ������BKRCspeak_TTS_Data(123);
void BKRCspeak_TTS_Num(unsigned int num)
{
    char str[50];
    sprintf(str,"%d",num);
    BKRCspeak_TTS(str);
}

unsigned char start_voice_dis[5]= {0xFA,0xFA,0xFA,0xFA,0xA1};
unsigned char SYN7318_Flag = 0;           // SYN7318����ʶ������ID���
unsigned char number1 = 0;                // ����ֵ1
unsigned int number2 = 0;               // ����ֵ2


/**************************************************
�������ƣ�BKRC_Voice_Extern
����˵��������ʶ����
���������	��
�� �� ֵ��	��������ID    ��������
**************************************************/
unsigned char BKRC_Voice_Extern(void)		// ����ʶ��
{


    while (1)
    {

        delay_ms(1);
        number2++;
        SYN7318_Flag = Voice_Drive();

        if (SYN7318_Flag != 0x00||number2>5000)//�жϳ�ʱ�˳�
        {
            break;
        }
    }

    return SYN7318_Flag;
}



unsigned char Voice_Drive(void)
{
    unsigned char voice_status = 0;

    if(UART4_Deal())
    {
        voice_falg = UART4_Rbuf[2];
    }
    return voice_status;
}



