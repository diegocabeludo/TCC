#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>

    // host e porta do broker
    long lastMsg = 0;
    char msg[50];
    int value = 0;
    int threhold=50; // you might need to adjust this value to define light on/off status
    const char* BROKER_MQTT = "broker.hivemq.com";
    int BROKER_PORT = 1883;

    void initPins();
    void initSerial();
    void initWiFi();
    void initMQTT();

WiFiClient espClient;
// instancia o mqtt
PubSubClient MQTT(espClient);

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);

    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("AutoConnectAP");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

    
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)"); 
  initSerial();
  initPins();
  initWiFi();
  initMQTT();
}

void loop() {
  Serial.println("LOOP iniciado"); 
  if (!MQTT.connected()) {
    reconnectMQTT();
    Serial.println("reconnectMQTT iniciado"); 
    
  }
  recconectWiFi();
  Serial.println("recconectWiFi iniciado"); 
      //envia o status de todos os outputs para o Broker no protocolo esperado
  EnviaEstadoOutputMQTT();
  
  MQTT.loop();
  Serial.println("MQTT.loop iniciado"); 
}

void initPins() {
  pinMode(D4, OUTPUT);
  digitalWrite(D4, 0);
  Serial.println("Pinos iniciados"); 
}

void initSerial() {
  Serial.begin(115200);
  Serial.println("Serial iniciado"); 
  
}
void initWiFi() {
  delay(10);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  Serial.println();
  Serial.println(WiFi.localIP());
  Serial.println("Wifi iniciado"); 
}

// Funcão para se conectar ao Broker MQTT
void initMQTT() {
  Serial.println("MQTT iniciado"); 
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  MQTT.setCallback(mqtt_callback);
}

//Função que recebe as mensagens publicadas
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("mqtt_callback iniciado ##########################################################"); 

  String message;
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    message += c;
  }
  Serial.println("Tópico => " + String(topic) + " | Valor => " + String(message));
  if (message == "1") {
    digitalWrite(D4, 1);
    
  } else {
    digitalWrite(D4, 0);
  }
  Serial.flush();
}

void reconnectMQTT() {
  while (!MQTT.connected()) {
    if (MQTT.connect("ESP8266-raphael/led")) {
      MQTT.subscribe("ESP8266-raphael/led");
      

    } else {
      delay(2000);
    }
  }
}

//Função: envia ao Broker o estado atual do output 
//Parâmetros: nenhum
//Retorno: nenhum
void EnviaEstadoOutputMQTT(void)
{
  long now = millis();
 //send data every second
  if (now - lastMsg > 500) {
     lastMsg = now;
    int val=analogRead(A0);
     String msg="real time light strength: ";
     msg= msg+ val;
   if (val>threhold)
      msg="0: "+msg;
    else
      msg="1: "+msg;
     char message[58];
     msg=val;
     msg.toCharArray(message,58);
     Serial.println(message);
  
     //publish sensor data to MQTT broker

      MQTT.publish("ESP8266-raphael/ldr", message);
}}

void recconectWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
}
