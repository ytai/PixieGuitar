/*
    FreeRTOS V8.0.1 - Copyright (C) 2014 Real Time Engineers Ltd.
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

#ifndef PORTMACRO_H
#define PORTMACRO_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------
 * Port specific definitions.
 *
 * The settings in this file configure FreeRTOS correctly for the
 * given hardware and compiler.
 *
 * These settings should not be altered.
 *-----------------------------------------------------------
 */

/* Type definitions. */
#define portCHAR		char
#define portFLOAT		float
#define portDOUBLE		double
#define portLONG		long
#define portSHORT		short
#define portSTACK_TYPE	uint16_t
#define portBASE_TYPE	short

typedef portSTACK_TYPE StackType_t;
typedef short BaseType_t;
typedef unsigned short UBaseType_t;

#if( configUSE_16_BIT_TICKS == 1 )
	typedef uint16_t TickType_t;
	#define portMAX_DELAY ( TickType_t ) 0xffff
#else
	typedef uint32_t TickType_t;
	#define portMAX_DELAY ( TickType_t ) 0xffffffffUL
#endif
/*-----------------------------------------------------------*/

/* Hardware specifics. */
#define portBYTE_ALIGNMENT			2
#define portSTACK_GROWTH			1
#define portTICK_PERIOD_MS			( ( TickType_t ) 1000 / configTICK_RATE_HZ )
/*-----------------------------------------------------------*/

/* Critical section management. */
#define portDISABLE_INTERRUPTS()	_IPL = configKERNEL_INTERRUPT_PRIORITY
#define portENABLE_INTERRUPTS()		_IPL = 0

#if ( configGENERATE_RUN_TIME_STATS == 1 )
extern uint32_t xPortGetRunTimeCounterValue( void );
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portGET_RUN_TIME_COUNTER_VALUE() xPortGetRunTimeCounterValue()
#endif

/* Note that exiting a critical sectino will set the IPL bits to 0, nomatter
what their value was prior to entering the critical section. */
extern void vPortEnterCritical( void );
extern void vPortExitCritical( void );
#define portENTER_CRITICAL()		vPortEnterCritical()
#define portEXIT_CRITICAL()			vPortExitCritical()
/*-----------------------------------------------------------*/

/* Task utilities. */
extern void vPortYield( void );
#define portYIELD()				asm volatile ( "CALL _vPortYield			\n"		\
												"NOP					  " );
/*-----------------------------------------------------------*/

/* Task function macros as described on the FreeRTOS.org WEB site. */
#define portTASK_FUNCTION_PROTO( vFunction, pvParameters ) void vFunction( void *pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters ) void vFunction( void *pvParameters )
/*-----------------------------------------------------------*/

/* Required by the kernel aware debugger. */
#ifdef __DEBUG
	#define portREMOVE_STATIC_QUALIFIER
#endif

#define portNOP()				asm volatile ( "NOP" )

# if defined(__dsPIC33E__)
        #define portSAVE_CONTEXT()                                                                                          \
            asm volatile(   "PUSH	SR						\n"/* Save the SR used by the task.... */                       \
                            "PUSH	W0						\n"/* ....then disable interrupts. */                           \
                            "MOV    #32, W0                 \n"                                                             \
                            "MOV	W0, SR                  \n"                                                             \
                            "PUSH	W1						\n"/* Save registers to the stack. */                           \
                            "PUSH.D	W2                      \n"                                                             \
                            "PUSH.D	W4                      \n"                                                             \
                            "PUSH.D	W6                      \n"                                                             \
                            "PUSH.D W8                      \n"                                                             \
                            "PUSH.D W10                     \n"                                                             \
                            "PUSH.D	W12                     \n"                                                             \
                            "PUSH	W14                     \n"                                                             \
                            "PUSH	RCOUNT                  \n"                                                             \
                            "PUSH	TBLPAG                  \n"                                                             \
                            "PUSH	ACCAL                   \n"                                                             \
                            "PUSH	ACCAH                   \n"                                                             \
                            "PUSH	ACCAU                   \n"                                                             \
                            "PUSH	ACCBL                   \n"                                                             \
                            "PUSH	ACCBH                   \n"                                                             \
                            "PUSH	ACCBU                   \n"                                                             \
                            "PUSH	CORCON                  \n"                                                             \
                            "PUSH	DSRPAG                  \n"                                                             \
                            "PUSH	DSWPAG                  \n"                                                             \
                            "MOV	_uxCriticalNesting, W0	\n"	/* Save the critical nesting counter for the task. */       \
                            "PUSH	W0                      \n"                                                             \
                            "MOV	_pxCurrentTCB, W0		\n"	/* Save the new top of stack into the TCB. */               \
                            "MOV	W15, [W0]                 ");

        #define portRESTORE_CONTEXT()																						\
			asm volatile(	"MOV	_pxCurrentTCB, W0		\n"	/* Restore the stack pointer for the task. */				\
							"MOV	[W0], W15				\n"																\
							"POP	W0						\n"	/* Restore the critical nesting counter for the task. */	\
							"MOV	W0, _uxCriticalNesting	\n"																\
							"POP	DSWPAG					\n"																\
							"POP    DSRPAG					\n"																\
							"POP	CORCON					\n"																\
							"POP	ACCBU					\n"																\
							"POP	ACCBH					\n"																\
							"POP	ACCBL					\n"																\
							"POP	ACCAU					\n"																\
							"POP	ACCAH					\n"																\
							"POP	ACCAL					\n"																\
							"POP	TBLPAG					\n"																\
							"POP	RCOUNT					\n"	/* Restore the registers from the stack. */					\
							"POP	W14						\n"																\
							"POP.D	W12						\n"																\
							"POP.D	W10						\n"																\
							"POP.D	W8						\n"																\
							"POP.D	W6						\n"																\
							"POP.D	W4						\n"																\
							"POP.D	W2						\n"																\
							"POP.D	W0						\n"																\
							"POP	SR						  " );
#endif  // defined(__dsPIC33E__)

#ifdef __cplusplus
}
#endif

#endif /* PORTMACRO_H */

