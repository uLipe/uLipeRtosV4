@
@
@ 							ULIPE RTOS VERSION 4
@
@
@  \file OsPort.h
@
@  \brief this file is contains the header for functions of machine dependent code
@
@	In this file definitions about the stack frame, context switching procedures,
@ 	The user should not call these routines or use its data structures.
@
@   Author: FSN
@

@
@	execute this code in thumb mode:
@

#include "../../OsConfig.h"

#if (OS_ARCH_CORTEX_M0 == 1)


 		.thumb
		.syntax unified
@
@	extern variables:
@

		.extern currentTask
		.extern highPrioTask
		.extern osMainStk
		.extern osRunning
		.extern uLipeKernelRtosTick

@
@	make the routines visible outside this module
@

		.global uLipeEnterCritical
		.global uLipeExitCritical
		.global SVC_Handler
		.global PendSV_Handler
		.global SysTick_Handler
		.global uLipeMemCpy
		.global uLipeMemSet

@
@	place this code on text section:
@

		.section .text

@
@	uint32_t uLipeEnterCritical(void)
@
		.thumb_func
uLipeEnterCritical:
		mrs r0,	primask		@pushes the status register
		cpsid I				@shut down interrupts
		bx	lr				@

@
@	void uLipeExitCritical(uint32_t sReg)
@
		.thumb_func
uLipeExitCritical:
		msr	primask, r0		@pops the status register & interrupts
		bx	lr				@

@
@   void uLipeMemCpy(void *dest, void *src, size_t size)
@
uLipeMemCpy:

		bx lr


@
@   void uLipeMemCpy(void *dest, void *src, size_t size)
@
uLipeMemSet:

		bx lr


@
@	void uLipeFirstSwt(void)
@
		.thumb_func

SVC_Handler:
		ldr r0, =osMainStk  	@carrega variavel que vai guardar o stack principal
		movs r2, #4
		mov  r1, lr
		ands r1,r1,r2
		bne ___already_msp
		mrs r1, msp				@carrega o main stack pointer
___already_msp:
		subs  r1, #32
		str	r1,[r0]
		stmia r1!, {r4 - r7}	@ save the software context
	    mov  r4,r8
	    mov  r5,r9
	    mov  r6,r10
	    mov  r7,r11
	    stmia r1!, {r4 - r7}	@ save the software context

		movs r0, #0x02			@
		msr  control, r0		@ uses psp for now.
		isb						@ flushes the pipeline
		ldr r0, =highPrioTask	@
		ldr r1, =currentTask	@
		ldr r2, [r0]			@
		ldr r2, [r2]			@ takes the first task stack:

		ldmia r2!, {r4 - r7}	@ pops the first sw context
	    mov  r8,r4
	    mov  r9,r5
	    mov  r10,r6
	    mov  r11,r7
   	    ldmia r2!, {r4 - r7}
		msr psp, r2				@ the hardware deals with remaining context

		ldr r0, [r0]			@
		str r0, [r1]			@ the current task is the high prio task

		ldr   r0, =osRunning		@
		movs  r1, #1			@ os is running
		strb  r1, [r0]			@
		movs r2, #4
		mov r1, lr
		orrs r1,r1, r2          @
		mov lr, r1
		bx	lr					@

@
@	void uLipeCtxSwt(void)
@

		.thumb_func
PendSV_Handler:
		ldr r0, =highPrioTask	@
		ldr r1, =currentTask	@
		ldr r2, [r1]			@ takes the current task stack:
		mrs r3, psp				@ takes the current stack pointer
		subs r3, #32			@
	    str   r3, [r2]			@
		stmia r3!, {r4 - r7}	@ save the software context
	    mov  r4,r8
	    mov  r5,r9
	    mov  r6,r10
	    mov  r7,r11
	    stmia r3!, {r4 - r7}	@ save the software context

		ldr r2,[r0]				@
		ldr r2,[r2]				@ takes the high prio task stk pointer
	    adds r2, #16				@ takes first the high registers
	    ldmia r2!, {r4 - r7}	@ pops the  sw context
	    mov  r8,r4
	    mov  r9,r5
	    mov  r10,r6
	    mov  r11,r7
		msr psp, r2				@ the hardware deals with remaining context
		subs r2, #32				@ go back and restore the low registers
	    ldmia r2!, {r4 - r7}

		ldr r2, [r0]			@
		str r2, [r1]			@ the high prio task is the current task
		movs r2, #4
		mov r1, lr
		orrs r1,r1, r2           @
		mov lr, r1
		bx	lr					@ the return depennds of current task stack contents

@
@ the systick handler, invoke kernel tick routine
@
	.thumb_func
SysTick_Handler:
	push {lr}
	ldr r3, =uLipeKernelRtosTick
	blx r3
	pop {pc}
#endif

.end
