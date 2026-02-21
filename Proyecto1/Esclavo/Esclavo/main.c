/*
 * Esclavo.c
 *
 * Created: 5/02/2026 11:18:22
 * Author : samur
 */ 

#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>

#include "I2C.h"
#include "ADC.h"
#include "UART.h"
#include "PWM/PWM.h"

#define SLAVE_ADDRESS 0x30

//LDR
uint8_t ldr[4];
uint8_t indice = 0;

//SERVO
volatile uint8_t servo_cmd = 90;

//STEPPER (28BYJ-48 + ULN2003)
//Pines PD2-PD5
#define STEPPER_DDR   DDRD
#define STEPPER_PORT  PORTD
#define IN1 PD2
#define IN2 PD3
#define IN3 PD4
#define IN4 PD5

//Slider 0-255 pasa a 0-360°
volatile uint8_t stepper_cmd = 0;

//pasos actuales y objetivo
volatile int16_t current_step = 0;
volatile int16_t target_step  = 0;

int8_t step_index = 0;

#define NUM_STEPS 8
const uint8_t step_sequence[NUM_STEPS] = {
    0b0001,
    0b0011,
    0b0010,
    0b0110,
    0b0100,
    0b1100,
    0b1000,
    0b1001
};

//CONFIGURACION STEPPER
//28BYJ-48 en medio paso ? 4096 pasos por vuelta
#define STEPS_PER_REV 4096
#define STEPPER_STEP_DELAY_MS 1

//UART debug
char buffer[80];

//Funciones

void stepper_init(void)
{
    STEPPER_DDR |= (1<<IN1) | (1<<IN2) | (1<<IN3) | (1<<IN4);
    STEPPER_PORT &= ~((1<<IN1) | (1<<IN2) | (1<<IN3) | (1<<IN4));
}

void stepper_write_pattern(uint8_t pattern)
{
    STEPPER_PORT &= ~((1<<IN1) | (1<<IN2) | (1<<IN3) | (1<<IN4));

    if (pattern & 0x01) STEPPER_PORT |= (1<<IN1);
    if (pattern & 0x02) STEPPER_PORT |= (1<<IN2);
    if (pattern & 0x04) STEPPER_PORT |= (1<<IN3);
    if (pattern & 0x08) STEPPER_PORT |= (1<<IN4);
}

void stepper_step_cw(void)
{
    step_index++;
    if (step_index >= NUM_STEPS) step_index = 0;
    stepper_write_pattern(step_sequence[(uint8_t)step_index]);
    current_step++;
}

void stepper_step_ccw(void)
{
    step_index--;
    if (step_index < 0) step_index = NUM_STEPS - 1;
    stepper_write_pattern(step_sequence[(uint8_t)step_index]);
    current_step--;
}

void servo_write_from_cmd(uint8_t ang)
{
    //ang ya viene en grados 0-180
    uint16_t pulse = 2000 + ((uint32_t)ang * 2000) / 180; //2000us = 0°, 4000us = 180°
    LD_T1_OCR1A(pulse);
}
 

ISR(TWI_vect)
{
    static uint8_t rx_byte_count = 0;

    switch (TWSR & 0xF8)
    {
        case 0x60:  //propia dirección + SA
        case 0x70:  //propia dirección + llamada general
            rx_byte_count = 0;
            TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWIE)|(1<<TWEA);
            break;

        case 0x80:  //dato recibido, esclavo recibe
        {
            uint8_t dato = TWDR;

            if (rx_byte_count == 0)
            {
                servo_cmd = dato;       //0-180
                rx_byte_count = 1;
            }
            else
            {
                stepper_cmd = dato;     //0-255
                //convertir de 0-255 a 0-4096
                target_step = ((int32_t)stepper_cmd * STEPS_PER_REV) / 255;
                rx_byte_count = 0;
            }

            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
        }
        break;

        case 0x88:  //ultimo dato recibido con NACK
            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            break;

        //Maestro quiere leer LDR, esclavo envia
        case 0xA8:  //propia dirección + SR
        case 0xB8:  //dato transmitido y ACK recibido
            TWDR = ldr[indice];
            indice = (indice + 1) % 4;
            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            break;

        case 0xC0:  //ultimo dato transmitido con NACK
        case 0xC8:  //dato transmitido y ACK recibido
            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            break;

        default:
            TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWEN)|(1<<TWIE);
            break;
    }
}

int main(void)
{
    cli();

    adc_iniciar();
    I2C_slave_init(SLAVE_ADDRESS);
    UART_Init();

    //Servo Timer1 OC1A PB1 periodo 20ms
    PWM_T1_OC1A_PB1_OC1B_PB2(1, 0, 0, 0, 1, 14, 8);
    LD_T1_ICR1(39999);  //TOP para 20ms con prescaler 8

    stepper_init();

    sei(); 

    while(1)
    {
        //Actualizar servo con el último comando recibido
        servo_write_from_cmd(servo_cmd);

        //Leer LDRs
        ldr[0] = adc_leer_8bits(0);
        ldr[1] = adc_leer_8bits(1);
        ldr[2] = adc_leer_8bits(2);
        ldr[3] = adc_leer_8bits(3);

        //Mover stepper hacia target_step
        if (current_step < target_step)
        {
            stepper_step_cw();
            _delay_ms(STEPPER_STEP_DELAY_MS);
        }
        else if (current_step > target_step)
        {
            stepper_step_ccw();
            _delay_ms(STEPPER_STEP_DELAY_MS);
        }
        else
        {
            //Si esta en posicion se apaga
            stepper_write_pattern(0);
            _delay_ms(2);
        }
		/*sprintf(buffer, "L1:%u  L2:%u  L3:%d  L4:%d\r\n", ldr[0], ldr[1], ldr[2], ldr[3]);
		UART_SendString(buffer);
		_delay_ms(100);*/
        //Debug
        /*
        sprintf(buffer, "servo:%u  stepper:%u  corriente:%d  target:%d\r\n", servo_cmd, stepper_cmd, current_step, target_step);
        UART_SendString(buffer);
        _delay_ms(100);
        */
    }
}