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
uint8_t RGB_COLOR_RED[]= {1,0,0}; //红色
uint8_t  RGB_COLOR_GREEN []= {0,1,0}; //绿色
uint8_t  RGB_COLOR_BLUE []= {0,0,1}; //蓝色
uint8_t  RGB_COLOR_YELLOW []= {1,1,0}; //黄色
uint8_t  RGB_COLOR_SORT []= {1,0,1}; //品色
uint8_t  RGB_COLOR_SYAN []= {0,1,1}; //青色
uint8_t  RGB_COLOR_WHITE []= {1,1,1}; //白色
uint8_t  RGB_COLOR_BLACK []= {0,0,0}; //黑色 无色
uint8_t RGB_Colour[3]; //RGB阈值
TaskHandle_t rgb_task_handle;
//RGB初始化函数
void RGB_Init(void)
{
    gpio_enable(LED1,DIR_OUT);
    gpio_enable(LED2,DIR_OUT);
    gpio_enable(LED3,DIR_OUT);
    gpio_write(LED1,0);
    gpio_write(LED2,0);
    gpio_write(LED3,0);

}

//删除初始化
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
            //红色
            gpio_write(LED1,1);
        }
        else
        {
            gpio_write(LED1,0);
        }
        if (rgb_task_count < RGB_Colour[1])
        {
            //绿色
            gpio_write(LED2,1);
        }
        else
        {
            gpio_write(LED2,0);
        }
        if (rgb_task_count < RGB_Colour[2])
        {
            //蓝色
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
        由于freeRTOS每一个tick为1ms 小于1ms低优先级任务不能运行
        删除delay_ms 即可实现0-255的色彩但是会卡在本任务中不会进行让步
        添加delay_ms 会出现刷新过慢导致开关间断明显
        */
        delay_ms(1); 
        //taskYIELD();
    }


}

/**
  * @brief  RGB颜色控制
  * @param  R_Data：红色输出值
					  G_Data：绿色输出值
					  B_Data：蓝色输出值
  * @retval	None
  * @note		0-255
  */
void RGB(uint8_t R_Data, uint8_t G_Data, uint8_t B_Data)
{
    if (R_Data != 0 || G_Data != 0 || B_Data != 0)
    {
        RGB_Colour[0] = R_Data;				  //红
        RGB_Colour[1] = G_Data;				  //绿
        RGB_Colour[2] = B_Data;				  //黄
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
 * @description: 设置RGB灯颜色
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
