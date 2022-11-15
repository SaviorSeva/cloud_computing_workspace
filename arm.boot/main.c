#include "main.h"
#include "kprintf.c" 

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

int increment_history_index(){
  current_index++;
  if(current_index >= MAX_HISTORY) current_index -= MAX_HISTORY;
  return 1;
}

int current_index_offset(int offset){
  int res = current_index + offset;
  if(res >= MAX_HISTORY) res -= MAX_HISTORY;
  else if(res < 0) res += MAX_HISTORY;
  return res;
}

int add_line_to_history(char* line){
  for(int i=0; i<MAX_LINE_LENGTH; i++){
    command_history[current_index][i] = line[i];
  }

  return increment_history_index();
}

int clear_line(char* line){
  for(int i=0; i<MAX_LINE_LENGTH; i++){
    line[i] = 0;
  }
  return 1;
}

void treat_line(char* line){
  add_line_to_history(line);
}

void del_process(){

}



int send_arrow(int direction){
  if (direction > 4 || direction <= 0) return 0;

  uart_send(UART0, '\033');
  uart_send(UART0, '[');
  // uart_send(UART0, 'D');
  switch (direction)
  {
  case 1:   // Up
    uart_send(UART0, 'A');
    break;
  case 2:   // Down
    uart_send(UART0, 'B');
    break;
  case 3:   // Left
    uart_send(UART0, 'D');
    break;
  case 4:   // Right
    uart_send(UART0, 'C');
    break;
  default:
    break;
  }
}

void erase_current_line(int *cursor_pos, char *line){
  clear_line(line);
  while(*cursor_pos != 0){
    uart_send(UART0, ' ');
    send_arrow(4);
    *cursor_pos--;
  }
}

void show_previous_line(int *cursor_pos, char *line){
  erase_current_line(cursor_pos, line);

}

void backspace_process(int *cursor_pos, char *line){
  *cursor_pos = *cursor_pos - 1;
  send_arrow(3);
  uart_send(UART0, ' ');
  send_arrow(3);
  line[*cursor_pos] = 0;
}

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

/**
 * This is the C entry point, upcalled once the hardware has been setup properly
 * in assembly language, see the reset.s file.
 */

void _start() {
  int i = 0;
  int count = 0;
  int variableL = 15;
  char line[MAX_LINE_LENGTH];
  int cursor_pos = 0;
  int arrow_status = 0;
  int del_status = 0;

  uart_send_string(UART0, "\nQuit with \"C-a c\" and then type in \"quit\".\n");
  uart_send_string(UART0, "\nHello world!\n");

  while (1) {
    unsigned char c;
    if (0 != uart_receive(UART0, &c)){
      // send_binary(c);
      
      if(c == '\033'){
        arrow_status = 1;
        del_status = 1;
        if (0 != uart_receive(UART0, &c)){}         // Skip the '['
        if (0 != uart_receive(UART0, &c)){
          // Possible arrow / del input
          switch (c)
          {
          case 'A': // Up
            show_previous_line(&cursor_pos, line);
            break;
          case 'B': // Down
            
            break;
          case 'C': // Right
            send_arrow(4);
            break;
          case 'D': // Left
            send_arrow(3);
            break;
          case '3':  // Delete
            if (0 != uart_receive(UART0, &c)){
              if (c == '~') del_process();
            }
            break;
          }
          c = '\000';
        }
        
      }

      // Backspace
      if(c == 127) backspace_process(&cursor_pos, line);

      // Normal characters
      if(c >= 32 && c <= 126){
        line[cursor_pos] = c;
        uart_send(UART0, c);
        cursor_pos++;
      }

      // Enter
      else if(c == '\r'){
        add_line_to_history(line);
        clear_line(line);
        cursor_pos = 0;
      }
    }
  }
}
