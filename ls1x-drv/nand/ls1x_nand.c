/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_nand.c
 *
 *  Created on: 2013-11-21
 *      Author: Bian
 */

/******************************************************************************
 * 本代码不是线程安全的.
 *
 * 如果由yaffs 调用, 需要实现yaffs_lock() 和yaffs_unlock() 函数.
 */

#include "bsp.h"

//-----------------------------------------------------------------------------

#ifdef BSP_USE_NAND

#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_irq.h"
#define LS1x_DMA0_IRQ 		LS1B_DMA0_IRQ
#define LS1x_NAND_BASE 		LS1B_NAND_BASE
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_irq.h"
#define LS1x_DMA0_IRQ 		LS1C_DMA0_IRQ
#define LS1x_NAND_BASE 		LS1C_NAND_BASE
#else
#error "No Loongson1x SoC defined."
#endif

#include "cpu.h"

#include "ls1x_io.h"
#include "drv_os_priority.h"

#include "ls1x_nand.h"
#include "ls1x_dma_hw.h"
#include "ls1x_nand_hw.h"

#include "nand_k9f1g08.h"

#if 0
#define DBG_NAND(...)    printk(__VA_ARGS__)
#else
#define DBG_NAND(...)
#endif

/*
 * NAND with DMA 0, XXX Never used this interrupt,
 */
#define USE_DMA0_INTERRUPT  0

/******************************************************************************
 * ls1x nand priv defination
 */
typedef struct
{
	/* Hardware shortcuts */
	LS1x_NAND_regs_t *hwNand;				/* NAND 寄存器 */
	unsigned int      dmaCtrl;				/* DMA  控制寄存器 */

	LS1x_dma_desc_t  *dmaDesc;				/* DMA  描述符 */
	unsigned char    *dmaBuf;				/* DMA  数据缓冲区 */

	/* Driver state */
	int          intialized;				/* 没有初始化时 ? */

	/* Statics */
#if (USE_DMA0_INTERRUPT)
	unsigned int intr_cnt;
#endif
	unsigned int error_cnt;
	unsigned int read_bytes;
	unsigned int read_cnt;
	unsigned int readerr_cnt;
	unsigned int write_bytes;
	unsigned int write_cnt;
	unsigned int writeerr_cnt;
	unsigned int erase_cnt;
	unsigned int eraseerr_cnt;
} LS1x_NAND_dev_t;

/*
 * DMA descriptor buffer and DMA transfer buffer.
 */
#define FULL_PAGE_SIZE	(BYTES_OF_PAGE+OOBBYTES_OF_PAGE)

/*
 * DMA ORDER_ADDR_IN Register's addr-ask, must align 64
 */
static char dma_desc_base[64] __attribute__((aligned(64)));

/*
 * DMA transfer 4 bytes once, align 32
 */
static char dma_buf_base[FULL_PAGE_SIZE] __attribute__((aligned(32)));

/*
 * Here is NAND interface defination
 */
static LS1x_NAND_dev_t  ls1x_NAND;

/******************************************************************************
 * MACROS
 ******************************************************************************/

/*
 * defined LS1x_K0_CACHED in ls1x.h
 */
#define NAND_RWBUF_CACHED	1		    /* Cached rw buffer. */

/******************************************************************************
 * Hardware Operating Implement
 ******************************************************************************/

#if 0
static int LS1x_NAND_read_sr(LS1x_NAND_dev_t *pNand, unsigned int *status)
{
	unsigned int saved_timing, id_h=0;

    saved_timing = pNand->hwNand->timing;
	pNand->hwNand->timing = 0x0412;

	pNand->hwNand->cmd = 0;
	pNand->hwNand->cmd = 0;
	pNand->hwNand->cmd = nand_cmd_rd_sr | nand_cmd_start;
	ls1x_sync();

    while (!(pNand->hwNand->cmd & nand_cmd_done))
        ;

	id_h = pNand->hwNand->id_h;
	*status = (id_h & nand_status_mask) >> nand_status_shift;

    pNand->hwNand->timing = saved_timing;

    return 0;
}
#endif

static int LS1x_NAND_read_id(LS1x_NAND_dev_t *pNand, unsigned int *id)
{
	unsigned int saved_timing;

    saved_timing = pNand->hwNand->timing;
	pNand->hwNand->timing = 0x0412;

	pNand->hwNand->cmd = 0;
	pNand->hwNand->cmd = 0;
	pNand->hwNand->cmd = nand_cmd_rd_id | nand_cmd_start;
	ls1x_sync();

    while (!(pNand->hwNand->cmd & nand_cmd_done))
        ;

	id[0] = pNand->hwNand->id_l;
	id[1] = pNand->hwNand->id_h & nand_id_h_mask;

    pNand->hwNand->timing = saved_timing;

    return 0;
}

static int LS1x_NAND_waitdone(LS1x_NAND_dev_t *pNand, int timeout_us)
{
    unsigned int cmd;

	while (timeout_us > 0)
	{
		delay_us(10);							    /* 一次延时 1us */
		cmd = pNand->hwNand->cmd;
		if ((cmd & nand_cmd_done) && (cmd & nand_cmd_rdy_0))
			return 0;

		timeout_us -= 10;
	}

	/*
	 * If timeout, send cancel command.
	 * Sequence: 0 - 0 - nand_cmd_done?
	 */
	pNand->hwNand->cmd = 0;
	pNand->hwNand->cmd = 0;
	pNand->hwNand->cmd = nand_cmd_done;
	ls1x_sync();

	return -ETIMEDOUT;
}

static void LS1x_NAND_chip_reset(LS1x_NAND_dev_t *pNand)
{
	unsigned int rt;

	pNand->hwNand->cmd = 0;
	pNand->hwNand->cmd = 0;
	pNand->hwNand->cmd = nand_cmd_reset | nand_cmd_start;
	ls1x_sync();

	/* delay 500us, wait */
	rt = -LS1x_NAND_waitdone(pNand, 500);
	if (0 != rt)
		pNand->error_cnt++;
}

/****************************************************************************************
 * Initialize
 ****************************************************************************************/

static void LS1x_NAND_dma_desc_init(LS1x_NAND_dev_t *pNand)
{
	pNand->dmaDesc->next_desc = 0;
	pNand->dmaDesc->length = 0;
	pNand->dmaDesc->step_length = 0;
	pNand->dmaDesc->step_times = 1;
	pNand->dmaDesc->command = 0;

	/* Must be physical address */
	pNand->dmaDesc->mem_addr = K0_TO_PHYS((unsigned int)pNand->dmaBuf);
	pNand->dmaDesc->dev_addr = K1_TO_PHYS((unsigned int)LS1x_DMA_ACCESS_ADDR);
}

static void LS1x_NAND_hw_initialize(LS1x_NAND_dev_t *pNand)
{
	unsigned int ptr;

	/* Interrupt control register. */
	pNand->dmaCtrl    = LS1x_DMA_CTRL_ADDR;

	pNand->hwNand = (LS1x_NAND_regs_t *)LS1x_NAND_BASE;

	pNand->hwNand->cs_rdy_map = 0x0000;
#if defined(LS1B)
	pNand->hwNand->param  = 0x00030000;			// Nand_size_1GB
#elif defined(LS1C)
	pNand->hwNand->param  = 0x08005000;			/* Nand_size_1GB */
#else
	#error "No Loongson SoC defined."
#endif

	pNand->hwNand->timing = 0x020C;				/* Default value is 0x0412. */

	/* DMA Descriptor, aligned 64 */
	ptr = (unsigned int)dma_desc_base;			/* 低 6 位为 0 */
	ptr = K0_TO_K1(ptr);
	pNand->dmaDesc = (LS1x_dma_desc_t *)ptr;

	/* DMA Buffer, aligned 32 */
	ptr = (unsigned int)dma_buf_base;			/* 低 5 位为 0 */
#if (!NAND_RWBUF_CACHED)
	ptr = K0_TO_K1(ptr);
#endif
	pNand->dmaBuf = (unsigned char *)ptr;

	/* Initialize dma-desc */
	// memset((void *)pNand->dmaDesc, 0, sizeof(LS1x_dma_desc_t));
	LS1x_NAND_dma_desc_init(pNand);

	/* Nand chipset reset */
	LS1x_NAND_chip_reset(pNand);

#if (USE_DMA0_INTERRUPT)
	LS1x_INTC_EDGE(LS1x_INTC0_BASE) |=  INTC0_DMA0_BIT;
	LS1x_INTC_POL( LS1x_INTC0_BASE) &= ~INTC0_DMA0_BIT;
#endif

    LS1x_INTC_IEN(LS1x_INTC0_BASE) &= ~INTC0_DMA0_BIT;
	LS1x_INTC_CLR(LS1x_INTC0_BASE)  =  INTC0_DMA0_BIT;
}

/****************************************************************************************
 * input param:	 1. bufSize  - buffer length of being r/w data
 * 				 2. colAddr   - the r/w offset in destination page
 * 				 3. opFlags - will be r/w "main" or "spare" or "both"
 * output param: 1. nand_cmd - will be r/w "main" or "spare" or "both"
 * 				 2. offset   - colAddr, when r/w in "spare", maybe minus 2048
 * 				 3. size     - acculate size will be r/w
 *
 *                 0                                              2048        2112
 * PAGE            |----------------------main-----------------------|---spare---|
 *
 * 1."main"        ^       |++++++++++++++++++++++++++++++++++++++++++++|
 *   IN:           ^    colAddr                                       bufSize+
 *   OUT:   nand_cmd = main
 *          offset   = colAddr, (if offset>2048, do nothing anyway)
 *          size     = min(bufSize, 2048-colAddr) --> limit in main
 *
 * 2."spare"                                                         ^   |^^^^^^|
 *   IN:                                                              colAddr  bufSize+
 *   OUT:   nand_cmd = spare
 *          offset   = colAddr, (if offset>64, do nothing anyway)
 *          size     = min(bufSize, 64-colAddr) --> limit in spare
 *
 * 3."both"        ^
 *
 * 3.1 AT "main"   ^   |++++++++++++++++++++++++++++++++++++++|
 *	               ^ colAddr                                bufSize+
 *   OUT:   nand_cmd = main
 *          offset   = colAddr
 *          size     = bufSize
 *
 * 3.2 AT "spare"  ^                                                     |^^^^^^|
 *   IN:           ^                                                  colAddr  bufSize+
 *   OUT:   nand_cmd = spare
 *          offset   = colAddr-2048, (if offset>64, do nothing anyway)
 *          size     = min(bufSize, 64-colAddr) --> limit in spare
 *
 * 3.3 AT "BOTH"   ^
 *   IN:           ^                   |^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^|
 *	                                 colAddr                                bufSize+
 *   OUT:   nand_cmd = main+spare
 *          offset   = colAddr
 *          size     = min(bufSize, 2112-colAddr) --> limit in page
 *
 ****************************************************************************************/

static void LS1x_NAND_calc_rw_size(unsigned int bufSize,
		                           unsigned int colAddr,
								   unsigned int opFlags,
							       unsigned int *cmd,
								   int *offset,
								   int *rwsize)
{
	int remain_size=0;

	/*
	 * XXX 读写数据必须4字节的倍数
	 */

	*rwsize = 0;
	if ((bufSize == 0) ||
		((opFlags == NAND_OP_MAIN) && (colAddr >= BYTES_OF_PAGE)) ||
		((opFlags == NAND_OP_SPARE) && (colAddr >= OOBBYTES_OF_PAGE)) ||
		(((opFlags == (NAND_OP_MAIN | NAND_OP_SPARE)) && (colAddr >= FULL_PAGE_SIZE))))
	{
		return;
	}

	switch (opFlags)
	{
		case NAND_OP_MAIN:
			*cmd |= nand_cmd_main;
			*offset = colAddr;
			remain_size = BYTES_OF_PAGE - colAddr;
			*rwsize = (bufSize < remain_size) ? bufSize : remain_size;
			break;

		case NAND_OP_SPARE:
			*cmd |= nand_cmd_spare;
			*offset = colAddr;
			remain_size = OOBBYTES_OF_PAGE - colAddr;
			*rwsize = (bufSize < remain_size) ? bufSize : remain_size;
			break;

		case NAND_OP_MAIN | NAND_OP_SPARE:
			if (bufSize + colAddr < BYTES_OF_PAGE)
			{
				/* High-boundary in "main" area */
				*cmd |= nand_cmd_main;
				*offset = colAddr;
				remain_size = BYTES_OF_PAGE - colAddr;
				*rwsize = (bufSize < remain_size) ? bufSize : remain_size;
			}
			else
			{
				/* high-boundary in "spare" area
				 */
				if (colAddr < BYTES_OF_PAGE)
				{
					/* Low-boundary in "main" area */
					*cmd |= nand_cmd_main | nand_cmd_spare;
					*offset = colAddr;
					remain_size = FULL_PAGE_SIZE - colAddr;
					*rwsize = (bufSize < remain_size) ? bufSize : remain_size;
				}
				else
				{
					/* Low-boundary in "spare" area */
					*cmd |= nand_cmd_spare;
					*offset = colAddr - BYTES_OF_PAGE;
					remain_size = OOBBYTES_OF_PAGE - (colAddr - BYTES_OF_PAGE);
					*rwsize = (bufSize < remain_size) ? bufSize : remain_size;
				}
			}
			break;

		default:
			break;
	}
}

static void LS1x_NAND_send_rw_command(LS1x_NAND_dev_t *pNand, unsigned int pageNum,
							          unsigned int offset, unsigned int nand_cmd)
{
	pNand->hwNand->cmd = 0;
	pNand->hwNand->cmd = 0;

	if (nand_cmd_spare & nand_cmd)
	{
		pNand->hwNand->addr_h = MAIN_SPARE_ADDRH(pageNum);
		if (nand_cmd_main & nand_cmd)
			pNand->hwNand->addr_l = MAIN_SPARE_ADDRL(pageNum) + offset;
		else
			pNand->hwNand->addr_l = MAIN_SPARE_ADDRL(pageNum) + BYTES_OF_PAGE + offset;
	}
	else
	{
		pNand->hwNand->addr_h = MAIN_ADDRH(pageNum);
		pNand->hwNand->addr_l = MAIN_ADDRL(pageNum) + offset;
	}

	/* Only Command main/spare/write/read: 0x0306 */
	pNand->hwNand->cmd = nand_cmd & (nand_cmd_spare | nand_cmd_main | nand_cmd_write | nand_cmd_read);

    /* XXX haven't started yet */
}

static void LS1x_NAND_cmd_start(LS1x_NAND_dev_t *pNand)
{
    pNand->hwNand->cmd |= nand_cmd_start;
    ls1x_sync();
}

static void LS1x_NAND_dma_start(LS1x_NAND_dev_t *pNand, unsigned int size, int rw)
{
	unsigned int dma_cmd;

	/* Set DMA descriptor */
	LS1x_NAND_dma_desc_init(pNand);
	pNand->dmaDesc->length = (size + 3) >> 2;			/* transfer 4 bytes once */
	pNand->dmaDesc->step_times = 1;

	if (DMA_READ == rw) 	/* READ */
		pNand->dmaDesc->command = desc_cmd_int_enable;
	else 					/* WRITE */
		pNand->dmaDesc->command = desc_cmd_int_enable | desc_cmd_r_w;

	/* Start DMA control, physical address */
	dma_cmd = K1_TO_PHYS((unsigned int)pNand->dmaDesc) & dma_desc_addr_mask;
	dma_cmd |= dma_start | ask_valid;

    LOCK_DMA;
	WRITE_REG32(pNand->dmaCtrl, dma_cmd);
	ls1x_sync();
}

static int LS1x_NAND_read_page(LS1x_NAND_dev_t *pNand,
						       unsigned int     pageNum,
						       unsigned int     colAddr,
						       unsigned char   *dataBuf,
						       unsigned int     bufSize,
						       unsigned int     opFlags,
						       int             *rdSize)
{
	int rt, rd_size = 0, offset = 0;
	unsigned int nand_cmd = nand_cmd_read; // | nand_cmd_start;
	unsigned int saved_timing;

	*rdSize = 0;

	if (pageNum >= PAGES_OF_CHIP)
    	return -EINVAL;

	if (dataBuf == NULL)
    	return -EFAULT;

	/*
	 * Calculate read parameters
	 */
	LS1x_NAND_calc_rw_size(bufSize, colAddr, opFlags, &nand_cmd, &offset, &rd_size);

	if (0 == rd_size)
		return -ENODATA;

#if (NAND_RWBUF_CACHED)
	clean_dcache_nowrite((unsigned)pNand->dmaBuf, rd_size);		/* 清除接收缓冲区 */
#endif

#if (USE_DMA0_INTERRUPT)
	LS1x_INTC_CLR(LS1x_INTC0_BASE) = INTC0_DMA0_BIT;			/* 清中断标志 */
#endif

	/*
	 * Begin do read work
	 */
	WRITE_REG32(pNand->dmaCtrl, dma_stop);

    saved_timing = pNand->hwNand->timing;
	pNand->hwNand->opnum = rd_size;
	pNand->hwNand->timing = 0x0206;

#if defined(LS1C)
	pNand->hwNand->param &= 0xC000FFFF;								/* XXX LS1C */
	pNand->hwNand->param |= rd_size << nand_param_opscope_shift;
#endif

	LS1x_NAND_send_rw_command(pNand, pageNum, offset, nand_cmd);	/* Send NAND commmand */
    LS1x_NAND_cmd_start(pNand);
	LS1x_NAND_dma_start(pNand, rd_size, DMA_READ);					/* Start DMA command */

	/*
	 * Wait start command done.
	 */
	while (READ_REG32(pNand->dmaCtrl) & dma_start)
		;

    // SLEEP(1);       /* RTOS 切换出时间? */

	/* delay 400us */
	rt = -LS1x_NAND_waitdone(pNand, 400);
	if (0 == rt)
	{
		if ((void *)pNand->dmaBuf != (void *)dataBuf)				/* Copy-back data to user. */
		{
			memcpy((void *)dataBuf, (void *)pNand->dmaBuf, rd_size);
			*rdSize = rd_size;										/* Return the accurate read data size */
		}
	}
	else
	{
		printk("NAND read timeout.\r\n");
		pNand->readerr_cnt++;
	}

    pNand->hwNand->timing = saved_timing;
#if (!USE_DMA0_INTERRUPT)
	WRITE_REG32(pNand->dmaCtrl, dma_stop);  /* Stop DMA */
	UNLOCK_DMA;
#endif

	return rt;
}

static int LS1x_NAND_write_page(LS1x_NAND_dev_t *pNand,
						        unsigned int     pageNum,
						        unsigned int     colAddr,
						        unsigned char   *dataBuf,
						        unsigned int     bufSize,
						        unsigned int     opFlags,
						        int             *wrSize)
{
	int rt, wr_size = 0, offset = 0;
	unsigned int nand_cmd = nand_cmd_write; // | nand_cmd_start;
	unsigned int saved_timing;

	*wrSize = 0;

	if (pageNum >= PAGES_OF_CHIP)
		return -EINVAL;

	if (dataBuf == NULL)
		return -EFAULT;

	/*
	 * Calculate write parameters
	 */
	LS1x_NAND_calc_rw_size(bufSize, colAddr, opFlags, &nand_cmd, &offset, &wr_size);

	if (0 == wr_size)
		return -ENODATA;

	memcpy((void *)pNand->dmaBuf, (void *)dataBuf, wr_size);

#if (NAND_RWBUF_CACHED)
	clean_dcache((unsigned)pNand->dmaBuf, wr_size);				/* 写入发送缓冲区 */
#endif

#if (USE_DMA0_INTERRUPT)
	LS1x_INTC_CLR(LS1x_INTC0_BASE) = INTC0_DMA0_BIT;			/* 清中断标志 */
#endif

	WRITE_REG32(pNand->dmaCtrl, dma_stop);							/* Begin to write work */

    saved_timing = pNand->hwNand->timing;
	pNand->hwNand->opnum = wr_size;
	pNand->hwNand->timing = 0x0206;

#if defined(LS1C)
	pNand->hwNand->param &= 0xC000FFFF;								/* XXX LS1C */
	pNand->hwNand->param |= wr_size << nand_param_opscope_shift;
#endif

	LS1x_NAND_send_rw_command(pNand, pageNum, offset, nand_cmd);	/* Send NAND command */
    LS1x_NAND_cmd_start(pNand);
	LS1x_NAND_dma_start(pNand, wr_size, DMA_WRITE);					/* Start DMA command */

	/*
	 * Wait start command done. XXX 应该有个超时
	 */
	while (READ_REG32(pNand->dmaCtrl) & dma_start)
		;

	*wrSize = bufSize;						/* Return the accurate write data size */

    // SLEEP(1);       /* RTOS 切换出时间 */

	/* delay 1000us */
	rt = -LS1x_NAND_waitdone(pNand, 1000);
	if (0 != rt)
	{
		printk("NAND write timeout.\r\n");
		pNand->writeerr_cnt++;
	}

    pNand->hwNand->timing = saved_timing;
#if (!USE_DMA0_INTERRUPT)
	WRITE_REG32(pNand->dmaCtrl, dma_stop);  /* Stop DMA */
	UNLOCK_DMA;
#endif

	return rt;
}

static int LS1x_NAND_is_blank_page(LS1x_NAND_dev_t *pNand, unsigned int pageNum)
{
	unsigned int  size = FULL_PAGE_SIZE / 4;
	unsigned int *data = (unsigned int *)pNand->dmaBuf;
	int rt, rdSize;

	rt = LS1x_NAND_read_page(pNand, pageNum, 0, pNand->dmaBuf,
							 FULL_PAGE_SIZE, NAND_OP_MAIN | NAND_OP_SPARE, &rdSize);

	if (0 == rt)
	{
		while (size--)
		{
			if ((*data) ^ 0xFFFFFFFF)
			{
				rt = -1;
				break;
			}
			data++;
		}
	}
	else
		rt = -EIO;

    if (rt < 0)
        return false;

    return true;
}

static int LS1x_NAND_verify_page(LS1x_NAND_dev_t *pNand,
						         unsigned int     pageNum,
						         unsigned char   *buf,
						         unsigned int     bufSize)
{
	unsigned char *data;
	int rt, rdSize;

	rt = LS1x_NAND_read_page(pNand, pageNum, 0, pNand->dmaBuf,
							 FULL_PAGE_SIZE, NAND_OP_MAIN | NAND_OP_SPARE, &rdSize);

	if (0 == rt)
	{
		data = pNand->dmaBuf;
		while (bufSize--)
		{
			if (*data != *buf)
			{
				rt = -1;
				break;
			}
			data++;
			buf++;
		}
	}
	else
		rt = -EIO;

	return rt;
}

static int LS1x_NAND_mark_badblock(LS1x_NAND_dev_t *pNand, unsigned int blkNum);

static int LS1x_NAND_erase_block(LS1x_NAND_dev_t *pNand, unsigned int blkNum, int checkbad)
{
	unsigned int rt=0, pageNum;

	if (blkNum >= BLOCKS_OF_CHIP)
	{
		printk("Try erase blkNum=0x%08X out of range\r\n", blkNum);
		return -EINVAL;
	}

	pageNum = BLOCK_TO_PAGE(blkNum);

	pNand->hwNand->cmd = 0;
	pNand->hwNand->opnum = 1;

	pNand->hwNand->addr_h = MAIN_ADDRH(pageNum);
	pNand->hwNand->addr_l = MAIN_ADDRL(pageNum);

#if defined(LS1C)
	pNand->hwNand->param &= 0xC000FFFF;							/* XXX LS1X必须设置这个读写大小值 */
	pNand->hwNand->param |= 1 << nand_param_opscope_shift;
#endif

	pNand->hwNand->cmd = nand_cmd_erase_1;                      /* command erase block */
    LS1x_NAND_cmd_start(pNand);

    // SLEEP(3);       /* RTOS 切换出时间 */

	/* delay 10000us */
	rt = -LS1x_NAND_waitdone(pNand, 10000);
	if (0 == rt)
	{
    	pNand->erase_cnt++;
    	DBG_NAND("NAND block %i erase successful.\r\n", blkNum);

	    /*
         * verify and mark the bad block
         */
    	if (checkbad != 0)
        {
            unsigned char tmpbuf[FULL_PAGE_SIZE];

            DBG_NAND("Checking bad of block %i ...\r\n", blkNum);
            memset(tmpbuf, 0xFF, BYTES_OF_PAGE+OOBBYTES_OF_PAGE);   // fill 0xFF

        	for (pageNum = 0; pageNum < PAGES_OF_BLOCK; pageNum++)
	        {
	            if (LS1x_NAND_verify_page(devNAND,
                                          BLOCK_TO_PAGE(blkNum) + pageNum,
                                          tmpbuf,
                                          FULL_PAGE_SIZE) != 0)
                {
                    LS1x_NAND_mark_badblock(devNAND, blkNum);
                    break;
                }
	        }
        }
    }
	else
	{
		printk("NAND erase timeout.\r\n");
		pNand->eraseerr_cnt++;
	}

	return rt;
}

static int LS1x_NAND_erase_chip(LS1x_NAND_dev_t *pNand, int checkbad)
{
	unsigned int blkNum;

	for (blkNum = 0; blkNum < BLOCKS_OF_CHIP; blkNum++)
	{
#if 0
		/* If it is a bad block, skip it. */
		if (LS1x_NAND_is_badblock(pNand, blkNum) == true)
		{
		    printk("Nand block %i is bad.\r\n", (int)blkNum);
        	continue;
        }
#endif

		if (0 != LS1x_NAND_erase_block(pNand, blkNum, checkbad))
		{
		    printk("Erase Nand block %i timeout, fail.\r\n", (int)blkNum);
        	return -ETIMEDOUT;
        }
	}

    DBG_NAND("Erase Nand done.\r\n");

	return 0;
}

static int LS1x_NAND_mark_badblock(LS1x_NAND_dev_t *pNand, unsigned int blkNum)
{
	unsigned int	pageNum;
	int				rt, wrSize;
	unsigned char	oobbuf[8];

	/* Initialize oob as bad */
	memset(oobbuf, 0xFF, 8);
	oobbuf[0] = 0x01;

	/* 1st page number in this block. */
	pageNum = BLOCK_TO_PAGE(blkNum);
	rt = LS1x_NAND_write_page(pNand, pageNum, 0, oobbuf, 8, NAND_OP_SPARE, &wrSize);

	if (0 != rt)
		return rt;

	/* 2st page number in this block. */
	pageNum += 1;
	rt = LS1x_NAND_write_page(pNand, pageNum, 0, oobbuf, 8, NAND_OP_SPARE, &wrSize);

    printk("Mark block %i bad.\r\n", (int)blkNum);

	return rt;
}

static int LS1x_NAND_is_badblock(LS1x_NAND_dev_t *pNand, unsigned int blkNum)
{
	unsigned int  pageNum;
	int           rt, rdSize;
	unsigned char ch0, ch1;

	pageNum = BLOCK_TO_PAGE(blkNum);			/* 1st page number in this block. */
	rt = LS1x_NAND_read_page(pNand, pageNum, 0, pNand->dmaBuf, 8, NAND_OP_SPARE, &rdSize);
	ch0 = pNand->dmaBuf[0];						/* first byte of spare */
	if (0 != rt)
		return rt;

	pageNum += 1;								/* 2st page number in this block. */
	rt = LS1x_NAND_read_page(pNand, pageNum, 0, pNand->dmaBuf, 8, NAND_OP_SPARE, &rdSize);
	ch1 = pNand->dmaBuf[0];						/* first byte of spare */
	if (0 != rt)
		return rt;

	if ((ch0 != 0xFF) || (ch1 != 0xFF))			/* judge it is bad... */
		return true;

	return false;
}

/******************************************************************************
 * Interrupt Implement
 ******************************************************************************/

#if (USE_DMA0_INTERRUPT)
static void LS1x_DMA0_interrupt_handler(int vector, void *arg)
{
	LS1x_NAND_dev_t *pNand = (LS1x_NAND_dev_t *)arg;

	WRITE_REG32(pNand->dmaCtrl, dma_stop);				/* Stop DMA */
    UNLOCK_DMA;

	if (pNand->dmaDesc->command & desc_cmd_r_w)			/* According dma-desc-cmd to know r/w done */
	{
		pNand->write_cnt++;
		pNand->write_bytes += pNand->dmaDesc->length * 4;
	}
	else
	{
		pNand->read_cnt++;
		pNand->read_bytes += pNand->dmaDesc->length * 4;
	}

	LS1x_INTC_CLR(LS1x_INTC0_BASE) = INTC0_DMA0_BIT;

	pNand->intr_cnt++;
}
#endif

/******************************************************************************
 * NAND Driver Implement
 ******************************************************************************/

STATIC_DRV int LS1x_NAND_initialize(void *dev, void *arg)
{
    LS1x_NAND_dev_t *pNand = (LS1x_NAND_dev_t *)dev;

    if (dev == NULL)
        return -1;

	if (pNand->intialized == 1)
		return 0;

	memset((void *)pNand, 0, sizeof(LS1x_NAND_dev_t));

    /*
     * Initialize nand and dma
     */
	LS1x_NAND_hw_initialize(pNand);

#if (USE_DMA0_INTERRUPT)
	/*
	 * Install DMA0 interrupt handler
	 */
	ls1x_install_irq_handler(LS1x_DMA0_IRQ, LS1x_DMA0_interrupt_handler, (void *)dev);
#endif

	pNand->intialized = 1;

    DBG_NAND("NAND controller initialized.\r\n");

	return 0;
}

STATIC_DRV int LS1x_NAND_open(void *dev, void *arg)
{
    if (dev == NULL)
        return -1;

#if (USE_DMA0_INTERRUPT)
	LS1x_INTC_IEN(LS1x_INTC0_BASE) |= INTC0_DMA0_BIT;
    LS1x_INTC_CLR(LS1x_INTC0_BASE)  = INTC0_DMA0_BIT;
#endif

	return 0;
}

STATIC_DRV int LS1x_NAND_close(void *dev, void *arg)
{
    if (dev == NULL)
        return -1;

#if (USE_DMA0_INTERRUPT)
	LS1x_INTC_IEN(LS1x_INTC0_BASE) &= ~INTC0_DMA0_BIT;
	LS1x_INTC_CLR(LS1x_INTC0_BASE)  =  INTC0_DMA0_BIT;
#endif

	return 0;
}

STATIC_DRV int LS1x_NAND_read(void *dev, void *buf, int size, void *arg)
{
    int rt, rdSize = 0;
    LS1x_NAND_dev_t *pNand = (LS1x_NAND_dev_t *)dev;
	NAND_PARAM_t *rdParam = (NAND_PARAM_t *)arg;

    if ((dev == NULL) || (buf == NULL))
        return -1;

	if (!rdParam)
		return -EINVAL;

	if (size & 0x03)
		return -EFAULT;

	/*
	 * Read one page
	 */
	rt = LS1x_NAND_read_page(pNand,
							 rdParam->pageNum,
							 rdParam->colAddr,
							 (unsigned char *)buf,
							 size,
							 rdParam->opFlags,
							 &rdSize);

    if (0 == rt)
        return rdSize;

	return rt;
}

STATIC_DRV int LS1x_NAND_write(void *dev, void *buf, int size, void *arg)
{
    int rt, wrSize = 0;
    LS1x_NAND_dev_t *pNand = (LS1x_NAND_dev_t *)dev;
	NAND_PARAM_t *wrParam = (NAND_PARAM_t *)arg;

    if ((dev == NULL) || (buf == NULL))
        return -1;

	if (!wrParam)
		return -EINVAL;

	if (size & 0x03)
		return -EFAULT;

	/*
	 * Write one page
	 */
	rt = LS1x_NAND_write_page(pNand,
							  wrParam->pageNum,
							  wrParam->colAddr,
							  (unsigned char *)buf,
							  size,
							  wrParam->opFlags,
							  &wrSize);

	if (0 == rt)
		return wrSize;

	return rt;
}

STATIC_DRV int LS1x_NAND_ioctl(void *dev, int cmd, void *arg)
{
	int rt = 0;
	LS1x_NAND_dev_t *pNand = (LS1x_NAND_dev_t *)dev;

    if (dev == NULL)
        return -1;

	switch (cmd)
	{
		case IOCTL_NAND_RESET:
			LS1x_NAND_chip_reset(pNand);
			break;

		case IOCTL_NAND_GET_ID:
			rt = LS1x_NAND_read_id(pNand, (unsigned int *)arg);
			break;

		case IOCTL_NAND_ERASE_BLOCK:
			rt = LS1x_NAND_erase_block(pNand, (unsigned int)arg, 0);
			break;

		case IOCTL_NAND_ERASE_CHIP:
			rt = LS1x_NAND_erase_chip(pNand, (unsigned int)arg);
			break;

		case IOCTL_NAND_PAGE_BLANK:
			rt = LS1x_NAND_is_blank_page(pNand, (unsigned int)arg);
			break;

		case IOCTL_NAND_MARK_BAD_BLOCK:
			rt = LS1x_NAND_mark_badblock(pNand, (unsigned int)arg);
			break;

		case IOCTL_NAND_IS_BAD_BLOCK:
			rt = LS1x_NAND_is_badblock(pNand, (unsigned int)arg);
			break;

		case IOCTL_NAND_GET_STATS:
		case IOCTL_NAND_PAGE_VERIFY:
		default:
			rt = -1;    // not implement
			break;
	}

	return rt;
}

/******************************************************************************
 * NAND device.
 */
void *devNAND = (void *)&ls1x_NAND; // LS1x_NAND_dev_t *

#if (PACK_DRV_OPS)
/******************************************************************************
 * NAND driver operators
 */
static driver_ops_t LS1x_NAND_drv_ops =
{
    .init_entry  = LS1x_NAND_initialize,
    .open_entry  = LS1x_NAND_open,
    .close_entry = LS1x_NAND_close,
    .read_entry  = LS1x_NAND_read,
    .write_entry = LS1x_NAND_write,
    .ioctl_entry = LS1x_NAND_ioctl,
};
driver_ops_t *ls1x_nand_drv_ops = &LS1x_NAND_drv_ops;
#endif

#endif
