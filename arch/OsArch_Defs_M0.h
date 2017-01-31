/*!
 *
 * 							ULIPE RTOS VERSION 4
 *
 *
 *  \file OsArch_M7.h
 *
 *  \brief this file is contains the machine dependent macros and definitions
 *
 *
 *  Author: FSN
 *
 */
#include "uLipeRtos4.h"

#ifndef __OS_ARCH_DEFS_M7_H
#define __OS_ARCH_DEFS_M7_H

#if (OS_ARCH_CORTEX_M0 == 1)

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
	volatile  uint32_t CPUID;                  /*!< Offset: 0x000 (R/ )  CPUID Base Register */
	volatile uint32_t ICSR;                   /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register */
	uint32_t RESERVED0;
	volatile uint32_t AIRCR;                  /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register */
	volatile uint32_t SCR;                    /*!< Offset: 0x010 (R/W)  System Control Register */
	volatile uint32_t CCR;                    /*!< Offset: 0x014 (R/W)  Configuration Control Register */
	uint32_t RESERVED1;
	volatile uint32_t SHP[2U];                /*!< Offset: 0x01C (R/W)  System Handlers Priority Registers. [0] is RESERVED */
	volatile uint32_t SHCSR;                  /*!< Offset: 0x024 (R/W)  System Handler Control and State Register */
 } SCB_Type;

#define SysTick_BASE        (0xE000E000UL +  0x0010UL)
#define SCB_BASE            (0xE000E000UL +  0x0D00UL)

#define SCB                 ((SCB_Type       *)     SCB_BASE      )   /*!< SCB configuration struct           */
#define SysTick             ((SysTick_Type   *)     SysTick_BASE  )   /*!< SysTick configuration struct       */

/*
 *  todo add ICACHE and DACHE macros.
 */


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


typedef struct ctx_   ArmCm0RegList_t;
typedef struct ctx_ * ArmCm0RegListPtr_t;


#endif
#endif
