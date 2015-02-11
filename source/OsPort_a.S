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
 		.thumb
		.syntax unified
@
@	extern variables:
@

		.extern currentTask
		.extern highPrioTask
		.extern osMainStk
		.extern osRunning

@
@	make the routines visible outside this module
@

		.global uLipeEnterCritical
		.global uLipeExitCritical
		.global uLipeKernelFindHighPrio
		.global uLipeFirstSwt
		.global uLipeCtxSwt

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
@	uint16_t uLipeKernelFindHighPrio(OsPrioListPtr_t prioList)
@
		.thumb_func
uLipeKernelFindHighPrio:
		ldr  r1, [r0], #4  		@load prio group
		movs r2, #31	   		@
		clz  r1, r1		   		@
		subs r1,r2,r1	  		@
		lsl  r1, #2		   		@calculates the correct prio displacement
		ldr  r3, [r0, r1]  		@
		clz  r3,r3		   		@
		subs r3, r2,r3	   		@
		add  r0, r3, r1, lsl #3 @calculates prio where: prio = r3 + (r1 << 5)
		bx 	 lr				 	@

@
@	void uLipeFirstSwt(void)
@
		.thumb_func
uLipeFirstSwt:

		ldr r0, =osMainStk  	@carrega variavel que vai guardar o stack principal
		tst	lr, #4
		it  EQ
		mrseq r1, msp			@carrega o main stack pointer
		stmdb r1!,{r4 - r11}	@salva registros que o hardware nao guarda
		str	r1,[r0]

		movs r0, #0x02			@
		msr  control, r0		@ uses psp for now.
		isb						@ flushes the pipeline
		ldr r0, =highPrioTask	@
		ldr r1, =currentTask	@
		ldr r2, [r0]			@
		ldr r2, [r2]			@ takes the first task stack:
		ldmia r2!, {r4 - r11}	@ pops the first sw context
		msr	  psp, r2			@ the remainning context is dealt by hardware

		ldr r0, [r0]			@
		str r0, [r1]			@ the current task is the high prio task

		ldr   r0, =osRunning		@
		movs  r1, #0xFF			@ os is running
		strb  r1, [r0]			@
		orr   lr,lr, #0x04		@ensures the correct EXC_RETURN

		bx	lr					@

@
@	void uLipeCtxSwt(void)
@

		.thumb_func
uLipeCtxSwt:
		ldr r0, =highPrioTask	@
		ldr r1, =currentTask	@
		ldr r2, [r1]			@ takes the current task stack:
		mrs r3, psp				@ takes the current stack pointer
		stmdb r3!, {r4 - r11}	@ save the software context
		str   r3, [r2]			@

		ldr r2,[r0]				@
		ldr r2,[r2]				@ takes the high prio task stk pointer
		ldmia r2!, {r4-r11}		@ pops  the new software saved context
		msr psp, r2				@ the hardware deals with remaining context

		ldr r2, [r0]			@
		str r2, [r1]			@ the high prio task is the current task
		orr lr,lr, #0x04        @
		bx	lr					@ the return depennds of current task stack contents




.end
