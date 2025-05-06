/*
    BIBLIOTECAS
*/


/*
    WIFI
*/ 
#include <WiFi.h>

/*
    MQTT
*/
#include <PubSubClient.h>

/*
    MANIPULA A MEMÓRIA PERMANENTE
*/
#include <Preferences.h>


/*
    HORA EM TEMPO REAL
*/ 
//#include <NTPClient.h>

/*
    JSON PARA ARDUINO/ESP
*/ 
//#include <ArduinoJson.h>



/*
    VARIÁVEIS DE SETUP
*/

/*
    VARIÁVEIS CONEXÃO WIFI
    SSID
    SENHA
*/
const char* ssid = "TP-Link_0486";
const char* password = "46179951";

/*
    VARIÁVEIS CONEXÃO BROKER
    IP DO SERVIDOR DO BROKER
    PORTA DO BROKER 
    ID DO CLIENTE
    TÓPICO DE SUBSCRIÇÃO
    TÓPICO DE ENVIO
*/
const char* ip_broker = "192.168.1.105";  // Ou IP do seu broker local
const int broker_port = 1883;
const char* client_id = "ESP32_MQTT";
const char* mqtt_topic_sub = "esp32/teste";   // Tópico para inscrição
const char* mqtt_topic_pub = "esp32/envio";   // Tópico para publicação




/*
    FUNÇÕES DE SETUP
*/

Preferences prefs;

WiFiClient espClient;

PubSubClient mqttClient(espClient);


// Callback quando receber uma mensagem MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Serial.print(" Mensagem recebida no tópico: ");
    // Serial.println(topic);

    // Serial.print(" Mensagem: ");
    // for (int i = 0; i < length; i++) {
    //     Serial.print((char)payload[i]);
    // }
    // Serial.println();
    String novoUser;
    for (int i = 0; i < length; i++) {
        novoUser += (char)payload[i];
    }

    // Salvar novo valor permanentemente
    prefs.begin("config", false);
    prefs.putString("user", novoUser);
    prefs.end();

    Serial.println("Novo usuário salvo: " + novoUser);
}

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

// Função para conectar ao Broker MQTT
void reconnect_mqtt() {
    while (!mqttClient.connected()) {
        Serial.print(" Conectando ao Broker MQTT...");
        if (mqttClient.connect(client_id)) {
            Serial.println(" Conectado!");
            mqttClient.subscribe(mqtt_topic_sub);  // Inscreve-se no tópico
        } else {
            Serial.print(" Falha, código: ");
            Serial.print(mqttClient.state());
            Serial.println(" Tentando novamente em 5s...");
            delay(5000);
        }
    }
}


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
    setup_wifi();

    mqttClient.setServer(ip_broker, broker_port);
    mqttClient.setCallback(mqttCallback);
    // Abrir namespace "config" no modo leitura/escrita
    prefs.begin("config", false);

    // Ver se já existe valor salvo
    String user = prefs.getString("user", "ze da manga");
    Serial.println("Usuário atual: " + user);
}

/*
    VOID LOOP
*/


void loop() {

    if (!mqttClient.connected()) {
      reconnect_mqtt();
    }
    mqttClient.loop();
    
    // if (mqttClient.publish(mqtt_topic_pub, "qualquer coisa")) {
    //     Serial.println("✅ Mensagem enviada: ");
    // } else {
    //     Serial.println("❌ ERRO ao enviar: ");
    // }
    delay(2000);  // Pequeno delay para evitar problemas com envio muito rápido
  
  }