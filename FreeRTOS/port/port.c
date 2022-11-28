/*
 * FreeRTOS V8.1.2 - Copyright (C) 2014 Real Time Engineers Ltd.
 * All rights reserved
 *
 * VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.
 *
 */

/*-----------------------------------------------------------------------------
 * Implementation of functions defined in portable.h for the MIPS32 port.
 *----------------------------------------------------------------------------*/

#include <string.h>

#include "regdef.h"
#include "mips.h"

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

/* Timer handling files */
#include "port_timer.h"

/* Interrupt handler */
#include "interrupt.h"
#include "ISR_Support.h"    // CTX_SIZE

/* Let the user override the pre-loading of the initial RA with the address of
   prvTaskExitError() in case is messes up unwinding of the stack in the
   debugger - in which case configTASK_RETURN_ADDRESS can be defined as 0 (NULL). */
#ifdef configTASK_RETURN_ADDRESS
	#define portTASK_RETURN_ADDRESS	configTASK_RETURN_ADDRESS
#else
	#define portTASK_RETURN_ADDRESS	prvTaskExitError
#endif

/* Set configCHECK_FOR_STACK_OVERFLOW to 3 to add ISR stack checking to task
   stack checking.  A problem in the ISR stack will trigger an assert, not call the
   stack overflow hook function (because the stack overflow hook is specific to a
   task stack, not the ISR stack). */
#if( configCHECK_FOR_STACK_OVERFLOW > 2 )

	/* Don't use 0xa5 as the stack fill bytes as that is used by the kernel for
	   the task stacks, and so will legitimately appear in many positions within
	   the ISR stack. */
	#define portISR_STACK_FILL_BYTE	0xee

	static const uint8_t ucExpectedStackBytes[] = {
		portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,   \
		portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,   \
		portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,	  \
		portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE,	  \
		portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE, portISR_STACK_FILL_BYTE }; \

	#define portCHECK_ISR_STACK() configASSERT( ( memcmp( ( void * ) xISRStack, ( void * ) ucExpectedStackBytes, sizeof( ucExpectedStackBytes ) ) == 0 ) )
#else
	/* Define the function away. */
	#define portCHECK_ISR_STACK()
#endif /* configCHECK_FOR_STACK_OVERFLOW > 2 */

/*-----------------------------------------------------------*/

/*
 * Place the prototype here to ensure the interrupt vector is correctly installed.
 * Note that because the interrupt is written in assembly, the IPL setting in the
 * following line of code has no effect.  The interrupt priority is set by the
 * call to ConfigIntTimer1() in vApplicationSetupTickTimerInterrupt().
 */
//extern void vPortTickInterruptHandler( void );

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );

/*-----------------------------------------------------------*/

/* Records the interrupt nesting depth. - unused currently as no int stack */
volatile UBaseType_t uxInterruptNesting = 0x0;

/* Stores the task stack pointer when a switch is made to use the system stack. */
UBaseType_t uxSavedTaskStackPointer = 0;

/* The stack used by interrupt service routines that cause a context switch. */
StackType_t xISRStack[ configISR_STACK_SIZE ] = { 0 };

/* The top of stack value ensures there is enough space to store 6 registers on
   the callers stack, as some functions seem to want to do this. */
const StackType_t * const xISRStackTop = &( xISRStack[ configISR_STACK_SIZE - 7 ] );

/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
    unsigned int sr=0, cause;
    StackType_t *stk;

#if (portSTACK_GROWTH < 0)
    stk = (StackType_t *)((uint32_t)pxTopOfStack & ~0x7);
#else
#error "TODO stack in ISR_Support.h"
#endif

	*stk = (StackType_t) 0xDEADBEEF;
	stk--;

	*stk = (StackType_t) 0x12345678;       /* Word to which the stack pointer will be left pointing after context restore. */
	stk--;

    //-------------------------------------------------------------------------
    // FPU into stack together, SEE "context.h"
    //-------------------------------------------------------------------------

	/* create a space for a full context */
#if (portSTACK_GROWTH < 0)
	stk -= CTX_SIZE/4;
#else
#error "TODO stack in ISR_Support.h"
#endif

    mips_get_sr(sr);
    mips_get_cause(cause);

	/* fill up some initial values for us to kick in
	 */
	*(stk + R_CAUSE) = (StackType_t) cause;
	*(stk + R_SR   ) = (StackType_t) (sr | portINITIAL_SR); /* ALWAYS include SR_TIMER_INT */
	*(stk + R_EPC  ) = (StackType_t) pxCode;
	*(stk + R_RA   ) = (StackType_t) portTASK_RETURN_ADDRESS;
	*(stk + R_A0   ) = (StackType_t) pvParameters; /* Parameters to pass in. */

#if __mips_hard_float
    *(stk + R_FCSR ) = (StackType_t) 0;
#endif

	/* Save GP register value in the context */
 	asm volatile ("sw $gp, %0" : "=m" (*(stk + R_GP)));

	return stk;
}
/*-----------------------------------------------------------*/

static void prvTaskExitError( void )
{
	/* A function that implements a task must not exit or attempt to return to
	its caller as there is nothing to return to.  If a task wants to exit it
	should instead call vTaskDelete( NULL ).

	Artificially force an assert() to be triggered if configASSERT() is
	defined, then stop here so application writers can catch the error. */
	configASSERT( uxSavedTaskStackPointer == 0UL );
	portDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

void vPortEndScheduler(void)
{
	/* Not implemented in ports where there is nothing to return to.
	   Artificially force an assert. */
	configASSERT( uxInterruptNesting == 1000UL );
}
/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler( void )
{
    extern void vPortStartFirstTask( void );
    extern void *pxCurrentTCB;

	#if ( configCHECK_FOR_STACK_OVERFLOW > 2 )
	{
		/* Fill the ISR stack to make it easy to asses how much is being used. */
		memset( ( void * ) xISRStack, portISR_STACK_FILL_BYTE, sizeof( xISRStack ) );
	}
	#endif /* configCHECK_FOR_STACK_OVERFLOW > 2 */

	/* Clear the pending interrupts */
	portCLEAR_PENDING_INTERRUPTS();

	/* Setup the timer to generate the tick.  Interrupts will have been
	   disabled by the time we get here. */
	vApplicationSetupTickTimerInterrupt();

	/* Install the software yield handler */
	vApplicationSetupSoftwareInterrupt();

	/* Kick off the highest priority task that has been created so far.
	   Its stack location is loaded into uxSavedTaskStackPointer. */
	uxSavedTaskStackPointer = *( UBaseType_t * ) pxCurrentTCB;
	vPortStartFirstTask();

	/* Should never get here as the tasks will now be executing!  Call the task
 	   exit error function to prevent compiler warnings about a static function
	   not being called in the case that the application writer overrides this
	   functionality by defining configTASK_RETURN_ADDRESS. */
	prvTaskExitError();

	return pdFALSE;
}
/*-----------------------------------------------------------*/

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
static volatile unsigned int Clock_driver_ticks;    /* Clock ticks since initialization */

unsigned int get_clock_ticks(void)
{
    return Clock_driver_ticks;
}

//-----------------------------------------------------------------------------

void vPortIncrementTick(int vector, void *arg)
{
    UBaseType_t uxSavedStatus;

    ++Clock_driver_ticks;       /* FIXME 2020.12.30 */

	uxSavedStatus = uxPortSetInterruptMaskFromISR();
	{
		if( xTaskIncrementTick() != pdFALSE )
		{
			/* Pend a context switch. */
			portYIELD();
		}
	}
	vPortClearInterruptMaskFromISR( uxSavedStatus );

	/* Look for the ISR stack getting near or past its limit. */
	portCHECK_ISR_STACK();

	/* Clear timer interrupt. */
	configCLEAR_TICK_TIMER_INTERRUPT();
}

/*-----------------------------------------------------------*/

#if configCHECK_FOR_STACK_OVERFLOW > 0
/* Assert at a stack overflow. */
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
	configASSERT( xTask == NULL );
}
#endif


