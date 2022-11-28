/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 *  LS1X AC97 Driver
 *
 *  Author: Bian, 2021/3/20
 */

#include "bsp.h"

#ifdef BSP_USE_AC97

#include <stdbool.h>
#include <string.h>

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_irq.h"
#define LS1x_AC97_BASE  LS1B_AC97_BASE
#define LS1x_AC97_IRQ 	LS1B_AC97_IRQ
#define LS1x_DMA1_IRQ 	LS1B_DMA1_IRQ
#define LS1x_DMA2_IRQ 	LS1B_DMA2_IRQ
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_irq.h"
#define LS1x_AC97_BASE  LS1C_AC97_BASE
#define LS1x_AC97_IRQ 	LS1C_AC97_IRQ
#define LS1x_DMA1_IRQ 	LS1C_DMA1_IRQ
#define LS1x_DMA2_IRQ 	LS1C_DMA2_IRQ
#else
#error "No Loongson1x SoC defined."
#endif

#if defined(OS_RTTHREAD)
#include "rtthread.h"
#define SLEEP   rt_thread_delay
#elif defined(OS_UCOS)
#include "os.h"
#define SLEEP   OSTimeDly
#elif defined(OS_FREERTOS)
#include "FreeRTOS.h"
#include "event_groups.h"
#define SLEEP   vTaskDelay
#endif

#include "cpu.h"

#include "ls1x_io.h"
#include "ls1x_ac97_hw.h"
#include "ls1x_ac97.h"
#include "ls1x_dma_hw.h"

#include "codec_alc655.h"

#if (BSP_USE_LWMEM)
#include "../../libc/lwmem.h"
#endif

#include "drv_os_priority.h"

//-------------------------------------------------------------------------------------------------
// MACROS
//-------------------------------------------------------------------------------------------------

#define DMA_DESC_COUNT      2           /* if 1 then dynamic malloc buffer for wavefile.
                                           NOW  we only use 2 descriptors alternately.
                                           TODO we can use more then 2 descriptors. */

#define DMA_BUF_SIZE        0x010000    /* Total = 64K * DMA_DESC_COUNT */

#define DMA_BUF_CACHED      1           /* K0 address of buffer */

/*
 * DMA0/DMA1 Interrupt Control
 */
#define DMA_INT_INIT(tag) \
    do { \
        LS1x_INTC_EDGE(LS1x_INTC0_BASE) |=  INTC0_DMA##tag##_BIT; \
	    LS1x_INTC_POL( LS1x_INTC0_BASE) &= ~INTC0_DMA##tag##_BIT; \
	    LS1x_INTC_CLR( LS1x_INTC0_BASE)  =  INTC0_DMA##tag##_BIT; \
	    LS1x_INTC_IEN( LS1x_INTC0_BASE) &= ~INTC0_DMA##tag##_BIT; \
    } while (0)

#define DMA_INT_ENABLE(tag) \
    do { \
	    LS1x_INTC_CLR(LS1x_INTC0_BASE)  = INTC0_DMA##tag##_BIT; \
	    LS1x_INTC_IEN(LS1x_INTC0_BASE) |= INTC0_DMA##tag##_BIT; \
    } while (0)

#define DMA_INT_DISABLE(tag) \
    do { \
	    LS1x_INTC_CLR(LS1x_INTC0_BASE)  =  INTC0_DMA##tag##_BIT; \
	    LS1x_INTC_IEN(LS1x_INTC0_BASE) &= ~INTC0_DMA##tag##_BIT; \
    } while (0)

/*
 * State
 */
#define AC97_IDLE               0x00        /* 空闲 */
#define AC97_PLAY_READY         0x01        /* 播放准备好 */
#define AC97_PLAYING            0x02        /* 正在播放 */
#define AC97_RECORD_READY       0x04        /* 录音准备好 */
#define AC97_RECORDING          0x08        /* 正在录音 */
#define AC97_PAUSE              0x10        /* 暂停 */
#define AC97_FORCE_STOP         0x20        /* 强制停止 */

/*
 * ERROR
 */
#ifndef DMA_DESC_COUNT
#error "AC97 DMA descriptor count is not defined!"
#elif (!((DMA_DESC_COUNT == 1) || (DMA_DESC_COUNT == 2)))
#error "AC97 DMA descriptor count is defined error!"
#endif

//-------------------------------------------------------------------------------------------------
// Device
//-------------------------------------------------------------------------------------------------

typedef struct AC97_dev
{
	/* Hardware shortcuts */
	LS1x_AC97_regs_t *hwAC97;		            /* AC97 寄存器 */
	unsigned int      dmaCtrl;			        /* DMA  控制寄存器 */

	LS1x_dma_desc_t  *rx_desc[DMA_DESC_COUNT];  /* DMA 接收描述符 */
	LS1x_dma_desc_t  *tx_desc[DMA_DESC_COUNT];  /* DMA 发送描述符 */

	unsigned char    *data_buf[DMA_DESC_COUNT]; /* DMA 数据缓冲区, rx or tx */

	int          intialized;				    /* initialize flag */
	int          hw_reseted;                    /* reset flag */

    /*
     * Current working state
     */
    volatile int state;                         /* working state */
    int          num_channels;                  /* 1=mono, 2=stereo */
    int          sample_bytes;                  /* 1=8 bits, 2=16 bits */

#if (DMA_DESC_COUNT > 1)
    void        *user_buffer;                   /* 用户的无格式声音内存指针 */
    volatile int limited_bytes;                 /* 录音或者播放的总长度 */
    volatile int prepare_bytes;                 /* 准备的buffer字节数 */
    volatile int done_bytes;                    /* 已录音或者播放的字节数 */
    volatile int desc_index;                    /* 当前使用的 desc 序号 */
#endif

} LS1x_AC97_dev_t;

static LS1x_AC97_dev_t ls1x_ac97 = { .intialized = 0, };

static LS1x_AC97_dev_t *pAC97 = &ls1x_ac97;

#if defined(OS_RTTHREAD)
static rt_event_t         ac97_event = NULL;    /* AC97 DMA Interrupt send this event */
static rt_thread_t        ac97_thread = NULL;   /* This thread respone dma_evevt to goon */
#elif defined(OS_UCOS)
static OS_FLAG_GRP       *ac97_event = NULL;
static OS_STK ac97_stack[AC97_STK_SIZE];
#elif defined(OS_FREERTOS)
static EventGroupHandle_t ac97_event = NULL;
static TaskHandle_t       ac97_handle = NULL;
#endif

//-------------------------------------------------------------------------------------------------
// VARIABLES
//-------------------------------------------------------------------------------------------------

/*
 * DMA ORDER_ADDR_IN Register's addr-ask, must align 64
 */
static char rx_dma_desc_addr[64*DMA_DESC_COUNT] __attribute__((aligned(64)));
static char tx_dma_desc_addr[64*DMA_DESC_COUNT] __attribute__((aligned(64)));

/*
 * DMA transfer 4*4 bytes once, align 32
 */
#if (DMA_DESC_COUNT > 1)
static char rw_dma_buf_addr[DMA_BUF_SIZE*DMA_DESC_COUNT] __attribute__((aligned(32)));
#define SWAP_DESC_INDEX(n)  ((n == 1) ? 0 : 1)
#endif

//-------------------------------------------------------------------------------------------------
// Forward functions
//-------------------------------------------------------------------------------------------------

static int AC97_prepare_play(const ac97_param_t *pa);
static int AC97_prepare_record(const ac97_param_t *pa);

static int AC97_start(void);
static int AC97_stop(void);

static int AC97_stop_play(void);
static int AC97_stop_record(void);

#if BSP_USE_OS
static void AC97_DMA1_tx_event_fire(bool from_isr);
static void AC97_DMA2_rx_event_fire(bool from_isr);
static void AC97_DMA12_irq_response_task(void *arg);
#endif

//-------------------------------------------------------------------------------------------------
// ac97 functions
//-------------------------------------------------------------------------------------------------

/*
 * Read codec(ALC655) register
 */
static unsigned short AC97_codec_reg_read(unsigned short regNum)
{
	int time_out = 2000;
	unsigned int val = AC97_CODEC_WR;

	val |= ((unsigned int)regNum << AC97_CODEC_ADR_SHIFT);
	pAC97->hwAC97->crac = val;

	/* now wait for the data */
	while (time_out-- > 0)
    {
		if ((pAC97->hwAC97->intraw & AC97_INT_CR_DONE) != 0)
			break;
		delay_us(1);
	}

    if (time_out > 0)
    {
        val = pAC97->hwAC97->rd_intclr;     // clear read int flag
        return pAC97->hwAC97->crac & AC97_CODEC_DAT_MASK;
	}

    printk("AC97 command read timeout\n");
	return 0;
}

/*
 * Write codec(ALC655) register
 */
static int AC97_codec_reg_write(unsigned short regNum, unsigned short regVal)
{
	int time_out = 2000;
	unsigned int val = 0;

	val |= ((unsigned int)regNum << AC97_CODEC_ADR_SHIFT) | regVal;
	pAC97->hwAC97->crac = val;

	while (time_out-- > 0)
    {
		if ((pAC97->hwAC97->intraw & AC97_INT_CW_DONE) != 0)
			break;
		delay_us(1);
	}

    if (time_out > 0)
    {
        val = pAC97->hwAC97->wr_intclr;     // clear write int flag
        return 0;
	}

    printk("AC97 command write timeout\n");
	return -1;
}

static void AC97_codec_power_switch(int poweron)
{
    if (poweron)
        AC97_codec_reg_write(CODEC_POWERDOWN, 0x0000);  // POWER-ON
    else
        AC97_codec_reg_write(CODEC_POWERDOWN, 0xFFFF);  // POWER-OFF
}

/*
 * Initialize codec(ALC655) chip
 */
static int AC97_codec_chip_init(void)
{
	unsigned int ch_val;

	/*
     * 输出通道配置寄存器
     */
	ch_val  = pAC97->hwAC97->occ0;
	ch_val &= ~(CHNL_FIFO_THRES_MASK | CHNL_SAMP_WIDTH_MASK);
	ch_val |= 3 << CHNL_FIFO_THRES_SHIFT;                               /* FIFO门限 */
    if (pAC97->sample_bytes == 2)
	    ch_val |= CHNL_SAMP_WIDTH_16;                                   /* 16位采样 */
    ch_val &= ~CHNL_VAR_SAMP_RATE;                                      /* 采样率固定(48KHz) */
	ch_val |= CHNL_DMA_EN | CHNL_EN;                                    /* 打开 DMA & channel */
	ch_val |= ((ch_val << AC97_OCH1_R_SHIFT) & AC97_OCH1_R_MASK) |      /* 右声道 */
              ((ch_val << AC97_OCH0_L_SHIFT) & AC97_OCH0_L_MASK);       /* 左声道 */
    pAC97->hwAC97->occ0 = ch_val;

	/*
     * 输入通道配置寄存器
     */
	ch_val  = pAC97->hwAC97->icc;
	ch_val &= ~(CHNL_FIFO_THRES_MASK | CHNL_SAMP_WIDTH_MASK);
	ch_val |= 3 << CHNL_FIFO_THRES_SHIFT;                               /* FIFO门限 */
    if (pAC97->sample_bytes == 2)
	    ch_val |= CHNL_SAMP_WIDTH_16;                                   /* 16位采样 */
    ch_val &= ~CHNL_VAR_SAMP_RATE;                                      /* 采样率固定(48KHz) */
	ch_val |= CHNL_DMA_EN | CHNL_EN;                                    /* 打开 DMA & channel */
    ch_val |= ((ch_val << AC97_ICH2_MIC_SHIFT) & AC97_ICH2_MIC_MASK) |  /* MIC输入 */
              ((ch_val << AC97_OCH1_R_SHIFT)   & AC97_OCH1_R_MASK) |    /* 右声道 */
              ((ch_val << AC97_OCH0_L_SHIFT)   & AC97_OCH0_L_MASK);     /* 左声道 */
    pAC97->hwAC97->icc = ch_val;

    AC97_codec_reg_write(CODEC_MULTI_CHNL_CTRL, 0x0201);   // bit[9] & bit[0]?

	AC97_codec_power_switch(1);                            // power on

	/*
     * 设置音量
     */
	AC97_codec_reg_write(CODEC_MASTER_VOL,     0x0808);
	AC97_codec_reg_write(CODEC_MONOOUT_VOL,    0x0008);
	AC97_codec_reg_write(CODEC_PHONE_VOL,      0x0008);
	AC97_codec_reg_write(CODEC_MIC_VOL,        0x035f);
	AC97_codec_reg_write(CODEC_LINEIN_VOL,     0x0808);
	AC97_codec_reg_write(CODEC_PCMOUT_VOL,     0x0808);
#if 0
	AC97_codec_reg_write(CODEC_PCBEEP_VOL,     0x0808);
	AC97_codec_reg_write(CODEC_CD_VOL,         0x0808);
	AC97_codec_reg_write(CODEC_AUX_VOL,        0x0808);
	AC97_codec_reg_write(CODEC_CENTER_LFE_VOL, 0x0808);
	AC97_codec_reg_write(CODEC_SURROUND_VOL,   0x0808);
#endif

	/*
     * record设置录音寄存器
     */

    /* 麦克风输入, mono.
     */
	AC97_codec_reg_write(CODEC_RECORD_SEL,  0x0000);	    /* MIC */
#if 0
    /* TODO 需要有线路输入, 然后调试. stereo ?
     */
	AC97_codec_reg_write(CODEC_RECORD_SEL,  0x0404);	    /* Linein */
#endif
	AC97_codec_reg_write(CODEC_RECORD_GAIN, 0x0f0f);

	AC97_codec_reg_write(CODEC_GENERAL_PURPOSE, (1<<13) | (1<<10));

	return 0;
}

static int AC97_hardware_reset(void)
{
    int tmo_ms = 100;

    pAC97->hwAC97->csr = AC97_CSR_RESET | AC97_CSR_RESUME;
	delay_us(100);
	
	while (pAC97->hwAC97->csr)                          // wait done
	{
        delay_ms(1);
        tmo_ms--;
        if (tmo_ms <= 0)
            return -1;
    }
    
    AC97_codec_reg_write(CODEC_RESET, 1);               // Reset CODEC
    delay_us(10);
    
    return 0;
}

static int AC97_hardware_initialize(void)
{
	int codec_id;

    /* AC97 codec cold-reset
     */
    if (!pAC97->hw_reseted)
    {
	    if (AC97_hardware_reset() != 0)
            return -1;
	    pAC97->hw_reseted = 1;
	}

	/* Eables the generation of an interrupt
     */
	pAC97->hwAC97->intmask = 0;

	(void)pAC97->hwAC97->intclr;
    (void)pAC97->hwAC97->oc_intclr;
    (void)pAC97->hwAC97->ic_intclr;
    (void)pAC97->hwAC97->wr_intclr;
    (void)pAC97->hwAC97->rd_intclr;

	codec_id = AC97_codec_reg_read(CODEC_VENDOR_ID1);
	codec_id = (codec_id << 16) | AC97_codec_reg_read(CODEC_VENDOR_ID2);

	if (codec_id != ALC655_ID)
	{
	    printk("Found unknown codec chip ID=%08X\r\n", codec_id);
	    return -1;
	}

    printk("Found ALC655 chip, ID=%08X\r\n", codec_id);

	AC97_codec_chip_init();

	return 0;
}

//-------------------------------------------------------------------------------------------------
// DMA operator
//-------------------------------------------------------------------------------------------------

#if 0
static int AC97_wait_dma_done(void)
{
    unsigned int dma_ctrl = READ_REG32(pAC97->dmaCtrl);

    while (dma_ctrl & ask_valid)
    {
        delay_ms(1);
        dma_ctrl = READ_REG32(pAC97->dmaCtrl);
    }

    return 0;
}

static void print_desc_info(int index, LS1x_dma_desc_t *desc)
{
    DBG_OUT("DESC[%i]: \r\n", index);
    DBG_OUT("  next_desc   = %08X\r\n", desc->next_desc);
    DBG_OUT("  mem_addr    = %08X\r\n", desc->mem_addr);
    DBG_OUT("  dev_addr    = %08X\r\n", desc->dev_addr);
    DBG_OUT("  length      = %08X\r\n", desc->length);
    DBG_OUT("  step_length = %08X\r\n", desc->step_length);
    DBG_OUT("  step_times  = %08X\r\n", desc->step_times);
    DBG_OUT("  command     = %08X\r\n", desc->command);
    DBG_OUT("  _blank      = %08X\r\n", desc->_blank);
    
    return;
}
#endif

/*
 * 发送描述符
 */
static int AC97_DMA1_tx_desc_init(int index, void *memAddr, int length)
{
#if (DMA_DESC_COUNT > 1)
    /*
     * next desc address
     */
    if (pAC97->prepare_bytes < pAC97->limited_bytes)
    {
        int next_index = SWAP_DESC_INDEX(index);
        unsigned int next_desc = K1_TO_PHYS((unsigned int)pAC97->tx_desc[next_index]);

        pAC97->tx_desc[index]->next_desc = next_desc | next_desc_enable;
    }
    else
        pAC97->tx_desc[index]->next_desc = 0;

#else

    index = 0;
    pAC97->tx_desc[index]->next_desc = 0;

#endif // #if (DMA_DESC_COUNT > 1)

    pAC97->tx_desc[index]->length      = (length & ~0x1F) >> 2;
    pAC97->tx_desc[index]->step_length = 0;
    pAC97->tx_desc[index]->step_times  = 1;
    pAC97->tx_desc[index]->command     = bit(13) | desc_cmd_int_enable | desc_cmd_r_w;
    pAC97->tx_desc[index]->mem_addr    = K0_TO_PHYS((unsigned int)memAddr);
    pAC97->tx_desc[index]->dev_addr    = K0_TO_PHYS(LS1x_AC97_TX_DMA_ADDR) |
                                         desc_daddr_ac97_wrmode_4 |
                                         desc_daddr_ac97_wren;

    if (pAC97->num_channels == 2)
    {
        pAC97->tx_desc[index]->dev_addr |= desc_daddr_ac97_stereo;
    }

    pAC97->tx_desc[index]->_blank = length & ~0x1F;   // saved length

    return 0;
}

/*
 * 接收描述符
 */
static int AC97_DMA2_rx_desc_init(int index, void *memAddr, int length)
{
#if (DMA_DESC_COUNT > 1)
    /*
     * next desc address
     */
    if (pAC97->prepare_bytes < pAC97->limited_bytes)
    {
        int next_index = SWAP_DESC_INDEX(index);
        unsigned int next_desc = K1_TO_PHYS((unsigned int)pAC97->rx_desc[next_index]);

        pAC97->rx_desc[index]->next_desc = next_desc | next_desc_enable;
    }
    else
        pAC97->rx_desc[index]->next_desc = 0;

#else

    index = 0;
    pAC97->rx_desc[index]->next_desc = 0;

#endif // #if (DMA_DESC_COUNT > 1)

    pAC97->rx_desc[index]->length      = (length & ~0x1F) >> 2;
    pAC97->rx_desc[index]->step_length = 0;
    pAC97->rx_desc[index]->step_times  = 1;
    pAC97->rx_desc[index]->command     = desc_cmd_int_enable;
    pAC97->rx_desc[index]->mem_addr    = K0_TO_PHYS((unsigned int)memAddr);
    pAC97->rx_desc[index]->dev_addr    = K0_TO_PHYS(LS1x_AC97_RX_DMA_ADDR);

    pAC97->rx_desc[index]->_blank = length & ~0x1F;   // saved length

    return 0;
}

//-------------------------------------------------------------------------------------------------
// AC97 DMA operator
//-------------------------------------------------------------------------------------------------

#if (DMA_DESC_COUNT > 1)

/*
 * load data from wav-file to the data-buffer.
 * if has data, waiting for play, else close the wavefile and clear enviroment.
 *
 * return the prepared length
 */
static int AC97_DMA1_tx_prepare(int index)
{
    int this_bytes;

    if (!pAC97->user_buffer)            // has done
        return -1;

    this_bytes = pAC97->limited_bytes - pAC97->prepare_bytes;
    this_bytes &= ~0x1F;

    if (this_bytes > 0)
    {
        this_bytes = this_bytes < DMA_BUF_SIZE ? this_bytes : DMA_BUF_SIZE;
        
        memcpy(pAC97->data_buf[index], pAC97->user_buffer + pAC97->prepare_bytes, this_bytes);
    #if (DMA_BUF_CACHED)
        flush_dcache();
    #endif
        AC97_DMA1_tx_desc_init(index, (void *)pAC97->data_buf[index], this_bytes);
        
        pAC97->prepare_bytes += this_bytes;
    }

    return this_bytes;
}

/*
 * save data to wav-file from the data-buffer.
 * if need more, waiting for record, else close the wavefile and clear enviroment.
 *
 * return the prepared length
 */
static int AC97_DMA2_rx_prepare(int index)
{
    int this_bytes;

    if (!pAC97->user_buffer)            // has done
        return -1;

    /*
     * See if need record more
     */
    this_bytes = pAC97->limited_bytes - pAC97->prepare_bytes;
    this_bytes &= ~0x1F;

    if (this_bytes > 0)
    {
        this_bytes = this_bytes < DMA_BUF_SIZE ? this_bytes : DMA_BUF_SIZE;

#if (DMA_BUF_CACHED)
        flush_dcache();
#endif
        AC97_DMA2_rx_desc_init(index, (void *)pAC97->data_buf[index], this_bytes);
        
        pAC97->prepare_bytes += this_bytes;
    }
    
    return this_bytes;
}

#endif // #if (DMA_DESC_COUNT > 1)

/*
 * start transmit data to codec
 */
static void AC97_DMA1_tx_start(int index)
{
    unsigned int dma_cmd;

	/* Start DMA control, physical address. Always from first desc
     */
	dma_cmd = K1_TO_PHYS((unsigned int)pAC97->tx_desc[index]) & dma_desc_addr_mask;
    DMA_INT_ENABLE(1);
	WRITE_REG32(pAC97->dmaCtrl, dma_cmd | dma_start | dma_dev_ac97_write);
	ls1x_sync();

	/* Wait start command done.
	 */
	while (READ_REG32(pAC97->dmaCtrl) & dma_start)
		;

	WRITE_REG32(pAC97->dmaCtrl, dma_cmd | ask_valid);
	ls1x_sync();

    pAC97->state = AC97_PLAYING;

    // AC97_wait_dma_done();
    // AC97_DMA1_tx_stop(-1, false);
}

/*
 * transmit data to codec has done
 */
static int AC97_DMA1_tx_done(int index)
{
#if (DMA_DESC_COUNT == 1)

    pAC97->data_buf[0] = NULL;
    pAC97->state = AC97_IDLE;
    DBG_OUT("AC97 play done.\r\n");

    return 0;

#else
    /*
     * Play over
     */
    if ((pAC97->state & AC97_FORCE_STOP) ||
        (pAC97->done_bytes >= pAC97->limited_bytes))
    {
        pAC97->user_buffer = NULL;
        pAC97->limited_bytes = 0;
        pAC97->prepare_bytes = 0;
        pAC97->done_bytes    = 0;
        pAC97->num_channels  = 1;
        pAC97->sample_bytes  = 2;
        pAC97->state = AC97_IDLE;

        DBG_OUT("AC97 play done.\r\n");
        return 0;
    }

    return pAC97->done_bytes - pAC97->limited_bytes;

#endif // #if (DMA_DESC_COUNT == 1)
}

/*
 * stop transmit data to codec
 */
static inline void AC97_DMA1_tx_stop(int index, bool from_irq)
{
    DMA_INT_DISABLE(1);
    WRITE_REG32(pAC97->dmaCtrl, dma_stop | dma_dev_ac97_write);

    if (!from_irq)
    {
#if (BSP_USE_OS)
        AC97_DMA1_tx_event_fire(false);
#else
        AC97_DMA1_tx_done(index);
#endif
    }
}

/*
 * start receive data from codec
 */
static void AC97_DMA2_rx_start(int index)
{
    unsigned int dma_cmd;

	/* Start DMA control, physical address. Always from first desc
     */
	dma_cmd  = K1_TO_PHYS((unsigned int)pAC97->rx_desc[index]) & dma_desc_addr_mask;
    DMA_INT_ENABLE(2);
	WRITE_REG32(pAC97->dmaCtrl, dma_cmd | dma_start | dma_dev_ac97_read);
	ls1x_sync();

	/* Wait start command done.
	 */
	while (READ_REG32(pAC97->dmaCtrl) & dma_start)
		;

	WRITE_REG32(pAC97->dmaCtrl, dma_cmd | ask_valid);
	ls1x_sync();

    pAC97->state = AC97_RECORDING;

    // AC97_wait_dma_done();
    // AC97_DMA2_rx_stop(-1, false);
}

/*
 * receiving from codec has done
 */
static int AC97_DMA2_rx_done(int index)
{
#if (DMA_DESC_COUNT == 1)

    pAC97->data_buf[0] = NULL;
    pAC97->state = AC97_IDLE;
    DBG_OUT("AC97 record done.\r\n");
    
    return 0;

#else

    /*
     * Save data_buf[index] to wave file
     */
    int this_bytes = pAC97->rx_desc[index]->_blank;

    if (!(pAC97->state & AC97_FORCE_STOP) && (this_bytes > 0))
    {
        memcpy((void *)pAC97->user_buffer + pAC97->done_bytes,
               (void *)pAC97->data_buf[index], this_bytes);
        pAC97->done_bytes += this_bytes * pAC97->num_channels;

        pAC97->rx_desc[index]->length   = 0;
        pAC97->rx_desc[index]->mem_addr = 0;
        pAC97->rx_desc[index]->command  = 0;
        pAC97->rx_desc[index]->_blank   = 0;
    }

    /*
     * Record done
     */
    if ((pAC97->state & AC97_FORCE_STOP) ||
        (pAC97->done_bytes >= pAC97->limited_bytes))
    {
        pAC97->user_buffer = NULL;
        pAC97->num_channels  = 1;
        pAC97->sample_bytes  = 2;
        pAC97->limited_bytes = 0;
        pAC97->prepare_bytes = 0;
        pAC97->done_bytes    = 0;
        pAC97->state = AC97_IDLE;

        DBG_OUT("AC97 record done.\r\n");
        return 0;
    }

    return pAC97->done_bytes - pAC97->limited_bytes;

#endif // #if (DMA_DESC_COUNT == 1)
}

/*
 * stop receive data from codec
 */
static void AC97_DMA2_rx_stop(int index, bool from_irq)
{
    DMA_INT_DISABLE(2);
    WRITE_REG32(pAC97->dmaCtrl, dma_stop | dma_dev_ac97_read);

    if (!from_irq)
    {
#if (BSP_USE_OS)
        AC97_DMA2_rx_event_fire(false);
#else
        AC97_DMA2_rx_done(index);
#endif
    }
}

//-------------------------------------------------------------------------------------------------
// AC97 RTOS wait done
//-------------------------------------------------------------------------------------------------

#if BSP_USE_OS

static void AC97_DMA1_tx_event_fire(bool from_isr)
{
#if defined(OS_RTTHREAD)
    rt_event_send(ac97_event, AC97_DMA1_EVENT);

#elif defined(OS_UCOS)
    unsigned char err;
    OSFlagPost(ac97_event, (OS_FLAGS)AC97_DMA1_EVENT, OS_FLAG_SET, &err);

#elif defined(OS_FREERTOS)
    if (from_isr)
    {
        BaseType_t xResult, xHigherPriorityTaskWoken = pdFALSE;
        xResult = xEventGroupSetBitsFromISR(ac97_event,
                                            AC97_DMA1_EVENT,
                                            &xHigherPriorityTaskWoken);
        if (xResult != pdPASS)  /* Was the message posted successfully? */
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    else
    {
        xEventGroupSetBits(ac97_event, AC97_DMA1_EVENT);
    }

#endif
}

static void AC97_DMA2_rx_event_fire(bool from_isr)
{
#if defined(OS_RTTHREAD)
    rt_event_send(ac97_event, AC97_DMA2_EVENT);

#elif defined(OS_UCOS)
    unsigned char err;
    OSFlagPost(ac97_event, (OS_FLAGS)AC97_DMA2_EVENT, OS_FLAG_SET, &err);

#elif defined(OS_FREERTOS)
    if (from_isr)
    {
        BaseType_t xResult, xHigherPriorityTaskWoken = pdFALSE;
        xResult = xEventGroupSetBitsFromISR(ac97_event,
                                            AC97_DMA2_EVENT,
                                            &xHigherPriorityTaskWoken);
        if (xResult != pdPASS)  /* Was the message posted successfully? */
        {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
    else
    {
        xEventGroupSetBits(ac97_event, AC97_DMA1_EVENT);
    }

#endif
}

#endif // #if BSP_USE_OS

//-------------------------------------------------------------------------------------------------
// AC97 Interrupt handler
//-------------------------------------------------------------------------------------------------

/*
 * DMA1 transmit irq handler, trigger after 1 desc transfer done
 */
static void AC97_DMA1_tx_interrupt_handler(int vector, void *arg)
{
#if (DMA_DESC_COUNT == 1)

    AC97_DMA1_tx_stop(0, true);

  #if BSP_USE_OS
    AC97_DMA1_tx_event_fire(true);      /* if RTOS send event to do done */
  #else
    AC97_DMA1_tx_done(0);
  #endif

#else

    int index = pAC97->desc_index;

    /* record the done bytes first
     */
    pAC97->done_bytes += pAC97->tx_desc[index]->_blank;

    pAC97->tx_desc[index]->length   = 0;
    pAC97->tx_desc[index]->mem_addr = 0;
    pAC97->tx_desc[index]->command  = 0;
    pAC97->rx_desc[index]->_blank   = 0;

  #if BSP_USE_OS

    if (pAC97->done_bytes >= pAC97->limited_bytes)
        AC97_DMA1_tx_stop(index, true);     /* if done then stop DMA */

    AC97_DMA1_tx_event_fire(true);          /* if RTOS send event to load data buffer */

  #else

    if (pAC97->done_bytes >= pAC97->limited_bytes)
    {
        AC97_DMA1_tx_stop(index, false);    /* if done then stop DMA */
        return;
    }

    AC97_DMA1_tx_prepare(index);
    pAC97->desc_index = SWAP_DESC_INDEX(index);

  #endif

#endif // #if (DMA_DESC_COUNT == 1)
}

/*
 * DMA2 receive irq handler, trigger after 1 desc transfer done
 */
static void AC97_DMA2_rx_interrupt_handler(int vector, void *arg)
{
#if (DMA_DESC_COUNT == 1)

    AC97_DMA2_rx_stop(0, true);

  #if BSP_USE_OS
    AC97_DMA2_rx_event_fire(true);      /* if RTOS send event to do done */
  #else
    AC97_DMA2_rx_done(0);
  #endif

#else

    int index = pAC97->desc_index, rx_done;

    /* TEST rx is done
     */
    pAC97->done_bytes += pAC97->rx_desc[index]->_blank;
    rx_done = (pAC97->done_bytes >= pAC97->limited_bytes) ? 1 : 0;
    pAC97->done_bytes -= pAC97->rx_desc[index]->_blank;

  #if BSP_USE_OS

    if (rx_done)
        AC97_DMA2_rx_stop(index, true);     /* if done then stop DMA */

    AC97_DMA2_rx_event_fire(true);          /* if RTOS send event to save data buffer */

  #else

    if (rx_done)
    {
        AC97_DMA2_rx_stop(index, false);    /* if done then stop DMA */
        return;
    }

    AC97_DMA2_rx_done(index);               /* To save these data */
    AC97_DMA2_rx_prepare(index);
    pAC97->desc_index = SWAP_DESC_INDEX(index);

  #endif

#endif // #if (DMA_DESC_COUNT == 1)
}

//-------------------------------------------------------------------------------------------------
// AC97 working with wave file
//-------------------------------------------------------------------------------------------------

static int AC97_prepare_play(const ac97_param_t *param)
{
    int count;
    
    if (pAC97->state != AC97_IDLE)
    {
        printk("AC97 is busy now.\r\n");
        return -1;
    }
    
    if ((param == NULL) || (param->buffer == NULL) ||
       ((count = param->buf_length & ~0x1F) <= 0))
    {
        printk("AC97 parameter error.\r\n");
        return -2;
    }

#if (DMA_DESC_COUNT == 1)
    /*
     * Audio data buffer use user-buffer
     */
    void *audioBuf = (void *)param->buffer;
    
    if ((((unsigned int)audioBuf) & 0x1F) != 0) // not aligned 32
    {
        printk("AC97 buffer address should be aligned 32.\r\n");
        return -3;
    }
    
#endif

    /*
     * Prepare
     */
    AC97_codec_chip_init();                     /* need sample_bytes? */

    pAC97->state = AC97_PLAY_READY;
    pAC97->num_channels = param->num_channels == 2 ? 2 : 1;
    pAC97->sample_bytes = param->sample_bits == 16 ? 2 : 1;

#if (DMA_DESC_COUNT == 1)

  #if (DMA_BUF_CACHED)
    flush_dcache();
  #endif
    pAC97->data_buf[0] = (unsigned char *)audioBuf;
    AC97_DMA1_tx_desc_init(0, (void *)audioBuf, count);

#else // DMA_DESC_COUNT == 2

    pAC97->user_buffer   = param->buffer;
    pAC97->limited_bytes = count;
    pAC97->prepare_bytes = 0;
    pAC97->done_bytes    = 0;

    AC97_DMA1_tx_prepare(0);
    AC97_DMA1_tx_prepare(1);

#endif // #if (DMA_DESC_COUNT == 1)

    return count;
}

static int AC97_prepare_record(const ac97_param_t *param)
{
    int count;

    if (pAC97->state != AC97_IDLE)
    {
        printk("AC97 is busy now.\r\n");
        return -1;
    }

    if ((param == NULL) || (param->buffer == NULL) ||
       ((count = param->buf_length & ~0x1F) <= 0))
    {
        printk("AC97 parameter error.\r\n");
        return -2;
    }

#if (DMA_DESC_COUNT == 1)
    /*
     * Audio data buffer use user-buffer
     */
    void *audioBuf = (void *)param->buffer;
    
    if ((((unsigned int)audioBuf) & 0x1F) != 0) // not aligned 32
    {
        printk("AC97 buffer address should be aligned 32.\r\n");
        return -3;
    }
    
#endif // #if (DMA_DESC_COUNT == 1)

    /*
     * Prepare
     */
    AC97_codec_chip_init();             /* need sample_bytes? */

    pAC97->state = AC97_RECORD_READY;
    pAC97->num_channels = 1;
    pAC97->sample_bytes = 2;
    
#if (DMA_DESC_COUNT == 1)

  #if (DMA_BUF_CACHED)
    flush_dcache();
  #endif
    pAC97->data_buf[0] = (unsigned char *)audioBuf;
    AC97_DMA2_rx_desc_init(0, audioBuf, count);

#else

    pAC97->user_buffer   = param->buffer;
    pAC97->limited_bytes = count;
    pAC97->prepare_bytes = 0;
    pAC97->done_bytes    = 0;

    AC97_DMA2_rx_prepare(0);
    AC97_DMA2_rx_prepare(1);

#endif // #if (DMA_DESC_COUNT == 1)

    return 0;
}

//-------------------------------------------------------------------------------------------------

static int AC97_start(void)
{
#if (DMA_DESC_COUNT > 1)
    pAC97->desc_index = 0;
#endif

/**
 * 线程唤醒
 */
#if defined(OS_RTTHREAD)
    rt_thread_resume(ac97_thread);
#elif defined(OS_UCOS)
    OSTaskResume(AC97_TASK_PRIO);
#elif defined(OS_FREERTOS)
    vTaskResume(ac97_handle);
#endif

    if (pAC97->state == AC97_PLAY_READY)
        AC97_DMA1_tx_start(0);
    else if (pAC97->state == AC97_RECORD_READY)
        AC97_DMA2_rx_start(0);
    else
        return -1;

    return 0;
}

static int AC97_stop(void)
{
    if (pAC97->state & AC97_PLAYING)
        AC97_stop_play();
    else if (pAC97->state & AC97_RECORDING)
        AC97_stop_record();
    else
        return -1;

    return 0;
}

static int AC97_stop_play(void)
{
#if (BSP_USE_OS)
    pAC97->state |= AC97_FORCE_STOP;
    AC97_DMA1_tx_stop(0, false);
#else
    AC97_DMA1_tx_stop(0, false);
    pAC97->state = AC97_IDLE;
#endif

    return 0;
}

static int AC97_stop_record(void)
{
#if (BSP_USE_OS)
    pAC97->state |= AC97_FORCE_STOP;
    AC97_DMA2_rx_stop(0, false);
#else
    AC97_DMA2_rx_stop(0, false);
    pAC97->state = AC97_IDLE;
#endif

    return 0;
}

//-------------------------------------------------------------------------------------------------
// driver implements
//-------------------------------------------------------------------------------------------------

/*
 * Initialize the AC97 device
 */
STATIC_DRV int LS1x_AC97_initialize(void *dev, void *arg)
{
	unsigned int i, ptr;

	if (pAC97->intialized == 1)
		return 0;

	memset((void *)pAC97, 0, sizeof(LS1x_AC97_dev_t));

    pAC97->hwAC97  = (LS1x_AC97_regs_t *)LS1x_AC97_BASE;
	pAC97->dmaCtrl = LS1x_DMA_CTRL_ADDR;
    pAC97->num_channels = 1;                /* default mono */
    pAC97->sample_bytes = 2;                /* default 16 bits */

	if (AC97_hardware_initialize() != 0)    /* Initialize hardware */
    {
        printk("AC97 initialize hardware fail.\r\n");
        return -1;
    }

    for (i=0; i<DMA_DESC_COUNT; i++)
    {
        /*
         * DMA descriptor initialize
         */
	    ptr = (unsigned int)rx_dma_desc_addr + i * 64;
        ptr = K0_TO_K1(ptr);
	    pAC97->rx_desc[i] = (LS1x_dma_desc_t *)ptr;

	    ptr = (unsigned int)tx_dma_desc_addr + i * 64;
	    ptr = K0_TO_K1(ptr);
	    pAC97->tx_desc[i] = (LS1x_dma_desc_t *)ptr;

#if (DMA_DESC_COUNT > 1)
	    /*
         * DMA data buffer
         */
	    ptr = (unsigned int)rw_dma_buf_addr + i * DMA_BUF_SIZE;
    #if (!DMA_BUF_CACHED)
	    ptr = K0_TO_K1(ptr);
    #endif
	    pAC97->data_buf[i] = (unsigned char *)ptr;
	    
#endif
	}

    /*
	 * Interrupt handler
	 */
    DMA_INT_INIT(1);
    DMA_INT_INIT(2);

	ls1x_install_irq_handler(LS1x_DMA1_IRQ, AC97_DMA1_tx_interrupt_handler, (void *)pAC97);
	ls1x_install_irq_handler(LS1x_DMA2_IRQ, AC97_DMA2_rx_interrupt_handler, (void *)pAC97);

#if defined(OS_RTTHREAD)

    ac97_event = rt_event_create("ac97_dma", 0);        /* 创建DMA发送的事件 */

    ac97_thread = rt_thread_create("ac97thread",        /* 创建响应DMA事件的线程 */
                                    AC97_DMA12_irq_response_task,
                                    NULL,               // arg
                                    AC97_STK_SIZE*4,    // statck size
                                    AC97_TASK_PRIO,     // priority
                                    AC97_TASK_SLICE);   // slice ticks

    if (ac97_thread == NULL)
    {
        printk("create ac97 dma response thread fail!\r\n");
		return -1;
	}

    rt_thread_startup(ac97_thread);
    rt_thread_suspend(ac97_thread);                 	/* 创建后挂起, 等待唤醒 */
    
#elif defined(OS_UCOS)

    unsigned char err;
    ac97_event = OSFlagCreate(0, &err);             	/* 创建DMA发送的事件 */

    err = OSTaskCreate(AC97_DMA12_irq_response_task, 	/* 创建响应DMA事件的线程 */
                       NULL,
        #if OS_STK_GROWTH == 1
                      (void *)&ac97_stack[AC97_STK_SIZE - 1],
        #else
                      (void *)&ac97_stack[0],
        #endif
                       AC97_TASK_PRIO);

    if ((err != 0) && (ac97_event == NULL))
    {
        printk("create ac97 dma response thread fail!\r\n");
		return -1;
	}

    OSTaskSuspend(AC97_TASK_PRIO);                  /* 创建后挂起, 等待唤醒 */
    
#elif defined(OS_FREERTOS)

    ac97_event = xEventGroupCreate();               /* 创建DMA发送的事件 */

    xTaskCreate(AC97_DMA12_irq_response_task,       /* 创建响应DMA事件的线程 */
                "ac97task",
                AC97_STK_SIZE, NULL,
                AC97_TASK_PRIO,
                &ac97_handle);

    if ((ac97_handle == NULL) || (ac97_event == NULL))
    {
        printk("create ac97 dma response thread fail!\r\n");
		return -1;
	}

    vTaskSuspend(ac97_handle);                      /* 创建后挂起, 等待唤醒 */

#endif

	pAC97->intialized = 1;

    DBG_OUT("AC97 controller initialized.\r\n");

	return 0;
}

/*
 * Open AC97 to play or record
 */
STATIC_DRV int LS1x_AC97_open(void *dev, void *arg)
{
    ac97_param_t *pa = (ac97_param_t *)arg;

    if (pa == NULL)
        return -1;

    if (pa->flag == 2)              // DMA_WRITE
        return AC97_prepare_play((const ac97_param_t *)pa);
    else if (pa->flag == 1)         // DMA_READ
        return AC97_prepare_record((const ac97_param_t *)pa);

	return -1;
}

/*
 * Close AC97, if it is playing or recording, stop it.
 */
STATIC_DRV int LS1x_AC97_close(void *dev, void *arg)
{
    if ((unsigned int)arg != 0)     /* force stop */
        return AC97_stop();
    else
        return -1;
}

/*
 * If AC97 is recording, read back the record data buffer
 */
STATIC_DRV int LS1x_AC97_read(void *dev, void *buf, int size, void *arg)
{
    DBG_OUT("AC97 read() is not implement yet.\r\n");
	return 0;
}

/*
 * If AC97 is ready for playing, play the data buffer
 */
STATIC_DRV int LS1x_AC97_write(void *dev, void *buf, int size, void *arg)
{
    DBG_OUT("AC97 write() is not implement yet.\r\n");
	return 0;
}

/*
 * If AC97 is working, do start/pause/resume/stop and set volume etc.
 */
STATIC_DRV int LS1x_AC97_ioctl(void *dev, int cmd, void *arg)
{
    int rt = 0;
    unsigned int val  = (unsigned int)arg;
    unsigned int *ptr = (unsigned int *)arg;
    unsigned short reg_num, reg_val;

    switch (cmd)
    {
        /**
         * AC97 参数
         */
        case IOCTL_AC97_POWER:
            if (pAC97->state == AC97_IDLE)
                AC97_codec_power_switch(val);
            else
                rt = -1;
            break;

        case IOCTL_AC97_RESET:
            if (pAC97->state == AC97_IDLE)
                AC97_hardware_reset();
            else
                rt = -1;
            break;

        case IOCTL_AC97_SET_VOLUME:
            unpack_vol_arg(val, &reg_val, &reg_num);
            if (is_codec_volume_register(reg_num))
                AC97_codec_reg_write(reg_num, reg_val);
            else
                rt = -1;
            break;

        case IOCTL_AC97_RECORD_GAIN:
            reg_val = ((unsigned short)val) & 0x1F1F;
            if (reg_val == 0)
                reg_val = 0x8000;
            AC97_codec_reg_write(CODEC_RECORD_GAIN, reg_val);
            break;

        case IOCTL_AC97_RECORD_SEL:
            reg_val = ((unsigned short)val) & 0x0707;
            AC97_codec_reg_write(CODEC_RECORD_SEL, reg_val);
            break;

        case IOCTL_AC97_SET_REGISTER:
            unpack_reg_arg(val, &reg_val, &reg_num);
            rt = is_codec_register(reg_num);
            if ((rt == 1) || (rt == 2))
            {
                AC97_codec_reg_write(reg_num, reg_val);
                rt = 0;
            }
            else
                rt = -1;
            break;

        case IOCTL_AC97_GET_REGISTER:
            unpack_reg_arg(*ptr, &reg_val, &reg_num);   // unsigned int *
            if (is_codec_register(reg_num) > 0)
                *ptr = (unsigned int)AC97_codec_reg_read(reg_num);
            else
                rt = -1;
            break;

        /**
         * AC97 工作
         */
        case IOCTL_AC97_START:
            rt = AC97_start();
            break;

        case IOCTL_AC97_STOP:
            rt = AC97_stop();
            break;

        default:
            break;
    }

	return rt;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * AC97 driver operators
 */
static driver_ops_t LS1x_AC97_drv_ops =
{
    .init_entry  = LS1x_AC97_initialize,
    .open_entry  = LS1x_AC97_open,
    .close_entry = LS1x_AC97_close,
    .read_entry  = LS1x_AC97_read,
    .write_entry = LS1x_AC97_write,
    .ioctl_entry = LS1x_AC97_ioctl,
};
driver_ops_t *ls1x_ac97_drv_ops = &LS1x_AC97_drv_ops;

void *devAC97 = (void *)&ls1x_ac97;

#endif

/******************************************************************************
 * User API
 */
int ls1x_ac97_play(const ac97_param_t *param)
{
    int rt;

    LS1x_AC97_initialize(NULL, NULL);

    rt = AC97_prepare_play(param);
    if (rt <= 0)
        return rt;

    /*
     * Start
     */
    AC97_start();

    return 0;
}

/*
 * TODO Record from MIC, only n_channels==1 && sample_bits==16 is correct.
 *
 *      Maybe related ac97 input channels
 *
 */
int ls1x_ac97_record(const ac97_param_t *param)
{
    int rt;

    LS1x_AC97_initialize(NULL, NULL);

    rt = AC97_prepare_record(param);
    if (rt < 0)
        return rt;

    /*
     * Start
     */
    AC97_start();

    return 0;
}

#if BSP_USE_OS

/*
 * While RTOS, response the DMA1/DMA2 interrupt event to goon
 */
static void AC97_DMA12_irq_response_task(void *arg)
{
    unsigned int wait_event;

    /*
     * Dead loop until kill This...
     */
    while (1)
    {
        switch (pAC97->state)
        {
            case AC97_PLAYING:   wait_event = AC97_DMA1_EVENT; break;
            case AC97_RECORDING: wait_event = AC97_DMA2_EVENT; break;
            default:             wait_event = 0;               break;
        }

        if (wait_event == 0)
        {
            SLEEP(100);
            continue;
        }

        /*
         * Waiting...
         */
    #if defined(OS_RTTHREAD)

	    unsigned int recv = 0;
        rt_event_recv(ac97_event,
                      wait_event,
                      RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER,           /* 无限等待 */
                      &recv);

        if (recv != wait_event)
	        continue;

    #elif defined(OS_UCOS)

        unsigned char  err;
        unsigned short recv;
        recv = OSFlagPend(ac97_event,
                          (OS_FLAGS)wait_event,     /* 接收事件 */
                          OS_FLAG_WAIT_SET_ALL |    /* 接收事件标志位置1时有效，否则任务挂在这里 */
                          OS_FLAG_CONSUME,          /* 清除指定事件标志位 */
                          0,                        /* 0=无限等待 */
                          &err);

        if (recv != wait_event)
	        continue;

    #elif defined(OS_FREERTOS)

        unsigned int recv;
        recv = xEventGroupWaitBits(ac97_event,        /* 事件对象句柄 */
                                   wait_event,        /* 接收事件 */
                                   pdTRUE,            /* 退出时清除事件位 */
                                   pdTRUE,            /* 满足感兴趣的所有事件 */
                                   portMAX_DELAY);    /* 指定超时事件, 一直等 */

        if (recv != wait_event)
	        continue;

    #endif

    #if (DMA_DESC_COUNT > 1)
        int index = pAC97->desc_index;
    #endif

        switch (pAC97->state)
        {
            case AC97_PLAYING:
            case AC97_PLAYING | AC97_FORCE_STOP:

        #if (DMA_DESC_COUNT == 1)
                AC97_DMA1_tx_done(0);
        #else
                if (AC97_DMA1_tx_done(index) != 0)
                {
                    pAC97->desc_index = SWAP_DESC_INDEX(index);
                    AC97_DMA1_tx_prepare(index);
                }
        #endif
                break;

            case AC97_RECORDING:
            case AC97_RECORDING | AC97_FORCE_STOP:

        #if (DMA_DESC_COUNT == 1)
                AC97_DMA2_rx_done(0);
        #else
                if (AC97_DMA2_rx_done(index) != 0)
                {
                    pAC97->desc_index = SWAP_DESC_INDEX(index);
                    AC97_DMA2_rx_prepare(index);
                }
        #endif
                break;
        }

        if (pAC97->state == AC97_IDLE)
        {
            /**
             * 线程挂起
             */
        #if defined(OS_RTTHREAD)
            rt_thread_suspend(ac97_thread);
        #elif defined(OS_UCOS)
            OSTaskSuspend(AC97_TASK_PRIO);
        #elif defined(OS_FREERTOS)
            vTaskSuspend(ac97_handle);
        #endif
        }
    }
}

#endif // #if BSP_USE_OS

int ls1x_ac97_busy(void)
{
    return (int)(pAC97->state != AC97_IDLE);
}

#endif // #ifdef BSP_USE_AC97

/*
 * @@ END
 */

