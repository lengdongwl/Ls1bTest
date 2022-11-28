/*
 * ls1x_bsp_init.c
 */

#include "bsp.h"

#include "ns16550.h"

#if BSP_USE_LWMEM
#include "../libc/lwmem.h"
#endif

#if defined(BSP_USE_SPI0) || defined(BSP_USE_SPI1)
#include "ls1x_spi_bus.h"
#endif

#if defined(BSP_USE_I2C0) || defined(BSP_USE_I2C1) || defined(BSP_USE_I2C2)
#include "ls1x_i2c_bus.h"
#endif

#if defined(BSP_USE_CAN0) || defined(BSP_USE_CAN1)
#include "ls1x_can.h"
#endif

#ifdef BSP_USE_NAND
#include "ls1x_nand.h"
#endif

#ifdef BSP_USE_FB       
#include "ls1x_fb.h"
#endif

#if defined(BSP_USE_GMAC0) || defined(BSP_USE_GMAC1)
#include "ls1x_gmac.h"
#endif

#if defined(BSP_USE_PWM0) || defined(BSP_USE_PWM1) || \
    defined(BSP_USE_PWM2) || defined(BSP_USE_PWM3)
#include "ls1x_pwm.h"
#endif

#ifdef BSP_USE_RTC
#include "ls1x_rtc.h"
#endif

#ifdef BSP_USE_AC97
#include "ls1x_ac97.h"
#endif

/*
 * I2C ´ÓÉè±¸
 */
#ifdef BSP_USE_I2C0
#if defined(BSP_USE_FB) && defined(GP7101_DRV)
#include "i2c/gp7101.h"         // LCD ÁÁ¶È¿ØÖÆ
#endif
#ifdef PCA9557_DRV
#include "i2c/pca9557.h"        // GPIO Ð¾Æ¬
#endif
#ifdef ADS1015_DRV
#include "i2c/ads1015.h"        // 4Â· 12bit ADC
#endif
#ifdef MCP4725_DRV
#include "i2c/mcp4725.h"        // 1Â· 12bit DAC
#endif
#ifdef RX8010_DRV
#include "i2c/rx8010.h"         // RTC Ð¾Æ¬
#endif
#ifdef GT1151_DRV
//#include "i2c/gt1151.h"         // ÊúÆÁ´¥ÃþÐ¾Æ¬
#include "gt1151q.h"         // ÊúÆÁ´¥ÃþÐ¾Æ¬
#endif
#endif

/*
 * SPI ´ÓÉè±¸
 */
#ifdef BSP_USE_SPI0
#ifdef W25X40_DRV
#include "spi/w25x40.h"         // SPI Flash
#endif
#if defined(BSP_USE_FB) && defined(XPT2046_DRV)
#include "spi/xpt2046.h"        // ´¥ÃþÆÁÐ¾Æ¬
#endif
#endif

//-----------------------------------------------------------------------------
// Initialize drivers according BSP's Configuration
//-----------------------------------------------------------------------------

int ls1x_drv_init(void)
{
    #if BSP_USE_LWMEM
        lwmem_initialize(0);
    #endif

  #if defined(LS1B)
    #ifdef BSP_USE_UART2
        ls1x_uart_init(devUART2, NULL);;
    #endif
    #ifdef BSP_USE_UART3
        ls1x_uart_init(devUART3, NULL);
    #endif
    #ifdef BSP_USE_UART4
        ls1x_uart_init(devUART4, NULL);
    #endif
    #ifdef BSP_USE_UART5
        ls1x_uart_init(devUART5, NULL);
    #endif
    #ifdef BSP_USE_UART0
        ls1x_uart_init(devUART0, NULL);
    #endif
    #ifdef BSP_USE_UART01
        ls1x_uart_init(devUART01, NULL);
    #endif
    #ifdef BSP_USE_UART02
        ls1x_uart_init(devUART02, NULL);
    #endif
    #ifdef BSP_USE_UART03
        ls1x_uart_init(devUART03, NULL);
    #endif
    #ifdef BSP_USE_UART1
        ls1x_uart_init(devUART1, NULL);
    #endif
    #ifdef BSP_USE_UART11
        ls1x_uart_init(devUART11, NULL);
    #endif
    #ifdef BSP_USE_UART12
        ls1x_uart_init(devUART12, NULL);
    #endif
    #ifdef BSP_USE_UART13
        ls1x_uart_init(devUART13, NULL);
    #endif
  #elif defined(LS1C)
    #ifdef BSP_USE_UART0
        ls1x_uart_init(devUART0, NULL);
    #endif
    #ifdef BSP_USE_UART1
        ls1x_uart_init(devUART1, NULL);
    #endif
    #ifdef BSP_USE_UART2
        ls1x_uart_init(devUART2, NULL);
    #endif
    #ifdef BSP_USE_UART3
        ls1x_uart_init(devUART3, NULL);
    #endif
    #ifdef BSP_USE_UART4
        ls1x_uart_init(devUART4, NULL);
    #endif
    #ifdef BSP_USE_UART5
        ls1x_uart_init(devUART5, NULL);
    #endif
    #ifdef BSP_USE_UART6
        ls1x_uart_init(devUART6, NULL);
    #endif
    #ifdef BSP_USE_UART7
        ls1x_uart_init(devUART7, NULL);
    #endif
    #ifdef BSP_USE_UART8
        ls1x_uart_init(devUART8, NULL);
    #endif
    #ifdef BSP_USE_UART9
        ls1x_uart_init(devUART9, NULL);
    #endif
    #ifdef BSP_USE_UART10
        ls1x_uart_init(devUART10, NULL);
    #endif
    #ifdef BSP_USE_UART11
        ls1x_uart_init(devUART11, NULL);
    #endif
  #endif

    #ifdef BSP_USE_SPI0
        ls1x_spi_initialize(busSPI0);
    #endif
    #ifdef BSP_USE_SPI1
        ls1x_spi_initialize(busSPI1);
    #endif

    #ifdef BSP_USE_I2C0
        ls1x_i2c_initialize(busI2C0);
    #endif
    #ifdef BSP_USE_I2C1
        ls1x_i2c_initialize(busI2C1);
    #endif
    #ifdef BSP_USE_I2C2
        ls1x_i2c_initialize(busI2C2);
    #endif

    #ifdef BSP_USE_CAN0
        ls1x_can_init(devCAN0, NULL);
    #endif
    #ifdef BSP_USE_CAN1
        ls1x_can_init(devCAN1, NULL);
    #endif

    #ifdef BSP_USE_NAND
        ls1x_nand_init(devNAND, NULL);
    #endif

    #ifdef BSP_USE_FB
        fb_open(); // ls1x_dc_init(devDC, NULL);
    #endif

    #ifdef BSP_USE_GMAC0
        //
    #endif
    #if defined(LS1B) &&  defined(BSP_USE_GMAC1)
        //
    #endif

    #ifdef BSP_USE_PWM0
        ls1x_pwm_init(devPWM0, NULL);
    #endif
    #ifdef BSP_USE_PWM1
        ls1x_pwm_init(devPWM1, NULL);
    #endif
    #ifdef BSP_USE_PWM2
        ls1x_pwm_init(devPWM2, NULL);
    #endif
    #ifdef BSP_USE_PWM3
        ls1x_pwm_init(devPWM3, NULL);
    #endif

    #ifdef BSP_USE_RTC
        ls1x_rtc_init(NULL, NULL);
    #endif

    #ifdef BSP_USE_AC97
        ls1x_ac97_init(NULL, NULL);
    #endif

    /**
     * I2C ´ÓÉè±¸
     */
    #ifdef GP7101_DRV
        // set_lcd_brightness(busI2C0, 90);     // LCD ÁÁ¶È¿ØÖÆ
    #endif
    #ifdef PCA9557_DRV
        ls1x_pca9557_init(busI2C0, NULL);       // GPIO Ð¾Æ¬
    #endif
    #ifdef ADS1015_DRV
        //                                      // 4Â· 12bit ADC
    #endif
    #ifdef MCP4725_DRV
        //                                      // 1Â· 12bit DAC
    #endif
    #ifdef RX8010_DRV
        ls1x_rx8010_init(busI2C0, NULL);        // RTC Ð¾Æ¬
    #endif
    #ifdef GT1151_DRV
        GT1151_Init();
        //ls1x_gt1151_init(busI2C0, NULL);        // ÊúÆÁ´¥ÃþÐ¾Æ¬
    #endif

    /**
     * SPI ´ÓÉè±¸
     */
    #ifdef W25X40_DRV
        // ls1x_w25x40_init(busSPI0, NULL);     // SPI Flash
    #endif
    #ifdef XPT2046_DRV
        ls1x_xpt2046_init(busSPI0, NULL);       // ´¥ÃþÆÁÐ¾Æ¬
    #endif

    return 0;
}

