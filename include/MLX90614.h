#ifndef _MLX90614_H
#define _MLX90614_H
#include "mylib.h"
#include "ls1b_gpio.h"

#define ACK	 0 //应答
#define	NACK 1 //无应答
#define SA				0x00 //Slave address 单个MLX90614时地址为0x00,多个时地址默认为0x5a
#define RAM_ACCESS		0x00 //RAM access command RAM存取命令
#define EEPROM_ACCESS	0x20 //EEPROM access command EEPROM存取命令
#define RAM_TOBJ1		0x07 //To1 address in the eeprom 目标1温度,检测到的红外温度 -70.01 ~ 382.19度

#define GPIO_SMBUS_SCK 32
#define GPIO_SMBUS_SDA 33

/*
#define SMBUS_SCK_H()	    gpio_write(GPIO_SMBUS_SCK, 1) //置高电平
#define SMBUS_SCK_L()	    gpio_write(GPIO_SMBUS_SCK, 0) //置低电平
#define SMBUS_SDA_H()	    gpio_write(GPIO_SMBUS_SDA, 1)	//置高电平
#define SMBUS_SDA_L()	    gpio_write(GPIO_SMBUS_SDA, 0) 	//置低电平

#define MLX90614_SDA_IN()  gpio_enable(GPIO_SMBUS_SDA,DIR_IN)	//PB9输入模式
#define MLX90614_SDA_OUT() gpio_enable(GPIO_SMBUS_SDA,DIR_OUT) //PB9输出模式
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
float SMBus_ReadTemp(void); //获取温度值
void SMBus_Delay(uint16_t time);

#endif

