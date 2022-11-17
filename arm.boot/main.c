#include "main.h"
#include "kprintf.c"
#include "vic.h"
#include "uart-irqs.h"
#include "cb.h"
#include "uart.h"

// Declare some global variables to see where they store at.
int variableG1 = 0;
int variableG2 = 1024;
int variableG3;

extern void _halt();
extern void _reset();

// Define the things for IRQ
// const int MAX_NHANDLERS = 5;

// typedef struct {
//     void (*callback)(void*);
//     void* cookie;
// } handler_t;

// handler_t handlers[MAX_NHANDLERS];

// void interrupt_service_routine() {
//     int irqs = gic_load_irqs();
//     for (int i=0;i<32;i++) {
//     handler_t* handler = &handlers[i];
//     if (i & (1<<i))
//         handler->callback(handler->cookie);
//     }
//     gic_ack_irqs(irqs);
//     return;
// }

// Declare the place to store history commands.
const int MAX_HISTORY = 20;
const int MAX_LINE_LENGTH = 80;
char command_history[20][80];
int current_index = 0;
int avail_history = 0;

int increment_history_index()
{
	current_index++;
	if (current_index >= MAX_HISTORY)
		current_index -= MAX_HISTORY;
	return 1;
}

int current_index_offset(int offset)
{
	int res = current_index + offset;
	if (res >= MAX_HISTORY)
		res -= MAX_HISTORY;
	else if (res < 0)
		res += MAX_HISTORY;
	return res;
}

int add_line_to_history(char *line)
{
	for (int i = 0; i < MAX_LINE_LENGTH; i++)
	{
		command_history[current_index][i] = line[i];
	}

	return increment_history_index();
}

int clear_line(char *line)
{
	for (int i = 0; i < MAX_LINE_LENGTH; i++)
	{
		line[i] = 0;
	}
	return 1;
}

int shift_line_left(int index, char *line)
{
	for (index; index < MAX_LINE_LENGTH; index++)
	{
		line[index - 1] = line[index];
	}
	return 1;
}

void command_echo(int line_length, char *line)
{
	uart_send(UART0, '\n');
	uart_send(UART0, '\r');
	for (int i = 5; i < line_length; i++)
	{
		uart_send(UART0, line[i]);
	}
	// uart_send(UART0, '\n');
}

void command_reset()
{
	for (int i = 0; i < 50; i++)
		send_arrow(1);
	uart_send(UART0, '\r');
	for (int i = 0; i < 50; i++)
	{
		for (int i = 0; i < MAX_LINE_LENGTH; i++)
			uart_send(UART0, ' ');
		send_arrow(2);
	}

	for (int i = 0; i < 50; i++)
		send_arrow(1);
	uart_send(UART0, '\r');
}

int send_arrow(int direction)
{
	if (direction > 4 || direction <= 0)
		return 0;

	uart_send(UART0, '\033');
	uart_send(UART0, '[');
	// uart_send(UART0, 'D');
	switch (direction)
	{
	case 1: // Up
		uart_send(UART0, 'A');
		break;
	case 2: // Down
		uart_send(UART0, 'B');
		break;
	case 3: // Left
		uart_send(UART0, 'D');
		break;
	case 4: // Right
		uart_send(UART0, 'C');
		break;
	default:
		break;
	}
}

void erase_current_line(int *cursor_pos, char *line)
{
	clear_line(line);
	while (*cursor_pos != 0)
	{
		send_arrow(3);
		uart_send(UART0, ' ');
		send_arrow(3);
		*cursor_pos = *cursor_pos - 1;
	}
}

void show_line(int *cursor_pos, int line_last_index, char *line)
{
	for (int i = 0; i < line_last_index; i++)
	{
		uart_send(UART0, line[i]);
		*cursor_pos = *cursor_pos + 1;
	}
}

void refresh_line(int *cursor_pos, int line_last_index, char *line)
{
	int original_cursor_pos = *cursor_pos;
	uart_send(UART0, '\r');
	for (int i = 0; i < line_last_index; i++)
	{
		uart_send(UART0, ' ');
	}
	uart_send(UART0, '\r');
	show_line(cursor_pos, line_last_index, line);
}

void reposition_cursor(int at)
{
	uart_send(UART0, '\r');
	for (int i = 0; i < at; i++)
	{
		send_arrow(4);
	}
}

void show_previous_line(int *cursor_pos, int history_offset, int *line_last_index, char *line)
{
	erase_current_line(cursor_pos, line);
	int i = current_index_offset(history_offset);
	int l = 0;
	while (command_history[i][l] != '\r')
	{
		line[l] = command_history[i][l];
		*cursor_pos = *cursor_pos + 1;
		*line_last_index = *line_last_index + 1;
		uart_send(UART0, command_history[i][l]);
		l++;
	}
}

void backspace_process(int *cursor_pos, int *line_last_index, char *line)
{
	int original_cursor_pos = *cursor_pos;
	shift_line_left(*cursor_pos, line);
	refresh_line(cursor_pos, *line_last_index, line);
	*cursor_pos = original_cursor_pos - 1;
	reposition_cursor(*cursor_pos);
}

void del_process(int *cursor_pos, int *line_last_index, char *line)
{
	int original_cursor_pos = *cursor_pos;
	shift_line_left(*cursor_pos + 1, line);
	refresh_line(cursor_pos, *line_last_index, line);
	*cursor_pos = original_cursor_pos;
	reposition_cursor(*cursor_pos);
}

void treat_line(int line_length, char *line)
{
	add_line_to_history(line);
	if (line[0] == 'e' && line[1] == 'c' && line[2] == 'h' && line[3] == 'o' && line[4] == ' ')
		command_echo(line_length, line);
	if (line[0] == 'r' && line[1] == 'e' && line[2] == 's' && line[3] == 'e' && line[4] == 't' && line[5] == '\r')
		command_reset();
}

void send_binary(char c)
{
	char bin[9] = {0}, bck = c;
	for (int i = 7; i >= 0; i--)
	{
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

/**
 * This is the C entry point, upcalled once the hardware has been setup properly
 * in assembly language, see the reset.s file.
 */

void _start()
{
	int i = 0;
	int count = 0;
	int variableL = 15;
	char line[MAX_LINE_LENGTH];
	int line_last_index = 0;
	int cursor_pos = 0;
	int history_offset = 0;

	vic_setup();

	vic_enable(); // Enable vic interrupts

	uart_enable_fifo(UART0);

	for (int i = 0; i < 32; i++)
	{
		vic_irq_disable(i);
	}

	vic_irq_enable(UART0_IRQ, uart_interrupt_handler, (uint8_t *)UART0);
	// vic_irq_enable(1, uart_rx_handler, tx_cookie);

	command_reset();

	while (1)
	{
		unsigned char c;
		if (0 != uart_receive(UART0, &c))
		{
			// send_binary(c);

			if (c == '\033')
			{
				if (0 != uart_receive(UART0, &c))
				{
				} // Skip the '['
				if (0 != uart_receive(UART0, &c))
				{
					// Possible arrow / del input
					switch (c)
					{
					case 'A': // Up
						if (history_offset != -avail_history)
							history_offset--;
						show_previous_line(&cursor_pos, history_offset, &line_last_index, line);

						break;
					case 'B': // Down
						history_offset++;
						if (history_offset == 0)
						{
							erase_current_line(&cursor_pos, line);
							cursor_pos = 0;
							line_last_index = 0;
						}
						else if (history_offset < 0)
						{
							show_previous_line(&cursor_pos, history_offset, &line_last_index, line);
						}
						else
							history_offset = 0;

						break;
					case 'C': // Right
						if (cursor_pos < line_last_index)
						{
							send_arrow(4);
							cursor_pos++;
						}
						break;
					case 'D': // Left
						if (cursor_pos >= 1)
						{
							send_arrow(3);
							cursor_pos--;
						}
						break;
					case '3': // Delete
						if (0 != uart_receive(UART0, &c))
						{
							if (c == '~')
								del_process(&cursor_pos, &line_last_index, line);
						}
						break;
					}
					c = '\000';
				}
				c = '\000';
			}

			// Backspace
			if (c == 127)
				backspace_process(&cursor_pos, &line_last_index, line);

			// Normal characters
			else if (c >= 32 && c <= 126)
			{
				line[cursor_pos] = c;
				uart_send(UART0, c);
				cursor_pos++;
				line_last_index++;
			}

			// Enter
			else if (c == '\r')
			{
				line[cursor_pos] = c;
				if (line[0] != '\r')
				{
					treat_line(line_last_index, line);
					kprintf(line);
					if (avail_history < 20)
						avail_history++;
				}
				clear_line(line);
				uart_send(UART0, '\n');
				cursor_pos = 0;
				line_last_index = 0;
				history_offset = 0;
			}
		}
	}

	wfi();
}
