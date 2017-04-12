/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file OsCpuMsg.c
 *
 *  \brief uLipe messaging inter cpu
 *
 *  In this file the is contained all the definitions about inter cpu messaging
 *
 *  Author: FSN
 *
 */

#include "uLipeRtos4.h"

#if (OS_CPU_MSG == 1) && (OS_ARCH_MULTICORE == 1)

/**
 * static variables
 */

/* mutex used to queue process acess to IPM */
static OsHandler_t ipmLock;
static size_t channelSize;
static OsPrioList_t ipmPrioList;


/* take kernel global information */
extern OsTCBPtr_t currentTask;
extern OsTCBPtr_t tcbPtrTbl[];
extern OsPrioList_t taskPrioList;
extern OsDualPrioList_t timerPendingList;


OsStatus_t uLipeIPMInit(void)
{

}
OsStatus_t uLipeIPMSendBlocking(void *m, ssize_t s, uint16_t timeout)
{

}
OsStatus_t uLipeIPMSendNonBlocking(void *m, ssize_t s)
{

}
ssize_t uLipeIPMGetChannelSize(void)
{

}
OsStatus_t uLipeIPMRecv(void *m, ssize_t *s, uint16_t timeout)
{

}
OsStatus_t uLipeIPMPeek(void *m, ssize_t *s)
{

}
#endif
