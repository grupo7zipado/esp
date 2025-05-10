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

const char* ssid = "777zip";
const char* password = "R125redes";
/*
    VARIÁVEIS CONEXÃO BROKER
    IP DO SERVIDOR DO BROKER
    PORTA DO BROKER 
    ID DO CLIENTE
    TÓPICO DE SUBSCRIÇÃO
    TÓPICO DE ENVIO
*/
// const char* ip_broker = "192.168.1.105";  // Ou IP do seu broker local
const char* ip_broker = "192.168.0.2";  // Ou IP do seu broker local
const int broker_port = 1883;
const char* client_id = "esp32_00:14:22:01:23:45";
const char* mqtt_topic_sub = "/use_id";   // Tópico para inscrição
const char* mqtt_topic_pub = "/dados_tipo";   // enviar dados
const char* first_conect =  "/request_user";   // Tópico para publicação

String mac_address;
String user;
bool pedio = false;
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
            String topico = mac_address.c_str()+mqtt_topic_sub;
            mqttClient.subscribe(topico);  // Inscreve-se no tópico
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
    if(pedio){
      Serial.println("ja pedio o usuário");
    }else{
      if( user.length()> 0 ){
        Serial.println("ja tem usuario");
      }else{
        String conection = mac_address + first_conect;
        if (mqttClient.publish(first_conect, mac_address.c_str())) {
            Serial.println("primeira conexão");
        } else {
            Serial.println("erro na primeira conexao");
        }
        pedio = true;
      }
    }
    //  if (mqttClient.publish(mqtt_topic_pub, temp)) {
    //  }
    delay(2000);  // Pequeno delay para evitar problemas com envio muito rápido
  
  }