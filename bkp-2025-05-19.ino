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
#include <NTPClient.h>

/*
    JSON PARA ARDUINO/ESP
*/ 
#include <ArduinoJson.h>



/*
    VARIÁVEIS DE SETUP
*/

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
/*
    Conexão broker MQTT
*/

const char* ip_broker = "10.67.23.26";  // Ou IP do seu broker local
// const char* ip_broker = "10.67.23.44";  // Ou IP do seu broker local
// const char* ip_broker = "192.168.0.2";  // Ou IP do seu broker local
// const char* ip_broker = "192.168.1.105";  // Ou IP do seu broker local

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
    //inicia o serial
    Serial.begin(115200);

    //Inicia o wifi
    setup_wifi();

    /*
        Da as informações do broker
        Ip do broker
        Porta
        Seta a função de callback que recebe todas as pensagens de subscrições no esp 
    */
    mqttClient.setServer(ip_broker, broker_port);
    mqttClient.setCallback(mqttCallback);

    timeClient.begin();
    timeClient.update();

    /*
    
    */
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


}

/*
    VOID LOOP
*/
const char* tipos[] = { "temperatura", "oxigenacao", "bpm" };

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
            valor = std::to_string(random(330, 370));
        } else if (strcmp(tipos[i], "oxigenacao") == 0) {
            valor = std::to_string(random(90, 100));
        } else if (strcmp(tipos[i], "bpm") == 0) {
            valor = std::to_string(random(60, 120));
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
  
  }
