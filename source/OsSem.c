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
 * Module variables
 */

static SemPtr_t semFree;		//points to currente free sem object
Sem_t semTbl[OS_SEM_COUNT];		//semaphore objects list

/*
 * External modules variables:
 */

extern OsTCBPtr_t currentTask;
extern OsTCBPtr_t tcbPtrTbl[];
extern OsPrioList_t taskPrioList;

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

#if OS_USE_DEPRECATED == 1
	//update all tasks in wait list
	for( i = 0; i < OS_NUMBER_OF_TASKS; i++)
	{
		if(s->tasksPending[i] != OS_SEM_NOT)
		{
			s->tasksPending[i] = OS_SEM_NOT;

			//Make this task ready, and add it to ready list:
			tcbPtrTbl[i]->taskStatus = (1 << kTaskReady);
			uLipePrioSet(i, &taskPrioList);
		}

	}
#else
	//Extract the highest task which wait a semaphore:
	i = uLipeKernelFindHighPrio(&s->tasksWaiting);
	if(i != 0)
	{
		uLipePrioClr(i, &s->tasksWaiting);

		//Make this task ready, and add it to ready list:
		tcbPtrTbl[i]->taskStatus = (1 << kTaskReady);
		uLipePrioSet(i, &taskPrioList);
	}
#endif
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

#if OS_USE_DEPRECATED == 1
	//update all tasks in wait list
	for( i = 0; i < OS_NUMBER_OF_TASKS; i++)
	{
		if(s->tasksPending[i] != OS_SEM_NOT)
		{
			//Make this task ready, and add it to ready list:
			tcbPtrTbl[i]->taskStatus = (1 << kTaskReady);
			uLipePrioSet(i, &taskPrioList);
		}
		s->tasksPending[i] = OS_SEM_NOT;
	}
#else
	//Extract all tasks from list and signal its as ready tasks:
	do
	{
		i = uLipeKernelFindHighPrio(&s->tasksWaiting);
		if(i != 0)
		{
			uLipePrioClr(i, &s->tasksWaiting);
			//Make this task ready, and add it to ready list:
			tcbPtrTbl[i]->taskStatus = (1 << kTaskReady);
			uLipePrioSet(i, &taskPrioList);
		}

	}while( i != 0);
#endif
}

/*
 * uLipeSemInit()
 */
void uLipeSemInit(void)
{
	uint32_t i,j;

	//Init the semFree pointer
	semFree = &semTbl[0];

	for( i = 0; i < OS_SEM_COUNT; i++)
	{
		//Link all semaphores in linked list:
		semTbl[i].nextNode = &semTbl[i+1];

		//Put the sem object in a inital known state:
		semTbl[i].semCount = 0;
		semTbl[i].semLimit = 0;

#if OS_USE_DEPRECATED == 1

		for( j = 0 ; j < OS_NUMBER_OF_TASKS; j++)
		{
			semTbl[i].tasksPending[j] = OS_SEM_NOT;
		}
#else

		memset(&semTbl[i].tasksWaiting, 0, sizeof(OsPrioList_t));
		(void)j;

#endif
	}
}

/*
 * uLipeSemCreate()
 */
OsHandler_t uLipeSemCreate(uint16_t initCount, uint16_t limitCount,OsStatus_t *err)
{
	SemPtr_t s = NULL;
	uint32_t sReg = 0;

	//Check for semaphore available:
	if(semFree == NULL)
	{
	    if(err != NULL) *err = kOutOfSem;
		return((OsHandler_t)s);
	}


	//So, we have semFree items, take one:
	OS_CRITICAL_IN();
	s = semFree;

	//update new free item:
	semFree = semFree->nextNode;
	OS_CRITICAL_OUT();

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

#if OS_USE_DEPRECATED == 1
		s->tasksPending[currentTask->taskPrio] = OS_SEM_PEND;
#else
		uLipePrioSet(currentTask->taskPrio, &s->tasksWaiting);
#endif
		OS_CRITICAL_IN();

		//...suspend and add this task in wait list:
		uLipePrioClr(currentTask->taskPrio, &taskPrioList);
		//Add timeout amount:
		currentTask->delayTime = timeout;
		currentTask->taskStatus = (1 << kTaskPendDelay) | (1 << kTaskPendSem);

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

	//Add this sem object to semFree:
	s->nextNode = semFree;
	semFree = s;

	OS_CRITICAL_OUT();

	h = (OsHandler_t *)NULL;
	s = NULL;

	//check for a context switch:
	uLipeKernelTaskYield();

	return(kStatusOk);
}

#endif
