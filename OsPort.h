/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsPort.h
 *
 *  \brief this file is contains the header for functions of machine dependent code
 *
 *	In this file definitions about the stack frame, context switching procedures,
 *	The user should not call these routines or use its data structures.
 *
 *  Author: FSN
 *
 */

#ifndef __OS_PORT_H
#define __OS_PORT_H


/*
 * Enter and exit criticals macro:
 */
#define OS_CRITICAL_IN() 	sReg = uLipeEnterCritical()
#define OS_CRITICAL_OUT()   uLipeExitCritical(sReg)


/*
 * Function prototypes:
 */


/*!
 *  uLipeInitMachine()
 *  \brief Inits the machine specific hardware
 *  \param
 *  \return
 */
void uLipeInitMachine(void);

/*!
 *  uLipeInitMachine()
 *  \brief Inits the machine specific hardware
 *  \param
 *  \return
 */
OsStackPtr_t uLipeStackInit(OsStackPtr_t taskStk, void * task, void *taskArgs );
/*!
 *  uLipePortChange()
 *  \brief request a context change interrupt via pendSv
 *  \param
 *  \return
 */
void uLipePortChange(void);

/*!
 *  uLipePortStartKernel()
 *  \brief request the kernel to start its execution
 *  \param
 *  \return
 */
extern void uLipePortStartKernel(void);

/*!
 *  uLipeEnterCritical()
 *  \brief Shut down interrupts and save status registers
 *  \param
 *  \return
 */
extern uint32_t uLipeEnterCritical(void);

/*!
 *  uLipeExitCritical()
 *  \brief Pops the saved interrupts status and status registers
 *  \param
 *  \return
 */
extern void uLipeExitCritical(uint32_t sReg);

/*!
 *  uLipeFisrtSwt()
 *  \brief First switching context routine
 *  \param
 *  \return
 */
//extern void uLipeFirstSwt(void);

/*!
 *  uLipeCtxSwt()
 *  \brief hardware specific context switching routine.
 *  \param
 *  \return
 */
//extern void uLipeCtxSwt(void);

/*!
 *  uLipePortBitLSScan()
 *  \brief hardware specific bit scan first set.
 *  \param
 *  \return
 */
extern uint32_t uLipePortBitLSScan(uint32_t arg);


/*!
 *  uLipePortBitLSScan()
 *  \brief hardware specific bit scan least set.
 *  \param
 *  \return
 */
extern uint32_t uLipePortBitFSScan(uint32_t x);



#if (OS_CPU_MSG == 1) && (OS_ARCH_MULTICORE == 1)

/*!
 * \brief gets the arch dependent ipm channel size
 *
 * \param
 * \return
 */
extern size_t uLipeArchGetIPMChannelSize(void);


/*!
 *
 * \brief sends message to arch dependent ipm
 *
 * \param
 * \return
 */
extern int uLipeArchIPMWrite(void *buf, size_t len);


/*!
 * \brief polls the channel for available data
 *
 * \param
 * \return
 */
extern size_t uLipeArchIPMPoll(void);

/*!
 *\brief receive a message came frol ipm channel
 *
 * \param
 * \return
 */
extern size_t uLipeArchIPMRecv(void *buf, int len);

/*!
 * \brief ipm channel irq handler
 *
 * \param
 * \return
 */
extern void uLipeArchIPMhandler(void);
#endif


#endif
