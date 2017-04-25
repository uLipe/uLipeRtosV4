/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsBase.h
 *
 *  \brief uLipe base definitions file
 *
 *  In this file the is contained all the definitions about the uLipe Rtos 4,
 *  including configuration parameters, custom types, errors type and assert
 *  macro for debugging.
 *
 *  Author: FSN
 *
 */

#ifndef __OS_BASE_H
#define __OS_BASE_H

#include "OsConfig.h"
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdint.h>


/*
 *  custom rtos types:
 */

typedef uint32_t 	OsStack_t;		//Stack pointer type normalization
typedef uint32_t *  OsStackPtr_t;	//

typedef void *	    OsHandler_t;	//custom handler for kernel objects.


typedef enum						//Rtos status codes
{									//
	kStatusOk = 0,					//
	kInvalidParam,					//
	kKernelStartFail,				//
	kOutOfTasks,					//
	kCantSuspend,					//
	kQueueFull,						//
	kQueueEmpty,					//
	kOutOfQueue,					//
	kOutOfFlags,					//
	kOutOfMutex,					//
	kMutexOwned,					//
	kOutOfSem,						//
	kTimeout,                       //
	kOutOfMem,                      //
	kDeviceNotFound,                //
	kNotImplementedForThisDevice,	//
	kDeviceBusy,					//
	kDeviceEnabled,					//
	kDeviceDisabled,				//
	kDeviceIoError,					//
}OsStatus_t;						//

/*
 * Generic Event Block
 * Reserved for future use.

struct ecb_
{
	void *eventNode;							//Node for event type block
	uint8_t eventType;							//event type
	uint8_t tasksPending[OS_NUMBER_OF_TASKS];	//tasks pending this evnt
};

typedef struct ecb_  Event_t;
typedef struct ecb_* EventPtr_t;

 *
 *  custom rtos defitions
 */
#define TRUE  		 1
#define FALSE		 0

/*
 * 	priorities custom definitions
 */
#define OS_LEAST_PRIO	(0)
#define OS_HIGHEST_PRIO  (OS_NUMBER_OF_TASKS - 1)
#define OS_INVALID_PRIO (0xFFFF)


#if IDLE_TASK_HOOK_EN > 0
/*
 * User Hook proto:
 */
extern void IdleTaskHook(void);
#endif


/*
 *  assert definition:
 */
static inline void _uLipeAssert(uint32_t x)
{
	if(x == FALSE)
	{
		/**FIXME: add a printk here */

		/* traps the execution */
		while(TRUE);
	}
}




/* default ulipe configuration */

#ifndef OS_CPU_RATE
#define OS_CPU_RATE             48000000 //in Hz
#endif

#ifndef OS_TICK_RATE
#define OS_TICK_RATE            1000    //in Hz
#endif

#ifndef OS_ARCH_MULTICORE
#define OS_ARCH_MULTICORE       0
#endif

#ifndef OS_IDLE_TASK_HOOK_EN
#define OS_IDLE_TASK_HOOK_EN    0
#endif

#ifndef OS_FAST_SCHED
#define OS_FAST_SCHED           0
#endif

#ifndef OS_MINIMAL_STACK
#define OS_MINIMAL_STACK        32
#endif


#ifndef OS_DELAY_TIME_BASE
#define OS_DELAY_TIME_BASE      (10000/(OS_TICKS_PER_SECOND)) //In steps of 0.1ms
#endif

#ifndef OS_HEAP_SIZE
#define OS_HEAP_SIZE            128
#endif

#if defined(OS_TASK_MODULE_EN) && (OS_NUMBER_OF_TASKS == 0)
#define OS_NUMBER_OF_TASKS      1
#endif

/* no support to fast sched in cortex cm0 */
#if (OS_ARCH_CORTEX_M0 == 1) && (OS_FAST_SCHED == 1)
  #error "uLipeKernel: this architecture does not provide hw optimized scheduler"
#endif

/*
 *  Assert macro is particular useful for debbuging purposes, so in
 *  Debug configurations it will always defined, in release this macro
 *  is omitted.
 */
#ifdef OS_DEBUG
#define   uLipeAssert(x)  	_uLipeAssert(x)
#else
#define   uLipeAssert(x)	(void)x
#endif

#define OS_1K                       1024
#define OS_MAX_SIZED_HEAP_BLOCK     (OS_1K * 16)

#endif
