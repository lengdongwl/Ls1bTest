/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * w25x40.c
 *
 * this file contains the low level ls1x SPI Flash driver
 * and board-specific functions
 *
 *  Created on: 2014-2-26
 *      Author: Bian
 */

#include "bsp.h"

#ifdef W25X40_DRV

#include <stdio.h>
#include <errno.h>

#include "../ls1x_spi_bus_hw.h"
#include "ls1x_io.h"
#include "ls1x_spi_bus.h"

#if defined(LS1B)
#include "ls1b.h"
#elif defined(LS1C)
#include "ls1c.h"
#else
#error "No Loongson1x SoC defined."
#endif

#include "spi/w25x40.h"
#include "w25x40_hw.h"

//-----------------------------------------------------------------------------

#define FLASH_MEMBASE			0xBFC00000

#define SPI_MEM_CMD_RDSR		W25X40_CMD_RDSR			// 0x05
#define SPI_MEM_CMD_WREN		W25X40_CMD_WREN			// 0x06
#define SPI_MEM_CMD_PP			W25X40_CMD_PP			// 0x02
#define SPI_MEM_CMD_READ		W25X40_CMD_READ			// 0x03
#define SPI_MEM_CMD_RDID		W25X40_CMD_RDMDID		// 0x90
#define SPI_MEM_CMD_SE			W25X40_CMD_SE4K			// 0x20  // sector erase
#define SPI_MEM_CMD_SE4			SPI_MEM_CMD_SE
#define SPI_MEM_CMD_SE32		W25X40_CMD_BE32K		// 0x52	 // block erase, 32KB
#define SPI_MEM_CMD_SE64		W25X40_CMD_BE64K		// 0xD8	 // block erase, 64KB
#define SPI_MEM_CMD_BE			W25X40_CMD_CE			// 0xC7  // bulk erase

#define SPI_FLASH_BAUDRATE		W25X40_BAUDRATE
#define SPI_FLASH_BITSPERCHAR	W25X40_BITSPERCHAR
#define SPI_FLASH_PAGE_SIZE		W25X40_PAGE_SIZE
#define SPI_FLASH_BLOCK_SIZE	W25X40_BLOCK_SIZE_64
#define SPI_FLASH_CHIP_SIZE		W25X40_CHIP_SIZE

#define SPI_FLASH_SR_BUSY_BIT	w25x_sr_busy

//-----------------------------------------------------------------------------

#define CHECK_DONE(rt) \
	do {               \
        if (0 != rt)   \
            goto lbl_done; \
    } while (0);

//-----------------------------------------------------------------------------
// device
//-----------------------------------------------------------------------------

/*
 * w25x40 片选
 */
#define W25X40_CS       0       // w25x40
 
/*
 * w25x40 芯片参数
 */
static w25x40_param_t m_chipParam =
{
    .baudrate             = SPI_FLASH_BAUDRATE,         /* 速率受SPI 限制 */
    .erase_before_program = true,
    .empty_state          = 0xff,
    .page_size            = SPI_FLASH_PAGE_SIZE,        /* programming page size in byte */
    .sector_size          = SPI_FLASH_BLOCK_SIZE,       /* 64K erase sector size in byte */
    .mem_size             = SPI_FLASH_CHIP_SIZE,        /* 512K total capacity in byte   */
};

/*
 * w25x40 通信模式
 */
static LS1x_SPI_mode_t m_devMode =
{
	.baudrate = 10000000,                       /* 实际用的速率: 10M */
	.bits_per_char = SPI_FLASH_BITSPERCHAR,     /* how many bits per byte/word/longword? */
	.lsb_first = false,                         /* true: send LSB first */
	.clock_pha = true,                          /* clock phase    - spi mode */
	.clock_pol = true,                          /* clock polarity - spi mode */
	.clock_inv = true,                          /* true: inverted clock (low active) - cs high or low */
	.clock_phs = false,                         /* true: clock starts toggling at start of data tfr - interface mode */
};

//-----------------------------------------------------------------------------

/*
 * read W25X40BV manufacturer / device ID
 */
static int W25X40_read_id(LS1x_SPI_bus_t *pSPI, unsigned int *id)
{
	unsigned char cmd[4];
	unsigned char val[2];
	unsigned int memory_en;
	int	rt, ret_cnt = 0;

	/******************************************************************
	 * check spi-flash is set to fast read mode
	 ******************************************************************/

    rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_GET_FAST_READ_MODE, &memory_en);
	CHECK_DONE(rt);

	/*
	 * change to not fast read mode.
	 */
	if (spi_param_memory_en & memory_en)
	{
		rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_FAST_READ_DISABLE, NULL);
        CHECK_DONE(rt);
	}

	/******************************************************************
	 * begin to read id
	 ******************************************************************/

	/* start transfer */
	rt = ls1x_spi_send_start(pSPI, W25X40_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls1x_spi_ioctl(pSPI, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* address device */
	rt = ls1x_spi_send_addr(pSPI, W25X40_CS, true);
	CHECK_DONE(rt);

	/*
	 * send "read md/id" command and address
	 */
	cmd[0] = SPI_MEM_CMD_RDID;
	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;

	ret_cnt = ls1x_spi_write_bytes(pSPI, cmd, 4);
	if (ret_cnt < 0)
		rt = ret_cnt;
	CHECK_DONE(rt);

	/*
	 * fetch read data
	 */
	ret_cnt = ls1x_spi_read_bytes(pSPI, val, 2);
	if (ret_cnt < 0)
		rt = ret_cnt;
	CHECK_DONE(rt);

	/*
	 * manufacturer id: bit15-bit8
	 * device id:       bit7-bit0
	 */
	*id = (val[0] << 8) | val[1];

	/*
	 * change back to fast read mode.
	 */
	if (spi_param_memory_en & memory_en)
	{
		rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
		CHECK_DONE(rt);
	}

lbl_done:
	/* terminate transfer */
	ls1x_spi_send_stop(pSPI, W25X40_CS);

	return rt;
}

/*
 * read W25X40BV status register
 */
static int W25X40_read_sr(LS1x_SPI_bus_t *pSPI, unsigned int *sr)
{
	unsigned char cmd, val;
	int rt, ret_cnt = 0;

	/* start transfer */
	rt = ls1x_spi_send_start(pSPI, W25X40_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls1x_spi_ioctl(pSPI, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* select device - chipsel */
	rt = ls1x_spi_send_addr(pSPI, W25X40_CS, true);
	CHECK_DONE(rt);

	/*
	 * send read status register command
	 */
	cmd = SPI_MEM_CMD_RDSR;
	ret_cnt = ls1x_spi_write_bytes(pSPI, &cmd, 1);
	if (ret_cnt < 0)
		rt = ret_cnt;
	CHECK_DONE(rt);

	/*
	 * fetch read data
	 */
	ret_cnt = ls1x_spi_read_bytes(pSPI, &val, 1);
	if (ret_cnt < 0)
		rt = ret_cnt;
	CHECK_DONE(rt);

	*sr = val;

lbl_done:
	/* terminate transfer */
	ls1x_spi_send_stop(pSPI, W25X40_CS);

	return rt;
}

static int W25X40_is_busy(LS1x_SPI_bus_t *pSPI)
{
	unsigned int sr;

	if (W25X40_read_sr(pSPI, &sr) != 0)
	{
		return -1;
	}

	return (sr & w25x_sr_busy);
}

/*
 * wait for x ms for spi-flash work done.
 */
static int W25X40_wait_ms(LS1x_SPI_bus_t *pSPI, unsigned int ms)
{
	while (ms-- > 0)
	{
		delay_ms(1);    /* delay 1 ms per loop, if done, exit immediately */
		if (0 == W25X40_is_busy(pSPI)) // TODO test dead lock ?
		    return 0;
	}

	return -ETIMEDOUT;
}

/*
 * set spi-flash writable.
 */
static int W25X40_set_write_en(LS1x_SPI_bus_t *pSPI)
{
	unsigned char cmd;
	int rt, ret_cnt = 0;

	/* start transfer */
	rt = ls1x_spi_send_start(pSPI, W25X40_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls1x_spi_ioctl(pSPI, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* select device - chipsel */
	rt = ls1x_spi_send_addr(pSPI, W25X40_CS, true);
	CHECK_DONE(rt);

	/*
	 * send write_enable command
	 */
	cmd = SPI_MEM_CMD_WREN;
	ret_cnt = ls1x_spi_write_bytes(pSPI, &cmd, 1);
	if (ret_cnt < 0)
		rt = ret_cnt;

lbl_done:
	/* terminate transfer */
	ls1x_spi_send_stop(pSPI, W25X40_CS);

	return rt;
}

/*
 * spi-flash erase 1 sector.
 */
static int W25X40_erase_1_sector(LS1x_SPI_bus_t *pSPI, unsigned char cmd, unsigned int addr)
{
    unsigned char cmdbuf[4];
    unsigned int memory_en, ms;
	int rt, cmd_size, ret_cnt = 0;

	/******************************************************************
	 * check spi-flash is set to fast read mode
	 ******************************************************************/

	rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_GET_FAST_READ_MODE, &memory_en);
	CHECK_DONE(rt);

	/*
	 * change to not fast read mode.
	 */
	if (spi_param_memory_en & memory_en)
	{
		rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_FAST_READ_DISABLE, NULL);
		CHECK_DONE(rt);
	}

	/******************************************************************
	 * after diasble spi-flash fast read mode...
	 ******************************************************************/

	/* check arguments */
	if (addr > m_chipParam.mem_size)
		rt = -EADDRNOTAVAIL;
	CHECK_DONE(rt);

	/******************************************************************
	 * begin to erase sector.
	 ******************************************************************/

	/**************************************************
	 * First we must set flash write enable
	 * Here also has set the transfer mode.
	 **************************************************/

	rt = W25X40_set_write_en(pSPI);
	CHECK_DONE(rt);

	/**************************************************
	 * Second we can do page-program
	 **************************************************/

	/* start transfer */
	rt = ls1x_spi_send_start(pSPI, W25X40_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = -ls1x_spi_ioctl(pSPI, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* address device */
	rt = ls1x_spi_send_addr(pSPI, W25X40_CS, true);
	CHECK_DONE(rt);

	/*
	 * send command and address,
	 * command may be:
	 * W25X40_CMD_SE		0x20	 sector erase, 4KB,  CMD, A23-A16, A15-A8, A7-A0
	 * W25X40_CMD_BE32		0x52	 block erase, 32KB,  CMD, A23-A16, A15-A8, A7-A0
	 * W25X40_CMD_BE64		0xD8	 block erase, 64KB,  CMD, A23-A16, A15-A8, A7-A0
	 */
	cmdbuf[0] = cmd;
	cmdbuf[1] = (addr >> 16) & 0xff;
	cmdbuf[2] = (addr >>  8) & 0xff;
	cmdbuf[3] = (addr >>  0) & 0xff;
	cmd_size  = 4;

	ret_cnt = ls1x_spi_write_bytes(pSPI, cmdbuf, cmd_size);
	if (ret_cnt < 0)
		rt = ret_cnt;
	CHECK_DONE(rt);

	/* terminate transfer */
	ls1x_spi_send_stop(pSPI, W25X40_CS);
	
	/**************************************************
	 * Delay for Erase done
	 **************************************************/

	switch (cmd)
	{
		case SPI_MEM_CMD_SE4:	ms = 200;	break;
		case SPI_MEM_CMD_SE32:	ms = 800;	break;
		case SPI_MEM_CMD_SE64:
		default:				ms = 1000;	break;
	}

	/* poll flash sr-busy flag, until device is finished */
	rt = W25X40_wait_ms(pSPI, ms);
	CHECK_DONE(rt);

	/*
	 * change back to fast read mode.
	 */
	if (spi_param_memory_en & memory_en)
	{
		rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
		CHECK_DONE(rt);
	}

lbl_done:
	/* terminate transfer */
	ls1x_spi_send_stop(pSPI, W25X40_CS);

	return rt;
}

/*
 * spi-flash erase chip.
 */
static int W25X40_erase_chip(LS1x_SPI_bus_t *pSPI)
{
    unsigned char cmd;
	unsigned int  memory_en;
	int rt, ret_cnt = 0;

	/******************************************************************
	 * check spi-flash is set to fast read mode
	 ******************************************************************/

	rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_GET_FAST_READ_MODE, &memory_en);
	CHECK_DONE(rt);

	/*
	 * change to not fast read mode.
	 */
	if (spi_param_memory_en & memory_en)
	{
		rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_FAST_READ_DISABLE, NULL);
		CHECK_DONE(rt);
	}

	/******************************************************************
	 * begin to erase chip.
	 * First we must set flash write enable
	 ******************************************************************/

	rt = W25X40_set_write_en(pSPI);
	CHECK_DONE(rt);

	/**************************************************
	 * Second we can do erase
	 **************************************************/

	/* start transfer */
	rt = ls1x_spi_send_start(pSPI, W25X40_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = -ls1x_spi_ioctl(pSPI, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* address device */
	rt = ls1x_spi_send_addr(pSPI, W25X40_CS, true);
	CHECK_DONE(rt);

	/*
	 * send command.
	 */
	cmd = SPI_MEM_CMD_BE;
	ret_cnt = ls1x_spi_write_bytes(pSPI, &cmd, 1);
	if (ret_cnt < 0)
		rt = ret_cnt;
	CHECK_DONE(rt);

	/* terminate transfer */
	ls1x_spi_send_stop(pSPI, W25X40_CS);
	
	/**************************************************
	 * Delay for Erase done
	 **************************************************/

	/* poll flash sr-busy flag, until device is finished */
	rt = W25X40_wait_ms(pSPI, 4000);
	CHECK_DONE(rt);

	/*
	 * change back to fast read mode.
	 */
	if (spi_param_memory_en & memory_en)
	{
		rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
		CHECK_DONE(rt);
	}

lbl_done:
	/* terminate transfer */
	ls1x_spi_send_stop(pSPI, W25X40_CS);

	return rt;
}

/*
 * check special spi-mem is blank
 */
static int W25X40_check_isblank(LS1x_SPI_bus_t *pSPI, unsigned int addr, int *size)
{
    unsigned char bit, *ptr;
    unsigned int  bblank = 1;
	int rt, off, cnt;

	rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
	CHECK_DONE(rt);

	cnt = *size;
	bit = fls(cnt) - 1;

	off = (0xFFFFFFFF << bit) & addr;
	if ((cnt < 0) || (off > (m_chipParam.mem_size - cnt)))
		rt = -EADDRNOTAVAIL;
	CHECK_DONE(rt);

	/*
	 * byte -> int -> byte to fast
	 */
	ptr = (void *)(unsigned int)(off + FLASH_MEMBASE);
	while (cnt-- > 0)
	{
		if ((unsigned char)*ptr != 0xFF)
		{
			bblank = 0;
			break;
		}
		ptr++;
	}

	*size = bblank;

lbl_done:
	return rt;
}

/**********************************************************************
 * spi flash driver impelememt
 **********************************************************************/

/**********************************************************************
 * Purpose: initialize
 * Input Parameters:
 * Return Value:		0 = ok or error code
 **********************************************************************/

STATIC_DRV int W25X40_init(void *bus, void *arg)
{
    return 0;
}

/**********************************************************************
 * Purpose: write a block of data to flash
 * Input Parameters:
 * Return Value:		0 = ok or error code
 **********************************************************************/

STATIC_DRV int W25X40_write(void *bus, void *buf, int size, void *arg)
{
	int rt, cmd_size;
	int curr_cnt, bytes_sent = 0, ret_cnt = 0;
	unsigned char cmdbuf[4], *pchbuf;
	unsigned int  memory_en, offset;
    LS1x_SPI_bus_t *pSPI = (LS1x_SPI_bus_t *)bus;

    if ((bus == NULL) || (buf == NULL))
        return -1;

    offset = *(unsigned int *)arg;
    pchbuf = (unsigned char *)buf;

	/******************************************************************
	 * check spi-flash is set to fast read mode
	 ******************************************************************/

	rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_GET_FAST_READ_MODE, &memory_en);
	CHECK_DONE(rt);

	/*
	 * change to not fast read mode.
	 */
	if (spi_param_memory_en & memory_en)
	{
		rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_FAST_READ_DISABLE, NULL);
		CHECK_DONE(rt);
	}

	/* check arguments */
	if ((size <= 0) ||
		(size > m_chipParam.mem_size) ||
		(offset > (m_chipParam.mem_size - size)))
	{
		rt = -EFAULT;
	}
	else if (pchbuf == NULL)
	{
		rt = -EADDRNOTAVAIL;
	}
	CHECK_DONE(rt);

	/******************************************************************
	 * begin to write data, PP = 256 bytes.
	 ******************************************************************/

	while (size > bytes_sent)
	{
		/*
		 * mention: curr_cnt / page_size / off.
		 */
		curr_cnt = size - bytes_sent;
		if ((m_chipParam.page_size > 0) &&
			(offset / m_chipParam.page_size) !=
			((offset+curr_cnt+1) / m_chipParam.page_size))
		{
			curr_cnt = m_chipParam.page_size - (offset % m_chipParam.page_size);
		}

		/**************************************************
		 * First we must set flash write enable
		 * Here also has set the transfer mode.
		 **************************************************/

		rt = W25X40_set_write_en(pSPI);
		CHECK_DONE(rt);

		/**************************************************
		 * Second we can do page-program
		 **************************************************/

		/* start transfer */
		rt = ls1x_spi_send_start(pSPI, W25X40_CS);
		CHECK_DONE(rt);

		/* set transfer mode */
		rt = -ls1x_spi_ioctl(pSPI, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
		CHECK_DONE(rt);

		/* address device */
		rt = ls1x_spi_send_addr(pSPI, W25X40_CS, true);
		CHECK_DONE(rt);

		/*
		 * send "page program" command and address
		 * remove the mem_size check
		 */
		cmdbuf[0] = SPI_MEM_CMD_PP;
		cmdbuf[1] = (offset >> 16) & 0xff;
		cmdbuf[2] = (offset >>  8) & 0xff;
		cmdbuf[3] = (offset >>  0) & 0xff;
		cmd_size  = 4;

		ret_cnt = ls1x_spi_write_bytes(pSPI, cmdbuf, cmd_size);
		if (ret_cnt < 0)
			rt = ret_cnt;
		CHECK_DONE(rt);

		/*
		 * send write data
		 */
		ret_cnt = ls1x_spi_write_bytes(pSPI, pchbuf, curr_cnt);
		if (ret_cnt < 0)
			rt = ret_cnt;
		CHECK_DONE(rt);

		/* terminate transfer */
		rt = ls1x_spi_send_stop(pSPI, W25X40_CS);
		CHECK_DONE(rt);

		/**************************************************
		 * Delay for page-program done
		 **************************************************/

		/* poll flash sr-busy flag, until device is finished */
		rt = W25X40_wait_ms(pSPI, 5);
		CHECK_DONE(rt);

		/* adjust bytecount to be sent and pointers */
		bytes_sent += curr_cnt;
		offset     += curr_cnt;
		pchbuf     += curr_cnt;
	}

	/*
	 * change back to fast read mode.
	 */
	if (spi_param_memory_en & memory_en)
	{
		rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
		CHECK_DONE(rt);
	}

//  *(unsigned int *)arg = offset;
	rt = bytes_sent;

lbl_done:
	/* terminate transfer */
	ls1x_spi_send_stop(pSPI, W25X40_CS);

	return rt;
}

/**********************************************************************
 * Purpose: read a block of data from flash
 * Input Parameters:
 * Return Value: 		0 = ok or error code
 **********************************************************************/

STATIC_DRV int W25X40_read(void *bus, void *buf, int size, void *arg)
{
	int rt=0, cmd_size, ret_cnt = 0;
	unsigned char cmdbuf[4], *pchbuf;
	unsigned int  memory_en, offset;
    LS1x_SPI_bus_t *pSPI = (LS1x_SPI_bus_t *)bus;

    if ((bus == NULL) || (buf == NULL))
        return -1;
        
    offset = *(unsigned int *)arg;
    pchbuf = (unsigned char *)buf;

	/* check arguments */
	if ((size <= 0)
		|| (size > m_chipParam.mem_size)
		|| (offset > (m_chipParam.mem_size - size)))
	{
		rt = -EFAULT;
	}
	else if (pchbuf == NULL)
	{
		rt = -EADDRNOTAVAIL;
	}
	CHECK_DONE(rt);

	/******************************************************************
	 * check spi-flash is set to fast read mode
	 ******************************************************************/

	rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_GET_FAST_READ_MODE, &memory_en);
	CHECK_DONE(rt);

	if (spi_param_memory_en & memory_en)
	{
		unsigned char *ptr;
		ptr = (void *)(unsigned int)(offset + FLASH_MEMBASE);

		while (size-- > 0)
		{
			*pchbuf++ = *ptr++;			/* is here need delay ? */
			ret_cnt++;
		}

		return ret_cnt;
	}

	/******************************************************************
	 * if spi-flash engine is not set...
	 ******************************************************************/

	/* start transfer */
	rt = ls1x_spi_send_start(pSPI, W25X40_CS);
	CHECK_DONE(rt);

	/* set transfer mode */
	rt = ls1x_spi_ioctl(pSPI, IOCTL_SPI_I2C_SET_TFRMODE, &m_devMode);
	CHECK_DONE(rt);

	/* select device - chipsel */
	rt = ls1x_spi_send_addr(pSPI, W25X40_CS, true);
	CHECK_DONE(rt);

	if (offset >= m_chipParam.mem_size)
	{
		/*
		 * HACK: beyond size of memory array? then read status register instead
		 */
		/*
		 * send read status register command
		 */
		cmdbuf[0] = SPI_MEM_CMD_RDSR;
		ret_cnt = ls1x_spi_write_bytes(pSPI, cmdbuf, 1);
		if (ret_cnt < 0)
			rt = ret_cnt;
	}
	else
	{
		/*
		 * send read command and address
		 * remove the mem_size check
		 */
		cmdbuf[0] = SPI_MEM_CMD_READ;
		cmdbuf[1] = (offset >> 16) & 0xff;
		cmdbuf[2] = (offset >>  8) & 0xff;
		cmdbuf[3] = (offset >>  0) & 0xff;
		cmd_size  = 4;

		/*
		 * get read data
		 */
		ret_cnt = ls1x_spi_write_bytes(pSPI, cmdbuf, cmd_size);

		if (ret_cnt < 0)
			rt = ret_cnt;
	}
	CHECK_DONE(rt);

	/*
	 * fetch read data
	 */
	ret_cnt = ls1x_spi_read_bytes(pSPI, pchbuf, size);
	if (ret_cnt < 0)
		rt = ret_cnt;
	CHECK_DONE(rt);

//  *(unsigned int *)arg = offset;
	rt = ret_cnt;

lbl_done:
	/* terminate transfer */
	ls1x_spi_send_stop(pSPI, W25X40_CS);

	return rt;
}

STATIC_DRV int W25X40_open(void *bus, void *arg)
{
#if 0
    if (bus == NULL)
        return -1;
        
    LS1x_SPI_bus_t *pSPI = (LS1x_SPI_bus_t *)bus;
    ls1x_spi_ioctl(pSPI, IOCTL_FLASH_FAST_READ_DISABLE, NULL);
#endif
	return 0;
}

STATIC_DRV int W25X40_close(void *bus, void *arg)
{
#if 0
    if (bus == NULL)
        return -1;

    LS1x_SPI_bus_t *pSPI = (LS1x_SPI_bus_t *)bus;
    ls1x_spi_ioctl(pSPI, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
#endif
	return 0;
}

STATIC_DRV int W25X40_ioctl(void *bus, int cmd, void *arg)
{
	int rt = 0;
	unsigned int val = 0;
    LS1x_SPI_bus_t *pSPI = (LS1x_SPI_bus_t *)bus;
    
    if (bus == NULL)
        return -1;
        
	switch (cmd)
	{
		case IOCTL_FLASH_FAST_READ_ENABLE:
			rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_FAST_READ_ENABLE, NULL);
			break;

		case IOCTL_FLASH_FAST_READ_DISABLE:
			rt = -ls1x_spi_ioctl(pSPI, IOCTL_FLASH_FAST_READ_DISABLE, NULL);
			break;

		case IOCTL_W25X40_READ_ID:
			rt = W25X40_read_id(pSPI, &val);
			*(unsigned int *)arg = val;
			break;

		case IOCTL_W25X40_READ_JDECID:
		case IOCTL_W25X40_READ_UNIQUEID:
			rt = -ENOTSUP;
			break;

		case IOCTL_W25X40_ERASE_4K:
			val = (unsigned int)arg;
			rt = W25X40_erase_1_sector(pSPI, SPI_MEM_CMD_SE4, val);
			break;

		case IOCTL_W25X40_ERASE_32K:
			val = (unsigned int)arg;
			rt = W25X40_erase_1_sector(pSPI, SPI_MEM_CMD_SE32, val);
			break;

		case IOCTL_W25X40_ERASE_64K:
			val = (unsigned int)arg;
			rt = W25X40_erase_1_sector(pSPI, SPI_MEM_CMD_SE64, val);
			break;

		case IOCTL_W25X40_SECTOR_ERASE:
			val = (unsigned int)arg;
			rt = W25X40_erase_1_sector(pSPI, SPI_MEM_CMD_SE, val);
			break;

		case IOCTL_W25X40_BULK_ERASE:
			rt = W25X40_erase_chip(pSPI);
			break;

		case IOCTL_W25X40_WRITE_PROTECT:
			rt = -ENOTSUP;
			break;

		default:
			if (cmd & IOCTL_W25X40_IS_BLANK)
			{
				int size;

				if (cmd & IOCTL_W25X40_ERASE_64K)
					size = 64 * 1024;
				else if (cmd & IOCTL_W25X40_ERASE_32K)
					size = 32 * 1024;
				else 					// default is IOCTL_W25X40_ERASE_4K
					size = 4 * 1024;

				val = *(unsigned int *)arg;
				rt = W25X40_check_isblank(pSPI, val, &size);
				if (0 == rt)
					*((unsigned int *)arg) = (unsigned int)size;
			}
			else
				return -ENOTSUP;

			break;
	}

	return rt;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * SPI0-W25X40 driver operators
 */
static driver_ops_t LS1x_W25X40_drv_ops =
{
    .init_entry  = W25X40_init,
    .open_entry  = W25X40_open,
    .close_entry = W25X40_close,
    .read_entry  = W25X40_read,
    .write_entry = W25X40_write,
    .ioctl_entry = W25X40_ioctl,
};
driver_ops_t *ls1x_w25x40_drv_ops = &LS1x_W25X40_drv_ops;
#endif

/******************************************************************************
 * user api
 */

#define TOUCHSCREEN_CAL_POS   (501*1024)

int save_touchscreen_calibrate_values_to_w25x40(int *calibrate_coords, int len)
{
    unsigned int pos = TOUCHSCREEN_CAL_POS;
    
    W25X40_init(busSPI0, NULL);
    W25X40_erase_1_sector(busSPI0, SPI_MEM_CMD_SE, pos);    // erase first
    W25X40_write(busSPI0, (unsigned char *)calibrate_coords, len*sizeof(int), &pos);

    return len;
}

int load_touchscreen_calibrate_values_from_w25x40(int *calibrate_coords, int len)
{
    unsigned int pos = TOUCHSCREEN_CAL_POS;
    
    W25X40_init(busSPI0, NULL);
    W25X40_read(busSPI0, (unsigned char *)calibrate_coords, len*sizeof(int), &pos);

    return len;
}

#endif


