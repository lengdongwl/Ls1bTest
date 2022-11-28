/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * ls1x_gmac_hw.h
 *
 * Created on: 2013/07/01
 *     Author: Bian
 *
 */

#ifndef LS1X_GMAC_HW_H_
#define LS1X_GMAC_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef bit
#define bit(x)	(1<<x)
#endif

/******************************************************************************
 * GMAC registers
 ******************************************************************************/

typedef struct LS1x_gmac_regs_s
{
	volatile unsigned int control;				/* 0xBFE10000 Configuration */
	volatile unsigned int framefilter;			/* 0xBFE10004 GMAC Frame Filter */
	volatile unsigned int hashhi;				/* 0xBFE10008 Hash Table High */
	volatile unsigned int hashlo;				/* 0xBFE1000C Hash Table Low */
	volatile unsigned int miictrl;				/* 0xBFE10010 GMII Address */
	volatile unsigned int miidata;				/* 0xBFE10014 GMII Data */
	volatile unsigned int flowctrl;				/* 0xBFE10018 Flow Control */
	volatile unsigned int vlantag;				/* 0xBFE1001C VLAN Tag */
	volatile unsigned int version;				/* 0xBFE10020 Version */
	volatile unsigned int rsv1[5];
	volatile unsigned int intstatus;			/* 0xBFE10038 Interrupt Status */
	volatile unsigned int intmask;				/* 0xBFE1003C Interrupt Mask */
	volatile unsigned int addr0hi;				/* 0xBFE10040 Address0 High */
	volatile unsigned int addr0lo;				/* 0xBFE10044 Address0 Low */
	volatile unsigned int addr1hi;				/* 0xBFE10048 Address1 High */
	volatile unsigned int addr1lo;				/* 0xBFE1004C Address1 Low */
	volatile unsigned int rsv2[28];
	volatile unsigned int anctrl;				/* 0xBFE100C0 AN Control */
	volatile unsigned int anstatus;				/* 0xBFE100C4 AN Status */
	volatile unsigned int anadvertise;			/* 0xBFE100C8 AN Advertisement */
	volatile unsigned int anlinkpa;				/* 0xBFE100CC AN Link Partner Ability */
	volatile unsigned int anexpansion;			/* 0xBFE100D0 AN Link Expansion */
	volatile unsigned int rsv3;
	volatile unsigned int miistatus;			/* 0xBFE100D8 SGMII/RGMII Status */

#if defined(LS1B)
	/*
	 * MMC registers of GMAC
	 */
	volatile unsigned int rsv4[12];
	volatile unsigned int mmc_imask_rx;         /* 0x010C mask for interrupts generated from rx counters */
	volatile unsigned int mmc_imask_tx;         /* 0x0110 mask for interrupts generated from tx counters */
	volatile unsigned int rsv5[60];
	volatile unsigned int mmc_imask_ipc;        /* 0x0200 mask for interrupts generated from rx IPC statistic counters */
#endif

} LS1x_gmac_regs_t;

/*
 * GMAC Config Register 0x0000
 */
typedef enum
{
	gmac_ctrl_tc			= bit(24),		/* Transmit Configuration in RGMII, 使能RGMII链路信息传输.
	      	  	  	  	  	  	  	  	  	   =1: 将会把双工模式, 链路速度, 链路以及链路连接/断开等信息通过RGMII接口传输给PHY */
	gmac_ctrl_wd			= bit(23),		/* Watchdog Disable, 关闭看门狗.
	 	 	 	 	 	 	 	 	 	 	   =1: GMAC将关闭接收端的看门狗定时器, 可以接收最大16384字节的以太网帧*/
	gmac_ctrl_jd			= bit(22),		/* Jabber Disable, 关闭Jabber定时器.
											   =1: GMAC关闭发送过程中的Jabber定时器, 可以发送最大16384字节的以太网帧 */
	gmac_ctrl_be			= bit(21),		/* Frame Burst Enable, =1: GMAC使能传输过程中的帧突发传输模式 */
	gmac_ctrl_je			= bit(20),		/* Jumbo Frame Enable - 巨帧使能, =1: GMAC使能巨帧(最大9018字节)的接收 */
	gmac_ctrl_ifg_mask		= (0x7<<17), 	/* Inter-Frame Gap - 最小帧间距 */
	gmac_ctrl_ifg_shift		= 17,
	gmac_ctrl_dcrs			= bit(16),		/* Disable Carrier Sense During Transmission, 传输过程中关闭载波冲突检测
											   =1: GMAC忽略半双工模式下CRS信号的检测 */
	gmac_ctrl_ps			= bit(15),		/* Port Select, 0: GMII (1000Mbps), 1: MII (10/100Mbps) */
	gmac_ctrl_fes			= bit(14),		/* Speed, 0: 10Mbps, 1: 100Mbps */
	gmac_ctrl_do			= bit(13),		/* Disable Receive Own, 关闭接收自己发出的以太网帧.
											   =1: GMAC不接收半双工模式下gmii_txen_o有效的以太网帧 */
	gmac_ctrl_lm			= bit(12),		/* Loopback Mode,  =1: GMII/MII工作在环回模式下 */
	gmac_ctrl_dm			= bit(11),		/* Duplex Mode, 使能全双工模式
											   =1: GMAC工作在全双工模式下, 在全双工模式下可以同时发送和接收以太网帧 */
	gmac_ctrl_ipc			= bit(10),		/* Checksum Offload, 校验和卸载使能
											   =1: GMAC硬件计算接收到以太网帧的负载(payload). 还检查IPV4头的校验和是否正确 */
	gmac_ctrl_dr			= bit(9),		/* Disable Retry, 关闭重传
											   =1: GMAC在遇到冲突时不重传发送冲突的以太网帧, 而只报告冲突错误 */
	gmac_ctrl_lud			= bit(8),		/* Link Up/Down, 0: 链路断开, 1: 链路连接  */
	gmac_ctrl_acs			= bit(7),		/* Automatic Pad/CRC Stripping, 以太网帧Pad/CRC自动去除
											   =1: GMAC中去除接收到的以太网帧的Pad和FCS */
	gmac_ctrl_bl_mask		= (0x03<<5), 	/* Back-Off Limit - 回退限制, 回退限制决定基于slot的延迟时间 */
	gmac_ctrl_bl_shift		= 5,
	gmac_ctrl_bl_0			= (0<<5),		// 00: k=min(n,10)
	gmac_ctrl_bl_1			= (1<<5),		// 01: k=min(n,8)
	gmac_ctrl_bl_2			= (2<<5),		// 10: k=min(n,4)
	gmac_ctrl_bl_3			= (3<<5),		// 11: k=min(n,1)
	gmac_ctrl_dc			= bit(4),		/* Deferral Check, =1: 使能deferral检测功能 */
	gmac_ctrl_te			= bit(3),		/* Transmitter Enable, =1: 使能GMAC传输功能 */
	gmac_ctrl_re			= bit(2),		/* Receiver Enable, =1: 使能GMAC接收功能 */
} LS1x_gmac_control_t;

/*
 * GMAC Frame Filter Register 0x0004
 */
typedef enum
{
	gmac_frmfilter_ra		= bit(31),		/* Receive All, =1: GMAC接收模块把接收到的所有帧都发给应用程序, 忽略源地址/目标地址过滤机制 */
	gmac_frmfilter_hpf		= bit(10),		/* Hash or Perfect Filter, 哈希或者完全过滤
											   =1: 在哈希/完全过滤机制中匹配的以太网帧发送给应用.
											   =0: 只有在哈希过滤机制中匹配的以太网帧才发送给应用.  */
	gmac_frmfilter_saf		= bit(9),		/* Source Address Filter Enable, 源地址过滤使能
											   GMAC CORE比较比较接收到以太网帧的源地址域和在SA寄存器中的值, 如果匹配, 接收状态寄存器中的SAMatch位设置为高.
											   =1: 源地址匹配失败, GMAC CORE将丢弃该以太网帧.
											   =0: 不管源地址匹配结果GMAC CORE都接收此帧, 而匹配结果写入接收状态寄存器. */
	gmac_frmfilter_saif		= bit(8),		/* SA Inverse Filtering, 源地址反转过滤.
											   =1: 和SA寄存器中源地址匹配的以太网帧将会标记为源地址匹配失败.
											   =0: 和SA寄存器中源地址不匹配的以太网帧将会标记为源地址匹配失败.  */
	gmac_frmfilter_pcf_mask	 = (0x03<<6),	/* bits: 7-6, Pass Control Frames, 接收控制帧 */
	gmac_frmfilter_pcf_shift = 6,
	gmac_frmfilter_pcf_0	 = (0<<6),		// 00: GMAC过滤所有控制帧;
	gmac_frmfilter_pcf_1	 = (1<<6),		// 01: GMAC接收除了pause帧以外的所有控制帧;
	gmac_frmfilter_pcf_2	 = (2<<6),		// 10: GMAC接收所有控制帧;
	gmac_frmfilter_pcf_3	 = (3<<6),		// 11: GMAC根据地址过滤情况接收控制帧 */

	gmac_frmfilter_dbf		= bit(5),		/* Disable Broadcast Frames, 关闭广播帧. =1: 过滤所有接收的广播帧. =0: 接收所有广播帧.  */
	gmac_frmfilter_pm		= bit(4),		/* Pass All Multicast, 接收所有多播帧. =1: 接收所有多播帧. =0: 过滤所有多播帧.  */
	gmac_frmfilter_daif		= bit(3),		/* DA Inverse Filtering, 目标地址反转过滤.
											   =1: 对单播和多播帧进行反向目标地址匹配.
											   =0: 对单播和多播帧进行正常目标地址匹配.  */
	gmac_frmfilter_hmc		= bit(2),		/* Hash Multicast, 哈希多播过滤, =1: 对接收到的多播帧根据哈希表的内容进行目标地址过滤 */
	gmac_frmfilter_huc		= bit(1),		/* Hash Unicast, 哈希单播过滤; =1: 对接收到的单播帧根据哈希表的内容进行目标地址过滤 */
	gmac_frmfilter_pr		= bit(0),		/* Promiscuous Mode, 混杂模式, =1: 接收所有以太网帧 */
} LS1x_gmac_frmfilter_t;

/*
 * GMAC Flow Control Register 0x0018
 */
typedef enum
{
	gmac_flowctrl_pt_mask	= (0xFFFF<<16),		/* bits31~16, Pause Time, 暂停时间, 此域保存了需要填入传输控制帧中的暂停时间域 */
	gmac_flowctrl_pt_shift  = 16,
	gmac_flowctrl_dzpq		= bit(7),			/* Disable Zero-Quanta Pause, 禁止零时间片暂停帧, =1: 禁止自动零时间片的暂停控制帧的产生 */
	gmac_flowctrl_plt_mask	= (0x03<<4),		/* bits5~4, Pause Low Threshold, 用于设置暂停时间的阈值 */
	gmac_flowctrl_plt_shift = 4,
	gmac_flowctrl_plt_0		= (0<<4),			/* 00: 暂停时间减少4个时间槽 */
	gmac_flowctrl_plt_1		= (1<<4),			/* 01: 暂停时间减少28个时间槽 */
	gmac_flowctrl_plt_2		= (2<<4),			/* 10: 暂停时间减少144个时间槽 */
	gmac_flowctrl_plt_3		= (3<<4),			/* 11: 暂停时间减少256个时间槽 */
												/* 一个时间槽为在GMII/MII接口上传输512比特或者64字节的时间 */
	gmac_flowctrl_up		= bit(3),			/* Unicast Pause Frame Detect, 单播的暂停帧探测,
												   =1: GMAC将会根据GMAC地址0指定的本站单播地址来探测暂停帧 */
	gmac_flowctrl_rxfcen	= bit(2),			/* Receive Flow Control Enable, 接收流控使能,
												   =1: GMAC将会解析接收到的暂停帧, 并且按照暂停帧指定的时间暂停帧的发送 */
	gmac_flowctrl_txfcen	= bit(1),			/* Transmit Flow Control Enable, 发送流控使能,
												   =1: 在全双工模式下, GMAC使能暂停帧的发送; 在半双工模式下, GMAC使能反压操作.  */
	gmac_flowctrl_fcb_bpa	= bit(0),			/* Flow Control Busy/Backpressure Activate, 流控忙/反压激活,
												   =1: 在全双工模式下发起暂停控制帧的发送或在半双工模式下启动反压操作 */
} LS1x_gmac_flowctrl_t;

/*
 * VLAN Tag Register 0x001C
 */
typedef enum
{
	gmac_vlan_tag_evt		= bit(16),			/* Enable 12-Bit VLAN Tag Comparison, 使能12位VLAN Tag比较.
												   =1: 使用12位VLAN Tag而不是使用16位VLAN Tag用于以太网帧比较和过滤. */
	gmac_vlan_tag_vl_mask	= 0xFFFF,			/* VLAN Tag Identifier for Receive Frames, 帧接收的VLAN Tag标识.
												      此域保存802.1Q格式的VLAN Tag, 用于比较接收到的以太网帧的位于第15和第16个字节的VLAN Tag. */
} LS1x_gmac_vlan_tag_t;

/*
 * GMAC Interrupt Status Register 0x0038
 */
typedef enum
{
	gmac_intstat_mmcerr			= bit(7),		/* MMC校验和卸载寄存器产生任何中断产生时, 此位设置为1 */
	gmac_intstat_mmctx			= bit(6),		/* MMC传输中断寄存器产生任何中断时, 此位设置为1 */
	gmac_intstat_mmcrx			= bit(5),		/* MMC接收中断寄存器产生任何中断时, 此位设置为1 */
	gmac_intstat_mmc			= bit(4),		/* MMC中断状态. 7:5的任何位为高时, 此位设置为1 */
	gmac_intstat_pmt			= bit(3),		/* 电源管理中断状态, 在PowerDown或者WakeUp时, 此位设置为1 */
	gmac_intstat_ancomp			= bit(2),		/* RGMII PHY接口自动协商完成时, 此位设置为1 */
	gmac_intstat_linkstatus		= bit(1),		/* RGMII PHY接口的链路状态发生任何变化时, 此位设置为1 */
	gmac_intstat_rgmii			= bit(0),		/* RGMII PHY接口的链路状态发生任何变化时, 此位设置为1 */
} LS1x_gmac_intr_status_t;

/*
 * GMAC Interrupt Mask Register 0x003C
 */
typedef enum
{
	gmac_intmask_timestamp		= bit(9),		/* =1: 禁止时间戳发生的中断 */
	gmac_intmask_pmt			= bit(3),		/* =1: 禁止电源管理引起的中断 */
	gmac_intmask_ancomp			= bit(2),		/* =1: 禁止PCS自动协商完成中断 */
	gmac_intmask_linkchange		= bit(1),		/* =1: 禁止由于PCS链路状态变化引起的中断 */
	gmac_intmask_rgmii			= bit(0),		/* =1: 禁止RGMII引起的中断 */
} LS1x_gmac_intr_mask_t;

/*
 * GMAC Address0 High Register x0040
 */

/*
 * GMAC Address0 Low Register 0x0044
 */

/*
 * GMAC Address1 High Register 0x0048
 */
typedef enum
{
	gmac_addr1_hi_ae			= bit(31),		/* Address Enable 地址使能.
												   =1: 地址过滤模块使用第2个GMAC地址用于完全地址过滤.
												   =0: 地址过滤模块不使用第2个GMAC地址用于地址过滤. */
	gmac_addr1_hi_sa			= bit(30),		/* Source Address 源GMAC地址.
												   =1: GMAC地址1用于比较接收帧的源GMAC地址. =0: GMAC地址1用于比较接收帧的目标GMAC地址. */
	gmac_addr1_hi_mbc_mask		= (0x3F<<24),	/* bit29~24, Mask Byte Control 掩模字节控制.
												      此域用于比较每个GMAC地址的字节掩模控制位. 比如第29位用于掩码寄存器18的[15:8]这个字节 */
	gmac_addr1_hi_mbc_shift 	= 24,
} LS1x_gmac_Addr1_high_t;

/*
 * GMAC Address1 Low Register 0x004C
 */

/*
 * AN Control Register 0x00C0
 */
typedef enum
{
	gmac_AN_ctrl_lr				= bit(17),		/* Lock to Reference 锁定到参考时钟. =1: PHY将其锁相环锁定到125MHz的参考时钟 */
	gmac_AN_ctrl_ecd			= bit(16),		/* Enable Comma Detect 使能停顿探测. =1: 使能PHY的停顿探测和字重同步 */
	gmac_AN_ctrl_ele			= bit(14),		/* External Loopback Enable 外部环回使能. =1: 使能PHY进入环回模式 */
	gmac_AN_ctrl_enable			= bit(12),		/* Auto-Negotiation Enable 自动协商使能. =1: GMAC将会和链路对方进行自动协商 */

	gmac_AN_ctrl_ran			= bit(9),		/* Restart Auto-Negotiation 重新进行自动协商. =1: 重新进行自动协商 */

} LS1x_gmac_AN_control_t;

/*
 * AN Status Register 0x00C4
 */
typedef enum
{
	gmac_AN_stat_es				= bit(8),		/* Extended Status 扩展状态. 只读, 因为GMAC支持扩展状态信息 */
	gmac_AN_stat_anc			= bit(5),		/* Auto-Negotiation Complete. 只读, 指示自动协商完成 */
	gmac_AN_stat_ana			= bit(3),		/* Auto-Negotiation Ability 自动协商能力. 只读, 因为GMAC支持自动协商 */
	gmac_AN_stat_ls				= bit(2),		/* Link Status 链路状态. =1: 指示链路连接上; =0: 指示链接未连接 */
} LS1x_gmac_AN_status_t;

/*
 * Auto-Negotiation Advertisement Register 0x00C8
 */
typedef enum
{
	gmac_AN_advertise_np		= bit(15),		/* Next Page Support 下一页面支持. 只读为0, 因为GMAC不支持下一页面.  */
	gmac_AN_advertise_rfe_mask	= (0x03<<12),	/* Remote Fault Encoding 远端错误编码 */
	gmac_AN_advertise_rfe_shift	= 12,			/* 此2位指示链路对端发生错误, 具体编码将IEEE 802.3z第37.2.1.5小节 */
	gmac_AN_advertise_pse_mask	= (0x03<<7),	/* Pause Encoding Pause位编码. 见IEEE 802.3z第37.2.1.4小节 */
	gmac_AN_advertise_pse_shift = 7,
	gmac_AN_advertise_hd		= bit(6),		/* Half-Duplex 半双工. =1: 指示GMAC支持半双工 */
	gmac_AN_advertise_fd		= bit(5),		/* Full-Duplex 全双工. =1: 指示GMAC支持全双工 */

} LS1x_gmac_AN_advertise_t;

/*
 * Auto-Negotiation Link Partner Ability Register 0x00CC
 */
typedef enum
{
	gmac_AN_lpa_np			= bit(15),		/* Next Page Support 下一页面支持. =1: 指示有更多下一页面信息可用; =0: 指示下一页面交换不可用 */
	gmac_AN_lpa_ack			= bit(14),		/* Acknowledge 确认. 指示在自动协商中, 链路对端成功接收到GMAC的基本页面 */
	gmac_AN_lpa_rfe_mask	= (0x03<<12),	/* Remote Fault Encoding 远端错误编码. 见IEEE 802.3z第37.2.1.5小节 */
	gmac_AN_lpa_rfe_shift	= 12,
	gmac_AN_lpa_pse_mask	= (0x03<<7),	/* Pause Encoding 对端pause状态编码. 见IEEE 802.3z第37.2.14小节 */
	gmac_AN_lpa_pse_shift	= 7,
	gmac_AN_lpa_hd			= bit(6),		/* Half-Duplex 半双工. 指示对端可以运行在半双工模式 */
	gmac_AN_lpa_fd			= bit(5),		/* Full-Duplex 全双工. 指示对端可以运行在全双工模式 */
} LS1x_gmac_AN_lpa_t;

/*
 * Auto-Negotiation Expansion Register 0x00D0
 */
typedef enum
{
	gmac_AN_exp_npa			= bit(2),		/* Next Page Ability 下一页面能力. 只读为0, 因为GMAC不支持下一页面 */
	gmac_AN_exp_npr			= bit(1),		/* New Page Received 接收到新页面. =1: 指示GMAC接收到新页面 */
} LS1x_gmac_AN_exp_t;

/*
 * SGMII/RGMII Status Register 0x00D8
 */
typedef enum
{
	gmac_mii_linked			= bit(3),		/* Link Status 链路状态. =1: 指示链路连接上; =0: 指示链路未连接上 */
	gmac_mii_speed_mask		= (0x03<<1),	/* Link Speed 链路速度 */
	gmac_mii_speed_shift	= 1,
	gmac_mii_speed_2p5MHZ	= 0x00,			/* 00: 2.5MHz */
	gmac_mii_speed_25MHZ	= 0x20,				/* 01: 25MHz */
	gmac_mii_speed_125MHZ	= 0x40,			/* 10: 125MHz */
	gmac_mii_linkmode		= bit(0),		/* Link Mode 链路模式 */
	gmac_mii_linkmode_hd	= 0,				/* 0: 半双工 */
	gmac_mii_linkmode_fd	= 1,				/* 1: 全双工 */
} LS1x_gmac_mii_status_t;

/*
 * GMII Address Register 0x0010
 * GMII Data Register 0x0014
 */
typedef enum
{
	gmac_miictrl_phyaddr_mask	= (0x1F<<11),		/* bits15~11, PHY Address. 此域选择需要访问32个PHY中的哪个 */
	gmac_miictrl_phyaddr_shift	= 11,
	gmac_miictrl_gmiireg_mask	= (0x1F<<6),		/* bits10~6, GMII Register. 此域选择需要访问的的PHY的哪个GMII配置寄存器 */
	gmac_miictrl_gmiireg_shift	= 6,
	gmac_miictrl_csr_mask		= (0x07<<2),		/* bits4~2, CSR Clock Range. 此域决定MDC时钟是clk_csr_i时钟频率比例 */
	gmac_miictrl_csr_shift		= 2,				/* (CR)CSR Clock Range: */
	gmac_miictrl_csr_5			= 0x00000014,		/* 250-300 MHz */
	gmac_miictrl_csr_4			= 0x00000010,		/* 150-250 MHz */
	gmac_miictrl_csr_3			= 0x0000000C,		/* 5-60 MHz   */
	gmac_miictrl_csr_2			= 0x00000008,		/* 20-35 MHz   */
	gmac_miictrl_csr_1			= 0x00000004,		/* 100-150 MHz */
	gmac_miictrl_csr_0			= 0x00000000,		/* 60-100 MHz  */
	gmac_miictrl_wr				= bit(1),			/* GMII Write. =1: 通过GMII 数据寄存器对PHY进行写操作, =0: 通过GMII数据寄存器对PHY进行读操作 */
	gmac_miictrl_busy			= bit(0),			/* GMII Busy. 对寄存器4和寄存器5写之前, 此位应为0. 在写寄存器4之前此位必须先置0.
                                                       在访问PHY 的寄存器时, 应用程序需要将此位设置为1, 表示GMII接口上有写或者读操作正在进行. */

	gmac_miidata_mask			= 0xFFFF,			/* 此域保存了对PHY进行管理读访问操作的16位数据, 或者对PHY进行管理写访问的16位数据. */

} LS1x_gmac_miictrl_t;

#ifdef LS1C

/******************************************************************************
 * IEEE1588 Registers of GMAC
 ******************************************************************************/

typedef struct LS1C_gmac_ieee1588_s
{
	volatile unsigned int stamp_ctrl;					/* 0xBFE10700 Time Stamp Control Register */
	volatile unsigned int sub_second_inc; 				/* 0xBFE10704 Sub-Second Increment Register */
	volatile unsigned int systm_second; 				/* 0xBFE10708 System Time - Seconds Register */
	volatile unsigned int systm_nanosecond; 			/* 0xBFE1070C System Time - Nanoseconds Register */
	volatile unsigned int systm_second_upd; 			/* 0xBFE10710 System Time - Seconds Update Register */
	volatile unsigned int systm_nanosecond_upd; 		/* 0xBFE10714 System Time - Nanoseconds Update Register */
	volatile unsigned int ts_addend;	 				/* 0xBFE10718 Time Stamp Addend Register */
	volatile unsigned int tgttm_second; 				/* 0xBFE1071C Target Time Seconds Register */
	volatile unsigned int tgttm_nanosecond; 			/* 0xBFE10720 Target Time Nanoseconds Register */
	volatile unsigned int systm_hisecond; 				/* 0xBFE10724 System Time - Higher Word Seconds Register */
	volatile unsigned int ts_status; 					/* 0xBFE10728 Time Stamp Status Register */
	volatile unsigned int pps_auxts_nanosecond; 		/* 0xBFE1072C PPS Auxiliary Time Stamp - Nanoseconds Register */
	volatile unsigned int pps_auxts_second; 			/* 0xBFE10730 PPS Auxiliary Time Stamp - Seconds Register */
} LS1C_gmac_ieee1588_t;

#endif

/******************************************************************************
 * DMA registers of GMAC
 ******************************************************************************/

#define LS1x_GDMA0_BASE			0xBFE11000
#if defined(LS1B)
#define LS1B_GDMA1_BASE			0xBFE21000
#endif

typedef struct LS1x_gdma_regs
{
	volatile unsigned int busmode; 			/* 0xBFE11000 Bus Mode */
	volatile unsigned int txpoll; 			/* 0xBFE11004 Transmit Poll Demand */
	volatile unsigned int rxpoll; 			/* 0xBFE11008 Receive Poll Demand */
	volatile unsigned int rxdesc0; 			/* 0xBFE1100C Start of Receive Descriptor List Address */
	volatile unsigned int txdesc0; 			/* 0xBFE11010 Start of Transmit Descriptor List Address */
	volatile unsigned int status; 			/* 0xBFE11014 Status */
	volatile unsigned int control; 			/* 0xBFE11018 Operation Mode */
	volatile unsigned int intenable; 		/* 0xBFE1101C Interrupt Enable */
	volatile unsigned int mfbocount; 		/* 0xBFE11020 Missed Frame and Buffer Overflow Counter */
#ifdef LS1C
	volatile unsigned int rxintwdtmr; 		/* 0xBFE11024 Receive Interrupt Watchdog Timer Register */
	volatile unsigned int axibusmode; 		/* 0xBFE11028 AXI Bus Mode Register */
	volatile unsigned int axistatus; 		/* 0xBFE1102C AXI Status Register */
	volatile unsigned int rsv2[6];
#else
	volatile unsigned int rsv1[9];          /* XXX LS1B no WD & AXI Register */
#endif
	volatile unsigned int curtxdesc; 		/* 0xBFE11048 Current Host Transmit Descriptor */
	volatile unsigned int currxdesc; 		/* 0xBFE1104C Current Host Receive Descriptor */
	volatile unsigned int curtxbuf; 		/* 0xBFE11050 Current Host Transmit Buffer Address */
	volatile unsigned int currxbuf; 		/* 0xBFE11054 Current Host Receive Buffer Address */
} LS1x_gdma_regs_t;

/*
 * Bus Mode Register of GMAC's DMA 	Offset: 0x00
 */
typedef enum
{
	gdma_busmode_mb			= bit(26),		/* Mixed Burst 混合突发访问 *//* 用户不用关心此位设置 */
	gdma_busmode_aal		= bit(25),		/* Address-Aligned Beats 地址对齐节拍 *//* 用户不用关心此位设置 */
	gdma_busmode_8xpbl		= bit(24),		/* 8XPBL Mode 是否使能PBLX8模式 *//* 用户不用关心此位设置  */
	gdma_busmode_usp		= bit(23),		/* Use Separate PBL 使用分离的PBL值 *//* 用户不用关心此位设置  */
	gdma_busmode_rpbl_mask	= (0x3F<<17),	/* RxDMA PBL RxDMA突发传输长度 */
	gdma_busmode_rpbl_shift = 17,			/* 表示一次RxDMA传输的最大突发传输长度. 只能为1,2,4,8,16和32, 其它值无效*/
	gdma_busmode_fb			= bit(16),		/* Fixed Burst 定长突发传输长度 *//* 用户不用关心此位设置 */
	gdma_busmode_pr_mask	= (0x03<<14),	/* Rx:Tx priority ratio, RxDMA与TxDMA优先级比例, 在DA位为1时起作用  */
	gdma_busmode_pr_shift   = 14,
	gdma_busmode_pr_1		= (0<<14),		/* (PR)TX:RX DMA priority ratio 1:1 */
	gdma_busmode_pr_2		= bit(14),		/* (PR)TX:RX DMA priority ratio 2:1 */
	gdma_busmode_pr_3		= (2<<14),		/* (PR)TX:RX DMA priority ratio 3:1 */
	gdma_busmode_pr_4		= (3<<14),		/* (PR)TX:RX DMA priority ratio 4:1 */
	gdma_busmode_pbl_mask	= (0x3f<<8),	/* Programmable Burst Length 可编程突发传输长度 *//* 用户不用关心此位设置 */
	gdma_busmode_pbl_shift	= 8,
	gdma_busmode_pbl_256	= 0x01002000,	/* (DmaBurstLengthx8 | DmaBurstLength32) = 256 */
	gdma_busmode_pbl_128	= 0x01001000,	/* (DmaBurstLengthx8 | DmaBurstLength16) = 128 */
	gdma_busmode_pbl_64		= 0x01000800,	/* (DmaBurstLengthx8 | DmaBurstLength8)  = 64 */
	gdma_busmode_pbl_32		= bit(13), 		/* Dma burst length = 32 */
	gdma_busmode_pbl_16		= bit(12), 		/* Dma burst length = 16 */
	gdma_busmode_pbl_8		= bit(11), 		/* Dma burst length = 8 */
	gdma_busmode_pbl_4		= bit(10), 		/* Dma burst length = 4 */
	gdma_busmode_pbl_2		= bit(9),		/* Dma burst length = 2 */
	gdma_busmode_pbl_1		= bit(8),		/* Dma burst length = 1 */
	gdma_busmode_pbl_0		= 0,			/* Dma burst length = 0 */
	gdma_busmode_atds		= bit(7),		/* Alternate Descriptor size, =1: 使用32字节大小的描述符, =0: 使用16字节大小的描述符 */
	gdma_busmode_dsl_mask	= (0x1F<<2),	/* Descriptor Skip Length, 设置2个描述符间的距离. 但此值为0时, 默认为DMA描述符大小 */
	gdma_busmode_dsl_shift  = 2,
	gdma_busmode_dsl_16		= bit(6),
	gdma_busmode_dsl_8		= bit(5),
	gdma_busmode_dsl_4		= bit(4),
	gdma_busmode_dsl_2		= bit(3),
	gdma_busmode_dsl_1		= bit(2),
	gdma_busmode_dsl_0		= 0,
	gdma_busmode_das		= bit(1),		/* DMA Arbitration scheme, =0: 在RxDMA和TxDMA间采用轮转仲裁机制, =1: RxDMA优先级高于TxDMA优先级(PR值) */
	gdma_busmode_swreset	= bit(0),		/* 此位置高DMA控制器将复位GMAC内部寄存器和逻辑. 当复位结束时该位自动清零 */
} LS1x_gdma_busmode_t;

/*
 * Status Register of GMAC's DMA 	Offset: 0x14
 */
typedef enum
{
	gdma_status_tti			= bit(29),		/* Time-Stamp Trigger Interrupt, 时间戳模块触发中断. 只读 */
	gdma_status_gpi			= bit(28),		/* GMAC PMT Interrupt, 电源管理模块触发中断. 只读 */
	gdma_status_gmi			= bit(27),		/* GMAC MMC Interrupt, MMC模块触发中断. 只读 */
	gdma_status_gli			= bit(26),		/* GMAC Line interface Interrupt, GMAC模块的PCS或者RGMII模块触发中断. 只读 */
	gdma_status_eb_mask		= (0x07<<23),	/* Error Bits, 错误位(参考编码表) */
	gdma_status_eb_shift    = 23,
	gdma_status_eb_25		= bit(25),		/* =1: 描述符访问错误; =0: 数据缓存访问错误 */
	gdma_status_eb_24		= bit(24),		/* =1: 读传输错误; =0: 写传输错误 */
	gdma_status_eb_23		= bit(23),		/* =1: TxDMA数据传输过程中发生错误; =0: RxDMA数据传输过程中发生错误 */
	gdma_status_txs_mask	= (0x07<<20),	/* Transmit Process State, 传输过程状态(参考编码表) */
	gdma_status_txs_shift   = 20,
	gdma_status_txs_0		= (0<<20),		// 000: 传输停止; 复位或者停止命令发送
	gdma_status_txs_1		= (1<<20),		// 001: 正在进行; 获取传输描述符
	gdma_status_txs_2		= (2<<20),		// 010: 正在进行; 等待传输状态
	gdma_status_txs_3		= (3<<20),		// 011: 正在进行; 从发送缓存读取数据并发送到传输FIFO(TxFIFO)
	gdma_status_txs_4		= (4<<20),		// 100: 写入时间戳状态
	gdma_status_txs_5		= (5<<20),		// 101: 保留
	gdma_status_txs_6		= (6<<20),		// 110: 挂起; 传输描述符不可用或者传输缓存下溢.
	gdma_status_txs_7		= (7<<20),		// 111: 运行; 关闭传输描述符.
	gdma_status_rxs_mask	= (0x07<<17),	/* bits: 19-17, Receive Process State, 接收过程状态(参考编码表) */
	gdma_status_rxs_shift   = 17,
	gdma_status_rxs_0		= (0<<17),		// 000: 停止; 复位或者接收到停止命令
	gdma_status_rxs_1		= (1<<17),		// 001: 运行; 获取接收描述符.
	gdma_status_rxs_2		= (2<<17),		// 010: 保留;
	gdma_status_rxs_3		= (3<<17),		// 011: 运行; 等待接收包.
	gdma_status_rxs_4		= (4<<17),		// 100: 暂停; 接收描述符不可用.
	gdma_status_rxs_5		= (5<<17),		// 101: 运行; 关闭接收描述符.
	gdma_status_rxs_6		= (6<<17),		// 110: 时间戳写状态.
	gdma_status_rxs_7		= (7<<17),		// 111: 运行; 将包内容从接收缓存传输到系统内存.
	gdma_status_nis			= bit(16),		/* Normal Interrupt Summary, 正常中断汇总, 提示系统是否存在正常中断 */
	gdma_status_ais			= bit(15),		/* Abnormal Interrupt Summary, 异常中断汇总, 提示系统是否存在异常中断 */
	gdma_status_erxi		= bit(14),		/* Early Receive Interrupt, 提前接收中断, 提示DMA控制器已经把包的第一个数据写入接收缓存 */
	gdma_status_fbei		= bit(13),		/* Fatal Bus Error Interrupt, 总线错误中断, 提示总线错误, 具体信息见[25:23]. 当此位设置后DMA引擎停止总线访问操作 */
	gdma_status_etxi		= bit(10),		/* Early Transmit Interrupt, 提前发送中断, 提示需要传输的以太网帧已经完全传输到MTL模块中的传输 FIFO */
	gdma_status_rxwt		= bit(9),		/* Receive Watchdog Timeout, 接收看门狗超时, 提示接收到一个大小超过2048字节的以太网帧 */
	gdma_status_rxstop		= bit(8),		/* Receive Process Stopped, 接收过程停止 */
	gdma_status_rxbufu		= bit(7),		/* Receive Buffer Unavailable, 接收缓存不可用 */
	gdma_status_rxi			= bit(6),		/* Receive Interrupt, 接收中断, 指示帧接收完成, 帧接收的状态信息已经写入接收描述符, 接收处于运行状态 */
	gdma_status_txunf		= bit(5),		/* Transmit Underflow, 传输缓存下溢, 指示帧发送过程中产生接收缓存下溢 */
	gdma_status_rxovf		= bit(4),		/* Receive Overflow, 接收缓存上溢, 指示帧接收过程中接收缓存上溢 */
	gdma_status_txjt		= bit(3),		/* Transmit Jabber Timeout */
	gdma_status_txbufu		= bit(2),		/* Transmit Buffer Unavailable, 传输缓存不可用, 提示传输列表中的下一个描述符不能被DMA控制器访问 */
	gdma_status_txstop		= bit(1),		/* Transmit Process Stopped, 传输过程停止 */
	gdma_status_txi			= bit(0),		/* Transmit Interrupt, 传输完成中断, 提示帧传输完成并且第一个描述符的31位置位 */
} LS1x_gdma_status_t;

/*
 * Operation Mode Register of GMAC's DMA 	Offset: 0x18
 */
typedef enum
{
	gdma_ctrl_droptcpcse	= bit(26),		/* 关闭丢弃TCP/IP Checksum错误以太网帧的功能, =1: GMAC将不丢弃checksum错误的以太网帧 */
	gdma_ctrl_rxsf			= bit(25),		/* Receive Store and Forward, 接收存储转发, =1: MTL模块只接收已经全部存储在接收FIFO中的以太网帧 */
	gdma_ctrl_disfrxf		= bit(24),		/* Disable Flushing of Received Frames, 关闭冲刷接收的以太网帧的功能,
	 	 	 	 	 	 	 	 	 	 	   =1: 接收DMA在接收描述符或者接收缓存不可用时不冲刷任何以太网帧 */
	gdma_ctrl_RFA_2			= bit(23),		/* RFA[2]: MSB of Threshold for Activating Flow Control 激活流控阈值 */
	gdma_ctrl_RFD_2			= bit(22),		/* RFD[2]: MSB of Threshold for Deactivating Flow Control 关闭流控阈值 */
	gdma_ctrl_txsf			= bit(21),		/* Transmit Store and Forward, 发送存储转发, =1: 帧的发送只在帧的内容已经全部进入MTL的传输FIFO中 */
	gdma_ctrl_ftxfifo		= bit(20),		/* Flush Transmit FIFO, 冲刷传输FIFO, =1: 传输控制逻辑复位为默认值, 并且会导致发送FIFO里面的数据全部丢失 */
	gdma_ctrl_ttc_mask		= (0x7<<14),	/* Transmit Threshold Control, 传输阈值控制, 当帧大小超过此值时MTL将会传输该帧 */
	gdma_ctrl_ttc_shift		= 14,
	gdma_ctrl_ttc_64		= (0<<14),		// 000: 64  字节
	gdma_ctrl_ttc_128		= (1<<14),		// 001: 128 字节
	gdma_ctrl_ttc_192		= (2<<14),		// 010: 192 字节
	gdma_ctrl_ttc_256		= (3<<14),		// 011: 256 字节
	gdma_ctrl_ttc_40		= (4<<14),		// 100: 40  字节
	gdma_ctrl_ttc_32		= (5<<14),		// 101: 32  字节
	gdma_ctrl_ttc_24		= (6<<14),		// 110: 24  字节
	gdma_ctrl_ttc_16		= (7<<14),		// 111: 16  字节
	gdma_ctrl_txstart		= bit(13),		/* Start/Stop Transmission Command, =1: 传输进入运行状态, =0: 传输进入停止状态 */
	gdma_ctrl_RFD			= (0x03<<11),	/* RFD: Threshold for deactivating flow control 关闭流控阈值 */
	gdma_ctrl_RFA			= (0x03<<9),	/* RFA:Threshold for Activating flow control 激活流控阈值 */
	gdma_ctrl_enhwfc		= bit(8),		/* Enable HW flow control, =1: 基于接收FIFO利用率的硬件流控电路生效 */
	gdma_ctrl_ferrf			= bit(7),		/* Forward Error Frames, 传输错误帧, =1: 接收错误帧(错误帧包括:CRC错误,冲突错误,巨帧,看门狗超时,溢出等) */
	gdma_ctrl_fugf			= bit(6),		/* Forward Undersized Good Frames, =1: 接收FIFO将会接收没有错误但小于64字节的以太网帧 */
	gdma_ctrl_rtc_mask		= (0x3<<3),		/* Receive Threshold Control, 接收阈值控制, 当帧大小超过此值时MTL将会接收该帧 */
	gdma_ctrl_rtc_shift		= 3,
	gdma_ctrl_rtc_64		= (0<<3),		// 00: 64  字节
	gdma_ctrl_rtc_32		= (1<<3),		// 01: 32  字节
	gdma_ctrl_rtc_96		= (2<<3),		// 10: 96  字节
	gdma_ctrl_rtc_128		= (3<<3),		// 11: 128 字节
	gdma_ctrl_txopsecf		= bit(2),		/* TX Operate on Second Frame, =1: DMA在第一个以太网帧的状态尚未写回时即可以开始处理第二个以太网帧 */
	gdma_ctrl_rxstart		= bit(1),		/* Start/Stop Receive, =1: 接收进入运行状态, =0: 接收进入停止状态 */
} LS1x_gdma_ctrl_t;

/*
 * Interrupt Enable Register of GMAC's DMA 	Offset: 0x1C
 */
typedef enum
{
	gdma_ienable_nis		= bit(16),		/* Normal Interrupt Summary Enable, =1: 正常中断使能, =0: 正常中断不使能 */
	gdma_ienable_ais		= bit(15),		/* Abnormal Interrupt Summary Enable, =1: 非正常中断使能, =0: 非正常中断不使能 */
	gdma_ienable_erxi		= bit(14),		/* Early Receive Interrupt Enable, 早期接收中断使能, =1: 早期接收中断使能 */
	gdma_ienable_fbei		= bit(13),		/* Fatal Bus Error Enable, =1: 总线致命错误中断使能 */
	gdma_ienable_etxi		= bit(10),		/* Early Transmit Interrupt Enable, =1: 使能早期传输中断 */
	gdma_ienable_rxwdog		= bit(9),		/* Receive Watchdog Timeout Enable, =1: 使能接收看门狗超时中断 */
	gdma_ienable_rxstop		= bit(8),		/* Receive Stopped Enable, =1: 使能接收停止中断 */
	gdma_ienable_rxbufu		= bit(7),		/* Receive Buffer Unavailable Enable, =1: 使能接收缓冲区不可用中断 */
	gdma_ienable_rxi		= bit(6),		/* Receive Interrupt Enable, =1: 使能接收完成中断 */
	gdma_ienable_txunf		= bit(5),		/* Underflow Interrupt Enable, =1: 使能传输FIFO下溢中断 */
	gdma_ienable_rxovf		= bit(4),		/* Overflow Interrupt Enable, =1: 使能接收FIFO上溢中断 */
	gdma_ienable_txjt		= bit(3),		/* Transmit Jabber Timeout Enable, =1: 使能Jabber超时中断 */
	gdma_ienable_txbufu		= bit(2),		/* Transmit Buffer Unavailable Enable, =1: 使能传输缓存不可用中断 */
	gdma_ienable_txstop		= bit(1),		/* Transmit Stopped Enable, =1: 使能传输停止中断 */
	gdma_ienable_txi		= bit(0),		/* Transmit Interrupt Enable, =1: 使能传输完成中断 */

	gdma_ien_base = (gdma_ienable_nis | gdma_ienable_ais | gdma_ienable_fbei),
	gdma_ien_rx	  = (gdma_ienable_rxstop | gdma_ienable_rxi | gdma_ienable_rxbufu),
	gdma_ien_tx	  = (gdma_ienable_txstop | gdma_ienable_txi | gdma_ienable_txunf),

} LS1x_gdma_intr_en_t;

/*
 * Missed Frame and Buffer Overflow Counter Register of GMAC's DMA 	Offset: 0x20
 */
typedef enum
{
	gdma_mfbocnt_ovfcnt			= bit(28),		/* FIFO 溢出指示位 */
	gdma_mfbocnt_fmcnt_mask		= (0x07FF<<17),	/* 指示应用程序丢失帧的个数 */
	gdma_mfbocnt_fmcnt_shift    = 17,
	gdma_mfbocnt_ovfmcnt		= bit(16),		/* 提示丢失帧个数已经超过计数的最大值 */
	gdma_mfbocnt_rxufmcnt_mask	= (0xFFFF<<0),	/* 指示因为主机接收缓存不可用导致帧丢失个数的计数 */
} LS1x_gdma_mfbo_cnt_t;

#ifdef LS1C

/*
 * Receive Interrupt Watchdog Timer Register
 */
typedef enum
{
	gdma_rxintwdtmr_riwt_mask	= 0xFF,			/* RI Watchdog Timer count, 接收看门狗时间计数 */
	gdma_rxintwdtmr_riwt_shift  = 0,			/* 当看门狗设置后, 表示以时钟周期x256的时间为单位计时.
												   当DMA接收到数据包且status寄存器中RI位为0时开始计时,
												   当看门狗计数超时后RI位置1. 当RI位为1后该域复位 */
} LS1C_gdma_rxintwdtmr_t;

/*
 * AXI Bus Mode Register
 */
typedef enum
{
	gdma_axibusmode_en_lpi		= bit(31),		/* Enable LPI (Low Power Interface), 启用低功耗接口 */
	gdma_axibusmode_unlock		= bit(30), 		/* Unlock on Magic Packet or Remote Wake Up, 解锁魔法包或远程唤醒包 */
												/* =1: GMAC处于低功耗(Low Power)状态下时只能通过魔法包或者远程唤醒包来重新回到工作状态;
												 * =0: GMAC处于低功耗(Low Power)状态下时只能通过任意包重新回到工作状态
												 */
	gdma_axibusmode_wrlmt_mask	= (0x07<<20),	/* AXI Maximum Write Out Standing Request Limit, AXI最大outstanding写请求 */
	gdma_axibusmode_wrlmt_shift = 20,			/* 该位设置了AXI接口写操作发出最大的outstanding请求数 */
	gdma_axibusmode_rdlmt_mask	= (0x07<<16),	/* AXI Maximum Read Out Standing Request Limit, AXI最大outstanding读请求 */
	gdma_axibusmode_rdlmt_shift = 16,			/* 该位设置了AXI接口读操作发出最大的outstanding请求数 */
	gdma_axibusmode_addr_align  = bit(12),		/* Address-Aligned Beats, AXI地址对齐, 只读 */
	gdma_axibusmode_blen_256	= bit(7),		/* AXI Burst Length 256, AXI Burst长度256 */
	gdma_axibusmode_blen_128	= bit(6),
	gdma_axibusmode_blen_64		= bit(5),
	gdma_axibusmode_blen_32		= bit(4),
	gdma_axibusmode_blen_16		= bit(3),
	gdma_axibusmode_blen_8		= bit(2),
	gdma_axibusmode_blen_4		= bit(1),
	gdma_axibusmode_blen_undef	= bit(0),		/* AXI Undefined Burst Length, 未定义AXI Burst长度 */
} LS1C_gdma_axibusmode_t;

/*
 * AXI Status Register
 */
typedef enum
{
	gdma_axistatus_rd_request	= bit(1),		/* =1: 表示当前AXI正在发送读请求 */
	gdma_axistatus_wr_request	= bit(0),		/* =1: 表示当前AXI正在发送写请求 */
} LS1C_gdma_axistatus_t;

#endif

/******************************************************************************
 * Receive Descriptor RDES0 - Status
 */

typedef enum
{
	gdma_rxdesc_stat_own		= bit(31),		/* OWN, =1: 描述符当前属于DMA控制, =0: 属于主机控制. 当DMA模块完成一次传输时, 会将该位主动清0 */
	gdma_rxdesc_stat_afm		= bit(30),		/* Destination Address Filter Fail, 目标地址过滤错误. =1: 当前数据帧目标地址不符合GMAC内部的帧目标地址过滤器 */
	gdma_rxdesc_stat_fr_mask    = (0x3FFF<<16), /* bit29~16, Frame length 帧长度, 表示接收当前帧的长度, 当ES位为0时有效 */
	gdma_rxdesc_stat_fr_shift   = 16,
	gdma_rxdesc_stat_es      	= bit(15),		/* Error Summary 总体错误信息, 指示当前帧是否出错, 其值为RDES[0,1,3,4,6,7,11,14]各位作或运算(OR)的结果 */
	gdma_rxdesc_stat_de			= bit(14),		/* Descriptor Error 描述符错误, =1: 当前描述符所指向的buffer与帧不相符或者OWN为0(主机控制) */
	gdma_rxdesc_stat_saf		= bit(13),		/* Source Address Filter Fail 源地址过滤错误, =1: 当前数据帧的源地址不符合GMAC内部的帧源地址过滤器 */
	gdma_rxdesc_stat_le			= bit(12),		/* Length Error 长度错误, =1: 当前接收帧长度与默认长度不符. 当Frame Type位为1且CRC Error位为0时有效 */
	gdma_rxdesc_stat_oe			= bit(11),		/* Over Flow Error 溢出错误, =1: 接收该帧时GMAC内部RxFIFO溢出 */
	gdma_rxdesc_stat_vlan		= bit(10),		/* VLAN Tag VLAN标志, =1: 该帧的类型为VLAN */
	gdma_rxdesc_stat_fs			= bit(9),		/* First Desciptor 第一个描述符, =1: 当前描述符所指向的buffer为当前接收帧的第一个保存buffer */
	gdma_rxdesc_stat_ls			= bit(8),		/* Last Desciptor 最后一个描述符, =1: 当前描述符所指向的buffer为当前接收帧的最后一个保存buffer */
	gdma_rxdesc_stat_ipce_gf	= bit(7),		/* IPC Checksum Error/Giant Frame 校验错误/超长帧.
	 	 	 	 	 	 	 	 	 	 	 	   =1: 如果IPC校验功能启用则表示当前帧的IPv4头校验值与帧内部校验域的值不相符.
	 	 	 	 	 	 	 	 	 	 	 	 	   如果未启用则表示当前帧为一个超长帧(长度大于1518字节) */
	gdma_rxdesc_stat_lc			= bit(6),		/* Late Collision 后期冲突, =1: 在半双工模式下, 当前帧接收时发生了一个后期冲突 */
	gdma_rxdesc_stat_ft			= bit(5),		/* Frame Type 帧类型, =1: 当前帧为一个以太网格式帧, =0: 当前帧为一个IEEE802.3格式帧 */
	gdma_rxdesc_stat_rwt		= bit(4),		/* Receive Watchdog Timeout, =1: 当前时钟值超过了接收模块看门狗电路时钟的值, 既接收帧超时 */
	gdma_rxdesc_stat_re			= bit(3),		/* Receive Error 接收错误, =1: 接收当前帧时内部模块出错. 内部信号rxer置1且rxdv置1 */
	gdma_rxdesc_stat_dbe		= bit(2),		/* Dribble bit Error 奇数位错误, =1: 接收帧长度不是整数, 即总长度为奇数位, 该位只有在mii模式下有效 */
	gdma_rxdesc_stat_ce			= bit(1),		/* CRC Error 接收CRC校验错误, =1: 接收当前帧时内部CRC校验出错. 该位只有在last descriptor(RDES0[8])为1时有效 */
	gdma_rxdesc_stat_rmpce		= bit(0),		/* RX GMAC Checksum/payload Checksum Error 接受校验/负载校验错误.
												   =1: 接收当前帧时内部RX GMAC寄存器组1-15中存在一个匹配当前帧目的地址.
												   =0: RX GMAC 寄存器组0匹配接受帧目的地址. 如果Full Checksum Offload Engine启用时,
												   =1: 该帧TCP/UDP/ICMP校验错误. 该位为1时也可能表示当前帧实际接受长度与帧内部记载长度不相符. */
} LS1x_gdma_rxdesc_stat_t;

/******************************************************************************
 * Receive Descriptor RDES1 - Control, Address
 */
typedef enum
{
	gdma_rxdesc_ctrl_di			= bit(31),		/* 1: Disable Interrupt in Completion */
	gdma_rxdesc_ctrl_userdef	= bit(30),		/* XXX 自定义, 用于循环控制 */
	gdma_rxdesc_ctrl_rer		= bit(25),		/* Receive End of Ring, =1: 该描述符为环型描述符链表的最后一个, 下一个描述符的地址为接收描述符链的基址 */
	gdma_rxdesc_ctrl_rch		= bit(24),		/* Second Address Chained, =1: 描述符中的第二个buffer地址指向的是下一个描述符的地址 */
	gdma_rxdesc_ctrl_rbs2_mask	= (0x07FF<<11),	/* bits: 21-11, Receive Buffer Size 2, 该域表示数据buffer2的大小 */
	gdma_rxdesc_ctrl_rbs2_shift = 11,
	gdma_rxdesc_ctrl_rbs1_mask	= (0x07FF<<0),	/* bits: 10-0, Receive Buffer Size 1, 该域表示数据buffer1的大小 */
} LS1x_gdma_rxdesc_ctrl_t;

/******************************************************************************
 * Transmit Descriptor TDES0 - Status
 */
typedef enum
{
	gdma_txdesc_stat_own		= bit(31),		/* OWN, =1: 表示描述符当前属于DMA控制, 0表示属于主机控制. 当DMA模块完成一次传输时, 会将该位主动清0 */
	gdma_txdesc_stat_ttss		= bit(17),		/* Tx Time Stamp Status, 当IEEE1588功能启用时, =1: TDES2和TDES3中保存了该发送帧的时间戳信息. 否则该位保留 */
	gdma_txdesc_stat_ihe		= bit(16),		/* IP Header Error, =1: 表示内部校验模块发现该发送帧的IP头出错, 并且不会对该域做任何修改 */
	gdma_txdesc_stat_es			= bit(15),		/* Error Summary, 指示当前帧是否出错, 其值为TDES[1,2,8,9,10,11,13,14]各位作或运算(OR)的结果 */
	gdma_txdesc_stat_jt			= bit(14),		/* Jabber Timeout, =1: 表示GMAC发送模块遇到了Jabber超时 */
	gdma_txdesc_stat_ff			= bit(13),		/* Frame Flushed, =1: 表示软件发出了一个刷新命令导致DMA/MTL将其内部的帧刷新掉 */
	gdma_txdesc_stat_pce		= bit(12),		/* Payload Checksum Error, =1: 表示内部负载校验模块再向发送帧中插入校验数据时出错. 当负载校验模块启用时, 该位有效 */
	gdma_txdesc_stat_lc			= bit(11),		/* Loss of Carrier, =1: 表示在发送该帧过程中载波丢失(gmii_crs信号多个周期未置起) */
	gdma_txdesc_stat_nc			= bit(10),		/* No Carrier, =1: 表示在发送过程中, PHY的载波信号一直未置起 */
	gdma_txdesc_stat_lco		= bit(9),		/* Late Collision, =1: 表示在半双工模式下, 当前帧接收时发生了一个后期冲突 */
	gdma_txdesc_stat_ec			= bit(8),		/* Excessive Collison, =1: 表示在发送当前帧的时候连续出现了16次冲突 */
	gdma_txdesc_stat_vf			= bit(7),		/* VLAN Frame, =1: 表示当前发送帧为一个VLAN帧 */
	gdma_txdesc_stat_cc_mask	= (0x0F<<3),	/* bits: 6-3, Collsion Count, 该域表示当前帧在成功发送之前所遇到冲突次数的总数 */
	gdma_txdesc_stat_cc_shift   = 3,
	gdma_txdesc_stat_ed			= bit(2),		/* Excessive Deferral, =1: 表示当前帧传输结束 */
	gdma_txdesc_stat_uf			= bit(1),		/* Underflow Error, =1: 表示当前帧传输时发生了溢出错误, 即数据传输buffer过小或不可用 */
	gdma_txdesc_stat_db			= bit(0),		/* Defered Bit, =1: 表示此次发送被延迟, 只有在半双工模式下有效 */
} LS1x_gdma_txdesc_stat_t;

/******************************************************************************
 * Transmit Descriptor TDES1 - Control, Address
 */
typedef enum
{
	gdma_txdesc_ctrl_ic			= bit(31),		/* Interrption on Complete, =1: 表示该帧接发送完成后将会置起STATUS寄存器中TI位(CSR5[0]) */
	gdma_txdesc_ctrl_ls			= bit(30),		/* Last Segment, =1: 表示当前buffer包含的是一帧数据的最后一段(如果帧分为多个段) */
	gdma_txdesc_ctrl_fs			= bit(29),		/* First Segment, =1: 表示当前buffer包含的是一帧数据的第一段(如果帧分为多个段) */
	gdma_txdesc_ctrl_cic_mask	= (0x02<<27),	/* bits: 28-27, Checksum Insertion Control, 该域控制内部模块是否在发送帧中填充校验数据 */
	gdma_txdesc_ctrl_cic_shift  = 27,
	gdma_txdesc_ctrl_cic_ipv4	= bit(27),
	gdma_txdesc_ctrl_cic_tcp	= bit(28),
	gdma_txdesc_ctrl_cic_all	= (0x02<<27),
	gdma_txdesc_ctrl_dc			= bit(26),		/* =1, Disable CRC, =1: GMAC硬件不在每个发送帧的结尾添加CRC校验数据 */
	gdma_txdesc_ctrl_ter		= bit(25),		/* Transmit End of Ring, =1: 表示该描述符为环型描述符链表的最后一个, 下一个描述符的地址为发送描述符链的基址 */
	gdma_txdesc_ctrl_tch		= bit(24),		/* Second Address Chained, =1: 表示描述符中的第二个buffer地址指向的是下一个描述符的地址  */
	gdma_txdesc_ctrl_dp			= bit(23),		/* Dissable Pading, =1: 表示GMAC将不会对长度小于64字节的数据包进行空数据填充 */
	gdma_txdesc_ctrl_ttse		= bit(22),		/* Transmit Time Stamp Enable, =1: 表示将启用内部模块计算IEEE1588硬件时间戳计算, 在TDES1[29]为1时有效 */
	gdma_txdesc_ctrl_tbs2_mask	= (0x07FF<<11),	/* bits: 21-11, Transmit Buffer Size 2, 该域表示数据buffer2的大小. 当TDES1[24]为1时, 该域无效 */
	gdma_txdesc_ctrl_tbs2_shift = 11,
	gdma_txdesc_ctrl_tbs1_mask	= (0x07FF<<0),	/* bits: 10-0, Transmit Buffer Size 1, 该域表示数据buffer1的大小. 该域一直有效 */
} LS1x_txdesc_ctrl_t;

/*
 * GMAC DMA Receive and Transmit Descriptor
 * __attribute__((packed))的作用: 告诉编译器取消结构在编译过程中的优化对齐,按照实际占用字节数进行对齐---是GCC特有的语法
 */

typedef struct gmac_dma_desc
{
	volatile unsigned int status;     	/* Status */
	volatile unsigned int control;		/* 31-22: Control; 21-11: Buffer 2 lenght; 10-0: Buffer 1 length */
	volatile unsigned int bufptr;		/* Buffer 1 pointer */
	volatile unsigned int nextdesc;		/* Next descriptor pointer (Dma-able) in chain structure */
	volatile unsigned int temp1;
	volatile unsigned int temp2;
	volatile unsigned int temp3;
	volatile unsigned int temp4;		/* Total: 32 Bytes */
} __attribute__((packed)) LS1x_gdma_desc_t;

typedef LS1x_gdma_desc_t 	LS1x_rxdesc_t;
typedef LS1x_gdma_desc_t 	LS1x_txdesc_t;

/*
 * mii status defination
 */
typedef enum
{
	LINKDOWN	= 0,
	LINKUP		= 1,
} mii_linkstate;

typedef enum
{
	HALFDUPLEX	= 1,
	FULLDUPLEX	= 2,
} mii_duplexmode;

typedef enum
{
	SPEED10     = 1,
	SPEED100    = 2,
	SPEED1000   = 3,
} mii_speed;

/*
 * mac-dma desc mode
 */
typedef enum
{
	RINGMODE	= 0,
	CHAINMODE	= 1,
} gdma_descmode;

/******************************************************************************
 * macro for operate
 ******************************************************************************/

/*
 * start gmac-dma
 */
#define GDMA_START_RX(hwGDMA) \
        do { \
            hwGDMA->control |= gdma_ctrl_rxstart; \
            ls1x_sync(); \
        } while (0)

#define GDMA_START_TX(hwGDMA) \
        do { \
            hwGDMA->control |= gdma_ctrl_txstart; \
            ls1x_sync(); \
        } while (0)

#define GDMA_START(hwGDMA) \
        do { \
            hwGDMA->control |= (gdma_ctrl_txstart | gdma_ctrl_rxstart); \
            ls1x_sync(); \
        } while (0)

/*
 * stop gmac-dma
 */
#define GDMA_STOP_RX(hwGDMA) \
        do { \
            hwGDMA->control &= ~gdma_ctrl_rxstart; \
            ls1x_sync(); \
        } while (0)

#define GDMA_STOP_TX(hwGDMA) \
        do { \
            hwGDMA->control &= ~gdma_ctrl_txstart; \
            ls1x_sync(); \
        } while (0)

#define GDMA_STOP(hwGDMA) \
        do { \
            hwGDMA->control &= ~(gdma_ctrl_txstart | gdma_ctrl_rxstart); \
            ls1x_sync(); \
        } while (0)

/*
 * gmac-dma interrupt
 */
#define GDMA_INT_EN(hwGDMA) \
        do { \
            hwGDMA->intenable = (gdma_ien_base | gdma_ien_rx | gdma_ien_tx); \
            ls1x_sync(); \
        } while (0)

#define GDMA_INT_DIS(hwGDMA) \
        do { \
            hwGDMA->intenable = 0; \
            ls1x_sync(); \
        } while (0)

/*
 * gmac start
 */
#define GMAC_START_TX(hwGMAC) \
        do { \
            hwGMAC->control |= gmac_ctrl_te; \
            ls1x_sync(); \
        } while (0)

#define GMAC_START_RX(hwGMAC) \
        do { \
            hwGMAC->control |= gmac_ctrl_re; \
            ls1x_sync(); \
        } while (0)

#define GMAC_START(hwGMAC) \
        do { \
            hwGMAC->control |= (gmac_ctrl_te | gmac_ctrl_re); \
            ls1x_sync(); \
        } while (0)

/*
 * gmac stop
 */
#define GMAC_STOP_TX(hwGMAC) \
        do { \
            hwGMAC->control &= ~gmac_ctrl_te; \
            ls1x_sync(); \
        } while (0)

#define GMAC_STOP_RX(hwGMAC) \
        do { \
            hwGMAC->control &= ~gmac_ctrl_re; \
            ls1x_sync(); \
        } while (0)

#define GMAC_STOP(hwGMAC) \
        do { \
            hwGMAC->control &= ~(gmac_ctrl_te | gmac_ctrl_re); \
            ls1x_sync(); \
        } while (0)

#ifdef __cplusplus
}
#endif

#endif /* LS1X_GMAC_HW_H_ */


