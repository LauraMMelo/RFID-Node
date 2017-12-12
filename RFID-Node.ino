#include "MFRC522.h"
#include <ESP8266WiFi.h> // Importa a Biblioteca ESP8266WiFi
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
#define RST_PIN D3 // RST-PIN for RC522 - RFID - SPI 
#define SS_PIN D4  // SDA-PIN for RC522 - RFID - SPI
#define ID_MQTT  "Node"  
#define TOPICO_PUBLISH "compras/teste"
#define TOPICO_SUBSCRIBE "test"
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

String conteudo= "";
const char* SSID = "GYROFLEX"; 
const char* PASSWORD = "tortadebanana"; 
const char* BROKER_MQTT = "m14.cloudmqtt.com"; 
const char* USER = "oleqzhkd"; 
const char* PASWRD = "7_yi2ywDEZYs"; 
int BROKER_PORT = 10280; 
WiFiClient espClient; 
PubSubClient MQTT(espClient);

void initWiFi();
void initMQTT();
void reconectWiFi(); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);

void setup() {
  Serial.begin(9600);    // Initialize serial communications
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522 
  initWiFi();
  initMQTT();
}

void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT, USER, PASWRD)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else 
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentativa de conexao em 2s");
            delay(2000);
        }
    }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
    String msg;
 
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
       char c = (char)payload[i];
       msg += c;
    }
}

void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}

void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
        
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
    
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
  
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.print("\n");
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}

void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    
    reconectWiFi();
}



void loop() { 
  // Look for new cards
  reconectWiFi();
  if(!MQTT.connected()){
    reconnectMQTT();
  }
  
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  Serial.print(F("Código do produto:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.print(conteudo);
  Serial.print(".");
  Serial.println();
  MQTT.publish(TOPICO_PUBLISH, conteudo.c_str());
  conteudo = "";
  delay(3500);
  MQTT.loop();
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    conteudo.concat(String(mfrc522.uid.uidByte[i]));
  }
}
