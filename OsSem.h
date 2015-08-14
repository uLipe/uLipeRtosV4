/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsSem.h
 *
 *  \brief this file contains the data structures and interface
 *  for count semaphore management
 *
 *	In this file the user will find the data structures, and function
 *	prototype to create and manage counting semaphores objects
 *
 *  Author: FSN
 *
 */

#ifndef __OS_SEM_H
#define __OS_SEM_H

/*
 * Semaphore control block:
 */
struct sem_
{
	uint16_t semLimit;						   //Semaphore counting limit
	uint16_t semCount;						   //Semaphore current count

#if OS_USE_DEPRECATED == 1
	uint8_t  tasksPending[OS_NUMBER_OF_TASKS]; //Wait list of pending tasks
#else
	OsPrioList_t tasksWaiting;
#endif

	struct sem_* nextNode;					   //next node in sem linked list
};

typedef struct sem_  Sem_t;
typedef struct sem_* SemPtr_t;

#if OS_SEM_MODULE_EN > 0


/*
 * Counting semaphore function prorotypes:
 */

/*!
 * uLipeSemInit()
 * \brief Init the semaphore kernel objects
 */
void uLipeSemInit(void);

/*!
 * uLipeSemCreate()
 * \brief Create a semaphore to manage
 * \param
 * \return
 */
OsHandler_t uLipeSemCreate(uint16_t initCount, uint16_t limitCount,OsStatus_t *err);

/*!
 * uLipeSemTake()
 * \brief Take a semaphore, if available, suspend task execution if not
 * \param
 * \return
 */
OsStatus_t uLipeSemTake(OsHandler_t h, uint16_t timeout);

/*!
 * uLipeSemGive()
 * \brief Release the previous used semaphore
 * \param
 * \return
 */
OsStatus_t uLipeSemGive(OsHandler_t h, uint16_t count);

/*!
 * uLipeSemDelete()
 * \brief Delete a semaphore kernel object
 * \param
 * \return
 */
OsStatus_t uLipeSemDelete(OsHandler_t *h);


#endif
#endif
