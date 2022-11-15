#include "main.h"
#include "kprintf.c" 

// Declare some global variables
int variableG1 = 0;
int variableG2 = 1024;
int variableG3;

/**
 * This is the C entry point, upcalled once the hardware has been setup properly
 * in assembly language, see the reset.s file.
 */

void send_binary(char c)
{ 
  char bin[9] = {0}, bck = c;
  for ( int i = 7; i >= 0; i-- ){
        bin[i] = (c % 2) + '0';
        c /= 2;
  }
  c = bck;
  kprintf("\n\r Input :");
  kputchar(c);
  kprintf(", binary is ");
  kprintf(bin);
  kprintf("\n\r");
}
 

void _start() {
  int i = 0;
  int count = 0;
  int variableL = 15;

  uart_send_string(UART0, "\nQuit with \"C-a c\" and then type in \"quit\".\n");
  uart_send_string(UART0, "\nHello world!\n");

  while (1) {
    unsigned char c;
    while (0 == uart_receive(UART0, &c)) {
      // friendly reminder that you are polling and therefore spinning...
      // not good for the planet! But until we introduce interrupts,
      // there is nothing you can do about it... except comment out
      // this annoying code ;-)
      // count++;

      // if (count > 10000000) {
      //   uart_send_string(UART0, "\n\rZzzz.... from UART0\n\r");
      //   count = 0;
      // }
    }
    if (c == '\r')
      uart_send(UART0, '\n');
    
    uart_send(UART0, c);
    // uart_send(UART1, c);
    send_binary(c);
  }
}
