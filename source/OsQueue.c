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
 * External used variables
 */
extern OsTCBPtr_t currentTask;
extern OsTCBPtr_t tcbPtrTbl[];
extern OsPrioList_t taskPrioList;
extern OsDualPrioList_t timerPendingList;
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

	//extract the highest priority task which waits for a slot:
	i = (uint16_t)uLipeKernelFindHighPrio(&q->queueSlotWait);

	//valid prio?
	if(i != 0)
	{
		//Remove task from wait list:
		uLipePrioClr(i, &q->queueSlotWait);


		//make this task ready:
		tcbPtrTbl[i]->taskStatus &= ~((1 << kTaskPendQueue)|(1 << kTaskPendDelay));
		if(tcbPtrTbl[i]->taskStatus == 0)
		{
	        uLipePrioSet(i, &taskPrioList);
		}
	}
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

	//extract the highest priority task which waits for a slot:
	i = (uint16_t)uLipeKernelFindHighPrio(&q->queueInsertWait);

	//valid prio?
	if(i != 0)
	{
		//Remove task from wait list:
		uLipePrioClr(i, &q->queueInsertWait);


        //make this task ready:
        tcbPtrTbl[i]->taskStatus &= ~((1 << kTaskPendQueue)|(1 << kTaskPendDelay));
        if(tcbPtrTbl[i]->taskStatus == 0)
        {
            uLipePrioSet(i, &taskPrioList);
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
inline static void QueueDeleteLoop(OsHandler_t h)
{
	QueuePtr_t q = (QueuePtr_t)h;
	uint32_t i = 0, j = 0;

		
	//remove out all tasks of all wait lists:
	while((q->queueSlotWait.prioGrp != 0)&&(q->queueInsertWait.prioGrp != 0))
	{
		//Remove for priority order:
		i = uLipeKernelFindHighPrio(&q->queueSlotWait);
		j = uLipeKernelFindHighPrio(&q->queueInsertWait);
		
		uLipePrioClr(i, &q->queueSlotWait);
		uLipePrioClr(j, &q->queueInsertWait);


		//Set these tasks as ready:
        //make this task ready:
        tcbPtrTbl[i]->taskStatus &= ~(1 << kTaskPendQueue);
        if(tcbPtrTbl[i]->taskStatus == 0)
        {
            uLipePrioSet(i, &taskPrioList);
        }

        //make this task ready:
        tcbPtrTbl[j]->taskStatus &= ~(1 << kTaskPendQueue);
        if(tcbPtrTbl[j]->taskStatus == 0)
        {
            uLipePrioSet(j, &taskPrioList);
        }
		
	}		
}

/*
 * uLipeQueueCreate()
 */
OsHandler_t uLipeQueueCreate(uint32_t slots, OsStatus_t *err)
{
	uint32_t sReg = 0;
	QueuePtr_t q = uLipeMemAlloc(sizeof(Queue_t));
	QueueData_t data_array = uLipeMemAlloc(sizeof(QueueData_t) * slots);




	//Valid arguments, then proceed:
	OS_CRITICAL_IN();

	//Check for free blocks:
	if(q == NULL)
	{
		OS_CRITICAL_OUT();
		if(err != NULL) *err = kOutOfQueue;
		return((OsHandler_t)NULL);
	}

    if(data_array == NULL)
    {
        OS_CRITICAL_OUT();
        uLipeMemFree(q);
        if(err != NULL)*err = kInvalidParam;
        return((OsHandler_t)NULL);

    }



	//fill the control block:
	q->queueBase = data_array;
	q->numSlots  = slots;
	q->queueBack = 0;
	q->queueFront = 0;
	q->usedSlots = 0;

    OS_CRITICAL_OUT();

	if(err != NULL) *err = kStatusOk;

	//Return in handler form:
	return((OsHandler_t)q);
}

/*
 * uLipeQueueInsert()
 */
OsStatus_t uLipeQueueInsert(OsHandler_t h, void *data, uint8_t opt, uint16_t timeout)
{
    QueuePtr_t q = (QueuePtr_t)h;
	uint32_t sReg = 0;

	//check arguments:
	if(data == NULL)
	{
		return(kInvalidParam);
	}
	if(q == 0)
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
				currentTask->taskStatus |= (1 << kTaskPendQueue);
				if(timeout != 0)
				{
				    currentTask->taskStatus |= (1 << kTaskPendDelay);
	                currentTask->delayTime = timeout;
	                uLipePrioSet(currentTask->taskPrio, &timerPendingList.list[timerPendingList.activeList]);
				}
				currentTask->queueBmp = &q->queueSlotWait;

				//Adds task to wait list:
				uLipePrioSet(currentTask->taskPrio, &q->queueSlotWait);

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
    QueuePtr_t q = (QueuePtr_t)h;
	uint32_t sReg = 0;
	void *ptr = NULL;

	//check arguments:
	if(q == 0)
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

                uLipePrioClr(currentTask->taskPrio, &taskPrioList);
				//prepare task to wait
                currentTask->taskStatus |= (1 << kTaskPendQueue);
                if(timeout != 0)
                {
                    currentTask->taskStatus |= (1 << kTaskPendDelay);
                    currentTask->delayTime = timeout;
                    uLipePrioSet(currentTask->taskPrio, &timerPendingList.list[timerPendingList.activeList]);
                }

				//Adds task to wait list:
                currentTask->queueBmp = &q->queueInsertWait;
				uLipePrioSet(currentTask->taskPrio, &q->queueInsertWait);
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
	uLipeMemFree(q->queueBase);
	uLipeMemFree(q);

	//Destroy the reference:
	 h = (OsHandler_t *)NULL;
	 q = NULL;

	OS_CRITICAL_OUT();

	uLipeKernelTaskYield();

	//All gone well:
	return(kStatusOk);
}

#endif
