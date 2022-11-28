/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_gmac.c
 *
 * Loongson1x GMAC driver
 * GMAC work with MII, support 10/100M, fullduplex/halfduplex mode.
 *
 * created: 2013/07/01
 *  author: Bian
 *
 */

#include "bsp.h"

#if defined(BSP_USE_GMAC0) || defined(BSP_USE_GMAC1)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#if defined(LS1B)
#include "ls1b.h"
#include "ls1b_irq.h"
#elif defined(LS1C)
#include "ls1c.h"
#include "ls1c_irq.h"
#else
#error "No Loongson1x SoC defined!"
#endif

#if defined(OS_RTTHREAD)
#include "rtthread.h"
#elif defined(OS_UCOS)
#include "os.h"
#elif defined(OS_FREERTOS)
#include "FreeRTOS.h"
#include "event_groups.h"
#endif

#include "cpu.h"
#include "mii.h"

#include "ls1x_io.h"
#include "drv_os_priority.h"

#include "ls1x_gmac_hw.h"
#include "ls1x_gmac.h"

//***************************************************************************************

#define GMAC_DEBUG              0               // 调试
#define GMAC_TRACE_INFO         1               // print some message

//***************************************************************************************

#define LS1x_K0_CACHED          1

#ifdef LS1x_K0_CACHED
#define VA_TO_PHYS(x)			K1_TO_PHYS(x)
#else
#define VA_TO_PHYS(x)			K0_TO_PHYS(x)
#endif

#define GMAC_BUF_CACHED		    1			    // buffer cached

#ifndef ETHER_MAX_LEN
#define ETHER_MAX_LEN           1518            // should defined by tcp/ip stack
#endif
#define MAX_BUF_SIZE			1536			// 48*32, reference by ETHER_MAX_LEN=1518

#define DMA_DESC_SIZE           32
#define NUM_TX_DMA_DESC 		4				// TX 描述符个数, 和 TX 缓冲区个数一致
#define NUM_RX_DMA_DESC 		4				// RX 描述符个数, 和 RX 缓冲区个数一致

#define TX_BUF_SIZE				MAX_BUF_SIZE	// TX 缓冲区大小
#define RX_BUF_SIZE				MAX_BUF_SIZE	// RX 缓冲区大小

#define MII_WR_TIMEOUT          0x0200  		// mii  write timeout
#define MII_RD_TIMEOUT          0x0200  		// mii  read  timeout
#define GDMA_RESET_TIMEOUT      0x0200  		// gdma reset timeout

//***************************************************************************************
// Hardware-specific storage, 驱动专用硬件相关的控制结构
//***************************************************************************************

typedef struct
{
	LS1x_gmac_regs_t   *hwGMAC;                     /* GMAC Hardware */
	LS1x_gdma_regs_t   *hwGDMA;
	unsigned int		phyAddr;					/* PHY Address */
	unsigned int		gmacVersion;				/* 设备版本号 */

    unsigned int        vector;	                    /* GMAC Interrupt Registers */
	unsigned int		int_ctrlr;
	unsigned int		int_mask;

	int					unitNumber;					/* 设备号 */
    int           		acceptBroadcast;			/* Indicates configuration */
    int                 autoNegotiation;            /* 是否自动协商 */
    int                 autoNegoTimeout;            /* 自动协商超时 ms */

	unsigned int		LinkState;					/* Link status as reported by the Phy */
	unsigned int		DuplexMode;					/* Duplex mode of the Phy */
	unsigned int		Speed;						/* Speed of the Phy */

	unsigned int		descmode;					/* DMA Desc Mode  chain or ring */
	LS1x_rxdesc_t	   *rx_desc[NUM_RX_DMA_DESC];	/* RX 描述符指针数组 */
	LS1x_txdesc_t	   *tx_desc[NUM_TX_DMA_DESC];	/* TX 描述符指针数组 */
	int					rx_head;
	int					rx_tail;
	int					tx_head;
	int					tx_tail;

	unsigned char	   *rx_buf[NUM_RX_DMA_DESC];	/* RX 缓冲区指针数组 */
	unsigned char	   *tx_buf[NUM_TX_DMA_DESC];	/* TX 缓冲区指针数组 */

#if defined(OS_RTTHREAD)
	rt_event_t          gmac_event;
#elif defined(OS_UCOS)
	OS_FLAG_GRP        *gmac_event;
#elif defined(OS_FREERTOS)
	EventGroupHandle_t  gmac_event;
#endif

	unsigned int		interrupts;	            /* Statistics 统计数 */
	unsigned int		dma_normal_intrs;
	unsigned int		dma_abnormal_intrs;
	unsigned int		dma_fatal_err;
	unsigned int		rx_interrupts;
	unsigned int		tx_interrupts;

	unsigned int		rx_pkts;
	unsigned int		rx_buffer_unavailable;
	unsigned int		rx_stopped;
	unsigned int		rx_errors;
	unsigned int		rx_length_err;
	unsigned int		rx_dribit_err;
	unsigned int		rx_dropped;

	unsigned int		tx_pkts;
	unsigned int		tx_buffer_unavailable;
	unsigned int		tx_stopped;
	unsigned int		tx_errors;
	unsigned int		tx_ipheader_err;
	unsigned int		tx_playload_err;
	unsigned int		tx_defered;
	unsigned int		tx_collsions;
	unsigned int		tx_underflow;

    int                 timeout;                /* 收发超时 */
	int					initialized;		    /* 是否初始化 */
	int                 started;		        /* 是否启动 */
    char                dev_name[16];		    /* 设备名称 */
} GMAC_t;

//***************************************************************************************
// ls1x gmac device and buffer
//***************************************************************************************

#define __ALIGN(x)      __attribute__((aligned(x)))

#if defined(BSP_USE_GMAC0)
static GMAC_t ls1x_GMAC0 =
{
  #if defined(LS1B)
    .hwGMAC      = (LS1x_gmac_regs_t *)LS1B_GMAC0_BASE,
    .hwGDMA      = (LS1x_gdma_regs_t *)LS1x_GDMA0_BASE,
    .int_ctrlr   = LS1B_INTC1_BASE,
    .int_mask    = INTC1_GMAC0_BIT,
    .vector      = LS1B_GMAC0_IRQ,
  #elif defined(LS1C)
    .hwGMAC      = (LS1x_gmac_regs_t *)LS1C_MAC_BASE,
    .hwGDMA      = (LS1x_gdma_regs_t *)LS1x_GDMA0_BASE,
    .int_ctrlr   = LS1C_INTC1_BASE,
    .int_mask    = INTC1_MAC_BIT,
    .vector      = LS1C_MAC_IRQ,
  #endif
    .unitNumber  = 0,
	.descmode    = CHAINMODE,       // XXX This is important
    .timeout     = 0,
    .started     = 0,
    .initialized = 0,
    .dev_name    = "gmac0",
	.acceptBroadcast = 0,
    .autoNegotiation = 1,
    .autoNegoTimeout = 3000,
};

void *devGMAC0 = (void *)&ls1x_GMAC0;

static unsigned char tx_desc_0[(NUM_TX_DMA_DESC*DMA_DESC_SIZE)] __ALIGN(32);
static unsigned char rx_desc_0[(NUM_RX_DMA_DESC*DMA_DESC_SIZE)] __ALIGN(32);
static unsigned char tx_buf_0[(NUM_TX_DMA_DESC*TX_BUF_SIZE)] __ALIGN(32);
static unsigned char rx_buf_0[(NUM_RX_DMA_DESC*RX_BUF_SIZE)] __ALIGN(32);
#endif

#if defined(BSP_USE_GMAC1)
static GMAC_t ls1x_GMAC1 =
{
    .hwGMAC      = (LS1x_gmac_regs_t *)LS1B_GMAC1_BASE,
    .hwGDMA      = (LS1x_gdma_regs_t *)LS1B_GDMA1_BASE,
    .int_ctrlr   = LS1B_INTC1_BASE,
    .int_mask    = INTC1_GMAC1_BIT,
    .vector      = LS1B_GMAC1_IRQ,
    .unitNumber  = 1,
	.descmode    = CHAINMODE,       // XXX This is important
    .timeout     = 0,
    .started     = 0,
    .initialized = 0,
    .dev_name    = "gmac1",
	.acceptBroadcast = 0,
    .autoNegotiation = 1,
    .autoNegoTimeout = 3000,
};

void *devGMAC1 = (void *)&ls1x_GMAC1;

static unsigned char tx_desc_1[(NUM_TX_DMA_DESC*DMA_DESC_SIZE)] __ALIGN(32);
static unsigned char rx_desc_1[(NUM_RX_DMA_DESC*DMA_DESC_SIZE)] __ALIGN(32);
static unsigned char tx_buf_1[(NUM_TX_DMA_DESC*TX_BUF_SIZE)] __ALIGN(32);
static unsigned char rx_buf_1[(NUM_RX_DMA_DESC*RX_BUF_SIZE)] __ALIGN(32);
#endif

//***************************************************************************************
// function prototypes
//***************************************************************************************

static int LS1x_init_tx_desc_queue(GMAC_t *pMAC);
static int LS1x_init_rx_desc_queue(GMAC_t *pMAC);

static void LS1x_GMAC_init_hw(GMAC_t *pMAC, unsigned char *macAddr);
static void LS1x_GMAC_irq_handler(int vector, void *arg);
static void LS1x_GMAC_do_reset(GMAC_t *pMAC);

//***************************************************************************************
// functions for gdma desc
//***************************************************************************************

static inline void GDMA_INIT_RXDESC(GMAC_t *pMAC)
{
	LS1x_init_rx_desc_queue(pMAC);
	pMAC->rx_head = 0;
	pMAC->rx_tail = 0;
}

static inline void GDMA_INIT_TXDESC(GMAC_t *pMAC)
{
	LS1x_init_tx_desc_queue(pMAC);
	pMAC->tx_head = 0;
	pMAC->tx_tail = 0;
}

static inline void GDMA_INIT_DESC(GMAC_t *pMAC)
{
	GDMA_INIT_RXDESC(pMAC);
	GDMA_INIT_TXDESC(pMAC);
}

static inline void GDMA_INIT_RXDESC_CUR(GMAC_t *pMAC)
{
	int i; unsigned int val;
	val = pMAC->hwGDMA->currxdesc;
	for (i=0; i<NUM_RX_DMA_DESC; i++)
	{
		if (val == VA_TO_PHYS((unsigned int)pMAC->rx_desc[i]))
		{
			pMAC->rx_head = i;
			pMAC->rx_tail = i;
			break;
		}
	}
}

static inline void GDMA_INIT_TXDESC_CUR(GMAC_t *pMAC)
{
	int i; unsigned int val;
	val = pMAC->hwGDMA->curtxdesc;
	for (i=0; i<NUM_TX_DMA_DESC; i++)
	{
		if (val == VA_TO_PHYS((unsigned int)pMAC->tx_desc[i]))
		{
			pMAC->tx_head = i;
			pMAC->tx_tail = i;
			break;
		}
	}
}

static inline void GDMA_INIT_DESC_CUR(GMAC_t *pMAC)
{
	GDMA_INIT_RXDESC_CUR(pMAC);
	GDMA_INIT_TXDESC_CUR(pMAC);
}

//***************************************************************************************
// MII r/w interface
//***************************************************************************************

/*
 * detect phy addr
 */
static unsigned short MII_detect_phy_addr(GMAC_t *pMAC)
{
    unsigned char i;

    for (i=0; i<32; i++)
    {
        unsigned int phyVal, j;

        phyVal = gmac_miictrl_csr_3 |		        // bits: 4-2,   CSR Clock Range
                 ((i & 0x1F) << 11) |		        // bits: 15-11, PHY Address
    		     ((MII_PHYIDR1 & 0x1F) << 6) |      // bits: 10-6,  MII Register
    		     gmac_miictrl_busy;			        // bit:  0,     MII is Busy

        pMAC->hwGMAC->miictrl = phyVal;
        ls1x_sync();

        /* wait for it to complete, become unbusy
         */
        for (j=0; j<MII_RD_TIMEOUT; j++)
            if (!(pMAC->hwGMAC->miictrl & gmac_miictrl_busy))
    		    break;

    	phyVal = pMAC->hwGMAC->miidata & gmac_miidata_mask;

        /*
         * DM9161AEP PHYID1
         */
        if ((phyVal != 0) && (phyVal != 0xFFFF)) // (phyVal == 0x0181) //
        {
            printk("detect phy address == %i\n", i);
            return (unsigned short)i;
        }
    }

    return pMAC->phyAddr;
}

static void MII_write_phy(GMAC_t *pMAC, unsigned char phyReg, unsigned short val)
{
	int i;
    unsigned int phyVal;

    phyVal = gmac_miictrl_csr_3 |					// bits: 4-2,   CSR Clock Range
             ((pMAC->phyAddr & 0x1F) << 11) |		// bits: 15-11, PHY Address
    		 ((phyReg  & 0x1F) << 6)  |				// bits: 10-6,  MII Register
    		 gmac_miictrl_wr |					    // bit:  1,     MII is Writting
    		 gmac_miictrl_busy;						// bit:  0,     MII is Busy

    pMAC->hwGMAC->miidata = (unsigned int)val & gmac_miidata_mask;
    pMAC->hwGMAC->miictrl = phyVal;
    ls1x_sync();

    /* wait for it to complete, become unbusy
     */
    for (i=0; i<MII_WR_TIMEOUT; i++)
    	if (!(pMAC->hwGMAC->miictrl & gmac_miictrl_busy))
    		break;

    if (i>=MII_WR_TIMEOUT)
    	printk("Error: mii write phy timeout.\n");
}

static void MII_read_phy(GMAC_t *pMAC, unsigned char phyReg, unsigned short *val)
{
	int i;
    unsigned int phyVal;

    phyVal = gmac_miictrl_csr_3 |					// bits: 4-2,   CSR Clock Range
             ((pMAC->phyAddr & 0x1F) << 11) |		// bits: 15-11, PHY Address
    		 ((phyReg  & 0x1F) << 6) |				// bits: 10-6,  MII Register
    		 gmac_miictrl_busy;					    // bit:  0,     MII is Busy

    pMAC->hwGMAC->miictrl = phyVal;
    ls1x_sync();

    /* wait for it to complete, become unbusy
     */
    for (i=0; i<MII_RD_TIMEOUT; i++)
    	if (!(pMAC->hwGMAC->miictrl & gmac_miictrl_busy))
    		break;

    if (i<MII_RD_TIMEOUT)
    {
    	phyVal = pMAC->hwGMAC->miidata & gmac_miidata_mask;
        *val = (unsigned short)phyVal;
    }
    else
    {
    	printk("Error: mii read phy timeout.\n");
    	*val = 0;
    }
}

static int MII_get_phy_link_mode(GMAC_t *pMAC, int *eSpeed, int *eDuplex, int *eLink)
{
    int rt;
	unsigned short ctrlVal, srVal;

	MII_read_phy(pMAC, MII_BMCR, &ctrlVal);		/* Read the phy control */
	MII_read_phy(pMAC, MII_BMSR, &srVal);		/* Read the phy status */

	pMAC->autoNegotiation = ctrlVal & BMCR_AUTOEN;

	if (pMAC->autoNegotiation)		/* autonegotiation mode */
	{
		/* Autonegotiation complete and Linkup
		 */
		if (srVal & BMSR_ACOMP)
		{
			unsigned short anVal;

			/* Read the phy Autonegotiation link partner abilities
			 */
			MII_read_phy(pMAC, MII_ANLPAR, &anVal);

			*eSpeed  = ((anVal & ANLPAR_TX_FD) || (anVal & ANLPAR_TX)) ? SPEED100 :
					   ((anVal & ANLPAR_10_FD) || (anVal & ANLPAR_10)) ? SPEED10 : SPEED100;
			*eDuplex = ((anVal & ANLPAR_TX_FD) || (anVal & ANLPAR_10_FD)) ? FULLDUPLEX :
					   ((anVal & ANLPAR_TX) || (anVal & ANLPAR_10)) ? HALFDUPLEX : FULLDUPLEX;
			*eLink   = (srVal & BMSR_LINK) ? LINKUP : LINKDOWN;
		}
		else
		{
			*eSpeed  = SPEED100;
			*eDuplex = FULLDUPLEX;
			*eLink   = LINKDOWN;
		}
	}
	else				/* special link mode */
	{
		*eSpeed  = ((ctrlVal & BMCR_SPEED(BMCR_S100)) == BMCR_SPEED(BMCR_S100)) ? SPEED100 :
				   ((ctrlVal & BMCR_SPEED(BMCR_S10)) == BMCR_SPEED(BMCR_S10)) ? SPEED10 : SPEED100;
		*eDuplex = (ctrlVal & BMCR_FDX) ? FULLDUPLEX : HALFDUPLEX;
		*eLink   = (srVal & BMSR_LINK) ? LINKUP : LINKDOWN;
	}

    rt = (srVal & BMSR_RFAULT) ? -1 : 0;

	if (rt == 0)
	{
		pMAC->Speed      = *eSpeed;
		pMAC->DuplexMode = *eDuplex;
	}
	else				/* default */
	{
		pMAC->Speed      = SPEED100;
		pMAC->DuplexMode = FULLDUPLEX;
	}
	pMAC->LinkState = *eLink;

	return rt;
}

static void MII_set_phy_link_mode(GMAC_t *pMAC, int eSpeed, int eDuplex, int ePowerDown)
{
	unsigned short ctrlVal;

	if (pMAC->autoNegotiation)
	{
		unsigned short anarVal;

		anarVal = ANAR_FC | ANAR_TX_FD | ANAR_TX | ANAR_10_FD | ANAR_10 | ANAR_CSMA;
		MII_write_phy(pMAC, MII_ANAR, anarVal);     /* set advertise register */

		ctrlVal = BMCR_AUTOEN | BMCR_STARTNEG;      /* set restart autonegotiation */
        if (ePowerDown) ctrlVal |= BMCR_PDOWN;      /* set power saving mode */
        MII_write_phy(pMAC, MII_BMCR, ctrlVal);     /* write control register to do autonegotiation */

	    /* wait for autonegotiation done, total delay 3000ms
	     */
        if (!ePowerDown)
		{
	        int wait_ticks = 0;
		    unsigned short srVal;

            while (wait_ticks <= pMAC->autoNegoTimeout)
		    {
		    	MII_read_phy(pMAC, MII_BMSR, &srVal);
		    	if (srVal & BMSR_ACOMP)
		    		break;

                delay_ms(100);
                wait_ticks += 100;
		    }

		    if (wait_ticks > pMAC->autoNegoTimeout)
		    {
		    	printk("Error: phy autonegotiation timeout.\n");
		    }
		}

		/* TODO else - timer check autonegotiation done */
	}
	else    /* has linked yet */
	{
		ctrlVal = (eSpeed == SPEED100) ? BMCR_SPEED(BMCR_S100) :
				  (eSpeed == SPEED10) ? BMCR_SPEED(BMCR_S10) : BMCR_SPEED(BMCR_S100);

		if (eDuplex) ctrlVal |= BMCR_FDX;		    /* set duplex mode */
		if (ePowerDown) ctrlVal |= BMCR_PDOWN;	    /* set power saving mode */
		MII_write_phy(pMAC, MII_BMCR, ctrlVal);		/* write control register to effect */
	}
}

static inline void MII_GET_PHY_LINKED(GMAC_t *pMAC, int *linked)
{
	unsigned short srVal;
	MII_read_phy(pMAC, MII_BMSR, &srVal);
	*linked = (srVal & BMSR_LINK) ? LINKUP : LINKDOWN;
}

/******************************************************************************
 * full check the ethernet link is changed.
 */
static int MII_check_link_change(GMAC_t *pMAC)
{
	int eDuplex, eSpeed, eLink;

	if ((NULL != pMAC) && pMAC->initialized &&
		(MII_get_phy_link_mode(pMAC, &eDuplex, &eSpeed, &eLink) == 0))
	{
#if (GMAC_TRACE_INFO)
		if ((pMAC->Speed != eSpeed) || (pMAC->DuplexMode != eDuplex) || (pMAC->LinkState != eLink))
		{
			printk("GMAC%i: SPEED=%iM, DUPLEX=%s, LINK=%s. AN=%s.\r\n", pMAC->unitNumber,
					(eSpeed == SPEED100) ? 100 : 10,
					(eDuplex == FULLDUPLEX) ? "FULL" : "HALF",
					(eLink == LINKUP) ? "UP" : "DOWN",
                     pMAC->autoNegotiation ? "YES" : "NO");
		}
#endif

		if ((pMAC->Speed != eSpeed) || (pMAC->DuplexMode != eDuplex))
		{
            LS1x_GMAC_do_reset(pMAC);
		}

		if (pMAC->LinkState != eLink)
		{
			pMAC->LinkState = eLink;
			return 1;
		}
	}

	return 0;
}

//***************************************************************************************
// set gmac work mode
//***************************************************************************************

static void LS1x_GMAC_set_workmode(GMAC_t *pMAC)
{
	unsigned int val;

	if (pMAC->DuplexMode == FULLDUPLEX)
	{
		val = pMAC->hwGMAC->control;
		val &= ~(gmac_ctrl_wd		|			// Enable Watch Dog
				 gmac_ctrl_je		|			// Disable Jumbo Frame
				 gmac_ctrl_do		|			// Enable Receive Own
				 gmac_ctrl_lm		|			// Disable Loopback mode
				 gmac_ctrl_dr		|			// Enable Retry
				 gmac_ctrl_acs		|			// Disable Pad/CRC Stripping
				 gmac_ctrl_bl_mask	|			// Set Back-Off Limit 0
				 gmac_ctrl_dc);					// Disable Deferral Check
		val |= (gmac_ctrl_jd		|			// Disable the Jabber Timer
				gmac_ctrl_be		|			// Enable Frame Burst
				gmac_ctrl_dm		|			// Set Full Duplex Mode
				gmac_ctrl_bl_0		|			//
				gmac_ctrl_te		|			// Enable Transmitter
				gmac_ctrl_re);					// Enable Receiver
		pMAC->hwGMAC->control = val;
		ls1x_sync();

		val = pMAC->hwGDMA->control;
		val |= (gdma_ctrl_rxsf	|				// Receive Store and Forward
				gdma_ctrl_txsf	|				// Transmit Store and Forward
				gdma_ctrl_ferrf);				// Forward Error Frames
		pMAC->hwGDMA->control = val;
		ls1x_sync();

		val = pMAC->hwGMAC->control;
		if (pMAC->Speed == SPEED1000)
		{
			val &= ~gmac_ctrl_ps;				// Port Select, 0: GMII (1000Mbps)
		}
		else
		{
			val |= gmac_ctrl_ps;				// Port Select, 1: MII (10/100Mbps)
			if (pMAC->Speed == SPEED100)
				val |= gmac_ctrl_fes; 			// Speed, 1: 100Mbps
			else
				val &= ~gmac_ctrl_fes; 			// Speed, 0: 10Mbps
		}
		pMAC->hwGMAC->control = val;

		val = pMAC->hwGMAC->framefilter;
		val &= ~(gmac_frmfilter_pcf_mask |		// Pass Control Frames, 00: GMAC过滤所有控制帧
				 gmac_frmfilter_dbf		 |		// Enable Broadcast Frames, 0:接收所有广播帧
				 gmac_frmfilter_saf		 |		// Disable Source Address Filter
				 gmac_frmfilter_pm		 |		// Pass All Multicast, 0:过滤所有多播帧
				 gmac_frmfilter_daif	 |		// DA Inverse Filtering, 0:对单播和多播帧进行正常目标地址匹配
				 gmac_frmfilter_hmc		 |		// Disable Hash Multicast
				 gmac_frmfilter_huc);			// Disable Hash Unicast
		val |= (gmac_frmfilter_ra		 |		// Disable Frame Filter
				gmac_frmfilter_pcf_0	 |		//
				gmac_frmfilter_pr);				// Enable Promiscuous Mode, 混杂模式, 接收所有以太网帧
		pMAC->hwGMAC->framefilter = val;

		val = pMAC->hwGMAC->flowctrl;
		val &= ~gmac_flowctrl_up;				// Disable Unicast Pause Frame Detect
		val |= (gmac_flowctrl_rxfcen	|		// Enable Receive Flow Control
				gmac_flowctrl_txfcen);			// Enable Transmit Flow Control
		pMAC->hwGMAC->flowctrl = val;
	}
	else	// HALFDUPLEX
	{
		val = pMAC->hwGMAC->control;
		val &= ~(gmac_ctrl_wd		|			// Enable Watch Dog
				 gmac_ctrl_jd		|			// Enable the Jabber frame
				 gmac_ctrl_je		|			// Disable Jumbo Frame
				 gmac_ctrl_do		|			// Enable Receive Own
				 gmac_ctrl_lm		|			// Disable Loopback mode
				 gmac_ctrl_dm		|			// Set Half Duplex Mode
				 gmac_ctrl_dr		|			// Enable Retry
				 gmac_ctrl_acs		|			// Disable Pad/CRC Stripping
				 gmac_ctrl_bl_mask	|			// Set Back-Off Limit 0
				 gmac_ctrl_dc);					// Disable Deferral Check
		val |= (gmac_ctrl_be	|				// Enable Frame Burst
				gmac_ctrl_bl_0);				//
		pMAC->hwGMAC->control = val;
		ls1x_sync();

		val = pMAC->hwGDMA->control;
		val |= (gdma_ctrl_rxsf	|				// Receive Store and Forward
				gdma_ctrl_txsf);				// Transmit Store and Forward
		pMAC->hwGDMA->control = val;
		ls1x_sync();

		val = pMAC->hwGMAC->control;
		val |= (gmac_ctrl_te	|				// Enable Transmitter
				gmac_ctrl_re);					// Receiver Enable
		if (pMAC->Speed == SPEED1000)
		{
			val &= ~gmac_ctrl_ps;				// Port Select, 0: GMII (1000Mbps)
		}
		else
		{
			val |= gmac_ctrl_ps;				// Port Select, 1: MII (10/100Mbps)
			if (pMAC->Speed == SPEED100)
				val |= gmac_ctrl_fes; 			// Speed, 1: 100Mbps
			else
				val &= ~gmac_ctrl_fes; 			// Speed, 0: 10Mbps
		}
		pMAC->hwGMAC->control = val;

		val = pMAC->hwGMAC->framefilter;
		val &= ~(gmac_frmfilter_ra	     |		// Enable Frame Filter
				 gmac_frmfilter_pcf_mask |		// Pass Control Frames, 00: GMAC过滤所有控制帧
				 gmac_frmfilter_dbf	     | 		// Enable Broadcast Frames, 0:接收所有广播帧
				 gmac_frmfilter_saf	     |		// Disable Source Address Filter
				 gmac_frmfilter_pm	     |		// Pass All Multicast, 0:过滤所有多播帧
				 gmac_frmfilter_daif	 |		// DA Inverse Filtering, 0:对单播和多播帧进行正常目标地址匹配
				 gmac_frmfilter_hmc	     |		// Disable Hash Multicast
				 gmac_frmfilter_huc	     |		// Disable Hash Unicast
				 gmac_frmfilter_pr);			// Disble Promiscuous Mode, 混杂模式, 接收所有以太网帧
		val |= gmac_frmfilter_pcf_0;			//
		pMAC->hwGMAC->framefilter = val;

		val = pMAC->hwGMAC->flowctrl;
		val &= ~(gmac_flowctrl_up	  |			// Disable Unicast Pause Frame Detect
				 gmac_flowctrl_rxfcen |			// Disable Receive Flow Control
				 gmac_flowctrl_txfcen);			// Disable Transmit Flow Control
		pMAC->hwGMAC->flowctrl = val;
	}
}

//***************************************************************************************

#if (GMAC_DEBUG)
static void LS1x_GMAC_dump_registers(GMAC_t *pMAC)
{
    DBG_OUT("GMAC registers:\r\n");
    DBG_OUT("Configuration           = 0x%08X\r\n", pMAC->hwGMAC->control);		/* 0xBFE10000  */
    DBG_OUT("GMAC Frame Filter       = 0x%08X\r\n", pMAC->hwGMAC->framefilter);	/* 0xBFE10004  */
//  DBG_OUT("Hash Table High         = 0x%08X\r\n", pMAC->hwGMAC->hashhi);		/* 0xBFE10008  */
//  DBG_OUT("Hash Table Low          = 0x%08X\r\n", pMAC->hwGMAC->hashlo);		/* 0xBFE1000C  */
//  DBG_OUT("GMII Address            = 0x%08X\r\n", pMAC->hwGMAC->miictrl);		/* 0xBFE10010  */
//  DBG_OUT("GMII Data               = 0x%08X\r\n", pMAC->hwGMAC->miidata);		/* 0xBFE10014  */
    DBG_OUT("Flow Control            = 0x%08X\r\n", pMAC->hwGMAC->flowctrl);		/* 0xBFE10018  */
//  DBG_OUT("VLAN Tag                = 0x%08X\r\n", pMAC->hwGMAC->vlantag);		/* 0xBFE1001C  */
    DBG_OUT("Version                 = 0x%08X\r\n", pMAC->hwGMAC->version);		/* 0xBFE10020  */
    DBG_OUT("Interrupt Status        = 0x%08X\r\n", pMAC->hwGMAC->intstatus);	/* 0xBFE10038  */
    DBG_OUT("Interrupt Mask          = 0x%08X\r\n", pMAC->hwGMAC->intmask);		/* 0xBFE1003C  */
//  DBG_OUT("Address0 High           = 0x%08X\r\n", pMAC->hwGMAC->addr0hi);		/* 0xBFE10040  */
//  DBG_OUT("Address0 Low            = 0x%08X\r\n", pMAC->hwGMAC->addr0lo);		/* 0xBFE10044  */
//  DBG_OUT("Address1 High           = 0x%08X\r\n", pMAC->hwGMAC->addr1hi);		/* 0xBFE10048  */
//  DBG_OUT("Address1 Low            = 0x%08X\r\n", pMAC->hwGMAC->addr1lo);		/* 0xBFE1004C  */
    DBG_OUT("AN Control              = 0x%08X\r\n", pMAC->hwGMAC->anctrl);		/* 0xBFE100C0  */
    DBG_OUT("AN Status               = 0x%08X\r\n", pMAC->hwGMAC->anstatus);		/* 0xBFE100C4  */
    DBG_OUT("AN Advertisement        = 0x%08X\r\n", pMAC->hwGMAC->anadvertise);	/* 0xBFE100C8  */
    DBG_OUT("AN Link Partner Ability = 0x%08X\r\n", pMAC->hwGMAC->anlinkpa);		/* 0xBFE100CC  */
    DBG_OUT("AN Link Expansion       = 0x%08X\r\n", pMAC->hwGMAC->anexpansion);	/* 0xBFE100D0  */
    DBG_OUT("SGMII/RGMII Status      = 0x%08X\r\n", pMAC->hwGMAC->miistatus);	/* 0xBFE100D8  */

    DBG_OUT("GDMA registers:\r\n");
    DBG_OUT("Bus Mode                = 0x%08X\r\n", pMAC->hwGDMA->busmode); 		/* 0xBFE11000  */
    DBG_OUT("Status                  = 0x%08X\r\n", pMAC->hwGDMA->status); 		/* 0xBFE11014  */
    DBG_OUT("Operation Mode          = 0x%08X\r\n", pMAC->hwGDMA->control); 		/* 0xBFE11018  */
    DBG_OUT("Interrupt Enable        = 0x%08X\r\n", pMAC->hwGDMA->intenable);	/* 0xBFE1101C */
//  DBG_OUT("Transmit Poll Demand    = 0x%08X\r\n", pMAC->hwGDMA->txpoll); 		/* 0xBFE11004  */
//  DBG_OUT("Receive Poll Demand     = 0x%08X\r\n", pMAC->hwGDMA->rxpoll); 		/* 0xBFE11008  */
//  DBG_OUT("Start of Receive Descriptor List Address  = 0x%08X\r\n", pMAC->hwGDMA->rxdesc0); 	/* 0xBFE1100C  */
//  DBG_OUT("Start of Transmit Descriptor List Address = 0x%08X\r\n", pMAC->hwGDMA->txdesc0); 	/* 0xBFE11010  */
//  DBG_OUT("Missed Frame and Buffer Overflow Counter  = 0x%08X\r\n", pMAC->hwGDMA->mfbocount);	/* 0xBFE11020  */
//  DBG_OUT("Current Host Transmit Descriptor          = 0x%08X\r\n", pMAC->hwGDMA->curtxdesc); 	/* 0xBFE11048  */
//  DBG_OUT("Current Host Receive Descriptor           = 0x%08X\r\n", pMAC->hwGDMA->currxdesc); 	/* 0xBFE1104C  */
//  DBG_OUT("Current Host Transmit Buffer Address      = 0x%08X\r\n", pMAC->hwGDMA->curtxbuf); 	/* 0xBFE11050  */
//  DBG_OUT("Current Host Receive Buffer Address       = 0x%08X\r\n", pMAC->hwGDMA->currxbuf); 	/* 0xBFE11054  */

    /* dump DM9161AEP registers */
    if (0)
    {
        unsigned short val;

//        pMAC->phyAddr = 17;  // LS1B==17, LS1C==19

        DBG_OUT("DM9161AEP general registers\r\n");
        MII_read_phy(pMAC, 0, &val);  DBG_OUT("0: control            = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 1, &val);  DBG_OUT("1: status             = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 2, &val);  DBG_OUT("2: phyid1             = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 3, &val);  DBG_OUT("3: phyid2             = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 4, &val);  DBG_OUT("4: auto-neg-advertise = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 5, &val);  DBG_OUT("5: link part ability  = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 6, &val);  DBG_OUT("6: auto-neg-expansion = 0x%04X\r\n", val);
        DBG_OUT("DM9161AEP special registers\r\n");
        MII_read_phy(pMAC, 16, &val);  DBG_OUT("16: specified config = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 17, &val);  DBG_OUT("17: specified status = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 18, &val);  DBG_OUT("18: 10T conf/status  = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 19, &val);  DBG_OUT("19: pwdor            = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 20, &val);  DBG_OUT("20: special config   = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 21, &val);  DBG_OUT("21: mdintr           = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 22, &val);  DBG_OUT("22: rcver            = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 23, &val);  DBG_OUT("23: dis-connect      = 0x%04X\r\n", val);
        MII_read_phy(pMAC, 24, &val);  DBG_OUT("24: rstlh            = 0x%04X\r\n", val);
    }
}
#endif

//***************************************************************************************
// Initialize GMAC Hardware
//***************************************************************************************

static void LS1x_GMAC_init_hw(GMAC_t *pMAC, unsigned char *macAddr)
{
	int i, eSpeed, eDuplex, eLink;
    unsigned int val;

#if defined(LS1B)
  #if defined(BSP_USE_GMAC1)
    if (pMAC == &ls1x_GMAC1)
    {
        OR_REG32(LS1B_MUX_CTRL0_ADDR, MUX_CTRL0_GMAC1_USE_UART1 | MUX_CTRL0_GMAC1_USE_UART0);
	}
  #endif

    val = LS1B_MUX_CTRL1;
    if (pMAC->Speed == SPEED1000)
    	val &= ~(MUX_CTRL1_GMAC0_USE_TX_CLK | MUX_CTRL1_GMAC0_USE_PWM01);
    else
    	val |= (MUX_CTRL1_GMAC0_USE_TX_CLK | MUX_CTRL1_GMAC0_USE_PWM01);
    LS1B_MUX_CTRL1 = val;

#elif defined(LS1C)
    /*
     * LS1C选用RMII模式. 注: 在QFP100封装下，MAC只能使用RMII模式
     */
    val  = LS1C_MISC_CTRL;
    val &= ~MISC_PHY_INTF_SEL_MASK;
    val |= MISC_PHY_INTF_SEL_RMII;
    LS1C_MISC_CTRL = val;

#endif

	/* close "gmac" system interrupt, set trigger mode
	 */
    LS1x_INTC_IEN(pMAC->int_ctrlr)  &= ~(pMAC->int_mask);
    LS1x_INTC_EDGE(pMAC->int_ctrlr) &= ~pMAC->int_mask;
    LS1x_INTC_POL(pMAC->int_ctrlr)  |= pMAC->int_mask;

    /* reset the GMAC and DMA, and wait for it to complete
     */
    pMAC->hwGDMA->busmode = gdma_busmode_swreset;
    ls1x_sync();

    for (i=0; i<GDMA_RESET_TIMEOUT; i++)
    	if (!(pMAC->hwGDMA->busmode & gdma_busmode_swreset))
    		break;

    if (i>=GDMA_RESET_TIMEOUT)
    	printk("Error: gmac-dma Reset timeout.\r\n");

    /* set mac address
     */
    if (macAddr != NULL)
    {
        pMAC->hwGMAC->addr0hi = ((macAddr[5] <<  8) | (macAddr[4] <<  0));
        pMAC->hwGMAC->addr0lo = ((macAddr[3] << 24) | (macAddr[2] << 16) |
                                 (macAddr[1] <<  8) | (macAddr[0] <<  0));
        ls1x_sync();

        /* get the MAC address from the chip
         */
        macAddr[5] = (pMAC->hwGMAC->addr0hi >>  8) & 0xFF;
        macAddr[4] = (pMAC->hwGMAC->addr0hi >>  0) & 0xFF;
        macAddr[3] = (pMAC->hwGMAC->addr0lo >> 24) & 0xFF;
        macAddr[2] = (pMAC->hwGMAC->addr0lo >> 16) & 0xFF;
        macAddr[1] = (pMAC->hwGMAC->addr0lo >>  8) & 0xFF;
        macAddr[0] = (pMAC->hwGMAC->addr0lo >>  0) & 0xFF;
    }

    /* read the gmac version
     */
	pMAC->gmacVersion = pMAC->hwGMAC->version;

	/* set mii-phy CSR Clock
	 */
    /*
	val = pMAC->hwGMAC->miictrl;
	val = (val & (~gmac_miictrl_csr_mask)) | gmac_miictrl_csr_3;
	pMAC->hwGMAC->miictrl = val;
     */

	/*
     * phy initialize
	 */
    pMAC->phyAddr = MII_detect_phy_addr(pMAC);
    pMAC->autoNegotiation = 1;          /* 自动协商吗 ? */
    MII_set_phy_link_mode(pMAC, SPEED100, FULLDUPLEX, 0);
	MII_get_phy_link_mode(pMAC, &eSpeed, &eDuplex, &eLink);

    printk("GMAC%i: SPEED=%iM, DUPLEX=%s, LINK=%s. AN=%s.\r\n\n", pMAC->unitNumber,
    	   (eSpeed == SPEED100) ? 100 : 10,
    	   (eDuplex == FULLDUPLEX) ? "FULL" : "HALF",
    	   (eLink == LINKUP) ? "UP" : "DOWN",
            pMAC->autoNegotiation ? "YES" : "NO");

    /* Set the "Start of Transmit Descriptor List Address" register
     */
    pMAC->hwGDMA->txdesc0 = VA_TO_PHYS((unsigned int)pMAC->tx_desc[0]);		// physical address

	/* Set the "Start of Receive Descriptor List Address" register
	 */
    pMAC->hwGDMA->rxdesc0 = VA_TO_PHYS((unsigned int)pMAC->rx_desc[0]);		// physical address

    /* set the gmac-dma busmode register(1), bit(7)=0 使用16字节大小的描述符
     */
    val = (gdma_busmode_dsl_1 |						// 设置2个描述符间的距离, RINGMODE
    	   gdma_busmode_pbl_4);						// Dma burst length = 4
    pMAC->hwGDMA->busmode = val;

	/* Init GDMA Control (1)
	 */
	val = (gdma_ctrl_txsf |							// Transmit Store and Forward
		   gdma_ctrl_rxsf |							// Receive Store and Forward
		   gdma_ctrl_txopsecf); 					// TX Operate on Second Frame
	val &= ~(gdma_ctrl_rtc_mask |					// Receive Threshold Control.
			 gdma_ctrl_ttc_mask);					//Transmit Threshold Control
	val |= (gdma_ctrl_rtc_128 |
			gdma_ctrl_ttc_64);
	pMAC->hwGDMA->control = val;

	/* gmac default value, move to here.
	 */
	LS1x_GMAC_set_workmode(pMAC);

	/* enables the pause control in Full duplex mode of operation
	 */
	val = pMAC->hwGDMA->control;
	val |= (gdma_ctrl_enhwfc | 						// Enable HW flow control
			0x00000300 | 							// Rx flow control Act. threhold (4kbytes)
			0x00400000);							// Rx flow control deact. threhold (5kbytes)
	pMAC->hwGDMA->control = val;

	val = pMAC->hwGMAC->flowctrl;
	val |= (gmac_flowctrl_rxfcen |					// Enable Receive Flow Control
			gmac_flowctrl_txfcen |					// Enable Transmit Flow Control
			0xFFFF0000);
	pMAC->hwGMAC->flowctrl = val;

	/* Clears all the pending interrupts.
	 */
	val = pMAC->hwGDMA->status;
	ls1x_sync();
	pMAC->hwGDMA->status = val;

#if defined(LS1B)
	/* 关闭 MMC 相关中断
	 */
	pMAC->hwGMAC->mmc_imask_rx  = 0xFFFFFFFF;
	pMAC->hwGMAC->mmc_imask_tx  = 0xFFFFFFFF;
	pMAC->hwGMAC->mmc_imask_ipc = 0xFFFFFFFF;
#endif

	/* Enable all gdma interrupt
	 */
    GDMA_INT_EN(pMAC->hwGDMA);
    ls1x_sync();

    if (pMAC->LinkState == LINKDOWN)
    {
        MII_write_phy(pMAC, MII_BMCR, BMCR_RESET);
	    MII_set_phy_link_mode(pMAC, SPEED100, FULLDUPLEX, 0);
        MII_get_phy_link_mode(pMAC, &eSpeed, &eDuplex, &eLink);
    }

    /******************************************************
	 * 硬件初始化完成
     */

	/* open "gmac" interrupt in system
	 */
    LS1x_INTC_CLR(pMAC->int_ctrlr)  = pMAC->int_mask;
    LS1x_INTC_IEN(pMAC->int_ctrlr) |= pMAC->int_mask;

	/* Clears all the pending interrupts again.
	 */
	val = pMAC->hwGDMA->status;
	ls1x_sync();
	pMAC->hwGDMA->status = val;
}

//*****************************************************************************
// Start the GMAC
//*****************************************************************************

static void LS1x_GMAC_do_reset(GMAC_t *pMAC)
{
    pMAC->started = 0;
	GMAC_STOP(pMAC->hwGMAC);		    /* stop the gmac. */
	GDMA_STOP(pMAC->hwGDMA);		    /* stop the gdma. */
	GDMA_INIT_DESC(pMAC);		        /* initialize the gmac */
	LS1x_GMAC_init_hw(pMAC, NULL);      /* GMAC started internal */
	GDMA_START(pMAC->hwGDMA);		    /* restart the gmac. */
	pMAC->started = 1;
}

//*****************************************************************************
// Start the GMAC
//*****************************************************************************

static void LS1x_GMAC_set_start(GMAC_t *pMAC)
{
    if (!pMAC->started)
    {
        GMAC_START(pMAC->hwGMAC);
        GDMA_START(pMAC->hwGDMA);
        pMAC->started = 1;
    }
}

//*****************************************************************************
// Stop the GMAC
//*****************************************************************************

static void LS1x_GMAC_set_stop(GMAC_t *pMAC)
{
    if (pMAC->started)
    {
        GDMA_STOP(pMAC->hwGDMA);
        GMAC_STOP(pMAC->hwGMAC);
        pMAC->started = 0;
    }
}

//***************************************************************************************
// Initialize Tx Descriptors
//***************************************************************************************

static int LS1x_init_tx_desc_queue(GMAC_t *pMAC)
{
	int i;
	unsigned int desc_ptr, buf_ptr;
	LS1x_txdesc_t *desc = NULL;

#if defined(BSP_USE_GMAC0)
    if (pMAC == &ls1x_GMAC0)
    {
	    desc_ptr = (unsigned int)&tx_desc_0[0];
	    buf_ptr  = (unsigned int)&tx_buf_0[0];
    }
    else
#endif
#if defined(BSP_USE_GMAC1)
    if (pMAC == &ls1x_GMAC1)
    {
	    desc_ptr = (unsigned int)&tx_desc_1[0];
	    buf_ptr  = (unsigned int)&tx_buf_1[0];
    }
    else
#endif
    {
        printk("%s parameter error!\r\n", __func__);
        return -1;
    }

	/* change to kseg1 address - uncached
	 */
#if (LS1x_K0_CACHED)
	desc_ptr = K0_TO_K1(desc_ptr);
  #if (!GMAC_BUF_CACHED)
	buf_ptr = K0_TO_K1(buf_ptr);
  #endif
#endif

	/* initializing desc and buffer
	 */
	for (i = 0; i < NUM_TX_DMA_DESC; i++)
	{
		/* tx descriptor
		 */
		desc = (LS1x_txdesc_t *)desc_ptr;
		memset((void *)desc, 0, sizeof(LS1x_txdesc_t));
		pMAC->tx_desc[i] = desc;

		desc_ptr += sizeof(LS1x_txdesc_t);

		if (pMAC->descmode == CHAINMODE)
		{
			desc->control = gdma_txdesc_ctrl_tch;
			if (i > 0)
			{
				pMAC->tx_desc[i-1]->nextdesc = VA_TO_PHYS((unsigned int)desc); // physical address
			}

			if (i == NUM_TX_DMA_DESC - 1)
			{
				desc->nextdesc = VA_TO_PHYS((unsigned int)pMAC->tx_desc[0]);   // physical address
			}
		}
		else if (i == NUM_TX_DMA_DESC - 1)	// RINGMODE
		{
			desc->control = gdma_txdesc_ctrl_ter;
		}

		/* tx buffer
		 */
    	pMAC->tx_desc[i]->bufptr = VA_TO_PHYS(buf_ptr);		// 发送缓存物理地址
    	pMAC->tx_buf[i] = (void *)buf_ptr;					// 发送缓冲区数组

    	buf_ptr += TX_BUF_SIZE;
	}

	return 0;
}

//***************************************************************************************
// Initialize Rx Descriptors
//***************************************************************************************

static int LS1x_init_rx_desc_queue(GMAC_t *pMAC)
{
	int i;
	unsigned int desc_ptr, buf_ptr;
	LS1x_rxdesc_t *desc = NULL;

#if defined(BSP_USE_GMAC0)
    if (pMAC == &ls1x_GMAC0)
    {
	    desc_ptr = (unsigned int)&rx_desc_0[0];
	    buf_ptr  = (unsigned int)&rx_buf_0[0];
    }
    else
#endif
#if defined(BSP_USE_GMAC1)
    if (pMAC == &ls1x_GMAC1)
    {
	    desc_ptr = (unsigned int)&rx_desc_1[0];
	    buf_ptr  = (unsigned int)&rx_buf_1[0];
    }
    else
#endif
    {
        printk("%s parameter error!\r\n", __func__);
        return -1;
    }

	/* change to kseg1 address - uncached
	 */
#if (LS1x_K0_CACHED)
	desc_ptr = K0_TO_K1(desc_ptr);
  #if (!GMAC_BUF_CACHED)
	buf_ptr = K0_TO_K1(buf_ptr);
  #endif
#endif

	/* initializing desc and buffer
	 */
	for (i = 0; i < NUM_RX_DMA_DESC; i++)
	{
		/* rx descriptor
		 */
		desc = (LS1x_rxdesc_t *)desc_ptr;
		memset((void *)desc, 0, sizeof(LS1x_rxdesc_t));
		pMAC->rx_desc[i] = desc;

		desc_ptr += sizeof(LS1x_rxdesc_t);

		if (pMAC->descmode == CHAINMODE)
		{
			desc->control = gdma_rxdesc_ctrl_rch;
			if (i > 0)
			{
				pMAC->rx_desc[i-1]->nextdesc = VA_TO_PHYS((unsigned int)desc);  // physical address
			}

			if (i == NUM_RX_DMA_DESC - 1)
			{
				desc->nextdesc = VA_TO_PHYS((unsigned int)pMAC->rx_desc[0]);	// physical address
			}
		}
		else if (i == NUM_RX_DMA_DESC - 1)	// RINGMODE
		{
			desc->control = gdma_rxdesc_ctrl_rer;
		}

		/* rx buffer
		 */
    	pMAC->rx_desc[i]->bufptr = VA_TO_PHYS(buf_ptr);		// 接收缓存物理地址
    	pMAC->rx_buf[i] = (void *)buf_ptr;					// 接收缓冲区数组

    	buf_ptr += RX_BUF_SIZE;

        pMAC->rx_desc[i]->control |= ETHER_MAX_LEN;
        pMAC->rx_desc[i]->status = gdma_rxdesc_stat_own;	// set as owned by dma
	}

	return 0;
}

//***************************************************************************************
// initialize the device
//***************************************************************************************

STATIC_DRV int LS1x_GMAC_initialize(void *dev, void *arg)
{
	GMAC_t *pMAC = (GMAC_t *)dev;

	if (dev == NULL)
        return -1;

    /* This is for stuff that only gets done once
     */
    if (!pMAC->initialized)
    {
    	/* initialize gmac registers
    	 */
    	GDMA_INIT_DESC(pMAC);
        LS1x_GMAC_init_hw(pMAC, (unsigned char *)arg);

#if BSP_USE_OS
    #if defined(OS_RTTHREAD)
        pMAC->gmac_event = rt_event_create(pMAC->dev_name, 0);
    #elif defined(OS_UCOS)
        unsigned char err;
        pMAC->gmac_event = OSFlagCreate(0, &err);
    #elif defined(OS_FREERTOS)
        pMAC->gmac_event = xEventGroupCreate();
    #endif

        if (NULL == pMAC->gmac_event)
        {
            printk("create GMAC event fail.\r\n");
            return -1;
        }
#endif

        /* install the interrupt handler
         */
        ls1x_install_irq_handler(pMAC->vector, LS1x_GMAC_irq_handler, (void *)pMAC);

        /* flag as initialized.
         */
        pMAC->initialized = 1;
    }

    /*
     * gdma start to receive and transmit, or not?
     *
     * TODO Some problem when FreeRTOS
     */
#if BSP_USE_OS
    GDMA_STOP(pMAC->hwGDMA);
    GMAC_STOP(pMAC->hwGMAC);
    pMAC->started = 0;
#else
    GDMA_START(pMAC->hwGDMA);
    pMAC->started = 1;
#endif

    printk("GMAC%i controller initialized.\r\n", \
           ((unsigned)pMAC->hwGDMA == LS1x_GDMA0_BASE) ? 0 : 1);

    return 0;
}

//***************************************************************************************
// Receive one packet
//***************************************************************************************

static int LS1x_GMAC_recv_packet_internal(GMAC_t *pMAC, unsigned char *buf, int size)
{
    int rx_len;
    unsigned int buf_ptr, status;

    status = pMAC->rx_desc[pMAC->rx_head]->status;
    if (status & gdma_rxdesc_stat_own)
    {
        /**************************************************
         * Info user while no packet in
         */
        // printk("gmac error rx_head=%i\r\n", pMAC->rx_head);
        return 0;
    }

    // DBG_OUT("rx_head = %i\r\n", pMAC->rx_head);

    /******************************************************
     * When current receice desc is not ownered by DMA
     */

    if (status & gdma_rxdesc_stat_es)   /* RX Error Summary */
        pMAC->rx_errors++;

    if (status & gdma_rxdesc_stat_le)   /* RX Length Error */
        pMAC->rx_length_err++;

    if (status & gdma_rxdesc_stat_dbe)  /* RX Dribble bit Error */
        pMAC->rx_dribit_err++;

    /******************************************************
     * If no errors, accept packet
     */

    buf_ptr = (unsigned int)pMAC->rx_buf[pMAC->rx_head];

    if (((status & gdma_rxdesc_stat_es) == 0) &&
        ((status & gdma_rxdesc_stat_fs) == gdma_rxdesc_stat_fs) &&
        ((status & gdma_rxdesc_stat_ls) == gdma_rxdesc_stat_ls))
    {
        pMAC->rx_pkts++;

        /* get total length of receive data.
         */
        rx_len = (status & gdma_rxdesc_stat_fr_mask) >> 16;

#if (LS1x_K0_CACHED && GMAC_BUF_CACHED)
        clean_dcache_nowrite(buf_ptr, rx_len);          /* invalidate dcache */
#endif
        rx_len = rx_len > size ? size : rx_len;

        /**************************************************
         * 用户自定义缓冲区, 需要复制; 否则直接使用
         */
        if (buf_ptr != (unsigned int)buf)
        {
            printk("gmac rx error!\r\n");
            // memcpy((void *)buf, (void *)buf_ptr, rx_len);
        }
    }
    else	// has error!
    {
        printk("gmac rx dropped!\r\n");
        pMAC->rx_dropped++;
        rx_len = 0;
    }

    /**************************************************
     * set up the receive dma buffer
     */

    pMAC->rx_desc[pMAC->rx_head]->control = ETHER_MAX_LEN + 18;
    if (pMAC->descmode == CHAINMODE)
    	pMAC->rx_desc[pMAC->rx_head]->control |= gdma_rxdesc_ctrl_rch;
    else if (pMAC->rx_head == NUM_RX_DMA_DESC - 1)
    	pMAC->rx_desc[pMAC->rx_head]->control |= gdma_rxdesc_ctrl_rer;
    pMAC->rx_desc[pMAC->rx_head]->bufptr = VA_TO_PHYS(buf_ptr);	    // Physical address
    pMAC->rx_desc[pMAC->rx_head]->status = gdma_rxdesc_stat_own;    // set as owned by dma
    ls1x_sync();

    /* Now force the DMA to start receive */
    pMAC->hwGDMA->rxpoll = 1;
    ls1x_sync();

    /*
     * increment the buffer index
     */
    pMAC->rx_head++;
    if (pMAC->rx_head >= NUM_RX_DMA_DESC)
        pMAC->rx_head = 0;

    return rx_len;
}

//***************************************************************************************
// LS1x_GMAC_read()
//***************************************************************************************

STATIC_DRV int LS1x_GMAC_read(void *dev, void *buf, int size, void *arg)
{
	GMAC_t *pMAC = (GMAC_t *)dev;

    if ((pMAC == NULL) || (buf == NULL))
        return -1;

    return LS1x_GMAC_recv_packet_internal(pMAC, (unsigned char *)buf, size);
}

//***************************************************************************************
// Send one packet
//***************************************************************************************

static int LS1x_GMAC_send_packet_internal(GMAC_t *pMAC, unsigned char *buf, int size)
{
    int tx_len = size;
    unsigned int buf_ptr, status;

    status = pMAC->tx_desc[pMAC->tx_head]->status;
    if (status & gdma_txdesc_stat_own)
    {
        /**************************************************
         * Info user while no buffer to send
         */
    //    printk("gmac error tx_head=%i\r\n", pMAC->tx_head);
        return 0;
    }

    // DBG_OUT("tx_head = %i\r\n", pMAC->tx_head);

    /******************************************************
     * Has GMAC Transmit Queue to become available.
     */

    /* Copy data to send buffer
     */
    if (tx_len > ETHER_MAX_LEN)
        tx_len = ETHER_MAX_LEN;

    buf_ptr = (unsigned int)pMAC->tx_buf[pMAC->tx_head];

    /******************************************************
     * 用户自定义缓冲区, 需要复制; 否则直接使用
     */
    if (buf_ptr != (unsigned int)buf)
    {
        printk("gmac tx error!\r\n");
        // memcpy((void *)buf_ptr, (void *)buf, tx_len);
    }

#if (LS1x_K0_CACHED && GMAC_BUF_CACHED)
	clean_dcache(buf_ptr, tx_len);          /* writeback and invalidate dcache */
#endif

    /* send it off
     */
    pMAC->tx_desc[pMAC->tx_head]->control = gdma_txdesc_ctrl_ic |		// Interrption on Complete
    									    gdma_txdesc_ctrl_ls	|		// Last Segment
    									    gdma_txdesc_ctrl_fs	|		// First Segment
    									   (tx_len & gdma_txdesc_ctrl_tbs1_mask);
	if (pMAC->descmode == CHAINMODE)									// chain mode
		pMAC->tx_desc[pMAC->tx_head]->control |= gdma_txdesc_ctrl_tch;
	else if (pMAC->tx_head == NUM_TX_DMA_DESC - 1)
		pMAC->tx_desc[pMAC->tx_head]->control |= gdma_txdesc_ctrl_ter;	// last of ring mode
	pMAC->tx_desc[pMAC->tx_head]->bufptr = VA_TO_PHYS(buf_ptr);			// Physical address
    pMAC->tx_desc[pMAC->tx_head]->status = gdma_txdesc_stat_own;
    ls1x_sync();

    /* Now force the DMA to start transmission
     */
    pMAC->hwGDMA->txpoll = 1;
    ls1x_sync();

    /******************************************************
     * 上面语句结束后, DMA 立即进入中断处理.
     ******************************************************/

    /*
     * increment the buffer index
     */
    pMAC->tx_head++;
    if (pMAC->tx_head >= NUM_TX_DMA_DESC)
        pMAC->tx_head = 0;

    return tx_len;
}

//***************************************************************************************
// LS1x_GMAC_write()
//***************************************************************************************

STATIC_DRV int LS1x_GMAC_write(void *dev, void *buf, int size, void *arg)
{
	GMAC_t *pMAC = (GMAC_t *)dev;

    if ((pMAC == NULL) || (buf == NULL))
        return -1;

    return LS1x_GMAC_send_packet_internal(pMAC, (unsigned char *)buf, size);
}

//***************************************************************************************
// Show interface statistics
//***************************************************************************************

static void LS1x_GMAC_stats(GMAC_t *pMAC)
{
    printk("Interrupts:%-8u", pMAC->interrupts);
    printk("    RX Interrupts:%-8u", pMAC->rx_interrupts);
    printk("    TX Interrupts:%-8u\n", pMAC->tx_interrupts);
    printk("RX Packets:%-8u", pMAC->rx_pkts);
    printk("    RX Error Summary:%-8u", pMAC->rx_errors);
    printk("    RX Length Error:%-8u", pMAC->rx_length_err);
    printk("    RX Dribble bit Error:%-8u", pMAC->rx_dribit_err);
    printk("    RX dropped:%-8u\n", pMAC->rx_dropped);
    printk("TX Packets:%-8u", pMAC->tx_pkts);
    printk("    TX Error Summary:%-8u", pMAC->tx_errors);
    printk("    TX IP Header Error:%-8u", pMAC->tx_ipheader_err);
    printk("    TX Payload Checksum Error:%-8u", pMAC->tx_playload_err);
    printk("    TX Defered Bit Error:%-8u", pMAC->tx_defered);
    printk("    TX Collsion Error:%-8u", pMAC->tx_collsions);
    printk("Fatal Error:%-8u", pMAC->dma_fatal_err);
}

//***************************************************************************************
// 等待接收事件
//***************************************************************************************

int LS1x_GMAC_wait_receive_packet(void *dev, unsigned char **bufptr)
{
    unsigned int status;
	GMAC_t *pMAC = (GMAC_t *)dev;

    if ((pMAC == NULL) || (bufptr == NULL))
        return -1;

    /*
     * Exsisting received packet, return immediately
     */
    status = pMAC->rx_desc[pMAC->rx_head]->status;
    if (!(status & gdma_rxdesc_stat_own))
    {
        *bufptr = pMAC->rx_buf[pMAC->rx_head];              /* receive buffer */
        return (status & gdma_rxdesc_stat_fr_mask) >> 16;   /* length of receive data. */
    }

    *bufptr = NULL;

    /*
     * Waiting...
     */
#if defined(OS_RTTHREAD)

	unsigned int recv = 0;
    rt_event_recv(pMAC->gmac_event,
                  GMAC_RX_EVENT,
                  RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                  RT_WAITING_FOREVER,           /* 无限等待 */
                  &recv);

    if (recv != GMAC_RX_EVENT)
	    return -1;

#elif defined(OS_UCOS)

    unsigned char err;
    unsigned short recv;
    recv = OSFlagPend(pMAC->gmac_event,
                      (OS_FLAGS)GMAC_RX_EVENT,  /* 接收事件 */
                      OS_FLAG_WAIT_SET_ALL |    /* 接收事件标志位置1时有效，否则任务挂在这里 */
                      OS_FLAG_CONSUME,          /* 清除指定事件标志位 */
                      0,                        /* 0=无限等待 */
                      &err);

    if (recv != GMAC_RX_EVENT)
	    return -1;

#elif defined(OS_FREERTOS)

    unsigned int recv;
    recv = xEventGroupWaitBits(pMAC->gmac_event,  /* 事件对象句柄 */
                               GMAC_RX_EVENT,     /* 接收事件 */
                               pdTRUE,            /* 退出时清除事件位 */
                               pdTRUE,            /* 满足感兴趣的所有事件 */
                               portMAX_DELAY);    /* 指定超时事件, 一直等 */

    if (recv != GMAC_RX_EVENT)
	    return -1;

#else

    int tmo = pMAC->timeout;

    while (tmo-- > 0)
    {
        delay_ms(1);
        status = pMAC->rx_desc[pMAC->rx_head]->status;
        if (!(status & gdma_rxdesc_stat_own))
            break;
    }

    if (tmo <= 0)
	    return -1;

#endif

    /*
     * Confirm received packet
     */
    status = pMAC->rx_desc[pMAC->rx_head]->status;
    if (!(status & gdma_rxdesc_stat_own))
    {
        *bufptr = pMAC->rx_buf[pMAC->rx_head];              /* receive buffer */
        return (status & gdma_rxdesc_stat_fr_mask) >> 16;   /* length of receive data. */
    }

    return 0;  // IS'S OK
}

//***************************************************************************************
// 等待发送事件
//***************************************************************************************

int LS1x_GMAC_wait_transmit_idle(void *dev, unsigned char **bufptr)
{
    unsigned int status;
	GMAC_t *pMAC = (GMAC_t *)dev;

    if ((pMAC == NULL) || (bufptr == NULL))
        return -1;

    /*
     * Exsisting idle tx buffer, return immediately
     */
    status = pMAC->tx_desc[pMAC->tx_head]->status;
    if (!(status & gdma_txdesc_stat_own))
    {
        *bufptr = pMAC->tx_buf[pMAC->tx_head];
        return ETHER_MAX_LEN;
    }

    *bufptr = NULL;

    /*
     * Waiting
     */
#if defined(OS_RTTHREAD)

	unsigned int recv = 0;
    rt_event_recv(pMAC->gmac_event,
                  GMAC_TX_EVENT,
                  RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
                  RT_WAITING_FOREVER,           /* 无限等待 */
                  &recv);

    if (recv != GMAC_TX_EVENT)
        return -1;

#elif defined(OS_UCOS)

    unsigned char err;
    unsigned short recv;
    recv = OSFlagPend(pMAC->gmac_event,
                      (OS_FLAGS)GMAC_TX_EVENT,  /* 发送事件 */
                      OS_FLAG_WAIT_SET_ALL |    /* 接收事件标志位置1时有效，否则任务挂在这里 */
                      OS_FLAG_CONSUME,          /* 清除指定事件标志位 */
                      0,                        /* 0=无限等待, 直到收到信号为止 */
                      &err);

    if (recv != GMAC_TX_EVENT)
        return -1;

#elif defined(OS_FREERTOS)

    unsigned int recv;
    recv = xEventGroupWaitBits(pMAC->gmac_event, /* 事件对象句柄 */
                               GMAC_TX_EVENT,    /* 发送事件 */
                               pdTRUE,           /* 退出时清除事件位 */
                               pdTRUE,           /* 满足感兴趣的所有事件 */
                               portMAX_DELAY);   /* 指定超时事件, 一直等 */

    if (recv != GMAC_TX_EVENT)
        return -1;

#else

    int tmo = pMAC->timeout;

    while (tmo-- > 0)
    {
        delay_ms(1);
        status = pMAC->tx_desc[pMAC->tx_head]->status;
        if (!(status & gdma_txdesc_stat_own))
            break;
    }

    if (tmo <= 0)
        return -1;

#endif

    /*
     * Confirm idle tx buffer
     */
    status = pMAC->tx_desc[pMAC->tx_head]->status;
    if (!(status & gdma_txdesc_stat_own))
    {
        *bufptr = pMAC->tx_buf[pMAC->tx_head];
        return ETHER_MAX_LEN;
    }

    return 0; // IS'S OK
}

//***************************************************************************************
// Driver ioctl handler
//***************************************************************************************

STATIC_DRV int LS1x_GMAC_ioctl(void *dev, int cmd, void *arg)
{
    int rt = 0;
	GMAC_t *pMAC = (GMAC_t *)dev;

    if (dev == NULL)
        return -1;

    switch (cmd)
    {
    	case IOCTL_GMAC_START:              /* start the GMAC hardware */
            LS1x_GMAC_set_start(pMAC);
            delay_us(100);
            break;

        case IOCTL_GMAC_STOP:               /* stop the GMAC hardware */
            LS1x_GMAC_set_stop(pMAC);
            delay_us(100);
            break;

        case IOCTL_GMAC_RESET:              /* stop then reset GMAC */
            LS1x_GMAC_do_reset(pMAC);
            break;

        case IOCTL_GMAC_SET_MACADDR:
            rt = -2;
            break;

        case IOCTL_GMAC_SET_TIMEOUT:        /* set transmit/receive time out */
            pMAC->timeout = (int)arg;
            break;

        case IOCTL_GMAC_IS_RUNNING:         /* GMAC is running? started or not */
            rt = pMAC->started;
            break;

    	case IOCTL_GMAC_SHOW_STATS:
    		LS1x_GMAC_stats(pMAC);
    		break;

    	default:
    		break;
    }

    return rt;
}

//***************************************************************************************
// interrupt handler
//***************************************************************************************

static void LS1x_GMAC_irq_handler(int vector, void *arg)
{
	GMAC_t *pMAC = (GMAC_t *)arg;
	unsigned int dma_status;
    int rx_flag=0, tx_flag=0;

    if (pMAC == NULL)
    {
        printk("gmac interrupt error: arg == NULL!\r\n");
        return;
    }

#if defined(OS_UCOS)
    unsigned char err;
#elif defined(OS_FREERTOS)
	BaseType_t xResult, xHigherPriorityTaskWoken = pdFALSE;
#endif

    pMAC->interrupts++;

	/* 读 GMAC-DMA 状态寄存器, 判断中断是否由本设备(DMA)产生, 如果不是, 返回
	 */
    dma_status = pMAC->hwGDMA->status;
    ls1x_sync();
	pMAC->hwGDMA->status = dma_status;	    // & ~0x1FFFF;

	if (0 == dma_status)
		return;

	GDMA_INT_DIS(pMAC->hwGDMA);	            /* disable gdma interrupt */
    ls1x_sync();

	/******************************************************
	 * 开始处理  GMAC-DMA 中断
	 */

	/* Fatal Bus Error Interrutp
	 */
	if (dma_status & gdma_status_fbei)
	{
#if (GMAC_DEBUG)
		DBG_OUT("Fatal Error: gmac-dma Bus Error Interrutp! Restart it.\n");
#endif
		pMAC->dma_fatal_err++;
        LS1x_GMAC_do_reset(pMAC);
	    return;
	}

	if (dma_status & gdma_status_nis)
		pMAC->dma_normal_intrs++;

	if (dma_status & gdma_status_ais)
		pMAC->dma_abnormal_intrs++;

	/******************************************************
	 * 接收部分的处理
	 */

	/* Normal Receive Interrupt
	 */
	if (dma_status & gdma_status_rxi)
	{
		/* Must prevent the dead loop: Flag "reserved" bit as "done"
		 */
	    while (((pMAC->rx_desc[pMAC->rx_tail]->status & gdma_rxdesc_stat_own) == 0) &&
	    	   ((pMAC->rx_desc[pMAC->rx_tail]->control & gdma_rxdesc_ctrl_userdef) == 0))
	    {
	        pMAC->rx_interrupts++;

	        pMAC->rx_desc[pMAC->rx_tail]->control |= gdma_rxdesc_ctrl_userdef;

	        pMAC->rx_tail++;
	        if (pMAC->rx_tail >= NUM_RX_DMA_DESC)
	            pMAC->rx_tail = 0;

            rx_flag = 1;
	    }

	    if (rx_flag)
	    {
#if defined(OS_RTTHREAD)
            rt_event_send(pMAC->gmac_event, GMAC_RX_EVENT);
#elif defined(OS_UCOS)
            OSFlagPost(pMAC->gmac_event,
                       (OS_FLAGS)GMAC_RX_EVENT,
                       OS_FLAG_SET,
                       &err);
#elif defined(OS_FREERTOS)
		    xResult = xEventGroupSetBitsFromISR(pMAC->gmac_event,
                                                GMAC_RX_EVENT,
                                                &xHigherPriorityTaskWoken);
            if (xResult != pdPASS)  /* Was the message posted successfully? */
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
#endif

#if (GMAC_DEBUG)
            DBG_OUT("rx_int.\r\n");
#endif
	    }
	}

	/* Receive Buffer Unavailable
	 */
	if (dma_status & gdma_status_rxbufu)
	{
#if (GMAC_DEBUG)
		DBG_OUT("gdma rxbufu!\n");
#endif
    	pMAC->rx_buffer_unavailable++;
    }

	/* Receive Process Stopped
	 */
	if (dma_status & gdma_status_rxstop)
	{
#if (GMAC_DEBUG)
		DBG_OUT("Warning: gmac-dma Receive Process Stopped!\n");
#endif
		pMAC->rx_stopped++;
#if !defined(DUAL_NIC_REDUNDANCY)
		GDMA_STOP_RX(pMAC->hwGDMA);
		GDMA_INIT_RXDESC(pMAC);
		GDMA_INIT_RXDESC_CUR(pMAC);
		GDMA_START_RX(pMAC->hwGDMA);
#endif
	}

	/******************************************************
	 * 发送部分的处理
	 */

	/* Normal Transmit Interrupt
	 */
	if (dma_status & gdma_status_txi)
	{
	    while (((pMAC->tx_desc[pMAC->tx_tail]->status & gdma_txdesc_stat_own) == 0) &&
	    	   ((pMAC->tx_desc[pMAC->tx_tail]->control & gdma_txdesc_ctrl_tbs1_mask) != 0))
	    {
	        volatile unsigned int status;

	        pMAC->tx_interrupts++;

	        status = pMAC->tx_desc[pMAC->tx_tail]->status;

	        if (status & gdma_txdesc_stat_es)			// TX Error Summary
	        	pMAC->tx_errors++;

	        if (status & gdma_txdesc_stat_ihe)			// TX IP Header Error
	        	pMAC->tx_ipheader_err++;

	        if (status & gdma_txdesc_stat_pce)			// Payload Checksum Error
	        	pMAC->tx_playload_err++;

	        if (status & gdma_txdesc_stat_db)			// Defered Bit
	        	pMAC->tx_defered++;

	        if (status & gdma_txdesc_stat_cc_mask)		// bits: 6-3, Collsion Count
	        	pMAC->tx_collsions += ((status & gdma_txdesc_stat_cc_mask) >> 3);

	        /* Reset the tx_desc: tx has done.
	         */
	        pMAC->tx_desc[pMAC->tx_tail]->status = 0;
	        if (pMAC->descmode == CHAINMODE)
	        	pMAC->tx_desc[pMAC->tx_tail]->control = gdma_txdesc_ctrl_tch;
	        else if (pMAC->tx_tail == NUM_TX_DMA_DESC - 1)	// RINGMODE
	        	pMAC->tx_desc[pMAC->tx_tail]->control = gdma_txdesc_ctrl_ter;

	        pMAC->tx_tail++;
	        if (pMAC->tx_tail >= NUM_TX_DMA_DESC)
	            pMAC->tx_tail = 0;

	        tx_flag = 1;
	    }

	    if (tx_flag)
	    {

#if defined(OS_RTTHREAD)
            rt_event_send(pMAC->gmac_event, GMAC_TX_EVENT);
#elif defined(OS_UCOS)
            OSFlagPost(pMAC->gmac_event,
                       (OS_FLAGS)GMAC_TX_EVENT,
                       OS_FLAG_SET,
                       &err);
#elif defined(OS_FREERTOS)
		    xResult = xEventGroupSetBitsFromISR(pMAC->gmac_event,
                                                GMAC_TX_EVENT,
                                                &xHigherPriorityTaskWoken);
            if (xResult != pdPASS)  /* Was the message posted successfully? */
            {
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
            }
#endif

#if (GMAC_DEBUG)
            DBG_OUT("tx_int.\r\n");
#endif
        }
	}

	/* Transmit Underflow
	 */
	if (dma_status & gdma_status_txunf)
	{
#if (GMAC_DEBUG)
		DBG_OUT("Error: gmac-dma Transmit Underflow Interrutp!\n");
#endif
		pMAC->tx_underflow++;
	}

	/* Transmit Buffer Unavailable, 该中断是否取消 ?
	 */
	if (dma_status & gdma_status_txbufu)
		pMAC->tx_buffer_unavailable++;

	/* Transmit Process Stopped
	 */
	if (dma_status & gdma_status_txstop)
	{
#if (GMAC_DEBUG)
		DBG_OUT("Warning: gmac-dma Transmit Process Stopped!\n");
#endif
		pMAC->tx_stopped++;
#if !defined(DUAL_NIC_REDUNDANCY)
		GDMA_STOP_TX(pMAC->hwGDMA);
		GDMA_INIT_TXDESC(pMAC);
		GDMA_INIT_TXDESC_CUR(pMAC);
		GDMA_START_TX(pMAC->hwGDMA);
#endif
	}

	/******************************************************
	 * GMAC 中断处理 结束
	 */

	/* open gdma interrupt
	 */
	GDMA_INT_EN(pMAC->hwGDMA);

	return;
}

//---------------------------------------------------------------------------------------

#if (PACK_DRV_OPS)
/******************************************************************************
 * GMAC driver operators
 */
static driver_ops_t LS1x_GMAC_drv_ops =
{
    .init_entry  = LS1x_GMAC_initialize,
    .open_entry  = NULL,
    .close_entry = NULL,
    .read_entry  = LS1x_GMAC_read,
    .write_entry = LS1x_GMAC_write,
    .ioctl_entry = LS1x_GMAC_ioctl,
};

driver_ops_t *ls1x_gmac_drv_ops = &LS1x_GMAC_drv_ops;
#endif

/******************************************************************************
 * for RT-Thread
 */
#if defined(OS_RTTHREAD)
const char *ls1x_gmac_get_device_name(void *pMAC)
{
    return ((GMAC_t *)pMAC)->dev_name;
}
#endif


#endif // #ifdef BSP_USE_GMAC

