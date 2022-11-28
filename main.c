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
#error "��bsp.h��ѡ������ XPT2046_DRV ���� GT1151_DRV"
"XPT2046_DRV:  ����800*480 �����Ĵ�����."
"GT1151_DRV:   ����480*800 �����Ĵ�����."
"�������ѡ��, ע�͵��� error ��Ϣ, Ȼ���Զ���: LCD_display_mode[]"
#endif
#endif
#if (BSP_USE_LWMEM)
#include "libc/lwmem.h"
#endif



char buf[50];

/*
Ӳ��һ������

������->UART1
����->FUN
��������->P3
ֱ�����->P2-P5 P5-P2
*/
void test()
{
    int i;
    fb_cons_puts( "beep test\n");
    printk("test task started.\n");
    //����������
    for(i=0; i<5; i++)
    {

        BEEP_On();
        delay_ms(100);
        BEEP_Off();
        delay_ms(100);
    }
    fb_cons_puts("rgb test\n");
    //led��ɫ�Ʋ���
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
    //�����
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



    //����
    fb_cons_puts( "С���������� test\n");
    UART4_Send(0x01);
    delay_ms(500);
    UART4_Send(0x02);
    delay_ms(500);
    UART4_Send(0x03);
    delay_ms(2000);


    //����
    fb_cons_puts( "fun test\n");
    sprintf(buf,"�¶�:%.2f\n",adc_get_temp());//��ȡ�¶�
    fb_cons_puts( buf);
    Resistance_Control(1);//�򿪼��ȵ���
    Fan_Control(100);
    delay_ms(5000);
    Fan_Control(0);
    Resistance_Control(0);
    sprintf(buf,"�¶�:%.2f\n",adc_get_temp());//��ȡ�¶�
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


    //ֱ�����
    fb_cons_puts( "Motor test\n");
    Motor_Interrupt_Init();//���Ӱ��freeRTOS��������ʹ��
    sprintf(buf,"count=%d\n",Motor_get_count());
    fb_cons_puts( buf);
    Motor_set_pwm(0);
    delay_ms(3000);
    Motor_set_pwm(30);
    delay_ms(3000);
    Motor_set_pwm(100);//ֹͣ
    sprintf(buf,"count=%d\n",Motor_get_count());
    fb_cons_puts( buf);
    Motor_Interrupt_DeInit();
    delay_ms(1000);


    //������
    sprintf(buf,"������:%.2f/",Ultrasonic_Get_Dist());
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



int current_group = 0;
TButton *current_btn = 0;
int *button_buf;
int button_buf_i = 0;

//ȡ����ǰ�۽���ť
void unfocused(void)
{
    if(current_btn!=NULL)//uncenter
    {
        focused_button_by_guid(current_btn->guid,false);
    }
}
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
                    if(button_buf_i == 0)//����ǰΪ�׸���ť���������һ��
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
                    if(button_buf_i == get_buttons_count(current_group)-1)//����ǰΪ���һ����ť�������׸�
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



                break;
                
            case 3:
                
#if (USE_MYKEY)
                unfocused();
                if (current_btn != NULL) //��ť����¼�
                {
                    if (current_btn->onclick != NULL)
                    {
                        current_btn->onclick(0, (void *)current_btn);
                    }
                }
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
    fb_open();//��ʼ������framebuffer����
    //TP_Init();//��ʼ��GT1151����������
#endif
    KEY_Init();
    BEEP_Init();
    RGB_Init();
    //IIC_Init();//bh1750 ����32 33ģ��
    adc_init();//ADCͨ�� �¶�
    fan_resistance_io_Config();//���� ���ȵ���
    Ultrasonic_Init();//������
    UART4_Config_Init(); //����ʶ��
    HCM5883L_Init(); //������
    Motor_GPIO_Config();//ֱ�����
    SMG_Init();//�����

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


    /*
    ע������߿���Ӱ��gui������
    */


    start_my_gui();//����GUI�¼�


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
