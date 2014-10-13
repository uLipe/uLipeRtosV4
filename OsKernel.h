/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsKernel.h
 *
 *  \brief uLipe Rtos kernel interface routines
 *
 *	This file contains all the kernel management definitions and routines
 *	these routines are commonly called by the other rtos modules and the
 *  user should used it with caution.
 *
 *  Author: FSN
 *
 */

//#include "OsBase.h"

#ifndef  __OS_KERNEL_H
#define __OS_KERNEL_H

/*
 *  kernel constants:
 */
#define OS_KERNEL_ENTRIES_FOR_GROUP  31
#define OS_IDLE_TASK_STACK_SIZE      48

/*
 * 	Priority list object:
 */
struct OsPrioList_
{
	uint32_t prioGrp;
	uint32_t prioTbl[OS_KERNEL_ENTRIES_FOR_GROUP];
};

typedef struct OsPrioList_  OsPrioList_t;
typedef struct OsPrioList_* OsPrioListPtr_t;

/*
 *  Kernel functions proto:
 */

/*!
 * 	uLipeKernelIrqIn()
 *
 *  \brief Notify to rtos the application entered in a ISR
 *  \param
 *
 *  \return
 *
 */
void uLipeKernelIrqIn(void);

/*!
 * 	ulipeKernelIrqOut()
 *
 *  \brief Notify to rtos the application ended the ISR and select a new task to run
 *  \param
 *
 *  \return
 *
 */
void uLipeKernelIrqOut(void);

/*!
 * 	ulipeKernelFindHighPrio()
 *
 *  \brief Find high priority set in a priolist
 *  \param
 *
 *  \return
 *
 */
uint16_t uLipeKernelFindHighPrio(OsPrioListPtr_t prioList);

/*!
 * 	ulipeKernelTaskYield()
 *
 *  \brief Checks if necessary the suspend the current task and change context
 *  \param
 *
 *  \return
 *
 */
void uLipeKernelTaskYield(void);

/*!
 * 	ulipeKernelRtosTick()
 *
 *  \brief Process the tick timer interurpt
 *  \param
 *
 *  \return
 *
 */
void uLipeKernelRtosTick(void);

/*!
 * 	ulipeKernelObjCopy()
 *
 *  \brief copies an object from a source location to a dest location
 *  \param
 *
 *  \return
 *
 */
OsStatus_t uLipeKernelObjCopy(uint8_t * dest, const uint8_t * src, uint16_t objSize );

/*!
 * 	ulipeKernelObjSet()
 *
 *  \brief Set a specific value in all bytes of an kernel object
 *  \param
 *
 *  \return
 *
 */
OsStatus_t uLipeKernelObjSet(uint8_t * dest, const uint8_t value, uint16_t objSize );

/*!
 * 	ulipePrioSet()
 *
 *  \brief Set a priority value in a prio list
 *  \param
 *
 *  \return
 *
 */
OsStatus_t uLipePrioSet(uint16_t prio, OsPrioListPtr_t prioList);

/*!
 * 	ulipePrioClr()
 *
 *  \brief clears a priority value in a prio list
 *  \param
 *
 *  \return
 *
 */
OsStatus_t uLipePrioClr(uint16_t prio, OsPrioListPtr_t prioList);


/*!
 * 	ulipeRtosInit()
 *
 *  \brief Perform kernel initialization
 *  \param
 *
 *  \return
 *
 */
OsStatus_t uLipeRtosInit(void);

/*!
 * 	ulipeRtosStart()
 *
 *  \brief Trigger the rtos and start kernel execution
 *  \param
 *
 *  \return
 *
 */
OsStatus_t uLipeRtosStart(void);

#endif

