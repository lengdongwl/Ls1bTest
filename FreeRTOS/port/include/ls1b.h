/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 *  Loongson GS232I LS1B specific information
 *  
 *  $Id: ls1b.h, v 1.0 2013/06/16 Bian Exp $
 */

#ifndef __LS1B_H__
#define __LS1B_H__

#ifdef  __cplusplus
extern "C" {
#endif

#define bit(x)         (1<<(x))

/*
 *  寄存器 Read/Write 操作
 */
/*
 * 8 Bits
 */
#define READ_REG8(Addr)         (*(volatile unsigned char*)(Addr))
#define WRITE_REG8(Addr, Val)   (*(volatile unsigned char*)(Addr) = (Val))
#define OR_REG8(Addr, Val)      (*(volatile unsigned char*)(Addr) |= (Val))
#define AND_REG8(Addr, Val)     (*(volatile unsigned char*)(Addr) &= (Val))

/*
 * 16 Bits
 */
#define READ_REG16(Addr)        (*(volatile unsigned short*)(Addr))
#define WRITE_REG16(Addr, Val)  (*(volatile unsigned short*)(Addr) = (Val))
#define OR_REG16(Addr, Val)     (*(volatile unsigned short*)(Addr) |= (Val))
#define AND_REG16(Addr, Val)    (*(volatile unsigned short*)(Addr) &= (Val))

/*
 * 32 Bits
 */
#define READ_REG32(Addr)        (*(volatile unsigned int*)(Addr))
#define WRITE_REG32(Addr, Val)  (*(volatile unsigned int*)(Addr) = (Val))
#define OR_REG32(Addr, Val)     (*(volatile unsigned int*)(Addr) |= (Val))
#define AND_REG32(Addr, Val)    (*(volatile unsigned int*)(Addr) &= (Val))

/* 
 * Loongson 1b CP0 registers
 */
#define CP0_Index           $0          /* 可写的寄存器，用于指定需要读/写的TLB表项 */
#define CP0_Random          $1          /* 用于TLB替换的伪随机计数器 */
#define CP0_EntryLo0        $2          /* TLB表项低半部分中对应于偶虚页的内容(主要是物理页号) */
#define CP0_EntryLo1        $3          /* TLB表项低半部分中对应于奇虚页的内容(主要是物理页号) */
#define CP0_Context         $4          /* 32位寻址模式下指向内核的虚拟页转换表(PTE) */
#define CP0_PageMask        $5          /* 设置TLB页大小的掩码值 */
#define CP0_Wired           $6          /* 固定连线的TLB表项数目(指不用于随机替换的低端TLB表项) */
#define CP0_HWREna          $7          /* ***读硬件寄存器时用到的mask位(R2) */
#define CP0_BadVAddr        $8          /* 错误的虚地址 */
#define CP0_Count           $9          /* 计数器 */
#define CP0_EntryHi         $10         /* TLB表项的高半部分内容(虚页号和ASID) */
#define CP0_Compare         $11         /* 计数器比较 */
#define CP0_Status          $12         /* 处理器状态寄存器 */
#define CP0_IntCtl          $12,1       /* 控制扩展的中断功能(R2) */
#define CP0_SRSCtl          $12,2       /* 控制对影子寄存器的操作(R2) */
#define CP0_SRSMap          $12,3       /* 影子寄存器与中断向量的对应关系(R2) */
#define CP0_Cause           $13         /* 最近一次例外的原因 */
#define CP0_EPC             $14         /* 例外程序计数器 */
#define CP0_PRId            $15         /* 处理器修订版本标识号 */
#define CP0_Ebase           $15         /* 保存异常当BEV为0时的向量入口的基址 */
#define CP0_Config          $16         /* 配置寄存器(Cache大小等) */
#define CP0_Config0         $16
#define CP0_Config1         $16,1       /* 配置寄存器 */
#define CP0_Config2         $16,2       /* 在没有二级cache的R2实现中，仅表示实现Config3 */
#define CP0_Config3         $16,3       /* 配置寄存器 (R2) */
#define CP0_Config6         $16,6       /* 配置分支预测的策略 */
#define CP0_LLAddr          $17         /* 链接读内存地址 */
#define CP0_WatchLo         $18         /* 虚地址空间访问陷阱地址 */
#define CP0_WatchHi         $19         /* 虚地址空间访问陷阱地址 */
#define CP0_Debug           $23         /* Debug */
#define CP0_DEPC            $24         /* EJTAG debug 例外程序计数器 */
#define CP0_PerfCnt0        $25,1       /* 性能计数器1 */
#define CP0_PerfCnt1        $25,3       /* 性能计数器2 */
#define CP0_PerfCnt0Ctrl    $25,0       /* 性能计数器的关联控制寄存器 */
#define CP0_PerfCnt1Ctrl    $25,2       /* 性能计数器的关联控制寄存器 */
#define CP0_TagLo           $28         /* CACHE TAG寄存器的低半部分*/
#define CP0_ErrorEPC        $30         /* 错误例外程序计数器 */
#define CP0_DESave          $31         /* EJTAG debug 例外保存寄存器*/

/******************************************************************************
 * AXI各模块地址分配
 */
#define LS1B_DDR_BASE           0x00000000      /* – 0x0FFFFFFF = 256MB */
#define LS1B_DCSLAVE_BASE       0x1C200000      /* – 0x1C2FFFFF = 1M */
#define LS1B_AXIMUXSLAVE_BASE   0x1F000000      /* – 0x1FFFFFFF = 16M */

/*
 * AXI MUX各模块地址分配
 */
#define LS1B_SPI0_MEM_BASE      0xBF000000      /* – 0xBF7FFFFF = 8MB */
#define LS1B_SPI1_MEM_BASE      0xBF800000      /* – 0xBFBFFFFF = 4MB */
#define LS1B_SPI0_DR_BASE       0xBFC00000      /* – 0xBFCFFFFF = 1MB */

#define LS1B_CONFREG_BASE       0xBFD00000      /* – 0xBFDFFFFF = 1MB */
#define LS1B_USB_BASE           0xBFE00000      /* – 0xBFE0FFFF = 64KB */
#define LS1B_GMAC0_BASE         0xBFE10000      /* – 0xBFE1FFFF = 64KB */
#define LS1B_GMAC1_BASE         0xBFE20000      /* – 0xBFE2FFFF = 64KB */
#define LS1B_APB_DEV_BASE       0xBFE40000      /* – 0xBFE7FFFF = 256KB */

#define LS1B_SPI0_BASE          0xBFE80000      /* – 0xBFEBFFFF = 256KB */
#define LS1B_SPI1_BASE          0xBFEC0000      /* – 0xBFEFFFFF = 256KB */

/*
 * APB各模块的地址空间分配
 */
#define LS1B_UART0_BASE         0xBFE40000      /* -0xBFE43FFF = 16KB */
#define LS1B_UART1_BASE         0xBFE44000      /* -0xBFE47FFF = 16KB */
#define LS1B_UART2_BASE         0xBFE48000      /* -0xBFE4BFFF = 16KB */
#define LS1B_UART3_BASE         0xBFE4C000      /* -0xBFE4FFFF = 16KB */
#define LS1B_UART4_BASE         0xBFE6C000      /* -0xBFE6FFFF = 16KB */
#define LS1B_UART5_BASE         0xBFE7C000      /* -0xBFE7FFFF = 16KB */

#define LS1B_CAN0_BASE          0xBFE50000      /* -0xBFE53FFF = 16KB */
#define LS1B_CAN1_BASE          0xBFE54000      /* -0xBFE57FFF = 16KB */

#define LS1B_I2C0_BASE          0xBFE58000      /* -0xBFE5BFFF = 16KB */
#define LS1B_I2C1_BASE          0xBFE68000      /* -0xBFE6BFFF = 16KB */
#define LS1B_I2C2_BASE          0xBFE70000      /* -0xBFE73FFF = 16KB */

#define LS1B_PWM0_BASE          0xBFE5C000
#define LS1B_PWM1_BASE          0xBFE5C010
#define LS1B_PWM2_BASE          0xBFE5C020
#define LS1B_PWM3_BASE          0xBFE5C030

#define LS1B_RTC_BASE           0xBFE64000      /* -0xBFE67FFF = 16KB */
#define LS1B_AC97_BASE          0xBFE74000      /* -0xBFE77FFF = 16KB */
#define LS1B_NAND_BASE          0xBFE78000      /* -0xBFE7BFFF = 16KB */

/*
 * PLL Frequency & Div Register
 */
#define LS1B_PLL_BASE           0xBFE78030

/* PLL frequency Register
 */
#define LS1B_CLK_PLL_FREQ       (*(volatile unsigned int*)LS1B_PLL_BASE)

#define PLL_FREQ_1_MASK         0x0000003F      /* bit 5:0 */
#define PLL_FREQ_1_SHIFT        0
#define PLL_FREQ_2_MASK         0x0003FF00      /* bit 17:8 */
#define PLL_FREQ_2_SHIFT        8

#define LS1B_PLL_FREQUENCY(xtal_freq)   \
        ((xtal_freq * (12+(LS1B_CLK_PLL_FREQ & PLL_FREQ_1_MASK) + \
        (((LS1B_CLK_PLL_FREQ & PLL_FREQ_2_MASK) >> PLL_FREQ_2_SHIFT) >> 10))) >> 1)

/* PLL div register
 */
#define LS1B_CLK_PLL_DIV        (*(volatile unsigned int*)(LS1B_PLL_BASE+4))

#define PLL_DC_EN               bit(31)         // DC_DIV enable
#define PLL_DC_RESET            bit(30)         // DC_DIV reset
#define PLL_DC_DIV_MASK         0x3c000000      // bit 29:26, DC_DIV (pll_out/4/DC_DIV)
#define PLL_DC_DIV_SHIFT        26
#define PLL_CPU_EN              bit(25)         // CPU_DIV enable
#define PLL_CPU_RESET           bit(24)         // CPU_DIV reset
#define PLL_CPU_DIV_MASK        0x00F00000      // bit 23:20, CPU_DIV
#define PLL_CPU_DIV_SHIFT       20
#define PLL_DDR_EN              bit(19)         // DDR_DIV enable
#define PLL_DDR_RESET           bit(18)         // DDR_DIV reset
#define PLL_DDR_DIV_MASK        0x0003c000      // bit 17:14, DDR_DIV
#define PLL_DDR_DIV_SHIFT       14
#define PLL_DC_BYPASS_EN        bit(13)         // DC_BYPASS enable
#define PLL_DC_BYPASS           bit(12)         // DC_BYPASS
#define PLL_DDR_BYPASS_EN       bit(11)         // DDR_BYPASS enable
#define PLL_DDR_BYPASS          bit(10)         // DDR_BYPASS
#define PLL_CPU_BYPASS_EN       bit(9)          // CPU_BYPASS enable
#define PLL_CPU_BYPASS          bit(8)          // CPU_BYPASS

#define PLL_DC_RST_EN           bit(5)          // DC_RST enable
#define PLL_DC_RST              bit(4)          // DC_RST
#define PLL_DDR_RST_EN          bit(3)          // DDR_RST enable
#define PLL_DDR_RST             bit(2)          // DDR_RST
#define PLL_CPU_RST_EN          bit(1)          // CPU_RST enable
#define PLL_CPU_RST             bit(0)          // CPU_RST

#define LS1B_CPU_FREQUENCY(xtal_freq)                    \
        (!((LS1B_CLK_PLL_DIV & PLL_CPU_EN) &&            \
          !(LS1B_CLK_PLL_DIV & PLL_CPU_BYPASS) &&        \
           (LS1B_CLK_PLL_DIV & PLL_CPU_DIV_MASK))        \
         ? xtal_freq : (LS1B_PLL_FREQUENCY(xtal_freq) /  \
         ((LS1B_CLK_PLL_DIV & PLL_CPU_DIV_MASK) >> PLL_CPU_DIV_SHIFT)))

#define LS1B_DDR_FREQUENCY(xtal_freq)                    \
        (!((LS1B_CLK_PLL_DIV & PLL_DDR_EN) &&            \
          !(LS1B_CLK_PLL_DIV & PLL_DDR_BYPASS) &&        \
           (LS1B_CLK_PLL_DIV & PLL_DDR_DIV_MASK))        \
         ? xtal_freq : (LS1B_PLL_FREQUENCY(xtal_freq) /  \
         ((LS1B_CLK_PLL_DIV & PLL_DDR_DIV_MASK) >> PLL_DDR_DIV_SHIFT)))

#define LS1B_DC_FREQUENCY(xtal_freq)                     \
        (!((LS1B_CLK_PLL_DIV & PLL_DC_EN) &&             \
          !(LS1B_CLK_PLL_DIV & PLL_DC_BYPASS) &&         \
           (LS1B_CLK_PLL_DIV & PLL_DC_DIV_MASK))         \
         ? xtal_freq : (LS1B_PLL_FREQUENCY(xtal_freq) /  \
         ((LS1B_CLK_PLL_DIV & PLL_DC_DIV_MASK) >> PLL_DC_DIV_SHIFT)) / 4)

#define LS1B_BUS_FREQUENCY(xtal_freq)  (LS1B_DDR_FREQUENCY(xtal_freq) >> 1)

#define LS1x_CPU_FREQUENCY(xtal_freq)   LS1B_CPU_FREQUENCY(xtal_freq)
#define LS1x_DDR_FREQUENCY(xtal_freq)   LS1B_DDR_FREQUENCY(xtal_freq)
#define LS1x_BUS_FREQUENCY(xtal_freq)   LS1B_BUS_FREQUENCY(xtal_freq)

/*
 *  GPIO registers
 */
#define LS1B_GPIO_CFG_BASE      0xBFD010C0      /* 1:对应PAD为GPIO功能; 0:对应PAD为普通功能 */
#define LS1B_GPIO_EN_BASE       0xBFD010D0      /* 1:对应GPIO被控制为输入; 0:对应GPIO被控制为输出 */
#define LS1B_GPIO_IN_BASE       0xBFD010E0      /* 1: GPIO输入值1,PAD驱动输入为3.3V; 0: GPIO输入值0,PAD驱动输入为0V */
#define LS1B_GPIO_OUT_BASE      0xBFD010F0      /* 1: GPIO输出值1,PAD驱动输出3.3V; 0: GPIO输出值0,PAD驱动输出0V */

/* i=0: GPIO30:GPIO0; i=1: GPIO61:GPIO32
 */
#define LS1B_GPIO_CFG(i)        (*(volatile unsigned int*)(LS1B_GPIO_CFG_BASE+i*4))
#define LS1B_GPIO_EN(i)         (*(volatile unsigned int*)(LS1B_GPIO_EN_BASE+i*4))
#define LS1B_GPIO_IN(i)         (*(volatile unsigned int*)(LS1B_GPIO_IN_BASE+i*4))
#define LS1B_GPIO_OUT(i)        (*(volatile unsigned int*)(LS1B_GPIO_OUT_BASE+i*4))

/*
 *  DMA register
 */

/*
 *  GPIO Config 1 register
 */
#define LS1B_GPIO_CFG1_CAN0_RX      bit(6)          // GPIO38
#define LS1B_GPIO_CFG1_CAN0_TX      bit(7)          // GPIO39
#define LS1B_GPIO_CFG1_CAN1_RX      bit(8)          // GPIO40
#define LS1B_GPIO_CFG1_CAN1_TX      bit(9)          // GPIO41

/*
 * UART split register
 */
#define LS1B_UART_SPLIT_ADDR        0xBFE78038
#define UART_SPLIT_UART1            bit(1)          // UART1被分成四个独立两线UART
#define UART_SPLIT_UART0            bit(0)          // UART0被分成四个独立两线UART

/*
 *  MUX Control register 0, 当管脚配置为GPIO功能时，MUX寄存器的配置不起作用
 */
#define LS1B_MUX_CTRL0_ADDR             0xBFD00420
#define LS1B_MUX_CTRL0                  (*(volatile unsigned int*)LS1B_MUX_CTRL0_ADDR)

#define MUX_CTRL0_UART0_USE_PWM23       bit(28)
#define MUX_CTRL0_UART0_USE_PWM01       bit(27)
#define MUX_CTRL0_UART1_USE_LCD0_5_6_11 bit(26)
#define MUX_CTRL0_I2C2_USE_CAN1         bit(25)
#define MUX_CTRL0_I2C1_USE_CAN0         bit(24)
#define MUX_CTRL0_NAND3_USE_UART5       bit(23)
#define MUX_CTRL0_NAND3_USE_UART4       bit(22)
#define MUX_CTRL0_NAND3_USE_UART1_DAT   bit(21)
#define MUX_CTRL0_NAND3_USE_UART1_CTS   bit(20)
#define MUX_CTRL0_NAND3_USE_PWM23       bit(19)
#define MUX_CTRL0_NAND3_USE_PWM01       bit(18)
#define MUX_CTRL0_NAND2_USE_UART5       bit(17)
#define MUX_CTRL0_NAND2_USE_UART4       bit(16)
#define MUX_CTRL0_NAND2_USE_UART1_DAT   bit(15)
#define MUX_CTRL0_NAND2_USE_UART1_CTS   bit(14)
#define MUX_CTRL0_NAND2_USE_PWM23       bit(13)
#define MUX_CTRL0_NAND2_USE_PWM01       bit(12)
#define MUX_CTRL0_NAND1_USE_UART5       bit(11)
#define MUX_CTRL0_NAND1_USE_UART4       bit(10)
#define MUX_CTRL0_NAND1_USE_UART1_DAT   bit(9)
#define MUX_CTRL0_NAND1_USE_UART1_CTS   bit(8)
#define MUX_CTRL0_NAND1_USE_PWM23       bit(7)
#define MUX_CTRL0_NAND1_USE_PWM01       bit(6)
#define MUX_CTRL0_GMAC1_USE_UART1       bit(4)
#define MUX_CTRL0_GMAC1_USE_UART0       bit(3)
#define MUX_CTRL0_LCD_USE_UART0_DAT     bit(2)
#define MUX_CTRL0_LCD_USE_UART15        bit(1)
#define MUX_CTRL0_LCD_USE_UART0         bit(0)

/*
 *  MUX Control register 1, 当管脚配置为GPIO功能时，MUX寄存器的配置不起作用
 */
#define LS1B_MUX_CTRL1_ADDR             0xBFD00424
#define LS1B_MUX_CTRL1                  (*(volatile unsigned int*)LS1B_MUX_CTRL1_ADDR)

#define MUX_CTRL1_USB_RESET             bit(31)
#define MUX_CTRL1_SPI1_CS_USE_PWM01     bit(24)
#define MUX_CTRL1_SPI1_USE_CAN          bit(23)
#define MUX_CTRL1_DISABLE_DDR_CONFSPACE bit(20)
#define MUX_CTRL1_DDR32TO16EN           bit(16)
#define MUX_CTRL1_GMAC1_SHUT            bit(13)
#define MUX_CTRL1_GMAC0_SHUT            bit(12)
#define MUX_CTRL1_USB_SHUT              bit(11)
#define MUX_CTRL1_UART1_3_USE_CAN1      bit(5)
#define MUX_CTRL1_UART1_2_USE_CAN0      bit(4)
#define MUX_CTRL1_GMAC1_USE_TX_CLK      bit(3)
#define MUX_CTRL1_GMAC0_USE_TX_CLK      bit(2)
#define MUX_CTRL1_GMAC1_USE_PWM23       bit(1)
#define MUX_CTRL1_GMAC0_USE_PWM01       bit(0)

/*
 * Interrupt Control register
 */
#define LS1B_INTC0_BASE         0xBFD01040
#define LS1B_INTC1_BASE         0xBFD01058
#define LS1B_INTC2_BASE         0xBFD01070
#define LS1B_INTC3_BASE         0xBFD01088

#define LS1B_INTC_ISR(base)     (*(volatile unsigned int*)(base + 0x00))   /* 中断控制状态寄存器 */
#define LS1B_INTC_IEN(base)     (*(volatile unsigned int*)(base + 0x04))   /* 中断控制使能寄存器 */
#define LS1B_INTC_SET(base)     (*(volatile unsigned int*)(base + 0x08))   /* 中断置位寄存器 */
#define LS1B_INTC_CLR(base)     (*(volatile unsigned int*)(base + 0x0C))   /* 中断清空寄存器 */
#define LS1B_INTC_POL(base)     (*(volatile unsigned int*)(base + 0x10))   /* 高电平触发中断使能寄存器 */
                                                                           /* EDGE 电平触发时: 1=高电平触发, 0=低电平触发
                                                                              EDGE 边沿触发时: 1=上升沿触发, 0=下降沿触发 */
#define LS1B_INTC_EDGE(base)    (*(volatile unsigned int*)(base + 0x14))   /* 边沿触发中断使能寄存器; 1:边沿触发, 0: 电平触发 */

/******************************************************************************
 * XXX for ls1x driver
 */
#define LS1x_INTC0_BASE         LS1B_INTC0_BASE
#define LS1x_INTC1_BASE         LS1B_INTC1_BASE
#define LS1x_INTC2_BASE         LS1B_INTC2_BASE
#define LS1x_INTC3_BASE         LS1B_INTC3_BASE

#define LS1x_INTC_ISR(base)     LS1B_INTC_ISR(base)
#define LS1x_INTC_IEN(base)     LS1B_INTC_IEN(base)
#define LS1x_INTC_SET(base)     LS1B_INTC_SET(base)
#define LS1x_INTC_CLR(base)     LS1B_INTC_CLR(base)
#define LS1x_INTC_POL(base)     LS1B_INTC_POL(base)
#define LS1x_INTC_EDGE(base)    LS1B_INTC_EDGE(base)

/*
 * Interrupt Control 0 source bit
 */
#define INTC0_UART5_BIT         bit(30)
#define INTC0_UART4_BIT         bit(29)
#define INTC0_TOY_BIT           bit(28)
#define INTC0_RTC_BIT           bit(27)
#define INTC0_TOY2_BIT          bit(26)
#define INTC0_TOY1_BIT          bit(25)
#define INTC0_TOY0_BIT          bit(24)
#define INTC0_RTC2_BIT          bit(23)
#define INTC0_RTC1_BIT          bit(22)
#define INTC0_RTC0_BIT          bit(21)
#define INTC0_PWM3_BIT          bit(20)
#define INTC0_PWM2_BIT          bit(19)
#define INTC0_PWM1_BIT          bit(18)
#define INTC0_PWM0_BIT          bit(17)
#define INTC0_DMA2_BIT          bit(15)
#define INTC0_DMA1_BIT          bit(14)
#define INTC0_DMA0_BIT          bit(13)
#define INTC0_AC97_BIT          bit(10)
#define INTC0_SPI1_BIT          bit(9)
#define INTC0_SPI0_BIT          bit(8)
#define INTC0_CAN1_BIT          bit(7)
#define INTC0_CAN0_BIT          bit(6)
#define INTC0_UART3_BIT         bit(5)
#define INTC0_UART2_BIT         bit(4)
#define INTC0_UART1_BIT         bit(3)
#define INTC0_UART0_BIT         bit(2)

/*
 * Interrupt Control 1 source bit
 */
#define INTC1_GMAC1_BIT         bit(3)
#define INTC1_GMAC0_BIT         bit(2)
#define INTC1_OHCI_BIT          bit(1)
#define INTC1_EHCI_BIT          bit(0)

/*
 * Interrupt Control 2 source bit (GPIO)
 */
#define INTC2_GPIO30_BIT        bit(30)
#define INTC2_GPIO29_BIT        bit(29)
#define INTC2_GPIO28_BIT        bit(28)
#define INTC2_GPIO27_BIT        bit(27)
#define INTC2_GPIO26_BIT        bit(26)
#define INTC2_GPIO25_BIT        bit(25)
#define INTC2_GPIO24_BIT        bit(24)
#define INTC2_GPIO23_BIT        bit(23)
#define INTC2_GPIO22_BIT        bit(22)
#define INTC2_GPIO21_BIT        bit(21)
#define INTC2_GPIO20_BIT        bit(20)
#define INTC2_GPIO19_BIT        bit(19)
#define INTC2_GPIO18_BIT        bit(18)
#define INTC2_GPIO17_BIT        bit(17)
#define INTC2_GPIO16_BIT        bit(16)
#define INTC2_GPIO15_BIT        bit(15)
#define INTC2_GPIO14_BIT        bit(14)
#define INTC2_GPIO13_BIT        bit(13)
#define INTC2_GPIO12_BIT        bit(12)
#define INTC2_GPIO11_BIT        bit(11)
#define INTC2_GPIO10_BIT        bit(10)
#define INTC2_GPIO9_BIT         bit(9)
#define INTC2_GPIO8_BIT         bit(8)
#define INTC2_GPIO7_BIT         bit(7)
#define INTC2_GPIO6_BIT         bit(6)
#define INTC2_GPIO5_BIT         bit(5)
#define INTC2_GPIO4_BIT         bit(4)
#define INTC2_GPIO3_BIT         bit(3)
#define INTC2_GPIO2_BIT         bit(2)
#define INTC2_GPIO1_BIT         bit(1)
#define INTC2_GPIO0_BIT         bit(0)

/*
 * Interrupt Control 3 source bit (GPIO)
 */
#define INTC3_GPIO61_BIT        bit(29)
#define INTC3_GPIO60_BIT        bit(28)
#define INTC3_GPIO59_BIT        bit(27)
#define INTC3_GPIO58_BIT        bit(26)
#define INTC3_GPIO57_BIT        bit(25)
#define INTC3_GPIO56_BIT        bit(24)
#define INTC3_GPIO55_BIT        bit(23)
#define INTC3_GPIO54_BIT        bit(22)
#define INTC3_GPIO53_BIT        bit(21)
#define INTC3_GPIO52_BIT        bit(20)
#define INTC3_GPIO51_BIT        bit(19)
#define INTC3_GPIO50_BIT        bit(18)
#define INTC3_GPIO49_BIT        bit(17)
#define INTC3_GPIO48_BIT        bit(16)
#define INTC3_GPIO47_BIT        bit(15)
#define INTC3_GPIO46_BIT        bit(14)
#define INTC3_GPIO45_BIT        bit(13)
#define INTC3_GPIO44_BIT        bit(12)
#define INTC3_GPIO43_BIT        bit(11)
#define INTC3_GPIO42_BIT        bit(10)
#define INTC3_GPIO41_BIT        bit(9)
#define INTC3_GPIO40_BIT        bit(8)
#define INTC3_GPIO39_BIT        bit(7)
#define INTC3_GPIO38_BIT        bit(6)
#define INTC3_GPIO37_BIT        bit(5)
#define INTC3_GPIO36_BIT        bit(4)
#define INTC3_GPIO35_BIT        bit(3)
#define INTC3_GPIO34_BIT        bit(2)
#define INTC3_GPIO33_BIT        bit(1)
#define INTC3_GPIO32_BIT        bit(0)

/*
 * WatchDog
 */
#define LS1B_WATCHDOG_EN_ADDR       0xBFE5C060
#define WATCHDOG_ENABLE             bit(0)              /* 看门狗使能 */

#define LS1B_WATCHDOG_TIMER_ADDR    0xBFE5C064          /* 计数值 */

#define LS1B_WATCHDOG_SET_ADDR      0xBFE5C068
#define WATCHDOG_START              bit(0)              /* 看门狗开始计数 */

/******************************************************************************
 * use for delay slot
 */
void static inline ls1x_sync(void)
{
    __asm__ volatile ("sync");
}

#define LS1X_SYNC   do { __asm__ volatile ("sync"); } while (0)

/******************************************************************************
 * cpu work at kseg0 cache mode
 ******************************************************************************/

#include <stddef.h>         /* size_t */

/**********************************************************************
 * OPERATE DATA AND INSTRUCTION CACHE
 **********************************************************************/

/* Flush and invalidate all caches
 */
extern void flush_cache(void);

/* Invalidate all caches
 */
extern void flush_cache_nowrite(void);

/* Writeback and invalidate address range in all caches
 */
extern void clean_cache(unsigned kva, size_t n);

/**********************************************************************
 * OPERATE DATA CACHE
 **********************************************************************/

/* Flush and invalidate data cache
 */
extern void flush_dcache(void);

/* Writeback and invalidate address range in primary data cache
 */
extern void clean_dcache(unsigned kva, size_t n);

/* Invalidate an address range in primary data cache
 */
extern void clean_dcache_nowrite(unsigned kva, size_t n);

/**********************************************************************
 * OPERATE INSTRUCTION CACHE
 **********************************************************************/

/* Flush and invalidate instruction cache
 */
extern void flush_icache(void);

/* Invalidate address range in primary instruction cache
 */
extern void clean_icache(unsigned kva, size_t n);

/******************************************************************************
 * extern functions
 ******************************************************************************/

/* Generate a software interrupt */
extern int assert_sw_irq(unsigned int irqnum);

/* Clear a software interrupt */
extern int negate_sw_irq(unsigned int irqnum);


#ifdef  __cplusplus
}
#endif

#endif  //__LS1B_H__

