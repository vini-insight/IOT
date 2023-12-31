
// CÓDIGO PARA nodeMCU de IP 109 conectada via protocolo MQTT

# include <ESP8266WiFi.h>
# include <ESP8266mDNS.h>
# include <WiFiUdp.h>
# include <ArduinoOTA.h>
#include <PubSubClient.h>

# ifndef STASSID
# define STASSID "INTELBRAS"
# define STAPSK  "Pbl-Sistemas-Digitais"
# endif

const char* ssid = STASSID; 
const char* password = STAPSK;
const char* host = "ESP-10.0.0.109";

int led_pin = LED_BUILTIN;

int buttonD0 = D0; // digital sensor 1 (push button)
int buttonD1 = D1; // digital sensor 2 (push button)
int temp0 = 0;   // temporary variable for reading the button pin status
int temp1 = 0;   // temporary variable for reading the button pin status

// MQTT broker credentials
const char* MQTT_username = "aluno"; 
const char* MQTT_password = "@luno*123"; 

// MQTT broker
const char* mqtt_server = "10.0.0.101";

const int QOS = 1;

#define PROTOCOLCODEST "PROTOCOLCODEST"
#define OKSTATUSMQTT "OKSTATUSMQTT"
#define ANALT "an4log"
#define SD1GT "d1g"
#define SD2GT "d2g"
#define LEDT "l3d"
#define ALLBROADCASTT "ALLBROADCASTT"

// Client MQTT
WiFiClient espClient;
PubSubClient client(espClient);

# define N_DIMMERS 3
int dimmer_pin[] = {14, 5, 15};

int analogInPin = A0; /* analog sensor (ADC) PIN */
int sensorValue = 0;  /* POTENCIOMETRO */

char protocolCodes[] = {
0b00000000, 0b00000001, 0b00000010, 0b00000011, 0b00000100, 0b00000101, 0b00000110, 0b00000111, 0b00001000, 0b00001001,
0b00001010, 0b00001011, 0b00001100, 0b00001101, 0b00001110, 0b00001111, 0b00010000, 0b00010001, 0b00010010, 0b00010011,
0b00010100, 0b00010101, 0b00010110, 0b00010111, 0b00011000, 0b00011001, 0b00011010, 0b00011011, 0b00011100, 0b00011101,
0b00011110, 0b00011111, 0b00100000, 0b00100001, 0b00100010, 0b00100011, 0b00100100, 0b00100101, 0b00100110, 0b00100111,
0b00101000, 0b00101001, 0b00101010, 0b00101011, 0b00101100, 0b00101101, 0b00101110, 0b00101111, 0b00110000, 0b00110001,
0b00110010, 0b00110011, 0b00110100, 0b00110101, 0b00110110, 0b00110111, 0b00111000, 0b00111001, 0b00111010, 0b00111011,
0b00111100, 0b00111101, 0b00111110, 0b00111111
};

char toRequest[] = { 0b00100010, 0b00100011, 0b00100100, 0b00100101, 0b00100110, 0b00100111, 0b00101000, 0b00101001, 0b00101010 };

void setup() {  
  Serial.begin(115200); /* BaudRate */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
  }
  digitalWrite(led_pin, LOW); /* switch off led */
  /* configure dimmers, and OTA server events */
  analogWriteRange(1000);
  analogWrite(led_pin, 990);
  for (int i = 0; i < N_DIMMERS; i++) {
    pinMode(dimmer_pin[i], OUTPUT);
    analogWrite(dimmer_pin[i], 50);
  }
  ArduinoOTA.setHostname(host);
  ArduinoOTA.onStart([]() { // switch off all the PWMs during upgrade
    for (int i = 0; i < N_DIMMERS; i++) {
      analogWrite(dimmer_pin[i], 0);
    }
    analogWrite(led_pin, 0);
  });
  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end
    for (int i = 0; i < 30; i++) {
      analogWrite(led_pin, (i * 100) % 1001);
      delay(50);
    }
  });
  ArduinoOTA.onError([](ota_error_t error) {
    (void)error;
    ESP.restart();
  });
  /* setup the OTA server */
  ArduinoOTA.begin();
  pinMode(led_pin, OUTPUT); /* configura LED embutido como saída */
  pinMode(buttonD0, INPUT); /* configura D0 embutido como entrada (botão) */
  pinMode(buttonD1, INPUT); /* configura D1 embutido como entrada (botão) */
  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback);
}
// This function is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// When a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
//void callback(String topic, byte* message, unsigned int length) {
void callback(String topic, byte* payload, unsigned int length) {
  if(topic == PROTOCOLCODEST)
  {
    String msg;
    if(length > 0)
    {
      for(int i = 0; i < length; i++)
      {
        char c = (char)payload[i];
        msg += c;
      }
    }
    if(msg.equals("1")) /*SE RECEBER O CÓDIGO RELACIONADO A ESTA UNIDADE ELA RETORNA CÓDIGO DE STATUS OK*/
      client.publish(OKSTATUSMQTT, "33"); //'0b00100001'
    if(msg.equals("134"))/*SE RECEBER O CÓDIGO REFERENTE AO SENSOR ANALÓGICO*/
    {
      char analog[] = "";
      sprintf(analog,"%d", analogRead(analogInPin));
      client.publish(ANALT, analog); //'toRequest[0]' ANAL // /* ENVIA O VALOR DO POTENCIOMENTRO */
    }
    if(msg.equals("135")){  /*SER RECEBER O CÓDIGO REFERENTE AO SENSOR DIGITAL 1 um*/
      temp0 = digitalRead(buttonD0);
      char digital0[] = "";
      sprintf(digital0,"%d", temp0);
      if (temp0 == HIGH) {
        client.publish(SD1GT, digital0);
      }
      else {
        client.publish(SD1GT, digital0);
      }
    }
    if(msg.equals("136")){  /*SER RECEBER O CÓDIGO REFERENTE AO SENSOR DIGITAL 2 dois*/
      char digital1[] = "";
      sprintf(digital1,"%d", digitalRead(buttonD1));
      client.publish(SD2GT, digital1);
    }
    if(msg.equals("137")){  /* SE RECEBER CÓDIGO PARA ATIVAR LED*/
      digitalWrite(led_pin, LOW);
      char led[] = "";
      sprintf(led,"%d", LOW);
      client.publish(LEDT, led);
    }
    if(msg.equals("138")){  /* SE RECEBER CÓDIGO PARA DESATIVAR LED*/
      digitalWrite(led_pin, HIGH);
      char led[] = "";
      sprintf(led,"%d", HIGH);
      client.publish(LEDT, led);
    }
    if(msg.equals("0")){ /*BROADCAST se receber o valor 0 (zero) responde com ID da node que nesse caso é 1. mas poderia ser qualquer outro valor */      
      client.publish(ALLBROADCASTT, "1");
    }
    client.publish("INO109", "109 ip F    U   N   F   A   !!!\n");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect("ESP8266Client109JVSVL", MQTT_username, MQTT_password)) {
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      //client.subscribe("comandos");
      client.subscribe("PROTOCOLCODEST", QOS);
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  ArduinoOTA.handle();
  /* NÃO RETIRAR A FUNÇÃO DA LINHA ACIMA */
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
//  if(!client.loop())
//    client.connect("ESP8266Client109JVSVL", MQTT_username, MQTT_password);  
}
