/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsQueue.h
 *
 *  \brief this file contains interface to queue managenment routines
 *
 *	In this file te user will find the data structures and function
 *	prototypes to create and manage queue messages.
 *
 *  Author: FSN
 */

#ifndef __OS_QUEUE_H
#define __OS_QUEUE_H

/*
 * Queue custom types:
 */
typedef void*  QueueData_t;

/*
 * Custom macro to reserve an array of pointers to queue data:
 */

#define OS_QUEUE_RESERVE(dataName, numEntries) QueueData_t dataName[numEntries]

/*
 * Queue custom codes
 */
#define OS_Q_BLOCK_EMPTY	0x01 //suspend task on queue empty
#define OS_Q_BLOCK_FULL		0x02 //suspend task to insert in a full queue
#define OS_Q_NON_BLOCK      0x03



/*
 * Queue control block
 */
struct queue_
{
	QueueData_t **queueBase;		//queue data base address
	uint32_t queueFront;			//current queue remove point
	uint32_t queueBack;			    //current queue insertion point
	uint32_t numSlots;				//Number of entries of current queue
	uint32_t usedSlots;				//Number of current used slots

#if OS_USE_DEPRECATED == 1	
	uint8_t tasksPending[OS_NUMBER_OF_TASKS]; //Wait list for pending tasks
#else	
	struct queue_* nextNode;		//Pointer to next queue control block	
	OsPrioList_t queueInsertWait;
	OsPrioList_t queueSlotWait;

#endif
	
};

typedef struct queue_  Queue_t;
typedef struct queue_* QueuePtr_t;

#if OS_QUEUE_MODULE_EN > 0

/*
 * Function prototypes
 */

/*!
 * uLipeQueueInit()
 * \brief Init queue kernel objects
 */
void uLipeQueueInit(void);

/*!
 * uLipeQueueCreate()
 * \brief Creates a queue to be managed
 * \param
 * \return
 */
OsHandler_t uLipeQueueCreate(QueueData_t *data, uint32_t size, OsStatus_t *err);

/*!
 * uLipeQueueInsert()
 * \brief Insert data on selected queue, and pend if desired
 * \param
 * \return
 */
OsStatus_t uLipeQueueInsert(OsHandler_t h, void *data, uint8_t opt, uint16_t timeout);

/*!
 * uLipeQueueRemove()
 * \brief remove data from a slot of queue, and pend if desired
 * \param
 * \return
 */
void *uLipeQueueRemove(OsHandler_t h, uint8_t opt, uint16_t timeout, OsStatus_t *err);

/*!
 * uLipeQueueQuery()
 * \brief query on queue for its status
 * \param
 * \return
 * reserved for future use
 * OsStatus_t uLipeQueueQuery(OsHandler_t h, QueuePtr_t data);
 */

/*!
 * uLipeQueueFlush()
 * \brief Flushes the queue
 * \param
 * \return
 */
OsStatus_t uLipeQueueFlush(OsHandler_t h);


/*!
 * uLipeQueueDelete()
 * \brief Destroy a queue kernel object
 * \param
 * \return
 */
OsStatus_t uLipeQueueDelete(OsHandler_t *h);

#endif


#endif
