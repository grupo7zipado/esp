#include <WiFi.h>
#include <PubSubClient.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <vector>

// Definições da rede Wi-Fi
const char* ssid = "TP-Link_0486";
const char* password = "46179951";

// Definições do Broker MQTT
const char* mqtt_broker = "192.168.1.105";  // Ou IP do seu broker local
const int mqtt_port = 1883;
const char* mqtt_client_id = "ESP32_MQTT";
const char* mqtt_topic_sub = "esp32/teste";   // Tópico para inscrição
const char* mqtt_topic_pub = "esp32/envio";   // Tópico para publicação

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Configuração do NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -3 * 3600, 60000); // UTC-3 (Brasil)


// Callback quando receber uma mensagem MQTT
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print(" Mensagem recebida no tópico: ");
    Serial.println(topic);

    Serial.print(" Mensagem: ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
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
        if (mqttClient.connect(mqtt_client_id)) {
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

const char* tipos[] = { "temperatura", "oxigenacao", "bpm" };

void setup() {
  Serial.begin(115200);
  setup_wifi();

  mqttClient.setServer(mqtt_broker, mqtt_port);
  mqttClient.setCallback(mqttCallback);
  timeClient.begin();
  timeClient.update();
}
std::vector<char*> memoria = {};
void loop() {

  if (!mqttClient.connected()) {
    reconnect_mqtt();
  }
  mqttClient.loop();

  // put your main code here, to run repeatedly:
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
    doc["use_id"] = 1;
    doc["dados_tipo"] = tipos[i];
    doc["dados_valor"] = valor;
    doc["dados_generate"] = timeClient.getEpochTime();

    String msg;
    serializeJson(doc, msg);

    // 🚀 Verifica se a mensagem foi publicada com sucesso
    if (mqttClient.publish(mqtt_topic_pub, msg.c_str())) {
      Serial.println("✅ Mensagem enviada: " + msg);
    } else {
      memoria.push_back(msg.c_str());
      Serial.println(memoria);
      Serial.println("❌ ERRO ao enviar: " + msg);
    }

    delay(2000);  // Pequeno delay para evitar problemas com envio muito rápido
  }

}
