/*
 * pwm_ic.c
 *
 * created: 2022/3/2
 *  author: 
 */

 #include "pwm_ic.h"
 #include "ls1x_i2c_bus.h"

 #define WR_8BIT_CMD            0x03
 #define WR_16BIT_CMD           0X02
 #define GP7101_ADDRESS         0x58
 #define GP7101_BAUDRATE        1000000

 //MUX����ַ�ͼĴ�����ַ
 #define GPIO_MUX_BASE    0XBFD00420
 /*
  * i=0: GPIO_MUX_CTRL0; i=1: GPIO_MUX_CTRL1
  */
 #define LS1B_MUX_CFG(i)        (*(volatile unsigned int*)(GPIO_MUX_BASE+i*4))

/***************************************************************************************************
 ** �������ܣ���ʼ��I2C1
 ** ��������
 ** ˵����
****************************************************************************************************/
 void IIC0_init(void)
 {
    /* i=0: GPIO30:GPIO0; i=1: GPIO61:GPIO32*/
    //LS1B_GPIO_CFG(1)  &= ~(3 << 6);    	  //���ö�ӦPADΪ��ͨ����
    //LS1B_MUX_CFG(0)   |= 1<< 24;          //����IIC1�ĸ���

    //��ʼ��
    ls1x_i2c_initialize(busI2C0);
    //��������
    //ls1x_i2c_ioctl(busI2C0, IOCTL_SPI_I2C_SET_TFRMODE, (void *)GP7101_BAUDRATE);

 }

//#if 1
 //����20ms
 //�����ȷ�Χ900~2100us -- 0.9~2.1ms -->11~27 -->8λ
 void Set_PWM(unsigned int brightpercent)
 {
    unsigned char brightness = brightpercent * 255.0 / 10000 ;
    unsigned char data[2] = {0};

    //8λPWMģʽ
    data[0] = WR_8BIT_CMD;
    //����
    data[1] = brightness;

    //��ʼ�ź�
    ls1x_i2c_send_start(busI2C0, GP7101_ADDRESS);

    //���͵�ַ
    ls1x_i2c_send_addr(busI2C0, GP7101_ADDRESS, false);

    //д������
    ls1x_i2c_write_bytes(busI2C0, data, 2);

    //ֹͣ�ź�
    ls1x_i2c_send_stop(busI2C0, GP7101_ADDRESS);
    
 }
//#endif

//�����ȷ�Χ500~1500~2500us -- 0.5~1.5~2.5ms -->1638~8192 -->16λ
//8350 -- 4720 -- 1680
/***************************************************************************************************
 ** �������ܣ����ƶ����ƫת�Ƕ�
 ** ������unsigned char brightpercent���������ֵΪ��0 ~ 120
 ** ˵��������20ms
          FS90ƫת�Ƕ�0~120��
          �����ȷ�Χ900~1500~2100us -- 0.9~2.1ms -->2949~6881 -->16λ
          0�̶ȵ���ֵ--6650
          90�̶ȵ���ֵ--4915
          120�̶ȵ���ֵ--2940
****************************************************************************************************/
/*
void Set_PWM(unsigned short brightpercent)
{
    unsigned char data[3] = {0};
    unsigned short brightness = brightpercent;

#if 0
    if(brightpercent >= 95)
    {
        brightness = 3325 - (brightpercent - 95) * 10;//ÿ��10��ָ��ƫתһ��
    }
    else
    {
        brightness = 6650 - brightpercent * 35;//ÿ��35��ָ��ƫתһ��
    }
#endif

#if 1
    if(brightpercent >= 140)
    {
        brightness = 2880 - (brightpercent - 140) * 30; //ÿ��30��ָ��ƫתһ��
    }
    else
    {
        brightness = 8350 - brightpercent * 40;         //ÿ��40��ָ��ƫתһ��
    }
#endif

    //16λPWMģʽ
    data[0] = WR_16BIT_CMD;
    //����
    data[1] = brightness;
    data[2] = brightness >> 8;

    //��ʼ�ź�
    ls1x_i2c_send_start(busI2C1, GP7101_ADDRESS);

    //���͵�ַ
    ls1x_i2c_send_addr(busI2C1, GP7101_ADDRESS, false);

    //д������
    ls1x_i2c_write_bytes(busI2C1, data, 3);

    //ֹͣ�ź�
    ls1x_i2c_send_stop(busI2C1, GP7101_ADDRESS);
}

*/


