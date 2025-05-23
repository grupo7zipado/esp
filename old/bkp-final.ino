/*
    BIBLIOTECAS
*/
#include <Wire.h> //configuração de I2C
#include <WiFi.h> //biblioteca WiFi
#include <Preferences.h> //manipulação de memoria
#include <NTPClient.h> //data e hora
#include "MAX30105.h" //sensor BPM/Oxigenação
#include "heartRate.h" //processamento de BPM?Oxigenação
#include <Adafruit_MLX90614.h> //sensor de temperatura
#include <Adafruit_SSD1306.h> //controle do display
#include <Adafruit_GFX.h> //biblioteca grafica do display
#include <PubSubClient.h> //broker MQTT
#include <ArduinoJson.h> //dados em formato Json

/*
    VARIÁVEIS DE BIBLIOTECAS
*/

Preferences prefs;

WiFiClient espClient;

PubSubClient mqttClient(espClient);

// Configuração do NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600, 60000); // UTC-3 (Brasil)

/*
    VARIÁVEIS DE SETUP
*/
//  [Conexões Externas]
const char* ssid = "777zip";  //nome da rede
const char* password = "R125redes";

// const char* ip_broker = "10.67.23.26";  //ou IP do seu broker local
const int broker_port = 1883; //porta do broker
const char* client_id = "esp32_00:14:22:01:23:45"; //id do equipamento conectado

/*
    Tópicos sem MAC
*/
String topic_pub_request_user = "request_user";
String topic_sub_response_user = "response_user";
String topic_pub_temperatura = "temperatura";
String topic_pub_bpm = "bpm";
String topic_pub_oxigenacao = "oxigenacao";


String mac_address; // Variável que recebera o mac do dispositivo

String user; // Variável que recebera o id do relacionamento do esp <--> usuário


//  [Conexões Internas]
// --- Interface I2C --- //
TwoWire I2C_0 = TwoWire(0);

// --- Sensores e display --- //
MAX30105 particleSensor;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SSD1306 display(128, 64, &I2C_0, -1);


// --- Variáveis para cálculos spO2 e BPM --- //
double avered = 0; // Média leitura de luz red
double aveir = 0; // Média leitura de luz infrared
double sumirrms = 0;
double sumredrms = 0;
int i = 0;
int Num = 200; // Intervalo de amostragem para o cálculo de SpO2
double ESpO2 = 95.0; // Valor inicial de SpO2 estimado
double FSpO2 = 0.7;  // Fator de filtro para SpO2 estimado
double frate = 0.95; // Filtro passa-baixo para o valor do LED IR/vermelho

#define TIMETOBOOT 3000 // Tempo de espera (ms) para a saída do SpO2
#define SCALE 88.0 // Ajuste para exibir batimento cardíaco e SpO2 na mesma escala
#define SAMPLING 5 // Mais preciso se definido como 1
#define FINGER_ON 30000 // Se o sinal vermelho for menor que isso, indica que o dedo não está no sensor
#define MINIMUM_SPO2 80.0 // Mínimo inicial da oxigenação

const byte RATE_SIZE = 4; // Aumente isso para mais média. 4 é bom.
byte rates[RATE_SIZE]; // Array de batimentos cardíacos
byte rateSpot = 0;
long lastBeat = 0; // Hora em que ocorreu o último batimento
float beatsPerMinute; 

int beatAvg; // Váriavel de BPM
double spo2 = 0.0; // Váriavel de Oxigenação
double temp = 0.0; // Váriavel de temperatura

#define USEFIFO


/*
    FUNÇÕES DE SETUP
*/
// Função para conectar ao Wi-Fi
void setup_wifi() {
    Serial.print(" Conectando ao Wi-Fi: ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\n Wi-Fi conectado!");
    Serial.print(" IP: ");
    Serial.println(WiFi.localIP());
}

/*
    mqttCallback
        função responsavel por receber todo mensagens enviadas aos topicos que o esp se inscreveu 
*/

void mqttCallback(char* topic, byte* payload, unsigned int length) {


    //LOGS TOPICO RECEBIDO
    Serial.print("Recebido topic: [");
    Serial.print(topic);
    Serial.println("]");

    //LOGS TOPICO ESPERADO
    Serial.print("Esperado topic: [");
    Serial.print(topic_sub_response_user.c_str());
    Serial.println("]");
    //MENSAGEM RECEBIDA
    Serial.print("Payload: ");
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);  // converte byte para char
    }
    Serial.println();

    //verifica se o topico e o correto
    if(strcmp(topic, topic_sub_response_user.c_str()) == 0){
        //joga o payload(usuario) dentro de uma variavel
        String novoUser;
        for (int i = 0; i < length; i++) {
            novoUser += (char)payload[i];
        }

        // Salvar novo valor do usuario permanentemente
        prefs.begin("config", false);
        prefs.putString("user", novoUser);
        prefs.end();
        // Atualiza o valor do usuário no codigo
        user = novoUser;
        Serial.println("Novo usuário salvo: " + novoUser);
    }else{
        Serial.println("topico errado AaaaaaaaaaaaaaaaaaHH");
    }
    
}



/*
    reconnect_mqtt
    Conecta ao brokerMQTT e verifica se a conexão esta ativa senão tenta conectar
*/
void reconnect_mqtt() {
    while (!mqttClient.connected()) {
        Serial.print(" Conectando ao Broker MQTT...");
        if (mqttClient.connect(client_id)) {
            Serial.println(" Conectado!");
            //SE INCREVE DO TOPICO PARA RECEBER NOVO ESPUSUARIO
            mqttClient.subscribe(topic_sub_response_user.c_str());  // Inscreve-se no tópico
        } else {
            Serial.print(" Falha, código: ");
            Serial.print(mqttClient.state());
            Serial.println(" Tentando novamente em 5s...");
            delay(5000);
        }
    }
}

//TESTE BLUBLICAÇÃO
void enviarPrimeiraMensagem() {
    while (!mqttClient.publish(topic_pub_request_user.c_str(), mac_address.c_str())) {
        Serial.println("Erro ao publicar. Tentando novamente em 3 segundos...");
        delay(3000);
    }

    Serial.println("✅ Publicação bem-sucedida no tópico: " + topic_pub_request_user);
}

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
mlx.begin(0x5A, &I2C_0); //inicia o sensor MLX90614

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

 BPM = beatAvg;

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
  display.println(BPM, 1);

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
 setup_wifi();
 mqttClient.setServer(ip_broker, broker_port);
    mqttClient.setCallback(mqttCallback);

    timeClient.begin();
    timeClient.update();

     // Abrir namespace "config" no modo leitura/escrita
    prefs.begin("config", false);
    // temporario para testes
    //prefs.remove("user");
    // Ver se já existe valor salvo
    user = prefs.getString("user", "");
    mac_address = WiFi.macAddress();

    Serial.println("Usuário atual: " + user);
    Serial.println("Mac: " + mac_address);


    /*
        Atualiza os valores dos topicos adicionando o mac_address e o / a eles
    */
    topic_pub_request_user = mac_address + "/" + topic_pub_request_user;
    topic_sub_response_user = mac_address + "/" + topic_sub_response_user;

    //REVER OS TOPICOS DE DADOS E TORNA ELES DINAMICOS
    topic_pub_temperatura = mac_address + "/" + topic_pub_temperatura;
    topic_pub_bpm = mac_address + "/" + topic_pub_bpm;
    topic_pub_oxigenacao = mac_address + "/" + topic_pub_oxigenacao;

    Serial.println("topic_pub_request_user: " + topic_pub_request_user);
    Serial.println("topic_sub_response_user: " + topic_sub_response_user);
    Serial.println("topic_pub_temperatura: " + topic_pub_temperatura);
    Serial.println("topic_pub_bpm: " + topic_pub_bpm);
    Serial.println("topic_pub_oxigenacao: " + topic_pub_oxigenacao);


 initI2C(); //inicia o barramento I2C
 initDisplay(); //inicia o Display 
 initSensors(); //inicia os sensores
}

const char* tipos[] = { "temperatura", "oxigenacao", "bpm" };

/*
    VOID LOOP
*/
void loop() {
      if (!mqttClient.connected()) {
        reconnect_mqtt();  // sua função de reconexão
    }

    mqttClient.loop();
    
    

    // Verifica se o user está vazio
    if (user == "") {

        Serial.println("Usuário não definido.");
        static bool primeiraMensagemEnviada = false;
        //pede um novo usuário
        if (!primeiraMensagemEnviada) {
            enviarPrimeiraMensagem();
            primeiraMensagemEnviada = true;
        }
    } else {
       for (int i = 0; i < 3; i++) {
        std::string valor;

        if (strcmp(tipos[i], "temperatura") == 0) {
            valor = std::to_string(temp);
        } else if (strcmp(tipos[i], "oxigenacao") == 0) {
            valor = std::to_string(spo2);
        } else if (strcmp(tipos[i], "bpm") == 0) {
            valor = std::to_string(BPM);
        }

        StaticJsonDocument<200> doc;
        doc["use_id"] = user;
        doc["dados_tipo"] = tipos[i];
        doc["dados_valor"] = valor;
        doc["dados_generate"] = timeClient.getEpochTime();

        String msg;
        serializeJson(doc, msg);

        String topicoaaa = mac_address+ "/" + tipos[i];
        // 🚀 Verifica se a mensagem foi publicada com sucesso
        if (mqttClient.publish(topicoaaa.c_str(), msg.c_str())) {
          Serial.println("✅ Mensagem enviada: " + msg);
        } else {
          Serial.println("❌ ERRO ao enviar: " + msg);
        }

        delay(2000);  // Pequeno delay para evitar problemas com envio muito rápido
      }
        Serial.println("Usuário salvo: " + user);
    }
    

    delay(2000);  // Pequeno delay para evitar problemas com envio muito rápido
  
 readMAX();
 readMLX();
 displayOled();
}
