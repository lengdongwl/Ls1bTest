/*
 * ls1x_watchdog.c
 *
 * created: 2021/3/12
 *  author: 
 */

#include "bsp.h"

#if defined(BSP_USE_WATCHDOG)

#include <stdint.h>

#if defined(LS1B)
#include "ls1b.h"
#define LS1x_WATCHDOG_EN_REG    LS1B_WATCHDOG_EN_ADDR
#define LS1x_WATCHDOG_TIMER_REG LS1B_WATCHDOG_TIMER_ADDR
#define LS1x_WATCHDOG_SET_REG   LS1B_WATCHDOG_SET_ADDR
#elif defined(LS1C)
#include "ls1c.h"
#define LS1x_WATCHDOG_EN_REG    LS1C_WDT_EN_ADDR
#define LS1x_WATCHDOG_TIMER_REG LS1C_WDT_TIMER_ADDR
#define LS1x_WATCHDOG_SET_REG   LS1C_WDT_SET_ADDR
#else
#error "No Loongson1x SoC defined."
#endif

#include "ls1x_io.h"
#include "ls1x_watchdog.h"

//-------------------------------------------------------------------------------------------------
// DOG driver implement
//-------------------------------------------------------------------------------------------------

static int bus_clocks_per_ms = 0;

/*
 * Parameters: dev: NULL
 *             arg: unsigned int * ms
 */
STATIC_DRV int LS1x_DOG_open(void *dev, void *arg)
{
    unsigned int ms = *((unsigned int *)arg);
    
    if (ms == 0)
        return -1;

    bus_clocks_per_ms = LS1x_BUS_FREQUENCY(CPU_XTAL_FREQUENCY) / 1000;
    ms *= bus_clocks_per_ms;

    WRITE_REG32(LS1x_WATCHDOG_EN_REG, 1);
    WRITE_REG32(LS1x_WATCHDOG_TIMER_REG, ms);
    WRITE_REG32(LS1x_WATCHDOG_SET_REG, 1);

    return 0;
}

/*
 * Parameters: dev: NULL
 *             arg: NULL
 */
STATIC_DRV int LS1x_DOG_close(void *dev, void *arg)
{
    WRITE_REG32(LS1x_WATCHDOG_SET_REG, 0);
    WRITE_REG32(LS1x_WATCHDOG_EN_REG, 0);

    return 0;
}

/*
 * Parameters: dev:  NULL
 *             buf:  unsigned int * 返回当前 dog 的 ms 数值
 *             size: 4
 */
STATIC_DRV int LS1x_DOG_write(void *dev, void *buf, int size, void *arg)
{
    unsigned int ms = *((unsigned int *)buf);

    if (ms == 0)
    {
        return LS1x_DOG_close(NULL, NULL);
    }
    
    if (READ_REG32(LS1x_WATCHDOG_EN_REG) == 0)
        return 0;

    ms *= bus_clocks_per_ms;
    WRITE_REG32(LS1x_WATCHDOG_SET_REG, 0);
    WRITE_REG32(LS1x_WATCHDOG_TIMER_REG, ms);
    WRITE_REG32(LS1x_WATCHDOG_SET_REG, 1);
    
    return 4;
}

#if (PACK_DRV_OPS)
/******************************************************************************
 * DOG driver operators
 */
static driver_ops_t LS1x_DOG_drv_ops =
{
    .init_entry  = NULL,
    .open_entry  = LS1x_DOG_open,
    .close_entry = LS1x_DOG_close,
    .read_entry  = NULL,
    .write_entry = LS1x_DOG_write,
    .ioctl_entry = NULL,
};

driver_ops_t *ls1x_dog_drv_ops = &LS1x_DOG_drv_ops;
#endif

/******************************************************************************
 * user api
 */
int ls1x_watchdog_start(unsigned int ms)
{
    return LS1x_DOG_open(NULL, (void *)&ms);
}

int ls1x_watchdog_feed(unsigned int ms)
{
    return LS1x_DOG_write(NULL, (void *)&ms, 4, NULL);
}

int ls1x_watchdog_stop(void)
{
    return LS1x_DOG_close(NULL, NULL);
}

#endif // #if defined(BSP_USE_WATCHDOG)


