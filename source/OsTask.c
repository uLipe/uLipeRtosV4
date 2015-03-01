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
	OsTCBPtr_t tcb_a = NULL;

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

	OS_CRITICAL_OUT();

	//Fills the new find tcb:
	taskTbl[taskPrio].delayTime = 0;
	taskTbl[taskPrio].taskPrio  = taskPrio;
	taskTbl[taskPrio].stackBot  = taskStack;

	taskStack += (OsStack_t)stkSize;

	//Initialize the stack frame:
	taskTbl[taskPrio].stackTop = uLipeStackInit(taskStack, &uLipeTaskEntry, taskArgs);
	taskTbl[taskPrio].stackSize = stkSize;
	taskTbl[taskPrio].task = task;
	taskTbl[taskPrio].taskStatus = 1 << kTaskReady;

	//Attach the tcb in linked list:
	tcbPtrTbl[taskPrio] = &taskTbl[taskPrio];

	tcb_a = tcbPtrTbl[OS_LEAST_PRIO];

	//check if we are trying to install the idle task:
	if(taskPrio == OS_LEAST_PRIO)
	{
		tcb_a->nextTCB = NULL;
		tcb_a->prevTCB = NULL;
	}
	else
	{
		OS_CRITICAL_IN();
		while(tcb_a->nextTCB != NULL)
		{
			tcb_a = (OsTCBPtr_t)tcb_a->nextTCB;
		}

		//attach tcb:
		tcb_a->nextTCB = tcbPtrTbl[taskPrio];
		tcbPtrTbl[taskPrio]->prevTCB = (OsTCBPtr_t)tcb_a;
	}

	//all ready, lets make this task ready to run:
	uLipePrioSet(taskPrio, &taskPrioList);

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
	OsTCBPtr_t tcb_a = NULL;

	//check arguments:
	if(taskTbl[taskPrio].tcbTaken == FALSE) return(kInvalidParam); //Task already deleted

	OS_CRITICAL_IN();

	//Remove task from ready list first:
	uLipePrioClr(taskPrio, &taskPrioList);
	taskTbl[taskPrio].taskStatus &= ~(1 << kTaskReady);

	//Detach from tcb linked list:
	tcb_a = &taskTbl[taskPrio];
	tcb_a =(OsTCBPtr_t) tcb_a->prevTCB;
	tcb_a->nextTCB = taskTbl[taskPrio].nextTCB;
	taskTbl[taskPrio].prevTCB = (OsTCBPtr_t)tcb_a;

	OS_CRITICAL_OUT();

	//Fill out the tcb:
	tcbPtrTbl[taskPrio] = NULL;
	taskTbl[taskPrio].stackBot =  NULL;
	taskTbl[taskPrio].stackTop =  NULL;
	taskTbl[taskPrio].stackSize = 0;

	//Free this tcb:
	OS_CRITICAL_IN();
	taskTbl[taskPrio].tcbTaken = FALSE;
	OS_CRITICAL_OUT();

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
	tcbPtrTbl[taskPrio]->taskStatus &= ~(1 << kTaskReady);
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
	if((taskTbl[taskPrio].taskStatus & (1 << kTaskReady)))return(kTaskReady);	    //ready tasks already is resumed

	OS_CRITICAL_IN();

	//Pute the suspended task in ready list:
	tcbPtrTbl[taskPrio]->taskStatus |=  (1 << kTaskReady);
	tcbPtrTbl[taskPrio]->taskStatus &=  ~(1 << kTaskSuspend);
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

	OS_CRITICAL_IN();

	//Remove the current task from the ready list:
	uLipePrioClr(currentTask->taskPrio, &taskPrioList);
	currentTask->taskStatus &= ~(1 << kTaskReady);
	currentTask->taskStatus |=  (1 << kTaskPendDelay);

	//Put the delay value:
	currentTask->delayTime = ticks;

	OS_CRITICAL_OUT();

	//Task suspended, check the ready list, find a new task:
	uLipeKernelTaskYield();

	return(kStatusOk);
}
