/*
 * @Description: ���������
 * @Autor: 309 Mushroom
 * @Date: 2022-11-12 20:18:47
 * @LastEditors: 309 Mushroom
 * @LastEditTime: 2022-11-17 20:15:39
 * 						�˿ڶ��壺
                        PD15:SER		75HC595���������
                        PC8:SCK			75HC595����ʱ��
                        PC7:RCK			75HC595����ʱ��
                        PD10:OE			75HC595���ʹ�ܿ��ƶ�
                        PD11:SMG_1	�����λѡ1
                        PD12:SMG_2	�����λѡ2
                        PD13:SMG_3	�����λѡ3
                        PD14:SMG_4	�����λѡ4	
 */
#include "SMG.h"

uint8_t SMG_MODE = 0;	//��ʾģʽ
uint16_t SMG_Data;		//����ܶ�̬ɨ������
int16_t SMG_DataARR[4]; //����ܶ�̬ɨ�����λ���ϵ�����
uint8_t SMG_task_status = 0; //���������״̬
uint8_t SMGSWAP = 21;//ѡ����������� 0-23
/*8λ����ܹ�������*/
//uint8_t SEG_Table[17]={0xC0,0xF9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xff};
//����                   0   1    2     3    4    5    6    7    8    9    A    B    C    D    E    F   �ر�
/*8λ����ܹ�������*/  // 0      1    2     3     4      5     6    7     8      9     A     B     C     D     E     F  �ر�
uint8_t SEG_Table[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x00,
0x40, 0x80};
// -    .
/*
dp dg df de dd dc db da
0 0 0 0 0 0 0 0
   da
   -
df| |db
   - dg
de| |dc
   -  .dp
   dd
*/


TaskHandle_t smg_task_handle;
uint8_t TIM4_Count = 0;
void smg_task(void *arg)
{
	while (1)
	{
		if (SMG_MODE == 0) //��ֵ��ʾ
		{
			TIM4_Count += 1;
			switch (TIM4_Count)
			{
			case 1:
			{
				SMG_Display_Bit(SMG_Data / 1000, 4); //����ܵ���λ��ʾ
				break;
			}
			case 2:
			{
				SMG_Display_Bit(SMG_Data % 1000 / 100, 3); //����ܵ���λ��ʾ
				break;
			}
			case 3:
			{
				SMG_Display_Bit(SMG_Data % 100 / 10, 2); //����ܵ���λ��ʾ
				break;
			}
			case 4:
			{
				SMG_Display_Bit(SMG_Data % 10, 1); //����ܵ���λ��ʾ
				break;
			}
			}
		}
		else //��ʾ����λ�õ���
		{
			TIM4_Count += 1;
			switch (TIM4_Count)
			{
			case 1:
			{
				if (SMG_DataARR[3] != -1)
				{
					SMG_Display_Bit(SMG_DataARR[3], 4); //����ܵ���λ��ʾ
				}
				break;
			}
			case 2:
			{
				if (SMG_DataARR[2] != -1)
				{
					SMG_Display_Bit(SMG_DataARR[2], 3); //����ܵ���λ��ʾ
				}
				break;
			}
			case 3:
			{
				if (SMG_DataARR[1] != -1)
				{
					SMG_Display_Bit(SMG_DataARR[1], 2); //����ܵ���λ��ʾ
				}
				break;
			}
			case 4:
			{
				if (SMG_DataARR[0] != -1)
				{
					SMG_Display_Bit(SMG_DataARR[0], 1); //����ܵ���λ��ʾ
				}
				break;
			}
			}
		}
		if (TIM4_Count >= 4)
		{ //�����ռ���
			TIM4_Count = 0;
		}
		delay_ms(5);
	}
}

void SMG_start()
{
    if(SMG_task_status)
    {
        vTaskResume(smg_task_handle);//�ָ�����

       }else
        {
            if(xTaskCreate(smg_task,"smgtask",2048,NULL,30,&smg_task_handle) == pdPASS)
            {
                SMG_task_status=1;//�����ɹ�
		      }else
		      {
			     SMG_task_status=0;//����ʧ��
		      }
        }
}

void SMG_stop()
{
    vTaskSuspend(smg_task_handle);//��������
}

/**
 * @description: ����ܶ˿ڳ�ʼ��
 * @param {*}
 * @return {*}
 */
void SMG_Init(void)
{
	/*
    GPIO_InitTypeDef   GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD, ENABLE);       //����GPIOC��ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       //���ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //�ٶ�50MHz
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       //���ģʽ
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //�ٶ�50MHz
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOD,GPIO_Pin_10);
    */

	gpio_enable(GPIO_SMG_SI, DIR_OUT);
	gpio_enable(GPIO_SMG_SCK, DIR_OUT);
	gpio_enable(GPIO_SMG_RCK, DIR_OUT);
	gpio_enable(GPIO_SMG_COM1, DIR_OUT);
	gpio_enable(GPIO_SMG_COM2, DIR_OUT);
	gpio_enable(GPIO_SMG_COM3, DIR_OUT);
	gpio_enable(GPIO_SMG_COM4, DIR_OUT);
	if(xTaskCreate(smg_task,"smgtask",2048,NULL,30,&smg_task_handle) == pdPASS)
	{
		//�����ɹ�
		SMG_task_status=1;
	}else
	{
		//����ʧ��
		SMG_task_status=0;
    }

}

/**
 * @description: 
 * @param {*}
 * @return {*}
 */
void SMG_DeInit(void)
{
	gpio_disable(GPIO_SMG_SI);
	gpio_disable(GPIO_SMG_SCK);
	gpio_disable(GPIO_SMG_RCK);
	gpio_disable(GPIO_SMG_COM1);
	gpio_disable(GPIO_SMG_COM2);
	gpio_disable(GPIO_SMG_COM3);
	gpio_disable(GPIO_SMG_COM4);
	
	vTaskDelete(smg_task_handle);
	SMG_task_status=0;
}

/**
  * @brief  ����ܵ���λ��ʾ
  * @param  Input_Data������λ���� Bit_Data��λ
  * @retval	None 
  * @note		None
  */
void SMG_Display_Bit(uint8_t Input_Data, uint8_t Bit_Data)
{
	/*��ѡ*/
	uint8_t Data_Bit;
	int i = 0;
	for (i = 0; i < 8; i++) //��i=1ʱ�������ʾС����
	{
		Data_Bit = SEG_Table[Input_Data]; //���� |0x80
		if (((Data_Bit) << i) & 0x80)	  //�ж����������			�� 1 ���� 0
		{
			SER_H;
		}
		else
		{
			SER_L;
		}
		delay_us(1);
		SCK_L; //��λ(��ȷ�����������֮�������λ�����������ݴ���Ĵ�������)
        delay_us(1);
        SCK_H;
	}
	RCK_L; //�Ĵ�����������ݴ��������
	delay_us(3);
	RCK_H;

	/*λѡ*/
	SMG_Position(Bit_Data);
}

/**
  * @brief  �������ʾ
  * @param  Input_Data������ 
  * @param  Ctrl_Bit:���ؿ���
  * @retval	None 
  * @note		None
  */
void SMG_Display(uint16_t Input_Data, uint8_t Ctrl_Bit)
{
	if (Ctrl_Bit)
	{
		SMG_MODE = 0;
		SMG_Data = Input_Data;
		
		SMG_start();
		
		//HAL_TIM_Base_Start_IT(&TIM4_Handler); //ʹ�ܶ�ʱ��4�Ͷ�ʱ��4�����жϣ�TIM_IT_UPDATE
	}
	else
	{
		//HAL_TIM_Base_Stop_IT(&TIM4_Handler); //�رն�ʱ��13�Ͷ�ʱ��13�����жϣ�TIM_IT_UPDATE
		//vTaskDelete(smg_task_handle);
		
		SMG_stop();
		SMG_1_L;
		SMG_2_L;
		SMG_3_L;
		SMG_4_L;
	}
}

/**
 * @description: �������ʾ����λ��
 * @param {int16_t} Input_Data4 -1�ر�
 * @param {int16_t} Input_Data3
 * @param {int16_t} Input_Data2
 * @param {int16_t} Input_Data1
 * @param {uint8_t} Ctrl_Bit ���ض�ʱ������
 * @return {*}
 */
void SMG_DisplayP(int16_t Input_Data4, int16_t Input_Data3, int16_t Input_Data2, int16_t Input_Data1, uint8_t Ctrl_Bit)
{
	if (Ctrl_Bit)
	{
		SMG_MODE = 1;
		SMG_DataARR[3] = Input_Data4;
		SMG_DataARR[2] = Input_Data3;
		SMG_DataARR[1] = Input_Data2;
		SMG_DataARR[0] = Input_Data1;
		
		SMG_start();
		//HAL_TIM_Base_Start_IT(&TIM4_Handler); //ʹ�ܶ�ʱ��4�Ͷ�ʱ��4�����жϣ�TIM_IT_UPDATE
	}
	else
	{
		//HAL_TIM_Base_Stop_IT(&TIM4_Handler); //�رն�ʱ��13�Ͷ�ʱ��13�����жϣ�TIM_IT_UPDATE
		
		SMG_stop();
		SMG_1_L;
		SMG_2_L;
		SMG_3_L;
		SMG_4_L;
	}
}

void position1(void)
{
	SMG_1_L;
	SMG_2_H;
	SMG_3_L;
	SMG_4_L;
}
void position2()
{
	SMG_1_H;
	SMG_2_L;
	SMG_3_L;
	SMG_4_L;
}
void position3()
{
	SMG_1_L;
	SMG_2_L;
	SMG_3_L;
	SMG_4_H;
}
void position4()
{
	SMG_1_L;
	SMG_2_L;
	SMG_3_H;
	SMG_4_L;
}

void SMG_Position(uint8_t Bit_Data)
{
	/*
1 2 3 4
1 2 4 3
1 3 2 4 
1 3 4 2
1 4 2 3 
1 4 3 2

2 1 3 4
2 1 4 3
2 3 1 4
2 3 4 1
2 4 1 3
2 4 3 1

3 1 2 4
3 1 4 2
3 2 1 4
3 2 4 1
3 4 1 2
3 4 2 1

4 1 2 3 
4 1 3 2
4 2 1 3
4 2 3 1
4 3 1 2
4 3 2 4
*/

	switch (SMGSWAP) //��ͨ�����ڴ�ֵ���������λ��ѡ��
	{
	case 0:
		//����
		switch (Bit_Data)
		{
		case 1:
			position1();
			break;
		case 2:
			position2();
			break;
		case 3:
			position3();
			break;
		case 4:
			position4();
			break;
		}
		break;
	case 1:

		switch (Bit_Data)
		{
		case 1:
			position1();
			break;
		case 2:
			position2();
			break;
		case 3:
			position4();
			break;
		case 4:
			position3();
			break;
		}
		break;
	case 2:

		switch (Bit_Data)
		{
		case 1:
			position1();
			break;
		case 2:
			position3();
			break;
		case 3:
			position2();
			break;
		case 4:
			position4();
			break;
		}
		break;
	case 3:

		switch (Bit_Data)
		{
		case 1:
			position1();
			break;
		case 2:
			position3();
			break;
		case 3:
			position4();
			break;
		case 4:
			position2();
			break;
		}
		break;
	case 4:

		switch (Bit_Data)
		{
		case 1:
			position1();
			break;
		case 2:
			position4();
			break;
		case 3:
			position2();
			break;
		case 4:
			position3();
			break;
		}
		break;
	case 5:

		switch (Bit_Data)
		{
		case 1:
			position1();
			break;
		case 2:
			position4();
			break;
		case 3:
			position3();
			break;
		case 4:
			position2();
			break;
		}
		break;
	case 6:

		switch (Bit_Data)
		{
		case 1:
			position2();
			break;
		case 2:
			position1();
			break;
		case 3:
			position3();
			break;
		case 4:
			position4();
			break;
		}
		break;
	case 7:

		switch (Bit_Data)
		{
		case 1:
			position2();
			break;
		case 2:
			position1();
			break;
		case 3:
			position4();
			break;
		case 4:
			position3();
			break;
		}
		break;
	case 8:

		switch (Bit_Data)
		{
		case 1:
			position2();
			break;
		case 2:
			position3();
			break;
		case 3:
			position1();
			break;
		case 4:
			position4();
			break;
		}
		break;
	case 9:

		switch (Bit_Data)
		{
		case 1:
			position2();
			break;
		case 2:
			position3();
			break;
		case 3:
			position4();
			break;
		case 4:
			position1();
			break;
		}
		break;
	case 10:

		switch (Bit_Data)
		{
		case 1:
			position2();
			break;
		case 2:
			position4();
			break;
		case 3:
			position1();
			break;
		case 4:
			position3();
			break;
		}
		break;
	case 11:

		switch (Bit_Data)
		{
		case 1:
			position2();
			break;
		case 2:
			position4();
			break;
		case 3:
			position3();
			break;
		case 4:
			position1();
			break;
		}
		break;
	case 12:

		switch (Bit_Data)
		{
		case 1:
			position3();
			break;
		case 2:
			position1();
			break;
		case 3:
			position2();
			break;
		case 4:
			position4();
			break;
		}
		break;
	case 13:

		switch (Bit_Data)
		{
		case 1:
			position3();
			break;
		case 2:
			position1();
			break;
		case 3:
			position4();
			break;
		case 4:
			position2();
			break;
		}
		break;
	case 14:

		switch (Bit_Data)
		{
		case 1:
			position3();
			break;
		case 2:
			position2();
			break;
		case 3:
			position1();
			break;
		case 4:
			position4();
			break;
		}
		break;
	case 15:

		switch (Bit_Data)
		{
		case 1:
			position3();
			break;
		case 2:
			position2();
			break;
		case 3:
			position4();
			break;
		case 4:
			position1();
			break;
		}
		break;
	case 16:

		switch (Bit_Data)
		{
		case 1:
			position4();
			break;
		case 2:
			position1();
			break;
		case 3:
			position2();
			break;
		case 4:
			position3();
			break;
		}
		break;
	case 17:

		switch (Bit_Data)
		{
		case 1:
			position4();
			break;
		case 2:
			position1();
			break;
		case 3:
			position3();
			break;
		case 4:
			position2();
			break;
		}
		break;
	case 18:

		switch (Bit_Data)
		{
		case 1:
			position4();
			break;
		case 2:
			position2();
			break;
		case 3:
			position1();
			break;
		case 4:
			position3();
			break;
		}
		break;
	case 19:

		switch (Bit_Data)
		{
		case 1:
			position4();
			break;
		case 2:
			position3();
			break;
		case 3:
			position1();
			break;
		case 4:
			position2();
			break;
		}
		break;
	case 20:

		switch (Bit_Data)
		{
		case 1:
			position4();
			break;
		case 2:
			position3();
			break;
		case 3:
			position2();
			break;
		case 4:
			position1();
			break;
		}
		break;
	case 21:

		switch (Bit_Data)
		{
		case 1:
			position3();
			break;
		case 2:
			position4();
			break;
		case 3:
			position1();
			break;
		case 4:
			position2();
			break;
		}
		break;
	case 22:

		switch (Bit_Data)
		{
		case 1:
			position3();
			break;
		case 2:
			position4();
			break;
		case 3:
			position2();
			break;
		case 4:
			position1();
			break;
		}
		break;
	case 23:

		switch (Bit_Data)
		{
		case 1:
			position4();
			break;
		case 2:
			position2();
			break;
		case 3:
			position1();
			break;
		case 4:
			position3();
			break;
		}
		break;
	}
}
