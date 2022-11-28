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
 * 假定: XPT2046_DRV 使用800*480 触摸屏
 *       GT1151_DRV  使用480*800 触摸屏
 *
 * 其它分辨率, 需要修改 grid/button 的位置参数
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
#define MENU_2X	(10 + BUTTON_WIDTH +20) /*子级菜单起始x坐标*/


#define menu_func(name) func_onclick_##name  /*func_onclick_xxx*/
#define menu_task(name) func_task_##name  /*func_task_xxx*/

char func_task_status[20][10];
/*
 * right: 160*480, button to navigate
 */
static void main_onclick_func(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;

    fb_fillrect(MENU_2X,0,MENU_2X+BUTTON_WIDTH+10,480,cidxSILVER); //清除子级菜单
    set_objects_active_group(MAIN_GROUP|btn->guid); //主菜单里的ID从id=0x0001起为1-6
}

static void func_onclick_0001(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;
    if(!(strcmp(btn->caption,"初始化")))
    {
        RGB_Init();
    }
    else if(!(strcmp(btn->caption,"开灯")))
    {
        RGB_Set(RGB_COLOR_WHITE);
    }
    else if(!(strcmp(btn->caption,"关灯")))
    {
        RGB_Set(RGB_COLOR_BLACK);
    }
    else if(!(strcmp(btn->caption,"闪烁")))
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
    if(!(strcmp(btn->caption,"初始化")))
    {
        BEEP_Init();
    }
    else if(!(strcmp(btn->caption,"开启")))
    {
        BEEP_On();
    }
    else if(!(strcmp(btn->caption,"关闭")))
    {

        BEEP_Off();
    }
    else if(!(strcmp(btn->caption,"备用")))
    {


    }
}
TaskHandle_t handle_HMC5883L,handle_ultrasonic,handle_tempratrue,handle_bh1750;
static void func_onclick_0003(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;
    int x,y,buf[50];

    if(!(strcmp(btn->caption,"初始化")))
    {
        /*
        HCM5883L_Init();
        Ultrasonic_Init();
        adc_init();
        */
        /*删除所有任务*/

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
        if(func_task_status[3][2]==pdPASS)return; //防止多次创建任务
        func_task_status[3][2]=xTaskCreate(menu_task(HMC5883L),
                                           "taskHMC5883L",
                                           2048,
                                           NULL,
                                           10,
                                           &handle_HMC5883L);


    }
    else if(!(strcmp(btn->caption,"超声波")))
    {
        if(func_task_status[3][3]==pdPASS)return;
        func_task_status[3][3]=xTaskCreate(menu_task(ultrasonic),
                                           "taskultrasonic",
                                           2048,
                                           NULL,
                                           10,
                                           &handle_ultrasonic);



    }
    else if(!(strcmp(btn->caption,"温度")))
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
    else if(!(strcmp(btn->caption,"全部显示")))
    {
        x = 500;
        y = 42+20;
        sprintf(buf,"HCM5883L:%.2f",HCM5883L_Get_Angle());
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);

        x = 500;
        y = 42*2+20*2;
        sprintf(buf,"超声波:%.2f",Ultrasonic_Get_Dist());
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);

        x = 500;
        y = 42*3+20*3;
        sprintf(buf,"温度:%.2f",adc_get_temp());
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

    if(!(strcmp(btn->caption,"初始化")))
    {
        UART4_Config_Init();
    }
    else if(!(strcmp(btn->caption,"语音播报")))
    {
        UART4_Send(0x01);

    }
    else if(!(strcmp(btn->caption,"语音识别")))
    {
        if(func_task_status[4][3]==pdPASS)return;
        func_task_status[4][3]=xTaskCreate(menu_task(YY),
                                           "YY",
                                           2048,
                                           NULL,
                                           10,
                                           NULL);

    }
    else if(!(strcmp(btn->caption,"备用")))
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
            case "初始化":
                break;
            case "数码管1":
                break;
            case "数码管2":
                break;
            case "数码管调整":
                break;
        }
    */
    if(!(strcmp(btn->caption,"初始化")))
    {
        SMG_Init();
        //SMG_Display(1234,1);
    }
    else if(!(strcmp(btn->caption,"数码管1")))
    {
        menu_task(SMG1)();
    }
    else if(!(strcmp(btn->caption,"数码管2")))
    {
        menu_task(SMG2)();

    }
    else if(!(strcmp(btn->caption,"数码管+++")))
    {
        x = 500;
        y = 42*3+20*3;
        sprintf(buf,"SMGSWAP:%d",++SMGSWAP);
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);
    }
    else if(!(strcmp(btn->caption,"数码管---")))
    {
        x = 500;
        y = 42*3+20*3;
        sprintf(buf,"SMGSWAP:%d",--SMGSWAP);
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        fb_textout(x,y,buf);
    }
    else if(!(strcmp(btn->caption,"取消初始化")))
    {
        SMG_DeInit();
    }
}

TaskHandle_t handle_Fun;
static void func_onclick_0006(unsigned msg, void *param)
{
    int x,y;
    TButton *btn = (TButton *)param;
    if(!(strcmp(btn->caption,"初始化")))
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
    else if(!(strcmp(btn->caption,"风扇50%")))
    {
        Fan_Control(50);

    }
    else if(!(strcmp(btn->caption,"风扇100%")))
    {
        Fan_Control(100);

    }
    else if(!(strcmp(btn->caption,"风扇关闭")))
    {
        Fan_Control(0);

    }
    else if(!(strcmp(btn->caption,"电阻开启")))
    {
        Resistance_Control(1);
    }
    else if(!(strcmp(btn->caption,"电阻关闭")))
    {
        Resistance_Control(0);
    }
    else if(!(strcmp(btn->caption,"恒温控制")))
    {
        if(func_task_status[6][7]==pdPASS)
        {
            return;
        }
        Resistance_Control(1);//电阻打开
        //Fan_Control(100);//风扇打开
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
/*直流电机*/
TaskHandle_t handle_Motor;
int Motor_speed=0;
static void func_onclick_0007(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;
    int x,y;
    char buf[50];

    if(!(strcmp(btn->caption,"初始化")))
    {
        Motor_GPIO_Config();

    }
    else if(!(strcmp(btn->caption,"开启")))
    {
        Motor_speed = 0;
        Motor_set_pwm(Motor_speed);

        if(func_task_status[7][2]==pdPASS)
        {
            return;
        }
        Motor_Interrupt_Init();//这个影响freeRTOS谨慎开关使用
        func_task_status[7][2]=xTaskCreate(menu_task(Motor),
                                           "Motor",
                                           2048,
                                           NULL,
                                           30,
                                           &handle_Motor);

    }
    else if(!(strcmp(btn->caption,"关闭")))
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
    else if(!(strcmp(btn->caption,"减速")))
    {
        x = 400;
        y = 200;
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        sprintf(buf,"电机速度:%d",100-Motor_speed);
        fb_textout(x,y,buf);
        if(Motor_speed<100)
        {
            Motor_speed++;
            Motor_set_pwm(Motor_speed);
        }

    }
    else if(!(strcmp(btn->caption,"加速")))
    {
        x = 400;
        y = 200;
        fb_fillrect(x,y,x+200,y+16,cidxSILVER);
        sprintf(buf,"电机速度:%d",100-Motor_speed);
        fb_textout(x,y,buf);
        if(Motor_speed>0)
        {
            Motor_speed--;
            Motor_set_pwm(Motor_speed);
        }

    }


}

/*屏幕*/
TaskHandle_t handle_Wave,handle_ADC;
static void func_onclick_0008(unsigned msg, void *param)
{
    TButton *btn = (TButton *)param;

    if(!(strcmp(btn->caption,"初始化")))
    {
        fb_open();//初始化并打开framebuffer驱动

        clear_screen();
        if(func_task_status[8][4]==pdPASS)
        {
            func_task_status[8][4] = 0;
            vTaskDelete(handle_ADC);
            vTaskDelete(handle_Wave);
            
        }
    }
    else if(!(strcmp(btn->caption,"文字显示")))
    {

        fb_textout(400,0,"喜迎二十大、永远跟党走、奋进新征程。");

    }
    else if(!(strcmp(btn->caption,"图片显示")))
    {
        menu_task(img)();

    }
    else if(!(strcmp(btn->caption,"颜色显示")))
    {
        menu_task(color)();
    }
    else if(!(strcmp(btn->caption,"波形显示")))
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

static int create_buttons_main(void)
{
    TRect rect;
    TButton *tbn;
    int space=20,id=0x0001;
    /*主菜单*/
    rect.left   = 0;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "LED灯", main_onclick_func);


    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "蜂鸣器", main_onclick_func);

    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "传感器", main_onclick_func);

    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "语音模块", main_onclick_func);

    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "数码管", main_onclick_func);

    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "风扇与电阻", main_onclick_func);

    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "直流电机", main_onclick_func);

    rect.left   = 0;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP , "屏幕显示", main_onclick_func);


    /*LED灯*/
    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0001 , "初始化", menu_func(0001));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0001 , "开灯", menu_func(0001));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0001 , "关灯", menu_func(0001));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0001 , "闪烁", menu_func(0001));

    /*蜂鸣器*/
    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0002 , "初始化", menu_func(0002));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0002 , "开启", menu_func(0002));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0002 , "关闭", menu_func(0002));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0002 , "备用", menu_func(0002));


    /*传感器*/
    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0003 , "初始化", menu_func(0003));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0003 , "全部显示", menu_func(0003));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0003 , "HMC5883L", menu_func(0003));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0003 , "超声波", menu_func(0003));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0003 , "温度", menu_func(0003));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0003 , "bh1750", menu_func(0003));

    /*语音模块*/

    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0004 , "初始化", menu_func(0004));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0004 , "语音播报", menu_func(0004));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0004 , "语音识别", menu_func(0004));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0004 , "备用", menu_func(0004));

    /*数码管*/

    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0005 , "初始化", menu_func(0005));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0005 , "数码管1", menu_func(0005));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0005 , "数码管2", menu_func(0005));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0005 , "数码管+++", menu_func(0005));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0005 , "数码管---", menu_func(0005));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0005 , "取消初始化", menu_func(0005));


    /*风扇与电阻*/

    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "初始化", menu_func(0006));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "风扇50%", menu_func(0006));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "风扇100%", menu_func(0006));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "风扇关闭", menu_func(0006));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "电阻开启", menu_func(0006));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "电阻关闭", menu_func(0006));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0006 , "恒温控制", menu_func(0006));



    /*直流电机*/

    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0007 , "初始化", menu_func(0007));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0007 , "开启", menu_func(0007));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0007 , "关闭", menu_func(0007));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0007 , "加速", menu_func(0007));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0007 , "减速", menu_func(0007));

    /*屏幕显示*/

    rect.left   = MENU_2X;
    rect.top    = 0;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0008 , "初始化", menu_func(0008));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0008 , "文字显示", menu_func(0008));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0008 , "图片显示", menu_func(0008));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0008 , "颜色显示", menu_func(0008));

    rect.left   = MENU_2X;
    rect.top    = rect.top + BUTTON_HEIGHT + space;
    rect.right  = rect.left + BUTTON_WIDTH;
    rect.bottom = rect.top + BUTTON_HEIGHT;
    new_button(&rect, id++, MAIN_GROUP|0x0008 , "波形显示", menu_func(0008));


    return 0;
}

static int create_main_objects(void)
{
    if (clear_screen() != 0) //清除屏幕失败则返回
        return -1;

    if (get_buttons_count(MAIN_GROUP) == 0)
    {
        create_buttons_main(); //创建按钮
    }

    set_objects_active_group(MAIN_GROUP);//设置首页菜单

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

    init_simple_gui_env();          /* GUI环境创建 必须创建声明内存 */

    create_main_objects();	        /* 创建对象组件 */

#if BSP_USE_OS
    start_gui_monitor_task();	    /* 启动GUI任务 绘制按钮以及事件触摸检测等 */
#endif
}



#endif

