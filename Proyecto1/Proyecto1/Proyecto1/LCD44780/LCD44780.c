/*
 * LCD44780.c
 *
 * Created: 18/02/2026 11:48:24
 *  Author: samur
 */ 

#define F_CPU 16000000
#include <util/delay.h>
#include <avr/io.h>
#include <stdint.h>
#include "LCD44780.h"

extern char* itoa(int value, char* str, int base);

void LCD_WRITE_BUS(uint8_t R01){
	// Clear old data
	PORTD &= ~((1<<PIND2)|(1<<PIND3)|(1<<PIND4)|(1<<PIND5)|(1<<PIND6)|(1<<PIND7));
	PORTB &= ~((1<<PINB0)|(1<<PINB1));

	// Lower 6 bits -> PD2..PD7
	PORTD |= ((R01 & 0x3F) << 2);

	// Upper 2 bits -> PB0..PB1
	PORTB |= ((R01 >> 6) & 0x03);
}
void LCD_PULSE(void){
	PORTB |=  (1<<PINB4);   // EN = 1
	_delay_us(1);
	PORTB &= ~(1<<PINB4);  // EN = 0
}
void LCD_START_UP(void){
	_delay_ms(40);

	LCD_WRITE_BUS(0x30);
	LCD_PULSE();
	_delay_ms(10);

	LCD_WRITE_BUS(0x30);
	LCD_PULSE();
	_delay_us(200);

	LCD_WRITE_BUS(0x30);
	LCD_PULSE();
	_delay_us(200);

	LCD_WRITE_BUS(0x38);
	LCD_PULSE();
	_delay_us(100);

	LCD_WRITE_BUS(0x08);
	LCD_PULSE();
	_delay_us(100);

	LCD_WRITE_BUS(0x01);
	LCD_PULSE();
	_delay_ms(10);

	LCD_WRITE_BUS(0x06);
	LCD_PULSE();
	_delay_us(100);

	LCD_WRITE_BUS(0x0C);
	LCD_PULSE();
	_delay_us(100);
}

//OUT:
void LCD_OUT(char c){
	PORTB |=  (1<<PORTB2);   // RS = 1 ? DATA
	LCD_WRITE_BUS(c);       // put ASCII on bus
	LCD_PULSE();            // latch into DDRAM
}
void LCD_OUT_FULL(const char *s){
	while(*s){
		LCD_OUT(*s);
		s++;
	}
}

//LOCATION:
void LOC(uint8_t LC){
	PORTB &= ~(1<<PINB2);   // RS = 0 ? COMMAND
	LCD_WRITE_BUS(LC);     // put command on D0..D7
	LCD_PULSE();            // latch
	_delay_us(40);          // let LCD execute
}

//S1 and S2:
void u8_to_dec(uint8_t v, char *buf){
	buf[0] = '0' + (v / 100);
	buf[1] = '0' + (v / 10) % 10;
	buf[2] = '0' + (v % 10);
	buf[3] = '\0';
}
void format_voltage(uint8_t adc, char *buf){
	uint32_t mv = (uint32_t)adc * 5000;  // use 32-bit to avoid overflow
	mv /= 255;

	uint16_t v = mv / 1000;        // integer volts
	uint16_t d = (mv % 1000) / 10; // centivolts (0–99)

	buf[0] = '0' + v;
	buf[1] = '.';
	buf[2] = '0' + (d / 10);
	buf[3] = '0' + (d % 10);
	buf[4] = 'V';
	buf[5] = '\0';
}
void format_adc1024(uint8_t adc, char *buf)
{
	uint16_t val = ((uint32_t)adc * 1024) / 255;

	// format as 4-digit right-aligned number
	buf[0] = (val / 1000) ? ('0' + val / 1000) : ' ';
	buf[1] = (val / 100)  ? ('0' + (val / 100) % 10) : ' ';
	buf[2] = (val / 10)   ? ('0' + (val / 10) % 10) : ' ';
	buf[3] = '0' + (val % 10);
	buf[4] = '\0';
}

//S3:
void LCD_print_CNT(int32_t IN){
	if (IN > 99999){
		IN = 99999;
	}
	else{}
	
	uint8_t d4 = IN / 10000;        // ten-thousands
	uint8_t d3 = (IN / 1000) % 10;  // thousands
	uint8_t d2 = (IN / 100)  % 10;  // hundreds
	uint8_t d1 = (IN / 10)   % 10;  // tens
	uint8_t d0 = IN % 10;           // ones
	
	//START: 1-16 -> 12...
	LOC(0xCB);
	
	//OUT: '0' + 7 = 48 + 7 = 55.
	LCD_OUT('0' + d4);
	LCD_OUT('0' + d3);
	LCD_OUT('0' + d2);
	LCD_OUT('0' + d1);
	LCD_OUT('0' + d0);
}