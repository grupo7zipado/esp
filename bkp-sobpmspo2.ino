#define BLYNK_TEMPLATE_ID "TMPL2WCYK3C-W" 
#define BLYNK_TEMPLATE_NAME "ESP32"
#define BLYNK_AUTH_TOKEN "Ps4x-azvYfIefO9nQjBK-Orcbyc-N5kh"

/*
    BIBLIOTECAS
*/

#include <Wire.h>
#include <WiFi.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_MLX90614.h>
#include <BlynkSimpleEsp32.h>

/*
    CONFIGURAÇÃO DO BLYNK E WI-FI
*/

// Substitua pelos seus dados:

char auth[] = "Ps4x-azvYfIefO9nQjBK-Orcbyc-N5kh";     // Token do Blynk
// char ssid[] = "12S";       // Nome da rede WiFi
// char pass[] = "midi0303";          // Senha da rede WiFi
char ssid[] = "777zip";       // Nome da rede WiFi
char pass[] = "R125redes";          // Senha da rede WiFi



// Inicialização do Blynk

BlynkTimer timer;

/*
    VARIÁVEIS DO SENSOR
*/

TwoWire I2C_0 = TwoWire(0);
MAX30105 particleSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

double avered = 0, aveir = 0, sumirrms = 0, sumredrms = 0;
int i = 0;
int Num = 200;
double ESpO2 = 95.0;
double FSpO2 = 0.7;
double frate = 0.95;

#define TIMETOBOOT 3000
#define SAMPLING 5
#define FINGER_ON 30000
#define MINIMUM_SPO2 80.0

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;
double spo2 = 0.0;
double temp = 0.0;
double abpm = 0.0;

#define USEFIFO

/*
    FUNÇÕES DE SETUP
*/

void initI2C() {
  I2C_0.begin(21, 22);
  I2C_0.setClock(100000);
}

void confMAX30102() {
  byte ledBrightness = 0x7F;
  byte sampleAverage = 4;
  byte ledMode = 3;
  int sampleRate = 200;
  int pulseWidth = 411;
  int adcRange = 16384;

  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}



void initSensors() {
  if (!particleSensor.begin(I2C_0)) {
    Serial.println("Erro: MAX30102 não encontrado.");
    while (1);
  }

  confMAX30102();
  // if (!mlx.begin(0x5A, &I2C_0)) {
  //   Serial.println("Erro: MLX90614 não encontrado.");
  //   while (1);
  // }
}

/*
    FUNÇÕES DE LEITURA
*/

void readMAX() {
  uint32_t ir, red;
  double fred, fir;
  double SpO2 = 0;

#ifdef USEFIFO

  particleSensor.check();
  while (particleSensor.available()) {
    red = particleSensor.getFIFORed();
    ir = particleSensor.getFIFOIR();

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
        abpm = beatAvg;
      }
    }

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
  temp = mlx.readObjectTempC();
}

// Envia dados ao Blynk

void sendToBlynk() {
  Blynk.virtualWrite(V0, spo2);
  Blynk.virtualWrite(V1, beatAvg);
  Blynk.virtualWrite(V2, temp);
}

/*
    SETUP
*/

void setup() {
  Serial.begin(115200); //monitor serial
  initI2C();
  initSensors();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado com sucesso!");
  //Blynk.begin(auth, ssid, pass);
  //timer.setInterval(5000L, sendToBlynk); // envia dados a cada 5 segundos
}

/*
    LOOP
*/

void loop() {
  //Blynk.run();
  //timer.run();

  readMAX();
  Serial.println("BPM");
  Serial.println(abpm);
  Serial.println("OXIGENAÇÃO");
  Serial.println(spo2);

  
  
  //readMLX();
  //Serial.println("TEMPERATURA");
  //Serial.println(temp);

}