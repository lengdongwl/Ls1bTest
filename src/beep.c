#include "beep.h"
#include "ls1b_gpio.h"
//BEEP��ʼ������
void BEEP_Init(void)
{
    gpio_enable(BEEP,DIR_OUT);
    gpio_write(BEEP,0);

}
//����ָ��BEEP����
void BEEP_On(void)
{

    gpio_write(BEEP,1);

}
//�ر�ָ��BEEP����
void BEEP_Off(void)
{
    gpio_write(BEEP,0);

}

