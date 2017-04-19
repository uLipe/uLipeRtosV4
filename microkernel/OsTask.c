/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsTask.c
 *
 *  \brief uLipe Rtos task management routines
 *
 *	In this file definitions about task kernel objects and management
 *	routines are provided, the inclusion of this file is made automatically
 *	including the main header file in user application
 *
 *
 *  Author: FSN
 *
 */

#include "uLipeRtos4.h"

/*
 * Task constants:
 */

/*
 * Module variables:
 */

OsTCBPtr_t  tcbPtrTbl[OS_NUMBER_OF_TASKS]= {0};		//Array of tcb pointers to external access
uint16_t tasksCount={0};							//count of installed tasks.

/*
 * External variables
 */
extern OsPrioList_t taskPrioList;
extern OsTCBPtr_t   currentTask;
extern OsDualPrioList_t timerPendingList;

/*
 * Module implementation:
 */


/*
 * 	ulipeTaskCreate()
 */
OsStatus_t uLipeTaskCreate(void (*task) (void * args), uint32_t stackSize,
                           uint16_t taskPrio, void *taskArgs)
{

	extern void uLipeTaskEntry(void *);
	uint32_t sReg = 0;
	OsTCBPtr_t tcb = uLipeMemAlloc(sizeof(OsTCB_t));
	OsStackPtr_t sp = uLipeMemAlloc(sizeof(uint32_t) * stackSize);

	//Check arguments:
	if(task == NULL) return(kInvalidParam);
	if(taskPrio > (OS_NUMBER_OF_TASKS - 1)) return(kInvalidParam);

	//check for freeblocks:
	OS_CRITICAL_IN();
	if(tcb == NULL)
	{
        OS_CRITICAL_OUT();
        return(kOutOfTasks);
	}

	if(sp == NULL)
	{
        OS_CRITICAL_OUT();
        uLipeMemFree(tcb);
        return(kOutOfMem);
	}

	if(tasksCount >= OS_NUMBER_OF_TASKS)
	{
		OS_CRITICAL_OUT();
		return(kOutOfTasks);
	}

	//Check if prio was used:
	if(tcbPtrTbl[taskPrio] != NULL)
	{
		OS_CRITICAL_OUT();
        uLipeMemFree(tcb);
        uLipeMemFree(sp);
		return(kInvalidParam);
	}


	//add a new task as used:
	tasksCount++;


	//Take this tcb
	tcb->taskPrio  = taskPrio;
	//Initialize the stack frame:
	tcb->stackTop = uLipeStackInit(sp + stackSize, &uLipeTaskEntry, taskArgs);
	tcb->task = task;
	tcb->taskStatus = 0;


	//Attach the tcb in linked list:
	tcbPtrTbl[taskPrio] = tcb;

	//all ready, lets make this task ready to run:
	uLipePrioSet(taskPrio, &taskPrioList);

    OS_CRITICAL_OUT();

	//check for a context switching:
	uLipeKernelTaskYield();

	return(kStatusOk);
}

/*
 * 	ulipeTaskDelete()
 */
OsStatus_t uLipeTaskDelete( uint16_t taskPrio)
{
	uint32_t sReg = 0;
	OsTCBPtr_t tcb;

	//check arguments:
	if(tcbPtrTbl[taskPrio] == NULL) return(kInvalidParam); //Task already deleted

	OS_CRITICAL_IN();
	tcb = tcbPtrTbl[taskPrio];
	tcbPtrTbl[taskPrio] = NULL;
	//Remove task from ready list first:
	uLipePrioClr(taskPrio, &taskPrioList);
    uLipeMemFree(tcb->stackTop);
	uLipeMemFree(tcb);

	OS_CRITICAL_OUT();

    //check for a context switching:
    uLipeKernelTaskYield();

	return(kStatusOk);
}

/*
 * 	ulipeTaskSuspend()
 */
OsStatus_t uLipeTaskSuspend( uint16_t taskPrio)
{
	uint32_t sReg = 0;

	//Check arguments:
	if(tcbPtrTbl[taskPrio] == NULL) return(kInvalidParam);					//Task is deleted cant suspend
	if((tcbPtrTbl[taskPrio]->taskStatus != 0) )return(kCantSuspend);	//Only ready tasks can suspended

	OS_CRITICAL_IN();

	//First remove this task from ready list:
	uLipePrioClr(taskPrio, &taskPrioList);
	tcbPtrTbl[taskPrio]->taskStatus |=  (1 << kTaskSuspend);

	//Task suspended, then find a new task to run:
	OS_CRITICAL_OUT();

	uLipeKernelTaskYield();

	return(kStatusOk);

}

/*
 * 	ulipeTaskResume()
 */
OsStatus_t uLipeTaskResume( uint16_t taskPrio)
{
	uint32_t sReg = 0;

	//Check arguments:
	if(tcbPtrTbl[taskPrio] == NULL) return(kInvalidParam);					//Task is deleted cant suspend

	OS_CRITICAL_IN();

	//Pute the suspended task in ready list:
	tcbPtrTbl[taskPrio]->taskStatus &=  ~(1 << kTaskSuspend);
    if(tcbPtrTbl[taskPrio]->taskStatus == 0)
    {
        uLipePrioSet(taskPrio, &taskPrioList);
    }

	//Task resumed, check the ready list:
	OS_CRITICAL_OUT();

	uLipeKernelTaskYield();

	return(kStatusOk);
}

/*
 * 	ulipeTaskDelay()
 */
OsStatus_t uLipeTaskDelay( uint16_t ticks)
{
	uint32_t sReg = 0;

	if(ticks != 0)
	{
	    OS_CRITICAL_IN();

	    //Remove the current task from the ready list:
	    uLipePrioClr(currentTask->taskPrio, &taskPrioList);
	    currentTask->taskStatus |=  (1 << kTaskPendDelay);

	    //Put the delay value:
	    currentTask->delayTime = ticks;
        uLipePrioSet(currentTask->taskPrio, &timerPendingList.list[timerPendingList.activeList]);

	    OS_CRITICAL_OUT();

	    //Task suspended, check the ready list, find a new task:
	    uLipeKernelTaskYield();

	}

	return(kStatusOk);
}
