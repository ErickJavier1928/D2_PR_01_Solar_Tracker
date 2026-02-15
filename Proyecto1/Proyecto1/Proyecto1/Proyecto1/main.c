/*
 * Proyecto1.c
 *
 * Created: 5/02/2026 09:57:23
 * Author : samur
 */ 

// main_maestro.c
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdio.h>

#include "I2C/I2C.h"
#include "UART/UART.h"

#define BH1750_ADDR_W   0x46
#define BH1750_ADDR_R   0x47

#define SLAVE_30_W      0x60   // 0x30 << 1
#define SLAVE_30_R      0x61

#define SLAVE_40_W      0x80   // 0x40 << 1
#define SLAVE_40_R      0x81


char buffer[60];

//variables sensor de luz
uint8_t lux_h, lux_l;
uint16_t lux_entero = 0;

//Sensores del esclavo 0x30
uint8_t ldr0, ldr1, ldr2, ldr3;

//Potencia del esclavo 0x40
uint8_t potencia_rx = 0;

//Comandos recibidos del ESP32
uint8_t cmd_dc = 0;          //0-255
uint8_t cmd_servo = 90;       //0-180
uint8_t cmd_stepper = 0;      //0-255

//Turno de envío a ESP32
uint8_t turn = 0;   //0 = LUX, 1 = POTENCIA


//Convierte de string a int
int parse_int(const char *s)
{
    int v = 0;
    uint8_t neg = 0;

    if (*s == '-') { neg = 1; s++; }

    while (*s >= '0' && *s <= '9')
    {
        v = v * 10 + (*s - '0');
        s++;
    }

    if (neg) v = -v;
    return v;
}


int main(void)
{
    I2C_master_init(100000, 1);
    UART_Init();
    _delay_ms(200);
    sei();

    //Iniciar BH1750
    I2C_master_start();
    I2C_master_write(BH1750_ADDR_W);
    I2C_master_write(0x01); //encender
    I2C_master_stop();
    _delay_ms(10);

    I2C_master_start();
    I2C_master_write(BH1750_ADDR_W);
    I2C_master_write(0x10); //alta resolucion
    I2C_master_stop();
    _delay_ms(180);

    char rxLine[25];

    //Temporizadores
    uint16_t t_uart_i2c = 0;
    uint16_t t_bh1750  = 0;
    uint16_t t_sendESP = 0;
    uint16_t t_ldr     = 0;

    while (1)
    {
        //Leer UART del esp32
        while (UART_ReadLine_NonBlocking(rxLine, sizeof(rxLine)))
        {
            //"S:90", "T:120", "D:255"
            if (rxLine[1] == ':')
            {
                int val = parse_int(&rxLine[2]);

                if (rxLine[0] == 'D')
                {
                    if (val < 0) val = 0;
                    if (val > 255) val = 255;
                    cmd_dc = (uint8_t)val;
                }
                else if (rxLine[0] == 'S')
                {
                    if (val < 0) val = 0;
                    if (val > 180) val = 180;
                    cmd_servo = (uint8_t)val;
                }
                else if (rxLine[0] == 'T')
                {
                    if (val < 0) val = 0;
                    if (val > 255) val = 255;
                    cmd_stepper = (uint8_t)val;
                }
            }
        }

        //Cada 20ms enviar comandos al esclavo 0x30
        if (t_uart_i2c >= 20)
        {
            t_uart_i2c = 0;
            //Mandar servo 0-180 y stepper 0-255
			I2C_master_start();
			I2C_master_write(SLAVE_30_W);
			I2C_master_write(cmd_servo);
			I2C_master_write(cmd_stepper);
			I2C_master_stop();

            //esclavo 0x40
            /*
            I2C_master_start();
            I2C_master_write(SLAVE_40_W);
            I2C_master_write(cmd_dc);
            I2C_master_stop();
            */
        }

        //leer BH1750
        if (t_bh1750 >= 250)
        {
            t_bh1750 = 0;
            I2C_master_start();
            I2C_master_write(BH1750_ADDR_W);
            I2C_master_repeatedstart();
            I2C_master_write(BH1750_ADDR_R);

            I2C_master_read(&lux_h, 1);
            I2C_master_read(&lux_l, 0);
            I2C_master_stop();

            uint16_t raw_lux = ((uint16_t)lux_h << 8) | lux_l;
            lux_entero = (raw_lux * 10) / 12;
        }

        //leer LDRs del esclavo 0x30
        if (t_ldr >= 500)
        {
            t_ldr = 0;
            I2C_master_start();
            I2C_master_write(SLAVE_30_R);
            I2C_master_read(&ldr0, 1);
            I2C_master_read(&ldr1, 1);
            I2C_master_read(&ldr2, 1);
            I2C_master_read(&ldr3, 0);
            I2C_master_stop();
        }

        //Cada 2S enviar a ESP32 lux y potencia
        if (t_sendESP >= 2000)
        {
            t_sendESP = 0;

            if (turn == 0)
            {
                sprintf(buffer, "L:%u\r\n", lux_entero);
                UART_SendString(buffer);
                turn = 1;
            }
            else
            {
                sprintf(buffer, "P:%u\r\n", potencia_rx);
                UART_SendString(buffer);
                turn = 0;
            }
        }

        _delay_ms(1);
        t_uart_i2c++;
        t_bh1750++;
        t_sendESP++;
        t_ldr++;
    }
}