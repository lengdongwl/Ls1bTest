/*
 * mytask.c
 *
 * created: 2022/11/23
 *  author:
 */

#include "mytask.h"
#include "mylib.h"
#include "bsp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ls1x_fb.h"


#include "FreeRTOS.h"
#include "task.h"

#include ".\gui\demo_gui.h"
/* 板上驱动*/
#include "led.h"
#include "beep.h"
#include "SMG.h"
#include "bh1750.h"
#include "adc.h"
#include "hmc5883l_drv.h"
#include "motor_drv.h"
#include "uart.h"
#include "ultrasonic_ranging_drv.h"
#include "fan_resistance_control_drv.h"
#include "mytask.h"
#include "bh1750.h"
#include "my_img.h"
#include "queue.h"
/******************************************************************************
 * task Hardware
 ******************************************************************************/


void func_task_HMC5883L(void* arg)
{
    volatile int x,y;
    volatile char buf[50];
    volatile double temp=0;
    volatile char *direction;
    printk("func_task_HMC5883L thread started!\n");
    x = 500;
    y = 42+20;
    while(1)
    {
        temp = HCM5883L_Get_Angle();
        if (temp >= 22 && temp <= 67)
        {
            direction = "东北";
        }
        else if (temp >= 68  && temp <= 111)
        {
            direction = "东";
        }
        else if (temp >= 112 && temp <= 157)
        {
            direction = "东南";
        }
        else if (temp >= 158 && temp <= 201)
        {
            direction = "南";
        }
        else if (temp >= 202 && temp <= 247)
        {
            direction = "西南";
        }
        else if (temp >= 248 && temp <= 291)
        {
            direction = "西";
        }
        else if (temp >= 292 && temp <= 337)
        {
            direction = "西北";
        }
        else
        {
            direction = "北";

        }




        sprintf(buf,"HCM5883L:%.2f°%s",temp,direction);
        fb_fillrect(x + strlen("HCM5883L:")*8,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);

        delay_ms(10);
    }
}

void func_task_ultrasonic(void* arg)
{
    volatile int x,y;
    volatile char buf[50];
    printk("func_task_ultrasonic thread started!\n");
    x = 500;
    y = 42*2+20*2;
    while(1)
    {
        sprintf(buf,"超声波:%.2f cm",Ultrasonic_Get_Dist());
        fb_fillrect(x + strlen("超声波:")*8,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);

        delay_ms(200);
    }
}

void func_task_tempratrue(void* arg)
{
    volatile int x,y;
    volatile char buf[50];
    printk("func_task_tempratrue thread started!\n");
    x = 500;
    y = 42*3+20*3;
    while(1)
    {
        sprintf(buf,"温度:%.2f ℃",adc_get_temp());
        fb_fillrect(x + strlen("温度:")*8,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);
        delay_ms(400);
    }
}

void func_task_bh1750(void* arg)
{
    volatile int x,y;
    volatile char buf[50];
    printk("func_task_bh1750 thread started!\n");
    x = 500;
    y = 42*4+20*4;
    while(1)
    {
        sprintf(buf,"bh1750:%d lx",BH1750_Test());
        fb_fillrect(x + strlen("bh1750:")*8,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);
        delay_ms(400);
    }
}

void func_task_YY(void* arg)
{
    volatile int x,y;
    volatile char flag=0;
    volatile char buf[50];
    printk("func_task_YY thread started!\n");
    x = 400;
    y = 50;
    fb_textout(x,y,"开始语音识别");
    fb_fillrect(x + 12*8,y,x+200,y+16,cidxSILVER);
    while(func_task_status[4][3])
    {
        flag = XiaoChuang_ASR();
        
        /*
实践锻炼能力::实践锻炼能力:55022300
比赛彰显才智::比赛彰显才智:55022400
技能成就人生::技能成就人生:55022500
人才改变世界::人才改变世界:55022600
        */
        
        switch(flag)
        {
            case 0x17:
                func_task_status[4][3]=0;
                break;
            case 0x12:
                RGB_Set(RGB_COLOR_WHITE);
                break;
            case 0x13:
                RGB_Set(RGB_COLOR_BLACK);
                break;
            case 0x23:
                
                fb_fillrect(x ,y+16*2,x+200,y+16*3,cidxSILVER);
                fb_textout(x,y+16*2,"实践锻炼能力");
                break;
            case 0x24:
               
                fb_fillrect(x ,y+16*2,x+200,y+16*3,cidxSILVER);
                fb_textout(x,y+16*2,"比赛彰显才智");
                break;
            case 0x25:
                
                fb_fillrect(x ,y+16*2,x+200,y+16*3,cidxSILVER);
                fb_textout(x,y+16*2,"技能成就人生");
                break;
            case 0x26:
              
                fb_fillrect(x ,y+16*2,x+200,y+16*3,cidxSILVER);
                fb_textout(x,y+16*2,"人才改变世界");
                break;
        }
        
        sprintf(buf,"识别结果:0x%X",flag);
        fb_fillrect(x + 9*8,y+16,x+200,y+16+16,cidxSILVER);
        fb_textout(x,y+16,buf);
        delay_ms(10);
    }
    fb_fillrect(x,y,x+200,y+16,cidxSILVER);
    fb_fillrect(x,y+16,x+200,y+16+16,cidxSILVER);
    vTaskDelete(NULL);//结束本任务

}



void func_task_SMG1()
{
    int i;
    /*5秒倒计时*/
    SMG_DisplayP(0x11,0,5,0x11,1); 
    delay_ms(1000);
    SMG_DisplayP(0x11,0,4,0x11,1);
    delay_ms(1000);
    SMG_DisplayP(0x11,0,3,0x11,1);
    delay_ms(1000);
    SMG_DisplayP(0x11,0,2,0x11,1);
    delay_ms(1000);
    SMG_DisplayP(0x11,0,1,0x11,1);
    delay_ms(1000);
    SMG_DisplayP(0x11,0,0,0x11,1);
    delay_ms(1000);

    for(i=0; i<3; i++)
    {

        SMG_DisplayP(0x11,0x0F,0x0F,0x11,1); /* -FF- */
        BEEP_On();
        RGB_Set(RGB_COLOR_RED);
        delay_ms(500);
        SMG_DisplayP(-1,-1,-1,-1,0);
        BEEP_Off();
        RGB_Set(RGB_COLOR_BLACK);
        delay_ms(500);
    }
}

void func_task_SMG2()
{
    SMG_DisplayP(1,-1,-1,-1,1);
    delay_ms(1000);
    SMG_DisplayP(1,2,-1,-1,1);
    delay_ms(1000);
    SMG_DisplayP(1,2,3,-1,1);
    delay_ms(1000);
    SMG_DisplayP(1,2,3,4,1);

}

/*风扇控制温度*/
void func_task_Fun(void *arg)
{
    volatile char speed=0;
    volatile int temp = 35;//温度指定范围
    volatile float adc,adc2;
    printk("func_task_Fun thread started!\n");
    
    while(1)
    {
        adc=adc_get_temp();

        if((temp)>adc && speed>0) //温度低 减速
        {
            Fan_Control(--speed);
        }
        printk("temp=%.2f,r=%.2f,speed=%d\n",adc2,-1*(adc-adc2),speed);
        if((temp)<adc && speed<100)//温度高 加速
        {

           Fan_Control(++speed);
        }
        
        delay_ms(1);
        
        
        
    }
}


void func_task_Motor(void *arg)
{
    volatile int x,y;
    volatile unsigned int buf_count=0;
    volatile char buf[50];
    printk("func_task_Motor thread started!\n");
    x = 400;
    y = 42*2+20*2;
    fb_fillrect(x,y,x+200,y+16,cidxSILVER);

    for(;;)
    {
        Motor_count_init();//重置count
        buf_count = Motor_get_count();
        delay_ms(1000);
        /*rpm 表示电机每分钟旋转的次数） 每秒次数x60 */
        sprintf(buf,"当前电机转速： %d rpm",(Motor_get_count()-buf_count)*60);
        fb_fillrect(x+8*13,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);
        //delay_ms(10);
        taskYIELD();
    }


}


void func_task_img()
{
    //fb_cons_clear();//清屏
    display_pic(400,0,120,120,gImage_1);
}

void func_task_color()
{
    int x,y;
    x=fb_get_pixelsx();
    y=fb_get_pixelsy();
    fb_fillrect(0,0,x,y,cidxRED);
    delay_ms(1000);
    fb_fillrect(0,0,x,y,cidxGREEN);
    delay_ms(1000);
    fb_fillrect(0,0,x,y,cidxBLUE);
    delay_ms(1000);
    clear_screen();
    set_objects_active_group_mainUI();

}


void func_task_Other1(void *arg)
{
    volatile int x,y;
    volatile char buf[50];
    volatile float distance;

    printk("func_task_Other1 thread started!\n");
    x = 500;
    y = 42;
    while(1)
    {
        distance = Ultrasonic_Get_Dist();
        sprintf(buf,"距离:%.2f cm",distance);
        fb_fillrect(x + 5*8,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);
        if(distance > 20)
        {
            Motor_set_pwm(0);//占空比越小 速度越大
        }else if(distance < 20 && distance >= 10)
        {
            Motor_set_pwm(30-distance);//电机转速随被测距离较小而减小
        }else if(distance < 10)
        {
            Motor_set_pwm(100);
        }
        delay_ms(200);


    }
}








#define ADC_bit 1000//((1<<12)-1) //12位adc //y的最大范围
#define WAVE_HEIGHT 300
#define WAVE_WIDTH 480
#define WAVE_SHOW_SX 300 //波形显示范围
#define WAVE_SHOW_SY 0
#define WAVE_SHOW_EX WAVE_SHOW_SX + WAVE_WIDTH
#define WAVE_SHOW_EY WAVE_SHOW_SY + WAVE_HEIGHT

QueueHandle_t ADCQueueHandle;
void func_task_ADC(void *arg)
{
    unsigned short  sbuf;
    ADCQueueHandle = xQueueCreate(  10,  sizeof(int));

    printk("func_task_ADC thread started!\n");
    while(1)
    {

        sbuf = adc_get(ADC1);
        //sbuf = BH1750_Test();
        
        //printk("%d\n",sbuf);
        xQueueSend(ADCQueueHandle,&sbuf,5);
        taskYIELD();
    }
}


void func_task_Wave(void *arg)
{
    int y,n,i=0;
    unsigned short buf;
    unsigned short buf_arr[WAVE_WIDTH];
    printk("func_task_Wave thread started!\n");

    //绘制矩形框
    fb_drawrect(WAVE_SHOW_SX-1,
                WAVE_SHOW_SY,
                WAVE_SHOW_EX+1,
                WAVE_SHOW_EY+1,
                cidxBLACK);
                

    while(1)
    {
        xQueueReceive(ADCQueueHandle,&buf,portMAX_DELAY);//等待队列数据
        y=WAVE_SHOW_SY+buf/(ADC_bit/WAVE_HEIGHT);//ADC值映射到指定显示范围

        //计算图上的y起点  WAVE_SHOW_SY + WAVE_HEIGHT
        y=WAVE_SHOW_SY + WAVE_HEIGHT - y; //映射到图表

        if(WAVE_SHOW_SY<=y && y<=WAVE_SHOW_EY)
        {
            

            #if 1
            //由于中间数据为全部移动，速度慢
            //头部删除
            fb_drawline(WAVE_SHOW_SX,WAVE_SHOW_SY,WAVE_SHOW_SX,WAVE_SHOW_EY,cidxSILVER);
            
            //后部向前移动
            fb_copyrect(WAVE_SHOW_SX+1, WAVE_SHOW_SY, WAVE_SHOW_EX, WAVE_SHOW_EY, WAVE_SHOW_SX, WAVE_SHOW_SY);
            
            //尾部插入
            fb_drawpixel(WAVE_SHOW_EX,y,cidxRED);
            #else

            if(i<=WAVE_WIDTH)//不到宽度不显示
            {
                buf_arr[i++] = y;
            }else
            {
                
                for(n=0;n<WAVE_WIDTH-1;n++)//清除上一段波
                {
                    fb_drawpixel(WAVE_SHOW_SX+n,buf_arr[n],cidxSILVER);
                }
                for(n=0;n<WAVE_WIDTH;n++)//波形左移
                {
                    buf_arr[n]=buf_arr[n+1];
                }
                buf_arr[WAVE_WIDTH-1]=y;
                for(n=0;n<WAVE_WIDTH-1;n++)//新波形绘制
                {
                    fb_drawpixel(WAVE_SHOW_SX+n,buf_arr[n],cidxRED);
                }
            }



            #endif
        }
        taskYIELD();
        //fb_drawpixel(screen_x-1,);

        //delay_ms(500);
    }
}
