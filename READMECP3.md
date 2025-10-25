# Projeto IoT: Estação de Monitoramento (ESP32 + MQTT)

Este projeto implementa uma estação de monitoramento simples usando um ESP32. O dispositivo coleta dados de temperatura, umidade (sensor DHT) e luminosidade (sensor LDR), e os publica em um broker MQTT. Ele também é capaz de receber comandos via MQTT para controlar o LED onboard do ESP32.

A estrutura dos tópicos MQTT foi desenhada para ser compatível com a plataforma FIWARE, especificamente para integração com um IoT Agent (como o IoT Agent over MQTT).

## 1. Visão Geral dos Arquivos

Para garantir a clareza e facilitar a replicação, o repositório está organizado da seguinte forma:

/ ├── README.md (Este arquivo, com todas as instruções) ├── esp32_codigo_fonte/ │ └── esp32_codigo_fonte.ino (O código-fonte para o ESP32) └── assets/ └── esquematico.png (A imagem do circuito elétrico)


-   **`README.md`**: Contém a documentação completa do projeto.
-   **`esp32_codigo_fonte/esp32_codigo_fonte.ino`**: É o código C++ (Arduino) que deve ser carregado no ESP32.
-   **`assets/esquematico.png`**: É o diagrama de ligação do hardware.

---

## 2. Requisitos de Hardware

Para montar este projeto, você precisará dos seguintes componentes:

* **ESP32** (placa de desenvolvimento, ex: DOIT DEVKIT V1)
* **Sensor DHT22 (ou DHT11)** - Sensor de Temperatura e Umidade.
    * *Nota: A imagem mostra um HT22 (DHT22), mas o código usa a definição `DHTTYPE DHT11`. Veja a Seção 4.3 para o ajuste necessário.*
* **Módulo Sensor LDR** (Sensor de Luminosidade) - Módulo de 4 pinos (VCC, GND, DO, AO).
* **Resistor de 10kΩ** (Usado como pull-up para o sensor DHT).
* **Protoboard (Breadboard)**
* **Jumpers (fios)**

---

## 3. Montagem do Circuito (Hardware)

Utilize o arquivo `assets/esquematico.png` como referência visual. Siga as conexões de pinos abaixo para garantir a montagem correta.

![Diagrama de ligação do ESP32, DHT22 e LDR](assets/esquematico.png)

### 3.1. Conexões Principais (ESP32)

* **ESP32 `VIN`** -> Barramento Positivo (`+`) da protoboard.
* **ESP32 `GND`** -> Barramento Negativo (`-`) da protoboard.

### 3.2. Sensor DHT22 (HT22)

* **Pino 1 (VCC)** -> Barramento Positivo (`+`).
* **Pino 2 (DATA)** -> **ESP32 `GPIO 4`**.
* **Pino 4 (GND)** -> Barramento Negativo (`-`).
* **Resistor Pull-up**: Conecte o resistor de 10kΩ entre o **Pino 1 (VCC)** e o **Pino 2 (DATA)** do DHT22.

### 3.3. Módulo LDR

* **Pino `VCC`** -> Barramento Positivo (`+`).
* **Pino `GND`** -> Barramento Negativo (`-`).
* **Pino `AO` (Saída Analógica)** -> **ESP32 `GPIO 34`**.
* *Pino `DO` (Saída Digital) -> Não é utilizado neste projeto.*

---

## 4. Configuração do Software (ESP32)

### 4.1. Ambiente de Desenvolvimento (IDE)

1.  Baixe e instale a **Arduino IDE** (ou utilize o VS Code com a extensão PlatformIO).
2.  Configure a IDE para a placa ESP32. Se estiver na Arduino IDE, vá em `Ferramentas` > `Placa` > `Gerenciador de Placas`, procure por "esp32" e instale o pacote da Espressif Systems.
3.  Selecione a placa correta (ex: `DOIT ESP32 DEVKIT V1`) e a porta COM correspondente.

### 4.2. Instalação das Bibliotecas

Este projeto requer duas bibliotecas. Na Arduino IDE, vá em `Sketch` > `Incluir Biblioteca` > `Gerenciar Bibliotecas...` e instale:

1.  **`PubSubClient`** (por Nick O'Leary) - Para comunicação MQTT.
2.  **`DHT sensor library`** (por Adafruit) - Para o sensor DHT.
3.  **`Adafruit Unified Sensor`** (Dependência da biblioteca DHT).

### 4.3. Configuração do Código-Fonte

Abra o arquivo `esp32_codigo_fonte/esp32_codigo_fonte.ino` na sua IDE. Você **deve** editar as seguintes seções antes de carregar o código:

**1. Configurações de Rede e Broker:**
Altere as variáveis no início do arquivo para corresponder ao seu ambiente.

```cpp
// ========================= Configurações - variáveis editáveis =========================
const char* default_SSID = "SUA_REDE_WIFI";       // <-- SEU WI-FI AQUI
const char* default_PASSWORD = "SUA_SENHA_WIFI"; // <-- SUA SENHA AQUI
const char* default_BROKER_MQTT = "IP_DO_BROKER"; // <-- IP DO SEU BROKER MQTT
// ...
2. Tipo do Sensor DHT (Importante!) O código-fonte está configurado por padrão para um DHT11. A imagem do esquemático mostra um HT22 (que é um DHT22).

Se você estiver usando um DHT11, mantenha a linha como está:

C++

#define DHTTYPE DHT11 
Se você estiver usando um DHT22 (ou HT22), altere a linha para:

C++

#define DHTTYPE DHT22
4.4. Carregar o Código
Após editar as configurações, clique no botão "Carregar" (seta para a direita) na Arduino IDE para enviar o firmware para o ESP32. Você pode usar o "Monitor Serial" (Ctrl+Shift+M) com a velocidade 115200 para ver os logs de conexão e publicação.

5. Configuração do Servidor (Broker MQTT)
O ESP32 precisa de um "servidor" (Broker MQTT) para onde enviar os dados. O código está configurado para o IP 192.168.15.16 na porta 1883. Você pode usar qualquer broker, como:

Mosquitto (Local): Um broker leve que você pode instalar no seu computador.

HiveMQ (Nuvem): Um broker público para testes rápidos.

MyMQTT (App): Um aplicativo de celular que pode funcionar como cliente de teste.

5.1. Testando com um Cliente MQTT (Ex: MyMQTT ou MQTT Explorer)
Para verificar se o ESP32 está funcionando, use um cliente MQTT (como o app MyMQTT ou o software MQTT Explorer) e conecte-se ao mesmo broker que você configurou no ESP32.

Para VER os dados enviados pelo ESP32: Assine (Subscribe) os seguintes tópicos:

/TEF/device001/attrs (Mostra o estado do LED: s|on ou s|off)

/TEF/device001/attrs/p (Mostra a luminosidade: 0 a 100)

/TEF/device001/attrs/h (Mostra a umidade: ex: 45.5)

/TEF/device001/attrs/t (Mostra a temperatura: ex: 23.1)

Para ENVIAR comandos para o ESP32 (Ligar/Desligar LED): Publique (Publish) no tópico:

Tópico: /TEF/device001/cmd

Mensagem (Payload) para LIGAR: device001@on|

Mensagem (Payload) para DESLIGAR: device001@off|

5.2. Integração com FIWARE (Avançado)
A estrutura de tópicos (/TEF/device001/attrs/p) e os payloads (s|on) são padrões do FIWARE IoT Agent over MQTT.

Para uma integração completa com um servidor FIWARE:

Configure um IoT Agent for MQTT.

Provisione um novo dispositivo (ex: device001) no IoT Agent, mapeando os atributos:

p (luminosidade)

h (umidade)

t (temperatura)

s (estado do LED, como atributo)

Configure o IoT Agent para assinar os tópicos .../attrs/# no broker.

Provisione um comando no dispositivo (ex: on) que, quando acionado no FIWARE, fará o IoT Agent publicar a mensagem device001@on| no tópico /TEF/device001/cmd.