/*
	FreeRTOS V8.1.2 - Copyright (C) 2014 Real Time Engineers Ltd.
	All rights reserved

	VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

	***************************************************************************
	 *                                                                       *
	 *    FreeRTOS provides completely free yet professionally developed,    *
	 *    robust, strictly quality controlled, supported, and cross          *
	 *    platform software that has become a de facto standard.             *
	 *                                                                       *
	 *    Help yourself get started quickly and support the FreeRTOS         *
	 *    project by purchasing a FreeRTOS tutorial book, reference          *
	 *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
	 *                                                                       *
	 *    Thank you!                                                         *
	 *                                                                       *
	***************************************************************************

	This file is part of the FreeRTOS distribution.

	FreeRTOS is free software; you can redistribute it and/or modify it under
	the terms of the GNU General Public License (version 2) as published by the
	Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

	>>!   NOTE: The modification to the GPL is included to allow you to     !<<
	>>!   distribute a combined work that includes FreeRTOS without being   !<<
	>>!   obliged to provide the source code for proprietary components     !<<
	>>!   outside of the FreeRTOS kernel.                                   !<<

	FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE.  Full license text is available from the following
	link: http://www.freertos.org/a00114.html

	1 tab == 4 spaces!

	***************************************************************************
	 *                                                                       *
	 *    Having a problem?  Start by reading the FAQ "My application does   *
	 *    not run, what could be wrong?"                                     *
	 *                                                                       *
	 *    http://www.FreeRTOS.org/FAQHelp.html                               *
	 *                                                                       *
	***************************************************************************

	http://www.FreeRTOS.org - Documentation, books, training, latest versions,
	license and Real Time Engineers Ltd. contact details.

	http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
	including FreeRTOS+Trace - an indispensable productivity tool, a DOS
	compatible FAT file system, and our tiny thread aware UDP/IP stack.

	http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
	Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
	licenses offer ticketed support, indemnification and middleware.

	http://www.SafeRTOS.com - High Integrity Systems also provide a safety
	engineered and independently SIL3 certified version for use in safety and
	mission critical applications that require provable dependability.

	1 tab == 4 spaces!
*/

#ifndef _INT_HANDLER_H
#define	_INT_HANDLER_H

#ifdef	__cplusplus
extern "C"
{
#endif

#include "portmacro.h"

/* Initialise the ISR table */
void vPortInitISR( void );

/* Interrupt manipulation */
extern void pvPortInstallISR( uint32_t, void ( * )( int, void * ) );

/*
 * The software interrupt handler that performs the yield.
 */
extern void vPortYieldISR( int, void * );

extern UBaseType_t uxPortSetInterruptMaskFromISR( void );
extern void vPortClearInterruptMaskFromISR( UBaseType_t );
extern void vApplicationSetupSoftwareInterrupt( void );
extern void vLevelTrigExternalNonEicInterrupt( uint32_t ext_int, uint32_t pol);
extern void vRouteExternalNonEicInterrupt( uint32_t ext_int, uint32_t vpe, uint32_t vpe_int);

#if __mips_hard_float
#define portINITIAL_SR                     (SR_CU1 | SR_IMASK | SR_IE) 
#else
#define portINITIAL_SR                     (SR_IMASK | SR_IE)
#endif

#define portCLEAR_PENDING_INTERRUPTS() \
    do { \
        unsigned int _cause;       \
        mips_get_cause(_cause);    \
        _cause &= ~(CAUSE_IPMASK); \
        mips_set_cause(_cause);    \
    } while (0)
    
#define portSET_INTERRUPT_MASK_FROM_ISR()   uxPortSetInterruptMaskFromISR()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedStatusRegister ) \
	vPortClearInterruptMaskFromISR( uxSavedStatusRegister )

/* Generate a software interrupt */
int assert_sw_irq(unsigned int irqnum);
/* Clear a software interrupt */
int negate_sw_irq(unsigned int irqnum);

#ifdef	__cplusplus
}
#endif

#endif	/* INT_HANDLER_H */

