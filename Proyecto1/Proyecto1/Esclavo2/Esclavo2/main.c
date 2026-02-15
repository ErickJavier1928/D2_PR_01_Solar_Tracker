/*
 * Esclavo2.c
 *
 * Created: 12/02/2026 11:09:47
 * Author : samur
 */

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "I2C.h"
#include "ADC.h"
#include "UART.h"
#include "PWM/PWM.h"

#define SLAVE_ADDRESS 0x40

uint8_t potencia;
uint8_t voltaje;
uint8_t corriente;
uint8_t motor;
char buffer[20];       //Para UART

ISR(TWI_vect) {
	switch (TWSR & 0xF8) {
		//Maestro llama
		case 0x60:
		case 0x70:
		TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
		break;
		
		case 0x80:
		motor = TWDR;
		TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
		break;
		
		case 0x88:   //Dato recibido, NACK
		TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE);
		break;
		
		//Maestro quiere leer
		case 0xA8:
		case 0xB8:
		TWDR = potencia; //Enviar potencia actual
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;
		
		//Maestro termina
		case 0xC0:
		case 0xC8:
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;
		
		default:
		TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
		break;
	}
}

int main(void) {
	cli();
	adc_iniciar();
	I2C_slave_init(SLAVE_ADDRESS);
	UART_Init();
	//PD6
	PWM_T0_OC0A_PD6_OC0B_PD5(1, 0, 0, 0, 1, 0, 64);
	sei();
	
	
	while(1) {
		LD_T0_OCR0A(motor);
		corriente = adc_leer_8bits(0);
		voltaje = adc_leer_8bits(1);
		uint16_t producto = (uint16_t)corriente * voltaje;
		potencia = producto/255;
		//Debug por UART
		sprintf(buffer, "%u %u %u %u\r\n", corriente, voltaje, potencia, motor);
		UART_SendString(buffer);
		
		_delay_ms(100);
	}
}

