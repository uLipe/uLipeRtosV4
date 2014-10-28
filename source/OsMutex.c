/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsMutex.c
 *
 *  \brief this file contains the the modules for mutual
 *  exclusion semaphore management
 *
 *	In this file the user will find the implementation of the routines
 *	to manage mutex kernel objects.
 *
 *  Author: FSN
 *
 */

#include "uLipeRtos4.h"


/*
 * Mutex custom codes
 */

#define OS_MTX_FREE 0x01
#define OS_MTX_BUSY 0x02

/*
 * Module internal variables
 */
static MutexPtr_t freeList;			//Pointer to next free mutex block
Mutex_t mutexTbl[OS_MTX_COUNT];		//table of mutex control block

/*
 * External module variables
 */
extern OsTCBPtr_t currentTask;
extern OsTCBPtr_t tcbPtrTbl[];
extern OsPrioList_t taskPrioList;

/*
 * Module implementation:
 */

/*
 * MutexDeleteLoop
 */


