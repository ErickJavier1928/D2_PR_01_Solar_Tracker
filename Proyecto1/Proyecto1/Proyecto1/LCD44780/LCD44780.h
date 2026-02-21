/*
 * LCD44780.h
 *
 * Created: 18/02/2026 11:48:37
 *  Author: samur
 */ 


#ifndef LCD44780_H_
#define LCD44780_H_

//SETUP:
void LCD_WRITE_BUS(uint8_t R01);
void LCD_PULSE(void);
void LCD_START_UP(void);

//OUT:
void  LCD_OUT(char c);
void LCD_OUT_FULL(const char *s);

//LOCATION:
void LOC(uint8_t LC);
void u8_to_dec(uint8_t v, char *buf);
void format_voltage(uint8_t adc, char *buf);
void format_adc1024(uint8_t adc, char *buf);
void LCD_print_CNT(int32_t val);


#endif /* LCD44780_H_ */
