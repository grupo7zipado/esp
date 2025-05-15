/*
    BIBLIOTECAS
*/
#include <Wire.h> //configuração de I2C
#include <WiFi.h> //biblioteca WiFi
#include <time.h> //data e hora
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

/* [Conexões Externas]
// --- WiFi --- //
const char* ssid = "";
const char* password = "";

// --- MQTT --- //
const char* mqtt_server = "";
const int mqtt_port = ;
const char* mqtt_topic = "";

WiFiClient espClient;
PubSubClient client(espClient); 
*/

//  [Conexões Internas]
// --- Interface I2C --- //
TwoWire I2C_0 = TwoWire(0);
TwoWire I2C_1 = TwoWire(1);

// --- Sensores e display --- //
MAX30105 particleSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SSD1306 display(128, 64, &I2C_0, -1);


// --- Variáveis para cálculos spO2 e BPM --- //
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
double spo2 = 0.0;
double temp = 0.0;

#define USEFIFO


/*
    FUNÇÕES DE SETUP
*/

void initI2C() {
    I2C_0.begin(8, 9); //I2C 0 para o MAX30102 e Display Oled
    I2C_1.begin(6, 7); //I2C 1 para o MLX90614
}

void initDisplay() {
display.begin(SSD1306_SWITCHCAPVCC, 0x3C); //inicia o display
  display.clearDisplay();
  display.display();
}

void confMAX30102() {
// variáveis de hardware do sensor
  byte ledBrightness = 0x7F; //intensidade do led
  byte sampleAverage = 4; //amostras para média 
  byte ledMode = 3; //modo do led
  int sampleRate = 200; //frequência de amostragem (Hz)
  int pulseWidth = 411; //duração de pulso do led
  int adcRange = 16384; // faixa de leitura adc


  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void initSensors() {
particleSensor.begin(I2C_0); //inicia sensor MAX30102 (velocidade padrão 100khz) 
//I2C_0.setClock(400000); //altere a velocidade entre 100/400 kHz
confMAX30102();
mlx.begin(0x5A, &I2C_1); //inicia o sensor MLX90614

}

/*
    FUNÇÕES DE LEITURA
*/

void readMAX() {
 uint32_t ir, red;
 double fred, fir;
 double SpO2 = 0; // SpO2 bruto

#ifdef USEFIFO
 particleSensor.check(); // Verifica o sensor, lê até 3 amostras

 while (particleSensor.available()) {
  red = particleSensor.getFIFORed();
  ir = particleSensor.getFIFOIR();

 //Cálculo de BPM
  if (checkForBeat(ir)) {
   long delta = millis() - lastBeat;
   lastBeat = millis();
   beatsPerMinute = 60 / (delta / 1000.0);

   if (beatsPerMinute < 255 && beatsPerMinute > 20) {
    rates[rateSpot++] = (byte)beatsPerMinute;
    rateSpot %= RATE_SIZE;

    beatAvg = 0;
    for (byte x = 0; x < RATE_SIZE; x++) beatAvg += rates[x];
    beatAvg /= RATE_SIZE;
   }
  }

 //Cálculo de SpO2
  i++;
  fred = (double)red;
  fir = (double)ir;

  avered = avered * frate + red * (1.0 - frate);
  aveir = aveir * frate + ir * (1.0 - frate);
  sumredrms += (fred - avered) * (fred - avered);
  sumirrms += (fir - aveir) * (fir - aveir);

  if ((i % SAMPLING) == 0 && millis() > TIMETOBOOT) {
   if (ir < FINGER_ON) {
    ESpO2 = MINIMUM_SPO2;
   }
  }

  if ((i % Num) == 0) {
   double R = (sqrt(sumredrms) / avered) / (sqrt(sumirrms) / aveir);
   SpO2 = -23.3 * (R - 0.4) + 100;
   ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;

   sumredrms = 0.0;
   sumirrms = 0.0;
   i = 0;
   break;
  }

  particleSensor.nextSample();
 }
#endif

 spo2 = ESpO2;
}


void readMLX() {
// --- MLX90614 --- //
 temp = mlx.readObjectTempC(); //leitura da temperatura corporal
}

void displayOled() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("SpO2: ");
  display.print(spo2, 1);
  display.println(" %");
    
  display.print("BPM: ");
  display.println(beatAvg, 1);

  display.print("Temp: ");
  display.print(temp, 1);
  display.println(" C");
  display.display();
    }
/*
    SISTEMA OPERACIONAL
*/


/*
    VOID SETUP
*/
void setup() {
 Serial.begin(115200); //monitor serial
 initI2C(); //inicia o barramento I2C
 initDisplay(); //inicia o Display 
 initSensors(); //inicia os sensores
}

/*
    VOID LOOP
*/
void loop() {
 readMAX();
 readMLX();
 displayOled();
 //delay(2000);

}
