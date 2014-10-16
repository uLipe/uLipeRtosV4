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
 *  Machine dependent header files:
 */
#include "cmsis_device.h"

/*
 * Enter and exit criticals macro:
 */
#define OS_CRITICAL_IN() 	sReg = uLipeEnterCritical()
#define OS_CRITICAL_OUT()   uLipeExitCritical(sReg)


/*
 * Stack frame data structure:
 */
struct ctx_
{

	//And these are the sofware context saved registers
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;

	//These are the hardware context saved registers.
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t pc;
	uint32_t xPsr;
};


typedef struct ctx_   ArmCm4RegList_t;
typedef struct ctx_ * ArmCm4RegListPtr_t;


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
extern void uLipeFirstSwt(void);

/*!
 *  uLipeCtxSwt()
 *  \brief hardware specific context switching routine.
 *  \param
 *  \return
 */
extern void uLipeCtxSwt(void);



#endif
