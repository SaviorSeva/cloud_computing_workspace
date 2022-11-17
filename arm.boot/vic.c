/*
 * isr.c
 *
 *  Created on: Jan 21, 2021
 *      Author: ogruber
 */
#include "main.h"

/*
 * PrimeCell Vectored Interrupt Controller (PL190) Technical Reference Manual
 * https://developer.arm.com/documentation/ddi0183/latest/)
 */
#define VIC_BASE_ADDR 0x10140000
/*
 * Shows the status of the interrupts after masking by
 * the VICINTENABLE and VICINTSELECT Registers.
 * A HIGH bit indicates that the interrupt is active,
 * and generates an IRQ interrupt to the processor.
 */
#define VICIRQSTATUS 0x000
/*
 * Shows the status of the interrupts after masking by
 * the VICINTENABLE and VICINTSELECT Registers.
 * A HIGH bit indicates that the interrupt is active,
 * and generates an FIQ interrupt to the processor.
 */
#define VICFIQSTATUS 0x004
/*
 * Shows the status of the interrupts before masking by
 * the enable registers. A HIGH bit indicates that
 * the appropriate interrupt request is active before masking.
 */
#define VICRAWSTATUS 0x008
/*
 * [31:0] Selects the type of interrupt for interrupt requests:
 *   1 = FIQ interrupt
 *   0 = IRQ interrupt.
 */
#define VICINTSELECT 0x00C
/*
 * Enables the interrupt request lines:
 *   1 = Interrupt enabled. Enables interrupt request to processor.
 *   0 = Interrupt disabled.
 * On reset, all interrupts are disabled.
 * On writes, a HIGH bit sets the corresponding bit in
 * the VICINTENABLE Register, while a LOW bit has no effect.
 */
#define VICINTENABLE 0x010
/*
 * Clears bits in the VICINTENABLE Register.
 * On writes, a HIGH bit clears the corresponding bit in the
 * VICINTENABLE Register, while a LOW bit has no effect.
 */
#define VICINTENCLEAR 0x014

struct handler {
  void (*handler)(void* cookie);
  void *cookie;
};

static
struct handler handlers[32];

/*
 * This function is the upcall from the interrupt service
 * routine in assembly. It needs to figure out which
 * interrupt occurred and call the appropriate handler.
 */
void isr() {
  uint32_t irqs = *(uint32_t*)(VIC_BASE_ADDR+VICIRQSTATUS);
  for (int irq=0;irq<32;irq++) {
    struct handler* h = &handlers[irq];
    if ((irqs & (1<<irq)) && h->handler!=NULL) {
      h->handler(h->cookie);
    }
  }
}

/*
 * Assembly functions, see _isr.s
 */
extern void _irqs_setup(void);
extern void _irqs_enable(void);
extern void _irqs_disable(void);
extern void _wfi(void);


/*
 * The necessary setup at boot time.
 */
void vic_setup() {
  *(uint32_t*)(VIC_BASE_ADDR+VICINTENCLEAR) = ~0;
  _irqs_setup();
}

/*
 * Enable the IRQs as the processor level,
 * allowing the VIC to raise the interrupt
 * signal.
 */
void vic_enable() {
  _irqs_enable();
}

/*
 * Disable the IRQ as the processor level
 * preventing the VIC to raise the interrupt
 * signal.
 */
void vic_disable() {
  _irqs_disable();
}

/*
 * Wait for interrupts, halting the core.
 * This function will enable the VIC to raise
 * the interrupt signal. Therefore, this function
 * will only halt the core if there is no pending
 * interrupts.
 */
void wfi() {
#ifdef BOARD_VERSATILEAB
  void _wfi_arm11(void);
  _wfi_arm11();
#endif
#ifdef BOARD_VERSATILEPB
  void _wfi(void);
  _wfi();
#endif
}

/*
 * Enable the given IRQ at the VIC
 */
void vic_irq_enable(int irq,
    void (*handler)(void*),
    void *cookie) {
  struct handler* h = &handlers[irq];
  h->handler = handler;
  h->cookie = cookie;
  *(uint32_t*)(VIC_BASE_ADDR+VICINTENABLE) = (1<<irq);
}

/*
 * Disable the given IRQ at the VIC
 */
void vic_irq_disable(int irq) {
  *(uint32_t*)(VIC_BASE_ADDR+VICINTENCLEAR) = (1<<irq);
  struct handler* h = &handlers[irq];
  h->handler = NULL;
  h->cookie = NULL;
}
