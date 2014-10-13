;
;
; 							ULIPE RTOS VERSION 4
;
;
;  \file OsPort.h
;
;  \brief this file is contains the header for functions of machine dependent code
;
;	In this file definitions about the stack frame, context switching procedures,
; 	The user should not call these routines or use its data structures.
;
;   Author: FSN
;

;
;	execute this code in thumb mode:
;
 		.thumb
;
;	extern variables:
;

		.extern currentTask
		.extern highPrioTask

;
;	make the routines visible outside this module
;

		.global uLipeEnterCritical
		.global uLipeExitCritical
		.global uLipeFirstSwt
		.global uLipeCtxSwt

;
;	place this code on text section:
;

		.section .text

;
;	uint32_t uLipeEnterCritical(void)
;
uLipeEnterCritical:
		mrs r0,	primask		;pushes the status register
		cpsid I				;shut down interrupts
		bx	lr				;

;
;	void uLipeExitCritical(uint32_t sReg)
;
uLipeExitCritical:
		msr	primask, r0		;pops the status register & interrupts
		bx	lr				;

;
;	void uLipeFirstSwt(void)
;
uLipeFirstSwt:
		movs r0, #0x02			;
		msr  control, r0		; uses psp for now.
		isb						; flushes the pipeline
		ldr r0, =highPrioTask	;
		ldr r1, =currentTask	;
		ldr r2, [r0]			;
		ldr r2, [r2]			; takes the first task stack:
		ldmia r2!, {r4 - r11}	; pops the first sw context
		msr	  psp, r2			; the remainning context is dealt by hardware

		ldr r0, [r0]			;
		str r0, [r1]			; the current task is the high prio task
		bx	lr					;

;
;	void uLipeCtxSwt(void)
;
uLipeCtxSwt:
		ldr r0, =highPrioTask	;
		ldr r1, =currentTask	;
		ldr r2, [r1]			;
		ldr r2, [r2]			; takes the current task stack:
		mrs r2, psp				; takes the current stack pointer
		stmdb r2!, {r4 - r11}	; save the software context
		dsb						; flushes the pipeline

		ldr r2,[r0]				;
		ldr r2,[r2]				; takes the high prio task stk pointer
		ldmia r2!, {r4-r11}		; pops  the new software saved context
		msr psp, r2				; the hardware deals with remaining context

		ldr r2, [r0]			;
		str r0, [r1]			; the high prio task is the current task
		dsb						; flushes pipeline for data
		bx	lr					; the return depennds of current task stack contents




.end
