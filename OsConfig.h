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

/*
 * 	clock system and ticks per seconds
 */

#define OS_CPU_RATE  			48000000 //in Hz
#define OS_TICK_RATE			1000	//in Hz

/*
 *  Architecture selection:
 */

//
// ARM Cortex:
//
#define OS_ARCH_CORTEX_M0 	  1
#define OS_ARCH_CORTEX_M3 	  0
#define OS_ARCH_CORTEX_M4     0
#define OS_ARCH_CORTEX_M7     0


//
// More arch definitions
//
#define OS_ARCH_MULTICORE     1

//
// Other archs TBD
//

/*
 * 	Kernel configurations:
 */
#define OS_IDLE_TASK_HOOK_EN		0
#define OS_FAST_SCHED           	0
#define OS_MINIMAL_STACK            32

/*
 * 	task kernel objects and generation code:
 */

#define OS_NUMBER_OF_TASKS 			  	8 //MUST BE > 0
#define OS_TASK_MODULE_EN			    1 //Gererate code for task management

/*
 *  timers and delays:
 */
#define OS_DELAY_TIME_BASE 		(10000/(OS_TICKS_PER_SECOND)) //In steps of 0.1ms

/*
 * event flags kernel objects and code:
 */
#define OS_FLAGS_MODULE_EN			 1
#define OS_FLAGS_COUNT				 1

/*
 * semaphore kernel objects and code
 */
#define OS_SEM_MODULE_EN			  1
#define OS_SEM_COUNT				  1

/*
 * Mutex kernel objects and code
 */
#define OS_MTX_MODULE_EN			  1
#define OS_MTX_COUNT				  1

/*
 * Queue kernel objects and code
 */
#define OS_QUEUE_MODULE_EN		      1
#define OS_QUEUE_COUNT				  1


/*
 * Messaging inter cpu enabled
 */
#define OS_CPU_MSG                   0

/* no support to fast sched in cortex cm0 */
#if (OS_ARCH_CORTEX_M0 == 1) && (OS_FAST_SCHED == 1)
  #error "uLipeKernel: this architecture does not provide hw optimized scheduler"
#endif



#endif
