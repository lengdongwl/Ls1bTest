/**
  ******************************************************************************
  * @file    rc522.c
  * @author  xie
  * @version V2
  * @date    2021年11月2日
  * @brief   扩展端口模块RC552 RFID读卡器驱动函数库
	
						端口定义：
							PC12:	MY_IIC_SCL		MPU6050IIC数据脚	
							PD2:	MY_IIC_SDA		MPU6050IIC时钟脚	

	******************************************************************************
**/

#include <string.h>
#include "rc522.h"

#include "bsp.h"
#ifdef BSP_USE_I2C0

#include "ls1x_i2c_bus.h"
#endif

#define  MODE         1                    	 // 模式  1：先写后读   0：只读
#define  DATA_LEN    16                      // 定义数据字节长度

/*全局变量*/
unsigned char CT[2];//卡类型
unsigned char SN[4]; //卡号
unsigned char RFID[16];			//存放RFID
uint8_t KEY[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t KEY_A[6]= {0xff,0xff,0xff,0xff,0xff,0xff};

uint8_t WRITE_RFID[DATA_LEN+2]="AaBbCcDdEe123456";
uint8_t READ_RFID[DATA_LEN+2]= {0};
uint8_t buffer_data[99];                                  //RC_check_read数据缓存区
void RC522_Init(void)
{
	
	InitRc522();		//RC522 芯片初始化
}

/**************************************************************
*功  能：RC522 芯片初始化
*参  数: 无
*返回值: 无
**************************************************************/
void InitRc522(void)
{
    PcdReset();
    PcdAntennaOff();
    delay_ms(2);
    PcdAntennaOn();
    M500PcdConfigISOType( 'A' );
}


void Reset_RC522(void)
{
    PcdReset();
    PcdAntennaOff();
    delay_ms(2);
    PcdAntennaOn();
}

/**************************************************************
*功  能：寻卡
*参  数: req_code[IN]:寻卡方式
*		 0x52 = 寻感应区内所有符合14443A标准的卡
*		 0x26 = 寻未进入休眠状态的卡
*		 pTagType[OUT]：卡片类型代码
*		 0x4400 = Mifare_UltraLight
*		 0x0400 = Mifare_One(S50)
*		 0x0200 = Mifare_One(S70)
*		 0x0800 = Mifare_Pro(X)
*		 0x4403 = Mifare_DESFire
*返回值: 成功返回MI_OK
**************************************************************/
char PcdRequest(uint8_t   req_code,uint8_t *pTagType)
{
    char   status;
    uint8_t   unLen;
    uint8_t   ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x07);
    SetBitMask(TxControlReg,0x03);

    ucComMF522Buf[0] = req_code;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);

    if ((status == MI_OK) && (unLen == 0x10))
    {
        *pTagType     = ucComMF522Buf[0];
        *(pTagType+1) = ucComMF522Buf[1];
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/**************************************************************
*功  能：防冲撞
*参  数: pSnr[OUT]:卡片序列号，4字节
*返回值: 成功返回MI_OK
**************************************************************/
char PcdAnticoll(uint8_t *pSnr)
{
    char   status;
    uint8_t   i,snr_check=0;
    uint8_t   unLen;
    uint8_t   ucComMF522Buf[MAXRLEN];


    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
        for (i=0; i<4; i++)
        {
            *(pSnr+i)  = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {
            status = MI_ERR;
        }
    }

    SetBitMask(CollReg,0x80);
    return status;
}

/**************************************************************
*功  能：选定卡片
*参  数: pSnr[IN]:卡片序列号，4字节
*返回值: 成功返回MI_OK
**************************************************************/
char PcdSelect(uint8_t *pSnr)
{
    char   status;
    uint8_t   i;
    uint8_t   unLen;
    uint8_t   ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
        ucComMF522Buf[i+2] = *(pSnr+i);
        ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);

    ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);

    if ((status == MI_OK) && (unLen == 0x18))
    {
        status = MI_OK;
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/**************************************************************
*功  能：验证卡片密码
*参  数: auth_mode[IN]: 密码验证模式 0x60 = 验证A密钥 0x61 = 验证B密钥
*		 addr[IN]：块地址   pKey[IN]：密码  pSnr[IN]：卡片序列号，4字节
*返回值: 成功返回MI_OK
**************************************************************/
char PcdAuthState(uint8_t   auth_mode,uint8_t   addr,uint8_t *pKey,uint8_t *pSnr)
{
    char   status;
    uint8_t   unLen;
    uint8_t   ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
//    for (i=0; i<6; i++)
//    {    ucComMF522Buf[i+2] = *(pKey+i);   }
//    for (i=0; i<6; i++)
//    {    ucComMF522Buf[i+8] = *(pSnr+i);   }
    memcpy(&ucComMF522Buf[2], pKey, 6);
    memcpy(&ucComMF522Buf[8], pSnr, 4);

    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {
        status = MI_ERR;
    }

    return status;
}

/**************************************************************
*功  能：读取M1卡一块数据
*参  数: addr[IN]：块地址 pData[OUT]：读出的数据，16字节
*返回值: 成功返回MI_OK
**************************************************************/
char PcdRead(uint8_t   addr,uint8_t *p )
{
    char   status;
    uint8_t   unLen;
    uint8_t   i,ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) && (unLen == 0x90))
//   {   memcpy(p , ucComMF522Buf, 16);   }
    {
        for (i=0; i<16; i++)
        {
            *(p +i) = ucComMF522Buf[i];
        }
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/**************************************************************
*功  能：写数据到M1卡一块
*参  数: addr[IN]：块地址  pData[IN]：写入的数据，16字节
*返回值: 成功返回MI_OK
**************************************************************/
char PcdWrite(uint8_t   addr,uint8_t *p )
{
    char   status;
    uint8_t   unLen;
    uint8_t   i,ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, p , 16);
        for (i=0; i<16; i++)
        {
            ucComMF522Buf[i] = *(p +i);
        }
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }

    return status;
}

/**************************************************************
*功  能：命令卡片进入休眠状态
*参  数: 无
*返回值: 成功返回MI_OK
**************************************************************/
char PcdHalt(void)
{
    uint8_t   status;
    uint8_t   unLen;
    uint8_t   ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return status;
}

/**************************************************************
*功  能：用MF522计算CRC16函数
*参  数: 无
*返回值: 无
**************************************************************/
void CalulateCRC(uint8_t *pIn,uint8_t   len,uint8_t *pOut )
{
    uint8_t   i,n;
    ClearBitMask(DivIrqReg,0x04);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {
        WriteRawRC(FIFODataReg, *(pIn +i));
    }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOut [0] = ReadRawRC(CRCResultRegL);
    pOut [1] = ReadRawRC(CRCResultRegM);
}

/**************************************************************
*功  能：复位RC522
*参  数: 无
*返回值: 成功返回MI_OK
**************************************************************/
char PcdReset(void)
{

    delay_us(10);
    WriteRawRC(CommandReg,PCD_RESETPHASE);
    WriteRawRC(CommandReg,PCD_RESETPHASE);
    delay_us(10);

    WriteRawRC(ModeReg,0x3D);            //和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL,30);
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);
    WriteRawRC(TPrescalerReg,0x3E);

    WriteRawRC(TxAutoReg,0x40);//必须要

    return MI_OK;
}
//////////////////////////////////////////////////////////////////////
//设置RC632的工作方式
//////////////////////////////////////////////////////////////////////
char M500PcdConfigISOType(uint8_t   type)
{
    if (type == 'A')                     //ISO14443_A
    {
        ClearBitMask(Status2Reg,0x08);
        WriteRawRC(ModeReg,0x3D);//3F
        WriteRawRC(RxSelReg,0x86);//84
        WriteRawRC(RFCfgReg,0x7F);   //4F
        WriteRawRC(TReloadRegL,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec)
        WriteRawRC(TReloadRegH,0);
        WriteRawRC(TModeReg,0x8D);
        WriteRawRC(TPrescalerReg,0x3E);
        delay_us(1000);
        PcdAntennaOn();
    }
    else {
        return 1;
    }

    return MI_OK;
}

/**************************************************************
*功  能：置RC522寄存器位
*参  数: reg[IN]:寄存器地址  mask[IN]:置位值
*返回值: 无
**************************************************************/
void SetBitMask(uint8_t   reg,uint8_t   mask)
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/**************************************************************
*功  能：清RC522寄存器位
*参  数: reg[IN]:寄存器地址  mask[IN]:清位值
*返回值: 无
**************************************************************/
void ClearBitMask(uint8_t   reg,uint8_t   mask)
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
}

/**************************************************************
*功  能：通过RC522和ISO14443卡通讯
*参  数: Command[IN]:RC522命令字
*		 pInData[IN]:通过RC522发送到卡片的数据
*		 InLenByte[IN]:发送数据的字节长度
*		 pOutData[OUT]:接收到的卡片返回数据
*		 *pOutLenBit[OUT]:返回数据的位长度
*返回值: 无
**************************************************************/
char PcdComMF522(uint8_t   Command,
                 uint8_t *pIn,
                 uint8_t   InLenByte,
                 uint8_t *pOut,
                 uint8_t *pOutLenBit)
{
    char   status = MI_ERR;
    uint8_t   irqEn   = 0x00;
    uint8_t   waitFor = 0x00;
    uint8_t   lastBits;
    uint8_t   n;
    uint16_t   i;
    switch (Command)
    {
    case PCD_AUTHENT:
        irqEn   = 0x12;
        waitFor = 0x10;
        break;
    case PCD_TRANSCEIVE:
        irqEn   = 0x77;
        waitFor = 0x30;
        break;
    default:
        break;
    }

    WriteRawRC(ComIEnReg,irqEn|0x80);
    ClearBitMask(ComIrqReg,0x80);	//清所有中断位
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);	 	//清FIFO缓存

    for (i=0; i<InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pIn [i]);
    }
    WriteRawRC(CommandReg, Command);
//   	 n = ReadRawRC(CommandReg);

    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg,0x80);     //开始传送
    }

    //i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
    i = 2000;
    do
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80);

    if (i!=0)
    {
        if(!(ReadRawRC(ErrorReg)&0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }
            if (Command == PCD_TRANSCEIVE)
            {
                n = ReadRawRC(FIFOLevelReg);
                lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {
                    *pOutLenBit = (n-1)*8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n*8;
                }
                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAXRLEN)
                {
                    n = MAXRLEN;
                }
                for (i=0; i<n; i++)
                {
                    pOut [i] = ReadRawRC(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }

    }


    SetBitMask(ControlReg,0x80);           // stop timer now
    WriteRawRC(CommandReg,PCD_IDLE);
    return status;
}

/**************************************************************
*功  能：开启天线（每次启动或关闭天险发射之间应至少有1ms的间隔）
*参  数: 无
*返回值: 无
**************************************************************/
void PcdAntennaOn(void)
{
    uint8_t   i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}

/**************************************************************
*功  能：关闭天线
*参  数: 无
*返回值: 无
**************************************************************/
void PcdAntennaOff(void)
{
    ClearBitMask(TxControlReg, 0x03);
}

/////////////////////////////////////////////////////////////////////
//功    能：读RC632寄存器
//参数说明：Address[IN]:寄存器地址
//返    回：读出的值
/////////////////////////////////////////////////////////////////////
uint8_t ReadRawRC(uint8_t   Address)
{
    uint8_t ucResult=0;
    ucResult = RC522_RD_Reg(SLA_ADDR,  Address);
    return ucResult;          		//返回收到的数据

}

/////////////////////////////////////////////////////////////////////
//功    能：写RC632寄存器
//参数说明：Address[IN]:寄存器地址
//          value[IN]:写入的值
/////////////////////////////////////////////////////////////////////
void WriteRawRC(uint8_t   Address, uint8_t   value)
{
    RC522_WR_Reg(SLA_ADDR,  Address, value);
}



/**************************************************************
*功  能：读寄存器
*参  数: addr:寄存器地址
*返回值: 读到的值
**************************************************************/
uint8_t RC522_RD_Reg(uint8_t RCsla,uint8_t addr)
{
    uint8_t temp=0;

    #ifdef BSP_USE_I2C0
    uint8_t BUF[1];

    ls1x_i2c_send_start(busI2C0, RCsla);//开始信号
    if(ls1x_i2c_send_addr(busI2C0,addr,1))//发送设备地址加读信号
    {
        return;
    }
 
    ls1x_i2c_read_bytes(busI2C0, BUF, 1);
    ls1x_i2c_send_stop(busI2C0,RCsla);//发送停止信号
    

    #else

    MY_IIC_Start();
    MY_IIC_Send_Byte(RCsla);	//发送写器件指令
    temp=MY_IIC_Wait_Ack();

    MY_IIC_Send_Byte(addr);   		//发送寄存器地址
    temp=MY_IIC_Wait_Ack();

    MY_IIC_Start();  	 	   		//重新启动
    MY_IIC_Send_Byte(RCsla+1);	//发送读器件指令
    temp=MY_IIC_Wait_Ack();

    temp=MY_IIC_Read_Byte(0);		//读取一个字节,不继续再读,发送NAK
    MY_IIC_Stop();					//产生一个停止条件

    #endif

    printk("buf=%d\n",BUF[0]);
    return BUF[0];				//返回读到的值
    
    
}


//写寄存器
//addr:寄存器地址
//val:要写入的值
//返回值:无
void RC522_WR_Reg(uint8_t RCsla,uint8_t addr,uint8_t val)
{

    #ifdef BSP_USE_I2C0
    uint8_t rt ;
    
    uint8_t BUF[2];
    rt=ls1x_i2c_send_start(busI2C0, RCsla);//起始信号
    rt=ls1x_i2c_send_addr(busI2C0,RCsla,0);	//发送设备地址+写信号
    //CHECK_DONE(rt);
    BUF[0] = addr;
    BUF[1] = val;
    ls1x_i2c_write_bytes(busI2C0, BUF, 1);	//发送值
    ls1x_i2c_send_stop(busI2C0,RCsla);	//发送停止信号


    #else
    MY_IIC_Start();
    MY_IIC_Send_Byte(RCsla);     	//发送写器件指令
    MY_IIC_Wait_Ack();
    MY_IIC_Send_Byte(addr);   			//发送寄存器地址
    MY_IIC_Wait_Ack();
    MY_IIC_Send_Byte(val);     		//发送值
    MY_IIC_Wait_Ack();
    MY_IIC_Stop();						//产生一个停止条件
    
    #endif
}


//等待卡离开
void WaitCardOff(void)
{
    unsigned char status, TagType[2];

    while(1)
    {
        status = PcdRequest(REQ_ALL, TagType);
        if(status)
        {
            status = PcdRequest(REQ_ALL, TagType);
            if(status)
            {
                status = PcdRequest(REQ_ALL, TagType);
                if(status)
                {
                    return;
                }
            }
        }
        delay_ms(10);
    }
}

uint8_t RX_RFID[16];
uint8_t	KEY_Data[6]={0xff,0xff,0xff,0xff,0xff,0xff}; 		//密钥
void Read_Card(void)
{
		if(PcdRequest(PICC_REQALL,CT) == MI_OK)							//寻卡
		{
			if(PcdAnticoll(SN) == MI_OK)											// 防冲撞成功
			{
				if(PcdSelect(SN)==MI_OK)												//选定此卡
				{	
					if(PcdAuthState(0x60,1/4*4+3,KEY_Data,SN)==MI_OK)			//验证密钥
					{
						if(PcdRead(1,RX_RFID)==MI_OK)								//读卡
						{	
							
							
						}
					}
				}
			}
		}
//    uint8_t status;
//    uint8_t s=0x08;
//    status = PcdRequest(PICC_REQALL,CT);/*尋卡*/
//    if(status==MI_OK)//尋卡成功
//    {
//        delay_ms(200);
//        status=MI_ERR;
//        status = PcdAnticoll(SN);/*防冲撞*/
//    }
//    if (status==MI_OK)//防衝撞成功
//    {
//        status=MI_ERR;
//        status =PcdSelect(SN);

//    }
//    if(status==MI_OK)//選卡成功
//    {
//        status=MI_ERR;
//        status =PcdAuthState(0x60,0x09,KEY,SN);
//    }
//    if(MODE)	// 判读是否写入
//    {
//        if(status==MI_OK)		//验证成功
//        {
//            status=MI_ERR;
//            status=PcdWrite(s,WRITE_RFID);	 //写入数据
////					GUI_Show_Str(32,260, RED,WHITE,WRITE_RFID,32,0);
//        }
//    }
//    status=PcdRead(s,READ_RFID);				//读卡
//    if(status==MI_OK)//讀卡成功
//    {
//					LED_1=1;
////        GUI_Show_Str(32,340, RED,WHITE,READ_RFID,32,0);
//        WaitCardOff( );
//        status=MI_ERR;
//        delay_ms(100);
//    }
//    GUI_Show_Str(32,260, RED,WHITE,"                ",32,0);
//		 GUI_Show_Str(32,340, RED,WHITE,"                ",32,0);

}




/**
 * @description: 地址计算
 * @param {uint8_t} S 扇区
 * @param {uint8_t} Block 块
 * @return {*}
 */
uint8_t RC_Get_address(uint8_t S, uint8_t Block)
{
    return S * 4 + Block-1;
}

/**
 * @description: 检测RFID卡
 * @param {*} 
 * @return {*}0.失败 1.成功
 */
uint8_t RC_check(void)
{
    if (PcdRequest(PICC_REQALL, CT) == MI_OK) //寻卡
    {
        return 1;
    }
    return 0;
}

/**
 * @description: 检测卡片并读取内容 
 * @param {uint8_t} Block_address
 * @param {uint8_t} *KEY
 * @return {*}1.读取成功 0.读取失败
 */
uint8_t RC_check_read(uint8_t Block_address, uint8_t *KEY)
{
    
    if (PcdRequest(PICC_REQALL, CT) == MI_OK) //寻卡
    {
        if (PcdAnticoll(SN) == MI_OK) // 取卡片序列号
        {
            if (PcdSelect(SN) == MI_OK) //选定此卡
            {
                if (PcdAuthState(0x60, (int)(Block_address / 4) * 4 + 3, KEY, SN) == MI_OK) //验证密钥
                {
                    if (PcdRead(Block_address, buffer_data) == MI_OK) //读卡
                    {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}


/**
 * @description: 读取RC_check_read()的数据缓存
 * @param {*}
 * @return {*}
 */
uint8_t *RC_Get_buffer()
{
    return buffer_data;
}



/**
 * @description: RFID写卡
 * @param {uint8_t} Block_address　块地址
 * @param {uint8_t} *KEY　密钥，无密钥则为""
 * @param {uint8_t} *data　写入数据值
 * @return {*}
 */
uint8_t RC_write(uint8_t Block_address, uint8_t *KEY, uint8_t *data)
{
    if (PcdRequest(PICC_REQALL, CT) == MI_OK) //寻卡
	{
        if (PcdAnticoll(SN) == MI_OK) // 防冲撞成功
        {
            if (PcdSelect(SN) == MI_OK) //选定此卡
            {

                if (PcdAuthState(0x60, Block_address / 4 * 4 + 3, KEY, SN) == MI_OK) //验证扇区0的密钥A
                {
                    if (PcdWrite(Block_address, data) == MI_OK) //读卡 块地址1
                    {
                        return 1;
                    }
                }
                
            }
        }
    }
    return 0;
}
