# 🩺 Sistema de Monitoramento de Sinais Vitais com ESP32, MQTT, Backend e Web

Este projeto tem como objetivo o monitoramento remoto de sinais vitais como temperatura corporal, batimentos cardíacos (BPM) e oxigenação do sangue (SpO₂), utilizando o microcontrolador ESP32 com sensores específicos, um broker MQTT, backend em Node.js e uma interface web construída em React.

---

## 🤖 Parte ESP32 (Firmware)

Essa parte roda diretamente no ESP32 e é responsável por:

- Ler os dados dos sensores (temperatura, batimentos e oxigenação);
- Exibir essas informações no display OLED;
- Conectar-se ao Wi-Fi;
- Enviar os dados via MQTT para o broker.

### 📦 Bibliotecas Utilizadas

| Biblioteca                  | Função no Projeto |
|----------------------------|-------------------|
| `<Wire.h>`                 | Habilita a comunicação via barramento I2C, utilizado para conectar os sensores e o display ao ESP32. |
| `<WiFi.h>`                 | Responsável por conectar o ESP32 à rede Wi-Fi, permitindo envio dos dados para o broker MQTT. |
| `<Preferences.h>`          | Permite armazenar dados persistentes na memória flash, como o usuário, mesmo após reinicialização. |
| `<time.h>`                 | Utilizada para sincronizar o horário via NTP, permitindo enviar dados com timestamp correto. |
| `"MAX30105.h"`             | Controla o sensor MAX30102/30105, responsável por ler batimentos cardíacos e oxigenação do sangue. |
| `"heartRate.h"`            | Complementar ao sensor MAX3010x, usada para processar os sinais e calcular BPM e SpO₂. |
| `<Adafruit_MLX90614.h>`   | Controla o sensor de temperatura sem contato MLX90614, utilizado para medir a temperatura corporal. |
| `<Adafruit_SSD1306.h>`    | Usada para controlar o display OLED, onde os dados vitais são exibidos localmente. |
| `<PubSubClient.h>`        | Implementa o protocolo MQTT, permitindo que o ESP32 publique os dados lidos no broker em tempo real. |
| `<ArduinoJson.h>`         | Usada para formatar os dados em JSON, facilitando o envio estruturado via MQTT para o backend. |

---

## 🖥️ Parte Web (Frontend)

Essa é a interface do sistema de monitoramento de sinais vitais, feita em React. É onde os dados de temperatura, batimentos cardíacos e oxigenação recebidos do ESP32 via MQTT são apresentados visualmente ao usuário. A interface consome os dados enviados pelo broker e pela API, exibindo gráficos e dados em tempo real.

### 📦 Bibliotecas Utilizadas

| Biblioteca       | Versão     | Função no Projeto |
|------------------|------------|-------------------|
| `axios`          | ^1.8.1     | Cliente HTTP para se comunicar com a API REST. Usado para buscar dados do banco, como históricos. |
| `lucide-react`   | ^0.511.0   | Biblioteca de ícones SVG para React. Representa temperatura, batimentos, etc. |
| `mqtt`           | ^5.10.4    | Cliente MQTT em JavaScript. Se conecta ao broker via WebSocket para receber dados em tempo real. |
| `react`          | ^19.0.0    | Biblioteca base para construir a interface do usuário. |
| `react-dom`      | ^19.0.0    | Responsável por renderizar os componentes React no HTML. |
| `recharts`       | ^2.15.1    | Biblioteca de gráficos SVG. Exibe os dados em forma de gráficos de linha e área. |
| `sweetalert2`    | ^11.21.0   | Biblioteca de alertas visuais e personalizados. Usada para pop-ups de sucesso/erro. |

---

## 🔧 Parte API (Backend)

Essa é a camada de aplicação responsável por fornecer uma interface REST para o frontend. Ela **não se comunica diretamente com o ESP32** — sua função é consultar e armazenar dados no banco de dados, permitindo que a interface web acesse históricos, registros e outros dados relevantes de forma segura e organizada.

### 📦 Bibliotecas Utilizadas

| Biblioteca    | Versão     | Função no Projeto |
|---------------|------------|-------------------|
| `cors`        | ^2.8.5     | Middleware para habilitar requisições entre diferentes origens (CORS). Necessário para React se comunicar com a API. |
| `dotenv`      | ^16.4.7    | Carrega variáveis de ambiente do `.env`, como credenciais do banco. |
| `express`     | ^4.21.2    | Framework web. Define rotas, trata requisições e respostas da API. |
| `mysql2`      | ^3.12.0    | Driver para conectar ao banco MySQL. Armazena e consulta dados dos sinais vitais. |

---

## 📡 Parte Broker MQTT (aedes)

Essa é a **ponte principal entre o ESP32 e o sistema**. O broker MQTT recebe as mensagens publicadas pelo ESP (com dados vitais) e:

- Armazena essas informações diretamente no banco de dados (MySQL);
- Permite que o frontend se conecte via WebSocket MQTT e receba os dados em tempo real.

Tudo isso é feito de forma leve, eficiente e sem depender de serviços externos.

### 📦 Bibliotecas Utilizadas

| Biblioteca    | Versão     | Função no Projeto |
|---------------|------------|-------------------|
| `aedes`       | ^0.51.3    | Broker MQTT leve feito em Node.js. Recebe dados do ESP32 via MQTT. |
| `dotenv`      | ^16.4.7    | Carrega variáveis de ambiente como porta, usuário e senha do banco. |
| `http`        | ^0.0.1-security | Cria um servidor HTTP para conexão via WebSocket MQTT. |
| `mysql2`      | ^3.12.0    | Insere os dados recebidos no banco de dados MySQL. |
| `net`         | ^1.0.2     | Cria servidor TCP para aceitar conexões MQTT padrão (usado pelo ESP32). |
| `ws`          | ^8.18.1    | Permite que a interface web se conecte ao broker por WebSocket e receba dados em tempo real. |

---

📁 **Estrutura Recomendável do Projeto**

