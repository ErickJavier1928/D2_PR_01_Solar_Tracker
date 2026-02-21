/*
 * ADC.c
 *
 * Created: 29/01/2026 19:23:39
 *  Author: samur
 */ 
#include "ADC.h"
#include <avr/io.h>

void adc_iniciar(void) {
	ADMUX = (1 << REFS0);               // Referencia AVcc, sin ADLAR (justificación derecha)
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Prescaler 128
	
	ADCSRA |= (1 << ADSC);               // Primera conversión para estabilizar
	while (ADCSRA & (1 << ADSC));
}


uint16_t adc_leer_10bits(uint8_t canal) {
	ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);   // Seleccionar canal (0-7)
	ADCSRA |= (1 << ADSC);                      // Iniciar conversión
	while (ADCSRA & (1 << ADSC));                // Esperar a que termine
	uint8_t low = ADCL;                          // Leer primero ADCL (obligatorio)
	uint8_t high = ADCH;                         // Luego ADCH
	return (high << 8) | low;                     // Combinar en 16 bits
}