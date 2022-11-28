/*
 * gt1151q.c
 *
 * created: 2021/5/19
 *  author: 
 */
 #include "gt1151q.h"
 #include "ls1b_gpio.h"
 #include "ls1x_i2c_bus.h"
 #include "ls1x_fb.h"
 
//gpio_read(GT_INT)  //GT1151中断引脚

/************************************************************************
** 功能：  向GT1151写入一次数据
** 参数：
           @buf:数据缓缓存区
           @len:写数据长度
** 返回值：0,成功;-1,失败.
*************************************************************************/
int GT1151_WR_Reg(int reg,unsigned char *buf, int len)
{
	int ret=0;
	unsigned char reg_buf[2];

    //分解出寄存器的高低地址
    reg_buf[0] = reg >> 8;
    reg_buf[1] = reg;
	
	//起始信号
	ret = ls1x_i2c_send_start(busI2C0,NULL);
    if(ret < 0)
    {
        printf("send_start error!!!\r\n");
        return -1;
    }
    
 	//发送从机地址和写命令
 	ret = ls1x_i2c_send_addr(busI2C0, GT_CMD_WR, 0);
 	if(ret < 0)
    {
        printf("send_addr error!!!\r\n");
        return -1;
    }
    
    //发送寄存器地址
	ret = ls1x_i2c_write_bytes(busI2C0, reg_buf, 2);
	if(ret < 0)
    {
        printf("write_bytes_reg error!!!\r\n");
        return -1;
    }
    
    //发送数据
	ret = ls1x_i2c_write_bytes(busI2C0, buf, len);
	if(ret < 0)
    {
        printf("write_bytes error!!!\r\n");
        return -1;
    }
    
    //发送停止信号
    ret = ls1x_i2c_send_stop(busI2C0,NULL);
    if(ret < 0)
    {
        printf("send_stop error!!!\r\n");
        return -1;
    }
    
	return ret;
}

/************************************************************************
** 功能：  从GT1151读出一次数据
** 参数：
           @reg:寄存器的地址
           @buf:数据缓缓存区
           @len:写数据长度
** 返回值：0,成功;-1,失败.
*************************************************************************/
int GT1151_RD_Reg(int reg,unsigned char *buf,int len)
{
	int ret=0;
    unsigned char reg_buf[2];
    
    //分解出寄存器的高低地址
    reg_buf[0] = reg >> 8;
    reg_buf[1] = reg;
	
 	//起始信号
	ret = ls1x_i2c_send_start(busI2C0,NULL);
    if(ret < 0)
    {
        printf("send_start error!!!\r\n");
        return -1;
    }

 	//发送从机地址和写命令
 	ret = ls1x_i2c_send_addr(busI2C0, GT_CMD_WR, 0);
 	if(ret < 0)
    {
        printf("send_addr_W error!!!\r\n");
        return -1;
    }

    //发送寄存器地址
	ret = ls1x_i2c_write_bytes(busI2C0, reg_buf, 2);
	if(ret < 0)
    {
        printf("write_bytes_reg error!!!\r\n");
        return -1;
    }
    
    //发送停止信号
    ret = ls1x_i2c_send_stop(busI2C0,NULL);
    if(ret < 0)
    {
        printf("send_stop error!!!\r\n");
        return -1;
    }
    
    //起始信号
	ret = ls1x_i2c_send_start(busI2C0,NULL);
    if(ret < 0)
    {
        printf("send_start error!!!\r\n");
        return -1;
    }
    
    //发送从机地址和读命令
 	ret = ls1x_i2c_send_addr(busI2C0, GT_CMD_WR, 1);
 	if(ret < 0)
    {
        printf("send_addr_R error!!!\r\n");
        return -1;
    }
    
    //读取数据
    ls1x_i2c_read_bytes(busI2C0,buf,len);
    if(ret < 0)
    {
        printf("read_bytes_Data error!!!\r\n");
        return -1;
    }

    //发送停止信号
    ret = ls1x_i2c_send_stop(busI2C0,NULL);
    if(ret < 0)
    {
        printf("send_stop error!!!\r\n");
        return -1;
    }
    
    return 0;
}
_m_tp_dev tp_dev = {0,0,0,0,0,0,0,0,};

//画笔的颜色
const unsigned short POINT_COLOR_TBL[5]={cidxRED,cidxGREEN,cidxBLUE,cidxYELLOW,cidxWHITE};

/************************************************************************
** 功能：  测试触摸屏(采用查询方式)
** 参数：  无
** 返回值：无
*************************************************************************/
void GT1151_Test(void)
{
	unsigned char t=0;
	unsigned char i=0;
 	unsigned short lastpos[5][2];		//最后一次的数据
	while(1)
	{
		GT1151_Scan(0);
		for(t = 0; t < 5; t++)
		{
			if((tp_dev.sta) & (1 << t))
			{
				if(tp_dev.x[t] < 800 && tp_dev.y[t] < 480)
				{
					if(lastpos[t][0] == 0XFFFF)
					{
						lastpos[t][0] = tp_dev.x[t];
						lastpos[t][1] = tp_dev.y[t];
					}

                    fb_drawline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],POINT_COLOR_TBL[t]);//画线
					lastpos[t][0] = tp_dev.x[t];
					lastpos[t][1] = tp_dev.y[t];
					if(tp_dev.x[t] > (400-24) && tp_dev.y[t] < 20)
					{
						fb_cons_clear();//清除
					}
				}
			}
            else
            {
                lastpos[t][0]=0XFFFF;
            }
		}
		delay_ms(5);
	}
}



//合力泰
#if 1
const unsigned char GT1151_CFG_TBL[]=
{
	0x44,0x20,0x03,0xE0,0x01,0x05,0x35,0x04,0x00,0x08,
	0x09,0x0F,0x55,0x37,0x33,0x11,0x00,0x03,0x08,0x56,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x48,0x00,0x00,
	0x3C,0x08,0x0A,0x28,0x1E,0x50,0x00,0x00,0x82,0xB4,
	0xD2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x85,0x25,0x11,0x41,0x43,0x31,
	0x0D,0x00,0xAD,0x22,0x24,0x7D,0x1D,0x1D,0x32,0xDF,
	0x4F,0x44,0x0F,0x80,0x2C,0x50,0x50,0x00,0x00,0x00,
	0x00,0xD3,0x00,0x00,0x00,0x00,0x0F,0x28,0x1E,0xFF,
	0xF0,0x37,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x50,0xB4,0xC0,0x94,0x53,0x2D,
	0x0A,0x02,0xBE,0x60,0xA2,0x71,0x8F,0x82,0x80,0x92,
	0x74,0xA3,0x6B,0x01,0x0F,0x14,0x03,0x1E,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x0C,0x0D,0x0E,0x0F,0x10,
	0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,
	0x1D,0x1F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x19,0x18,0x17,
	0x15,0x14,0x13,0x12,0x0C,0x08,0x06,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	0xC4,0x09,0x23,0x23,0x50,0x5D,0x54,0x4B,0x3C,0x0F,
	0x32,0xFF,0xE4,0x04,0x40,0x00,0x8A,0x05,0x40,0x00,
	0xAA,0x00,0x22,0x22,0x00,0x00,0xAA,0x05,0x01,
};
#endif



//北泰
#if 0
const unsigned char GT1151_CFG_TBL[]=
{
	0x46,0x20,0x03,0xE0,0x01,0x05,0x35,0x14,0x00,0x00,
	0x02,0x0C,0x5F,0x4B,0x35,0x01,0x00,0x06,0x06,0x1E,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0x00,0x00,
	0x64,0x08,0x32,0x28,0x28,0x64,0x00,0x00,0x87,0xA0,
	0xCD,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x8B,0x00,0x13,0x74,0x76,0xEB,
	0x06,0x00,0x1D,0x52,0x24,0xFA,0x1E,0x18,0x32,0xAA,
	0x63,0x99,0x6E,0x82,0x79,0x80,0x85,0x00,0x00,0x00,
	0x00,0x64,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x58,0x9C,0xC0,0x94,0x85,0x28,
	0x0A,0x03,0xAA,0x63,0x99,0x6E,0x8A,0x79,0x80,0x85,
	0x77,0x90,0x71,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x0A,0x0B,0x0C,0x0D,0x0E,
	0x0F,0x10,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,
	0x1A,0x1B,0x1D,0x1F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x1A,0x19,0x18,
	0x17,0x15,0x14,0x13,0x12,0x0C,0x08,0x06,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,
	0xC4,0x09,0x23,0x23,0x50,0x5D,0x54,0x50,0x3C,0x14,
	0x32,0xFF,0xFF,0x06,0x51,0x00,0x8A,0x02,0x40,0x00,
	0xAA,0x00,0x22,0x22,0x00,0x40,0x80,0x61,0x01
};
#endif

void check_sum(void)
{
	unsigned short checksum=0;
	unsigned char checksumH,checksumL;
	unsigned char i=0;
	for(i=0;i<(sizeof(GT1151_CFG_TBL)-3);i+=2)
	checksum +=((GT1151_CFG_TBL[i]<<8)|GT1151_CFG_TBL[i+1]);//计算校验和
	//checksum +=(GT1151_CFG_TBL[i]<<8)+GT1151_CFG_TBL[i+1];
	//checksum =0-checksum;
	checksum =(~checksum)+1;
	checksumH=checksum>>8;
	checksumL=checksum;
	printf("chksum:0x%X,\r\n",checksum);
	printf("chksumH:0x%X,\r\n",checksumH);
	printf("chksumL:0x%X,\r\n",checksumL);
}



//发送GT1151配置参数
//mode:0,参数不保存到flash
//     1,参数保存到flash
unsigned char GT1151_Send_Cfg(unsigned char mode)
{
	unsigned short checksum=0;
	unsigned char buf[3];
	unsigned char i=0;
	for(i=0;i<(sizeof(GT1151_CFG_TBL)-3);i+=2)
	checksum +=((GT1151_CFG_TBL[i]<<8)|GT1151_CFG_TBL[i+1]);//计算校验和
	//checksum +=(GT1151_CFG_TBL[i]<<8)+GT1151_CFG_TBL[i+1];
	//checksum =0-checksum;
	checksum =(~checksum)+1;
	//printf("chksum:0x%X,\r\n",checksum);
	buf[0]= checksum>>8;
	buf[1]= checksum;
	buf[2]= mode;	//是否写入到GT5688 FLASH?  即是否掉电保存
	GT1151_WR_Reg(GT_CFGS_REG,(unsigned char*)GT1151_CFG_TBL,sizeof(GT1151_CFG_TBL)-3);//发送寄存器配置
	GT1151_WR_Reg(GT_CHECK_REG,buf,3);
	return 0;
}

//初始化GT1151触摸屏
//返回值:0,初始化成功;1,初始化失败
unsigned char Cfg_Info[239] = {0};
unsigned char GT1151_Init(void)
{
	unsigned char temp[5];
	unsigned char i=0;
	unsigned char buf[2];
	buf[0]=0;


	//使能GPIO口
    gpio_enable(GT_RST,DIR_OUT);
    gpio_enable(GT_INT,DIR_IN);


	ls1x_i2c_initialize(busI2C0);      //初始化电容屏的I2C总线
	ls1x_i2c_ioctl(busI2C0, IOCTL_SPI_I2C_SET_TFRMODE, (void *)GT1151Q_BAUDRATE);

	gpio_write(GT_RST, 0);              //GT1151复位
	delay_ms(10);
	gpio_write(GT_RST, 1);              //GT1151释放复位
	delay_ms(100);
	

	delay_ms(200);
	//temp[4]=0;
	GT1151_RD_Reg(GT_FW_REG,temp,2);//读取IC FW版本
	printf("IC FW:0x%X\r\n",(temp[0]<<8|temp[1]));
	GT1151_RD_Reg(GT_CFGS_REG,temp,1);//读取GT_CFGS_REG寄存器
	printf("Default Ver:0x%x\r\n",temp[0]);
	GT1151_RD_Reg(GT_PID_REG,temp,4);//读取产品ID
	printf("CTP ID:GT%s\r\n",temp);	//打印ID
	check_sum();
	//printf("Default Ver:0x%x\r\n",temp[0]);
	GT1151_Send_Cfg(1);
	if(strcmp((char*)temp,"1158")==0)//ID==1151
	{

		//GT1151_Send_Cfg(1);

		#if 1
		GT1151_RD_Reg(0x8050,Cfg_Info,239);
		printf("Config Info:\r\n");
		for( i = 0; i < 239; i++ )
		{
		printf("0x%02X,",Cfg_Info[i]);
		if((i+1)%10==0)
		printf("\r\n");
		}
		printf("\r\n");
		#endif
		return 0;
	}
	return 1;
}

const unsigned short GT1151_TPX_TBL[5]={GT_TP1_REG,GT_TP2_REG,GT_TP3_REG,GT_TP4_REG,GT_TP5_REG};
//扫描触摸屏(采用查询方式)
//mode:0,正常扫描.
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
unsigned char GT1151_Scan(unsigned char mode)
{
	unsigned char buf[4];
	unsigned char i=0;
	unsigned char res=0;
	unsigned char temp;
	unsigned char tempsta;
 	static unsigned char t=0;//控制查询间隔,从而降低CPU占用率
	t++;
	if((t%10)==0||t<10)//空闲时,每进入10次CTP_Scan函数才检测1次,从而节省CPU使用率
	{
		GT1151_RD_Reg(GT_GSTID_REG,&mode,1);	//读取触摸点的状态
 		if(mode&0X80&&((mode&0XF)<6))
		{
			temp=0;
			GT1151_WR_Reg(GT_GSTID_REG,&temp,1);//清标志
		}
		if((mode&0XF)&&((mode&0XF)<6))
		{
			temp=0XFF<<(mode&0XF);		//将点的个数转换为1的位数,匹配tp_dev.sta定义
			tempsta=tp_dev.sta;			//保存当前的tp_dev.sta值
			tp_dev.sta=(~temp)|TP_PRES_DOWN|TP_CATH_PRES;
			tp_dev.x[4]=tp_dev.x[0];	//保存触点0的数据
			tp_dev.y[4]=tp_dev.y[0];
			for(i=0;i<5;i++)
			{
				if(tp_dev.sta&(1<<i))	//触摸有效?
				{
					GT1151_RD_Reg(GT1151_TPX_TBL[i],buf,4);	//读取XY坐标值
					if(1)//横屏
					{
						tp_dev.x[i]=(((unsigned short)buf[1]<<8)+buf[0]);
						tp_dev.y[i]=(((unsigned short)buf[3]<<8)+buf[2]);

						//tp_dev.x[i]=480-(((unsigned short)buf[3]<<8)+buf[2]);
						//tp_dev.y[i]=272-(((unsigned short)buf[1]<<8)+buf[0]);


					}else
					{

						tp_dev.x[i]=(((unsigned short)buf[1]<<8)+buf[0]);
						tp_dev.y[i]=(((unsigned short)buf[3]<<8)+buf[2]);
					}
					//printf("x[%d]:%d,y[%d]:%d\r\n",i,tp_dev.x[i],i,tp_dev.y[i]);
				}
			}
			res=1;
			if(tp_dev.x[0]>800||tp_dev.y[0]>480)//非法数据(坐标超出了)
			{
				if((mode&0XF)>1)		//有其他点有数据,则复第二个触点的数据到第一个触点.
				{
					tp_dev.x[0]=tp_dev.x[1];
					tp_dev.y[0]=tp_dev.y[1];
					t=0;				//触发一次,则会最少连续监测10次,从而提高命中率
				}else					//非法数据,则忽略此次数据(还原原来的)
				{
					tp_dev.x[0]=tp_dev.x[4];
					tp_dev.y[0]=tp_dev.y[4];
					mode=0X80;
					tp_dev.sta=tempsta;	//恢复tp_dev.sta
				}
			}else t=0;					//触发一次,则会最少连续监测10次,从而提高命中率
		}
	}
	if((mode&0X8F)==0X80)//无触摸点按下
	{
		if(tp_dev.sta&TP_PRES_DOWN)	//之前是被按下的
		{
			tp_dev.sta&=~(1<<7);	//标记按键松开
		}else						//之前就没有被按下
		{
			tp_dev.x[0]=0xffff;
			tp_dev.y[0]=0xffff;
			tp_dev.sta&=0XE0;	//清除点有效标记
		}
	}
	if(t>240)t=10;//重新从10开始计数
	return res;
}



