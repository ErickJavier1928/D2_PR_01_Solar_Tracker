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
#include "LCD44780/LCD44780.h"

#define BH1750_ADDR_W   0x46
#define BH1750_ADDR_R   0x47

#define SLAVE_30_W      0x60   // 0x30 << 1
#define SLAVE_30_R      0x61

#define SLAVE_40_W      0x80   // 0x40 << 1
#define SLAVE_40_R      0x81

char buffer[60];
char buf[8];
//variables sensor de luz
uint8_t lux_h, lux_l;
uint16_t lux_entero = 0;

//Sensores del esclavo 0x30
uint8_t ldr0, ldr1, ldr2, ldr3;

//Potencia del esclavo 0x40
uint8_t potencia_rx = 0;

//Comandos recibidos del ESP32
uint8_t cmd_dc = 0;          // 0-255
uint8_t cmd_servo = 90;      // 0-180
uint8_t cmd_stepper = 0;     // 0-255

//Modo de operacion: 1=auto, 0=manual
uint8_t modo_auto = 1;

uint8_t CN = 0x00;
uint8_t last0 = 255;
uint8_t last1 = 255;

volatile uint32_t CNT=0x00;

uint8_t servo_auto = 90;
uint8_t stepper_auto = 0;
#define UMBRAL 50            //diferencia minima para reaccionar
#define PASO_AUTO1 2         //incremento por ciclo para servo
#define PASO_AUTO2 5        // incremento por ciclo para stepper

//Turno de envío a ESP32
uint8_t turn = 0;            // 0 = LUX, 1 = POTENCIA

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
	DDRB = 0xFF;
	DDRD = 0xFF;	
	//LCD:
	LCD_START_UP();
	//0: 0x80 -> 0x40 -> 0xC0.
	LOC(0x80);
	LCD_OUT_FULL("Lux POT POS");
	
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
    uint16_t t_bh1750 = 0;
    uint16_t t_sendESP = 0;
    uint16_t t_ldr = 0;
	uint16_t t_dc =0;
    while (1)
    {
        //Leer UART del esp32
        while (UART_ReadLine_NonBlocking(rxLine, sizeof(rxLine)))
        {
            //"S:90", "T:120", "D:255", "M:1"
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
                else if (rxLine[0] == 'M')
                {
                    modo_auto = (val != 0) ? 1 : 0;
                }
            }
        }

        //Cada 20ms enviar comandos al esclavo 0x30
        if (t_uart_i2c >= 20)
        {
            t_uart_i2c = 0;
            //Elegir que valores enviar segun modo
            uint8_t servo_enviar = modo_auto ? servo_auto : cmd_servo;
            uint8_t stepper_enviar = modo_auto ? stepper_auto : cmd_stepper;

            I2C_master_start();
            I2C_master_write(SLAVE_30_W);
            I2C_master_write(servo_enviar);
            I2C_master_write(stepper_enviar);
            I2C_master_stop();

            //esclavo 0x40
            
            I2C_master_start();
            I2C_master_write(SLAVE_40_W);
            I2C_master_write(cmd_dc);
            I2C_master_stop();
            
        }

        //leer BH1750 cada 250ms
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
			
			format_adc1024(lux_entero, buf);
			LOC(0xC0);   // row2 col1
			LCD_OUT_FULL(buf);
        }
		 if (t_dc >= 30)
		 {
			if (lux_entero < 2000){
				if (potencia_rx > 50){
					cmd_dc = 255;
				}
				else{
					cmd_dc = 0;
				}
			}
		 }

        //leer LDRs del esclavo 0x30
        if (t_ldr >= 50)
        {
            t_ldr = 0;

            //Leer los 4 LDRs
            I2C_master_start();
            I2C_master_write(SLAVE_30_R);
            I2C_master_read(&ldr0, 1);
            I2C_master_read(&ldr1, 1);
            I2C_master_read(&ldr2, 1);
            I2C_master_read(&ldr3, 0);
            I2C_master_stop();
			
			//leer potencia al esclavo 0x40
			I2C_master_start();
			I2C_master_write(SLAVE_40_R);
			I2C_master_read(&potencia_rx, 0);
			I2C_master_stop();
			
			format_voltage(potencia_rx, buf);
			LOC(0xC6);   // row2 col1
			LCD_OUT_FULL(buf);
			
			

            //Logica de seguimiento automatico
            if (modo_auto)
            {
                //comparar ldr0 0 y ldr1 180
                if (ldr0 > ldr1 + UMBRAL)
                {
                    if (stepper_auto < 255) stepper_auto += PASO_AUTO2;
                    if (stepper_auto > 255) stepper_auto = 255;
					LOC(0xCB);
					LCD_OUT_FULL("ESTE");
                }
                else if (ldr1 > ldr0 + UMBRAL)
                {
                    if (stepper_auto > 0) stepper_auto -= PASO_AUTO2;
                    if (stepper_auto < 0) stepper_auto = 0;
					LOC(0xCB);
					LCD_OUT_FULL("OESTE");
                }

                //comparar ldr2 90 y ldr3 270
                if (ldr2 > ldr3 + UMBRAL)
                {
                    if (servo_auto < 180) servo_auto += PASO_AUTO1;
                    if (servo_auto > 180) servo_auto = 180;
					LOC(0xCB);
					LCD_OUT_FULL("NORTE");
                }
                else if (ldr3 > ldr2 + UMBRAL)
                {
                    if (servo_auto > 0) servo_auto -= PASO_AUTO1;
                    if (servo_auto < 0) servo_auto = 0;
					LOC(0xCB);
					LCD_OUT_FULL("SUR");
                }
            }
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
		t_dc++;
    }
}