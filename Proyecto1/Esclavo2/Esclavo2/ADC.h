/*
 * ADC.h
 *
 * Created: 29/01/2026 19:23:24
 *  Author: samur
 */ 


#ifndef ADC_H_
#define ADC_H_
#include <stdint.h>

void adc_iniciar(void);
uint16_t adc_leer_10bits(uint8_t canal);
#endif