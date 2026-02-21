/************************ Adafruit IO Config *******************************/

#ifndef CONFIG_H
#define CONFIG_H

// Credenciales Adafruit IO
#define USUARIO_IO  
#define CLAVE_IO   
                  //Detecta github
// WiFi
#define WIFI_SSID   
#define WIFI_PASS   

// UART con Arduino Nano
#define PIN_RX  16
#define PIN_TX  17
#define VELOCIDAD 9600

// Control de tasa para Adafruit IO (30 datos/minuto)
#define MIN_ENVIO_MS  3000   //3 segundos entre envíos
#define MAX_POR_MIN   30     //máximo 30 datos por minuto

#endif