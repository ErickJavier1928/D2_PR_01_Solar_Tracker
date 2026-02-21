#include "config.h"
#include <AdafruitIO_WiFi.h>

//Crear conexión WiFi con Adafruit IO
AdafruitIO_WiFi io(USUARIO_IO, CLAVE_IO, WIFI_SSID, WIFI_PASS);

//Feeds
AdafruitIO_Feed *feedPotencia = io.feed("potencia");
AdafruitIO_Feed *feedLuz      = io.feed("luz");
AdafruitIO_Feed *feedMotorDC  = io.feed("motordc");
AdafruitIO_Feed *feedServo    = io.feed("servo");
AdafruitIO_Feed *feedStepper  = io.feed("stepper");
AdafruitIO_Feed *feedModo     = io.feed("modo");

//Comunicación serie con el Nano uart2
#include <HardwareSerial.h>
HardwareSerial nano(2);

//Variables para control de envio
unsigned long ultimoEnvio = 0; //momento del último envío
unsigned int datosEnviados = 0; //contador en el minuto actual
unsigned long inicioMinuto = 0;//inicio del minuto actual

//Buffer para línea recibida del Nano
String linea = "";

//Variable para modo (solo debug)
int modo_actual = 1;

//cuando cambia un slider en Adafruit, se envía al Nano
void alCambiarMotorDC(AdafruitIO_Data *dato) {
  int valor = constrain(dato->toInt(), 0, 255);
  nano.print("D:");
  nano.println(valor);
  Serial.print("Enviado a Nano MotorDC ");
  Serial.println(valor);
}

void alCambiarServo(AdafruitIO_Data *dato) {
  int valor = constrain(dato->toInt(), 0, 180);
  nano.print("S:");
  nano.println(valor);
  Serial.print("Enviado a Nano Servo ");
  Serial.println(valor);
}

void alCambiarStepper(AdafruitIO_Data *dato) {
  int valor = constrain(dato->toInt(), 0, 255);
  nano.print("T:");
  nano.println(valor);
  Serial.print("Enviado a Nano Stepper ");
  Serial.println(valor);
}

//Nuevo callback para modo
void alCambiarModo(AdafruitIO_Data *dato) {
  int valor = dato->toInt();
  modo_actual = (valor != 0) ? 1 : 0;
  nano.print("M:");
  nano.println(modo_actual);
  Serial.print("Modo cambiado a ");
  Serial.println(modo_actual ? "AUTO" : "MANUAL");
}

//Verifica si se puede enviar un dato a Adafruit cada 3 segundos
bool sePuedeEnviar() {
  unsigned long ahora = millis();

  //Reiniciar contador cada minuto
  if (ahora - inicioMinuto >= 60000) {
    datosEnviados = 0;
    inicioMinuto = ahora;
    Serial.println("Minuto nuevo, contador reiniciado");
  }

  //Comprobar límite por minuto y tiempo mínimo entre envíos
  if (datosEnviados < MAX_POR_MIN && (ahora - ultimoEnvio) >= MIN_ENVIO_MS) {
    return true;
  }
  return false;
}

//Configuración inicial
void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("ESP32 iniciado");

  //Iniciar UART con el Nano
  nano.begin(VELOCIDAD, SERIAL_8N1, PIN_RX, PIN_TX);

  //Conectar a Adafruit IO
  Serial.print("Conectando a Adafruit IO");
  io.connect();

  //Guardar datos de los sliders
  feedMotorDC->onMessage(alCambiarMotorDC);
  feedServo->onMessage(alCambiarServo);
  feedStepper->onMessage(alCambiarStepper);
  feedModo->onMessage(alCambiarModo);

  //Obtener ultimos valores guardados
  feedMotorDC->get();
  feedServo->get();
  feedStepper->get();
  feedModo->get();

  //Esperar conexion
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConectado a Adafruit IO");

  //Inicializar temporizadores
  inicioMinuto = millis();
  ultimoEnvio = millis();  //evitar envio sin control
}

//Bucle principal
void loop() {
  //Mantener conexion con adafruit IO
  io.run();

  //Leer datos del Nano
  while (nano.available()) {
    char c = nano.read();

    if (c == '\n') {
      linea.trim();
      Serial.println("Desde Nano: " + linea);

      //Enviar a adafruit si se cumplen las condiciones de tiempo
      if (sePuedeEnviar()) {
        unsigned long ahora = millis();
        ultimoEnvio = ahora;

        if (linea.startsWith("P:")) {
          float valor = linea.substring(2).toFloat();
          feedPotencia->save(valor);
          datosEnviados++;
          Serial.print("Potencia enviada (");
          Serial.print(datosEnviados);
          Serial.println("/30 este minuto)");
        }
        else if (linea.startsWith("L:")) {
          int valor = linea.substring(2).toInt();
          feedLuz->save(valor);
          datosEnviados++;
          Serial.print("Luz enviada (");
          Serial.print(datosEnviados);
          Serial.println("/30 este minuto)");
        }
        else {
          Serial.println("Formato de línea no reconocido");
        }
      } else {
        Serial.println("Esperando para enviar (límite de tasa)");
      }

      linea = "";//reiniciar buffer
    }
    else if (c != '\r') {
      if (linea.length() < 40) {
        linea += c;
      }
    }
  }

  delay(10);
}