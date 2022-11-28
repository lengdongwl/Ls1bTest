/*
 * FreeRTOS V8.1.2
 */

#ifndef _FREERTOS_CONFIG_H
#define _FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define CONFIG_CPU_CLOCK_HZ     264000000
#define CONFIG_TIMER_CLOCK_HZ   1000
#define CONFIG_TICK_RATE_HZ     1000

#define CONFIG_MAX_PRIORITIES   31
#define CONFIG_STACK_SIZE       (2*1024)
#define CONFIG_HEAP_SIZE        (1*1024*1024)       /* 堆大小: 1M~ */

#define configUSE_PREEMPTION                        1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION     0
#define configCPU_CLOCK_HZ                          ( CONFIG_CPU_CLOCK_HZ )
#define configTIMER_RATE_HZ                         ( ( TickType_t ) CONFIG_TIMER_CLOCK_HZ )
#define configTICK_RATE_HZ                          ( CONFIG_TICK_RATE_HZ )
#define configUSE_16_BIT_TICKS                      0
#define configMAX_PRIORITIES                        ( CONFIG_MAX_PRIORITIES )
#define configMINIMAL_STACK_SIZE                    ( CONFIG_STACK_SIZE )
#define configISR_STACK_SIZE                        ( CONFIG_STACK_SIZE )
#define configTOTAL_HEAP_SIZE						( ( size_t ) CONFIG_HEAP_SIZE )
#define configMAX_TASK_NAME_LEN                     ( 16 )
#define configCHECK_FOR_STACK_OVERFLOW              1           // remove when release

#define configUSE_MUTEXES                           1           // mutex: 设备保护与线程安全
#define configUSE_TIMERS                            1           // event: 中断与任务同步
#if configUSE_TIMERS
#define configTIMER_TASK_PRIORITY                   16          // 配置软件定时器任务的优先级: big is high
#define configTIMER_QUEUE_LENGTH                    8           // 配置软件定时器命令队列的长度
#define configTIMER_TASK_STACK_DEPTH                (2*1024)    // 配置软件定时器任务的栈空间大小
#endif

/* Hook functions */
#define configUSE_IDLE_HOOK                         0
#define configUSE_TICK_HOOK                         0

/* Co routines */
#define configUSE_CO_ROUTINES                       0

/* The interrupt priority of the RTOS kernel */
#define configKERNEL_INTERRUPT_PRIORITY             0x01

/* The maximum priority from which API functions can be called */
#define configMAX_API_CALL_INTERRUPT_PRIORITY       0x03

/* Prevent assert code from being used in assembly files */
#ifndef __ASSEMBLER__
  #if 0 // CANCEL It
	void vAssertCalled( const char *pcFileName, unsigned long ulLine );
	#define configASSERT( x )						\
		do {										\
			if( ( x ) == 0 )						\
				vAssertCalled( __FILE__, __LINE__ );\
		} while (0)
  #else
    #define configASSERT( x )
  #endif
#endif

/* Optional functions */
#define INCLUDE_vTaskPrioritySet					1
#define INCLUDE_uxTaskPriorityGet					1
#define INCLUDE_vTaskDelayUntil						1
#define INCLUDE_vTaskDelay							1
#define INCLUDE_vTaskDelete							1
#define INCLUDE_vTaskSuspend						1
#define INCLUDE_xTimerPendFunctionCall              1       // event group

#if defined(ENABLE_TRACE)
#include "trace.h"
#endif

#endif	/* FREERTOSCONFIG_H */

