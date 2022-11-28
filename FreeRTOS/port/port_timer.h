/*
 * Copyright (C) 2020-2021 Suzhou Tiancheng Software Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _PORT_TIMER_H
#define	_PORT_TIMER_H

#ifdef	__cplusplus
extern "C"
{
#endif

void portClearTickTimerInterrupt( void );
#define configCLEAR_TICK_TIMER_INTERRUPT() portClearTickTimerInterrupt()

extern void portClearTickTimer( void );
extern void vApplicationSetupTickTimerInterrupt( void );

/******************************************************************************
 * —” ±≥Ã–Ú
 */

void delay_us(unsigned int us);
void delay_ms(unsigned int ms);

#ifdef	__cplusplus
}
#endif

#endif	/* _PORT_TIMER_H */

