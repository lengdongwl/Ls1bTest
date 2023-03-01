/**
  ******************************************************************************
  * @file    rc522.c
  * @author  xie
  * @version V2
  * @date    2021��11��2��
  * @brief   ��չ�˿�ģ��RC552 RFID����������������
	
						�˿ڶ��壺
							PC12:	MY_IIC_SCL		MPU6050IIC���ݽ�	
							PD2:	MY_IIC_SDA		MPU6050IICʱ�ӽ�	

	******************************************************************************
**/

#include <string.h>
#include "rc522.h"

#include "bsp.h"
#ifdef BSP_USE_I2C0

#include "ls1x_i2c_bus.h"
#endif

#define  MODE         1                    	 // ģʽ  1����д���   0��ֻ��
#define  DATA_LEN    16                      // ���������ֽڳ���

/*ȫ�ֱ���*/
unsigned char CT[2];//������
unsigned char SN[4]; //����
unsigned char RFID[16];			//���RFID
uint8_t KEY[6]= {0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t KEY_A[6]= {0xff,0xff,0xff,0xff,0xff,0xff};

uint8_t WRITE_RFID[DATA_LEN+2]="AaBbCcDdEe123456";
uint8_t READ_RFID[DATA_LEN+2]= {0};
uint8_t buffer_data[99];                                  //RC_check_read���ݻ�����
void RC522_Init(void)
{
	
	InitRc522();		//RC522 оƬ��ʼ��
}

/**************************************************************
*��  �ܣ�RC522 оƬ��ʼ��
*��  ��: ��
*����ֵ: ��
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
*��  �ܣ�Ѱ��
*��  ��: req_code[IN]:Ѱ����ʽ
*		 0x52 = Ѱ��Ӧ�������з���14443A��׼�Ŀ�
*		 0x26 = Ѱδ��������״̬�Ŀ�
*		 pTagType[OUT]����Ƭ���ʹ���
*		 0x4400 = Mifare_UltraLight
*		 0x0400 = Mifare_One(S50)
*		 0x0200 = Mifare_One(S70)
*		 0x0800 = Mifare_Pro(X)
*		 0x4403 = Mifare_DESFire
*����ֵ: �ɹ�����MI_OK
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
*��  �ܣ�����ײ
*��  ��: pSnr[OUT]:��Ƭ���кţ�4�ֽ�
*����ֵ: �ɹ�����MI_OK
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
*��  �ܣ�ѡ����Ƭ
*��  ��: pSnr[IN]:��Ƭ���кţ�4�ֽ�
*����ֵ: �ɹ�����MI_OK
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
*��  �ܣ���֤��Ƭ����
*��  ��: auth_mode[IN]: ������֤ģʽ 0x60 = ��֤A��Կ 0x61 = ��֤B��Կ
*		 addr[IN]�����ַ   pKey[IN]������  pSnr[IN]����Ƭ���кţ�4�ֽ�
*����ֵ: �ɹ�����MI_OK
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
*��  �ܣ���ȡM1��һ������
*��  ��: addr[IN]�����ַ pData[OUT]�����������ݣ�16�ֽ�
*����ֵ: �ɹ�����MI_OK
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
*��  �ܣ�д���ݵ�M1��һ��
*��  ��: addr[IN]�����ַ  pData[IN]��д������ݣ�16�ֽ�
*����ֵ: �ɹ�����MI_OK
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
*��  �ܣ����Ƭ��������״̬
*��  ��: ��
*����ֵ: �ɹ�����MI_OK
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
*��  �ܣ���MF522����CRC16����
*��  ��: ��
*����ֵ: ��
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
*��  �ܣ���λRC522
*��  ��: ��
*����ֵ: �ɹ�����MI_OK
**************************************************************/
char PcdReset(void)
{

    delay_us(10);
    WriteRawRC(CommandReg,PCD_RESETPHASE);
    WriteRawRC(CommandReg,PCD_RESETPHASE);
    delay_us(10);

    WriteRawRC(ModeReg,0x3D);            //��Mifare��ͨѶ��CRC��ʼֵ0x6363
    WriteRawRC(TReloadRegL,30);
    WriteRawRC(TReloadRegH,0);
    WriteRawRC(TModeReg,0x8D);
    WriteRawRC(TPrescalerReg,0x3E);

    WriteRawRC(TxAutoReg,0x40);//����Ҫ

    return MI_OK;
}
//////////////////////////////////////////////////////////////////////
//����RC632�Ĺ�����ʽ
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
*��  �ܣ���RC522�Ĵ���λ
*��  ��: reg[IN]:�Ĵ�����ַ  mask[IN]:��λֵ
*����ֵ: ��
**************************************************************/
void SetBitMask(uint8_t   reg,uint8_t   mask)
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/**************************************************************
*��  �ܣ���RC522�Ĵ���λ
*��  ��: reg[IN]:�Ĵ�����ַ  mask[IN]:��λֵ
*����ֵ: ��
**************************************************************/
void ClearBitMask(uint8_t   reg,uint8_t   mask)
{
    char   tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
}

/**************************************************************
*��  �ܣ�ͨ��RC522��ISO14443��ͨѶ
*��  ��: Command[IN]:RC522������
*		 pInData[IN]:ͨ��RC522���͵���Ƭ������
*		 InLenByte[IN]:�������ݵ��ֽڳ���
*		 pOutData[OUT]:���յ��Ŀ�Ƭ��������
*		 *pOutLenBit[OUT]:�������ݵ�λ����
*����ֵ: ��
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
    ClearBitMask(ComIrqReg,0x80);	//�������ж�λ
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);	 	//��FIFO����

    for (i=0; i<InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pIn [i]);
    }
    WriteRawRC(CommandReg, Command);
//   	 n = ReadRawRC(CommandReg);

    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg,0x80);     //��ʼ����
    }

    //i = 600;//����ʱ��Ƶ�ʵ���������M1�����ȴ�ʱ��25ms
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
*��  �ܣ��������ߣ�ÿ��������ر����շ���֮��Ӧ������1ms�ļ����
*��  ��: ��
*����ֵ: ��
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
*��  �ܣ��ر�����
*��  ��: ��
*����ֵ: ��
**************************************************************/
void PcdAntennaOff(void)
{
    ClearBitMask(TxControlReg, 0x03);
}

/////////////////////////////////////////////////////////////////////
//��    �ܣ���RC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//��    �أ�������ֵ
/////////////////////////////////////////////////////////////////////
uint8_t ReadRawRC(uint8_t   Address)
{
    uint8_t ucResult=0;
    ucResult = RC522_RD_Reg(SLA_ADDR,  Address);
    return ucResult;          		//�����յ�������

}

/////////////////////////////////////////////////////////////////////
//��    �ܣ�дRC632�Ĵ���
//����˵����Address[IN]:�Ĵ�����ַ
//          value[IN]:д���ֵ
/////////////////////////////////////////////////////////////////////
void WriteRawRC(uint8_t   Address, uint8_t   value)
{
    RC522_WR_Reg(SLA_ADDR,  Address, value);
}



/**************************************************************
*��  �ܣ����Ĵ���
*��  ��: addr:�Ĵ�����ַ
*����ֵ: ������ֵ
**************************************************************/
uint8_t RC522_RD_Reg(uint8_t RCsla,uint8_t addr)
{
    uint8_t temp=0;

    #ifdef BSP_USE_I2C0
    uint8_t BUF[1];

    ls1x_i2c_send_start(busI2C0, RCsla);//��ʼ�ź�
    if(ls1x_i2c_send_addr(busI2C0,addr,1))//�����豸��ַ�Ӷ��ź�
    {
        return;
    }
 
    ls1x_i2c_read_bytes(busI2C0, BUF, 1);
    ls1x_i2c_send_stop(busI2C0,RCsla);//����ֹͣ�ź�
    

    #else

    MY_IIC_Start();
    MY_IIC_Send_Byte(RCsla);	//����д����ָ��
    temp=MY_IIC_Wait_Ack();

    MY_IIC_Send_Byte(addr);   		//���ͼĴ�����ַ
    temp=MY_IIC_Wait_Ack();

    MY_IIC_Start();  	 	   		//��������
    MY_IIC_Send_Byte(RCsla+1);	//���Ͷ�����ָ��
    temp=MY_IIC_Wait_Ack();

    temp=MY_IIC_Read_Byte(0);		//��ȡһ���ֽ�,�������ٶ�,����NAK
    MY_IIC_Stop();					//����һ��ֹͣ����

    #endif

    printk("buf=%d\n",BUF[0]);
    return BUF[0];				//���ض�����ֵ
    
    
}


//д�Ĵ���
//addr:�Ĵ�����ַ
//val:Ҫд���ֵ
//����ֵ:��
void RC522_WR_Reg(uint8_t RCsla,uint8_t addr,uint8_t val)
{

    #ifdef BSP_USE_I2C0
    uint8_t rt ;
    
    uint8_t BUF[2];
    rt=ls1x_i2c_send_start(busI2C0, RCsla);//��ʼ�ź�
    rt=ls1x_i2c_send_addr(busI2C0,RCsla,0);	//�����豸��ַ+д�ź�
    //CHECK_DONE(rt);
    BUF[0] = addr;
    BUF[1] = val;
    ls1x_i2c_write_bytes(busI2C0, BUF, 1);	//����ֵ
    ls1x_i2c_send_stop(busI2C0,RCsla);	//����ֹͣ�ź�


    #else
    MY_IIC_Start();
    MY_IIC_Send_Byte(RCsla);     	//����д����ָ��
    MY_IIC_Wait_Ack();
    MY_IIC_Send_Byte(addr);   			//���ͼĴ�����ַ
    MY_IIC_Wait_Ack();
    MY_IIC_Send_Byte(val);     		//����ֵ
    MY_IIC_Wait_Ack();
    MY_IIC_Stop();						//����һ��ֹͣ����
    
    #endif
}


//�ȴ����뿪
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
uint8_t	KEY_Data[6]={0xff,0xff,0xff,0xff,0xff,0xff}; 		//��Կ
void Read_Card(void)
{
		if(PcdRequest(PICC_REQALL,CT) == MI_OK)							//Ѱ��
		{
			if(PcdAnticoll(SN) == MI_OK)											// ����ײ�ɹ�
			{
				if(PcdSelect(SN)==MI_OK)												//ѡ���˿�
				{	
					if(PcdAuthState(0x60,1/4*4+3,KEY_Data,SN)==MI_OK)			//��֤��Կ
					{
						if(PcdRead(1,RX_RFID)==MI_OK)								//����
						{	
							
							
						}
					}
				}
			}
		}
//    uint8_t status;
//    uint8_t s=0x08;
//    status = PcdRequest(PICC_REQALL,CT);/*����*/
//    if(status==MI_OK)//�����ɹ�
//    {
//        delay_ms(200);
//        status=MI_ERR;
//        status = PcdAnticoll(SN);/*����ײ*/
//    }
//    if (status==MI_OK)//���nײ�ɹ�
//    {
//        status=MI_ERR;
//        status =PcdSelect(SN);

//    }
//    if(status==MI_OK)//�x���ɹ�
//    {
//        status=MI_ERR;
//        status =PcdAuthState(0x60,0x09,KEY,SN);
//    }
//    if(MODE)	// �ж��Ƿ�д��
//    {
//        if(status==MI_OK)		//��֤�ɹ�
//        {
//            status=MI_ERR;
//            status=PcdWrite(s,WRITE_RFID);	 //д������
////					GUI_Show_Str(32,260, RED,WHITE,WRITE_RFID,32,0);
//        }
//    }
//    status=PcdRead(s,READ_RFID);				//����
//    if(status==MI_OK)//�x���ɹ�
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
 * @description: ��ַ����
 * @param {uint8_t} S ����
 * @param {uint8_t} Block ��
 * @return {*}
 */
uint8_t RC_Get_address(uint8_t S, uint8_t Block)
{
    return S * 4 + Block-1;
}

/**
 * @description: ���RFID��
 * @param {*} 
 * @return {*}0.ʧ�� 1.�ɹ�
 */
uint8_t RC_check(void)
{
    if (PcdRequest(PICC_REQALL, CT) == MI_OK) //Ѱ��
    {
        return 1;
    }
    return 0;
}

/**
 * @description: ��⿨Ƭ����ȡ���� 
 * @param {uint8_t} Block_address
 * @param {uint8_t} *KEY
 * @return {*}1.��ȡ�ɹ� 0.��ȡʧ��
 */
uint8_t RC_check_read(uint8_t Block_address, uint8_t *KEY)
{
    
    if (PcdRequest(PICC_REQALL, CT) == MI_OK) //Ѱ��
    {
        if (PcdAnticoll(SN) == MI_OK) // ȡ��Ƭ���к�
        {
            if (PcdSelect(SN) == MI_OK) //ѡ���˿�
            {
                if (PcdAuthState(0x60, (int)(Block_address / 4) * 4 + 3, KEY, SN) == MI_OK) //��֤��Կ
                {
                    if (PcdRead(Block_address, buffer_data) == MI_OK) //����
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
 * @description: ��ȡRC_check_read()�����ݻ���
 * @param {*}
 * @return {*}
 */
uint8_t *RC_Get_buffer()
{
    return buffer_data;
}



/**
 * @description: RFIDд��
 * @param {uint8_t} Block_address�����ַ
 * @param {uint8_t} *KEY����Կ������Կ��Ϊ""
 * @param {uint8_t} *data��д������ֵ
 * @return {*}
 */
uint8_t RC_write(uint8_t Block_address, uint8_t *KEY, uint8_t *data)
{
    if (PcdRequest(PICC_REQALL, CT) == MI_OK) //Ѱ��
	{
        if (PcdAnticoll(SN) == MI_OK) // ����ײ�ɹ�
        {
            if (PcdSelect(SN) == MI_OK) //ѡ���˿�
            {

                if (PcdAuthState(0x60, Block_address / 4 * 4 + 3, KEY, SN) == MI_OK) //��֤����0����ԿA
                {
                    if (PcdWrite(Block_address, data) == MI_OK) //���� ���ַ1
                    {
                        return 1;
                    }
                }
                
            }
        }
    }
    return 0;
}
