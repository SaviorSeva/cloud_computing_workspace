---------------------------------------------------
Overview
---------------------------------------------------

This is the second step of your hands-on homework.
With step1, you got acquainted with the project
structure (makefile and code) and how to run or
debug bare-metal code on QEMU. You also had to
write code for a simple console over the serial line,
helping you fully understand of the serial line 
works.

This second step is about introducing the use of
interrupts for that serial line, both RX and TX 
interrupts. The device is the PrimeCell PL011 
UART and the documentation is the following:

  PrimeCell PL011 Technical Reference Manual 
  (ARM DDI 0183G)
  
The home work will be composed of two parts,
the first part is mandatory, the second is optional.
You will find the optional part at the end of this
document. The mandatory part starts just below.

---------------------------------------------------
Provided Code
---------------------------------------------------

We provide you with some code, source and header 
files, that are the hardware-dependent parts.
As usual, you are asked to look and understand 
the provided code.

First, there is the generic code to be able
to turn interrupts on and off:

  - irqs.S
  - vic.c and vic.h
  
Then, there is the header file for the UART-specific 
defines, translated from the PL011 documentation:
  
  - uart-irqs.h
  
Finally, there is a lock-free implementation
of a circular buffer for bytes

  - cb.c and cb.h


---------------------------------------------------
Design Points
---------------------------------------------------

The main program will not change much:

  uart_send_string(0, "\nHello world!\n");

  uint8_t byte;
  for (;;) {
    while (0 == uart_receive(0, &byte)) {
      if (byte == 13) {
        uart_send(0, '\r');
        uart_send(0, '\n');
      } else {
        uart_send(0, byte);
      }
    }
    wfi();
  }

Except for the use of the call to "wfi()" that halts the core
and Waits For Interrupts. Of course, this is only an example,
your code will be different because of all the work you did
previously.

The function "uart_receive" will pull bytes from a circular 
buffer, called "rxcb", that the handler of the RX interrupt
will fill with the available bytes from the RX FIFO.

The function "uart_send" will push bytes in another circula buffer,
called "txcb", that the handler of the TX interrupt will 
empty, writing the bytes to the TX FIFO, when there is room to do so.

The difficulty is to understand the UART programming model with 
interrupts. All the necessary information is available in the PL011
documentation:

  PrimeCell PL011 Technical Reference Manual 
  (ARM DDI 0183G)

Always refer to that documentation for details, especially the 
Section 2.8 on interrupts and Section 3.2 and 3.3 for the details
about the mmio registers.

But because the documentation style may be difficult for you at
first, we will provide you with key points below. Try to find 
them in the documentation so that you get a feel for how such
documentations are organized.

The first essential point is that the UART behaves radically 
differently if the FIFO queues are enabled or not. So let's make
sure they are enabled:

  // Enable FIFO queues, both rx-queue and tx-queue.
  uint16_t lcr = *(uint16_t*) (uart->bar + CUARTLCR_H);
  lcr |= CUARTLCR_H_FEN;
  *(uint16_t*) (uart->bar + CUARTLCR_H) = lcr;
  
This code goes in the initialization of the UART, just before
enabling the interrupts at the UART level. The UART provide 
three interrupts that we are interested in: RX, RT, and TX. 
See UART_IMSC_RXIM, UART_IMSC_RTIM, and UART_IMSC_TXIM 
for the UART_IMSC, that is, the Interrupt Mask Set/Clear (IMSC)
register on Page 63 - Section 3.3.10.

The RX and TX interrupts work with trigger levels, controlled by the
Interrupt FIFO Level Select Register (UARTIFLS). By default, the trigger
levels are at 1/8 of the FIFO capacity. You can read the details in the
Section 3.3.9 of the PL011 technical reference manual (PL011 TRM). 

The receive interrupt (RX) works as follow when the FIFO queues are
enabled. When the receive FIFO (RX) reaches the programmed trigger level,
the receive interrupt is asserted HIGH. The receive interrupt is cleared 
by reading data from the receive FIFO until it becomes less than the trigger 
level, or by clearing the interrupt.

This means that it is possible that there are some bytes in the RX FIFO 
queue without the RX interrupt being asserted high. This is when the 
Receive Timeout (RT) interrupt comes in. 

The RT interrupt is asserted when the receive FIFO is not empty and no
more data is received during a 32-bit period. The RT interrupt is cleared
either when the FIFO becomes empty through reading all the data or by clearing
the interrupt in the Interrupt Clear Register, UARTICR on page 3-21.

The transmit interrupt (TX), with the FIFO enabled, changes state when 
the transmit FIFO "fullness" becomes equal to or lower than the programmed 
trigger level then the transmit interrupt is asserted HIGH. The transmit 
interrupt is cleared by writing data to the transmit FIFO until it becomes 
greater than the trigger level or by clearing the interrupt.

In other words, the transmit interrupt is based on a transition through 
a level, rather than on the level itself. When the interrupt and the UART 
is enabled before any data is written to the transmit FIFO the interrupt 
is not set. The interrupt is only set, after written data leaves the TX FIFO 
and the programmed trigger level is crossed. 

---------------------------------------------------
Optional Part
---------------------------------------------------

The optional part is to add an event-oriented programming
model, with a top-bottom framework for handling interrupts.
Start without delayed events and only then add delayed events
because delayed events require the use of a hardware timer. 






