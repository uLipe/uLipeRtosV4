/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsMutex.h
 *
 *  \brief this file contains the data structures and interface
 *  for mutual exclusion semaphore management
 *
 *	In this file the user will find the data structures, and function
 *	prototype to create and manage mutex objects
 *
 *  Author: FSN
 *
 */

#ifndef __OS_MUTEX_H
#define __OS_MUTEX_H

/*
 * Mutex priority value (default)
 */
#define OS_MUTEX_PRIO (OS_HIGHEST_PRIO)


/*
 * Mutex control block:
 */

struct mutex_
{
	uint16_t mutexOwner;		//prio value of mutex owner
	uint16_t mutexTaken;		//flag to mutex taken
	OsPrioList_t tasksPending;  //tasks that pending the mutex
};

typedef struct mutex_  Mutex_t;
typedef struct mutex_* MutexPtr_t;

#if OS_MTX_MODULE_EN > 0

/*
 * Function prototypes:
 */

/*!
 * uLipeMutexCreate
 * \brief Creates a Mutex to be managed
 * \param
 * \return
 */
OsHandler_t uLipeMutexCreate(OsStatus_t *err);


/*!
 * uLipeMutexTake()
 * \brief Take a resource from a mutex, and suspend task if its not available
 * \param
 * \return
 */
OsStatus_t uLipeMutexTake(OsHandler_t h);


/*!
 * uLipeMutexGive()
 * \brief Release a resource used from a mutex
 * \param
 * \return
 */
OsStatus_t uLipeMutexGive(OsHandler_t h);

/*!
 * uLipeMutexDelete()
 * \brief Destroy a mutex kernel object
 * \param
 * \return
 */
OsStatus_t uLipeMutexDelete(OsHandler_t *h);

#endif
#endif
