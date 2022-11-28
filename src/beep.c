#include "beep.h"
#include "ls1b_gpio.h"
//BEEP初始化函数
void BEEP_Init(void)
{
    gpio_enable(BEEP,DIR_OUT);
    gpio_write(BEEP,0);

}
//开启指定BEEP函数
void BEEP_On(void)
{

    gpio_write(BEEP,1);

}
//关闭指定BEEP函数
void BEEP_Off(void)
{
    gpio_write(BEEP,0);

}

