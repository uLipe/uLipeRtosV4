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

static FlagsGrpPtr_t flagsFree;				//pointer to next free block
FlagsGrp_t flagsTbl[OS_FLAGS_COUNT];//Flags node linked list

/*
 *  Module external variables
 */
extern OsTCBPtr_t  tcbPtrTbl[OS_NUMBER_OF_TASKS];		//Array of tcb pointers to external access
extern OsPrioList_t taskPrioList;					    //Ready task list.
extern OsTCBPtr_t currentTask;
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
	for(i = 0; i < OS_NUMBER_OF_TASKS; i++)
	{
		//check for pend type of this task:
		switch(f->taskPending[i] & ~(OS_PEND_ANY_C | OS_PEND_ALL_C))
		{
			case OS_PEND_NOT:
				 match = FALSE; //task waiting nothing
			break;

			case OS_PEND_ALL:
			{
				//Only match if all specific flags are set
				mask = f->flagRegister & tcbPtrTbl[i]->flagsPending;
				if(mask == tcbPtrTbl[i]->flagsPending) match = TRUE;
			}
			break;

			case OS_PEND_ANY:
			{
				//Match if any of flags are set
				mask = f->flagRegister & tcbPtrTbl[i]->flagsPending;
				if(mask != 0) match = TRUE;
			}
			break;
		}

		//match ocurred?
		if(match != FALSE)
		{

			//Assert a false for next loop
			match = FALSE;

			//Make this task as ready:
			tcbPtrTbl[i]->taskStatus = (1 << kTaskReady);
			uLipePrioSet(i, &taskPrioList);

			//Check if this assert will consume flags:
			if((f->taskPending[i] & OS_PEND_ANY_C) || (f->taskPending[i] & OS_PEND_ALL_C))
			{
				//clear these flags
				tmp |= mask;
			}
			//clear code of pending type
			f->taskPending[i] = OS_PEND_NOT;
		}


	}

	//If we had a consume event, so clear these flags;
	f->flagRegister &= ~(tmp);

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

	for( i = 0; i < OS_NUMBER_OF_TASKS; i++)
	{
		//Check if the task is waiting a event first:
		if(f->taskPending[i] != OS_PEND_NOT)
		{
			//make this task ready:
			tcbPtrTbl[i]->taskStatus = (1 << kTaskReady);
			uLipePrioSet(i, &taskPrioList);
		}
		f->taskPending[i]= OS_PEND_NOT;
	}

}
/*
 *  uLipeFlagsInit()
 */
void uLipeFlagsInit(void)
{
	uint16_t i = 0, j = 0;

	//Init all kernel objects:
	flagsFree = &flagsTbl[0];

	for( i = 0; i < OS_FLAGS_COUNT; i++)
	{
		//Link the next element of LL
		flagsTbl[i].nextNode = &flagsTbl[i + 1];
		flagsTbl[i].flagRegister = 0;

		for(j = 0; j < OS_NUMBER_OF_TASKS; j++)
		{
			//Put tasks pending in initial conditions
			flagsTbl[i].taskPending[j] = OS_PEND_NOT;
		}


	}

	//The last element of LL is null to mark its end.
	flagsTbl[OS_FLAGS_COUNT - 1].nextNode = NULL;

}

/*
 * 	uLipeFlagsCreate()
 */
OsHandler_t uLipeFlagsCreate(OsStatus_t *err)
{
	uint32_t sReg = 0;
	OsHandler_t h;

	//Check if we have freeFlags:
	OS_CRITICAL_IN();

	if(flagsFree == NULL)
	{
		OS_CRITICAL_OUT();
		if(err != NULL) *err = kOutOfFlags;

		//Return a null handler:
		return((OsHandler_t)0);
	}

	//Give this flag:
	h = (OsHandler_t)flagsFree;

	//Points to next flag free:
	flagsFree = flagsFree->nextNode;

	OS_CRITICAL_OUT();

	if(err != NULL)*err = kStatusOk;

	//All gone well.
	return(h);
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

				//Put the flag code to pend:
				f->taskPending[currentTask->taskPrio] = OS_PEND_ALL;

				//check if wants to consume:
				if(opt & OS_FLAGS_CONSUME)
				{
					f->taskPending[currentTask->taskPrio] |= OS_PEND_ALL_C;
				}

			}
			else
			{
				//The flags of this task is already asserted
				match = TRUE;
			}
		}
		break;

		case OS_FLAGS_PEND_ANY:
		{
			if(mask == 0x0000000)
			{

				//Put the flag code to pend:
				f->taskPending[currentTask->taskPrio] = OS_PEND_ANY;

				//check if wants to consume:
				if(opt & OS_FLAGS_CONSUME)
				{
					f->taskPending[currentTask->taskPrio] |= OS_PEND_ANY_C;
				}
			}
			else
			{
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
	currentTask->taskStatus &= ~(1 << kTaskReady);

	//Assert the pend type.
	currentTask->delayTime = timeout;
	currentTask->taskStatus = (1 << kTaskPendDelay) | (1 << kTaskPendFlag);


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

	//link this handler on flagsFrees:
	f->nextNode = flagsFree;
	flagsFree = f;

	OS_CRITICAL_OUT();

	//Destroy this handler:
	*h = 0 ;
	 f = NULL;


	uLipeKernelTaskYield();

	//All gone well:
	return(kStatusOk);
}

#endif
