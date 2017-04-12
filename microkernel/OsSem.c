/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsSem.c
 *
 *  \brief this file contains the data structures and routines
 *  for count semaphore management
 *
 *	In this file the user will find the data structures, and function
 *	implementation to create and manage counting semaphores objects
 *
 *  Author: FSN
 */

#include "uLipeRtos4.h"

/*
 * custom internal semaphore pend codes:
 */
#define OS_SEM_PEND 0x01		//Task is waiting for a semaphore
#define OS_SEM_NOT  0x80		//task is not waiting for a semaphore


#if OS_SEM_MODULE_EN > 0
/*
 * External modules variables:
 */

extern OsTCBPtr_t currentTask;
extern OsTCBPtr_t tcbPtrTbl[];
extern OsPrioList_t taskPrioList;
extern OsDualPrioList_t timerPendingList;

/*
 * Module implementation:
 */

/*
 * SemPostLoop()
 *
 * Internal function, used to process tasks which waits for semaphore
 * when its assert.
 */
inline static void SemPostLoop(OsHandler_t h)
{
	SemPtr_t s = (SemPtr_t)h;
	uint32_t i;

	//Extract the highest task which wait a semaphore:
	i = uLipeKernelFindHighPrio(&s->tasksWaiting);
	if(i != 0)
	{
		uLipePrioClr(i, &s->tasksWaiting);

        //make this task ready:
        tcbPtrTbl[i]->taskStatus &= ~((1 << kTaskPendSem)|(1 << kTaskPendDelay));
        if(tcbPtrTbl[i]->taskStatus == 0)
        {
            uLipePrioSet(i, &taskPrioList);
        }
	}

}

/*
 * SemDeleteLoop()
 *
 * Internal function, used to signal all tasks which the sem object will
 * be deleted.
 */
inline static void SemDeleteLoop(OsHandler_t h)
{
	SemPtr_t s = (SemPtr_t)h;
	uint32_t i;

	//Extract all tasks from list and signal its as ready tasks:
	do
	{
		i = uLipeKernelFindHighPrio(&s->tasksWaiting);
		if(i != 0)
		{
			uLipePrioClr(i, &s->tasksWaiting);
			//Make this task ready, and add it to ready list:
	        tcbPtrTbl[i]->taskStatus &= ~((1 << kTaskPendSem)|(1 << kTaskPendDelay));
	        if(tcbPtrTbl[i]->taskStatus == 0)
	        {
	            uLipePrioSet(i, &taskPrioList);
	        }
		}

	}while( i != 0);

}
/*
 * uLipeSemCreate()
 */
OsHandler_t uLipeSemCreate(uint16_t initCount, uint16_t limitCount,OsStatus_t *err)
{
	SemPtr_t s = uLipeMemAlloc(sizeof(Sem_t));

	//Check for semaphore available:
	if(s == NULL)
	{
	    if(err != NULL) *err = kOutOfSem;
		return((OsHandler_t)s);
	}

	//Initalize this block:
	s->semCount = initCount;
	s->semLimit = limitCount;

	//All gone well:
	if(err != NULL) *err = kStatusOk;
	return((OsHandler_t)s);
}

/*
 * uLipeSemTake()
 */
OsStatus_t uLipeSemTake(OsHandler_t h, uint16_t timeout)
{
	SemPtr_t s = (SemPtr_t)h;
	uint32_t sReg = 0;

	//Check arguments:
	if( h == 0)
	{
		return(kInvalidParam);
	}

	//Argument valid, proceed:
	OS_CRITICAL_IN();

	if(s->semCount == 0)
	{
		//No semaphore key available, so...

		OS_CRITICAL_OUT();

        currentTask->semBmp = &s->tasksWaiting;
		uLipePrioSet(currentTask->taskPrio, &s->tasksWaiting);

		OS_CRITICAL_IN();

		//...suspend and add this task in wait list:
		uLipePrioClr(currentTask->taskPrio, &taskPrioList);
		//Add timeout amount:
        currentTask->taskStatus |= (1 << kTaskPendSem);
        if(timeout != 0)
        {
            currentTask->taskStatus |= (1 << kTaskPendDelay);
            currentTask->delayTime = timeout;
            uLipePrioSet(currentTask->taskPrio, &timerPendingList.list[timerPendingList.activeList]);
        }

		OS_CRITICAL_OUT();

		//Check for a context switch:
		uLipeKernelTaskYield();

		OS_CRITICAL_IN();
		//check if count are available
		if(s->semCount > 0)
		{
		    s->semCount--;
	        OS_CRITICAL_OUT();

		    return(kStatusOk);
		}
		else
		{
	        OS_CRITICAL_OUT();
            return(kTimeout);
		}


	}


	//If keys are available, so take the semaphore:
	s->semCount--;
	OS_CRITICAL_OUT();

	//a ctx swt is not needed here
	return(kStatusOk);
}

/*
 * uLipeSemGive()
 */
OsStatus_t uLipeSemGive(OsHandler_t h, uint16_t count)
{
	SemPtr_t s = (SemPtr_t)h;
	uint32_t sReg = 0;
	uint32_t total = 0;

	//check arguments:
	if(h == 0)
	{
		return(kInvalidParam);
	}

	//Arguments valid, proceed:
	OS_CRITICAL_IN();

	total = s->semCount + count;
	if(total > s->semLimit)
	{
		s->semCount = s->semLimit;
	}
	else
	{
		s->semCount += count;
	}

	//Semaphore updated, process wait list:
	SemPostLoop(h);

	OS_CRITICAL_OUT();

	//Check for context switch:
	uLipeKernelTaskYield();


	return(kStatusOk);
}

/*
 * uLipeSemDelete()
 */
OsStatus_t uLipeSemDelete(OsHandler_t *h)
{
	SemPtr_t s = (SemPtr_t)h;
	uint32_t sReg = 0;

	//Check arguments:
	if( h == (OsHandler_t *)NULL)
	{
		return(kInvalidParam);
	}


	//Argument valid, proceed:
	OS_CRITICAL_IN();

	//Signal tasks which this sem will be deleted:
	SemDeleteLoop(*h);
	uLipeMemFree(s);

	OS_CRITICAL_OUT();

	h = (OsHandler_t *)NULL;

	//check for a context switch:
	uLipeKernelTaskYield();

	return(kStatusOk);
}

#endif
