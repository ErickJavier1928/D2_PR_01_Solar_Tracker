/*
 * I2C.h
 *
 * Created: 5/02/2026 10:00:56
 *  Author: samur
 */ 


#ifndef I2C_H_
#define I2C_H_

#include <avr/io.h>
#include <stdint.h>

void I2C_master_init(unsigned long sclock, uint8_t prescaler);
uint8_t I2C_master_start(void);
uint8_t I2C_master_repeatedstart(void);
void I2C_master_stop(void);
uint8_t I2C_master_write(uint8_t dato);
uint8_t I2C_master_read(uint8_t *buffer, uint8_t ack);
void I2C_slave_init(uint8_t address);

#endif /* I2C_H_ */