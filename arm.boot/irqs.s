/*
 * _isr.s
 *
 *  Created on: Oct 8, 2022
 *      Author: ogruber
 */

 /* Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs */

    .equ    CPSR_USR_MODE,       0x10
    .equ    CPSR_FIQ_MODE,       0x11
    .equ    CPSR_IRQ_MODE,       0x12
    .equ    CPSR_SVC_MODE,       0x13
    .equ    CPSR_ABT_MODE,       0x17
    .equ    CPSR_UND_MODE,       0x1B
    .equ    CPSR_SYS_MODE,       0x1F

    .equ    CPSR_IRQ_FLAG,         0x80      /* when set, IRQs are disabled, at the core level */
    .equ    CPSR_FIQ_FLAG,         0x40      /* when set, FIQs are disabled, at the core level */

/*
 * Initial setup for handling interrupts on the processor
 * It is about setting up the stack for the interrupt mode.
 */
.global _irqs_setup
	.func _irqs_setup
_irqs_setup:
    /* get Program Status Register */
    mrs r0, cpsr
    /* go in IRQ mode */
    bic r1, r0, #CPSR_SYS_MODE  /* 0x1F */
    orr r1, r1, #CPSR_IRQ_MODE  /* 0x12 */
    msr cpsr, r1
    /* set the pre-allocated IRQ stack */
    ldr sp, =irq_stack_top
    /* go back to the mode the processor was in
     * when this function was called, normally,
     * it should be the Supervisor mode */
    msr cpsr, r0
    mov pc,lr
    .size   _irqs_setup, . - _irqs_setup
	.endfunc

/*
 * Enable all interrupts at the processor.
 */
.global _irqs_enable
	.func _irqs_enable
_irqs_enable:
    /* get Program Status Register */
    mrs r0, cpsr
    /* Enable IRQs on the processor by clearing the mask bit
     * The BIC (BIt Clear) instruction performs an AND operation
     * with the complements of the given contants #0x80.
     */
    bic r0, r0, #CPSR_IRQ_FLAG /*0x80*/
    /* set Program Status Register */
    msr cpsr, r0
    mov pc,lr
    .size   _irqs_enable, . - _irqs_enable
	.endfunc

/*
 * Disable all interrupts at the processor and
 * at the processor only, the VIC is still managing
 * interrupts. When the interrupts will be turned back
 * on, the VIC will interrupt the processor if they are
 * pending interrupts.
 */
.global _irqs_disable
	.func _irqs_disable
_irqs_disable:
    /* get Program Status Register */
    mrs r0, cpsr
    /* Disable IRQs on the processor */
    orr r0, r0, #CPSR_IRQ_FLAG /*0x80*/
    /* set Program Status Register */
    msr cpsr, r0
    mov pc,lr
    .size   _irqs_disable, . - _irqs_disable
	.endfunc

.global _interrupt_service_routine
    .func _interrupt_service_routine
_interrupt_service_routine:
    sub lr,lr,#4
    stmfd sp!, {r0-r12, lr} 
    bl uart_interrupt_handler
    ldmfd sp!, {r0-r12, pc}^ 
    .endfunc
    
/*
 * ARM11 MPCore Processor Technical Reference Manual
 * Page 159, Control Coprocessor CP15, Wait for interrupt
 *
 * (1) Puts the ARM into a low power state and stops it executing further until
 *  an interrupt, or a debug request, occurs. Interrupt and debug events
 *  always cause the ARM processor to restart, irrespective of whether
 *  the interrupt is masked.
 *  (2) When an interrupt does occur, the MCR instruction completes and either
 *  the next instruction executes (if an interrupt event and the interrupt is
 *  masked), or the IRQ or FIQ handler is entered as normal. The return link
 *  in R14_irq or R14_fiq contains the address of the MCR instruction plus
 *  8, so that the normal instruction used for interrupt return is LR-4
 *  returns to the instruction following the MCR.
 */
 #ifdef BOARD_VERSATILEAB
.global _wfi_arm11
	.func _wfi_arm11
_wfi_arm11:
	MCR p15,0,r0,c7,c0,4
	mov pc,lr
    .size   _wfi_arm11, . - _wfi_arm11
	.endfunc
#endif

#ifdef BOARD_VERSATILEPB
.global _wfi
	.func _wfi
_wfi:
	wfi
	mov pc,lr
    .size   _wfi, . - _wfi
	.endfunc
#endif

