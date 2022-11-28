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
// 按键中断的标志
static bool flag = 0;
static unsigned int cnt = 0;
static float distance;
static float last_distance;

//用于计时
static volatile unsigned int Timer = 0;

//定义一个PWM的结构体
pwm_cfg_t pwm2_cfg;
//PWM2中断处理函数
static void pwmtimer_callback(void *pwm, int *stopit)
{
    Timer++;
}

/*************************************************************************************
**函数功能：使用PWM2实现约10us定时
**说明：此定时器主要用超声波测距计时
**************************************************************************************/
void PWM2_Time_Init(void)
{
    pwm2_cfg.hi_ns = 5000;  //高电平脉冲宽度(纳秒), 定时器模式仅用 hi_ns（约等于1us）
    pwm2_cfg.lo_ns = 0;         //低电平脉冲宽度(纳秒),定时器模式没用 lo_ns
    pwm2_cfg.mode = PWM_CONTINUE_TIMER; //脉冲持续产生
    pwm2_cfg.cb = pwmtimer_callback;    //定时器中断回调函数
    pwm2_cfg.isr = NULL;                //工作在定时器模式

    ls1x_pwm_init(devPWM2,NULL);
    ls1x_pwm_open(devPWM2,(void *)&pwm2_cfg);
    ls1x_pwm_timer_stop(devPWM2);
}


/*********************************************************************
 **函数功能：ECHO返回下降沿产生中断
 **形参：
    @IRQn  中断号
    @param 传递给中断处理函数的参数
 **返回值：无
 **说明：
 ********************************************************************/
void ECHO_irqhandler(int IRQn, void *param)
{
    ls1x_pwm_timer_stop(devPWM2);   //关闭定时器
    distance = Timer*1.7/10/1.2;		//（1.2为定时器补偿）/1.2
    Timer = 0;
}


void Ultrasonic_IO_Config(void)
{
    gpio_enable(TRIG,DIR_OUT);
    gpio_enable(ECHO,DIR_IN);
    ls1x_install_gpio_isr(ECHO,INT_TRIG_EDGE_DOWN,ECHO_irqhandler,NULL);//下降沿触发 gpio 中断
    ls1x_enable_gpio_interrupt(ECHO);//使能中断
    //发送引脚默认设置为低电平
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
    ls1x_disable_gpio_interrupt(ECHO);//关闭中断
    ls1x_pwm_timer_stop(devPWM2);
    
}

/*******************************************************************
 **函数功能：超声波测距函数
 **形参：无
 **返回值：测得的距离，单位厘米
 **说明：s=Vt V=340m/s
 *******************************************************************/

float Ultrasonic_Get_Dist(void)
{
    uint8_t i=0;
    for(i=0; i<4; i++)
    {
        gpio_write(TRIG,1);
        delay_us(12);
        gpio_write(TRIG,0);
        delay_us(12);
    }
    ls1x_pwm_timer_start(devPWM2,(void *)&pwm2_cfg);    //打开定时器
    delay_ms(100);

    if(distance>=2000)
    {
        distance=2000;
    }
    if(distance<=0)
    {
        distance=last_distance;
    }
    last_distance=distance;
    return distance;
}





