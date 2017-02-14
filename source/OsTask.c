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

OsTCB_t 	taskTbl[OS_NUMBER_OF_TASKS];  		//Reserve space for user tcbs
OsTCBPtr_t  tcbPtrTbl[OS_NUMBER_OF_TASKS];		//Array of tcb pointers to external access
uint16_t tasksCount;							//count of installed tasks.

/*
 * External variables
 */
extern OsPrioList_t taskPrioList;
extern OsTCBPtr_t   currentTask;
extern OsTCBPtr_t delayedTcbs;


/*
 * Module implementation:
 */

/*
 * 	ulipeTaskInit()
 */
OsStatus_t uLipeTaskInit(void)
{
	uint8_t i;
	uint8_t err = kStatusOk;

	//clears all task kernel obj:
	for( i = 0; i < OS_NUMBER_OF_TASKS; i++)
	{
		err = uLipeKernelObjSet((uint8_t*)&taskTbl[i], 0, sizeof(OsTCB_t));
		uLipeAssert(err == kStatusOk);

		err = uLipeKernelObjSet((uint8_t*)&tcbPtrTbl[i], 0 , sizeof(OsTCBPtr_t));
		uLipeAssert(err == kStatusOk);

		//Free all blocks too:
		taskTbl[i].tcbTaken = FALSE;

	}

	return((OsStatus_t)err);
}

/*
 * 	ulipeTaskCreate()
 */
OsStatus_t uLipeTaskCreate(void (*task) (void * args), OsStackPtr_t taskStack, uint32_t stkSize,
						   uint16_t taskPrio, void *taskArgs)
{

	extern void uLipeTaskEntry(void *);
	uint32_t sReg = 0;

	//Check arguments:
	if(task == NULL) return(kInvalidParam);
	if(taskStack == NULL)return(kInvalidParam);
	if(taskPrio > (OS_NUMBER_OF_TASKS - 1)) return(kInvalidParam);

	//check for freeblocks:
	OS_CRITICAL_IN();

	if(tasksCount >= OS_NUMBER_OF_TASKS)
	{
		OS_CRITICAL_OUT();
		return(kOutOfTasks);
	}

	//Check if prio was used:
	if(taskTbl[taskPrio].tcbTaken != FALSE)
	{
		OS_CRITICAL_OUT();
		return(kInvalidParam);
	}

	//add a new task as used:
	tasksCount++;


	//Take this tcb
	taskTbl[taskPrio].tcbTaken = TRUE;
	taskTbl[taskPrio].taskPrio  = taskPrio;

	//Initialize the stack frame:
	taskTbl[taskPrio].stackTop = uLipeStackInit(taskStack + stkSize, &uLipeTaskEntry, taskArgs);
	taskTbl[taskPrio].task = task;
	taskTbl[taskPrio].taskStatus = 0;

	//Attach the tcb in linked list:
	tcbPtrTbl[taskPrio] = &taskTbl[taskPrio];

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

	//check arguments:
	if(taskTbl[taskPrio].tcbTaken == FALSE) return(kInvalidParam); //Task already deleted

	OS_CRITICAL_IN();

	//Remove task from ready list first:
	uLipePrioClr(taskPrio, &taskPrioList);
	taskTbl[taskPrio].taskStatus |= (1 << kTaskDeleted);

	//Fill out the tcb:
	tcbPtrTbl[taskPrio] = NULL;
	taskTbl[taskPrio].stackTop =  NULL;

	//Free this tcb:
	taskTbl[taskPrio].tcbTaken = FALSE;
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
	if(taskTbl[taskPrio].tcbTaken == FALSE) return(kInvalidParam);					//Task is deleted cant suspend
	if(!(taskTbl[taskPrio].taskStatus & (1 << kTaskReady)))return(kCantSuspend);	//Only ready tasks can suspended

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
	if(taskTbl[taskPrio].tcbTaken == FALSE) return(kInvalidParam);					//Task is deleted cant suspend

	OS_CRITICAL_IN();

	//Pute the suspended task in ready list:
	tcbPtrTbl[taskPrio]->taskStatus &=  ~(1 << kTaskSuspend);
    if(tcbPtrTbl[taskPrio]->taskStatus == 0)
    {
        uLipePrioSet(taskPrio, &taskPrioList);
    }

	uLipePrioSet(taskPrio, &taskPrioList);

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

	    // prepend this task on pendable delay list
	    if(delayedTcbs == NULL) {
	        delayedTcbs = currentTask;
	    } else {
	        currentTask->next = delayedTcbs;
	        delayedTcbs->prev = currentTask;
	        delayedTcbs = currentTask;
	    }

	    OS_CRITICAL_OUT();

	    //Task suspended, check the ready list, find a new task:
	    uLipeKernelTaskYield();

	}

	return(kStatusOk);
}
