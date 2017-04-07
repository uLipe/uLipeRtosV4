/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsFlags.c
 *
 *  \brief uLipe Rtos flag bits management module file
 *
 *  In this file the user will find the definitions and prototypes
 *  of the functions which manages the flag bits groups
 *
 *  Author: FSN
 *
 */


#include "uLipeRtos4.h"

/*
 * Flags internal custom codes
 */

#define OS_PEND_ALL 	0x01  //Only pending flags without consume
#define OS_PEND_ALL_C	0x02  //Pend all flags and consume when assert
#define OS_PEND_ANY		0x04  //Onlu pend for any flags without consume
#define OS_PEND_ANY_C	0x08  //Pend for any flags and consume when asserted
#define OS_PEND_NOT		0x80  //Task which no wait for a flags

#if OS_FLAGS_MODULE_EN > 0

/*
 * Module internal variables:
 */

/*
 *  Module external variables
 */
extern OsTCBPtr_t  tcbPtrTbl[OS_NUMBER_OF_TASKS];		//Array of tcb pointers to external access
extern OsPrioList_t taskPrioList;					    //Ready task list.
extern OsTCBPtr_t currentTask;
extern OsDualPrioList_t timerPendingList;
/*
 * Module implementation:
 *
 */

/*
 * FlagsPostLoop()
 * Internal, used to process when all tasks when a flag is asserted
 */
inline static void FlagsPostLoop(OsHandler_t h)
{
	FlagsGrpPtr_t f = (FlagsGrpPtr_t)h;
	uint16_t i = 0;
	uint16_t match = FALSE;
	uint32_t mask  = 0;
	uint16_t tmp = 0;

	//Search for a match flags:
	i = uLipeKernelFindHighPrio(&f->waitTasks[f->activeList]);

	while(i != 0)
	{
		//check for pend type of this task:
		switch(tcbPtrTbl[i]->taskStatus & ((1 << kTaskPendFlagAll) | (1 << kTaskPendFlagAny)))
		{
			case (1 << kTaskPendFlagAll):
			{
				//Only match if all specific flags are set
				mask = f->flagRegister & tcbPtrTbl[i]->flagsPending;
				if(mask == tcbPtrTbl[i]->flagsPending)
				{
				    match = TRUE;
				    tcbPtrTbl[i]->taskStatus &= ~((1 << kTaskPendFlagAll | (1 << kTaskPendDelay)) );
				}
			}
			break;

			case (1 << kTaskPendFlagAny):
			{
				//Match if any of flags are set
				mask = f->flagRegister;
				if(mask != 0)
				{
				    match = TRUE;
                    tcbPtrTbl[i]->taskStatus &= ~((1 << kTaskPendFlagAny | (1 << kTaskPendDelay)) );
				}
			}
			break;

			default:
                match = FALSE; //task waiting nothing
            break;
		}

		//match ocurred?
		if(match != FALSE)
		{

			//Assert a false for next loop
			match = FALSE;


			//Check if this assert will consume flags:
			if(tcbPtrTbl[i]->taskStatus & (1 << kTaskPenFlagConsume))
			{
				//clear these flags
				tmp |= mask;
				tcbPtrTbl[i]->taskStatus &= ~(1 << kTaskPenFlagConsume) ;
                //If we had a consume event, so clear these flags;
                f->flagRegister &= ~(tmp);

			}

            //Make this task as ready:
            if(tcbPtrTbl[i]->taskStatus == 0) {
                uLipePrioSet(i, &taskPrioList);
            }

		}
		else
		{
		    /* ensure this flag is waiting for flags */
		    if(tcbPtrTbl[i]->taskStatus & ((1 << kTaskPendFlagAll) | (1 << kTaskPendFlagAny))) {
	            uLipePrioSet(i, &f->waitTasks[f->activeList ^ 0x01]);
		    }
		}

	    i = uLipeKernelFindHighPrio(&f->waitTasks[f->activeList]);
	    uLipePrioClr(i, &f->waitTasks[f->activeList]);
	}

	/* swap to new active list */
	f->activeList ^= 0x01;
}

/*
 *  FlagsDeleteLoop()
 *
 *  Internal function, used, when a flag Grp destroyed, to assert
 *  all tasks which pending its
 */

inline static void FlagsDeleteLoop(OsHandler_t h)
{
	FlagsGrpPtr_t f = (FlagsGrpPtr_t)h;
	uint16_t i = 0;

	i = uLipeKernelFindHighPrio(&f->waitTasks[f->activeList]);
	while( i != 0)
	{
        //make this task ready:
        tcbPtrTbl[i]->taskStatus &= ~((1 << kTaskPendFlagAll) | (1 << kTaskPendFlagAny) | (1 << kTaskPenFlagConsume));
        if(tcbPtrTbl[i]->taskStatus == 0) uLipePrioSet(i, &taskPrioList);
        i = uLipeKernelFindHighPrio(&f->waitTasks[f->activeList]);
        uLipePrioClr(i, &f->waitTasks[f->activeList]);

	}
}


/*
 * 	uLipeFlagsCreate()
 */
OsHandler_t uLipeFlagsCreate(OsStatus_t *err)
{
	uint32_t sReg = 0;
	FlagsGrpPtr_t f = uLipeMemAlloc(sizeof(FlagsGrp_t));

	//Check if we have freeFlags:
	OS_CRITICAL_IN();

	if(f == NULL)
	{
		OS_CRITICAL_OUT();
		if(err != NULL) *err = kOutOfFlags;

		//Return a null handler:
		return((OsHandler_t)f);
	}

	f->activeList = 0;
	f->flagRegister = 0;
	f->waitTasks[0].prioGrp = 0;
	f->waitTasks[1].prioGrp = 0;

	OS_CRITICAL_OUT();

	if(err != NULL)*err = kStatusOk;

	//All gone well.
    return((OsHandler_t)f);
}

/*
 *  uLipeFlagsPend()
 */
OsStatus_t uLipeFlagsPend(OsHandler_t h, uint32_t flags, uint8_t opt, uint16_t timeout)
{
	uint32_t sReg = 0;
	uint32_t mask = 0;
	uint16_t match = FALSE;
	FlagsGrpPtr_t f = (FlagsGrpPtr_t)h;


	//Check for valid handler
	if(h == 0)
	{
		return(kInvalidParam);
	}

	OS_CRITICAL_IN();

	//Check if this task already asserted:
	mask = f->flagRegister & currentTask->flagsPending;


	//check the pend type:
	switch(opt & ~(OS_FLAGS_CONSUME))
	{
		case OS_FLAGS_PEND_ALL:
		{
			if(mask != flags)
			{
			    currentTask->taskStatus |= (1 << kTaskPendFlagAll);
				//check if wants to consume:
				if(opt & OS_FLAGS_CONSUME)
				{
	                currentTask->taskStatus |= (1 << kTaskPenFlagConsume);
				}

			}
			else
			{

                if(opt & OS_FLAGS_CONSUME)
                {
                    f->flagRegister &= ~currentTask->flagsPending;
                }


				//The flags of this task is already asserted
				match = TRUE;
			}
		}
		break;

		case OS_FLAGS_PEND_ANY:
		{
			if(f->flagRegister == 0x0000000)
			{
                currentTask->taskStatus |= (1 << kTaskPendFlagAny);
                //check if wants to consume:
                if(opt & OS_FLAGS_CONSUME)
                {
                    currentTask->taskStatus |= (1 << kTaskPenFlagConsume);
                }
			}
			else
			{
                if(opt & OS_FLAGS_CONSUME)
                {
                    f->flagRegister = 0;
                }

				//Any flags of this task was already assert
				match = TRUE;
			}
		}
		break;

		default:
		{
			//Invalid option, return with error:
			OS_CRITICAL_OUT();
			return(kInvalidParam);
		}

	}

	//check for match:
	if(match != FALSE)
	{

		//Only return, without suspend task:
		OS_CRITICAL_OUT();

		return(kStatusOk);
	}

	//Set the flags:
	currentTask->flagsPending |= flags;

	//if not, then suspend task:
	uLipePrioClr(currentTask->taskPrio, &taskPrioList);
	uLipePrioSet(currentTask->taskPrio, &f->waitTasks[f->activeList]);
	currentTask->flagsBmp = &f->waitTasks[0];

	//adds the timeout
	if(timeout != 0)
	{
	    currentTask->delayTime  = timeout;
	    currentTask->taskStatus |= (1 << kTaskPendDelay);
	    uLipePrioSet(currentTask->taskPrio, &timerPendingList.list[timerPendingList.activeList]);
	}

	OS_CRITICAL_OUT();

	//Check for a context switch:
	uLipeKernelTaskYield();

	//all gone well:
	return(kStatusOk);
}

/*
 *  uLipeFlagsPost()
 */
OsStatus_t uLipeFlagsPost(OsHandler_t h, uint32_t flags)
{
	FlagsGrpPtr_t f = (FlagsGrpPtr_t)h;
	uint32_t sReg = 0;

	//Check argument:
	if(h == 0)
	{
		return(kInvalidParam);
	}

	//Valid argument, proceed:


	OS_CRITICAL_IN();

	//Assert the flags in register:
	f->flagRegister |= flags;
	//Run the PostLoop to update the tasks pending:
	FlagsPostLoop(h);

	OS_CRITICAL_OUT();

	//Check for a context switch:
	uLipeKernelTaskYield();

	//All gone well:
	return(kStatusOk);
}

/*
 *  uLipeFlagsDelete()
 */
OsStatus_t uLipeFlagsDelete(OsHandler_t *h)
{
	FlagsGrpPtr_t f = (FlagsGrpPtr_t)h;
	uint32_t sReg = 0;

	//Check argument:
	if(h == NULL)
	{
		return(kInvalidParam);
	}


	//valid argument, then proceed:
	OS_CRITICAL_IN();

	//Assert all flag events before to destroy it:
	FlagsDeleteLoop(*h);
	uLipeMemFree(f);

	OS_CRITICAL_OUT();

	//Destroy this handler:
	*h = 0 ;
	 f = NULL;

	uLipeKernelTaskYield();

	//All gone well:
	return(kStatusOk);
}

#endif
