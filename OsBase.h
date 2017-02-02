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


/*
 *  custom rtos types:
 */

typedef uint32_t 	OsStack_t;		//Stack pointer type normalization
typedef uint32_t *  OsStackPtr_t;	//

typedef uint32_t	OsHandler_t;	//custom handler for kernel objects.


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
static __inline void _uLipeAssert(uint32_t x)
{
	if(x == FALSE)
	{
		asm("   bkpt  #0 \n\r");

		//Trava aqui:
		while(TRUE);

	}
}

/*
 *  Assert macro is particular useful for debbuging purposes, so in
 *  Debug configurations it will always defined, in release this macro
 *  is omitted.
 */
#ifdef DEBUG
#define   uLipeAssert(x)  	_uLipeAssert(x)
#else
#define   uLipeAssert(x)
#endif

#endif
