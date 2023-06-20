
// CÓDIGO PARA nodeMCU conectada via interface UART

# include <ESP8266WiFi.h>
# include <ESP8266mDNS.h>
# include <WiFiUdp.h>
# include <ArduinoOTA.h>

# ifndef STASSID
# define STASSID "INTELBRAS"
# define STAPSK  "Pbl-Sistemas-Digitais"
# endif

const char* ssid = STASSID; 
const char* password = STAPSK;
const char* host = "ESP-10.0.0.107";

int led_pin = LED_BUILTIN;

int buttonD0 = D0; // digital sensor 1 (push button)
int buttonD1 = D1; // digital sensor 2 (push button)
int temp0 = 0;   // temporary variable for reading the button pin status
int temp1 = 0;   // temporary variable for reading the button pin status

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
  /* pinMode(analogInPin, INPUT); */
}

void loop() {
  ArduinoOTA.handle();
  
  /* NÃO RETIRAR A FUNÇÃO DA LINHA ACIMA */
  
  if(Serial.available() > 0) { /* CHECA SE ESTÁ RECEBENDO DADOS */
    char pc = Serial.read();
    if(pc == protocolCodes[1] ){ /*SE RECEBER O CÓDIGO RELACIONADO A ESTA UNIDADE ELA RETORNA CÓDIGO DE STATUS OK*/
        Serial.print(protocolCodes[33], BIN); 
    }      
    if(pc == toRequest[0] ){ /*SE RECEBER O CÓDIGO REFERENTE AO SENSOR ANALÓGICO*/
      sensorValue = analogRead(analogInPin);
      Serial.print(sensorValue, BIN); /* ENVIA O VALOR DO POTENCIOMENTRO */
    }
    if(pc == toRequest[1] ){  /*SER RECEBER O CÓDIGO REFERENTE AO SENSOR DIGITAL 1*/
      temp0 = digitalRead(buttonD0);
      if (temp0 == HIGH) {
        Serial.print(1, BIN);
      }
      else {
        Serial.print(0, BIN);
      }
    }
    if(pc == toRequest[2] ){  /*SER RECEBER O CÓDIGO REFERENTE AO SENSOR DIGITAL 2*/
      temp1 = digitalRead(buttonD1);       
      if (temp1 == HIGH) {
        Serial.print(1, BIN);
      }
      else {
        Serial.print(0, BIN);
      }
    }
    if(pc == toRequest[3] ){ /* SE RECEBER CÓDIGO PARA ATIVAR LED*/
      digitalWrite(led_pin, LOW);
      Serial.print(1, BIN);
    }
    if(pc == toRequest[4] ){  /* SE RECEBER CÓDIGO PARA DESATIVAR LED*/
      digitalWrite(led_pin, HIGH);
      Serial.print(0, BIN);
    }      
    if(pc == protocolCodes[0] ){ /*BROADCAST se receber o valor 0 (zero) responde com ID da node que nesse caso é 1. mas poderia ser qualquer outro valor */
      Serial.println(1, BIN);
    }
  }
}
