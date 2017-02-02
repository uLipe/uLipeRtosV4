/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsKernel.c
 *
 *  \brief uLipe Rtos kernel routines
 *
 *	This file contains all the kernel management definitions and routines
 *	these routines are commonly called by the other rtos modules and the
 *  user should used it with caution.
 *
 *  Author: FSN
 *
 */

#include "uLipeRtos4.h"

/*
 * Module Variables:
 */

OsPrioList_t taskPrioList; 		  //Main installed task priority list
OsTCBPtr_t   currentTask;   	  //pointer to current tcb is being executed
OsTCBPtr_t   highPrioTask;		  //pointer to high priority task ready to run
volatile uint32_t tickCounter;    //Incremented every os tick interrupt
uint8_t  osConfigured = FALSE;
uint8_t  osRunning = FALSE;				  //Kernel executing flag
uint16_t irqCounter;     		  //Irq nesting counter
OsStack_t idleTaskStack[OS_IDLE_TASK_STACK_SIZE];
OsStack_t osMainStk[48];		  //Initial stack pointer

static uint8_t const clz_lkup[] = {
    32, 31, 30, 30, 29, 29, 29, 29,
    28, 28, 28, 28, 28, 28, 28, 28
};
/*
 *	Extenal  variables:
 */
extern OsTCBPtr_t tcbPtrTbl[];

/*
 *  Kernel functions implementation:
 */

/*
 *  uLipeKernelIdleTask()
 *
 *  This is the idle task, which run at lhe least priority
 */
void uLipeKernelIdleTask(void *args)
{
	uint32_t execCounter = 0;

	for(;;)
	{
#if IDLE_TASK_HOOK_EN > 0
		//Hook for a user defined callback:
		IdleTaskHook();
#endif
		//Stays here:
		execCounter++;
		(void)execCounter;
	}
}

/*
 * uLipeKernelClz()
 *
 * This internal function counts the number of leading zeros
 *
 */
#if (OS_FAST_SCHED == 0)
static inline uint32_t uLipeKernelClz(uint32_t x) {
    uint32_t n;

    if (x >= (1 << 16)) {
        if (x >= (1 << 24)) {
            if (x >= (1 << 28)) {
                n = 28;
            }
            else {
                n = 24;
            }
        }
        else {
            if (x >= (1 << 20)) {
                n = 20;
            }
            else {
                n = 16;
            }
        }
    }
    else {
        if (x >= (1 << 8)) {
            if (x >= (1 << 12)) {
                n = 12;
            }
            else {
                n = 8;
            }
        }
        else {
            if (x >= (1 << 4)) {
                n = 4;
            }
            else {
                n = 0;
            }
        }
    }
    return (uint32_t)clz_lkup[x >> n] - n;
}

#endif
/*
 * 	uLipeTaskEntry()
 *
 * 	This internal function, is used to prepare task to start
 * 	its execution, and catch possible task return
 */
void uLipeTaskEntry(void *args)
{
	OsTCBPtr_t tcb;
	OsStatus_t err;

	//Takes the current task to start execution:
	tcb = currentTask;

	//start task:
	tcb->task(args);

	//if this task return, suspend it:
	if(tcb->taskPrio != 0)
	{
		err = uLipeTaskSuspend(tcb->taskPrio);
		if(err != kStatusOk)
		{
			//if cant suspend, task may have some fault, so for safety,
			//delete it:
			err = uLipeTaskDelete(tcb->taskPrio);

		}
	}

	//task trapped for debugf purposes.
	while(1);
}

/*
 * 	uLipeKernelIrqIn()
 */
void uLipeKernelIrqIn(void)
{
	//should run only if kernel is running:
	if(osRunning != TRUE)return;

	if(irqCounter < 0xFFFF) irqCounter++;
}

/*
 * 	ulipeKernelIrqOut()
 */
void uLipeKernelIrqOut(void)
{
	//should run only if kernel is running:
	if(osRunning != TRUE)return;

	if(irqCounter > 0) irqCounter--;

	if(irqCounter == 0)
	{
		//If executed all irqs then request a switch context:
		uLipeKernelTaskYield();
	}
}

/*
 * 	ulipeKernelFindHighPrio()
 */


#if (OS_FAST_SCHED == 0)
uint16_t uLipeKernelFindHighPrio(OsPrioListPtr_t prioList)
{
	uint16_t x   = 0;
	uint16_t y   = 0;
	uint16_t ret = 0;

	//check if argument is valid:
	if(NULL == prioList) return(OS_INVALID_PRIO);

	//find the x and y priority:
	x = OS_KERNEL_ENTRIES_FOR_GROUP - uLipeKernelClz(prioList->prioGrp);
	if(x > 32) x = 31;
	y = OS_KERNEL_ENTRIES_FOR_GROUP - uLipeKernelClz(prioList->prioTbl[x]);
	if(y > 32) y = 31;

	//forms the base priority value:
	ret = (x << 5) | y;
	/* wraps the ret with correct value */
	ret = (ret > (OS_NUMBER_OF_TASKS-1)) ? 0 : ret;

	return(ret);

}
#endif

/*
 * 	ulipeKernelTaskYield()
 */
void uLipeKernelTaskYield(void)
{
	uint16_t prio = 0;

	//should run only if kernel running:
	if(osRunning != TRUE) return;

	// interrupts to treat? abort!
	if(irqCounter > 0) return;

	//find the new highest prio ready to run:
	prio = uLipeKernelFindHighPrio(&taskPrioList);

	//the priority should not be invalid
	uLipeAssert(prio != OS_INVALID_PRIO);

	//access the desired tcb:
	highPrioTask = tcbPtrTbl[prio];

	//check if a context switch is nedded:
	if(highPrioTask != currentTask)
	{
		uLipePortChange();
	}

}

/*
 * 	ulipeKernelRtosTick()
 */
void uLipeKernelRtosTick(void)
{
	OsTCBPtr_t tcb = NULL;
	OsStatus_t err = kStatusOk;

	if(osRunning != TRUE)return;

	uLipeKernelIrqIn();

	//start always from the start of tasklist:
	tcb = tcbPtrTbl[OS_LEAST_PRIO];

	tickCounter++;

	//proccess tick delays for all installed tasks:
	do
	{
		if(tcb->delayTime != 0)
		{
			tcb->delayTime--;

			//Delay time reached to 0:
			if(tcb->delayTime == 0)
			{
				//make this task ready:
				tcb->taskStatus = (1 << kTaskReady);
				uLipePrioSet(tcb->taskPrio, &taskPrioList);

			}
		}

		//goto to next task:
		tcb =(OsTCBPtr_t)tcb->nextTCB;
	}while(tcb != NULL);

	(void)err;

	//find the next task ready to run:
	uLipeKernelIrqOut();
}

/*
 * 	ulipeKernelObjCopy()
 */
OsStatus_t uLipeKernelObjCopy(uint8_t * dest, const uint8_t * src, uint16_t objSize )
{
	//check arguments:
	if(dest == NULL) return(kInvalidParam);
	if(src  == NULL) return(kInvalidParam);
	if(objSize == 0) return(kInvalidParam);

	//copy:
	while(objSize)
	{
		*dest++ = *src++;
		objSize--;
	}

	return(kStatusOk);
}

/*
 * 	ulipeKernelObjSet()
 */
OsStatus_t uLipeKernelObjSet(uint8_t * dest, const uint8_t value, uint16_t objSize )
{
	//check arguments:
	if(dest == NULL)return(kInvalidParam);
	if(objSize == 0)return(kInvalidParam);

	//set the value in all positions:
	while(objSize)
	{
		*dest++ = value;
		objSize--;
	}

	return(kStatusOk);
}

/*
 *
 *
 * 			DEPRECATED:
 *
 *
 */


/*
 * 	ulipePrioSet()
 */
/*
OsStatus_t uLipePrioSet(uint16_t prio, OsPrioListPtr_t prioList)
{
	uint16_t x,y;

	//check arguments:
	if(prioList == NULL) return(kInvalidParam);
	if(prio == OS_INVALID_PRIO) return(kInvalidParam);

	//split prio in groups and elements:
	x = prio >> 5;
	y = prio & 0x1F;

	prioList->prioGrp |= ( 1 << x);
	prioList->prioTbl[x] |= (1 << y);

	return(kStatusOk);
}
*/
/*
 * 	ulipePrioClr()
 */
/*
OsStatus_t uLipePrioClr(uint16_t prio, OsPrioListPtr_t prioList)
{
	uint16_t x,y;

	//check arguments:
	if(prioList == NULL) return(kInvalidParam);
	if(prio == OS_INVALID_PRIO) return(kInvalidParam);

	//split prio in groups and elements:
	x = prio >> 5;
	y = prio & 0x1F;

	prioList->prioTbl[x] &= ~(1 << y);
	//Only clears a group if its is empty:
	if(prioList->prioTbl[x] == 0)
	{
		prioList->prioGrp &= ~( 1 << x);
	}


	return(kStatusOk);

}
*/



/*
 * 	ulipeRtosInit()
 */
OsStatus_t uLipeRtosInit(void)
{
	uint16_t err;

	//put all local variables in known state:
	currentTask  = NULL;
	highPrioTask = NULL;
	tickCounter = 0x0000;
	osRunning = FALSE;
	irqCounter = 0x0000;

	//Init all kernel objects:
	err = uLipeTaskInit();
	uLipeAssert(err == kStatusOk);

#if OS_FLAGS_MODULE_EN > 0
	uLipeFlagsInit();
#endif

#if OS_QUEUE_MODULE_EN > 0
	uLipeQueueInit();
#endif

#if OS_MTX_MODULE_EN > 0
	uLipeMutexInit();
#endif

#if OS_SEM_MODULE_EN > 0
	uLipeSemInit();
#endif

	(void)err;

	//init low level hardware
	uLipeInitMachine();

	//Install idle task:
	err = uLipeTaskCreate(&uLipeKernelIdleTask, &idleTaskStack[0], OS_IDLE_TASK_STACK_SIZE,
						  OS_LEAST_PRIO, 0);
	uLipeAssert(err == kStatusOk);

	osConfigured = TRUE;

	return(kStatusOk);

}



/*
 * 	ulipeRtosStart()
 */
OsStatus_t uLipeRtosStart(void)
{
	//check if os was pre configured:
	if(osConfigured != TRUE) return(kKernelStartFail);

	//Find the first task to run:
	highPrioTask = tcbPtrTbl[uLipeKernelFindHighPrio(&taskPrioList)];

	//check for problems:
	uLipeAssert(highPrioTask != NULL);

	//os is running:
	//osRunning = TRUE;

	//perform the first ctx switch:
	asm("		movs r0, #0x00 \n\r"
		"		msr  primask, r0 \n\r"
		"		svc	 #0x00     \n\r");

	//The os is running.

	//this function should not return:
	return(kKernelStartFail);

}
