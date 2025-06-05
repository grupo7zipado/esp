/*
  ╔════════════════════════════════════════════════════════════╗
  ║                 Projeto: SafeGuard e 7life                 ║
  ║                  Plataforma: ESP32 + MQTT                  ║
  ║------------------------------------------------------------║
  ║  Descrição:                                                ║
  ║  Código desenvolvido para ler sensores de batimentos,      ║
  ║  oxigenação (SpO2) e temperatura corporal. Os dados são    ║
  ║  exibidos em um display OLED e enviados via MQTT para um   ║
  ║  servidor central para monitoramento remoto.               ║
  ║                                                            ║
  ║  Funcionalidades:                                          ║
  ║   - Leitura dos sensores MAX30102 e MLX90614 via I2C       ║
  ║   - Exibição dos dados em display SSD1306                  ║
  ║   - Comunicação via Wi-Fi + MQTT (PubSubClient)            ║
  ║   - Sincronização com horário NTP                          ║
  ║                                                            ║
  ║  Autor: grupo7zipado                                       ║
  ║  Curso: Técnico em Redes de Computadores                   ║
  ║  Instituição: ETEC Tupã Prof. Massuyuki Kawano             ║
  ║  Github: github.com/grupo7zipado                           ║
  ║                                                            ║
  ║  Data de criação: 31/05/2025                               ║
  ║  Versão: 1.1.3                                             ║
  ║  Licença: MIT                                              ║
  ╚════════════════════════════════════════════════════════════╝
*/

/*
  RECADOS

  ⚙️ CONFIGURAÇÕES DE REDE

  Altere as variáveis abaixo conforme sua rede:
  - BROKER_IP     → IP do broker MQTT
  - BROKER_PORT   → Porta do broker
  - SSID          → Nome da rede Wi-Fi
  - PASSWORD      → Senha do Wi-Fi
*/

/*
  BIBLIOTECAS
*/


/*
  Configuração de I2C (Inter-Integrated Circuit)
*/
#include <Wire.h> 

/*
  Conexão Wireless WiFi
*/
#include <WiFi.h> 

/*
  Manipulação de Memória Permanente
*/
#include <Preferences.h> 

/*
  Conexão NTP (Network Time Protocol)
*/
// #include <NTPClient.h>

#include <time.h>
/*
  Biblioteca do Sensor Oxímetro
*/
#include "MAX30105.h" 

/*
  VERIFICAR 
  USO 
  OQUE FAZ 
  SE PRESCISA
*/
#include "heartRate.h" //processamento de BPM?Oxigenação

/*
  Biblioteca do Sensor de Temperatura
*/
#include <Adafruit_MLX90614.h>

/*
  Biblioteca de Controle do Display
*/
#include <Adafruit_SSD1306.h>

/*
  Biblioteca Gráfica do Display

  VERIFICAR
  PARECE QUE NÃO TA SENDO USADA 
*/
#include <Adafruit_GFX.h>

/*
  Biblioteca do MQTT (Message Queuing Telemetry Transport)
*/
#include <PubSubClient.h>

/*
  Biblioteca de Formato JSON para IoT
  JSON (JavaScript Object Notation)
  IoT (Internet of Things)
*/
#include <ArduinoJson.h>

/*
  VARIÁVEIS DE BIBLIOTECAS
*/

/*
  INSTÂNCIAS DE OBJETOS
*/

/*
  // -----[Preferences]----- //
  Gerência o Armazenamento Dados de Forma Persistente na Memória Flash (NVS - Non-Volatile Storage)
*/
Preferences prefs;

/*
  // -----[WiFi]----- //
  Permite Conexões Wireless Possibilitando comunicar-se pela internet ou rede local
*/
WiFiClient espClient;

/*
  // -----[PubSubClient]----- //
  Comunicação Via Protocolo MQTT, Permitindo que o Microcontrlador 
  Publique Dados (Como Sensores) e Receba Comandos de um Broker MQTT
*/
PubSubClient mqttClient(espClient);

/*
  // -----[I2C]----- //
  Permite a Criação de Mais de um Barramento I2C Funcionando ao Mesmo Tempo
  VERIFICAR
*/ 
TwoWire I2C_0 = TwoWire(0);

//REMOVER DEPOIS
//TwoWire I2C_1 = TwoWire(1);



/*
  // -----[MAX30105-Oxímetro]----- //
  Objeto do Sensor MAX30105 para Leitura de SpO2 e BPM
*/
MAX30105 particleSensor;

/*
  // -----[MLX90614-Temperatura]----- //
  Objeto do Sensor MLX90614 para Leitura da Temperatura
*/
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

/*
  // -----[Display-OLED]----- //
  Objeto do Display OLED para Exibir os Dados no Display
*/
Adafruit_SSD1306 display(128, 64, &I2C_0, -1);


/*
  // -----[NTPClient]----- //
  VERIFICAR
  NÃO SEI COMO TA USANDO SE TA USANDO AQUI CHAMDNO LA EMBAIXO
*/
// Configuração do NTP
// WiFiUDP ntpUDP;
// NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600, 60000); // UTC-3 (Brasil)



/*
  VARIÁVEIS DE GLOBAIS
*/


/*
  // -----[Wifi]----- //
  SSID - da Rede Wifi
  PASSWORD - Senha da Rede Wifi
*/
const char* ssid = "777zip";
const char* password = "R125redes";

// const char* ssid = "TP-Link_0486";
// const char* password = "46179951";

// Definições do Broker MQTT
// const char* ip_broker = "192.168.1.105";  // Ou IP do seu broker local


/*
  // -----[Broker]----- //
  IP_BROKER - Ip do Serviço do Broker
  BROKER_PORT - Porta do Serviço do Broker

  VERIFICAR ACHO QUE TEM QUE GERAR DEPOIS
  TEM QUE SER UNICO AGEITAR URGENTE
  CLIENT_ID - Indentificador Único de Conexão com Broker
*/
const char* ip_broker = "10.67.23.44";  // Ou IP do seu broker local
const int broker_port = 1883;
String client_id;


/*
  // -----[Tópicos]----- //
  PUB - Tópicos de Publicação
  SUB - Tópicos de Subscrição
  REQUEST_USER - Requisição de Usuário
  RESPONSE_USER - Recebe Novo Usuário
  MSG - Recebe Mensagens
*/
String topic_pub_request_user;
String topic_sub_response_user;
String topic_sub_msg;

/*
  // -----[MAC]----- //
  Declara a Variável que Recebera o Endereço MAC do Dispositivo
*/
String mac_address;

/*
  // -----[Usuário]----- //
  Declara a Variável que Recebera o Id do Relacionamento do Esp com o Usuário
*/
String user;

/*
  // -----[Dados]----- //
*/
const char* tipos[] = { "temperatura", "oxigenacao", "bpm" };

unsigned long tempoAnterior = 0;
const unsigned long intervalo = 5000; // 5 segundos


/*
  // -----[Oxímetro]----- //
  Variáveis para Cálculos spO2 e BPM
  Variáveis de Setup
*/
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
double abpm = 0.0;
#define USEFIFO


/*
  // -----[Display]----- //
  Estados do Display
  Bitmaps
*/

// --- Estados --- //
unsigned long tempoMensagem = 0;
bool mostrandoMensagem = false;
unsigned long ultimoBatimento = 0;
bool batendo = false;

// --- Gota de Sangue (SpO2) --- //
const unsigned char epd_bitmap_gota_de_sangue [] PROGMEM = {
  0xff, 0xff, 0xfe, 0x7f, 0xfe, 0x7f, 0xfc, 0x3f,
  0xf8, 0x1f, 0xf0, 0x1f, 0xf0, 0x0f, 0xf0, 0x0f,
  0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07,
  0xf0, 0x0f, 0xf0, 0x0f, 0xfc, 0x3f, 0xff, 0xff
};
// --- Coração (BPM) --- //
const unsigned char epd_bitmap_coracao [] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xc3, 0xc3, 0x80, 0x01,
  0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x80, 0x01,
  0x80, 0x01, 0xc0, 0x03, 0xe0, 0x07, 0xf0, 0x0f,
  0xfc, 0x3f, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff
};
// --- Termômetro (Temperatura) --- //
const unsigned char epd_bitmap_termometro [] PROGMEM = {
  0xfe, 0x7f, 0xfc, 0x3f, 0xf9, 0x9f, 0xf9, 0x9f,
  0xf9, 0x9f, 0xf9, 0x9f, 0xf8, 0x1f, 0xf8, 0x1f,
  0xf8, 0x1f, 0xf8, 0x1f, 0xf2, 0x4f, 0xf0, 0x0f,
  0xf0, 0x0f, 0xf2, 0x4f, 0xf8, 0x1f, 0xfc, 0x3f
};

/*
    FUNÇÕES DE SETUP
*/
/*
  // -----[setup_wifi]----- //
  Função para conectar ao Wi-Fi
*/ 
void setup_wifi() {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }
}

/*
  // -----[mqttCallback]----- //
  Trata Mensagens Recebidas Via MQTT
  TOPIC - Tópico em que a Mensagem foi Recebida
  PAYLOAD - Conteudo da Mensagem
  LENGTH - Tamanho da Mensagem em Bytes
*/

void mqttCallback(char* topic, byte* payload, unsigned int length) {

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
    }
  String msg = "";

  if (strcmp(topic, topic_sub_msg.c_str()) == 0) {
    for (int i = 0; i < length; i++) {
      msg += (char)payload[i];
    }
    tempoMensagem = millis();
    mostrandoMensagem = true;
    displayMensagem(msg);  // ou o que você quiser fazer com a mensagem
  }

    
}


/*
  // -----[reconnect_mqtt]----- //
  Verifica se a Conexão Esta Ativa
  Conecta ao BrokerMQTT 
  Subscreve nos Tópicos response_user e msg
*/
void reconnect_mqtt() {
    while (!mqttClient.connected()) {
        if (mqttClient.connect(client_id.c_str())) {
            mqttClient.subscribe(topic_sub_response_user.c_str()); 
            mqttClient.subscribe(topic_sub_msg.c_str());
        } else {
          delay(5000);
        }
    }
}


/*
  // -----[enviarPrimeiraMensagem]----- //
  Envia o Pedido do Primeiro Usuário
*/
void enviarPrimeiraMensagem() {
    while (!mqttClient.publish(topic_pub_request_user.c_str(), mac_address.c_str())) {
        delay(3000);
    }

}

/*
  // -----[initI2C]----- //
  Define as Pinos SDA e SCL
  Inicia o Barramento I2C 
*/
void initI2C() {
  I2C_0.begin(8, 9); //I2C 0 para o MAX30102 e Display Oled //TEMPERATURA
}


/*
  // -----[initDisplay]----- //
  Inicia o Display
  Limpa o Display
  Atualiza o Display
*/
void initDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
}


/*
  // -----[confMAX30102]----- //
  Configura o Sensor MAX30102/30105


  VERIFICAR PRA MIM FAZENDO A BOA

*/
void confMAX30102() {

  byte ledBrightness = 0x7F; //intensidade do led
  byte sampleAverage = 4; //amostras para média 
  byte ledMode = 3; //modo do led
  int sampleRate = 200; //frequência de amostragem (Hz)
  int pulseWidth = 411; //duração de pulso do led
  int adcRange = 16384; // faixa de leitura adc
  
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}


/*
  // -----[initSensors]----- //
  Inicia o Sensor de Oxímetro no Barramento I2C_0
  Inicia o Sensor de Temperatura no Barramento I2C_0
  Envia as Configurações do Oxímetro
  Define a Frequência de Comunicação do Barramento I2C
*/
void initSensors() {
  particleSensor.begin(I2C_0); //inicia sensor MAX30102 (velocidade padrão 100khz) 
  I2C_0.setClock(100000); //altere a velocidade entre 100/400 kHz
  confMAX30102();
  mlx.begin(0x5A, &I2C_0); //inicia o sensor MLX90614

}


/*
  // -----[drawHeart]----- //

  QUEM FEZ COMENTA AI RESUMO DOQUE FAZ 
*/
void drawHeart(float scale) {
  int centerX = 8;
  int centerY = 8;

  for (int y = 0; y < 16; y++) {
    for (int x = 0; x < 16; x++) {
      if (pgm_read_byte(&epd_bitmap_coracao[y * 2 + (x / 8)]) & (1 << (7 - (x % 8)))) {
        int dx = x - centerX;
        int dy = y - centerY;
        int sx = centerX + dx * scale;
        int sy = centerY + dy * scale;
        if (sx >= 0 && sx < 128 && sy >= 0 && sy < 64) {
          display.drawPixel(sx, sy, WHITE);
        }
      }
    }
  }
}

/*
  // -----[drawHeart]----- //
  Retorna a Hora e Minutos (HH:MM)
*/
String getHoraAtual() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char buffer[6];
  strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);
  return String(buffer);
}


/*
    FUNÇÕES DE LEITURA
*/

/*
  // -----[readMAX]----- //
  Faz a Leitura e Cálculo do SpO2 e BPM
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
    abpm = beatAvg;
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

/*
  // ------[readMLX]----- //
  Faz a Leitura da Temperatura
*/
void readMLX() {
 temp = mlx.readObjectTempC(); //leitura da temperatura corporal
}

/*
  // ------[displayOled]----- //
  Cria o Cérebro Visual do Projeto
*/
void displayOled(bool beat) {
  display.clearDisplay();

  // Hora
  String hora = getHoraAtual();
  display.setCursor(90, 4);
  display.print(hora);

  // Batimento
  float scale = beat ? 0.8 : 1.0;
  drawHeart(scale);

  // BPM
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 4);
  display.print(abpm);
  display.print("BPM");

  // Oxigênio
  display.drawBitmap(0, 20, epd_bitmap_gota_de_sangue, 16, 16, WHITE);
  display.setCursor(20, 24);
  display.print(spo2);
  display.print("%");
  // Temperatura
  display.drawBitmap(0, 40, epd_bitmap_termometro, 16, 16, WHITE);
  display.setCursor(20, 44);
  display.print(temp);
  display.print("°C");

  display.display();
}


void atualizarTopicos(){
  topic_pub_request_user = mac_address + "/request_user";
  topic_sub_response_user = mac_address + "/response_user";
  topic_sub_msg = mac_address + "/msg";
}


/*
  // ------[displayMensagem]----- //
  Cria Display para o Evento de Mensagem
*/
void displayMensagem(String msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("MENSAGEM:");
  display.setCursor(0, 16);
  display.println(msg);
  display.display();
}


/*
  VOID SETUP
  // ------[setup]----- //
  Inicia o Wifi
  Configura a Hora Interna do Esp
  Atualisa para o Usuário Salvo
  Captura o Mac Address do Esp
  Inicia as Funções de Inicialização dos Sensores, I2C e Display

  VERIFICAR
  TIRAR O SERIAL
  VERIFICA O NPT PRA FAZER O HORARIO INTERNO DO ESP FICAR CORETO
  PROVAVELMENTE REMOVER O NPT CLIENT E USAR SO O CONFIGTIME
  DESATIVAR O RESET DE USUARIO .remove()
  DEIXAR OS TOPICOS DINAMICOS
*/
void setup() {
  
  setup_wifi();
  mqttClient.setServer(ip_broker, broker_port);
  mqttClient.setCallback(mqttCallback);

  // timeClient.begin();
  // timeClient.update();
  configTime(-3 * 3600, 0, "pool.ntp.org");

  // Abrir namespace "config" no modo leitura/escrita
  prefs.begin("config", false);
  // temporario para testes
  //prefs.remove("user");
  // Ver se já existe valor salvo
  user = prefs.getString("user", "");
  mac_address = WiFi.macAddress();
  client_id = "ESP_"+ mac_address; 
  atualizarTopicos();
  initI2C(); //inicia o barramento I2C
  initDisplay(); //inicia o Display 
  initSensors(); //inicia os sensores
}

/*
    VOID LOOP
*/



/*
  // ------[loop]----- //
  Verifica a Conexão com broker
  Mantem a Conexão Viva e Processa Mensagens
  Verifica a Existência do Usuário

  VERIVICAR
  Faz a  leitura dos sinais vitais e envia os dados
  FAZER UM FUNÇÃO DE RECONECT DO WIFI SE ELE NÃO OCNSEGUIR SE CONECTAR NO BROKER IF PRA VE SE TAR CONECTADO NO WIFI SE NÃO TENTA CONECTAR

*/
void loop() {
  if (!mqttClient.connected()) {
      if(WiFi.status() != WL_CONNECTED){
        setup_wifi();
      }
      reconnect_mqtt();  // sua função de reconexão
    }
    mqttClient.loop();

    readMLX();
    readMAX();
    

    // Verifica se o user está vazio
    if (user == "") {
        static bool primeiraMensagemEnviada = false;
        //pede um novo usuário
        if (!primeiraMensagemEnviada) {
            enviarPrimeiraMensagem();
            primeiraMensagemEnviada = true;
        }
    } else {
        unsigned long tempoAtual = millis();

        // Se passaram 5 segundos?
        if (tempoAtual - tempoAnterior >= intervalo) {
            tempoAnterior = tempoAtual;
            // Aqui você coloca o que deseja fazer a cada 5 segundos
            for (int i = 0; i < 3; i++) {
            String valor;
            
            if (strcmp(tipos[i], "temperatura") == 0) {
                valor = String(temp, 2); // duas casas decimais

            } else if (strcmp(tipos[i], "oxigenacao") == 0) {
                valor = String(spo2, 2);
                
            } else if (strcmp(tipos[i], "bpm") == 0) {
                valor = String(abpm);

            }

            StaticJsonDocument<200> doc;
            doc["use_id"] = user;
            doc["dados_tipo"] = tipos[i];
            doc["dados_valor"] = valor;
            doc["dados_generate"] = time(NULL);

            String e;
            serializeJson(doc, e);

            String topico = mac_address+ "/" + tipos[i];
            // 🚀 Verifica se a mensagem foi publicada com sucesso
            mqttClient.publish(topico.c_str(), e.c_str());
        }
      }
    }
    
  if (mostrandoMensagem && millis() - tempoMensagem >= 10000) {
    mostrandoMensagem = false;
    display.clearDisplay();
  }

  if (!mostrandoMensagem && millis() - ultimoBatimento >= 500) {
    batendo = !batendo;
    displayOled(batendo);
    ultimoBatimento = millis();
  }
}
