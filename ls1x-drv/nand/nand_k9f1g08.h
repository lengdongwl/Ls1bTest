/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * nand_flash_chip.h
 *
 *  Created on: 2013-11-20
 *      Author: Bian
 */

#ifndef _K9F1G08U0X_H
#define _K9F1G08U0X_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 * ATO AFND1G08U3
 *
 * 							min     typ     max
 * Program Time				 -      200 	700 	us
 * Block Erase Time 		 -      2 	    3		ms
 *
 * Data Transfer from Cell to Register			25		 us
 * Device Resetting Time(Read/Program/Erase)	5/10/500 us
 *
 */
 
/******************************************************************************
 * SAMSUNG K9F1G08U0C
 *
 * 							min     typ     max
 * Program Time				 -      200 	700 	us
 * Block Erase Time 		 -      1.5 	10		ms
 *
 * Data Transfer from Cell to Register			25		 us
 * Device Resetting Time(Read/Program/Erase)	5/10/500 us
 *
 */

enum
{
	K9F1G08_sr_writeprotect	= (1<<7),			// =0: protect
	K9F1G08_sr_ready_busy	= (1<<6),			// =0: busy, =1: ready
	K9F1G08_sr_pass_fail	= (1<<0),			// =0: pass, =1: fail
} K9F1G08_info;

/*
 * k9f1g08 HW info
 */
#define BYTES_OF_PAGE			0x800			// 2048 Bytes
#define PAGES_OF_BLOCK			0x40			// 64
#define BLOCKS_OF_CHIP			0x400			// 1024
#define OOBBYTES_OF_PAGE		0x40			// 64 Bytes

#define PAGES_OF_CHIP			(PAGES_OF_BLOCK*BLOCKS_OF_CHIP)
#define BYTES_OF_CHIP			0x08000000		// 128M Bytes

#define BLOCK_TO_PAGE(x)		((x)<<6)		// first page in block
#define PAGE_TO_BLOCK(x)		((x)>>6)		// block of page

/*
 * XXX 注意: 1A/1B/1C 的NAND控制器是不一样的
 */
#define PAGE_SHIFT              12              /* 页内地址(列地址)A0-A11 */

#if defined(LS1B)
#define MAIN_ADDRH(page)		((page)>>(32-(PAGE_SHIFT-1)))
#define MAIN_ADDRL(page)		((page)<<(PAGE_SHIFT-1))        /* 不访问spare区时A11无效 */
#define MAIN_SPARE_ADDRH(page)	((page)>>(32-PAGE_SHIFT))
#define MAIN_SPARE_ADDRL(page)	((page)<<PAGE_SHIFT)
#elif defined(LS1C)
#define MAIN_ADDRH(page)		(page)
#define MAIN_ADDRL(page)		((page)<<PAGE_SHIFT)
#define MAIN_SPARE_ADDRH(page)	(page)
#define MAIN_SPARE_ADDRL(page)	((page)<<PAGE_SHIFT)
#else
#error "No Loongson1x SoC defined."
#endif

/*
 * Function 			 | 1st Cycle | 2nd Cycle
 * Read 				 | 00h 		 | 30h
 * Read for Copy Back 	 | 00h 		 | 35h
 * Read ID 				 | 90h 		 | -
 * Reset 				 | FFh 		 | -
 * Page Program 		 | 80h 		 | 10h
 * Copy-Back Program 	 | 85h 		 | 10h
 * Block Erase 			 | 60h 		 | D0h
 * Random Data Input(1)  | 85h 		 | -
 * Random Data Output(1) | 05h 		 | E0h
 * Read Status 			 | 70h 		 | -
 */

/*
 * K9F1G08U0x flash commands
 */
#define NAND_CMD_READ1				0x00
#define NAND_CMD_READ2				0x30
#define NAND_CMD_READ_COPYBACK1		0x00
#define NAND_CMD_READ_COPYBACK2		0x35
#define NAND_CMD_READID				0x90
#define NAND_CMD_RESET				0xFF
#define NAND_CMD_PAGEPROG1			0x80
#define NAND_CMD_PAGEPROG2			0x10
#define NAND_CMD_COPYBACK_PROG1		0x85
#define NAND_CMD_COPYBACK_PROG2		0x10
#define NAND_CMD_ERASE1				0x60
#define NAND_CMD_ERASE2				0xD0
#define NAND_CMD_RANDOM_IN			0x85
#define NAND_CMD_RANDOM_OUT1		0x05
#define NAND_CMD_RANDOM_OUT2		0xE0
#define NAND_CMD_STATUS				0x70

#ifdef __cplusplus
}
#endif

#endif /* _K9F1G08U0X_H */

