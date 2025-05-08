/*
    BIBLIOTECAS
*/
#include <Wire.h> //configuração de I2C
#include <WiFi.h> //biblioteca WiFi
#include <NTPClient.h> //data e hora
#include <driver/adc.h> //nível de bateria
#include "MAX30105.h" //sensor BPM/Oxigenação
#include "heartRate.h" //processamento de BPM?Oxigenação
#include <Adafruit_MLX90614.h> //sensor de temperatura
#include <Adafruit_SSD1306.h> //controle do display
#include <Adafruit_GFX.h> //biblioteca grafica do display
#include <PubSubClient.h> //broker MQTT
#include <ArduinoJson.h> // dados em formato Json
#include <Preferences.h> // manipulação de memoria

/*
    VARIÁVEIS DE SETUP
*/

/*
// --- WiFi --- //
const char* ssid = ;
const char* password = ;

// --- MQTT --- //
const char* mqtt_server = ;
const int mqtt_port = ;
const char* mqtt_topic = ;

WiFiClient espClient;
PubSubClient client(espClient); */

// --- Sensores e dados --- //
//ESP32Time rtc;
MAX30105 particleSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// --- Interface I2C --- //
TwoWire I2C_0 = TwoWire(0); // Barramento I2C 0
TwoWire I2C_1 = TwoWire(1); // Barramento I2C 1

// --- Variáveis para cálculos --- //
double avered = 0;
double aveir = 0;
double sumirrms = 0;
double sumredrms = 0;
int i = 0;
int Num = 200; //intervalo de amostragem para o cálculo de SpO2
double ESpO2 = 95.0; // valor inicial de SpO2 estimado
double FSpO2 = 0.7;  // fator de filtro para SpO2 estimado
double frate = 0.95; // filtro passa-baixo para o valor do LED IR/vermelho

#define TIMETOBOOT 3000 // tempo de espera (ms) para a saída do SpO2
#define SCALE 88.0 // ajuste para exibir batimento cardíaco e SpO2 na mesma escala
#define SAMPLING 5 // mais preciso se definido como 1
#define FINGER_ON 30000 // se o sinal vermelho for menor que isso, indica que o dedo não está no sensor
#define MINIMUM_SPO2 80.0

const byte RATE_SIZE = 4; // Aumente isso para mais média. 4 é bom.
byte rates[RATE_SIZE]; // Array de batimentos cardíacos
byte rateSpot = 0;
long lastBeat = 0; // Hora em que ocorreu o último batimento
float beatsPerMinute;
int beatAvg;

#define USEFIFO

/*
    FUNÇÕES DE SETUP
*/


/*
    FUNÇÕES DE LEITURA
*/


/*
    SISTEMA OPERACIONAL
*/


/*
    VOID SETUP
*/
void setup() {
Serial.begin(115200);
Wire.begin();
    I2C_0.begin(8, 9); //I2C0
    I2C_1.begin(6, 7); //I2C1
particleSensor.begin(Wire, I2C_SPEED_STANDARD); //inicia sensor MAX30102
particleSensor.setup(); //configura o sensor
mlx.begin(); //inicia o sensor MLX90614
display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //inicia o display
  display.clearDisplay();
  display.display();

/*
    VOID LOOP
*/







// restante do codigo
// codigo para otimizar
----------------------------------------------------------

/*[22:17, 5/6/2025] Mid: #include <Wire.h> +
#include <Adafruit_MLX90614.h> +

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(9600);
  mlx.begin();
}*/

void loop() {
  double temp = mlx.readObjectTempC();
  double ambientTemp = mlx.readAmbientTempC();
  
  Serial.print("Temperatura do objeto: ");
  Serial.print(temp);
  Serial.print(" Â°C, Temperatura ambiente: ");
  Serial.print(ambientTemp);
  Serial.println(" Â°C");
  
  delay(1000);  // Espera 1 segundo
}
 
primeiro código temperatura
-----------------------------------------------------------------




[22:17, 5/6/2025] Mid: /* Bibliotecas */

#include <Wire.h> //Conf. I2C +
#include <WiFi.h> //B. WiFi +
 
#include <driver/adc.h> //Niv. Bateria +
#include "MAX30105.h" //Sensor MAX30102 +
#include "heartRate.h" //Proc. de BPM +
#include <Adafruit_MLX90614.h> // Sensor MLX90614 + 
#include <Adafruit_SSD1306.h> //Controle display +
#include <Adafruit_GFX.h> //Bib.Grafica display +
#include <PubSubClient.h> //Broker MQTT +
#include <ArduinoJson.h> //Dados formato Json +

/*
// --- WiFi --- //
const char* ssid = ;
const char* password = ;

// --- MQTT --- //
const char* mqtt_server = ;
const int mqtt_port = ;
const char* mqtt_topic = ;

WiFiClient espClient;
PubSubClient client(espClient); */

// --- Sensores e dados --- //
MAX30105 particleSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// --- Interface I2C --- //
TwoWire I2C_0 = TwoWire(0); // Barramento I2C 0
TwoWire I2C_1 = TwoWire(1); // Barramento I2C 1

// --- Variáveis para cálculos --- //
double avered = 0;
double aveir = 0;
double sumirrms = 0;
double sumredrms = 0;
int i = 0;
int Num = 200; //intervalo de amostragem para o cálculo de SpO2
double ESpO2 = 95.0; // valor inicial de SpO2 estimado
double FSpO2 = 0.7;  // fator de filtro para SpO2 estimado
double frate = 0.95; // filtro passa-baixo para o valor do LED IR/vermelho

#define TIMETOBOOT 3000 // tempo de espera (ms) para a saída do SpO2
#define SCALE 88.0 // ajuste para exibir batimento cardíaco e SpO2 na mesma escala
#define SAMPLING 5 // mais preciso se definido como 1
#define FINGER_ON 30000 // se o sinal vermelho for menor que isso, indica que o dedo não está no sensor
#define MINIMUM_SPO2 80.0

const byte RATE_SIZE = 4; // Aumente isso para mais média. 4 é bom.
byte rates[RATE_SIZE]; // Array de batimentos cardíacos
byte rateSpot = 0;
long lastBeat = 0; // Hora em que ocorreu o último batimento
float beatsPerMinute;
int beatAvg;

#define USEFIFO


/*Void setup
void setup() {
 Serial.begin(115200);
 Wire.begin();
    I2C_0.begin(8, 9); //I2C0
    I2C_1.begin(6, 7); //I2C1
    
 particleSensor.begin(Wire,
    I2C_SPEED_STANDARD); //inicia sensor MAX30102
 particleSensor.setup(); // Configuração o sensor


mlx.begin(); //Inicia sensor mlx90614
    

display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //inicia o display
  display.clearDisplay();
  display.display();
}*/

void MAX30102() {
// Configuração do sensor
  byte ledBrightness = 0x7F; // Opções: 0=Off até 255=50mA
  byte sampleAverage = 4; // Opções: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; // Opções: 1 = Vermelho apenas, 2 = Vermelho + IR, 3 = Vermelho + IR + Verde
  int sampleRate = 200; // Opções: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; // Opções: 69, 118, 215, 411
  int adcRange = 16384; // Opções: 2048, 4096, 8192, 16384

  // Configurar parâmetros do sensor
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  particleSensor.enableDIETEMPRDY();
}


//void loop
void loop() {
  uint32_t ir, red, green;
  double fred, fir;
  double SpO2 = 0; // SpO2 bruto antes de passar pelo filtro

#ifdef USEFIFO
  particleSensor.check(); // Verifique o sensor, leia até 3 amostras

  while (particleSensor.available()) {
    red = particleSensor.getFIFORed(); // Sensor Sparkfun MAX30105
    ir = particleSensor.getFIFOIR();  // Sensor Sparkfun MAX30105

    i++;

    fred = (double)red;
    fir = (double)ir;
    avered = avered * frate + (double)red * (1.0 - frate); // Filtragem de média para o sinal vermelho
    aveir = aveir * frate + (double)ir * (1.0 - frate); // Filtragem de média para o sinal IR
    sumredrms += (fred - avered) * (fred - avered); // Soma quadrática do componente alternado do sinal vermelho
    sumirrms += (fir - aveir) * (fir - aveir); // Soma quadrática do componente alternado do sinal IR

    if ((i % SAMPLING) == 0) {
      if (millis() > TIMETOBOOT) {
        if (ir < FINGER_ON) ESpO2 = MINIMUM_SPO2; // Se o dedo for retirado, atribui o mínimo de SpO2
        float temperature = particleSensor.readTemperatureF(); // Leitura da temperatura, pode ser usada se necessário

        // Exibe o valor de SpO2 no monitor serial
        Serial.print("Oxygen % = ");
        Serial.println(ESpO2);
      }
    }

    if ((i % Num) == 0) {
      double R = (sqrt(sumredrms) / avered) / (sqrt(sumirrms) / aveir); // Cálculo de SpO2
      SpO2 = -23.3 * (R - 0.4) + 100;
      ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2; // Filtro passa-baixo

      sumredrms = 0.0;
      sumirrms = 0.0;
      i = 0;

      break;
    }

    particleSensor.nextSample(); // Passa para a próxima amostra
  }
#endif
}

--------------------------------------------------------------------------
segundo código batimento