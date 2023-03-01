/*
 * ultrasonic_ranging_drv.c
 *
 * created: 2022/7/15
 *  author:
 */

#include "ultrasonic_ranging_drv.h"
#include "ls1b_gpio.h"
#include "ls1x_pwm.h"
#include "bsp.h"
#include <stdbool.h>
#include "stdint.h"
#define TRIG 51
#define ECHO 50


extern void *devPWM2;

static bool flag = 0;//�ɹ����ձ�־
//static unsigned int cnt = 0;
static float distance;
static float last_distance;

//���ڼ�ʱ
static volatile unsigned int Timer = 0;

//����һ��PWM�Ľṹ��
pwm_cfg_t pwm2_cfg;
//PWM2�жϴ�����
static void pwmtimer_callback(void *pwm, int *stopit)
{
    Timer++;
}

/*************************************************************************************
**�������ܣ�ʹ��PWM2ʵ��Լ10us��ʱ
**˵�����˶�ʱ����Ҫ�ó���������ʱ
**************************************************************************************/
void PWM2_Time_Init(void)
{
    pwm2_cfg.hi_ns = 5000;  //�ߵ�ƽ������(����), ��ʱ��ģʽ���� hi_ns��Լ����1us��
    pwm2_cfg.lo_ns = 0;         //�͵�ƽ������(����),��ʱ��ģʽû�� lo_ns
    pwm2_cfg.mode = PWM_CONTINUE_TIMER; //�����������
    pwm2_cfg.cb = pwmtimer_callback;    //��ʱ���жϻص�����
    pwm2_cfg.isr = NULL;                //�����ڶ�ʱ��ģʽ

    ls1x_pwm_init(devPWM2,NULL);
    ls1x_pwm_open(devPWM2,(void *)&pwm2_cfg);
    ls1x_pwm_timer_stop(devPWM2);
}


/*********************************************************************
 **�������ܣ�ECHO�����½��ز����ж�
 **�βΣ�
    @IRQn  �жϺ�
    @param ���ݸ��жϴ������Ĳ���
 **����ֵ����
 **˵����
 ********************************************************************/
void ECHO_irqhandler(int IRQn, void *param)
{
    ls1x_pwm_timer_stop(devPWM2);   //�رն�ʱ��
    distance = Timer*1.7/10/1.2;		//��1.2Ϊ��ʱ��������/1.2
    Timer = 0;
    flag = 1;//�ѽ��ձ�־
}


void Ultrasonic_IO_Config(void)
{
    gpio_enable(TRIG,DIR_OUT);
    gpio_enable(ECHO,DIR_IN);
    ls1x_install_gpio_isr(ECHO,INT_TRIG_EDGE_DOWN,ECHO_irqhandler,NULL);//�½��ش��� gpio �ж�
    ls1x_enable_gpio_interrupt(ECHO);//ʹ���ж�
    //��������Ĭ������Ϊ�͵�ƽ
    gpio_write(TRIG,0);
}

void Ultrasonic_Init(void)
{
    PWM2_Time_Init();
    Ultrasonic_IO_Config();
}

void Ultrasonic_DeInit(void)
{
    gpio_disable(TRIG);
    gpio_disable(ECHO);
    ls1x_disable_gpio_interrupt(ECHO);//�ر��ж�
    ls1x_pwm_timer_stop(devPWM2);
    
}

/*******************************************************************
 **�������ܣ���������ຯ��
 **�βΣ���
 **����ֵ����õľ��룬��λ����
 **˵����s=Vt V=340m/s
 *******************************************************************/

float Ultrasonic_Get_Dist(void)
{
    uint8_t i=0;
    flag = 0;
    for(i=0; i<4; i++)
    {
        gpio_write(TRIG,1);
        delay_us(12);
        gpio_write(TRIG,0);
        delay_us(12);
    }
    ls1x_pwm_timer_start(devPWM2,(void *)&pwm2_cfg);    //�򿪶�ʱ��
    delay_ms(100);

    //�ж��Ƿ�ɹ����յ�����
    if(flag)
    {
        if(distance>=2000)//���Χ
        {
            distance=2000;
        }
        if(distance<=0)//���ݴ��� ��ʾ��һ������
        {
            distance=last_distance;
        }
        last_distance=distance;
    }else
    {
        return 9999.99;//������Χ
    }
    
    return distance;
}





