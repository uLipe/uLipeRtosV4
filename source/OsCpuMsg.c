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

#if OS_CPU_MSG == 1




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
