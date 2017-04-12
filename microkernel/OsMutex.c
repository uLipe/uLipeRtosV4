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

/*
 * uLipeMutexCreate()
 */
OsHandler_t uLipeMutexCreate(OsStatus_t *err)
{
	MutexPtr_t m = uLipeMemAlloc(sizeof(Mutex_t));

	//check if we have available mutex:
	if(m == NULL)
	{
	    if(err != NULL) *err = kOutOfMutex;
		return((OsHandler_t)m);
	}
	m->mutexOwner = 0;
	m->mutexTaken = FALSE;
	m->tasksPending.prioGrp = 0;


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
		currentTask->taskStatus |= (1 << kTaskPendMtx);
		currentTask->mtxBmp = &m->tasksPending;

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
    mutexTCB->taskStatus &= ~( 1 << kTaskPendMtx);
    if(mutexTCB->taskStatus == 0)
    {
        uLipePrioSet(OS_MUTEX_PRIO, &taskPrioList);
    }

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
	mutexTCB->taskStatus &= ~( 1 << kTaskPendMtx);

	//swap priorities:
	tcbPtrTbl[m->mutexOwner]->taskPrio = m->mutexOwner;

	//remove the current owner of mutex pending list:
	uLipePrioClr(m->mutexOwner, &m->tasksPending);


	//Check if have items on wait list:
	if(m->tasksPending.prioGrp != 0)
	{

	    //so, take the new owner of mutex:
		m->mutexOwner = uLipeKernelFindHighPrio(&m->tasksPending);

		//change priority
		mutexTCB = tcbPtrTbl[m->mutexOwner];
		mutexTCB->taskPrio = OS_MUTEX_PRIO;

		//Make mutex task ready:
	    mutexTCB->taskStatus &= ~( 1 << kTaskPendMtx);
	    if(mutexTCB->taskStatus == 0)
	    {
	        uLipePrioSet(OS_MUTEX_PRIO, &taskPrioList);
	    }


		OS_CRITICAL_OUT();

		//Check for a context switch
		uLipeKernelTaskYield();


		//all gone well:
		return(kStatusOk);
	}


	//If no tasks pending, so release mutex:
	m->mutexTaken = FALSE;
	m->mutexOwner = OS_MUTEX_PRIO;

    OS_CRITICAL_OUT();


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

	uLipeMemFree(m);
	//Destroy reference for this control block:
	h = NULL;
    OS_CRITICAL_OUT();


	//all gone well:
	return(kStatusOk);
}

#endif
