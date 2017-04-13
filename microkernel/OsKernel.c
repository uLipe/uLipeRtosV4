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

OsPrioList_t taskPrioList = {0}; 		     //Main installed task priority list
OsDualPrioList_t timerPendingList = {0};    //timer pending delayed list
OsTCBPtr_t   currentTask = NULL;   	        //pointer to current tcb is being executed
OsTCBPtr_t   highPrioTask = NULL;		     //pointer to high priority task ready to run



volatile uint32_t tickCounter;    //Incremented every os tick interrupt
uint8_t  osConfigured = FALSE;
uint8_t  osRunning = FALSE;				  //Kernel executing flag
uint16_t irqCounter;     		  //Irq nesting counter

/*
 *	External  variables:
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

	for(;;)
	{
#if IDLE_TASK_HOOK_EN > 0
		//Hook for a user defined callback:
		IdleTaskHook();
#endif
	}
}


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
	x = OS_KERNEL_ENTRIES_FOR_GROUP - uLipePortBitFSScan(prioList->prioGrp);
	if(x > 32) x = 31;
	y = OS_KERNEL_ENTRIES_FOR_GROUP - uLipePortBitFSScan(prioList->prioTbl[x]);
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
	uLipeAssert(tcbPtrTbl[prio] != NULL);

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
	uint16_t i = 0;

	if(osRunning != TRUE)return;

	uLipeKernelIrqIn();
	//start always from the start of tasklist:


	i = uLipeKernelFindHighPrio(&timerPendingList.list[timerPendingList.activeList]);

	if(i != 0)
	{
	    //proccess tick delays for all installed tasks:
	    do
	    {
	        OsTCBPtr_t tcb = tcbPtrTbl[i];
	        uLipePrioClr(i, &timerPendingList.list[timerPendingList.activeList]);

	        uLipeAssert(tcb->delayTime > 0);

            //goto to next task:
	        if(tcb->delayTime != 0)
	        {
	            tcb->delayTime--;

	            //Delay time reached to 0:
	            if(tcb->delayTime == 0)
	            {
	                //make this task ready and if pending another object
	                //discard it
	                tcb->taskStatus = 0;
                    if(tcb->mtxBmp != NULL)
                    {
                        uLipePrioClr(tcb->taskPrio, tcb->mtxBmp);
                        tcb->mtxBmp = NULL;

                    }

                    /* flags has a special acess case */
	                if(tcb->flagsBmp != NULL)
	                {
	                    uLipePrioClr(tcb->taskPrio, tcb->flagsBmp);
                        uLipePrioClr(tcb->taskPrio, tcb->flagsBmp + sizeof(OsPrioList_t));
                        tcb->flagsBmp = NULL;
	                }

	                if(tcb->queueBmp != NULL)
	                {
                        uLipePrioClr(tcb->taskPrio, tcb->queueBmp);
                        tcb->queueBmp = NULL;
	                }

                    if(tcb->semBmp != NULL)
                    {
                        uLipePrioClr(tcb->taskPrio, tcb->semBmp);
                        tcb->semBmp= NULL;

                    }

	                uLipePrioSet(tcb->taskPrio, &taskPrioList);
	            }
	            else if((tcb->taskStatus & (1 << kTaskPendDelay)) == 0)
	            {
	                (void)0;
	            }
	            else
	            {
	                //task does not reached to it delay value, move its tcb to backup list
	                uLipePrioSet(i, &timerPendingList.list[timerPendingList.activeList ^ 0x01]);
	            }
	        }

	        i = uLipeKernelFindHighPrio(&timerPendingList.list[timerPendingList.activeList]);
	    }while(i != 0);
	    timerPendingList.activeList ^= 0x01;
	}


	//find the next task ready to run:
	uLipeKernelIrqOut();
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


bool uLipeKernelIsRunning(void)
{
    bool ret = (osRunning == TRUE)? true : false;
    return(ret);
}
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


	err = uLipeMemInit();
    uLipeAssert(err == kStatusOk);

	//init low level hardware
	uLipeInitMachine();

	//Install idle task:
	err = uLipeTaskCreate(&uLipeKernelIdleTask, OS_IDLE_TASK_STACK_SIZE,
						  OS_LEAST_PRIO, 0);
	uLipeAssert(err == kStatusOk);

#if OS_USE_DEVICE_DRIVERS > 0
	uLipeDeviceTblInit();
#endif

	osConfigured = TRUE;
	(void)err;


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


	//perform the first ctx switch:
	uLipePortStartKernel();

	//this function should not return:
	return(kKernelStartFail);

}
