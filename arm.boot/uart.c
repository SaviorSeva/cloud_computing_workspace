#include "main.h"
#include "uart-irqs.h"
#include "uart.h"
#include "cb.h"

struct cb rxcb;
struct cb txcb;


int uart_enable_fifo(int uart)
{
	uint16_t lcr = *(uint16_t *)(uart + CUARTLCR_H);
	lcr |= CUARTLCR_H_FEN;
	*(uint16_t *)(uart + CUARTLCR_H) = lcr;
}

/**
 * Receive a character from the given uart, this is a non-blocking call.
 * Returns 0 if there are no character available.
 * Returns 1 if a character was read.
 */
// int uart_receive(int uart, unsigned char *s) {
//   unsigned short* uart_fr = (unsigned short*) (uart + UART_FR);
//   unsigned short* uart_dr = (unsigned short*) (uart + UART_DR);
//   if (*uart_fr & UART_RXFE)
//     return 0;
//   *s = (*uart_dr & 0xff);
//   return 1;
// }

int uart_receive(int uart, unsigned char *s)
{
	char c;
	int i = cb_get(&rxcb, c);
	if(i == -1) return -1;
	return 0;
}

void uart_rx_interrupt_handler(void *uart)
{
	unsigned short *uart_fr = (unsigned short *)(uart + UART_FR);
	unsigned short *uart_dr = (unsigned short *)(uart + UART_DR);
	if (*uart_fr & UART_RXFE)
		return 0;
	char s = *uart_dr & 0xff;
	return cb_put(&rxcb, s);
}

int uart_rt_interrupt_handler(int uart)
{
	unsigned short *uart_fr = (unsigned short *)(uart + UART_FR);
	unsigned short *uart_dr = (unsigned short *)(uart + UART_DR);
  	while (*uart_fr & UART_RXFE != 1){

 	}
}

/**
 * Sends a character through the given uart, this is a blocking call.
 * The code spins until there is room in the UART TX FIFO queue to send
 * the character.
 */
// void uart_send(int uart, unsigned char s) {
//   unsigned short* uart_fr = (unsigned short*) (uart + UART_FR);
//   unsigned short* uart_dr = (unsigned short*) (uart + UART_DR);
//   while (*uart_fr & UART_TXFF)
//     ;
//   *uart_dr = s;
// }

void uart_send(int uart, unsigned char s)
{
	cb_put(&txcb, s);
}

int uart_tx_interrupt_handler(void* uart)
{
	unsigned short *uart_fr = (unsigned short *)(uart + UART_FR);
	unsigned short *uart_dr = (unsigned short *)(uart + UART_DR);
	while (*uart_fr & UART_TXFF)
		;
	return cb_get(&txcb, uart_dr);
}

void uart_interrupt_handler(uint8_t *uart){
	if(*(uart + UART_IMSC) & UART_IMSC_RXIM) 
		uart_rx_interrupt_handler(uart);
	else if(*(uart + UART_IMSC) & UART_IMSC_TXIM)
		uart_tx_interrupt_handler(uart);
	else if(*(uart + UART_IMSC) & UART_IMSC_RTIM)
		uart_rt_interrupt_handler(uart);
}

/**
 * This is a wrapper function, provided for simplicity,
 * it sends a C string through the given uart.
 */
// void uart_send_string(int uart, const unsigned char *s) {
//   while (*s != '\0') {
//     uart_send(uart, *s);
//     s++;
//   }
// }
