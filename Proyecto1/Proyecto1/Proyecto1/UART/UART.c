/*
 * UART.c
 *
 * Created: 29/01/2026 19:55:29
 *  Author: samur
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "UART.h"

#define BAUD 9600
#define UBRR_VAL ((F_CPU / 16 / BAUD) - 1)

volatile char uart_buffer[UART_BUFFER_SIZE];
volatile uint8_t uart_buffer_index = 0;

void UART_Init(void)
{
	UBRR0H = (uint8_t)(UBRR_VAL >> 8);
	UBRR0L = (uint8_t)UBRR_VAL;
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
}

void UART_SendChar(char c)
{
	while (!(UCSR0A & (1 << UDRE0)));
	UDR0 = c;
}

void UART_SendString(const char *str)
{
	while (*str)
	UART_SendChar(*str++);
}

ISR(USART_RX_vect)
{
	char c = UDR0;
	uint8_t oldSREG = SREG;  //Guarda estado de interrupciones
	
	if (uart_buffer_index < UART_BUFFER_SIZE - 1) {
		uart_buffer[uart_buffer_index++] = c;
	}
	
	SREG = oldSREG;
}

char UART_GetChar(void)
{
	char c = 0;
	uint8_t oldSREG = SREG;  //Guarda estado de interrupciones
	cli();
	
	if (uart_buffer_index > 0) {
		c = uart_buffer[0];
		//Pra desplazar el buffer
		for (uint8_t i = 1; i < uart_buffer_index; i++) {
			uart_buffer[i-1] = uart_buffer[i];
		}
		uart_buffer_index--;
	}
	
	SREG = oldSREG;
	return c;
}
//Uart sin bloquear
uint8_t UART_ReadLine_NonBlocking(char *out, uint8_t maxLen)
{
	static uint8_t idx = 0;

	while (1)
	{
		char c = UART_GetChar();
		if (c == 0) return 0;

		if (c == '\r') continue;

		if (c == '\n')
		{
			out[idx] = '\0';
			idx = 0;
			return 1;
		}

		if (idx < (maxLen - 1))
		{
			out[idx++] = c;
		}
	}
}