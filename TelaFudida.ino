#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>

// ========== CONFIG Wi-Fi ==========
const char* ssid = "777zip";
const char* password = "R125redes";

// ========== CONFIG MQTT ==========
const char* mqtt_server = "10.67.23.44";
const char* mqtt_topic = "kiraz/cliente/mensagem";

WiFiClient espClient;
PubSubClient client(espClient);

// ========== DISPLAY ==========
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ========== ESTADOS ==========
unsigned long tempoMensagem = 0;
bool mostrandoMensagem = false;
unsigned long ultimoBatimento = 0;
bool batendo = false;

// ========== BITMAPS ==========
const unsigned char epd_bitmap_gota_de_sangue [] PROGMEM = {
  0xff, 0xff, 0xfe, 0x7f, 0xfe, 0x7f, 0xfc, 0x3f,
  0xf8, 0x1f, 0xf0, 0x1f, 0xf0, 0x0f, 0xf0, 0x0f,
  0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07, 0xe0, 0x07,
  0xf0, 0x0f, 0xf0, 0x0f, 0xfc, 0x3f, 0xff, 0xff
};

const unsigned char epd_bitmap_coracao [] PROGMEM = {
  0xff, 0xff, 0xff, 0xff, 0xc3, 0xc3, 0x80, 0x01,
  0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x80, 0x01,
  0x80, 0x01, 0xc0, 0x03, 0xe0, 0x07, 0xf0, 0x0f,
  0xfc, 0x3f, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff
};

const unsigned char epd_bitmap_termometro [] PROGMEM = {
  0xfe, 0x7f, 0xfc, 0x3f, 0xf9, 0x9f, 0xf9, 0x9f,
  0xf9, 0x9f, 0xf9, 0x9f, 0xf8, 0x1f, 0xf8, 0x1f,
  0xf8, 0x1f, 0xf8, 0x1f, 0xf2, 0x4f, 0xf0, 0x0f,
  0xf0, 0x0f, 0xf2, 0x4f, 0xf8, 0x1f, 0xfc, 0x3f
};

// ========== FUNÇÕES ==========
void conectaWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void reconnectMQTT() {
  while (!client.connected()) {
    if (client.connect("ESP32_Display")) {
      client.subscribe(mqtt_topic);
    } else {
      delay(2000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  displayMensagem(msg);
  tempoMensagem = millis();
  mostrandoMensagem = true;
}

void initDisplay() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for (;;); // trava se não encontrar o display
  }
  display.clearDisplay();
  display.display();
}

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

String getHoraAtual() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char buffer[6];
  strftime(buffer, sizeof(buffer), "%H:%M", timeinfo);
  return String(buffer);
}

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
  display.print("120 BPM");

  // Oxigênio
  display.drawBitmap(0, 20, epd_bitmap_gota_de_sangue, 16, 16, WHITE);
  display.setCursor(20, 24);
  display.print("90%");

  // Temperatura
  display.drawBitmap(0, 40, epd_bitmap_termometro, 16, 16, WHITE);
  display.setCursor(20, 44);
  display.print("25 C");

  display.display();
}

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

// ========== SETUP ==========
void setup() {
  Serial.begin(115200);
  Wire.begin();
  initDisplay();
  conectaWiFi();
  configTime(-3 * 3600, 0, "pool.ntp.org");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// ========== LOOP ==========
void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();

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