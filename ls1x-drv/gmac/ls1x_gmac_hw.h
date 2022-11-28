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
	gmac_ctrl_tc			= bit(24),		/* Transmit Configuration in RGMII, ʹ��RGMII��·��Ϣ����.
	      	  	  	  	  	  	  	  	  	   =1: �����˫��ģʽ, ��·�ٶ�, ��·�Լ���·����/�Ͽ�����Ϣͨ��RGMII�ӿڴ����PHY */
	gmac_ctrl_wd			= bit(23),		/* Watchdog Disable, �رտ��Ź�.
	 	 	 	 	 	 	 	 	 	 	   =1: GMAC���رս��ն˵Ŀ��Ź���ʱ��, ���Խ������16384�ֽڵ���̫��֡*/
	gmac_ctrl_jd			= bit(22),		/* Jabber Disable, �ر�Jabber��ʱ��.
											   =1: GMAC�رշ��͹����е�Jabber��ʱ��, ���Է������16384�ֽڵ���̫��֡ */
	gmac_ctrl_be			= bit(21),		/* Frame Burst Enable, =1: GMACʹ�ܴ�������е�֡ͻ������ģʽ */
	gmac_ctrl_je			= bit(20),		/* Jumbo Frame Enable - ��֡ʹ��, =1: GMACʹ�ܾ�֡(���9018�ֽ�)�Ľ��� */
	gmac_ctrl_ifg_mask		= (0x7<<17), 	/* Inter-Frame Gap - ��С֡��� */
	gmac_ctrl_ifg_shift		= 17,
	gmac_ctrl_dcrs			= bit(16),		/* Disable Carrier Sense During Transmission, ��������йر��ز���ͻ���
											   =1: GMAC���԰�˫��ģʽ��CRS�źŵļ�� */
	gmac_ctrl_ps			= bit(15),		/* Port Select, 0: GMII (1000Mbps), 1: MII (10/100Mbps) */
	gmac_ctrl_fes			= bit(14),		/* Speed, 0: 10Mbps, 1: 100Mbps */
	gmac_ctrl_do			= bit(13),		/* Disable Receive Own, �رս����Լ���������̫��֡.
											   =1: GMAC�����հ�˫��ģʽ��gmii_txen_o��Ч����̫��֡ */
	gmac_ctrl_lm			= bit(12),		/* Loopback Mode,  =1: GMII/MII�����ڻ���ģʽ�� */
	gmac_ctrl_dm			= bit(11),		/* Duplex Mode, ʹ��ȫ˫��ģʽ
											   =1: GMAC������ȫ˫��ģʽ��, ��ȫ˫��ģʽ�¿���ͬʱ���ͺͽ�����̫��֡ */
	gmac_ctrl_ipc			= bit(10),		/* Checksum Offload, У���ж��ʹ��
											   =1: GMACӲ��������յ���̫��֡�ĸ���(payload). �����IPV4ͷ��У����Ƿ���ȷ */
	gmac_ctrl_dr			= bit(9),		/* Disable Retry, �ر��ش�
											   =1: GMAC��������ͻʱ���ش����ͳ�ͻ����̫��֡, ��ֻ�����ͻ���� */
	gmac_ctrl_lud			= bit(8),		/* Link Up/Down, 0: ��·�Ͽ�, 1: ��·����  */
	gmac_ctrl_acs			= bit(7),		/* Automatic Pad/CRC Stripping, ��̫��֡Pad/CRC�Զ�ȥ��
											   =1: GMAC��ȥ�����յ�����̫��֡��Pad��FCS */
	gmac_ctrl_bl_mask		= (0x03<<5), 	/* Back-Off Limit - ��������, �������ƾ�������slot���ӳ�ʱ�� */
	gmac_ctrl_bl_shift		= 5,
	gmac_ctrl_bl_0			= (0<<5),		// 00: k=min(n,10)
	gmac_ctrl_bl_1			= (1<<5),		// 01: k=min(n,8)
	gmac_ctrl_bl_2			= (2<<5),		// 10: k=min(n,4)
	gmac_ctrl_bl_3			= (3<<5),		// 11: k=min(n,1)
	gmac_ctrl_dc			= bit(4),		/* Deferral Check, =1: ʹ��deferral��⹦�� */
	gmac_ctrl_te			= bit(3),		/* Transmitter Enable, =1: ʹ��GMAC���书�� */
	gmac_ctrl_re			= bit(2),		/* Receiver Enable, =1: ʹ��GMAC���չ��� */
} LS1x_gmac_control_t;

/*
 * GMAC Frame Filter Register 0x0004
 */
typedef enum
{
	gmac_frmfilter_ra		= bit(31),		/* Receive All, =1: GMAC����ģ��ѽ��յ�������֡������Ӧ�ó���, ����Դ��ַ/Ŀ���ַ���˻��� */
	gmac_frmfilter_hpf		= bit(10),		/* Hash or Perfect Filter, ��ϣ������ȫ����
											   =1: �ڹ�ϣ/��ȫ���˻�����ƥ�����̫��֡���͸�Ӧ��.
											   =0: ֻ���ڹ�ϣ���˻�����ƥ�����̫��֡�ŷ��͸�Ӧ��.  */
	gmac_frmfilter_saf		= bit(9),		/* Source Address Filter Enable, Դ��ַ����ʹ��
											   GMAC CORE�ȽϱȽϽ��յ���̫��֡��Դ��ַ�����SA�Ĵ����е�ֵ, ���ƥ��, ����״̬�Ĵ����е�SAMatchλ����Ϊ��.
											   =1: Դ��ַƥ��ʧ��, GMAC CORE����������̫��֡.
											   =0: ����Դ��ַƥ����GMAC CORE�����մ�֡, ��ƥ����д�����״̬�Ĵ���. */
	gmac_frmfilter_saif		= bit(8),		/* SA Inverse Filtering, Դ��ַ��ת����.
											   =1: ��SA�Ĵ�����Դ��ַƥ�����̫��֡������ΪԴ��ַƥ��ʧ��.
											   =0: ��SA�Ĵ�����Դ��ַ��ƥ�����̫��֡������ΪԴ��ַƥ��ʧ��.  */
	gmac_frmfilter_pcf_mask	 = (0x03<<6),	/* bits: 7-6, Pass Control Frames, ���տ���֡ */
	gmac_frmfilter_pcf_shift = 6,
	gmac_frmfilter_pcf_0	 = (0<<6),		// 00: GMAC�������п���֡;
	gmac_frmfilter_pcf_1	 = (1<<6),		// 01: GMAC���ճ���pause֡��������п���֡;
	gmac_frmfilter_pcf_2	 = (2<<6),		// 10: GMAC�������п���֡;
	gmac_frmfilter_pcf_3	 = (3<<6),		// 11: GMAC���ݵ�ַ����������տ���֡ */

	gmac_frmfilter_dbf		= bit(5),		/* Disable Broadcast Frames, �رչ㲥֡. =1: �������н��յĹ㲥֡. =0: �������й㲥֡.  */
	gmac_frmfilter_pm		= bit(4),		/* Pass All Multicast, �������жಥ֡. =1: �������жಥ֡. =0: �������жಥ֡.  */
	gmac_frmfilter_daif		= bit(3),		/* DA Inverse Filtering, Ŀ���ַ��ת����.
											   =1: �Ե����Ͷಥ֡���з���Ŀ���ַƥ��.
											   =0: �Ե����Ͷಥ֡��������Ŀ���ַƥ��.  */
	gmac_frmfilter_hmc		= bit(2),		/* Hash Multicast, ��ϣ�ಥ����, =1: �Խ��յ��Ķಥ֡���ݹ�ϣ������ݽ���Ŀ���ַ���� */
	gmac_frmfilter_huc		= bit(1),		/* Hash Unicast, ��ϣ��������; =1: �Խ��յ��ĵ���֡���ݹ�ϣ������ݽ���Ŀ���ַ���� */
	gmac_frmfilter_pr		= bit(0),		/* Promiscuous Mode, ����ģʽ, =1: ����������̫��֡ */
} LS1x_gmac_frmfilter_t;

/*
 * GMAC Flow Control Register 0x0018
 */
typedef enum
{
	gmac_flowctrl_pt_mask	= (0xFFFF<<16),		/* bits31~16, Pause Time, ��ͣʱ��, ���򱣴�����Ҫ���봫�����֡�е���ͣʱ���� */
	gmac_flowctrl_pt_shift  = 16,
	gmac_flowctrl_dzpq		= bit(7),			/* Disable Zero-Quanta Pause, ��ֹ��ʱ��Ƭ��ͣ֡, =1: ��ֹ�Զ���ʱ��Ƭ����ͣ����֡�Ĳ��� */
	gmac_flowctrl_plt_mask	= (0x03<<4),		/* bits5~4, Pause Low Threshold, ����������ͣʱ�����ֵ */
	gmac_flowctrl_plt_shift = 4,
	gmac_flowctrl_plt_0		= (0<<4),			/* 00: ��ͣʱ�����4��ʱ��� */
	gmac_flowctrl_plt_1		= (1<<4),			/* 01: ��ͣʱ�����28��ʱ��� */
	gmac_flowctrl_plt_2		= (2<<4),			/* 10: ��ͣʱ�����144��ʱ��� */
	gmac_flowctrl_plt_3		= (3<<4),			/* 11: ��ͣʱ�����256��ʱ��� */
												/* һ��ʱ���Ϊ��GMII/MII�ӿ��ϴ���512���ػ���64�ֽڵ�ʱ�� */
	gmac_flowctrl_up		= bit(3),			/* Unicast Pause Frame Detect, ��������ͣ֡̽��,
												   =1: GMAC�������GMAC��ַ0ָ���ı�վ������ַ��̽����ͣ֡ */
	gmac_flowctrl_rxfcen	= bit(2),			/* Receive Flow Control Enable, ��������ʹ��,
												   =1: GMAC����������յ�����ͣ֡, ���Ұ�����ָͣ֡����ʱ����ͣ֡�ķ��� */
	gmac_flowctrl_txfcen	= bit(1),			/* Transmit Flow Control Enable, ��������ʹ��,
												   =1: ��ȫ˫��ģʽ��, GMACʹ����ͣ֡�ķ���; �ڰ�˫��ģʽ��, GMACʹ�ܷ�ѹ����.  */
	gmac_flowctrl_fcb_bpa	= bit(0),			/* Flow Control Busy/Backpressure Activate, ����æ/��ѹ����,
												   =1: ��ȫ˫��ģʽ�·�����ͣ����֡�ķ��ͻ��ڰ�˫��ģʽ��������ѹ���� */
} LS1x_gmac_flowctrl_t;

/*
 * VLAN Tag Register 0x001C
 */
typedef enum
{
	gmac_vlan_tag_evt		= bit(16),			/* Enable 12-Bit VLAN Tag Comparison, ʹ��12λVLAN Tag�Ƚ�.
												   =1: ʹ��12λVLAN Tag������ʹ��16λVLAN Tag������̫��֡�ȽϺ͹���. */
	gmac_vlan_tag_vl_mask	= 0xFFFF,			/* VLAN Tag Identifier for Receive Frames, ֡���յ�VLAN Tag��ʶ.
												      ���򱣴�802.1Q��ʽ��VLAN Tag, ���ڱȽϽ��յ�����̫��֡��λ�ڵ�15�͵�16���ֽڵ�VLAN Tag. */
} LS1x_gmac_vlan_tag_t;

/*
 * GMAC Interrupt Status Register 0x0038
 */
typedef enum
{
	gmac_intstat_mmcerr			= bit(7),		/* MMCУ���ж�ؼĴ��������κ��жϲ���ʱ, ��λ����Ϊ1 */
	gmac_intstat_mmctx			= bit(6),		/* MMC�����жϼĴ��������κ��ж�ʱ, ��λ����Ϊ1 */
	gmac_intstat_mmcrx			= bit(5),		/* MMC�����жϼĴ��������κ��ж�ʱ, ��λ����Ϊ1 */
	gmac_intstat_mmc			= bit(4),		/* MMC�ж�״̬. 7:5���κ�λΪ��ʱ, ��λ����Ϊ1 */
	gmac_intstat_pmt			= bit(3),		/* ��Դ�����ж�״̬, ��PowerDown����WakeUpʱ, ��λ����Ϊ1 */
	gmac_intstat_ancomp			= bit(2),		/* RGMII PHY�ӿ��Զ�Э�����ʱ, ��λ����Ϊ1 */
	gmac_intstat_linkstatus		= bit(1),		/* RGMII PHY�ӿڵ���·״̬�����κα仯ʱ, ��λ����Ϊ1 */
	gmac_intstat_rgmii			= bit(0),		/* RGMII PHY�ӿڵ���·״̬�����κα仯ʱ, ��λ����Ϊ1 */
} LS1x_gmac_intr_status_t;

/*
 * GMAC Interrupt Mask Register 0x003C
 */
typedef enum
{
	gmac_intmask_timestamp		= bit(9),		/* =1: ��ֹʱ����������ж� */
	gmac_intmask_pmt			= bit(3),		/* =1: ��ֹ��Դ����������ж� */
	gmac_intmask_ancomp			= bit(2),		/* =1: ��ֹPCS�Զ�Э������ж� */
	gmac_intmask_linkchange		= bit(1),		/* =1: ��ֹ����PCS��·״̬�仯������ж� */
	gmac_intmask_rgmii			= bit(0),		/* =1: ��ֹRGMII������ж� */
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
	gmac_addr1_hi_ae			= bit(31),		/* Address Enable ��ַʹ��.
												   =1: ��ַ����ģ��ʹ�õ�2��GMAC��ַ������ȫ��ַ����.
												   =0: ��ַ����ģ�鲻ʹ�õ�2��GMAC��ַ���ڵ�ַ����. */
	gmac_addr1_hi_sa			= bit(30),		/* Source Address ԴGMAC��ַ.
												   =1: GMAC��ַ1���ڱȽϽ���֡��ԴGMAC��ַ. =0: GMAC��ַ1���ڱȽϽ���֡��Ŀ��GMAC��ַ. */
	gmac_addr1_hi_mbc_mask		= (0x3F<<24),	/* bit29~24, Mask Byte Control ��ģ�ֽڿ���.
												      �������ڱȽ�ÿ��GMAC��ַ���ֽ���ģ����λ. �����29λ��������Ĵ���18��[15:8]����ֽ� */
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
	gmac_AN_ctrl_lr				= bit(17),		/* Lock to Reference �������ο�ʱ��. =1: PHY�������໷������125MHz�Ĳο�ʱ�� */
	gmac_AN_ctrl_ecd			= bit(16),		/* Enable Comma Detect ʹ��ͣ��̽��. =1: ʹ��PHY��ͣ��̽�������ͬ�� */
	gmac_AN_ctrl_ele			= bit(14),		/* External Loopback Enable �ⲿ����ʹ��. =1: ʹ��PHY���뻷��ģʽ */
	gmac_AN_ctrl_enable			= bit(12),		/* Auto-Negotiation Enable �Զ�Э��ʹ��. =1: GMAC�������·�Է������Զ�Э�� */

	gmac_AN_ctrl_ran			= bit(9),		/* Restart Auto-Negotiation ���½����Զ�Э��. =1: ���½����Զ�Э�� */

} LS1x_gmac_AN_control_t;

/*
 * AN Status Register 0x00C4
 */
typedef enum
{
	gmac_AN_stat_es				= bit(8),		/* Extended Status ��չ״̬. ֻ��, ��ΪGMAC֧����չ״̬��Ϣ */
	gmac_AN_stat_anc			= bit(5),		/* Auto-Negotiation Complete. ֻ��, ָʾ�Զ�Э����� */
	gmac_AN_stat_ana			= bit(3),		/* Auto-Negotiation Ability �Զ�Э������. ֻ��, ��ΪGMAC֧���Զ�Э�� */
	gmac_AN_stat_ls				= bit(2),		/* Link Status ��·״̬. =1: ָʾ��·������; =0: ָʾ����δ���� */
} LS1x_gmac_AN_status_t;

/*
 * Auto-Negotiation Advertisement Register 0x00C8
 */
typedef enum
{
	gmac_AN_advertise_np		= bit(15),		/* Next Page Support ��һҳ��֧��. ֻ��Ϊ0, ��ΪGMAC��֧����һҳ��.  */
	gmac_AN_advertise_rfe_mask	= (0x03<<12),	/* Remote Fault Encoding Զ�˴������ */
	gmac_AN_advertise_rfe_shift	= 12,			/* ��2λָʾ��·�Զ˷�������, ������뽫IEEE 802.3z��37.2.1.5С�� */
	gmac_AN_advertise_pse_mask	= (0x03<<7),	/* Pause Encoding Pauseλ����. ��IEEE 802.3z��37.2.1.4С�� */
	gmac_AN_advertise_pse_shift = 7,
	gmac_AN_advertise_hd		= bit(6),		/* Half-Duplex ��˫��. =1: ָʾGMAC֧�ְ�˫�� */
	gmac_AN_advertise_fd		= bit(5),		/* Full-Duplex ȫ˫��. =1: ָʾGMAC֧��ȫ˫�� */

} LS1x_gmac_AN_advertise_t;

/*
 * Auto-Negotiation Link Partner Ability Register 0x00CC
 */
typedef enum
{
	gmac_AN_lpa_np			= bit(15),		/* Next Page Support ��һҳ��֧��. =1: ָʾ�и�����һҳ����Ϣ����; =0: ָʾ��һҳ�潻�������� */
	gmac_AN_lpa_ack			= bit(14),		/* Acknowledge ȷ��. ָʾ���Զ�Э����, ��·�Զ˳ɹ����յ�GMAC�Ļ���ҳ�� */
	gmac_AN_lpa_rfe_mask	= (0x03<<12),	/* Remote Fault Encoding Զ�˴������. ��IEEE 802.3z��37.2.1.5С�� */
	gmac_AN_lpa_rfe_shift	= 12,
	gmac_AN_lpa_pse_mask	= (0x03<<7),	/* Pause Encoding �Զ�pause״̬����. ��IEEE 802.3z��37.2.14С�� */
	gmac_AN_lpa_pse_shift	= 7,
	gmac_AN_lpa_hd			= bit(6),		/* Half-Duplex ��˫��. ָʾ�Զ˿��������ڰ�˫��ģʽ */
	gmac_AN_lpa_fd			= bit(5),		/* Full-Duplex ȫ˫��. ָʾ�Զ˿���������ȫ˫��ģʽ */
} LS1x_gmac_AN_lpa_t;

/*
 * Auto-Negotiation Expansion Register 0x00D0
 */
typedef enum
{
	gmac_AN_exp_npa			= bit(2),		/* Next Page Ability ��һҳ������. ֻ��Ϊ0, ��ΪGMAC��֧����һҳ�� */
	gmac_AN_exp_npr			= bit(1),		/* New Page Received ���յ���ҳ��. =1: ָʾGMAC���յ���ҳ�� */
} LS1x_gmac_AN_exp_t;

/*
 * SGMII/RGMII Status Register 0x00D8
 */
typedef enum
{
	gmac_mii_linked			= bit(3),		/* Link Status ��·״̬. =1: ָʾ��·������; =0: ָʾ��·δ������ */
	gmac_mii_speed_mask		= (0x03<<1),	/* Link Speed ��·�ٶ� */
	gmac_mii_speed_shift	= 1,
	gmac_mii_speed_2p5MHZ	= 0x00,			/* 00: 2.5MHz */
	gmac_mii_speed_25MHZ	= 0x20,				/* 01: 25MHz */
	gmac_mii_speed_125MHZ	= 0x40,			/* 10: 125MHz */
	gmac_mii_linkmode		= bit(0),		/* Link Mode ��·ģʽ */
	gmac_mii_linkmode_hd	= 0,				/* 0: ��˫�� */
	gmac_mii_linkmode_fd	= 1,				/* 1: ȫ˫�� */
} LS1x_gmac_mii_status_t;

/*
 * GMII Address Register 0x0010
 * GMII Data Register 0x0014
 */
typedef enum
{
	gmac_miictrl_phyaddr_mask	= (0x1F<<11),		/* bits15~11, PHY Address. ����ѡ����Ҫ����32��PHY�е��ĸ� */
	gmac_miictrl_phyaddr_shift	= 11,
	gmac_miictrl_gmiireg_mask	= (0x1F<<6),		/* bits10~6, GMII Register. ����ѡ����Ҫ���ʵĵ�PHY���ĸ�GMII���üĴ��� */
	gmac_miictrl_gmiireg_shift	= 6,
	gmac_miictrl_csr_mask		= (0x07<<2),		/* bits4~2, CSR Clock Range. �������MDCʱ����clk_csr_iʱ��Ƶ�ʱ��� */
	gmac_miictrl_csr_shift		= 2,				/* (CR)CSR Clock Range: */
	gmac_miictrl_csr_5			= 0x00000014,		/* 250-300 MHz */
	gmac_miictrl_csr_4			= 0x00000010,		/* 150-250 MHz */
	gmac_miictrl_csr_3			= 0x0000000C,		/* 5-60 MHz   */
	gmac_miictrl_csr_2			= 0x00000008,		/* 20-35 MHz   */
	gmac_miictrl_csr_1			= 0x00000004,		/* 100-150 MHz */
	gmac_miictrl_csr_0			= 0x00000000,		/* 60-100 MHz  */
	gmac_miictrl_wr				= bit(1),			/* GMII Write. =1: ͨ��GMII ���ݼĴ�����PHY����д����, =0: ͨ��GMII���ݼĴ�����PHY���ж����� */
	gmac_miictrl_busy			= bit(0),			/* GMII Busy. �ԼĴ���4�ͼĴ���5д֮ǰ, ��λӦΪ0. ��д�Ĵ���4֮ǰ��λ��������0.
                                                       �ڷ���PHY �ļĴ���ʱ, Ӧ�ó�����Ҫ����λ����Ϊ1, ��ʾGMII�ӿ�����д���߶��������ڽ���. */

	gmac_miidata_mask			= 0xFFFF,			/* ���򱣴��˶�PHY���й�������ʲ�����16λ����, ���߶�PHY���й���д���ʵ�16λ����. */

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
	gdma_busmode_mb			= bit(26),		/* Mixed Burst ���ͻ������ *//* �û����ù��Ĵ�λ���� */
	gdma_busmode_aal		= bit(25),		/* Address-Aligned Beats ��ַ������� *//* �û����ù��Ĵ�λ���� */
	gdma_busmode_8xpbl		= bit(24),		/* 8XPBL Mode �Ƿ�ʹ��PBLX8ģʽ *//* �û����ù��Ĵ�λ����  */
	gdma_busmode_usp		= bit(23),		/* Use Separate PBL ʹ�÷����PBLֵ *//* �û����ù��Ĵ�λ����  */
	gdma_busmode_rpbl_mask	= (0x3F<<17),	/* RxDMA PBL RxDMAͻ�����䳤�� */
	gdma_busmode_rpbl_shift = 17,			/* ��ʾһ��RxDMA��������ͻ�����䳤��. ֻ��Ϊ1,2,4,8,16��32, ����ֵ��Ч*/
	gdma_busmode_fb			= bit(16),		/* Fixed Burst ����ͻ�����䳤�� *//* �û����ù��Ĵ�λ���� */
	gdma_busmode_pr_mask	= (0x03<<14),	/* Rx:Tx priority ratio, RxDMA��TxDMA���ȼ�����, ��DAλΪ1ʱ������  */
	gdma_busmode_pr_shift   = 14,
	gdma_busmode_pr_1		= (0<<14),		/* (PR)TX:RX DMA priority ratio 1:1 */
	gdma_busmode_pr_2		= bit(14),		/* (PR)TX:RX DMA priority ratio 2:1 */
	gdma_busmode_pr_3		= (2<<14),		/* (PR)TX:RX DMA priority ratio 3:1 */
	gdma_busmode_pr_4		= (3<<14),		/* (PR)TX:RX DMA priority ratio 4:1 */
	gdma_busmode_pbl_mask	= (0x3f<<8),	/* Programmable Burst Length �ɱ��ͻ�����䳤�� *//* �û����ù��Ĵ�λ���� */
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
	gdma_busmode_atds		= bit(7),		/* Alternate Descriptor size, =1: ʹ��32�ֽڴ�С��������, =0: ʹ��16�ֽڴ�С�������� */
	gdma_busmode_dsl_mask	= (0x1F<<2),	/* Descriptor Skip Length, ����2����������ľ���. ����ֵΪ0ʱ, Ĭ��ΪDMA��������С */
	gdma_busmode_dsl_shift  = 2,
	gdma_busmode_dsl_16		= bit(6),
	gdma_busmode_dsl_8		= bit(5),
	gdma_busmode_dsl_4		= bit(4),
	gdma_busmode_dsl_2		= bit(3),
	gdma_busmode_dsl_1		= bit(2),
	gdma_busmode_dsl_0		= 0,
	gdma_busmode_das		= bit(1),		/* DMA Arbitration scheme, =0: ��RxDMA��TxDMA�������ת�ٲû���, =1: RxDMA���ȼ�����TxDMA���ȼ�(PRֵ) */
	gdma_busmode_swreset	= bit(0),		/* ��λ�ø�DMA����������λGMAC�ڲ��Ĵ������߼�. ����λ����ʱ��λ�Զ����� */
} LS1x_gdma_busmode_t;

/*
 * Status Register of GMAC's DMA 	Offset: 0x14
 */
typedef enum
{
	gdma_status_tti			= bit(29),		/* Time-Stamp Trigger Interrupt, ʱ���ģ�鴥���ж�. ֻ�� */
	gdma_status_gpi			= bit(28),		/* GMAC PMT Interrupt, ��Դ����ģ�鴥���ж�. ֻ�� */
	gdma_status_gmi			= bit(27),		/* GMAC MMC Interrupt, MMCģ�鴥���ж�. ֻ�� */
	gdma_status_gli			= bit(26),		/* GMAC Line interface Interrupt, GMACģ���PCS����RGMIIģ�鴥���ж�. ֻ�� */
	gdma_status_eb_mask		= (0x07<<23),	/* Error Bits, ����λ(�ο������) */
	gdma_status_eb_shift    = 23,
	gdma_status_eb_25		= bit(25),		/* =1: ���������ʴ���; =0: ���ݻ�����ʴ��� */
	gdma_status_eb_24		= bit(24),		/* =1: ���������; =0: д������� */
	gdma_status_eb_23		= bit(23),		/* =1: TxDMA���ݴ�������з�������; =0: RxDMA���ݴ�������з������� */
	gdma_status_txs_mask	= (0x07<<20),	/* Transmit Process State, �������״̬(�ο������) */
	gdma_status_txs_shift   = 20,
	gdma_status_txs_0		= (0<<20),		// 000: ����ֹͣ; ��λ����ֹͣ�����
	gdma_status_txs_1		= (1<<20),		// 001: ���ڽ���; ��ȡ����������
	gdma_status_txs_2		= (2<<20),		// 010: ���ڽ���; �ȴ�����״̬
	gdma_status_txs_3		= (3<<20),		// 011: ���ڽ���; �ӷ��ͻ����ȡ���ݲ����͵�����FIFO(TxFIFO)
	gdma_status_txs_4		= (4<<20),		// 100: д��ʱ���״̬
	gdma_status_txs_5		= (5<<20),		// 101: ����
	gdma_status_txs_6		= (6<<20),		// 110: ����; ���������������û��ߴ��仺������.
	gdma_status_txs_7		= (7<<20),		// 111: ����; �رմ���������.
	gdma_status_rxs_mask	= (0x07<<17),	/* bits: 19-17, Receive Process State, ���չ���״̬(�ο������) */
	gdma_status_rxs_shift   = 17,
	gdma_status_rxs_0		= (0<<17),		// 000: ֹͣ; ��λ���߽��յ�ֹͣ����
	gdma_status_rxs_1		= (1<<17),		// 001: ����; ��ȡ����������.
	gdma_status_rxs_2		= (2<<17),		// 010: ����;
	gdma_status_rxs_3		= (3<<17),		// 011: ����; �ȴ����հ�.
	gdma_status_rxs_4		= (4<<17),		// 100: ��ͣ; ����������������.
	gdma_status_rxs_5		= (5<<17),		// 101: ����; �رս���������.
	gdma_status_rxs_6		= (6<<17),		// 110: ʱ���д״̬.
	gdma_status_rxs_7		= (7<<17),		// 111: ����; �������ݴӽ��ջ��洫�䵽ϵͳ�ڴ�.
	gdma_status_nis			= bit(16),		/* Normal Interrupt Summary, �����жϻ���, ��ʾϵͳ�Ƿ���������ж� */
	gdma_status_ais			= bit(15),		/* Abnormal Interrupt Summary, �쳣�жϻ���, ��ʾϵͳ�Ƿ�����쳣�ж� */
	gdma_status_erxi		= bit(14),		/* Early Receive Interrupt, ��ǰ�����ж�, ��ʾDMA�������Ѿ��Ѱ��ĵ�һ������д����ջ��� */
	gdma_status_fbei		= bit(13),		/* Fatal Bus Error Interrupt, ���ߴ����ж�, ��ʾ���ߴ���, ������Ϣ��[25:23]. ����λ���ú�DMA����ֹͣ���߷��ʲ��� */
	gdma_status_etxi		= bit(10),		/* Early Transmit Interrupt, ��ǰ�����ж�, ��ʾ��Ҫ�������̫��֡�Ѿ���ȫ���䵽MTLģ���еĴ��� FIFO */
	gdma_status_rxwt		= bit(9),		/* Receive Watchdog Timeout, ���տ��Ź���ʱ, ��ʾ���յ�һ����С����2048�ֽڵ���̫��֡ */
	gdma_status_rxstop		= bit(8),		/* Receive Process Stopped, ���չ���ֹͣ */
	gdma_status_rxbufu		= bit(7),		/* Receive Buffer Unavailable, ���ջ��治���� */
	gdma_status_rxi			= bit(6),		/* Receive Interrupt, �����ж�, ָʾ֡�������, ֡���յ�״̬��Ϣ�Ѿ�д�����������, ���մ�������״̬ */
	gdma_status_txunf		= bit(5),		/* Transmit Underflow, ���仺������, ָʾ֡���͹����в������ջ������� */
	gdma_status_rxovf		= bit(4),		/* Receive Overflow, ���ջ�������, ָʾ֡���չ����н��ջ������� */
	gdma_status_txjt		= bit(3),		/* Transmit Jabber Timeout */
	gdma_status_txbufu		= bit(2),		/* Transmit Buffer Unavailable, ���仺�治����, ��ʾ�����б��е���һ�����������ܱ�DMA���������� */
	gdma_status_txstop		= bit(1),		/* Transmit Process Stopped, �������ֹͣ */
	gdma_status_txi			= bit(0),		/* Transmit Interrupt, ��������ж�, ��ʾ֡������ɲ��ҵ�һ����������31λ��λ */
} LS1x_gdma_status_t;

/*
 * Operation Mode Register of GMAC's DMA 	Offset: 0x18
 */
typedef enum
{
	gdma_ctrl_droptcpcse	= bit(26),		/* �رն���TCP/IP Checksum������̫��֡�Ĺ���, =1: GMAC��������checksum�������̫��֡ */
	gdma_ctrl_rxsf			= bit(25),		/* Receive Store and Forward, ���մ洢ת��, =1: MTLģ��ֻ�����Ѿ�ȫ���洢�ڽ���FIFO�е���̫��֡ */
	gdma_ctrl_disfrxf		= bit(24),		/* Disable Flushing of Received Frames, �رճ�ˢ���յ���̫��֡�Ĺ���,
	 	 	 	 	 	 	 	 	 	 	   =1: ����DMA�ڽ������������߽��ջ��治����ʱ����ˢ�κ���̫��֡ */
	gdma_ctrl_RFA_2			= bit(23),		/* RFA[2]: MSB of Threshold for Activating Flow Control ����������ֵ */
	gdma_ctrl_RFD_2			= bit(22),		/* RFD[2]: MSB of Threshold for Deactivating Flow Control �ر�������ֵ */
	gdma_ctrl_txsf			= bit(21),		/* Transmit Store and Forward, ���ʹ洢ת��, =1: ֡�ķ���ֻ��֡�������Ѿ�ȫ������MTL�Ĵ���FIFO�� */
	gdma_ctrl_ftxfifo		= bit(20),		/* Flush Transmit FIFO, ��ˢ����FIFO, =1: ��������߼���λΪĬ��ֵ, ���һᵼ�·���FIFO���������ȫ����ʧ */
	gdma_ctrl_ttc_mask		= (0x7<<14),	/* Transmit Threshold Control, ������ֵ����, ��֡��С������ֵʱMTL���ᴫ���֡ */
	gdma_ctrl_ttc_shift		= 14,
	gdma_ctrl_ttc_64		= (0<<14),		// 000: 64  �ֽ�
	gdma_ctrl_ttc_128		= (1<<14),		// 001: 128 �ֽ�
	gdma_ctrl_ttc_192		= (2<<14),		// 010: 192 �ֽ�
	gdma_ctrl_ttc_256		= (3<<14),		// 011: 256 �ֽ�
	gdma_ctrl_ttc_40		= (4<<14),		// 100: 40  �ֽ�
	gdma_ctrl_ttc_32		= (5<<14),		// 101: 32  �ֽ�
	gdma_ctrl_ttc_24		= (6<<14),		// 110: 24  �ֽ�
	gdma_ctrl_ttc_16		= (7<<14),		// 111: 16  �ֽ�
	gdma_ctrl_txstart		= bit(13),		/* Start/Stop Transmission Command, =1: �����������״̬, =0: �������ֹͣ״̬ */
	gdma_ctrl_RFD			= (0x03<<11),	/* RFD: Threshold for deactivating flow control �ر�������ֵ */
	gdma_ctrl_RFA			= (0x03<<9),	/* RFA:Threshold for Activating flow control ����������ֵ */
	gdma_ctrl_enhwfc		= bit(8),		/* Enable HW flow control, =1: ���ڽ���FIFO�����ʵ�Ӳ�����ص�·��Ч */
	gdma_ctrl_ferrf			= bit(7),		/* Forward Error Frames, �������֡, =1: ���մ���֡(����֡����:CRC����,��ͻ����,��֡,���Ź���ʱ,�����) */
	gdma_ctrl_fugf			= bit(6),		/* Forward Undersized Good Frames, =1: ����FIFO�������û�д���С��64�ֽڵ���̫��֡ */
	gdma_ctrl_rtc_mask		= (0x3<<3),		/* Receive Threshold Control, ������ֵ����, ��֡��С������ֵʱMTL������ո�֡ */
	gdma_ctrl_rtc_shift		= 3,
	gdma_ctrl_rtc_64		= (0<<3),		// 00: 64  �ֽ�
	gdma_ctrl_rtc_32		= (1<<3),		// 01: 32  �ֽ�
	gdma_ctrl_rtc_96		= (2<<3),		// 10: 96  �ֽ�
	gdma_ctrl_rtc_128		= (3<<3),		// 11: 128 �ֽ�
	gdma_ctrl_txopsecf		= bit(2),		/* TX Operate on Second Frame, =1: DMA�ڵ�һ����̫��֡��״̬��δд��ʱ�����Կ�ʼ����ڶ�����̫��֡ */
	gdma_ctrl_rxstart		= bit(1),		/* Start/Stop Receive, =1: ���ս�������״̬, =0: ���ս���ֹͣ״̬ */
} LS1x_gdma_ctrl_t;

/*
 * Interrupt Enable Register of GMAC's DMA 	Offset: 0x1C
 */
typedef enum
{
	gdma_ienable_nis		= bit(16),		/* Normal Interrupt Summary Enable, =1: �����ж�ʹ��, =0: �����жϲ�ʹ�� */
	gdma_ienable_ais		= bit(15),		/* Abnormal Interrupt Summary Enable, =1: �������ж�ʹ��, =0: �������жϲ�ʹ�� */
	gdma_ienable_erxi		= bit(14),		/* Early Receive Interrupt Enable, ���ڽ����ж�ʹ��, =1: ���ڽ����ж�ʹ�� */
	gdma_ienable_fbei		= bit(13),		/* Fatal Bus Error Enable, =1: �������������ж�ʹ�� */
	gdma_ienable_etxi		= bit(10),		/* Early Transmit Interrupt Enable, =1: ʹ�����ڴ����ж� */
	gdma_ienable_rxwdog		= bit(9),		/* Receive Watchdog Timeout Enable, =1: ʹ�ܽ��տ��Ź���ʱ�ж� */
	gdma_ienable_rxstop		= bit(8),		/* Receive Stopped Enable, =1: ʹ�ܽ���ֹͣ�ж� */
	gdma_ienable_rxbufu		= bit(7),		/* Receive Buffer Unavailable Enable, =1: ʹ�ܽ��ջ������������ж� */
	gdma_ienable_rxi		= bit(6),		/* Receive Interrupt Enable, =1: ʹ�ܽ�������ж� */
	gdma_ienable_txunf		= bit(5),		/* Underflow Interrupt Enable, =1: ʹ�ܴ���FIFO�����ж� */
	gdma_ienable_rxovf		= bit(4),		/* Overflow Interrupt Enable, =1: ʹ�ܽ���FIFO�����ж� */
	gdma_ienable_txjt		= bit(3),		/* Transmit Jabber Timeout Enable, =1: ʹ��Jabber��ʱ�ж� */
	gdma_ienable_txbufu		= bit(2),		/* Transmit Buffer Unavailable Enable, =1: ʹ�ܴ��仺�治�����ж� */
	gdma_ienable_txstop		= bit(1),		/* Transmit Stopped Enable, =1: ʹ�ܴ���ֹͣ�ж� */
	gdma_ienable_txi		= bit(0),		/* Transmit Interrupt Enable, =1: ʹ�ܴ�������ж� */

	gdma_ien_base = (gdma_ienable_nis | gdma_ienable_ais | gdma_ienable_fbei),
	gdma_ien_rx	  = (gdma_ienable_rxstop | gdma_ienable_rxi | gdma_ienable_rxbufu),
	gdma_ien_tx	  = (gdma_ienable_txstop | gdma_ienable_txi | gdma_ienable_txunf),

} LS1x_gdma_intr_en_t;

/*
 * Missed Frame and Buffer Overflow Counter Register of GMAC's DMA 	Offset: 0x20
 */
typedef enum
{
	gdma_mfbocnt_ovfcnt			= bit(28),		/* FIFO ���ָʾλ */
	gdma_mfbocnt_fmcnt_mask		= (0x07FF<<17),	/* ָʾӦ�ó���ʧ֡�ĸ��� */
	gdma_mfbocnt_fmcnt_shift    = 17,
	gdma_mfbocnt_ovfmcnt		= bit(16),		/* ��ʾ��ʧ֡�����Ѿ��������������ֵ */
	gdma_mfbocnt_rxufmcnt_mask	= (0xFFFF<<0),	/* ָʾ��Ϊ�������ջ��治���õ���֡��ʧ�����ļ��� */
} LS1x_gdma_mfbo_cnt_t;

#ifdef LS1C

/*
 * Receive Interrupt Watchdog Timer Register
 */
typedef enum
{
	gdma_rxintwdtmr_riwt_mask	= 0xFF,			/* RI Watchdog Timer count, ���տ��Ź�ʱ����� */
	gdma_rxintwdtmr_riwt_shift  = 0,			/* �����Ź����ú�, ��ʾ��ʱ������x256��ʱ��Ϊ��λ��ʱ.
												   ��DMA���յ����ݰ���status�Ĵ�����RIλΪ0ʱ��ʼ��ʱ,
												   �����Ź�������ʱ��RIλ��1. ��RIλΪ1�����λ */
} LS1C_gdma_rxintwdtmr_t;

/*
 * AXI Bus Mode Register
 */
typedef enum
{
	gdma_axibusmode_en_lpi		= bit(31),		/* Enable LPI (Low Power Interface), ���õ͹��Ľӿ� */
	gdma_axibusmode_unlock		= bit(30), 		/* Unlock on Magic Packet or Remote Wake Up, ����ħ������Զ�̻��Ѱ� */
												/* =1: GMAC���ڵ͹���(Low Power)״̬��ʱֻ��ͨ��ħ��������Զ�̻��Ѱ������»ص�����״̬;
												 * =0: GMAC���ڵ͹���(Low Power)״̬��ʱֻ��ͨ����������»ص�����״̬
												 */
	gdma_axibusmode_wrlmt_mask	= (0x07<<20),	/* AXI Maximum Write Out Standing Request Limit, AXI���outstandingд���� */
	gdma_axibusmode_wrlmt_shift = 20,			/* ��λ������AXI�ӿ�д������������outstanding������ */
	gdma_axibusmode_rdlmt_mask	= (0x07<<16),	/* AXI Maximum Read Out Standing Request Limit, AXI���outstanding������ */
	gdma_axibusmode_rdlmt_shift = 16,			/* ��λ������AXI�ӿڶ�������������outstanding������ */
	gdma_axibusmode_addr_align  = bit(12),		/* Address-Aligned Beats, AXI��ַ����, ֻ�� */
	gdma_axibusmode_blen_256	= bit(7),		/* AXI Burst Length 256, AXI Burst����256 */
	gdma_axibusmode_blen_128	= bit(6),
	gdma_axibusmode_blen_64		= bit(5),
	gdma_axibusmode_blen_32		= bit(4),
	gdma_axibusmode_blen_16		= bit(3),
	gdma_axibusmode_blen_8		= bit(2),
	gdma_axibusmode_blen_4		= bit(1),
	gdma_axibusmode_blen_undef	= bit(0),		/* AXI Undefined Burst Length, δ����AXI Burst���� */
} LS1C_gdma_axibusmode_t;

/*
 * AXI Status Register
 */
typedef enum
{
	gdma_axistatus_rd_request	= bit(1),		/* =1: ��ʾ��ǰAXI���ڷ��Ͷ����� */
	gdma_axistatus_wr_request	= bit(0),		/* =1: ��ʾ��ǰAXI���ڷ���д���� */
} LS1C_gdma_axistatus_t;

#endif

/******************************************************************************
 * Receive Descriptor RDES0 - Status
 */

typedef enum
{
	gdma_rxdesc_stat_own		= bit(31),		/* OWN, =1: ��������ǰ����DMA����, =0: ������������. ��DMAģ�����һ�δ���ʱ, �Ὣ��λ������0 */
	gdma_rxdesc_stat_afm		= bit(30),		/* Destination Address Filter Fail, Ŀ���ַ���˴���. =1: ��ǰ����֡Ŀ���ַ������GMAC�ڲ���֡Ŀ���ַ������ */
	gdma_rxdesc_stat_fr_mask    = (0x3FFF<<16), /* bit29~16, Frame length ֡����, ��ʾ���յ�ǰ֡�ĳ���, ��ESλΪ0ʱ��Ч */
	gdma_rxdesc_stat_fr_shift   = 16,
	gdma_rxdesc_stat_es      	= bit(15),		/* Error Summary ���������Ϣ, ָʾ��ǰ֡�Ƿ����, ��ֵΪRDES[0,1,3,4,6,7,11,14]��λ��������(OR)�Ľ�� */
	gdma_rxdesc_stat_de			= bit(14),		/* Descriptor Error ����������, =1: ��ǰ��������ָ���buffer��֡���������OWNΪ0(��������) */
	gdma_rxdesc_stat_saf		= bit(13),		/* Source Address Filter Fail Դ��ַ���˴���, =1: ��ǰ����֡��Դ��ַ������GMAC�ڲ���֡Դ��ַ������ */
	gdma_rxdesc_stat_le			= bit(12),		/* Length Error ���ȴ���, =1: ��ǰ����֡������Ĭ�ϳ��Ȳ���. ��Frame TypeλΪ1��CRC ErrorλΪ0ʱ��Ч */
	gdma_rxdesc_stat_oe			= bit(11),		/* Over Flow Error �������, =1: ���ո�֡ʱGMAC�ڲ�RxFIFO��� */
	gdma_rxdesc_stat_vlan		= bit(10),		/* VLAN Tag VLAN��־, =1: ��֡������ΪVLAN */
	gdma_rxdesc_stat_fs			= bit(9),		/* First Desciptor ��һ��������, =1: ��ǰ��������ָ���bufferΪ��ǰ����֡�ĵ�һ������buffer */
	gdma_rxdesc_stat_ls			= bit(8),		/* Last Desciptor ���һ��������, =1: ��ǰ��������ָ���bufferΪ��ǰ����֡�����һ������buffer */
	gdma_rxdesc_stat_ipce_gf	= bit(7),		/* IPC Checksum Error/Giant Frame У�����/����֡.
	 	 	 	 	 	 	 	 	 	 	 	   =1: ���IPCУ�鹦���������ʾ��ǰ֡��IPv4ͷУ��ֵ��֡�ڲ�У�����ֵ�����.
	 	 	 	 	 	 	 	 	 	 	 	 	   ���δ�������ʾ��ǰ֡Ϊһ������֡(���ȴ���1518�ֽ�) */
	gdma_rxdesc_stat_lc			= bit(6),		/* Late Collision ���ڳ�ͻ, =1: �ڰ�˫��ģʽ��, ��ǰ֡����ʱ������һ�����ڳ�ͻ */
	gdma_rxdesc_stat_ft			= bit(5),		/* Frame Type ֡����, =1: ��ǰ֡Ϊһ����̫����ʽ֡, =0: ��ǰ֡Ϊһ��IEEE802.3��ʽ֡ */
	gdma_rxdesc_stat_rwt		= bit(4),		/* Receive Watchdog Timeout, =1: ��ǰʱ��ֵ�����˽���ģ�鿴�Ź���·ʱ�ӵ�ֵ, �Ƚ���֡��ʱ */
	gdma_rxdesc_stat_re			= bit(3),		/* Receive Error ���մ���, =1: ���յ�ǰ֡ʱ�ڲ�ģ�����. �ڲ��ź�rxer��1��rxdv��1 */
	gdma_rxdesc_stat_dbe		= bit(2),		/* Dribble bit Error ����λ����, =1: ����֡���Ȳ�������, ���ܳ���Ϊ����λ, ��λֻ����miiģʽ����Ч */
	gdma_rxdesc_stat_ce			= bit(1),		/* CRC Error ����CRCУ�����, =1: ���յ�ǰ֡ʱ�ڲ�CRCУ�����. ��λֻ����last descriptor(RDES0[8])Ϊ1ʱ��Ч */
	gdma_rxdesc_stat_rmpce		= bit(0),		/* RX GMAC Checksum/payload Checksum Error ����У��/����У�����.
												   =1: ���յ�ǰ֡ʱ�ڲ�RX GMAC�Ĵ�����1-15�д���һ��ƥ�䵱ǰ֡Ŀ�ĵ�ַ.
												   =0: RX GMAC �Ĵ�����0ƥ�����֡Ŀ�ĵ�ַ. ���Full Checksum Offload Engine����ʱ,
												   =1: ��֡TCP/UDP/ICMPУ�����. ��λΪ1ʱҲ���ܱ�ʾ��ǰ֡ʵ�ʽ��ܳ�����֡�ڲ����س��Ȳ����. */
} LS1x_gdma_rxdesc_stat_t;

/******************************************************************************
 * Receive Descriptor RDES1 - Control, Address
 */
typedef enum
{
	gdma_rxdesc_ctrl_di			= bit(31),		/* 1: Disable Interrupt in Completion */
	gdma_rxdesc_ctrl_userdef	= bit(30),		/* XXX �Զ���, ����ѭ������ */
	gdma_rxdesc_ctrl_rer		= bit(25),		/* Receive End of Ring, =1: ��������Ϊ������������������һ��, ��һ���������ĵ�ַΪ�������������Ļ�ַ */
	gdma_rxdesc_ctrl_rch		= bit(24),		/* Second Address Chained, =1: �������еĵڶ���buffer��ַָ�������һ���������ĵ�ַ */
	gdma_rxdesc_ctrl_rbs2_mask	= (0x07FF<<11),	/* bits: 21-11, Receive Buffer Size 2, �����ʾ����buffer2�Ĵ�С */
	gdma_rxdesc_ctrl_rbs2_shift = 11,
	gdma_rxdesc_ctrl_rbs1_mask	= (0x07FF<<0),	/* bits: 10-0, Receive Buffer Size 1, �����ʾ����buffer1�Ĵ�С */
} LS1x_gdma_rxdesc_ctrl_t;

/******************************************************************************
 * Transmit Descriptor TDES0 - Status
 */
typedef enum
{
	gdma_txdesc_stat_own		= bit(31),		/* OWN, =1: ��ʾ��������ǰ����DMA����, 0��ʾ������������. ��DMAģ�����һ�δ���ʱ, �Ὣ��λ������0 */
	gdma_txdesc_stat_ttss		= bit(17),		/* Tx Time Stamp Status, ��IEEE1588��������ʱ, =1: TDES2��TDES3�б����˸÷���֡��ʱ�����Ϣ. �����λ���� */
	gdma_txdesc_stat_ihe		= bit(16),		/* IP Header Error, =1: ��ʾ�ڲ�У��ģ�鷢�ָ÷���֡��IPͷ����, ���Ҳ���Ը������κ��޸� */
	gdma_txdesc_stat_es			= bit(15),		/* Error Summary, ָʾ��ǰ֡�Ƿ����, ��ֵΪTDES[1,2,8,9,10,11,13,14]��λ��������(OR)�Ľ�� */
	gdma_txdesc_stat_jt			= bit(14),		/* Jabber Timeout, =1: ��ʾGMAC����ģ��������Jabber��ʱ */
	gdma_txdesc_stat_ff			= bit(13),		/* Frame Flushed, =1: ��ʾ���������һ��ˢ�������DMA/MTL�����ڲ���֡ˢ�µ� */
	gdma_txdesc_stat_pce		= bit(12),		/* Payload Checksum Error, =1: ��ʾ�ڲ�����У��ģ��������֡�в���У������ʱ����. ������У��ģ������ʱ, ��λ��Ч */
	gdma_txdesc_stat_lc			= bit(11),		/* Loss of Carrier, =1: ��ʾ�ڷ��͸�֡�������ز���ʧ(gmii_crs�źŶ������δ����) */
	gdma_txdesc_stat_nc			= bit(10),		/* No Carrier, =1: ��ʾ�ڷ��͹�����, PHY���ز��ź�һֱδ���� */
	gdma_txdesc_stat_lco		= bit(9),		/* Late Collision, =1: ��ʾ�ڰ�˫��ģʽ��, ��ǰ֡����ʱ������һ�����ڳ�ͻ */
	gdma_txdesc_stat_ec			= bit(8),		/* Excessive Collison, =1: ��ʾ�ڷ��͵�ǰ֡��ʱ������������16�γ�ͻ */
	gdma_txdesc_stat_vf			= bit(7),		/* VLAN Frame, =1: ��ʾ��ǰ����֡Ϊһ��VLAN֡ */
	gdma_txdesc_stat_cc_mask	= (0x0F<<3),	/* bits: 6-3, Collsion Count, �����ʾ��ǰ֡�ڳɹ�����֮ǰ��������ͻ���������� */
	gdma_txdesc_stat_cc_shift   = 3,
	gdma_txdesc_stat_ed			= bit(2),		/* Excessive Deferral, =1: ��ʾ��ǰ֡������� */
	gdma_txdesc_stat_uf			= bit(1),		/* Underflow Error, =1: ��ʾ��ǰ֡����ʱ�������������, �����ݴ���buffer��С�򲻿��� */
	gdma_txdesc_stat_db			= bit(0),		/* Defered Bit, =1: ��ʾ�˴η��ͱ��ӳ�, ֻ���ڰ�˫��ģʽ����Ч */
} LS1x_gdma_txdesc_stat_t;

/******************************************************************************
 * Transmit Descriptor TDES1 - Control, Address
 */
typedef enum
{
	gdma_txdesc_ctrl_ic			= bit(31),		/* Interrption on Complete, =1: ��ʾ��֡�ӷ�����ɺ󽫻�����STATUS�Ĵ�����TIλ(CSR5[0]) */
	gdma_txdesc_ctrl_ls			= bit(30),		/* Last Segment, =1: ��ʾ��ǰbuffer��������һ֡���ݵ����һ��(���֡��Ϊ�����) */
	gdma_txdesc_ctrl_fs			= bit(29),		/* First Segment, =1: ��ʾ��ǰbuffer��������һ֡���ݵĵ�һ��(���֡��Ϊ�����) */
	gdma_txdesc_ctrl_cic_mask	= (0x02<<27),	/* bits: 28-27, Checksum Insertion Control, ��������ڲ�ģ���Ƿ��ڷ���֡�����У������ */
	gdma_txdesc_ctrl_cic_shift  = 27,
	gdma_txdesc_ctrl_cic_ipv4	= bit(27),
	gdma_txdesc_ctrl_cic_tcp	= bit(28),
	gdma_txdesc_ctrl_cic_all	= (0x02<<27),
	gdma_txdesc_ctrl_dc			= bit(26),		/* =1, Disable CRC, =1: GMACӲ������ÿ������֡�Ľ�β���CRCУ������ */
	gdma_txdesc_ctrl_ter		= bit(25),		/* Transmit End of Ring, =1: ��ʾ��������Ϊ������������������һ��, ��һ���������ĵ�ַΪ�������������Ļ�ַ */
	gdma_txdesc_ctrl_tch		= bit(24),		/* Second Address Chained, =1: ��ʾ�������еĵڶ���buffer��ַָ�������һ���������ĵ�ַ  */
	gdma_txdesc_ctrl_dp			= bit(23),		/* Dissable Pading, =1: ��ʾGMAC������Գ���С��64�ֽڵ����ݰ����п�������� */
	gdma_txdesc_ctrl_ttse		= bit(22),		/* Transmit Time Stamp Enable, =1: ��ʾ�������ڲ�ģ�����IEEE1588Ӳ��ʱ�������, ��TDES1[29]Ϊ1ʱ��Ч */
	gdma_txdesc_ctrl_tbs2_mask	= (0x07FF<<11),	/* bits: 21-11, Transmit Buffer Size 2, �����ʾ����buffer2�Ĵ�С. ��TDES1[24]Ϊ1ʱ, ������Ч */
	gdma_txdesc_ctrl_tbs2_shift = 11,
	gdma_txdesc_ctrl_tbs1_mask	= (0x07FF<<0),	/* bits: 10-0, Transmit Buffer Size 1, �����ʾ����buffer1�Ĵ�С. ����һֱ��Ч */
} LS1x_txdesc_ctrl_t;

/*
 * GMAC DMA Receive and Transmit Descriptor
 * __attribute__((packed))������: ���߱�����ȡ���ṹ�ڱ�������е��Ż�����,����ʵ��ռ���ֽ������ж���---��GCC���е��﷨
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


