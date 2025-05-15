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
const char* ssid = "TP-Link_0486";
const char* password = "46179951";

// const char* ssid = "777zip";
// const char* password = "R125redes";

/*
    VARIÁVEIS CONEXÃO BROKER
    IP DO SERVIDOR DO BROKER
    PORTA DO BROKER 
    ID DO CLIENTE
    TÓPICO DE SUBSCRIÇÃO
    TÓPICO DE ENVIO
*/
/*
    Conexão broker MQTT
*/
// const char* ip_broker = "192.168.0.2";  // Ou IP do seu broker local
const char* ip_broker = "192.168.1.105";  // Ou IP do seu broker local
const int broker_port = 1883;

/*
    Nome da conexão do broker <--> esp
    //depois fazer a diferenciação entre cada esp
*/
const char* client_id = "esp32_00:14:22:01:23:45";


// const char* mqtt_topic_sub = "/use_id";   // Tópico para inscrição
// const char* mqtt_topic_pub = "/dados_tipo";   // enviar dados
// const char* first_conect =  "/request_user";   // Tópico para publicação
/*
    Tópicos sem MAC
*/
String topic_pub_request_user = "request_user";
String topic_sub_response_user = "response_user";
String topic_pub_temperatura = "temperatura";
String topic_pub_bpm = "bpm";
String topic_pub_oxigenacao = "oxigenacao";



/*
    Variável que recebera o mac do dispositivo
*/
String mac_address;

/*
    Variável que recebera o id do relacionamento do esp <--> usuário
*/
String user;


bool pedio = false;

/*
    FUNÇÕES DE SETUP
*/

Preferences prefs;

WiFiClient espClient;

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


PubSubClient mqttClient(espClient);




/*
    mqttCallback
        função responsavel por receber todo mensagens enviadas aos topicos que o esp se inscreveu 
*/

void mqttCallback(char* topic, byte* payload, unsigned int length) {

    //verifica se o tipico e o correto
    if(strcmp(topic, topic_sub_response_user)){
        //joga o payload(usuario) dentro de uma variavel
        String novoUser;
        for (int i = 0; i < length; i++) {
            novoUser += (char)payload[i];
        }

        // Salvar novo valor do usuario permanentemente
        prefs.begin("config", false);
        prefs.putString("user", novoUser);
        prefs.end();

        Serial.println("Novo usuário salvo: " + novoUser);
    }else{
        Serial.println("topico errado AaaaaaaaaaaaaaaaaaHH")
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


    /*
        Atualiza os valores dos topicos adicionando o mac_address e o / a eles
    */
    topic_pub_request_user = mac_address + "/" + topic_pub_request_user;
    topic_sub_response_user = mac_address + "/" + topic_sub_response_user;
    topic_pub_temperatura = mac_address + "/" + topic_pub_temperatura;
    topic_pub_bpm = mac_address + "/" + topic_pub_bpm;
    topic_pub_oxigenacao = mac_address + "/" + topic_pub_oxigenacao;



    
}

/*
    VOID LOOP
*/


void loop() {

    if (!mqttClient.connected()) {
      reconnect_mqtt();
    }
    mqttClient.loop();
    
    if( user.length()> 0 ){
    Serial.println("ja tem usuario");
    }else{
    String conection = mac_address + first_conect;
    if (mqttClient.publish(first_conect, mac_address.c_str())) {
        Serial.println("primeira conexão");
    } else {
        Serial.println("erro na primeira conexao");
    }
    }

    delay(2000);  // Pequeno delay para evitar problemas com envio muito rápido
  
  }