/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsPort.c
 *
 *  \brief this file is contains the  functions of machine dependent code
 *
 *	In this file definitions about the stack frame, context switching procedures,
 *	The user should not call these routines or use its data structures.
 *
 *  Author: FSN
 *
 */

#include "uLipeRtos4.h"
#include "include/arch/OsArch_Defs_M3_M4_M7.h"

#if (OS_ARCH_CORTEX_M7 == 1) || (OS_ARCH_CORTEX_M4 == 1) || (OS_ARCH_CORTEX_M3 == 1)

/*
 * Systick load value macro:
 */
#define OS_TIMER_LOAD_VAL (uint32_t)(OS_CPU_RATE/OS_TICK_RATE)



/*
 * Functions implementation:
 */

/*
 *  uLipeInitMachine()
 */
void uLipeInitMachine(void)
{
	//First, turn off systick timer:
	SysTick->CTRL = 0x00;

	//Put the load value:
	SysTick->LOAD = OS_TIMER_LOAD_VAL;

	//Enable double word align
	SCB->CCR = 0x200;

	//Assign priority value to pendSv and systick exc:
	SCB->SHP[10] = 0xFE;
	SCB->SHP[11] = 0xFF;
	SCB->SHP[7]  = 0xFF;


	//Enable systick interrupts, ann use external clock source:
	SysTick->CTRL |= 0x07;

}

/*
 *  uLipeInitMachine()
 */
OsStackPtr_t uLipeStackInit(OsStackPtr_t taskStk, void * task, void *taskArgs )
{
	ArmCm4RegListPtr_t ptr;

	//Initialize the stkpointer on first free top position
	ptr = (ArmCm4RegListPtr_t)taskStk - 1;

	ptr->lr = 0xFFFFFFFD;			//Adds exec return on link reg
	ptr->pc = (uint32_t)task;		//task function at pc
	ptr->xPsr = 0x01000000;			//xPsr default value with interrupts enabled
	ptr->r0  = (uint32_t)taskArgs;	//Task arguments are passed thru R0

	//Fill some known values at stack:
	ptr->r1 = 0x11111111;
	ptr->r2 = 0x22222222;
	ptr->r3 = 0x33333333;
	ptr->r4 = 0x44444444;
	ptr->r5 = 0x55555555;
	ptr->r6 = 0x66666666;
	ptr->r7 = 0x77777777;
	ptr->r8 = 0x88888888;
	ptr->r9 = 0x99999999;
	ptr->r10= 0xAAAAAAAA;
	ptr->r11= 0xBBBBBBBB;
	ptr->r12 =0xCCCCCCCC;

	//initialized return current stkptr
	return((OsStackPtr_t)ptr);
}

/*
 *  uLipePortChange()
 */
void uLipePortChange(void)
{
	uint32_t sReg;

	OS_CRITICAL_IN();

	//Request a pendSv execption:
	SCB->ICSR |= (1<<28);

	OS_CRITICAL_OUT();
}

uint32_t uLipePortBitLSScan(uint32_t arg)
{

}


uint32_t uLipePortBitFSScan(uint32_t arg)
{

}

#endif
