/*
 * uart-irqs.h
 *
 *  Created on: Oct 8, 2022
 *      Author: ogruber
 */

#ifndef UART_IRQS_H_
#define UART_IRQS_H_

/*
 * UARTICR: Interrupt Clear Register (0x044)
 *     write-only, 11 lower bits
 *     write HIGH bit clears the corresponding interrupt
 *
 * 15:11: Reserved Reserved, read as zero, do not modify.
 * 10: OEIC Overrun error interrupt clear. Clears the UARTOEINTR interrupt.
 *  9: BEIC Break error interrupt clear. Clears the UARTBEINTR interrupt.
 *  8: PEIC Parity error interrupt clear. Clears the UARTPEINTR interrupt.
 *  7: FEIC Framing error interrupt clear. Clears the UARTFEINTR interrupt.
 *  6: RTIC Receive timeout interrupt clear. Clears the UARTRTINTR interrupt.
 *  5: TXIC Transmit interrupt clear. Clears the UARTTXINTR interrupt.
 *  4: RXIC Receive interrupt clear. Clears the UARTRXINTR interrupt.
 *  3: DSRMIC nUARTDSR modem interrupt clear. Clears the UARTDSRINTR interrupt.
 *  2: DCDMIC nUARTDCD modem interrupt clear. Clears the UARTDCDINTR interrupt.
 *  1: CTSMIC nUARTCTS modem interrupt clear. Clears the UARTCTSINTR interrupt.
 *  0: RIMIC nUARTRI modem interrupt clear. Clears the UARTRIINTR interrupt.
 */
#define UART_ICR 0x44

#define UART_ICR_OEIC (1<<10)
#define UART_ICR_BEIC (1<<9)
#define UART_ICR_PEIC (1<<8)
#define UART_ICR_FEIC (1<<7)
#define UART_ICR_RTIC (1<<6)
#define UART_ICR_TXIC (1<<5)
#define UART_ICR_RXIC (1<<4)
#define UART_ICR_DSRMIC (1<<3)
#define UART_ICR_DCDMIC (1<<2)
#define UART_ICR_CTSMIC (1<<1)
#define UART_ICR_RIMIC (1<<0)

/*
 * PrimeCell UART (PL011) Technical Reference Manual
 *   https://developer.arm.com/documentation/ddi0183/g/
 */

/*
 * UART Line Control Register (UARTLCR)
 * 0x02C UARTLCR_H (R/W) reset value=0x00 width=8
 * Line Control Register, UARTLCR_H on page 3-12
 */
#define CUARTLCR_H 0x02C

#define CUARTLCR_H_SPS (1<<7) // Stick parity select.
#define CUARTLCR_H_WLEN8 (0x03<<5)  // Word length.
#define CUARTLCR_H_WLEN7 (0x02<<5)  // Word length.
#define CUARTLCR_H_WLEN6 (0x01<<5)  // Word length.
#define CUARTLCR_H_WLEN5 (0x00<<5)  // Word length.

#define CUARTLCR_H_FEN (1<<4)  // Enable FIFOs: 0 = FIFOs are disabled

#define CUARTLCR_H_STP2 (1<<3)  // Two stop bits select
#define CUARTLCR_H_EPS (1<<2)  // Even parity select, 0 = odd parity.
#define CUARTLCR_H_PEN (1<<1)  // Parity Enable, 0 = parity is disabled.
#define CUARTLCR_H_BRK (1<<0)  // Send break.

/*
 * Page 63 - Section 3.3.10
 * IMSC: Interrupt Mask Set/Clear
 * It is a read/write register.
 * On a read this register returns the current value
 * of the mask on the relevant interrupt.
 * On a write of 1 to the particular bit,
 * it sets the corresponding mask of that interrupt.
 * A write of 0 clears the corresponding mask.
 */
#define UART_IMSC 0x038

/* Receive interrupt mask
 * The receive interrupt changes state when one of the following events occurs:
 *   - If the FIFOs are enabled and the receive FIFO reaches the programmed
 *     trigger level. When this happens, the receive interrupt is asserted HIGH.
 *     The receive interrupt is cleared by reading data from the receive FIFO
 *     until it becomes less than the trigger level, or by clearing the
 *     interrupt.
 *   - If the FIFOs are disabled (have a depth of one location) and data is
 *     received thereby filling the location, the receive interrupt is asserted
 *     HIGH. The receive interrupt is cleared by performing a single read of
 *     the receive FIFO, or by clearing the interrupt
 */
#define UART_IMSC_RXIM (1<<4)

/*
 * Tx interrupt mask
 *
 * The transmit interrupt changes state when one of the following events occurs:
 *   - If the FIFOs are enabled and the transmit FIFO is equal to or lower
 *     than the programmed trigger level then the transmit interrupt is
 *     asserted HIGH. The transmit interrupt is cleared by writing data
 *     to the transmit FIFO until it becomes greater than the trigger level,
 *     or by clearing the interrupt.
 *   - If the FIFOs are disabled (have a depth of one location) and there is
 *     no data present in the transmitters single location, the transmit
 *     interrupt is asserted HIGH. It is cleared by performing a single write
 *     to the transmit FIFO, or by clearing the interrupt.
 * To update the transmit FIFO you must write data to the transmit FIFO,
 * either prior to enabling the UART and the interrupts, or after enabling
 * the UART and interrupts.
 * Note Bene:
 *   The transmit interrupt is based on a transition through a level, rather
 *   than on the level itself. When the interrupt and the UART is enabled
 *   before any data is written to the transmit FIFO the interrupt is not set.
 *   The interrupt is only set, after written data leaves the single location
 *   of the transmit FIFO and it becomes empty
 */
#define UART_IMSC_TXIM (1<<5)

/*
 * Received timeout mask
 *
 * The receive timeout interrupt is asserted when the receive FIFO is not empty,
 * and no more data is received during a 32-bit period. The receive timeout
 * interrupt is cleared either when the FIFO becomes empty through reading
 * all the data or by clearing the interrupt in the Interrupt Clear Register,
 * UARTICR on page 3-21.
 */
#define UART_IMSC_RTIM (1<<6)

/*
 * The UARTMIS Register is the masked interrupt status register.
 * It is a read-only register. This register returns the current
 * masked status value of the corresponding interrupt. A write has no effect.
 */
#define UART_MIS 0x040
/*
 * Receive masked interrupt status.
 * Returns the masked interrupt state of the UARTRXINTR interrupt
 */
#define UART_MIS_RXMIS (1<<4)
/*
 * Transmit masked interrupt status.
 * Returns the masked interrupt state of the UARTTXINTR interrupt.
 */
#define UART_MIS_TXMIS (1<<5)
/*
 * Receive timeout masked interrupt status.
 * Returns the masked interrupt state of the UARTRTINTR interrupt.
 */
#define UART_MIS_RTMIS (1<<6)


#endif /* UART_IRQS_H_ */
