/*
 * led.c
 *
 * created: 2022/7/6
 * authour:
 */
#include "led.h"
#include "ls1b_gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ls1b.h"
#include "bsp.h"
uint8_t rgb_task_status = 0;
uint8_t RGB_COLOR_RED[]= {1,0,0}; //��ɫ
uint8_t  RGB_COLOR_GREEN []= {0,1,0}; //��ɫ
uint8_t  RGB_COLOR_BLUE []= {0,0,1}; //��ɫ
uint8_t  RGB_COLOR_YELLOW []= {1,1,0}; //��ɫ
uint8_t  RGB_COLOR_SORT []= {1,0,1}; //Ʒɫ
uint8_t  RGB_COLOR_SYAN []= {0,1,1}; //��ɫ
uint8_t  RGB_COLOR_WHITE []= {1,1,1}; //��ɫ
uint8_t  RGB_COLOR_BLACK []= {0,0,0}; //��ɫ ��ɫ
uint8_t RGB_Colour[3]; //RGB��ֵ
TaskHandle_t rgb_task_handle;
//RGB��ʼ������
void RGB_Init(void)
{
    gpio_enable(LED1,DIR_OUT);
    gpio_enable(LED2,DIR_OUT);
    gpio_enable(LED3,DIR_OUT);
    gpio_write(LED1,0);
    gpio_write(LED2,0);
    gpio_write(LED3,0);

}

//ɾ����ʼ��
void RGB_DeInit(void)
{
    gpio_disable(LED1);
    gpio_disable(LED2);
    gpio_disable(LED3);

    if(rgb_task_status)
    {
        vTaskDelete(rgb_task_handle);
        rgb_task_status = 0;
    }
}



uint8_t rgb_task_count = 0;
void rgb_task(void* arg)
{
    while(1)
    {
        if (rgb_task_count < RGB_Colour[0])
        {
            //��ɫ
            gpio_write(LED1,1);
        }
        else
        {
            gpio_write(LED1,0);
        }
        if (rgb_task_count < RGB_Colour[1])
        {
            //��ɫ
            gpio_write(LED2,1);
        }
        else
        {
            gpio_write(LED2,0);
        }
        if (rgb_task_count < RGB_Colour[2])
        {
            //��ɫ
            gpio_write(LED3,1);
        }
        else
        {
            gpio_write(LED3,0);
        }
        rgb_task_count += 1;
        if (rgb_task_count >= 255)
        {
            rgb_task_count = 0;
        }
        /*
        ����freeRTOSÿһ��tickΪ1ms С��1ms�����ȼ�����������
        ɾ��delay_ms ����ʵ��0-255��ɫ�ʵ��ǻῨ�ڱ������в�������ò�
        ���delay_ms �����ˢ�¹������¿��ؼ������
        */
        delay_ms(1); 
        //taskYIELD();
    }


}

/**
  * @brief  RGB��ɫ����
  * @param  R_Data����ɫ���ֵ
					  G_Data����ɫ���ֵ
					  B_Data����ɫ���ֵ
  * @retval	None
  * @note		0-255
  */
void RGB(uint8_t R_Data, uint8_t G_Data, uint8_t B_Data)
{
    if (R_Data != 0 || G_Data != 0 || B_Data != 0)
    {
        RGB_Colour[0] = R_Data;				  //��
        RGB_Colour[1] = G_Data;				  //��
        RGB_Colour[2] = B_Data;				  //��
        if(rgb_task_status == 0)
        {

            if(xTaskCreate(rgb_task,"rgbtask",2048,NULL,11,&rgb_task_handle) == pdPASS)
            {
                rgb_task_status = 1;
            }
            else
            {
                rgb_task_status = 0;
            }
        }


    }
    else
    {

        if(rgb_task_status)
        {
            vTaskDelete(rgb_task_handle);
            rgb_task_status = 0;
        }
        gpio_write(LED1,0);
        gpio_write(LED2,0);
        gpio_write(LED3,0);
    }
}


/**
 * @description: ����RGB����ɫ
 * @param {uint8_t} *RGB_COLOR_x
 * @return {*}
 */
void RGB_Set(uint8_t *RGB_COLOR_x)
{
    if(rgb_task_status)
    {
        vTaskDelete(rgb_task_handle);
        rgb_task_status = 0;
    }
    gpio_write(LED1,RGB_COLOR_x[0]);
    gpio_write(LED2,RGB_COLOR_x[1]);
    gpio_write(LED3,RGB_COLOR_x[2]);
}
