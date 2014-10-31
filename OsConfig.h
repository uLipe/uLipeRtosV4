/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsConfig.h
 *
 *  \brief uLipe Rtos config file
 *
 *  In this file the user can select and specify the quantity of kernel objects
 *  and which code is generated for these modules, in this file also the user
 *  should provide the cpu clock rate and the ticks per second count
 *
 *  Author: FSN
 *
 */

#ifndef __OS_CONFIG_H
#define __OS_CONFIG_H

#include <stdint.h>


/*
 * 	clock system and ticks per seconds
 */

#define OS_CPU_RATE  			8000000 //in Hz
#define OS_TICK_RATE			100	//in Hz

/*
 * 	Kernel configurations:
 */
#define OS_IDLE_TASK_HOOK_EN		0


/*
 * 	task kernel objects and generation code:
 */

#define OS_NUMBER_OF_TASKS 			 10 //MUST BE > 0
#define OS_TASK_MODULE_EN			  1 //Gererate code for task management

/*
 *  timers and delays:
 */
#define OS_DELAY_TIME_BASE 		(10000/(OS_TICKS_PER_SECOND)) //In steps of 0.1ms

/*
 * event flags kernel objects and code:
 */
#define OS_FLAGS_MODULE_EN			 1
#define OS_FLAGS_COUNT				 2

/*
 * semaphore kernel objects and code
 */
#define OS_SEM_MODULE_EN			1
#define OS_SEM_COUNT				2

/*
 * Mutex kernel objects and code
 */
#define OS_MTX_MODULE_EN			1
#define OS_MTX_COUNT				2

/*
 * Queue kernel objects and code
 */
#define OS_QUEUE_MODULE_EN			1
#define OS_QUEUE_COUNT				2


#endif
