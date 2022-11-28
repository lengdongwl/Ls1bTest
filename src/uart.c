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
 **函数名：UART4_set_IO
 **函数功能：初始化uart的IO口
 **形参：无
 **返回值：无
 **说明：   UART4_RX:58 -- 数据接收
            UART4_TX:59 -- 数据发送
 *******************************************************************/
void UART4_Config_Init(void)
{
    unsigned int BaudRate = 115200;
    int r;
    
	//NS16550_SUPPORT_INT   0  关闭使用中断
    r=ls1x_uart_init(devUART4,(void *)BaudRate); //初始化串口
    if(r!=0)
    {
        fb_cons_puts("初始化URAT4失败！");
    }
    
    
    
    r=ls1x_uart_open(devUART4,NULL); //打开串口
 
    if(r!=0)
    {
        fb_cons_puts("打开URAT4失败！");
    }

    
}

//接收数据4位处理 成功返回true
bool UART4_Deal(void)
{
    int count=0,i;
    //接收数据
    count = ls1x_uart_read(devUART4,UART4_Rbuf,Rbuf_size,NULL); //arg 参数为非 0 时将阻塞查询模式
    if(count > 0)
    {
#ifdef BKRC_VOICE //syn7318语音模块
        if(UART4_Rbuf[0]==0x55 && UART4_Rbuf[1]== 0x02)
        {
            return true;
        }

#else //小创语音模块
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


//串口发送单字符
void UART4_Send(char Send_Data)
{
	char sbuf[1];
	sbuf[0] = Send_Data;
	ls1x_uart_write(devUART4, sbuf, 1, NULL);
}

//串口发送字符字符串
void UART4_Send_String(char *Send_Data, int Size)
{
	ls1x_uart_write(devUART4, Send_Data, Size, NULL);
}





/**********************************************************************
 * 函 数 名 ：  小创语音 12秒超时退出
 * 参    数 ：  无
 * 返 回 值 ：  语音编号
 * 全局变量 ：  无
 * 备    注 ：  无
*****************************************************************/
uint8_t XiaoChuang_ASR(void)
{
	uint16_t Counter_Fifo = 0;
	uint8_t Temp_flag = 0,ret = 0x00;


	/*发送语音识别*/
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

			/*重新发送语音识别*/
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
 * @description: 播报0-9
 * @param {int} number
 * @return {*}
 */
void XiaoChuang_PlayNUMbit(int number)
{
/*
A0::零:5503A000
A1::一:5503A100
A2::二:5503A200
A3::三:5503A300
A4::四:5503A400
A5::五:5503A500
A6::六:5503A600
A7::七:5503A700
A8::八:5503A800
A9::九:5503A900
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
 * 函 数 名 ：  小创语音播报数字 支持负数
 * 参    数 ：  语音数值 -999 到 999
 * 返 回 值 ：  无
 * 全局变量 ：  无
 * 备    注 ：
*****************************************************************/
void XiaoChuang_PlayNUM(int number)
{	int t = 500; //语音合成间隔/us 没有间隔会造成阻塞
	int buf = number;
	int len = 1;
	int bufARR[20];//将数据存放至数组
	if(number>9 || number<-9)
	{
		while (buf/=10)//计算数字位数
		{
			if(len==1)//读出各位
			{
				bufARR[len]=number%10;
			}else //读出中间位
			{
				bufARR[len]=pow(10,(len-1));
				bufARR[len]=number/bufARR[len]%10;
			}

			//printf("bufARR[%d]=%d\n",len,bufARR[len]);
			len++;
		}
		if(len>1)
		{
			//读出末位
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
		if(bufARR[2]>1)//防止播报一十
		{
			XiaoChuang_PlayNUMbit(bufARR[2]);
			delay_us(t);
		}
		UART4_Send(0xAA);//十
		delay_us(t);
		if(bufARR[1]!=0)//防止播报十零
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
			UART4_Send(0xAA);//十
			delay_us(t);
			if(bufARR[1]!=0)//防止播报十零
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
		if(bufARR[3]!=0)//千
		{
			XiaoChuang_PlayNUMbit(bufARR[3]);
			UART4_Send(0xAB);
		}//n0nn

		XiaoChuang_PlayNUMbit(bufARR[2]);
		UART4_Send(0xAA);//十
		if(bufARR[1]!=0)//防止播报十零
		{
			XiaoChuang_PlayNUMbit(bufARR[1]);
		}
		break;
	case 5:
		XiaoChuang_PlayNUMbit(bufARR[5]);//万
		UART4_Send(0xAD);
		XiaoChuang_PlayNUMbit(bufARR[4]);
		UART4_Send(0xAC);
		XiaoChuang_PlayNUMbit(bufARR[3]);
		UART4_Send(0xAB);
		XiaoChuang_PlayNUMbit(bufARR[2]);
		UART4_Send(0xAA);//十
		if(bufARR[1]!=0)//防止播报十零
		{
			XiaoChuang_PlayNUMbit(bufARR[1]);
		}
		break;
	case 6:
		if(bufARR[6]>1)
		{
			XiaoChuang_PlayNUMbit(bufARR[6]);
		}
		UART4_Send(0xAA);//十
		XiaoChuang_PlayNUMbit(bufARR[5]);//万
		UART4_Send(0xAD);
		XiaoChuang_PlayNUMbit(bufARR[4]);
		UART4_Send(0xAC);
		XiaoChuang_PlayNUMbit(bufARR[3]);
		UART4_Send(0xAB);
		XiaoChuang_PlayNUMbit(bufARR[2]);
		UART4_Send(0xAA);//十
		if(bufARR[1]!=0)//防止播报十零
		{
			XiaoChuang_PlayNUMbit(bufARR[1]);
		}
		break;*/
	}


}
