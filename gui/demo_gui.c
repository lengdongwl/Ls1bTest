/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * demo_gui.c
 *
 *  Created on: 2015-1-26
 *      Author: Bian
 */

/*
 * �ٶ�: XPT2046_DRV ʹ��800*480 ������
 *       GT1151_DRV  ʹ��480*800 ������
 *
 * �����ֱ���, ��Ҫ�޸� grid/button ��λ�ò���
 */

#include "bsp.h"

#ifdef BSP_USE_FB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include ".\simple-gui\simple_gui.h"
#include "ls1x_fb.h"


#include "FreeRTOS.h"
#include "task.h"


#define BUTTON_WIDTH		120
#define BUTTON_HEIGHT		42

/* ��������*/
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


/******************************************************************************
 * defined functions
 ******************************************************************************/

int clear_screen(void)
{
    /* fill screen color silver
     */
    ls1x_dc_ioctl(devDC, IOCTRL_FB_CLEAR_BUFFER, (void *)GetColor(cidxSILVER));

    return 0;
}

static int create_main_objects(void);

static void set_objects_active_group(int group);





/******************************************************************************
 * main frame
 ******************************************************************************/

#define MAIN_GROUP	0x00010000
#define MENU_2X	(10 + BUTTON_WIDTH +20) /*�Ӽ��˵���ʼx����*/


#define menu_func(name) func_onclick_##name  /*func_onclick_xxx*/
#define menu_task(name) func_task_##name  /*func_task_xxx*/

char func_task_status[20][10];
/*
 * right: 160*480, button to navigate
 */
static void main_onclick_func(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;

    fb_fillrect(MENU_2X,0,MENU_2X+BUTTON_WIDTH+10,480,cidxSILVER); //����Ӽ��˵�
    set_objects_active_group(MAIN_GROUP|btn->guid); //���˵����ID��id=0x0001��Ϊ1-6
}

static void func_onclick_0001(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;
    if(!(strcmp(btn->caption,"��ʼ��")))
    {
        RGB_Init();
    }
    else if(!(strcmp(btn->caption,"����")))
    {
        RGB_Set(RGB_COLOR_WHITE);
    }
    else if(!(strcmp(btn->caption,"�ص�")))
    {
        RGB_Set(RGB_COLOR_BLACK);
    }
    else if(!(strcmp(btn->caption,"��˸")))
    {
        RGB_Set(RGB_COLOR_RED);
        delay_ms(1000);
        RGB_Set(RGB_COLOR_BLACK);
        delay_ms(1000);
        RGB_Set(RGB_COLOR_GREEN);
        delay_ms(1000);
        RGB_Set(RGB_COLOR_BLACK);
        delay_ms(1000);
        RGB_Set(RGB_COLOR_BLUE);
        delay_ms(1000);
        RGB_Set(RGB_COLOR_BLACK);
    }
}

static void func_onclick_0002(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;
    if(!(strcmp(btn->caption,"��ʼ��")))
    {
        BEEP_Init();
    }
    else if(!(strcmp(btn->caption,"����")))
    {
        BEEP_On();
    }
    else if(!(strcmp(btn->caption,"�ر�")))
    {

        BEEP_Off();
    }
    else if(!(strcmp(btn->caption,"����")))
    {


    }
}
TaskHandle_t handle_HMC5883L,handle_ultrasonic,handle_tempratrue,handle_bh1750;
static void func_onclick_0003(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;
    int x,y,buf[50];

    if(!(strcmp(btn->caption,"��ʼ��")))
    {
        /*
        HCM5883L_Init();
        Ultrasonic_Init();
        adc_init();
        */
        /*ɾ����������*/

        if(func_task_status[3][2]==pdPASS)
        {
            func_task_status[3][2] = 0;
            vTaskDelete(handle_HMC5883L);
            x = 500;
            y = 42+20;
            fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        }
        if(func_task_status[3][3]==pdPASS)
        {
            func_task_status[3][3] = 0;
            vTaskDelete(handle_ultrasonic);
            x = 500;
            y = 42*2+20*2;
            fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        }
        if(func_task_status[3][4]==pdPASS)
        {
            func_task_status[3][4] = 0;
            vTaskDelete(handle_tempratrue);
            x = 500;
            y = 42*3+20*3;
            fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        }
        if(func_task_status[4][4]==pdPASS)
        {
            func_task_status[4][4] = 0;
            vTaskDelete(handle_bh1750);
            x = 500;
            y = 42*4+20*4;
            fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        }


    }
    else if(!(strcmp(btn->caption,"HMC5883L")))
    {
        if(func_task_status[3][2]==pdPASS)return; //��ֹ��δ�������
        func_task_status[3][2]=xTaskCreate(menu_task(HMC5883L),
                                           "taskHMC5883L",
                                           2048,
                                           NULL,
                                           10,
                                           &handle_HMC5883L);


    }
    else if(!(strcmp(btn->caption,"������")))
    {
        if(func_task_status[3][3]==pdPASS)return;
        func_task_status[3][3]=xTaskCreate(menu_task(ultrasonic),
                                           "taskultrasonic",
                                           2048,
                                           NULL,
                                           10,
                                           &handle_ultrasonic);



    }
    else if(!(strcmp(btn->caption,"�¶�")))
    {
        if(func_task_status[3][4]==pdPASS)return;
        func_task_status[3][4]=xTaskCreate(menu_task(tempratrue),
                                           "tasktempratrue",
                                           2048,
                                           NULL,
                                           10,
                                           &handle_tempratrue);
    }
    else if(!(strcmp(btn->caption,"bh1750")))
    {
        if(func_task_status[4][4]==pdPASS)return;
        func_task_status[4][4]=xTaskCreate(menu_task(bh1750),
                                           "tasktempratrue",
                                           2048,
                                           NULL,
                                           10,
                                           &handle_bh1750);
    }
    else if(!(strcmp(btn->caption,"ȫ����ʾ")))
    {
        x = 500;
        y = 42+20;
        sprintf(buf,"HCM5883L:%.2f",HCM5883L_Get_Angle());
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);

        x = 500;
        y = 42*2+20*2;
        sprintf(buf,"������:%.2f",Ultrasonic_Get_Dist());
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);

        x = 500;
        y = 42*3+20*3;
        sprintf(buf,"�¶�:%.2f",adc_get_temp());
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);

        x = 500;
        y = 42*4+20*4;
        sprintf(buf,"bh1750:%d",BH1750_Test());
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);
    }
}

static void func_onclick_0004(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;

    if(!(strcmp(btn->caption,"��ʼ��")))
    {
        UART4_Config_Init();
    }
    else if(!(strcmp(btn->caption,"��������")))
    {
        UART4_Send(0x01);

    }
    else if(!(strcmp(btn->caption,"����ʶ��")))
    {
        if(func_task_status[4][3]==pdPASS)return;
        func_task_status[4][3]=xTaskCreate(menu_task(YY),
                                           "YY",
                                           2048,
                                           NULL,
                                           10,
                                           NULL);

    }
    else if(!(strcmp(btn->caption,"����")))
    {

    }


}


static void func_onclick_0005(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;
    int x,y,buf[50];
    /*
        switch((btn->caption))
        {
            case "��ʼ��":
                break;
            case "�����1":
                break;
            case "�����2":
                break;
            case "����ܵ���":
                break;
        }
    */
    if(!(strcmp(btn->caption,"��ʼ��")))
    {
        SMG_Init();
        //SMG_Display(1234,1);
    }
    else if(!(strcmp(btn->caption,"�����1")))
    {
        menu_task(SMG1)();
    }
    else if(!(strcmp(btn->caption,"�����2")))
    {
        menu_task(SMG2)();

    }
    else if(!(strcmp(btn->caption,"�����+++")))
    {
        x = 500;
        y = 42*3+20*3;
        sprintf(buf,"SMGSWAP:%d",++SMGSWAP);
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);
    }
    else if(!(strcmp(btn->caption,"�����---")))
    {
        x = 500;
        y = 42*3+20*3;
        sprintf(buf,"SMGSWAP:%d",--SMGSWAP);
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);
    }
    else if(!(strcmp(btn->caption,"ȡ����ʼ��")))
    {
        SMG_DeInit();
    }
}

TaskHandle_t handle_Fun;
static void func_onclick_0006(unsigned msg, void *param)
{
    int x,y;
    TButton *btn = (TButton *)param;
    if(!(strcmp(btn->caption,"��ʼ��")))
    {
        fan_resistance_io_Config();
        if(func_task_status[6][7]==pdPASS)
        {
            func_task_status[6][7] = 0;
            vTaskDelete(handle_Fun);
        }
        if(func_task_status[3][4]==pdPASS)
        {
            func_task_status[3][4] = 0;
            vTaskDelete(handle_tempratrue);
            x = 500;
            y = 42*3+20*3;
            fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        }
    }
    else if(!(strcmp(btn->caption,"����50%")))
    {
        Fan_Control(50);

    }
    else if(!(strcmp(btn->caption,"����100%")))
    {
        Fan_Control(100);

    }
    else if(!(strcmp(btn->caption,"���ȹر�")))
    {
        Fan_Control(0);

    }
    else if(!(strcmp(btn->caption,"���迪��")))
    {
        Resistance_Control(1);
    }
    else if(!(strcmp(btn->caption,"����ر�")))
    {
        Resistance_Control(0);
    }
    else if(!(strcmp(btn->caption,"���¿���")))
    {
        if(func_task_status[6][7]==pdPASS)
        {
            return;
        }
        Resistance_Control(1);//�����
        //Fan_Control(100);//���ȴ�
        func_task_status[6][7]=xTaskCreate(menu_task(Fun),
                                           "Fun",
                                           2048,
                                           NULL,
                                           30,
                                           &handle_Fun);
        if(func_task_status[3][4]==pdPASS)return;
        func_task_status[3][4]=xTaskCreate(menu_task(tempratrue),
                                           "tasktempratrue",
                                           2048,
                                           NULL,
                                           10,
                                           &handle_tempratrue);
    }




}
/*ֱ�����*/
TaskHandle_t handle_Motor;
int Motor_speed=0;
static void func_onclick_0007(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;
    int x,y;
    char buf[50];

    if(!(strcmp(btn->caption,"��ʼ��")))
    {
        Motor_GPIO_Config();

    }
    else if(!(strcmp(btn->caption,"����")))
    {
        Motor_speed = 0;
        Motor_set_pwm(Motor_speed);

        if(func_task_status[7][2]==pdPASS)
        {
            return;
        }
        Motor_Interrupt_Init();//���Ӱ��freeRTOS��������ʹ��
        func_task_status[7][2]=xTaskCreate(menu_task(Motor),
                                           "Motor",
                                           2048,
                                           NULL,
                                           30,
                                           &handle_Motor);

    }
    else if(!(strcmp(btn->caption,"�ر�")))
    {
        Motor_set_pwm(100);
        if(func_task_status[7][2]==pdPASS)
        {
            Motor_Interrupt_DeInit();
            func_task_status[7][2] = 0;
            vTaskDelete(handle_Motor);
            x = 400;
            y = 42*2+20*2;
            fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        }

    }
    else if(!(strcmp(btn->caption,"����")))
    {
        x = 400;
        y = 200;
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        sprintf(buf,"����ٶ�:%d",100-Motor_speed);
        fb_textout(x,y,buf);
        if(Motor_speed<100)
        {
            Motor_speed++;
            Motor_set_pwm(Motor_speed);
        }

    }
    else if(!(strcmp(btn->caption,"����")))
    {
        x = 400;
        y = 200;
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        sprintf(buf,"����ٶ�:%d",100-Motor_speed);
        fb_textout(x,y,buf);
        if(Motor_speed>0)
        {
            Motor_speed--;
            Motor_set_pwm(Motor_speed);
        }

    }


}

/*��Ļ*/
TaskHandle_t handle_Wave,handle_ADC;
static void func_onclick_0008(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;

    if(!(strcmp(btn->caption,"��ʼ��")))
    {
        fb_open();//��ʼ������framebuffer����

        clear_screen();
        if(func_task_status[8][4]==pdPASS)
        {
            func_task_status[8][4] = 0;
            vTaskDelete(handle_ADC);
            vTaskDelete(handle_Wave);
            
        }
    }
    else if(!(strcmp(btn->caption,"������ʾ")))
    {

        fb_textout(400,0,"ϲӭ��ʮ����Զ�����ߡ��ܽ������̡�");

    }
    else if(!(strcmp(btn->caption,"ͼƬ��ʾ")))
    {
        menu_task(img)();

    }
    else if(!(strcmp(btn->caption,"��ɫ��ʾ")))
    {
        menu_task(color)();
    }
    else if(!(strcmp(btn->caption,"������ʾ")))
    {
        if(func_task_status[8][4]==pdPASS)
        {
            return;
        }
        if(xTaskCreate(menu_task(ADC),
                       "ADC",
                       2048,
                       NULL,
                       30,
                       &handle_ADC))
        {
            func_task_status[8][4]=xTaskCreate(menu_task(Wave),
                                               "Wave",
                                               2048,
                                               NULL,
                                               30,
                                               &handle_Wave);
        }


    }
}

/*����*/
TaskHandle_t handle_Other1,handle_Other2,handle_Other3;
static void func_onclick_0009(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;

    if(!(strcmp(btn->caption,"��ʼ��")))
    {
        if(func_task_status[9][4]==pdPASS)
        {
            func_task_status[9][4] = 0;
            Motor_set_pwm(100);
            vTaskDelete(handle_Other1);
            vTaskDelete(handle_Motor);
        }
    }else if(!(strcmp(btn->caption,"��������1")))/*ͨ����������������Ƶ���ٶ�*/
    {
        
        
        Motor_Interrupt_Init();//���Ӱ��freeRTOS��������ʹ�� ����жϴ�����ȡ
        if(func_task_status[9][4]==pdPASS)
        {
            return;
        }
        if(xTaskCreate(menu_task(Other1),//�����������
                       "Other",
                       2048,
                       NULL,
                       30,
                       &handle_Other1)==pdPASS)
        {
            func_task_status[9][4]=xTaskCreate(menu_task(Motor),
                                           "Motor",
                                           2048,
                                           NULL,
                                           30,
                                           &handle_Motor);//��ʾ���ת������
                                           
        }
      
        
    }else if(!(strcmp(btn->caption,"��������2")))
    {
        
    }else if(!(strcmp(btn->caption,"��������3")))
    {
        
    }
}
    


static int create_buttons_main(void)
{
    TRect rect;
    TButton *tbn;
    int space=20,id=0x0001;
    /*���˵�*/
    rect.left   = 0;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "LED��", main_onclick_func);


    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "������", main_onclick_func);

    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "������", main_onclick_func);

    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "����ģ��", main_onclick_func);

    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "�����", main_onclick_func);

    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "���������", main_onclick_func);

    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "ֱ�����", main_onclick_func);

    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "��Ļ��ʾ", main_onclick_func);
    
    rect.left   = MENU_2X;
    rect.top    = rect.top ;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "����", main_onclick_func);


    /*LED��*/
    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0001 , "��ʼ��", menu_func(0001));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0001 , "����", menu_func(0001));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0001 , "�ص�", menu_func(0001));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0001 , "��˸", menu_func(0001));

    /*������*/
    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0002 , "��ʼ��", menu_func(0002));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0002 , "����", menu_func(0002));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0002 , "�ر�", menu_func(0002));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0002 , "����", menu_func(0002));


    /*������*/
    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0003 , "��ʼ��", menu_func(0003));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0003 , "ȫ����ʾ", menu_func(0003));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0003 , "HMC5883L", menu_func(0003));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0003 , "������", menu_func(0003));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0003 , "�¶�", menu_func(0003));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0003 , "bh1750", menu_func(0003));

    /*����ģ��*/

    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0004 , "��ʼ��", menu_func(0004));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0004 , "��������", menu_func(0004));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0004 , "����ʶ��", menu_func(0004));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0004 , "����", menu_func(0004));

    /*�����*/

    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0005 , "��ʼ��", menu_func(0005));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0005 , "�����1", menu_func(0005));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0005 , "�����2", menu_func(0005));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0005 , "�����+++", menu_func(0005));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0005 , "�����---", menu_func(0005));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0005 , "ȡ����ʼ��", menu_func(0005));


    /*���������*/

    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "��ʼ��", menu_func(0006));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "����50%", menu_func(0006));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "����100%", menu_func(0006));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "���ȹر�", menu_func(0006));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "���迪��", menu_func(0006));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "����ر�", menu_func(0006));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "���¿���", menu_func(0006));



    /*ֱ�����*/

    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0007 , "��ʼ��", menu_func(0007));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0007 , "����", menu_func(0007));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0007 , "�ر�", menu_func(0007));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0007 , "����", menu_func(0007));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0007 , "����", menu_func(0007));

    /*��Ļ��ʾ*/

    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0008 , "��ʼ��", menu_func(0008));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0008 , "������ʾ", menu_func(0008));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0008 , "ͼƬ��ʾ", menu_func(0008));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0008 , "��ɫ��ʾ", menu_func(0008));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0008 , "������ʾ", menu_func(0008));
    
    
     /*����*/
    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0009 , "��ʼ��", menu_func(0009));
    
    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0009 , "��������1", menu_func(0009));
    
    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0009 , "��������2", menu_func(0009));
    
    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0009 , "��������3", menu_func(0009));




    return 0;
}

static int create_main_objects(void)
{
    if (clear_screen() != 0) //�����Ļʧ���򷵻�
        return -1;

    if (get_buttons_count(MAIN_GROUP) == 0)
    {
        create_buttons_main(); //������ť
    }

    set_objects_active_group(MAIN_GROUP);//������ҳ�˵�

    return 0;
}

/******************************************************************************
 * object set active
 ******************************************************************************/

static void set_objects_active_group(int group)
{
    set_gui_active_group(group);
}

int get_objects_active_group(void)
{
    return get_gui_active_group();
}

void set_objects_active_group_mainUI(void)
{
    set_gui_active_group(MAIN_GROUP);
}

/*
int delete_button_by_group(int group)
{
	TButton *btn;
	TAILQ_FOREACH(btn, &gui_buttons, next)
	{
		if (btn->group == group)
			delete_button_by_guid(btn->guid);
	}
	return 0;
}*/
/******************************************************************************
 * app with simple-gui & xyplot
 ******************************************************************************/

void start_my_gui(void)
{
    if (fb_open() != 0)
        return;

    if (!ls1x_dc_started())
        return;

    init_simple_gui_env();          /* GUI�������� ���봴�������ڴ� */

    create_main_objects();	        /* ����������� */

#if BSP_USE_OS
    start_gui_monitor_task();	    /* ����GUI���� ���ư�ť�Լ��¼��������� */
#endif
}



#endif

