/*
 * ADC.c
 *
 * Created: 29/01/2026 19:23:39
 *  Author: samur
 */ 
#include "ADC.h"
#include <avr/io.h>

void adc_iniciar(void) {
	ADMUX = (1 << REFS0) | (1 << ADLAR);  // Ajuste a izquierda para 8 bits
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
}

uint8_t adc_leer_8bits(uint8_t canal) {
	ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	
	return ADCH;  // Solo byte alto (8 bits)
}