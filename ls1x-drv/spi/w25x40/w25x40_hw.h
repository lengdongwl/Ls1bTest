/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * w25x40_hw.h
 *
 *  Created on: 2013-11-1
 *      Author: Bian
 */

#ifndef _W25X40_HW_H_
#define _W25X40_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * w25x40 electrical characteristics
 *
 * clock frequency for all instructions 104M
 * except Read Data (03H)				50M
 *
 * Time Delay:							type	max
 * write status register				10ms	15ms
 * byte program time(first byte)		30us	50us
 *			  (after first byte)		2.5us	12us
 * page program time					0.7ms	3ms
 * sector erase time (4KB)				30ms	200ms
 * block erase time (32KB)				120ms	800ms
 * block erase time (64KB)				150ms	1s
 * chip erase							1s		4s
 *
 */

/*
 * w25x40 instruction set
 */
#define W25X40_CMD_WREN  		0x06	/* write enable */
#define W25X40_CMD_WRDIS		0x04	/* write disable */
#define W25X40_CMD_RDSR			0x05	/* read status */
#define W25X40_CMD_WRSR			0x01	/* write status */
#define W25X40_CMD_READ			0x03	/* read data:
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A16, A15-A8, A7-A0, D7-D0, ...
 	 	 	 	 	 	 	 	 	 	 */
#define W25X40_CMD_FASTRD		0x0B	/* fast read data:
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A16, A15-A8, A7-A0, DUMMY, D7-D0, ...
 	 	 	 	 	 	 	 	 	 	 */
#define W25X40_CMD_FASTRD_DO	0x3B	/* fast read data, dual output:
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A16, A15-A8, A7-A0, DUMMY, D7-D0, ...
 	 	 	 	 	 	 	 	 	 	 */
#define W25X40_CMD_FASTRD_DIO	0xBB	/* fast read data, dual I/O:
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A8, A7-A0-M7-M0, D7-D0, ...
 	 	 	 	 	 	 	 	 	 	 */
#define W25X40_CMD_PP			0x02	/* page program, upto 256 bytes
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A16, A15-A8, A7-A0, DUMMY, D7-D0, ...
 	 	 	 	 	 	 	 	 	 	 */
#define W25X40_CMD_SE4K			0x20	/* sector erase, 4KB
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A16, A15-A8, A7-A0
 	 	 	 	 	 	 	 	 	 	 */
#define W25X40_CMD_BE32K		0x52	/* block erase, 32KB
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A16, A15-A8, A7-A0
 	 	 	 	 	 	 	 	 	 	 */
#define W25X40_CMD_BE64K		0xD8	/* block erase, 64KB
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A16, A15-A8, A7-A0
 	 	 	 	 	 	 	 	 	 	 */
#define W25X40_CMD_CE			0xC7	/* chip erase */
#define W25X40_CMD_CE_1			0x60	/* chip erase */
#define W25X40_CMD_PD			0xB9	/* power down */
#define W25X40_CMD_REPD_ID		0xAB	/* release from power down & get device id
 	 	 	 	 	 	 	 	 	 	 * CMD, DUMMY, DUMMY, DUMMY, ID7-ID0
 	 	 	 	 	 	 	 	 	 	 */
#define W25X40_CMD_RDMDID		0x90	/* read manufacturer / device ID
 	 	 	 	 	 	 	 	 	 	 * CMD, DUMMY, DUMMY, 0x00, M7-M0, ID7-ID0
 	 	 	 	 	 	 	 	 	 	 */
#define W25X40_CMD_RDMDID_DIO	0x92	/* read manufacturer / device ID by Dual I/O
 	 	 	 	 	 	 	 	 	 	 * CMD, A23-A8, A7-A0-M7-M0, MF7-MF0-ID7-ID0
 	 	 	 	 	 	 	 	 	 	 */
#define W25X40_CMD_RDJEDECID	0x9F	/* read ID: manufactor ID, memory type, capactity
 	 	 	 	 	 	 	 	 	 	 * CMD, M7-M0, ID15-ID8, ID7-ID0
 	 	 	 	 	 	 	 	 	 	 */
#define W25X40_CMD_RDUNIQEID	0x4B	/* read unique ID
 	 	 	 	 	 	 	 	 	 	 * CMD, DUMMY, DUMMY, DUMMY, DUMMY, ID63-ID0
 	 	 	 	 	 	 	 	 	 	 */

/*
 * w25x40 status register bits
 */
typedef enum
{
	w25x_sr_srp			= bit(7),		// status register protect
	w25x_sr_tb 			= bit(5),		// top/bottom write protect
	w25x_sr_bp_2 		= bit(4),
	w25x_sr_bp_1 		= bit(3),
	w25x_sr_bp_0 		= bit(2),
	w25x_sr_bp_mask 	= 0x1C,			// block protect bits
	w25x_sr_bp_shift	= 2,
	w25x_sr_wel			= bit(1),		// write enable latch
	w25x_sr_busy		= bit(0),		// erase or write in progress
};

/*
 * w25x40 write protect table
 */
typedef struct tag_w25x40_bp
{
	unsigned char sr_tb;
	unsigned char sr_bp;
	unsigned int  addr_begin;
	unsigned int  addr_end;
} w25x40_bp_t;

/*
 * hardware information
 */
#define W25X40_BAUDRATE			50000000		/* 10M baudrate */
#define W25X40_PAGE_SIZE		256				/* page size 256 bytes */
#define W25X40_SECTOR_SIZE_4	0x1000			/* sector size 4K */
#define W25X40_BLOCK_SIZE_32	0x8000			/* block size 32K */
#define W25X40_BLOCK_SIZE_64	0x10000			/* block size 64K */
#define W25X40_CHIP_SIZE		0x80000			/* chip size 512K */

#define W25X40_BITSPERCHAR		8

/*
 * chip parameter, driver use
 */
typedef struct
{
    unsigned int baudrate;				/* tfr rate, bits per second     */
    bool     	 erase_before_program;
    unsigned int empty_state;			/* value of erased cells         */
    unsigned int page_size;				/* programming page size in byte */
    unsigned int sector_size;			/* erase sector size in byte     */
    unsigned int mem_size;				/* total capacity in byte        */
} w25x40_param_t;

#ifdef __cplusplus
}
#endif

#endif /* _W25X40_HW_H_ */

