#ifndef _MLX90614_H
#define _MLX90614_H
#include "mylib.h"
#include "ls1b_gpio.h"

#define ACK	 0 //Ӧ��
#define	NACK 1 //��Ӧ��
#define SA				0x00 //Slave address ����MLX90614ʱ��ַΪ0x00,���ʱ��ַĬ��Ϊ0x5a
#define RAM_ACCESS		0x00 //RAM access command RAM��ȡ����
#define EEPROM_ACCESS	0x20 //EEPROM access command EEPROM��ȡ����
#define RAM_TOBJ1		0x07 //To1 address in the eeprom Ŀ��1�¶�,��⵽�ĺ����¶� -70.01 ~ 382.19��

#define GPIO_SMBUS_SCK 32
#define GPIO_SMBUS_SDA 33

/*
#define SMBUS_SCK_H()	    gpio_write(GPIO_SMBUS_SCK, 1) //�øߵ�ƽ
#define SMBUS_SCK_L()	    gpio_write(GPIO_SMBUS_SCK, 0) //�õ͵�ƽ
#define SMBUS_SDA_H()	    gpio_write(GPIO_SMBUS_SDA, 1)	//�øߵ�ƽ
#define SMBUS_SDA_L()	    gpio_write(GPIO_SMBUS_SDA, 0) 	//�õ͵�ƽ

#define MLX90614_SDA_IN()  gpio_enable(GPIO_SMBUS_SDA,DIR_IN)	//PB9����ģʽ
#define MLX90614_SDA_OUT() gpio_enable(GPIO_SMBUS_SDA,DIR_OUT) //PB9���ģʽ
*/
#define SMBUS_SDA_PIN()   gpio_read(GPIO_SMBUS_SDA)

void MLX90614_Init(void);
void SMBus_StartBit(void);
void SMBus_StopBit(void);
void SMBus_SendBit(uint8_t bit_out);
uint8_t SMBus_SendByte(uint8_t Tx_buffer);
uint8_t SMBus_ReceiveBit(uint8_t ack_nack);
uint8_t SMBus_ReceiveByte(uint8_t ack_nack);
void SMBus_Delay(uint16_t t);
uint16_t SMBus_ReadMemory(uint8_t, uint8_t);
uint8_t PEC_Calculation(uint8_t* data);
float SMBus_ReadTemp(void); //��ȡ�¶�ֵ
void SMBus_Delay(uint16_t time);

#endif

