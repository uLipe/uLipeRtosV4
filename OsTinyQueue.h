/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsTinyQueue.h
 *
 *  \brief uLipe Rtos queueing routines interface file
 *
 *	In this file definitions about queueing management is included
 *  such control blocks, interface to routines and useful macros.
 *
 *  Author: FSN
 *
 */

#ifndef OS_TINY_QUEUE_H
#define OS_TINY_QUEUE_H

/*
 *	 Options for queue pend/post
 */
#define OS_Q_PEND_BLOCK_EMPTY 		0x01
#define OS_Q_POST_BLOCK_FULL  		0x02
#define OS_Q_NOT_BLOCK				0x03

/*
 * Queue control block:
 */
typedef uint8_t OsQueueData_t;

struct tinyqueue_ {
	OsQueueData_t *queueData;					//pointer to queue data
	uint32_t queueDataSize;						//Data size of each entry
	uint32_t queueEntries;						//Number of entries
	uint32_t queueCurrentEntries;				//Current inserted entries
	uint32_t queueCurrentFree;					//Current entry free
	uint32_t queueCurrentFront;					//Current first position
	uint16_t queueTaken;							//queue kernel object taken
	uint8_t  tasksPending[OS_NUMBER_OF_TASKS ];	//tasks that wait for this queue
};


typedef struct tinyqueue_ OsQueue_t;
typedef struct tinyqueue_ * OsQueuePtr_t;

/*
 *		This macro can be used to reserve exactly space needed for queue data
 */

#define OS_QUEUE_RESERVE(name, DATASIZE, NUMENTRIES) OsQueueData_t name[(DATASIZE + 1) * NUMENTRIES]

/*!
 *	OsQueueInit()
 * \brief Init queue kernel objects
 * \param
 * \return
 */
void OsQueueInit(void);

/*!
 *	OsQueueCreate()
 * \brief Create and return a handler for a queue
 * \param
 * \return
 */
OsHandler_t OsQueueCreate(OsQueueData_t *dataPtr, uint32_t dataSize,
							uint32_t queueSize, uint8_t *err);

/*!
 *	OsQueuePend()
 * \brief Try to remove a registry from queue, and optionally block task if queue is empty
 * \param
 * \return
 */
OsStatus_t OsQueuePend(OsHandler_t handler, uint8_t *dataPtr,
						uint32_t *sizeInBytes, uint8_t opt);

/*!
 *	OsQueuePost()
 * \brief Try to put a data on queue, and optionally block task if the queue is full
 * \param
 * \return
 */
OsStatus_t OsQueuePost(OsHandler_t handler, uint8_t *dataPtr,
						uint32_t sizeInBytes, uint8_t opt);
/*!
 *	OsQueueGetStatus()
 * \brief Return the current status of queue and the free entries
 * \param
 * \return
 */
OsStatus_t OsQueueGetStatus(OsHandler_t handler, uint32_t *freeEntries);
						

/*!
 *	OsQueueDelete()
 * \brief Deletes a queue and releases the kernel object
 * \param
 * \return
 */
OsStatus_t OsQueueDelete(OsHandler_t handler);

#endif
