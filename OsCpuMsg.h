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


#if OS_CPU_MSG > 0
/**
 * \brief Inits the inter processor messaging mechanism
 *
 * \param
 * \return
 */
OsStatus_t uLipeIPMInit(void);

/**
 * \brief Send inter processor message and waits until is succesfull sent
 *
 * \param
 * \return
 */
OsStatus_t uLipeIPMSendBlocking(void *m, ssize_t s, uint16_t timeout);

/**
 * \brief Sends iner processor message and return immediately if fails
 *
 * \param
 * \return
 */
OsStatus_t uLipeIPMSendNonBlocking(void *m, ssize_t s);

/**
 * \brief Gets the current channel memory size
 *
 * \param
 * \return
 */
ssize_t uLipeIPMGetChannelSize(void);

/**
 * \brief Wait for a message from interprocessor channel, when arrive, consume it
 *
 * \param
 * \return
 */
OsStatus_t uLipeIPMRecv(void *m, ssize_t *s, uint16_t timeout);


/**
 * \brief check the presnce of message if positive, reads but not consume it
 *
 * \param
 * \return
 */
OsStatus_t uLipeIPMPeek(void *m, ssize_t *s);
#endif

#endif /* OSCPUMSG_H_ */
