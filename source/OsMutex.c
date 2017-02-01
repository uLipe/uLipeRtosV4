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

#if OS_MTX_MODULE_EN > 0

/*
 * Module internal variables
 */
static MutexPtr_t mtxFree;			//Pointer to next free mutex block
Mutex_t mutexTbl[OS_MTX_COUNT];		//table of mutex control block

#define mutexTCB tcbPtrTbl[OS_MUTEX_PRIO]

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
 * uLipeMutexInit()
 */

void uLipeMutexInit(void)
{
	uint32_t i = 0, j = 0;

	//Initializes mtxFree
	mtxFree = &mutexTbl[0];


	for( i = 0; i < OS_MTX_COUNT; i++)
	{
		//Link the mtx control blocks:
		mutexTbl[i].nextNode = &mutexTbl[i + 1];

		//Put mutex control block in inital known state:
		mutexTbl[i].mutexOwner = OS_MUTEX_PRIO;
		mutexTbl[i].mutexTaken = FALSE;
		mutexTbl[i].tasksPending.prioGrp = 0;
		for(j = 0; j < OS_KERNEL_ENTRIES_FOR_GROUP; j++)
		{
			mutexTbl[i].tasksPending.prioTbl[j] = 0;
		}
	}

	//So, mark the end of mutex linked list:
	mutexTbl[OS_MTX_COUNT - 1].nextNode = NULL;
}

/*
 * uLipeMutexCreate()
 */
OsHandler_t uLipeMutexCreate(OsStatus_t *err)
{
	uint32_t sReg = 0;
	MutexPtr_t m = NULL;

	//check if we have available mutex:
	if(mtxFree == NULL)
	{
	    if(err != NULL) *err = kOutOfMutex;
		return((OsHandler_t)m);
	}

	//If so, take a mutex control block:
	OS_CRITICAL_IN();
	m = mtxFree;

	//update current mtxFree:
	mtxFree = mtxFree->nextNode;

	OS_CRITICAL_OUT();

	//Every mutex contro block starts fully initialized.

	if(err != NULL) *err = kStatusOk;
	return((OsHandler_t)m);

}


/*
 * uLipeMutexTake()
 */
OsStatus_t uLipeMutexTake(OsHandler_t h)
{
	uint32_t sReg = 0;
	MutexPtr_t m = (MutexPtr_t)h;

	//Check arguments:
	if(h == 0)
	{
		return(kInvalidParam);
	}

	//Argument valid, then proceed:
	OS_CRITICAL_IN();

	//check if our mutex is already busy:
	if(m->mutexTaken != FALSE)
	{
		//Add a new task to wait list:
		uLipePrioSet(currentTask->taskPrio, &m->tasksPending);

		//Suspend current task execution:
		uLipePrioClr(currentTask->taskPrio, &taskPrioList);
		currentTask->taskStatus = (1 << kTaskPendMtx);

		OS_CRITICAL_OUT();

		//Check for new task to execute:
		uLipeKernelTaskYield();

		return(kMutexOwned);
	}


	//if resource available, then give it to caller task:
	m->mutexTaken = TRUE;
	OS_CRITICAL_OUT();

	//change this task prio:
	m->mutexOwner = currentTask->taskPrio;

	OS_CRITICAL_IN();
	mutexTCB = currentTask;
	mutexTCB->taskPrio = OS_MUTEX_PRIO;

	//Make mutex prio ready to run:
	uLipePrioSet(OS_MUTEX_PRIO, &taskPrioList);
	mutexTCB->taskStatus = ( 1 << kTaskReady);

	OS_CRITICAL_OUT();

	//request a context switch
	uLipeKernelTaskYield();

	//all gone well:
	return(kStatusOk);
}


/*
 * uLipeMutexGive()
 */
OsStatus_t uLipeMutexGive(OsHandler_t h)
{
	uint32_t sReg = 0;
	MutexPtr_t m = (MutexPtr_t)h;

	//check arguments:
	if( h == 0)
	{
		return(kInvalidParam);
	}

	//Arguments valid, then proceed:
	OS_CRITICAL_IN();

	//Suspend mutex task:
	uLipePrioClr(mutexTCB->taskPrio, &taskPrioList);
	mutexTCB->taskStatus = (1 << kTaskSuspend);

	//swap priorities:
	tcbPtrTbl[m->mutexOwner]->taskPrio = m->mutexOwner;
	//remove the current owner of mutex pending list:
	uLipePrioClr(m->mutexOwner, &m->tasksPending);

	OS_CRITICAL_OUT();

	//Check if have items on wait list:
	if(m->tasksPending.prioGrp != 0)
	{
		//so, take the new owner of mutex:
		m->mutexOwner = uLipeKernelFindHighPrio(&m->tasksPending);
		OS_CRITICAL_IN();

		//change priority
		mutexTCB->taskPrio = OS_MUTEX_PRIO;

		//Make mutex task ready:
		uLipePrioSet(mutexTCB->taskPrio, &taskPrioList);
		mutexTCB->taskStatus = (1 << kTaskReady);

		OS_CRITICAL_OUT();

		//Check for a context switch
		uLipeKernelTaskYield();


		//all gone well:
		return(kStatusOk);
	}


	//If no tasks pending, so release mutex:
	m->mutexTaken = FALSE;
	m->mutexOwner = OS_MUTEX_PRIO;

	//Check for a context switch:
	uLipeKernelTaskYield();


	//all gone well here too:
	return(kStatusOk);
}

/*
 * uLipeMutexDelete()
 */
OsStatus_t uLipeMutexDelete(OsHandler_t *h)
{
	uint32_t sReg = 0;
	MutexPtr_t m = (MutexPtr_t)*h;

	//check arguments:
	if(h == NULL)
	{
		return(kInvalidParam);
	}

	//Argument valid, then proceed:
	OS_CRITICAL_IN();

	if(m->mutexTaken != FALSE)
	{
		//mutex taken, cant be deleted:
		OS_CRITICAL_OUT();
		return(kMutexOwned);
	}

	//so insert this mutex on mtxFree:
	m->nextNode = mtxFree;
	mtxFree = m;

	//Destroy reference for this control block:
	h = NULL;
	m = NULL;

	//all gone well:
	return(kStatusOk);
}

#endif
