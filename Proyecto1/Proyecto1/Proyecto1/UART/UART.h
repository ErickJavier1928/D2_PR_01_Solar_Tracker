/*
 * UART.h
 *
 * Created: 29/01/2026 19:55:42
 *  Author: samur
 */ 


#ifndef UART_H_
#define UART_H_
#define UART_BUFFER_SIZE 16

extern volatile char uart_buffer[UART_BUFFER_SIZE];
extern volatile uint8_t uart_buffer_index;
void UART_Init(void);
void UART_SendChar(char c);
void UART_SendString(const char *str);
char UART_GetChar(void);
uint8_t UART_ReadLine_NonBlocking(char *out, uint8_t maxLen);
#endif /* UART_H_ */