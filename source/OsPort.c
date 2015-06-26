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

/*
 * Systick load value macro:
 */
#define OS_TIMER_LOAD_VAL (uint32_t)(OS_CPU_RATE/OS_TICK_RATE)

/*
 *  Machine dependent macros:
 */


/** \brief  Structure type to access the System Timer (SysTick).
 */
typedef struct
{
  volatile uint32_t CTRL;                    /*!< Offset: 0x000 (R/W)  SysTick Control and Status Register */
  volatile uint32_t LOAD;                    /*!< Offset: 0x004 (R/W)  SysTick Reload Value Register       */
  volatile uint32_t VAL;                     /*!< Offset: 0x008 (R/W)  SysTick Current Value Register      */
  volatile  uint32_t CALIB;                   /*!< Offset: 0x00C (R/ )  SysTick Calibration Register        */
} SysTick_Type;


/** \brief  Structure type to access the System Control Block (SCB).
 */
typedef struct
{
  volatile  uint32_t CPUID;                   /*!< Offset: 0x000 (R/ )  CPUID Base Register                                   */
  volatile uint32_t ICSR;                    /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register                  */
  volatile uint32_t VTOR;                    /*!< Offset: 0x008 (R/W)  Vector Table Offset Register                          */
  volatile uint32_t AIRCR;                   /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register      */
  volatile uint32_t SCR;                     /*!< Offset: 0x010 (R/W)  System Control Register                               */
  volatile uint32_t CCR;                     /*!< Offset: 0x014 (R/W)  Configuration Control Register                        */
  volatile uint8_t  SHP[12];                 /*!< Offset: 0x018 (R/W)  System Handlers Priority Registers (4-7, 8-11, 12-15) */
  volatile uint32_t SHCSR;                   /*!< Offset: 0x024 (R/W)  System Handler Control and State Register             */
  volatile uint32_t CFSR;                    /*!< Offset: 0x028 (R/W)  Configurable Fault Status Register                    */
  volatile uint32_t HFSR;                    /*!< Offset: 0x02C (R/W)  HardFault Status Register                             */
  volatile uint32_t DFSR;                    /*!< Offset: 0x030 (R/W)  Debug Fault Status Register                           */
  volatile uint32_t MMFAR;                   /*!< Offset: 0x034 (R/W)  MemManage Fault Address Register                      */
  volatile uint32_t BFAR;                    /*!< Offset: 0x038 (R/W)  BusFault Address Register                             */
  volatile uint32_t AFSR;                    /*!< Offset: 0x03C (R/W)  Auxiliary Fault Status Register                       */
  volatile  uint32_t PFR[2];                  /*!< Offset: 0x040 (R/ )  Processor Feature Register                            */
  volatile  uint32_t DFR;                     /*!< Offset: 0x048 (R/ )  Debug Feature Register                                */
  volatile  uint32_t ADR;                     /*!< Offset: 0x04C (R/ )  Auxiliary Feature Register                            */
  volatile  uint32_t MMFR[4];                 /*!< Offset: 0x050 (R/ )  Memory Model Feature Register                         */
  volatile  uint32_t ISAR[5];                 /*!< Offset: 0x060 (R/ )  Instruction Set Attributes Register                   */
  volatile  uint32_t RESERVED0[5];
  volatile  uint32_t CPACR;                   /*!< Offset: 0x088 (R/W)  Coprocessor Access Control Register                   */
} SCB_Type;

#define SysTick_BASE        (0xE000E000UL +  0x0010UL)
#define SCB_BASE            (0xE000E000UL +  0x0D00UL)

#define SCB                 ((SCB_Type       *)     SCB_BASE      )   /*!< SCB configuration struct           */
#define SysTick             ((SysTick_Type   *)     SysTick_BASE  )   /*!< SysTick configuration struct       */


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
	ptr;

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


