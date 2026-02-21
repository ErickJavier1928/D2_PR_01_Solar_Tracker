/*
 * I2C.c
 *
 * Created: 5/02/2026 10:01:09
 *  Author: samur
 */ 
#include "I2C.h"

/*#ifndef F_CPU
#define F_CPU 16000000UL
#endif*/
#define F_CPU 16000000UL
void I2C_master_init(unsigned long sclock, uint8_t prescaler) {
	//Configurar pines como entrada
	DDRC &= ~((1<<DDC4) | (1<<DDC5));
	PORTC |= (1<<PORTC4) | (1<<PORTC5);
	
	//Configurar prescaler
	TWSR = 0;
	if (prescaler == 4) TWSR |= (1<<TWPS0);
	else if (prescaler == 16) TWSR |= (1<<TWPS1);
	else if (prescaler == 64) TWSR |= (1<<TWPS1) | (1<<TWPS0);
	
	//Configurar velocidad
	TWBR = ((F_CPU / sclock) - 16) / (2 * prescaler);
	TWCR = (1<<TWEN);
}

uint8_t I2C_master_start(void) {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	return (TWSR & 0xF8) == 0x08;
}

uint8_t I2C_master_repeatedstart(void) {
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	return (TWSR & 0xF8) == 0x10;
}

void I2C_master_stop(void) {
	TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
	while (TWCR & (1<<TWSTO));
}

uint8_t I2C_master_write(uint8_t data) {
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
	return (TWSR & 0xF8) == 0x18 || (TWSR & 0xF8) == 0x28 || (TWSR & 0xF8) == 0x40;
}

uint8_t I2C_master_read(uint8_t *buffer, uint8_t ack) {
	if (ack) {
		TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
		} else {
		TWCR = (1<<TWINT) | (1<<TWEN);
	}
	
	while (!(TWCR & (1<<TWINT)));
	*buffer = TWDR;
	return 1;
}

void I2C_slave_init(uint8_t address) {
	DDRC &= ~((1<<DDC4) | (1<<DDC5));
	PORTC |= (1<<PORTC4) | (1<<PORTC5);
	TWAR = (address << 1) | 1;
	TWCR = (1<<TWEA) | (1<<TWEN) | (1<<TWIE);
}