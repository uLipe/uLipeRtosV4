/*!
 *
 *                          ULIPE RTOS VERSION 4
 *
 *
 *  \file OsCpuMsg.h
 *
 *  \brief uLipe messaging inter cpu
 *
 *  In this file the is contained all the definitions about inter cpu messaging
 *
 *  Author: FSN
 *
 */


#ifndef __OSCPUMSG_H
#define __OSCPUMSG_H


OsStatus_t uLipeIPMInit(void);
OsStatus_t uLipeIPMSendBlocking(void *m, size_t s, uint16_t timeout);
OsStatus_t uLipeIPMSendNonBlocking(void *m, size_t s);
size_t uLipeIPMGetChannelSize(void);
OsStatus_t uLipeIPMRecv(void *m, size_t *s, uint16_t timeout);
OsStatus_t uLipeIPMPeek(void *m, size_t *s);






#endif /* OSCPUMSG_H_ */
