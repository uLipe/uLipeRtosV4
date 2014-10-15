/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsTinyQueue.h
 *
 *  \brief uLipe Rtos queueing management routines file
 *
 *	In this file definitions about queueing management is included
 *  such control blocks, interface to routines and useful macros.
 *
 *  Author: FSN
 *
 */
#include "uLipeRtos4.h"

/*
 * 	Module constants
 */
#define OS_Q_PENDING_FULL   0x01
#define OS_Q_PENDING_EMPTY  0x02
#define OS_Q_PENDING_NOT    0x03

/*
 *	Modules variables
 */

#if OS_QUEUE_COUNT > 0

OsQueue_t queueBlockTbl[OS_QUEUE_COUNT];
uint32_t queueCount;

/*
 * External variables used here
 */

extern OsTCBPtr_t currentTask;
extern OsTCBPtr_t tcbPtrTbl[OS_NUMBER_OF_TASKS ];
extern OsPrioList_t taskPrioList;

#endif

/*
 *	Queue module implementation
 */

/*
 * OsQueueEmptyLoop()
 *
 * Internal function, this loop process a registry remotion for all tasks pending
 */
void OsQueueEmptyLoop(OsQueuePtr_t queue)
{
#if OS_QUEUE_COUNT > 0	
	uint32_t i;

	for (i = 0; i < OS_NUMBER_OF_TASKS ; i++)
	{
		if (queue->tasksPending[i] == OS_Q_PENDING_FULL)
		{
			//task is waited for a registry remotion, then clear this condition
			queue->tasksPending[i] = OS_Q_PENDING_NOT;

			//make this task ready
			tcbPtrTbl[i]->taskStatus &= ~(1 << kTaskPendQueue);
			tcbPtrTbl[i]->taskStatus |= (1 << kTaskReady);
			OsPrioSet(i, &taskPrioList);
		}
	}
#endif
}

/*
 * OsQueueFullLoop()
 *
 * Internal function, this loop process a registry insertion of a empty queue for all tasks pending
 */
void OsQueueFullLoop(OsQueuePtr_t queue)
{
#if OS_QUEUE_COUNT > 0	
	uint32_t i;

	for (i = 0; i < OS_NUMBER_OF_TASKS ; i++)
	{
		if (queue->tasksPending[i] == OS_Q_PENDING_EMPTY)
		{
			//This task waited for a registry remotion until here, clear this condition
			queue->tasksPending[i] = OS_Q_PENDING_NOT;

			//make this task as ready:
			tcbPtrTbl[i]->taskStatus &= ~(1 << kTaskPendQueue);
			tcbPtrTbl[i]->taskStatus |= (1 << kTaskReady);
			OsPrioSet(i, &taskPrioList);
		}
	}
#endif	 
}

/*
 * OsQueueDeleteLoop()
 *
 * Internal function, this loop process a queue delete for all tasks is waiting a event
 * related with this queue
 */
void OsQueueDeleteLoop(OsQueuePtr_t queue)
{
#if OS_QUEUE_COUNT > 0	
	uint32_t i;

	for (i = 0; i < OS_NUMBER_OF_TASKS ; i++)
	{
		//all tasks here, have its event pending condition cleared.
		queue->tasksPending[i] = OS_Q_PENDING_NOT;

		//make this task as ready
		tcbPtrTbl[i]->taskStatus &= ~(1 << kTaskPendQueue);
		tcbPtrTbl[i]->taskStatus |= (1 << kTaskReady);
		OsPrioSet(i, &taskPrioList);
	}
#endif	 
}

/*
 *	OsQueueInit()
 */
void OsQueueInit(void)
{
#if OS_QUEUE_COUNT > 0	 

	uint32_t i, j;

	queueCount = 0;

	//Put all kernel objects in a known inital state.
	for (i = 0; i < OS_QUEUE_COUNT; i++)
	{

		queueBlockTbl[i].queueData = NULL;
		queueBlockTbl[i].queueDataSize = 0;
		queueBlockTbl[i].queueEntries = 0;
		queueBlockTbl[i].queueCurrentFree = 0;
		queueBlockTbl[i].queueCurrentFront = 0;
		queueBlockTbl[i].queueCurrentEntries = 0;
		queueBlockTbl[i].queueTaken = FALSE;

		//This queue have not any tasks pending initially
		for (j = 0; j < OS_NUMBER_OF_TASKS ; j++)
		{
			queueBlockTbl[i].tasksPending[j] = OS_Q_PENDING_NOT;
		}
	}
#endif		  
}

/*
 *	OsQueueCreate()
 */
OsHandler_t OsQueueCreate(OsQueueData_t *dataPtr, uint32_t dataSize,
							uint32_t queueSize, uint8_t *err)
{
#if OS_QUEUE_COUNT > 0

	uint32_t sReg;
	uint32_t i;

	//check arguments.
	if (dataPtr == NULL)
	{
		*err = kInvalidParam;
		return (NULL );
	}
	if (dataSize == 0)
	{
		*err = kInvalidParam;
		return (NULL );
	}
	if (queueSize == 0)
	{
		*err = kInvalidParam;
		return (NULL );
	}

	//check for queue free blocks:
	OS_CRITICAL_IN();

	if (queueCount >= OS_QUEUE_COUNT)
	{
		OS_CRITICAL_OUT();
		
		*err = kOutOfQueue;
		return (NULL);
	}

	//if so, finds the first free queue:
	for (i = 0; i < OS_QUEUE_COUNT; i++)
	{
		if (queueBlockTbl[i].queueTaken != TRUE)
		{
			//achou livre
			queueBlockTbl[i].queueTaken = TRUE;
			queueCount++;
			break;
		}
	}

	OS_CRITICAL_OUT();

	//Fill the queue control block
	queueBlockTbl[i].queueData = dataPtr;
	queueBlockTbl[i].queueDataSize = dataSize;
	queueBlockTbl[i].queueEntries = queueSize;
	//Primeiro bloco livre da queue é sempre o indice 0
	queueBlockTbl[i].queueCurrentFree = 0;

	return ((OsHandler_t) &queueBlockTbl[i]);

#endif	 
}

/*
 *	OsQueuePend()
 */
OsStatus_t OsQueuePend(OsHandler_t handler, uint8_t *dataPtr,
						uint32_t *sizeInBytes, uint8_t opt)
{
#if OS_QUEUE_COUNT > 0	

	uint32_t i,tmp;
	uint32_t sReg;
	uint32_t queueBaseIndex;

	OsQueuePtr_t queue = (OsQueuePtr_t)handler;


	//Check parameters
	if (queue == NULL)
		return (kInvalidParam);
	if (dataPtr == NULL)
		return (kInvalidParam);
	if ((opt != OS_Q_PEND_BLOCK_EMPTY) && (opt != OS_Q_NOT_BLOCK))
		return (kInvalidParam);

	OS_CRITICAL_IN();

	//check for queue fifo empty
	if (queue->queueCurrentEntries == 0)
	{

		if (opt != OS_Q_NOT_BLOCK)
		{
			//we want to block this task

			//signal the queue pend type for this task:
			queue->tasksPending[currentTask->taskPrio] = OS_Q_PENDING_EMPTY;

			//suspends current Task execution
			OsPrioClr(currentTask->taskPrio, &taskPrioList);
			currentTask->taskStatus &= ~(1 << kTaskReady);
			currentTask->taskStatus |= (1 << kTaskPendQueue);

			OS_CRITICAL_OUT();

			//Find a new task ready to run
			uLipeKernelTaskYield();

			return (kStatusOk);
		}
		else
		{
			//The queue is empty but we not desire to block task,
			//so only return the status

			OS_CRITICAL_OUT();
			return (kQueueEmpty);
		}
	}



	//compute the base address to acess in queueu fifo:
	queueBaseIndex = ((queue->queueCurrentFront) * (queue->queueDataSize + 1));
	queue->queueCurrentFront++;
	if(queue->queueCurrentFront > queue->queueEntries)
	{
		//since we have free slots the queue beahves in circular addressing fashion
		queue->queueCurrentFront = 0;
	}
	queue->queueCurrentEntries--;

	OS_CRITICAL_OUT();

	//The first byte always contain the data quantity
	tmp = queue->queueData[queueBaseIndex];
	queueBaseIndex++;

	//Copy the bulk of data:
	for (i = 0; i < tmp; i++)
	{
		*dataPtr++ = queue->queueData[queueBaseIndex++];
	}

	if(sizeInBytes != NULL)
	{
		*sizeInBytes = tmp;		
	}
	
	//Process the action queue thru tasks:
	OS_CRITICAL_IN();
	OsQueueEmptyLoop(queue);
	OS_CRITICAL_OUT();

	uLipeKernelTaskYield();


#endif	
		
	return (kStatusOk);
}

/*
 *	OsQueuePost()
 */
OsStatus_t OsQueuePost(OsHandler_t handler, uint8_t *dataPtr,
						uint32_t sizeInBytes, uint8_t opt)
{
#if OS_QUEUE_COUNT > 0	 

	uint32_t i;
	uint32_t sReg;
	uint32_t queueBaseIndex;

	OsQueuePtr_t queue = (OsQueuePtr_t)handler;

	//check parameters:
	if (queue == NULL)
		return (kInvalidParam);
	if (dataPtr == NULL)
		return (kInvalidParam);
	if ((opt != OS_Q_POST_BLOCK_FULL) && (opt != OS_Q_NOT_BLOCK))
		return (kInvalidParam);
	if (sizeInBytes > queue->queueDataSize)
		return (kQueueTooManyData);

	OS_CRITICAL_IN();

	//check for free space:
	if (queue->queueCurrentEntries == queue->queueEntries)
	{
		if (opt != OS_Q_NOT_BLOCK)
		{
			//We want to block task, until queue stays not full

			//Suspends current task execution
			OsPrioClr(currentTask->taskPrio, &taskPrioList);
			currentTask->taskStatus &= ~(1 << kTaskReady);
			currentTask->taskStatus |=  (1 << kTaskPendQueue);
			queue->tasksPending[currentTask->taskPrio] = OS_Q_PENDING_FULL;

			OS_CRITICAL_OUT();

			//find a new task ready to run.
			uLipeKernelTaskYield();

			return (kStatusOk);
		}
		else
		{
			//We not desire to block task, so only returns.

			OS_CRITICAL_OUT();
			return (kQueueFull);
		}
	}

	//Compute the base address on queue fifo:
	queueBaseIndex = (queue->queueCurrentFree * (queue->queueDataSize + 1));
	queue->queueCurrentFree++;
	if(queue->queueCurrentFree == queue->queueEntries)
	{

		if(queue->queueCurrentEntries < queue->queueEntries)
		{
			//Since we have free slots, queue behaves as a circular fifo fashion
			queue->queueCurrentFree = 0;
		}
	}
	queue->queueCurrentEntries++;

	OS_CRITICAL_OUT();

	//Store the amount of data:
	queue->queueData[queueBaseIndex] = sizeInBytes;
	queueBaseIndex++;

	//copy the bulk data:
	for (i = 0; i < sizeInBytes; i++)
	{
		queue->queueData[queueBaseIndex++] = *dataPtr++;
	}


	//Processe the insertion action in queue thru pending tasks:
	OS_CRITICAL_IN();
	OsQueueFullLoop(queue);
	OS_CRITICAL_OUT();


	uLipeKernelTaskYield();


#endif	

	return (kStatusOk);
}

/*
 *	OsQueueGetStatus()
 */
OsStatus_t OsQueueGetStatus(OsHandler_t handler, uint32_t *freeEntries)
{
#if OS_QUEUE_COUNT > 0	 	
	uint32_t sReg;
	OsQueuePtr_t queue = (OsQueuePtr_t)handler;
	
	//check parameters:
	if(queue == NULL)return(kInvalidParam);
	
	OS_CRITICAL_IN();

	//Check queue status for:

	if(queue->queueCurrentEntries == queue->queueEntries)
	{
		//Full queue:
		
		OS_CRITICAL_OUT();
		
		*freeEntries = 0;
		return(kQueueFull);
	}
	

	if(queue->queueCurrentEntries == 0)
	{
		//Empty queue:
		
		OS_CRITICAL_OUT();
		
		*freeEntries = queue->queueEntries;
		return(kQueueEmpty);
	}
	
	//Or the current free entries:
	*freeEntries = (queue->queueEntries - queue->queueCurrentEntries);
	OS_CRITICAL_OUT();

#endif	
	return(kStatusOk);
}


/*
 *	OsQueueDelete()
 */
OsStatus_t OsQueueDelete(OsHandler_t handler)
{
#if OS_QUEUE_COUNT > 0	
	uint32_t sReg;
	OsQueuePtr_t queue = (OsQueuePtr_t)handler;

	//check parameters:
	if (queue == NULL)
		return (kInvalidParam);


	//First, make all tasks that pending this queue event as ready:
	OS_CRITICAL_IN();
	OsQueueDeleteLoop(queue);

	//Release this queue cb:
	queue->queueTaken = FALSE;
	queueCount--;

	queue->queueData = NULL;

	queue = NULL;
	OS_CRITICAL_OUT();

	uLipeKernelTaskYield();

#endif	

	return (kStatusOk);
}
