//dados... temperatura, oxi e bpm

//configurar primeiro acesso do esp: (requerimento MAC), visualizar MAC no display apoós sucesso no requerimento anterior

//
//

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

// const char* ssid = "TP-Link_0486";
// const char* password = "46179951";

// const char* ssid = "777zip";
// const char* password = "R125redes";

const char* ssid = "R124";
const char* password = "R124@redes";


/*
    VARIÁVEIS CONEXÃO BROKER
    IP DO SERVIDOR DO BROKER
    PORTA DO BROKER 
    ID DO CLIENTE
    TÓPICO DE SUBSCRIÇÃO
    TÓPICO DE ENVIO
*/
// const char* ip_broker = "192.168.1.105";  // Ou IP do seu broker local
const char* ip_broker = "10.67.23.44";  // Ou IP do seu broker local
const int broker_port = 1883;
const char* client_id = "esp32_00:14:22:01:23:45";


const char* topic_pub_request_user = "/request_user";
const char* topic_sub_recibe_user = "/recibe_user";


String mac_address;
String user;

/*
    FUNÇÕES DE SETUP
*/

Preferences prefs;

WiFiClient espClient;

PubSubClient mqttClient(espClient);


// Callback quando receber uma mensagem MQTT
void recibe_user(char* topic, byte* payload, unsigned int length) {
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
    // temporario para testes
    prefs.remove("user");
    // Ver se já existe valor salvo
    user = prefs.getString("user", "");
    mac_address = WiFi.macAddress();

    Serial.println("Usuário atual: " + user);
    Serial.println("Mac: " + mac_address);


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