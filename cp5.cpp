#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>  // Adafruit DHT sensor library
 
// ========================= Configurações - variáveis editáveis =========================
const char* default_SSID = "FIAP-IOT";            // Rede Wi-Fi (2,4GHz)
const char* default_PASSWORD = "F!@p25.IOT";      // Senha da rede Wi-Fi
const char* default_BROKER_MQTT = "192.168.15.16"; // IP/host do Broker MQTT
const int   default_BROKER_PORT = 1883;           // Porta do Broker MQTT
const int   default_D4 = 2;                       // Pino do LED onboard (GPIO2 no ESP32)
 
// ========================= Configurações - Dispositivo =========================
const char* default_TOPICO_SUBSCRIBE = "/TEF/device001/cmd";    // Tópico de escuta (comandos)
const char* default_TOPICO_PUBLISH_1 = "/TEF/device001/attrs";  // Atributos gerais (LED on/off)
const char* default_TOPICO_PUBLISH_2 = "/TEF/device001/attrs/p";// Luminosidade (%)
const char* default_ID_MQTT = "fiware_001";                     // ID MQTT do dispositivo
const char* topicPrefix = "device001";                          // Prefixo utilizado nos comandos Helix
 
// ===== Tópicos extras p/ DHT =====
const char* default_TOPICO_PUBLISH_H = "/TEF/device001/attrs/h"; // Umidade (%)
const char* default_TOPICO_PUBLISH_T = "/TEF/device001/attrs/t"; // Temperatura (°C)
 
// ========================= Configurações DHT (Adafruit) =========================
#define DHTPIN 4      // Altere para o pino conectado ao DHT
#define DHTTYPE DHT11  // Altere para DHT11 se for o seu caso
DHT dht(DHTPIN, DHTTYPE);
 
// ========================= Variáveis para configurações editáveis =========================
char* SSID = const_cast<char*>(default_SSID);
char* PASSWORD = const_cast<char*>(default_PASSWORD);
char* BROKER_MQTT = const_cast<char*>(default_BROKER_MQTT);
int   BROKER_PORT = default_BROKER_PORT;
char* TOPICO_SUBSCRIBE = const_cast<char*>(default_TOPICO_SUBSCRIBE);
char* TOPICO_PUBLISH_1 = const_cast<char*>(default_TOPICO_PUBLISH_1);
char* TOPICO_PUBLISH_2 = const_cast<char*>(default_TOPICO_PUBLISH_2);
char* TOPICO_PUBLISH_H = const_cast<char*>(default_TOPICO_PUBLISH_H);
char* TOPICO_PUBLISH_T = const_cast<char*>(default_TOPICO_PUBLISH_T);
char* ID_MQTT = const_cast<char*>(default_ID_MQTT);
int   D4 = default_D4;
 
// ========================= Objetos globais =========================
WiFiClient espClient;
PubSubClient MQTT(espClient);
char EstadoSaida = '0';
 
// ========================= Prototipagem =========================
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi();
void reconnectMQTT();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT();
void EnviaEstadoOutputMQTT();
void InitOutput();
void handleLuminosity();
void handleHumidity();
void handleTemperature();
 
// ========================= Inicializações =========================
void initSerial() {
  Serial.begin(115200);
}
 
void initWiFi() {
  delay(10);
  Serial.println("------Conexao WI-FI------");
  Serial.print("Conectando-se na rede: ");
  Serial.println(SSID);
  Serial.println("Aguarde");
  reconectWiFi();
}
 
void initMQTT() {
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}
 
// ========================= Setup =========================
void setup() {
  InitOutput();
  initSerial();
  initWiFi();
  initMQTT();
 
  dht.begin(); // Inicializa o DHT
 
  delay(5000);
  MQTT.publish(TOPICO_PUBLISH_1, "s|on");
}
 
// ========================= Loop =========================
void loop() {
  VerificaConexoesWiFIEMQTT();
  EnviaEstadoOutputMQTT();
 
  handleLuminosity();  // Publica luminosidade (%)
  handleHumidity();    // Publica umidade (%)
  handleTemperature(); // Publica temperatura (°C)
 
  MQTT.loop();
}
 
// ========================= Conexão Wi-Fi =========================
void reconectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
 
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Conectado com sucesso na rede ");
  Serial.print(SSID);
  Serial.println(" IP obtido: ");
  Serial.println(WiFi.localIP());
 
  // Garantir que o LED inicie desligado
  digitalWrite(D4, LOW);
}
 
// ========================= Callback MQTT =========================
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    char c = (char)payload[i];
    msg += c;
  }
  Serial.print("- Mensagem recebida: ");
  Serial.println(msg);
 
  // Padrão de comando Helix: deviceId@on|  e deviceId@off|
  String onTopic = String(topicPrefix) + "@on|";
  String offTopic = String(topicPrefix) + "@off|";
 
  if (msg.equals(onTopic)) {
    digitalWrite(D4, HIGH);
    EstadoSaida = '1';
  }
 
  if (msg.equals(offTopic)) {
    digitalWrite(D4, LOW);
    EstadoSaida = '0';
  }
}
 
// ========================= Verificações de Conexão =========================
void VerificaConexoesWiFIEMQTT() {
  if (!MQTT.connected()) reconnectMQTT();
  reconectWiFi();
}
 
void reconnectMQTT() {
  while (!MQTT.connected()) {
    Serial.print("* Tentando se conectar ao Broker MQTT: ");
    Serial.println(BROKER_MQTT);
    if (MQTT.connect(ID_MQTT)) {
      Serial.println("Conectado com sucesso ao broker MQTT!");
      MQTT.subscribe(TOPICO_SUBSCRIBE);
    } else {
      Serial.println("Falha ao reconectar no broker.");
      Serial.println("Haverá nova tentativa de conexão em 2s");
      delay(2000);
    }
  }
}
 
// ========================= Publicação de estado do LED =========================
void EnviaEstadoOutputMQTT() {
  if (EstadoSaida == '1') {
    MQTT.publish(TOPICO_PUBLISH_1, "s|on");
    Serial.println("- Led Ligado");
  }
 
  if (EstadoSaida == '0') {
    MQTT.publish(TOPICO_PUBLISH_1, "s|off");
    Serial.println("- Led Desligado");
  }
  Serial.println("- Estado do LED onboard enviado ao broker!");
  delay(1000);
}
 
// ========================= Inicialização do LED =========================
void InitOutput() {
  pinMode(D4, OUTPUT);
  digitalWrite(D4, HIGH);
  boolean toggle = false;
 
  for (int i = 0; i <= 10; i++) {
    toggle = !toggle;
    digitalWrite(D4, toggle);
    delay(200);
  }
}
 
// ========================= Sensores =========================
 
// Luminosidade (%)
void handleLuminosity() {
  const int potPin = 34; // Ajuste conforme seu LDR/entrada analógica
  int sensorValue = analogRead(potPin);
  int luminosity = map(sensorValue, 0, 4095, 0, 100);
  String mensagem = String(luminosity);
  Serial.print("Valor da luminosidade: ");
  Serial.println(mensagem.c_str());
  MQTT.publish(TOPICO_PUBLISH_2, mensagem.c_str()); // attrs/p
}
 
// Umidade (%RH) via DHT — intervalo mínimo 2s
void handleHumidity() {
  static unsigned long lastReadH = 0;
  const unsigned long MIN_INTERVAL = 2000; // 2s
 
  unsigned long now = millis();
  if (now - lastReadH < MIN_INTERVAL) return;
 
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Falha na leitura DHT (umidade).");
    return;
  }
 
  char payload[16];
  dtostrf(h, 0, 1, payload); // 1 casa decimal
 
  Serial.print("Umidade (%): ");
  Serial.println(payload);
 
  MQTT.publish(TOPICO_PUBLISH_H, payload); // attrs/h
  lastReadH = now;
}
 
// Temperatura (°C) via DHT — intervalo mínimo 2s
void handleTemperature() {
  static unsigned long lastReadT = 0;
  const unsigned long MIN_INTERVAL = 2000; // 2s
 
  unsigned long now = millis();
  if (now - lastReadT < MIN_INTERVAL) return;
 
  float t = dht.readTemperature(); // Para Fahrenheit use: dht.readTemperature(true)
  if (isnan(t)) {
    Serial.println("Falha na leitura DHT (temperatura).");
    return;
  }
 
  char payload[16];
  dtostrf(t, 0, 1, payload); // 1 casa decimal
 
  Serial.print("Temperatura (C): ");
  Serial.println(payload);
 
  MQTT.publish(TOPICO_PUBLISH_T, payload); // attrs/t
  lastReadT = now;
}