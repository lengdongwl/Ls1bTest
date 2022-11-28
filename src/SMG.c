/*
 * @Description: 数码管驱动
 * @Autor: 309 Mushroom
 * @Date: 2022-11-12 20:18:47
 * @LastEditors: 309 Mushroom
 * @LastEditTime: 2022-11-17 20:15:39
 * 						端口定义：
                        PD15:SER		75HC595数据输入段
                        PC8:SCK			75HC595串行时钟
                        PC7:RCK			75HC595并行时钟
                        PD10:OE			75HC595输出使能控制端
                        PD11:SMG_1	数码管位选1
                        PD12:SMG_2	数码管位选2
                        PD13:SMG_3	数码管位选3
                        PD14:SMG_4	数码管位选4	
 */
#include "SMG.h"

uint8_t SMG_MODE = 0;	//显示模式
uint16_t SMG_Data;		//数码管动态扫描数据
int16_t SMG_DataARR[4]; //数码管动态扫描各个位置上的数据
uint8_t SMG_task_status = 0; //数码管任务状态
uint8_t SMGSWAP = 21;//选择数码管排序 0-23
/*8位数码管共阳段码*/
//uint8_t SEG_Table[17]={0xC0,0xF9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e,0xff};
//共阳                   0   1    2     3    4    5    6    7    8    9    A    B    C    D    E    F   关闭
/*8位数码管共阴段码*/  // 0      1    2     3     4      5     6    7     8      9     A     B     C     D     E     F  关闭
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
		if (SMG_MODE == 0) //数值显示
		{
			TIM4_Count += 1;
			switch (TIM4_Count)
			{
			case 1:
			{
				SMG_Display_Bit(SMG_Data / 1000, 4); //数码管单个位显示
				break;
			}
			case 2:
			{
				SMG_Display_Bit(SMG_Data % 1000 / 100, 3); //数码管单个位显示
				break;
			}
			case 3:
			{
				SMG_Display_Bit(SMG_Data % 100 / 10, 2); //数码管单个位显示
				break;
			}
			case 4:
			{
				SMG_Display_Bit(SMG_Data % 10, 1); //数码管单个位显示
				break;
			}
			}
		}
		else //显示各个位置的数
		{
			TIM4_Count += 1;
			switch (TIM4_Count)
			{
			case 1:
			{
				if (SMG_DataARR[3] != -1)
				{
					SMG_Display_Bit(SMG_DataARR[3], 4); //数码管单个位显示
				}
				break;
			}
			case 2:
			{
				if (SMG_DataARR[2] != -1)
				{
					SMG_Display_Bit(SMG_DataARR[2], 3); //数码管单个位显示
				}
				break;
			}
			case 3:
			{
				if (SMG_DataARR[1] != -1)
				{
					SMG_Display_Bit(SMG_DataARR[1], 2); //数码管单个位显示
				}
				break;
			}
			case 4:
			{
				if (SMG_DataARR[0] != -1)
				{
					SMG_Display_Bit(SMG_DataARR[0], 1); //数码管单个位显示
				}
				break;
			}
			}
		}
		if (TIM4_Count >= 4)
		{ //溢出清空计数
			TIM4_Count = 0;
		}
		delay_ms(5);
	}
}

void SMG_start()
{
    if(SMG_task_status)
    {
        vTaskResume(smg_task_handle);//恢复任务

       }else
        {
            if(xTaskCreate(smg_task,"smgtask",2048,NULL,30,&smg_task_handle) == pdPASS)
            {
                SMG_task_status=1;//创建成功
		      }else
		      {
			     SMG_task_status=0;//创建失败
		      }
        }
}

void SMG_stop()
{
    vTaskSuspend(smg_task_handle);//挂起任务
}

/**
 * @description: 数码管端口初始化
 * @param {*}
 * @return {*}
 */
void SMG_Init(void)
{
	/*
    GPIO_InitTypeDef   GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC|RCC_AHB1Periph_GPIOD, ENABLE);       //开启GPIOC的时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       //输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //速度50MHz
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       //输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //速度50MHz
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
		//创建成功
		SMG_task_status=1;
	}else
	{
		//创建失败
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
  * @brief  数码管单个位显示
  * @param  Input_Data：单个位数据 Bit_Data：位
  * @retval	None 
  * @note		None
  */
void SMG_Display_Bit(uint8_t Input_Data, uint8_t Bit_Data)
{
	/*段选*/
	uint8_t Data_Bit;
	int i = 0;
	for (i = 0; i < 8; i++) //当i=1时数码管显示小数点
	{
		Data_Bit = SEG_Table[Input_Data]; //带点 |0x80
		if (((Data_Bit) << i) & 0x80)	  //判断输入的数据			是 1 或者 0
		{
			SER_H;
		}
		else
		{
			SER_L;
		}
		delay_us(1);
		SCK_L; //移位(当确定输入的数据之后进行移位操作，把数据存进寄存器里面)
        delay_us(1);
        SCK_H;
	}
	RCK_L; //寄存器里面的数据传给数码管
	delay_us(3);
	RCK_H;

	/*位选*/
	SMG_Position(Bit_Data);
}

/**
  * @brief  数码管显示
  * @param  Input_Data：数据 
  * @param  Ctrl_Bit:开关控制
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
		
		//HAL_TIM_Base_Start_IT(&TIM4_Handler); //使能定时器4和定时器4更新中断：TIM_IT_UPDATE
	}
	else
	{
		//HAL_TIM_Base_Stop_IT(&TIM4_Handler); //关闭定时器13和定时器13更新中断：TIM_IT_UPDATE
		//vTaskDelete(smg_task_handle);
		
		SMG_stop();
		SMG_1_L;
		SMG_2_L;
		SMG_3_L;
		SMG_4_L;
	}
}

/**
 * @description: 数码管显示各个位置
 * @param {int16_t} Input_Data4 -1关闭
 * @param {int16_t} Input_Data3
 * @param {int16_t} Input_Data2
 * @param {int16_t} Input_Data1
 * @param {uint8_t} Ctrl_Bit 开关定时器控制
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
		//HAL_TIM_Base_Start_IT(&TIM4_Handler); //使能定时器4和定时器4更新中断：TIM_IT_UPDATE
	}
	else
	{
		//HAL_TIM_Base_Stop_IT(&TIM4_Handler); //关闭定时器13和定时器13更新中断：TIM_IT_UPDATE
		
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

	switch (SMGSWAP) //可通过调节此值调节数码管位置选择
	{
	case 0:
		//正常
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
