#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <time.h>
#include <WiFi.h>

const char* ssid = "777zip";
const char* password = "R125redes";

void conectaWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

unsigned long tempoMensagem = 0;
bool mostrandoMensagem = false;

unsigned long ultimoBatimento = 0;
bool batendo = false;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Bitmaps 16x16 com fundo preto e ícone branco
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


const unsigned char epd_bitmap_termometro__1_ [] PROGMEM = {
  0xfe, 0x7f, 0xfc, 0x3f, 0xf9, 0x9f, 0xf9, 0x9f,
  0xf9, 0x9f, 0xf9, 0x9f, 0xf8, 0x1f, 0xf8, 0x1f,
  0xf8, 0x1f, 0xf8, 0x1f, 0xf2, 0x4f, 0xf0, 0x0f,
  0xf0, 0x0f, 0xf2, 0x4f, 0xf8, 0x1f, 0xfc, 0x3f
};

void initDisplay() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // Se não inicializar, trava aqui
    for(;;);
  }
  display.clearDisplay();
  display.display();
}

void displayOled(bool beat) {

  display.clearDisplay();

  String hora = getHoraAtual();
  display.setCursor(90, 4);
  display.print(hora);

  float scale = beat ? 0.8 : 1.0;
  drawHeart(scale);

  // BPM
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(20, 4);
  display.print("120 BPM");

  // Hora no canto superior direito
  display.setCursor(90, 4);
  display.print("");

  // Oxigênio
  display.drawBitmap(0, 20, epd_bitmap_gota_de_sangue, 16, 16, WHITE);
  display.setCursor(20, 24);
  display.print("90%");

  // Temperatura
  display.drawBitmap(0, 40, epd_bitmap_termometro__1_, 16, 16, WHITE);
  display.setCursor(20, 44);
  display.print("25 C");

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

void setup() {
  Serial.begin(115200); // ou 9600 se preferir
  Wire.begin();
  initDisplay();
  conectaWiFi();
  configTime(-3 * 3600, 0, "pool.ntp.org"); // Fuso horário -3h (Brasil)
}

void loop() {
  // Checa se recebeu algo no Serial
  if (Serial.available()) {
    String mensagem = Serial.readStringUntil('\n');
    mensagem.trim();

    if (mensagem.length() > 0) {
      displayMensagem(mensagem);
      tempoMensagem = millis();
      mostrandoMensagem = true;
    }
  }

  // Se está mostrando mensagem e passaram 10 segundos, volta pra tela normal
  if (mostrandoMensagem && millis() - tempoMensagem >= 10000) {
    mostrandoMensagem = false;
    display.clearDisplay(); // limpa a tela antes de voltar
  }

  // Se NÃO está mostrando mensagem, atualiza a tela normal com batimento
  if (!mostrandoMensagem && millis() - ultimoBatimento >= 500) {
    batendo = !batendo;
    displayOled(batendo);
    ultimoBatimento = millis();
  }
}