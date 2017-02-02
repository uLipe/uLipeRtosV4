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

static QueuePtr_t queueFree;			//Pointer to next freeblock of queue
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
inline static void QueueRemoveLoop(OsHandler_t h)
{
	QueuePtr_t q = (QueuePtr_t)h;	
	uint32_t i = 0;

#if OS_USE_DEPRECATED == 1	
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
#else

	//extract the highest priority task which waits for a slot:
	i = (uint16_t)uLipeKernelFindHighPrio(&q->queueSlotWait);

	//valid prio?
	if(i != 0)
	{
		//Remove task from wait list:
		uLipePrioClr(i, &q->queueSlotWait);


		//make this task ready:
		tcbPtrTbl[i]->taskStatus = (1 << kTaskReady);
		uLipePrioSet(i, &taskPrioList);
	}	
	
	
#endif	
	
}

/*
 * QueueInsertLoop()
 *
 * Internal function, used to proccess all tasks that waiting to at least
 * one slot became filled, when a slot had is space filled of data
 */
inline static void QueueInsertLoop(OsHandler_t h)
{
	QueuePtr_t q = (QueuePtr_t)h;
	uint32_t i = 0;

#if OS_USE_DEPRECATED == 1		
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
#else
	//extract the highest priority task which waits for a slot:
	i = (uint16_t)uLipeKernelFindHighPrio(&q->queueInsertWait);

	//valid prio?
	if(i != 0)
	{
		//Remove task from wait list:
		uLipePrioClr(i, &q->queueInsertWait);


		//make this task ready:
		tcbPtrTbl[i]->taskStatus = (1 << kTaskReady);		
		uLipePrioSet(i, &taskPrioList);
	}	
#endif	
}

/*
 * QueueDeleteLoop()
 *
 * Internal function, used to assert all tasks waiting for any event related
 * to queue when a kernel object is about to be destroyed
 *
 */
inline static void QueueDeleteLoop(OsHandler_t h)
{
	QueuePtr_t q = (QueuePtr_t)h;
	uint32_t i = 0, j = 0;

#if OS_USE_DEPRECATED == 1	
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
#else
		
	//remove out all tasks of all wait lists:
	while((q->queueSlotWait.prioGrp != 0)&&(q->queueInsertWait.prioGrp != 0))
	{
		//Remove for priority order:
		i = uLipeKernelFindHighPrio(&q->queueSlotWait);
		j = uLipeKernelFindHighPrio(&q->queueInsertWait);
		
		uLipePrioClr(i, &q->queueSlotWait);
		uLipePrioClr(j, &q->queueInsertWait);


		//Set these tasks as ready:
		tcbPtrTbl[i]->taskStatus = (1 << kTaskReady);
		uLipePrioSet(i, &taskPrioList);		
		tcbPtrTbl[j]->taskStatus = (1 << kTaskReady);
		uLipePrioSet(j, &taskPrioList);		
		
	}		
#endif
}
/*
 * uLipeQueueInit()
 */
void uLipeQueueInit(void)
{
	uint32_t i = 0, j = 0;

	//Inits the free list
	queueFree = &queueTbl[0];

	for(i = 0; i < OS_QUEUE_COUNT; i++)
	{
		//Link all queue control blocks:
		queueTbl[i].nextNode = &queueTbl[i + 1];

		//Put the queue control block in known initial state
		queueTbl[i].numSlots  = 0;
		queueTbl[i].queueBack = 0;
		queueTbl[i].queueFront= 0;
		queueTbl[i].queueBase = NULL;
		queueTbl[i].usedSlots = 0;

#if OS_USE_DEPRECATED == 1
		for( j = 0 ; j < OS_NUMBER_OF_TASKS; j++)
		{
			queueTbl[i].tasksPending[j] = OS_Q_PEND_NOT;
		}
#else
		/*
		 * Clears all wait lists:
		 */
		memset(&queueTbl[i].queueInsertWait, 0, sizeof(OsPrioList_t));
		memset(&queueTbl[i].queueSlotWait, 0, sizeof(OsPrioList_t));
		(void)j;

#endif

	}

	//mark the end of linked list:
	queueTbl[OS_QUEUE_COUNT - 1].nextNode = NULL;
}

/*
 * uLipeQueueCreate()
 */
OsHandler_t uLipeQueueCreate(QueueData_t *data, uint32_t size, OsStatus_t *err)
{
	uint32_t sReg = 0;
	QueuePtr_t q;

	//Check arguments:
	if(data == NULL)
	{
	    if(err != NULL)*err = kInvalidParam;
		return((OsHandler_t)NULL);
	}

	//Valid arguments, then proceed:
	OS_CRITICAL_IN();

	//Check for free blocks:
	if(queueFree == NULL)
	{
		OS_CRITICAL_OUT();
		if(err != NULL) *err = kOutOfQueue;
		return((OsHandler_t)NULL);
	}

	q = queueFree;

	//updates next queueFree:
	queueFree = queueFree->nextNode;

	OS_CRITICAL_OUT();

	//fill the control block:
	q->queueBase = data;
	q->numSlots  = size;
	q->queueBack = 0;
	q->queueFront = 0;
	q->usedSlots = 0;

	if(err != NULL) *err = kStatusOk;

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
	if(h == 0)
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
				//suspend current task:
				uLipePrioClr(currentTask->taskPrio, &taskPrioList);
				currentTask->taskStatus = (1 << kTaskPendQueue) | (1 << kTaskPendDelay);
				currentTask->delayTime = timeout;
#if OS_USE_DEPRECATED == 1
				//Assert this task on queue wait list:
				q->tasksPending[currentTask->taskPrio] = OS_Q_PEND_FULL;
#else
				//Adds task to wait list:
				uLipePrioSet(currentTask->taskPrio, &q->queueSlotWait);
#endif	


				OS_CRITICAL_OUT();

				//So check for a context switch:
				uLipeKernelTaskYield();

				return(kQueueFull);
			}
			break;

			case OS_Q_NON_BLOCK:
			{
			    if(q->usedSlots < q->numSlots) break;
			    else
			    {
	                OS_CRITICAL_OUT();
	                return(kQueueFull);
			    }

			}
			break;

			default:
			{

				OS_CRITICAL_OUT();
				return(kQueueFull);
			}
			break;
		}
	}


	//freespace, Insert the data pointer on queue:

	q->queueBase[q->queueFront] = (void *)data;
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
	uLipeKernelTaskYield();

	//All gone ok:
	return(kStatusOk);
}
/*
 * uLipeQueueRemove()
 */
void *uLipeQueueRemove(OsHandler_t h, uint8_t opt, uint16_t timeout, OsStatus_t *err)
{
	uint32_t sReg = 0;
	QueuePtr_t q = (QueuePtr_t)h;
	void *ptr = NULL;

	//check arguments:
	if(h == 0)
	{
        if(err != NULL )*err = kInvalidParam ;
		return(NULL);
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

#if OS_USE_DEPRECATED == 1				
				//Assert this task on queue wait list:
				q->tasksPending[currentTask->taskPrio] = OS_Q_PEND_EMPTY;
#else
				//Adds task to wait list:
				uLipePrioSet(currentTask->taskPrio, &q->queueInsertWait);
#endif	
				OS_CRITICAL_OUT();

				//So check for a context switch:
				uLipeKernelTaskYield();

                if(err != NULL )*err = kQueueEmpty;

				return(ptr);
			}
			break;
            case OS_Q_NON_BLOCK:
            {
                if(q->usedSlots != 0) break;
                else
                {
                    OS_CRITICAL_OUT();

                    if(err != NULL )*err = kQueueEmpty;
                    return(ptr);
                }

            }
            break;
			default:
			{
				//All other cases, only return:
                if(err != NULL )*err = kQueueEmpty;
				return(ptr);
			}
			break;
		}

	}

	//queue holds data, so remove it:
	ptr = (void *) q->queueBase[q->queueBack];
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
	uLipeKernelTaskYield();

	if(err != NULL )*err = kStatusOk;

	//All gone well:
	return(ptr);
}
/*
 * uLipeQueueFlush()
 */
OsStatus_t uLipeQueueFlush(OsHandler_t h)
{
	uint32_t sReg;
	QueuePtr_t q = (QueuePtr_t)h;

	//check parameters:
	if(h == 0)
	{
		return(kInvalidParam);
	}

	//So flush queue:
	OS_CRITICAL_IN();

	q->queueBack = 0;
	q->queueFront = 0;
	q->usedSlots = 0;

	//Update for all pending tasks:
	QueueRemoveLoop(h);
	QueueInsertLoop(h);

	//Queue flushed:
	OS_CRITICAL_OUT();

	//schedule a new task (if needed):
	uLipeKernelTaskYield();

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
	if( h == (OsHandler_t *)NULL)
	{
		return(kInvalidParam);
	}


	//Argument valid, then proceed:

	OS_CRITICAL_IN();

	//Assert all tasks pending the queue will be destroyed:
	QueueDeleteLoop(*h);

	//free this block:
	q->queueBase = NULL;
	q->nextNode = queueFree;
	queueFree = q;

	//Destroy the reference:
	 h = (OsHandler_t *)NULL;
	 q = NULL;

	OS_CRITICAL_OUT();

	uLipeKernelTaskYield();

	//All gone well:
	return(kStatusOk);
}

#endif
