/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsTask.h
 *
 *  \brief uLipe Rtos task management interface file
 *
 *	In this file definitions about task kernel objects and management
 *	routines are provided, the inclusion of this file is made automatically
 *	including the main header file in user application
 *
 *
 *  Author: FSN
 *
 */

//#include "OsBase.h"
#ifndef __OS_TASK_H
#define __OS_TASK_H

/*
 *	tasks modules constants
 */
#define OS_MAX_TASKS		256 //Internal limit of tasks can be created

/*
 *  task status code:
 */
typedef enum					//
{								//
	kTaskReady = 0,				//
	kTaskSuspend,				//
	kTaskDeleted,				//
	kTaskPendDelay,				//
	kTaskPendFlagAll,			//
    kTaskPendFlagAny,           //
    kTaskPenFlagConsume,
	kTaskPendSem,				//
	kTaskPendMtx,				//
	kTaskPendQueue,				//
}TaskState_t;

/*
 * 	task control block
 */
struct OsTCB_
{
	OsStackPtr_t stackTop;		//Pointer that contain the current top of stack
	void        (*task) (void*);//function pointer to task.
	uint16_t	 taskPrio;		//Id of this tcb, corresponds to its priority
	uint32_t	 flagsPending;	//flags to pend register
	uint16_t     delayTime;
	uint16_t     taskStatus;	//The current status of the task
	uint8_t		 tcbTaken;		///.;
    OsPrioListPtr_t mtxBmp;
    OsPrioListPtr_t flagsBmp;
    OsPrioListPtr_t queueBmp;
    OsPrioListPtr_t semBmp;
};

typedef struct OsTCB_ 	OsTCB_t;
typedef struct OsTCB_*	OsTCBPtr_t;

/*
 * 	task management routines proto:
 */

/*!
 * 	ulipeTaskInit()
 *
 *  \brief init all task kernel obects
 *  \param
 *
 *  \return
 *
 */
OsStatus_t uLipeTaskInit(void);

/*!
 * 	ulipeTaskCreate()
 *
 *  \brief install a task and make it ready to run
 *  \param
 *
 *  \return
 *
 */
OsStatus_t uLipeTaskCreate(void (*task) (void * args), OsStackPtr_t taskStack, uint32_t stackSize,
						   uint16_t taskPrio, void *taskArgs);

/*!
 * 	ulipeTaskDelete()
 *
 *  \brief uninstalls a task.
 *  \param
 *
 *  \return
 *
 */
OsStatus_t uLipeTaskDelete( uint16_t taskPrio);

/*!
 * 	ulipeTaskSuspend()
 *
 *  \brief suspend the execution of a task.
 *  \param
 *
 *  \return
 *
 */
OsStatus_t uLipeTaskSuspend( uint16_t taskPrio);

/*!
 * 	ulipeTaskResume()
 *
 *  \brief Resumes a task that is suspended
 *  \param
 *
 *  \return
 *
 */
OsStatus_t uLipeTaskResume( uint16_t taskPrio);

/*!
 * 	ulipeTaskDelay()
 *
 *  \brief Suspends current task execution for a amount of ticks
 *  \param
 *
 *  \return
 *
 */
OsStatus_t uLipeTaskDelay( uint16_t ticks);

#endif
