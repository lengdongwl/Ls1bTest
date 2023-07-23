#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "key.h"
#include "led.h"
#include "beep.h"
#include "bh1750_iic.h"
#include "SMG.h"
#include "led.h"
#include "pwm_ic.h"
#include "ultrasonic_ranging_drv.h"
#include "mylib.h"
#include "uart.h"
#include "motor_drv.h"
#include "fan_resistance_control_drv.h"
#include "hmc5883l_drv.h"
#include "bh1750.h"
#include "ls1x_i2c_bus.h"
#include "adc.h"
#include "MLX90614.h"
#include "rc522.h"
#include "i2c/gt1151.h"
#include ".\gui\demo_gui.h"
#include ".\gui\simple-gui\simple_gui.h"

#define USE_MYKEY 1
//-------------------------------------------------------------------------------------------------
// BSP
//-------------------------------------------------------------------------------------------------

#include "bsp.h"

#ifdef BSP_USE_FB
#include "ls1x_fb.h"
#ifdef XPT2046_DRV
char LCD_display_mode[] = LCD_800x480;
#elif defined(GT1151_DRV)
char LCD_display_mode[] = LCD_800x480;
#else
#error "在bsp.h中选择配置 XPT2046_DRV 或者 GT1151_DRV"
"XPT2046_DRV:  用于800*480 横屏的触摸屏."
"GT1151_DRV:   用于480*800 竖屏的触摸屏."
"如果都不选择, 注释掉本 error 信息, 然后自定义: LCD_display_mode[]"
#endif
#endif
#if (BSP_USE_LWMEM)
#include "libc/lwmem.h"
#endif



char buf[50];

/*
硬件一键测试

超声波->UART1
风扇->FUN
电子罗盘->P3
直流电机->P2-P5 P5-P2
*/
void test()
{
    int i;
    fb_cons_puts( "beep test\n");
    printk("test task started.\n");
    //蜂鸣器测试
    for(i=0; i<5; i++)
    {

        BEEP_On();
        delay_ms(100);
        BEEP_Off();
        delay_ms(100);
    }
    fb_cons_puts("rgb test\n");
    //led三色灯测试
    RGB_Set(RGB_COLOR_RED);
    delay_ms(1000);
    RGB_Set(RGB_COLOR_GREEN);
    delay_ms(1000);
    RGB_Set(RGB_COLOR_BLUE);
    delay_ms(1000);
    RGB_Set(RGB_COLOR_YELLOW);
    delay_ms(1000);
    RGB_Set(RGB_COLOR_BLACK);
    fb_cons_puts( "smg test\n");
    //数码管
    SMG_DisplayP(1,-1,-1,-1,1);
    delay_ms(1000);
    SMG_DisplayP(1,1,-1,-1,1);
    delay_ms(1000);
    SMG_DisplayP(1,1,0,-1,1);
    delay_ms(1000);
    SMG_DisplayP(1,1,0,4,1);
    delay_ms(1000);

    SMG_Display(4321,1);
    delay_ms(1000);
    SMG_Display(0,0);
    delay_ms(500);



    //语音
    fb_cons_puts( "小创语音播报 test\n");
    UART4_Send(0x01);
    delay_ms(500);
    UART4_Send(0x02);
    delay_ms(500);
    UART4_Send(0x03);
    delay_ms(2000);


    //风扇
    fb_cons_puts( "fun test\n");
    sprintf(buf,"温度:%.2f\n",adc_get_temp());//读取温度
    fb_cons_puts( buf);
    Resistance_Control(1);//打开加热电阻
    Fan_Control(100);
    delay_ms(5000);
    Fan_Control(0);
    Resistance_Control(0);
    sprintf(buf,"温度:%.2f\n",adc_get_temp());//读取温度
    fb_cons_puts( buf);


    //bh1750
    fb_cons_puts( "bh1750 test data=");
    sprintf(buf,"%d/",BH1750_Test());
    fb_cons_puts( buf);
    delay_ms(1000);
    sprintf(buf,"%d/",BH1750_Test());
    fb_cons_puts( buf);
    delay_ms(1000);
    sprintf(buf,"%d\n",BH1750_Test());
    fb_cons_puts( buf);
    delay_ms(2000);


    //HCM5883L
    fb_cons_puts( "HCM5883L test data=");
    sprintf(buf,"%.2f\n",HCM5883L_Get_Angle());
    fb_cons_puts( buf);


    //直流电机
    fb_cons_puts( "Motor test\n");
    Motor_Interrupt_Init();//这个影响freeRTOS谨慎开关使用
    sprintf(buf,"count=%d\n",Motor_get_count());
    fb_cons_puts( buf);
    Motor_set_pwm(0);
    delay_ms(3000);
    Motor_set_pwm(30);
    delay_ms(3000);
    Motor_set_pwm(100);//停止
    sprintf(buf,"count=%d\n",Motor_get_count());
    fb_cons_puts( buf);
    Motor_Interrupt_DeInit();
    delay_ms(1000);


    //超声波
    sprintf(buf,"超声波:%.2f/",Ultrasonic_Get_Dist());
    fb_cons_puts( buf);
    delay_ms(1000);
    sprintf(buf,"%.2f/",Ultrasonic_Get_Dist());
    fb_cons_puts( buf);
    delay_ms(1000);
    sprintf(buf,"%.2f/",Ultrasonic_Get_Dist());
    fb_cons_puts( buf);
    delay_ms(1000);
    sprintf(buf,"%.2f\n",Ultrasonic_Get_Dist());
    fb_cons_puts( buf);




    fb_cons_puts( "ok test\n");

}
void func_task_YyY(void* arg)
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
                fb_fillrect(x ,y+16*2,x+200,y+16*3,cidxBLACK);
                fb_textout(x,y+16*2,"请关灯");
                break;
            case 0x14:
                RGB_Set(RGB_COLOR_RED);
                fb_fillrect(x ,y+16*2,x+200,y+16*3,cidxBLACK);
                fb_textout(x,y+16*2,"请开红灯");
                break;
            case 0x15:
                RGB_Set(RGB_COLOR_GREEN);
                fb_fillrect(x ,y+16*2,x+200,y+16*3,cidxBLACK);
                fb_textout(x,y+16*2,"请开绿灯");
                break;
            case 0x16:
                RGB_Set(RGB_COLOR_BLUE);
                fb_fillrect(x ,y+16*2,x+200,y+16*3,cidxBLACK);
                fb_textout(x,y+16*2,"请开蓝灯");
                break;
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

void task_bh(void* arg)
{
    volatile int x,y;
    volatile char buf[50];
    printk("func_task_bh1750 thread started!\n");

    x = 500;
    y = 42*4+20*4;
    while(1)
    {
        if(!strcmp(pcTaskGetName(arg),"bh1750"))
        {
            sprintf(buf,"当前光照度:%d LX",BH1750_Test());
        }else
        {
            sprintf(buf,"当前光照度:%.4d LX",BH1750_Test());
        }
        fb_fillrect(x + strlen("当前光照度:")*8,y,x+200,y+16,cidxBLACK);
        fb_textout(x,y,buf);
        delay_ms(400);
    }
}


void tempratrue1(void* arg)
{
    volatile int x,y;
    volatile char buf[50];
    printk("func_task_tempratrue thread started!\n");
    x = 500;
    y = 42*3+20*3;
    while(1)
    {
        sprintf(buf,"当前温度:%.1f℃",adc_get_temp());
        fb_fillrect(x + strlen("当前温度:")*8,y,x+200,y+16,cidxBLACK);
        fb_textout(x,y,buf);
        delay_ms(400);
    }
}

void TESTSMG(unsigned char a)
{
    switch(a)
    {
        case 1:
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
            break;
        case 2:
            BEEP_On();
            break;
        case 3:
            BEEP_Off();
            break;
        case 4:
            xTaskCreate(func_task_YyY,
                                       "YY",
                                       2048,
                                       NULL,
                                       10,
                                       NULL);
            break;
        case 5:
            SMG_DisplayP(2,0,2,3,1);
            
            break;
        case 6:
            fb_textout(400,0,"青春飞扬遍天地,意气风发转乾坤。");
            break;
        case 7:
            xTaskCreate(tempratrue1,
                                       "tasktempratrue",
                                       2048,
                                       NULL,
                                       10,
                                       0);
            break;
        case 8:

             xTaskCreate(task_bh,
                                       "bh1750",
                                       2048,
                                       NULL,
                                       10,
                                       0);

           break;
        case 9:
            xTaskCreate(task_bh,
                                       "bh1750_4",
                                       2048,
                                       NULL,
                                       10,
                                      0);
            break;

    }
}

int current_group = 0;
TButton *current_btn = 0;
int *button_buf;
int button_buf_i = 0;

//取消当前聚焦按钮
void unfocused(void)
{
    if(current_btn!=NULL)//uncenter
    {
        focused_button_by_guid(current_btn->guid,false);
    }
}


//#define SMGMODE 1
unsigned char SMGFLAG=0;

static void key_task(void *arg)
{
    printk("key_task thread started!\n");

    for ( ; ; )
    {
        switch(KEY_Scan())
        {
            case 1:
#if (USE_MYKEY)
                if(current_group!=get_objects_active_group())//active_group change
                {
                    current_group=get_objects_active_group();
                    button_buf = find_button_by_group(current_group);
                    
                    

                    button_buf_i = 0;
                    current_btn = find_button(button_buf[button_buf_i]);//get first button

                    if(current_btn!=NULL)//focuse
                    {
                        focused_button_by_guid(current_btn->guid,true);
                    }
                }
                else
                {
                    if(button_buf_i == 0)//若当前为首个按钮则跳到最后一个
                    {
                        button_buf_i = get_buttons_count(current_group)-1;
                    }else
                    {
                        button_buf_i--;
                    }
                    unfocused();
                    current_btn = find_button(button_buf[button_buf_i]);//get button

                    if(current_btn!=NULL)//focuse
                    {
                        focused_button_by_guid(current_btn->guid,true);
                    }

                }
#endif


#if (SMGMODE)
    SMGFLAG--;
    SMG_DisplayP(0,0,0,SMGFLAG,1);
#endif


                break;
            case 2:
#if (USE_MYKEY)
                if(current_group!=get_objects_active_group())//active_group change
                {
                    current_group=get_objects_active_group();
                    button_buf = find_button_by_group(current_group);



                    button_buf_i = 0;
                    current_btn = find_button(button_buf[button_buf_i]);//get first button

                    if(current_btn!=NULL)//focuse
                    {
                        focused_button_by_guid(current_btn->guid,true);
                    }
                }
                else
                {
                    if(button_buf_i == get_buttons_count(current_group)-1)//若当前为最后一个按钮则跳到首个
                    {
                        button_buf_i = 0;
                    }else
                    {
                        button_buf_i++;
                    }
                    unfocused();
                    current_btn = find_button(button_buf[button_buf_i]);//get button

                    if(current_btn!=NULL)//focuse
                    {
                        focused_button_by_guid(current_btn->guid,true);
                    }



                }
#endif

#if (SMGMODE)
    SMGFLAG++;
    SMG_DisplayP(0,0,0,SMGFLAG,1);
#endif

                break;
                
            case 3:
                
#if (USE_MYKEY)
                unfocused();
                if (current_btn != NULL) //按钮点击事件
                {
                    if (current_btn->onclick != NULL)
                    {
                        current_btn->onclick(0, (void *)current_btn);
                    }
                }
#endif

#if (SMGMODE)
   TESTSMG(SMGFLAG);
#endif
                
                break;
            case 4:
                

                clear_screen();
                set_objects_active_group_mainUI();


                //test();
                /*
                while(1)
                {
                    sprintf(buf,"adc1=%d\n",adc_get(ADC1));
                printk(buf);
                }*/
                break;


        }



    }
}

void init_task()
{
#ifdef BSP_USE_FB
    fb_open();//初始化并打开framebuffer驱动
    //TP_Init();//初始化GT1151触摸屏驱动
#endif
    KEY_Init();
    BEEP_Init();
    RGB_Init();
    //IIC_Init();//bh1750 引脚32 33模拟
    adc_init();//ADC通道 温度
    fan_resistance_io_Config();//风扇 加热电阻
    Ultrasonic_Init();//超声波
    UART4_Config_Init(); //语音识别
    HCM5883L_Init(); //磁力计
    Motor_GPIO_Config();//直流电机
    
    /*P4*/
    //MLX90614_Init();//红外测温
    //RC522_Init();
    
    
    SMG_Init();//数码管

}


//-------------------------------------------------------------------------------------------------

int main(void)
{
    //printk("\r\nmain() function.\r\n");

    ls1x_drv_init();            		/* Initialize device drivers */

    install_3th_libraries();      		/* Install 3th libraies */


    init_task();


    fb_cons_puts("init ok...\n");
    sprintf(buf,"x=%d,y=%d\n",fb_get_pixelsx(),fb_get_pixelsy());
    fb_cons_puts(buf);


//    while(1){printk("Temp=%d\n",RC_check());delay_ms(500);}

    /*
    注：电机线可能影响gui的启动
    */


    start_my_gui();//启动GUI事件


    xTaskCreate(key_task,
                "keytask",
                2048,
                NULL,
                10,
                NULL);


    vTaskStartScheduler();

    /* If all is well we will never reach here as the scheduler will now be
     * running.  If we do reach here then it is likely that there was insufficient
     * heap available for the idle task to be created.
     */
    for ( ; ; )
    {
        ;
    }

    return 0;
}

/*
 * @@ End
 */
