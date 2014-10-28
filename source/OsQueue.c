/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsQueue.c
 *
 *  \brief this file contains the routines to queue managenment
 *
 *	In this file te user will find the implementation of routines
 *  to create and manage queue messages.
 *
 *  Author: FSN
 */

#include "uLipeRtos4.h"

/*
 * Custom internal queue codes:
 */

#define OS_Q_PEND_FULL  0x01	//Pending remove at least one message of queue
#define OS_Q_PEND_EMPTY 0x02 	//Pending to insert at least onde message in queue
#define OS_Q_PEND_NOT   0x80	//Not pending any event

#if OS_QUEUE_MODULE_EN > 0

/*
 * Module internal variables
 */

static QueuePtr_t freeList;			//Pointer to next freeblock of queue
Queue_t queueTbl[OS_QUEUE_COUNT];	//Table with the current block of queues

/*
 * External used variables
 */
extern OsTCBPtr_t currentTask;
extern OsTCBPtr_t tcbPtrTbl[];
extern OsPrioList_t taskPrioList;

/*
 * Implementation:
 */

/*
 * QueueRemoveLoop()
 *
 * Internal function, used to process all tasks that waiting to at least
 * one slot became free, when a slot had its data removed
 *
 */
static void QueueRemoveLoop(OsHandler_t h)
{
	QueuePtr_t q = (QueuePtr_t)h;
	uint32_t i = 0;

	for(i = 0; i < OS_NUMBER_OF_TASKS; i++)
	{
		//check for tasks waiting for slot free:
		if(q->tasksPending[i] == OS_Q_PEND_FULL)
		{
			//make this task ready:
			tcbPtrTbl[i]->taskStatus = (1 << kTaskReady);
			uLipePrioSet(i, &taskPrioList);

			//remove this task from wait list:
			q->tasksPending[i] = OS_Q_PEND_NOT;
		}
	}

}

/*
 * QueueInsertLoop()
 *
 * Internal function, used to proccess all tasks that waiting to at least
 * one slot became filled, when a slot had is space filled of data
 */
static void QueueInsertLoop(OsHandler_t h)
{
	QueuePtr_t q = (QueuePtr_t)h;
	uint32_t i = 0;

	for(i = 0; i < OS_NUMBER_OF_TASKS; i++)
	{
		//check for tasks waiting for slot free:
		if(q->tasksPending[i] == OS_Q_PEND_EMPTY)
		{
			//make this task ready:
			tcbPtrTbl[i]->taskStatus = (1 << kTaskReady);
			uLipePrioSet(i, &taskPrioList);

			//remove this task from wait list:
			q->tasksPending[i] = OS_Q_PEND_NOT;
		}
	}
}

/*
 * QueueDeleteLoop()
 *
 * Internal function, used to assert all tasks waiting for any event related
 * to queue when a kernel object is about to be destroyed
 *
 */
static void QueueDeleteLoop(OsHandler_t h)
{
	QueuePtr_t q = (QueuePtr_t)h;
	uint32_t i = 0;

	for(i = 0; i < OS_NUMBER_OF_TASKS; i++)
	{
		//check for tasks waiting for slot free:
		if(q->tasksPending[i] != OS_Q_PEND_NOT)
		{
			//make this task ready:
			tcbPtrTbl[i]->taskStatus = (1 << kTaskReady);
			uLipePrioSet(i, &taskPrioList);
		}

		//remove this task from wait list:
		q->tasksPending[i] = OS_Q_PEND_NOT;

	}

}
/*
 * uLipeQueueInit()
 */
void uLipeQueueInit(void)
{
	uint32_t i = 0, j = 0;

	//Inits the free list
	freeList = &queueTbl[0];

	for(i = 0; i < OS_QUEUE_COUNT; i++)
	{
		//Link all queue control blocks:
		queueTbl[i].nextNode = &queueTbl[i + 1];

		//Put the queue control block in known initial state
		queueTbl[i].numSlots  = 0;
		queueTbl[i].queueBack = 0;
		queueTbl[i].queueFront= 0;
		queueTbl[i].queueBottom = NULL;
		queueTbl[i].queueBase = NULL;
		queueTbl[i].usedSlots = 0;

		for( j = 0 ; j < OS_NUMBER_OF_TASKS; j++)
		{
			queueTbl[i].tasksPending[j] = OS_Q_PEND_NOT;
		}
	}

	//mark the end of linked list:
	queueTbl[OS_QUEUE_COUNT - 1].nextNode = NULL;
}

/*
 * uLipeQueueCreate()
 */
OsHandler_t uLipeQueueCreate(QueueData_t data, uint32_t size, OsStatus_t *err)
{
	uint32_t sReg = 0;
	QueuePtr_t q;

	//Check arguments:
	if(data == NULL)
	{
		err = kInvalidParam;
		return((OsHandler_t)NULL);
	}

	//Valid arguments, then proceed:
	OS_CRITICAL_IN();

	//Check for free blocks:
	if(freeList == NULL)
	{
		OS_CRITICAL_OUT();
		err = kOutOfQueue;
		return((OsHandler_t)NULL);
	}

	q = freeList;

	//updates next freelist:
	freeList = freeList->nextNode;

	OS_CRITICAL_OUT();

	//fill the control block:
	q->queueBase = data;
	q->numSlots  = size;
	q->queueBack = 0;
	q->queueFront = 0;
	q->usedSlots = 0;
	q->queueBottom = data + size;

	err = kStatusOk;

	//Return in handler form:
	return((OsHandler_t)q);
}

/*
 * uLipeQueueInsert()
 */
OsStatus_t uLipeQueueInsert(OsHandler_t h, void *data, uint8_t opt, uint16_t timeout)
{
	uint32_t sReg = 0;
	QueuePtr_t q = (QueuePtr_t)h;

	//check arguments:
	if(data == NULL)
	{
		return(kInvalidParam);
	}
	if(h == NULL)
	{
		return(kInvalidParam);
	}


	//Arguments valid, proceed then:
	OS_CRITICAL_IN();

	//Before insert, check queue status:
	if(q->usedSlots >= q->numSlots)
	{
		OS_CRITICAL_OUT();

		//Queue full, check options:
		switch(opt)
		{
			case OS_Q_BLOCK_FULL:
			{
				//task will block so:
				OS_CRITICAL_IN();

				//suspend current task:
				uLipePrioClr(currentTask->taskPrio, &taskPrioList);
				currentTask->taskStatus = (1 << kTaskPendQueue) | (1 << kTaskPendDelay);
				currentTask->delayTime = timeout;

				//Assert this task on queue wait list:
				q->tasksPending[currentTask->taskPrio] = OS_Q_PEND_EMPTY;

				OS_CRITICAL_OUT();

				//So check for a context switch:
				uLipeTaskYield();

				return(kQueueFull);
			}
			break;

			default:
			{
				//All other cases, only return:
				OS_CRITICAL_OUT();
				return(kQueueFull);
			}
			break;
		}
	}


	//freespace, Insert the data pointer on queue:
	q->queueBase[q->queueFront] = (QueueData_t)data;
	q->queueFront++;

	//queue behaves as a circular FIFO fashion:
	if(q->queueFront > (q->numSlots - 1))
	{
		q->queueFront = 0;
	}

	//update number of used slots
	q->usedSlots++;

	//Run insertion update loop:
	QueueInsertLoop(h);

	OS_CRITICAL_OUT();

	//check for a context switch:
	uLipeTaskYield();

	//All gone ok:
	return(kStatusOk);
}
/*
 * uLipeQueueRemove()
 */
OsStatus_t uLipeQueueRemove(OsHandler_t h, void *data, uint8_t opt, uint16_t timeout)
{
	uint32_t sReg = 0;
	QueuePtr_t q = (QueuePtr_t)h;

	//check arguments:
	if(h == NULL)
	{
		return(kInvalidParam);
	}

	if(data == NULL)
	{
		return(kInvalidParam);
	}


	//Arguments valid, then proceed:
	OS_CRITICAL_IN();

	//Check queue status first:
	if(q->usedSlots == 0)
	{
		OS_CRITICAL_OUT();

		//Queue full, check options:
		switch(opt)
		{
			case OS_Q_BLOCK_EMPTY:
			{
				//task will block so:
				OS_CRITICAL_IN();

				//suspend current task:
				uLipePrioClr(currentTask->taskPrio, &taskPrioList);
				currentTask->taskStatus = (1 << kTaskPendQueue) | (1 << kTaskPendDelay);
				currentTask->delayTime = timeout;

				//Assert this task on queue wait list:
				q->tasksPending[currentTask->taskPrio] = OS_Q_PEND_FULL;

				OS_CRITICAL_OUT();

				//So check for a context switch:
				uLipeTaskYield();

				return(kQueueEmpty);
			}
			break;

			default:
			{
				//All other cases, only return:
				OS_CRITICAL_OUT();
				return(kQueueEmpty);
			}
			break;
		}

	}

	//queue holds data, so remove it:
	data = (void *)q->queueBase[q->queueBack];
	q->queueBack++;

	//queue bevahes as circular FIFO fashion
	if(q->queueBack > (q->numSlots - 1))
	{
		q->queueBack = 0;
	}
	//Update the number of used slots:
	q->usedSlots--;

	//Update tasks wich pend this queue:
	QueueRemoveLoop(h);

	OS_CRITICAL_OUT();

	//Check for context switching:
	uLipeTaskYield();

	//All gone well:
	return(kStatusOk);
}


/*
 * uLipeQueueDelete()
 */
OsStatus_t uLipeQueueDelete(OsHandler_t *h)
{
	uint32_t sReg;
	QueuePtr_t q = (QueuePtr_t)h;


	//check arguments:
	if( *h == NULL)
	{
		return(kInvalidParam);
	}


	//Argument valid, then proceed:

	OS_CRITICAL_IN();

	//Assert all tasks pending the queue will be destroyed:
	QueueDeleteLoop(h);

	//free this block:
	q->queueBase = NULL;
	q->queueBottom = NULL;
	q->nextNode = freeList;
	freeList = q;

	//Destroy the reference:
	h = NULL;

	OS_CRITICAL_OUT();

	//All gone well:
	return(kStatusOk);
}

#endif
