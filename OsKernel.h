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
#include "OsConfig.h"

#define OS_KERNEL_ENTRIES_FOR_GROUP  31
#define OS_IDLE_TASK_STACK_SIZE      32

/*
 * 	Priority list object:
 */
struct OsPrioList_
{
	uint32_t prioGrp;
	uint32_t prioTbl[(OS_NUMBER_OF_TASKS/32) + 1];
};

typedef struct OsPrioList_  OsPrioList_t;
typedef struct OsPrioList_* OsPrioListPtr_t;

/*
 * Dual priolist
 */
struct dualpriolist_
{
    OsPrioList_t list[2];
    uint8_t activeList;

};

typedef struct dualpriolist_  OsDualPrioList_t;
typedef struct dualpriolist_* OsDualPrioListPtr_t;


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
#ifndef OS_FAST_SCHED
uint16_t uLipeKernelFindHighPrio(OsPrioListPtr_t prioList);
#else
extern uint16_t uLipeKernelFindHighPrio(OsPrioListPtr_t prioList);
#endif

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
//void uLipeKernelRtosTick(void);


/*!
 *  ulipeKernelIsRunning()
 *
 *  \brief Perform kernel initialization
 *  \param
 *
 *  \return
 *
 */
bool uLipeKernelIsRunning(void);

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


/*
 *
 * 	Inline fast functions:
 *
 */

/*!
 * 	ulipePrioSet()
 *
 *  \brief Set a priority value in a prio list
 *  \param
 *
 *  \return
 *
 */
static __inline void uLipePrioSet(uint16_t prio, OsPrioListPtr_t prioList)
{
	uint16_t x,y;
	//split prio in groups and elements:
	x = prio >> 5;
	y = prio & 0x1F;

	prioList->prioGrp |= ( 1 << x);
	prioList->prioTbl[x] |= (1 << y);
}

/*!
 * 	ulipePrioClr()
 *
 *  \brief clears a priority value in a prio list
 *  \param
 *
 *  \return
 *
 */
static __inline void uLipePrioClr(uint16_t prio, OsPrioListPtr_t prioList)
{
	uint16_t x,y;

	//split prio in groups and elements:
	x = prio >> 5;
	y = prio & 0x1F;

	prioList->prioTbl[x] &= ~(1 << y);
	//Only clears a group if its is empty:
	if(prioList->prioTbl[x] == 0)
	{
		prioList->prioGrp &= ~( 1 << x);
	}
}


#endif

