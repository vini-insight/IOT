// CÓDIGO DA ORANGE PI. ABAIXO COMANDOS PARA COMPILAR
// gcc SBC.c -o SBC -lwiringPi -lwiringPiDev -lpthread -lm -lcrypt -lrt -lpaho-mqtt3c -Wall && chmod +x SBC
// sudo ./SBC
// rm SBC.c && nano p.SBC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // FOR sleep
#include <stdbool.h> // FOR bool
#include <time.h>
#include <math.h>
#include <lcd.h> // for LCD
#include <wiringPi.h> // for GPIOS
#include <wiringSerial.h> /* for UART || serial || RS232 */
// #include <pthread.h>
#include <MQTTClient.h> // for MQTT protocol

char ID[6] = "1"; // ALTERAR ID da NODE AQUI
char AC[6] = "34";
char D0C[6] = "35";
char D1C[6] = "36";
char ONC[6] = "37";
char OFFC[6] = "38";
char command[6] = "\0";
char ponteiro[6] = "\0";
int randomNum = 0;

char* altera(char strD[], char strC[]);
char* getRandoms(int, int, int);

char* getRandoms(int lower, int upper, int count)
{
    srand(time(0));
    int i;
    for (i = 0; i < count; i++) {
        randomNum = (rand() % (upper - lower + 1)) + lower;
        // printf("%d ", num);
        sprintf(command, "%d", randomNum);
        // printf("%s\n", command);
    }
    return command;
}

char* altera(char strD[], char strC[])
{
    if(strlen(strC) == 1)
    {
        strD[0] = strC[0];
    }
    else
    {
        strD[0] = ID[0];
        int i;
        for (i = 1; i < strlen(strC)+1; i++)
        {
            strD[i] = strC[i-1];
        }
    }
        return strD;
}

#define SBCID (altera(command, ID))
#define ANALCODE (altera(command, AC))
#define SD0CODE (altera(command, D0C))
#define SD1CODE (altera(command, D1C))
#define LEDONCODE (altera(command, ONC))
#define LEDOFFCODE (altera(command, OFFC))

//USE WIRINGPI PIN NUMBERS
#define LCD_RS  13               //Register select pin
#define LCD_E   18               //Enable Pin
#define LCD_D4  21               //Data pin 4
#define LCD_D5  24               //Data pin 5
#define LCD_D6  26               //Data pin 6
#define LCD_D7  27               //Data pin 7
// dip switch pins
char DIP1 = 2;               //dip switch 1 = PA6  WiringPi    2 pin
char DIP2 = 5;               //dip switch 2 = PA1  WiringPi    5 pin
char DIP3 = 7;               //dip switch 3 = PA0  WiringPi    7 pin
char DIP4 = 8;               //dip switch 4 = PA3  WiringPi    8 pin
char espnode = 0b00000001;
char analog = 0b00100010;
char dig1 = 0b00100011;
char dig2 = 0b00100100;
char ledON = 0b00100101;
char ledOFF = 0b00100110;
char okStatus = 0b00100001;
char request = 0b00100001;
char responseCode = 0b00111111;
char allNodesMCU = 0b00111111;
// push buttons
char subButton = 19;
char selectButton = 23;
char addButton = 25;

bool timeOut = false;
bool enviarParaTodos = false;
bool espOK = false;
bool sOK = false;
bool timeOutMQTT = false;

int fd;
int lcd;
int allMonitors = 0;
int mainMenu = 0;
int nodeUART = 1;
int nodeMQTT = 1;
int oldnodeUART = 0;
int oldnodeMQTT = 0;
int sensorOption = 0;
int sda = 9;
int sgc = 9;
int part = -1;

float volts = -1.0;
int intAUX;

char nodeCodes[] = {
0b00000000, 0b00000001, 0b00000010, 0b00000011, 0b00000100, 0b00000101, 0b00000110, 0b00000111, 0b00001000, 0b00001001,
0b00001010, 0b00001011, 0b00001100, 0b00001101, 0b00001110, 0b00001111, 0b00010000, 0b00010001, 0b00010010, 0b00010011,
0b00010100, 0b00010101, 0b00010110, 0b00010111, 0b00011000, 0b00011001, 0b00011010, 0b00011011, 0b00011100, 0b00011101,
0b00011110, 0b00011111, 0b00100000, 0b00100001, 0b00100010, 0b00100011, 0b00100100, 0b00100101, 0b00100110, 0b00100111,
0b00101000, 0b00101001, 0b00101010, 0b00101011, 0b00101100, 0b00101101, 0b00101110, 0b00101111, 0b00110000, 0b00110001,
0b00110010, 0b00110011, 0b00110100, 0b00110101, 0b00110110, 0b00110111, 0b00111000, 0b00111001, 0b00111010, 0b00111011,
0b00111100, 0b00111101, 0b00111110, 0b00111111
};

// char *topicosUS[] = {"TSRVINI111", "TSRVINI109"};

char toRequest[] = { 0b00100010, 0b00100011, 0b00100100, 0b00100101, 0b00100110, 0b00100111, 0b00101000, 0b00101001, 0b00101010 };
char *lcdSensorOptions[] = {"Anal.", "Dig.1", "Dig.2", "LED on", "LED off", "Analog Monitor:", "Monitor Dig.1:", "Monitor Dig.2:", "ALL sensors", " <-- Voltar"};
// char *lcdMainOptions[] = {"Escolher UART", "Escolher MQTT", "BROADCAST", " <-- Voltar", "node MCU status"};
char *lcdMainOptions[] = {"Escolher UART", "Escolher MQTT", "BROADCAST", "node MCU status"};

char *lcdAnswersL0[] = {"Analog Sensor:", "Sensor Dig.1:", "Sensor Dig.2:", "LED on", "LED off", "status"};
char *lcdAnswersL1[] = {"ADC=%d V=%0.3f", "bit %d PB", "bit %d PB", "LED bit %d", "LED bit %d", "OK"};

char int_str[100];
//char okStatusMQTT[100];

int allNodesBroadcast[32];
int broadcastIndex = 0;
int broadcastCount = 0;

/* INICIO definição das funções (PRTÓTIPOS)     */
int convert(int);
void bTOd(char array[]);
void delayButton();

void changeControlValues();
void monitoringUART(char request);
void soltarBotao();

void serialResponse();
char* intTOstring(char int_str[], int);
void displayMessage(char l0[], char l1[]);
void displayMessageIntegerPointer(char l0[], char l1[], int *ip);
void chooseOptions();
void chooseUART();
void chooseMQTT();
void chooseSensorUART();
void showResponsesLCD();
/* FIM definição das funções (PRTÓTIPOS)        */

/*
* Defines               MQTT PROTOCOL
*/
/* Caso desejar utilizar outro broker MQTT, substitua o endereco abaixo */
//#define MQTT_ADDRESS   "tcp://iot.eclipse.org"

#define MQTT_ADDRESS   "tcp://10.0.0.101:1883"
// #define MQTT_ADDRESS   "tcp://10.0.0.101"
/* Substitua este por um ID unico em sua aplicacao */
// #define CLIENTID    "MQTTCClientIDjvsvl"
#define CLIENTID (getRandoms(999, 9999, 1))  // ID do cliente MQTT

#define USERNAME "aluno"
#define PASSWORD "@luno*123"
// tópicos
// #define MQTT_PUBLISH_TOPIC     "TSRVINI"
// #define MQTT_SUBSCRIBE_TOPIC   "TSRVINI"
// #define INO111               "INO111"
// #define TSRVINI111           "TSRVINI111"
#define GRAPHT "GRAPHT"
#define PROTOCOLCODEST "PROTOCOLCODEST"
// #define OKSTATUSMQTT           "33"
#define OKSTATUSMQTT "OKSTATUSMQTT"
#define ANALT "an4log"
#define SD1GT "d1g"
#define SD2GT "d2g"
#define LEDT "l3d"
#define ALLBROADCASTT "ALLBROADCASTT"


MQTTClient client;

void publish(MQTTClient client, char* topic, char* payload);
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message);

// char* concatenaProtocol(char strD[], char strP[], char strR[])
// char* concatenaProtocol(int p, int r)
void concatenaProtocol(int p, int r)
{
    int pro, req;
    char str[3] = "\0";
    char str2[3] = "\0";
    pro = nodeCodes[p]; // converte char to int
    sprintf(str, "%d", pro);  // converte int to string
    printf("pro %s\n", str);
    if(sensorOption == 5 || sensorOption == 6 || sensorOption == 7)
        req = r; // converte char to int
    else
        req = toRequest[r]; // converte char to int
    sprintf(str2, "%d", req);  // converte int to string
    printf("req %s\n", str2);
    printf("CMD%sCMD\n", ponteiro);
    ponteiro[0] = str[0];
    ponteiro[1] = str2[0];
    ponteiro[2] = str2[1];
    ponteiro[3] = '\0';
    printf("CMD%sCMD\n", ponteiro);
}
char* getProtocol(int p, int r)
{
    int pro, req;
    char str[3] = "\0";
    char str2[3] = "\0";
    char protocolRequest[6] = "\0";
        pro = nodeCodes[p]; // converte char to int
    sprintf(str, "%d", pro);  // converte int to string
    printf("pro %s\n", str);
    req = toRequest[r]; // converte char to int
    sprintf(str2, "%d", req);  // converte int to string
    printf("req %s\n", str2);
        // strcat(protocolRequest, str);           //CONCATENA
        strcat(command, str);           //CONCATENA
    // printf("pr%spr\n", protocolRequest);
    printf("pr%spr\n", command);
    // strcat(protocolRequest, str2);              //CONCATENA
    strcat(command, str2);              //CONCATENA
    // printf("pr%spr\n", protocolRequest);
    printf("pr%spr\n", command);
        // return protocolRequest;                         // RETORNA ID da NODE + CÓDIGO do SENSOR concatenados (protocolo / comando)
        return command;                         // RETORNA ID da NODE + CÓDIGO do SENSOR concatenados (protocolo / comando)
}

/* Funcao: publicacao de mensagens MQTT
 * Parametros: cleinte MQTT, topico MQTT and payload
 * Retorno: nenhum
*/
void publish(MQTTClient client, char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    //pubmsg.qos = 2;
    pubmsg.qos = 1;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
}

/* Funcao: callback de mensagens MQTT recebidas e echo para o broker
 * Parametros: contexto, ponteiro para nome do topico da mensagem recebida, tamanho do nome do topico e mensagem recebida
 * Retorno : 1: sucesso (fixo / nao ha checagem de erro neste exemplo)
*/
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payload = message->payload;

    /* Mostra a mensagem recebida */
    printf("Mensagem recebida! \n\rTopico: %s Mensagem: %s\n", topicName, payload);

        // if(strcmp(payload, okStatusMQTT) == 0)
        if(strcmp(topicName, OKSTATUSMQTT) == 0) // NÃO PRECISO SABER QUE MENSAGEM ENVIOU POIS SÓ ENVIA UM ÚNICO CÓDIGO DE PROTOCOLO NESTE TÓPICO. ENTÃO SE RESPONDEU É PORQUE ESTÁ OK
        {
                espOK = true;
        }
        if(strcmp(topicName, ANALT) == 0) // sensor analógico
        {
                // sOK = true;
                part = -1;
                sscanf(payload, "%d", &part);
                lcdClear(lcd);
                lcdPosition(lcd, 0, 0);
                if(sensorOption == 5) {
                        lcdPrintf(lcd, lcdAnswersL0[0]);
                }
                else if(sensorOption == 8)              // TODOS OS SENSORES
                {
                        printf("A       N       A       L       O       G\n");
                        lcdPrintf(lcd, lcdAnswersL0[allMonitors]);
                }
                else
                {
                        printf("N       Ã       O\n");
                        lcdPrintf(lcd, lcdAnswersL0[sensorOption]);
                }
                lcdPosition(lcd, 0, 1);
                if(part > 1023)
                {
                        lcdPrintf(lcd, "READING ERROR");
                }
                else
                {
                        volts = part*3.3/1024;
                        lcdPrintf(lcd, lcdAnswersL1[0], part, volts);
                }
                // sOK = false;
                sOK = true;
        }
        if(strcmp(topicName, SD1GT) == 0) // sensor Digital 1 um.
        {
                // sOK = true;
                part = -1;
                sscanf(payload, "%d", &part);
                lcdClear(lcd);
                if(sensorOption == 6)
                {
                        printf("N       Ã       O\n");
                        lcdPosition(lcd, 0, 0);
                        lcdPrintf(lcd, lcdAnswersL0[1]);
                        lcdPosition(lcd, 0, 1);
                        lcdPrintf(lcd, lcdAnswersL1[1], part); // NÃO TROCAR INDICE POIS DA ERRO.
                }
                else if(sensorOption == 8)
                {
                        printf("D       I       G       UM\n");
                        lcdPosition(lcd, 0, 0);
                        lcdPrintf(lcd, lcdAnswersL0[allMonitors]);
                        lcdPosition(lcd, 0, 1);
                        lcdPrintf(lcd, lcdAnswersL1[allMonitors], part);
                }
                else
                {
                        printf("N       Ã       O       ELSE\n");
                        lcdPosition(lcd, 0, 0);
                        lcdPrintf(lcd, lcdAnswersL0[sensorOption]);
                        lcdPosition(lcd, 0, 1);
                        lcdPrintf(lcd, lcdAnswersL1[sensorOption], part);
                }
                sOK = true;
                // sOK = false;
        }
        if(strcmp(topicName, SD2GT) == 0) // sensor Digital 1 um.
        {
                // sOK = true;
                part = -1;
                sscanf(payload, "%d", &part);
                lcdClear(lcd);
                if(sensorOption == 7)
                {
                        printf("N       Ã       O\n");
                        lcdPosition(lcd, 0, 0);
                        lcdPrintf(lcd, lcdAnswersL0[2]);
                        lcdPosition(lcd, 0, 1);
                        lcdPrintf(lcd, lcdAnswersL1[1], part); // NÃO TROCAR INDICE POIS DA ERRO.
                }
                else if(sensorOption == 8)
                {
                        printf("D       I       G       DOIS\n");
                        lcdPosition(lcd, 0, 0);
                        lcdPrintf(lcd, lcdAnswersL0[allMonitors]);
                        lcdPosition(lcd, 0, 1);
                        lcdPrintf(lcd, lcdAnswersL1[allMonitors], part);
                }
                else
                {
                        printf("N       Ã       O       ELSE\n");
                        lcdPosition(lcd, 0, 0);
                        lcdPrintf(lcd, lcdAnswersL0[sensorOption]);
                        lcdPosition(lcd, 0, 1);
                        lcdPrintf(lcd, lcdAnswersL1[sensorOption], part);
                }
                sOK = true;
                // sOK = false;
        }
        if(strcmp(topicName, LEDT) == 0) // sensor Digital 1 um.
        {
                sOK = true;
                part = -1;
                sscanf(payload, "%d", &part);
                lcdClear(lcd);
                lcdPosition(lcd, 0, 0);
                lcdPrintf(lcd, lcdAnswersL0[sensorOption]);
                lcdPosition(lcd, 0, 1);
                lcdPrintf(lcd, lcdAnswersL1[sensorOption], part);
                // sOK = false;
        }
        if(strcmp(topicName, ALLBROADCASTT) == 0) // NÃO PRECISO SABER QUE MENSAGEM ENVIOU POIS SÓ ENVIA UM ÚNICO CÓDIGO DE PROTOCOLO NESTE TÓPICO. ENTÃO SE RESPONDEU É PORQUE ESTÁ OK
        {
                broadcastCount++;
                displayMessage("TOTAL", intTOstring(int_str, broadcastCount));
        }

    /* Faz echo da mensagem recebida */
    //publish(client, MQTT_PUBLISH_TOPIC, payload);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

char* intTOstring(char int_str[], int v)
{
        sprintf(int_str, "%d", v);
        return int_str;
}

int convert(int n) {    // CONVERTE BINARIO EM DECIMAL
        int dec = 0, i = 0, rem;
        while (n!=0)
        {
                rem = n % 10;
                n /= 10;
                dec += rem * pow(2, i);
                ++i;
        }
        return dec;
}

void bTOd(char array[])
{
        int integerValue;
        sscanf(array, "%d", &integerValue);
        printf("VALOR INTEIRO É : %d\n", integerValue);
        part = convert(integerValue);
        printf("VALOR convertido É : %d\n", part);
}

void delayButton()      // enquanto não pressionar o botão não sai do laço
{
        while(digitalRead(selectButton) != 0)
        {
                //printf ("\t");
        }
        delay(250); // para não selecionar de novo e repetir requisição
}


void delayESP() // PARA AGUARDAR ESP RESPONDER
{
        int tempo = 1;
        while(!espOK && tempo < 10)
        {
                printf ("esperando resposta ESP\n");
                //sleep(1);
                delay(250);
                if(espOK) break;
                tempo++;
        }
}

void delaySensor()      // PARA AGUARDAR SENSOR RESPONDER
{
        int tempo = 1;
        while(!sOK && tempo < 10)
        {
                printf ("esperando resposta Sensor\n");
                sleep(1);
                if(sOK) break;
                tempo++;
        }
}

void displayMessage(char l0[], char l1[])       // configura o LCD para exibir texto necessário
{
        lcdClear(lcd);
        lcdPosition(lcd, 0, 0); // para linha 0 // DEFINE A POSIÇÃO ( COLUNA, LINHA) NÃO ACEITA VALORES MAIORES QUE 1 NA LINHA, POIS SÓ TEM DUAS LINHAS
        // lcdPuts(lcd, msg);
        lcdPrintf(lcd, l0);
        lcdPosition(lcd, 0, 1); // para linha 1
        lcdPrintf(lcd, l1);
}

void displayMessageIntegerPointer(char l0[], char l1[], int *ip)        // configura o LCD para exibir texto necessário E concatena com valor inteiro
{
        lcdClear(lcd);
        lcdPosition(lcd, 0, 0); // para linha 0 do LCD
        lcdPrintf(lcd, l0);
        lcdPosition(lcd, 0, 1); // para linha 1 do LCD
        lcdPrintf(lcd, l1, *ip);
}

void serialResponse()   // espera a resposta serial da UART
{
        int iLocal = 0;
        char caLocal[10];
        while(sda != 0)
        {
                sgc = serialGetchar(fd);
                // printf ("char %c\n", sgc);
                sda = serialDataAvail(fd);
                // printf ("AVAIL %d\n", sda);
                printf ("%c", sgc);
                fflush (stdout);
                if(sgc != -1)
                {
                        caLocal[iLocal] = (char) sgc;
                        iLocal++;
                }
                else
                {
                        printf ("\nTIMEOUT\n");
                        timeOut = true;
                        iLocal = 0;
                        displayMessage("NAO responde", "(TIMEOUT)");
                }
        }
        printf ("\n");
        sda = 9;
        sgc = 9;
        if(timeOut)
        {
                timeOut = false;
        }
        else
        {
                bTOd(caLocal);
                if(sensorOption == 0 || sensorOption == 5)
                {
                    volts = part*3.3/1024;
                }
                if(enviarParaTodos)
                {
                        allNodesBroadcast[broadcastIndex] = part;
                        broadcastCount++;
                        broadcastIndex++;
                }
        }
}

void showResponsesLCD()
{
    lcdClear(lcd);

        /* PARA LIHA 0 DO LCD */

        lcdPosition(lcd, 0, 0);
        if(sensorOption == 5) {
                lcdPrintf(lcd, lcdAnswersL0[0]);
        }
        else if(sensorOption == 6) {
                lcdPrintf(lcd, lcdAnswersL0[1]);
        }
        else if(sensorOption == 7) {
                lcdPrintf(lcd, lcdAnswersL0[2]);
        }
        else /*(sensorOption < 5)*/
        {
                lcdPrintf(lcd, lcdAnswersL0[sensorOption]);
        }
        printf ("\n");

        /* PARA LIHA 1 DO LCD */

        lcdPosition(lcd, 0, 1);
        if(sensorOption == 0 || sensorOption == 5) // analógico OU monitor analógico
        {
                if(part > 1023)
                {
                        lcdPrintf(lcd, "READING ERROR");
                }
                else
                        {
                            lcdPrintf(lcd, lcdAnswersL1[0], part, volts);
                            publish(client, ANALT, intTOstring(int_str, part));
                        }
        }
        else if(sensorOption == 6)              // MONITOR DIG 1.
        {
                lcdPrintf(lcd, lcdAnswersL1[1], part); // NÃO TROCAR INDICE POIS DA ERRO.
                publish(client, SD1GT, intTOstring(int_str, part));
        }
        else if(sensorOption == 7)              // MONITOR DIG 2.
        {
                lcdPrintf(lcd, lcdAnswersL1[1], part); // NÃO TROCAR INDICE POIS DA ERRO.
                publish(client, SD2GT, intTOstring(int_str, part));
        }
        else if (sensorOption == 1)     // DIG 1.
    {
                lcdPrintf(lcd, lcdAnswersL1[sensorOption], part);
                publish(client, SD1GT, intTOstring(int_str, part));
        }
        else if (sensorOption == 2)     // DIG 2.
    {
                lcdPrintf(lcd, lcdAnswersL1[sensorOption], part);
                publish(client, SD2GT, intTOstring(int_str, part));
        }
        else if (sensorOption == 3 || sensorOption == 4)        // LED ON ou LED OFF
                {lcdPrintf(lcd, lcdAnswersL1[sensorOption], part);}
        else if(sensorOption == 8) {            // TODOS OS SENSORES

            /* PARA LIHA 0 DO LCD */
                lcdPosition(lcd, 0, 0);

                lcdPrintf(lcd, lcdAnswersL0[allMonitors]);

                /* PARA LIHA 1 DO LCD */
            lcdPosition(lcd, 0, 1);

                if(allMonitors == 0) {
                        if(part > 1023)
                        {
                                lcdPrintf(lcd, "READING ERROR");
                        }
                        else
                        {
                                lcdPrintf(lcd, lcdAnswersL1[allMonitors], part, volts);
                                publish(client, ANALT, intTOstring(int_str, part));
                        }
                }
                else
                {
                        lcdPrintf(lcd, lcdAnswersL1[allMonitors], part);
                        if (allMonitors == 1)
                                publish(client, SD1GT, intTOstring(int_str, part));
                        if (allMonitors == 2)
                                publish(client, SD2GT, intTOstring(int_str, part));
                }
        }
        else //if(sensorOption == 9)
        {
            if(part == (int) nodeCodes[33])
            {
                printf("NODE OK");
                        displayMessage(lcdAnswersL0[5], lcdAnswersL1[5]);
            }
            else
            {
                printf("E R R O");
                        displayMessage("E R R O", "E R R O");
            }
            delay(1000);
        }
        delay(1000);
        printf ("\n");
}

void changeControlValues()
{
        printf("DESLIGADO\n");
        mainMenu = 0;
        nodeUART = 1;
        oldnodeUART = 0;
        sensorOption = 0;
        nodeMQTT = 1;
        oldnodeMQTT = 0;
}

void monitoringUART(char request)
{
        while (digitalRead(selectButton) == 1)
        {
                serialPutchar(fd, request);
        serialResponse();
                showResponsesLCD();
        }       // se não funcionar colocar o subButton
        displayMessage("monitoring OFF", "back to the menu");
        delay(1000);
}

void monitoringMQTT(char request)
{
        while (digitalRead(selectButton) == 1)
        {
                sleep(1);
                intAUX = request;
                printf("%dchar\n", intAUX);
                // publish(client, PROTOCOLCODEST, intTOstring(int_str, intAUX));
                // publish(client, PROTOCOLCODEST, getProtocol(nodeMQTT, intAUX));
                concatenaProtocol(nodeMQTT, intAUX);
                publish(client, PROTOCOLCODEST, ponteiro);
                delaySensor();
        }       // se não funcionar colocar o subButton
        displayMessage("monitoring OFF", "back to the menu");
        delay(1000);
}

void soltarBotao()
{
        delay(250);
        while(digitalRead(selectButton) == 0) // SE NÃO LARGAR O BOTÃO
        {       //delay(500);
                printf ("solta o botão! \n");
        }
}

void chooseSensorUART() // permite escolher sensores, Monitoramento e outras opções exibidas na linha 1 do LCD
{
    printf("Escolher SENSOR:\n");
        while(1)
        {
                displayMessage("Escolha Opcao:", lcdSensorOptions[sensorOption]);
                if (digitalRead(selectButton) == 0)
                {
                        soltarBotao();
                        if(sensorOption == 9)
                        {
                                publish(client, GRAPHT, GRAPHT); // FECHAR GÁFICO
                                oldnodeUART = nodeUART;
                                nodeUART = 0; // FAZ VOLTAR AO INICIO
                                break; // <-- VOLTAR    // SAI DO LAÇO ATUAL
                        }
                        else if(sensorOption == 5) {
                                monitoringUART(toRequest[0]);
                        }
                        else if(sensorOption == 6) {
                                monitoringUART(toRequest[1]);
                        }
                        else if(sensorOption == 7) {
                                monitoringUART(toRequest[2]);
                        }
                        else if(sensorOption == 8) {
                                displayMessage("mostraremos 1", "sensor por seg.");
                delay(1000);
                            while (digitalRead(selectButton) == 1)
                                {
                                    delay(2000);
                                    serialPutchar(fd, toRequest[allMonitors]);
                                    serialResponse();
                                    showResponsesLCD();
                                        allMonitors++;
                                        if(allMonitors > 2)
                                            allMonitors = 0;
                                }       // se não funcionar colocar o subButton
                                displayMessage("monitoring OFF", "back to the menu");
                                delay(1000);
                        }
                        else {
                                displayMessage("enviando ...", "requisicao ...");
                                serialPutchar(fd, toRequest[sensorOption]);
                                serialResponse();
                                showResponsesLCD();
                                delayButton();
                        }
                }

                if (digitalRead(addButton) == 0)
                {
                        sensorOption++;
                        if(sensorOption > 9) sensorOption = 0;
                        delay(100);
                }
                if (digitalRead(subButton) == 0)
                {
                        sensorOption--;
                        if(sensorOption < 0) sensorOption = 9;
                        delay(100);
                }
                delay(250); // PRECISA
                if(digitalRead(DIP4) == 0)
                {
            changeControlValues();
            chooseOptions();
            lcdClear(lcd);
            break;
                }
        }
}

void chooseSensorMQTT() // permite escolher sensores, Monitoramento e outras opções exibidas na linha 1 do LCD
{
    printf("Escolher SENSOR:\n");
        while(1)
        {
                displayMessage("Escolha Opcao:", lcdSensorOptions[sensorOption]);
                if (digitalRead(selectButton) == 0)
                {
                        soltarBotao();
                        if(sensorOption == 9)
                        {
                                publish(client, GRAPHT, GRAPHT); // FECHAR GÁFICO
                                oldnodeMQTT = nodeMQTT;
                                nodeMQTT = 0; // FAZ VOLTAR AO INICIO
                                break; // <-- VOLTAR    // SAI DO LAÇO ATUAL
                        }
                        else if(sensorOption == 5) {
                                monitoringMQTT(toRequest[0]);
                        }
                        else if(sensorOption == 6) {
                                monitoringMQTT(toRequest[1]);
                        }
                        else if(sensorOption == 7) {
                                monitoringMQTT(toRequest[2]);
                        }
                        else if(sensorOption == 8) {
                                displayMessage("mostraremos 1", "sensor por seg.");
                delay(1000);
                            while (digitalRead(selectButton) == 1)
                                {
                                    delay(1000);
                                        // intAUX = toRequest[allMonitors];
                                        // publish(client, PROTOCOLCODEST, intTOstring(int_str, intAUX));
                                        // publish(client, PROTOCOLCODEST, getProtocol(nodeMQTT, allMonitors));
                                        concatenaProtocol(nodeMQTT, allMonitors);
                                        publish(client, PROTOCOLCODEST, ponteiro);
                                        delaySensor();
                                        if(sOK) sOK = false;
                                        allMonitors++;
                                        if(allMonitors > 2) allMonitors = 0;
                                }       // se não funcionar colocar o subButton
                                displayMessage("monitoring OFF", "back to the menu");
                                delay(1000);
                        }
                        else {
                                displayMessage("enviando ...", "requisicao ...");
                                // intAUX = toRequest[sensorOption];
                                // printf("%d\n", intAUX);
                                // publish(client, PROTOCOLCODEST, intTOstring(int_str, intAUX));
                                // printf("%s\n", concatenaProtocol(nodeMQTT, sensorOption));
                                concatenaProtocol(nodeMQTT, sensorOption);
                                // publish(client, PROTOCOLCODEST, getProtocol(nodeMQTT, sensorOption));
                                publish(client, PROTOCOLCODEST, ponteiro);
                                // delaySensor();
                                delayButton();
                        }
                }

                if (digitalRead(addButton) == 0)
                {
                        sensorOption++;
                        if(sensorOption > 9) sensorOption = 0;
                        delay(100);
                }
                if (digitalRead(subButton) == 0)
                {
                        sensorOption--;
                        if(sensorOption < 0) sensorOption = 9;
                        delay(100);
                }
                delay(250); // PRECISA
                if(digitalRead(DIP4) == 0)
                {
            changeControlValues();
            chooseOptions();
            lcdClear(lcd);
            break;
                }
        }
}

void chooseUART()       // permite escolher node UART (serial) na linha 1 do LCD
{
    printf("Escolher ESP:\n");
        while(1)
        {
                if(nodeUART == 0)
                {
                        nodeUART = oldnodeUART; // devolve valor coerente para ESP
                        break;  // PARA VOLTAR AO MENU ANTERIOR
                }
                displayMessageIntegerPointer("Escolher ESP:", "node MCU = %d", &nodeUART);
                if (digitalRead(selectButton) == 0)
                {
                        soltarBotao();
                        displayMessage(lcdMainOptions[3], "pedir status");
                        delay(1000);
                        serialPutchar(fd, nodeCodes[nodeUART]);
                        serialResponse();
                        printf("part = %d\n", part);
                        if(part == okStatus)
                        {
                                printf("STATUS OK !\n");
                                displayMessage(lcdAnswersL0[5], lcdAnswersL1[5]);
                                delay(1000);
                                chooseSensorUART();
                        }
                        else
                        {
                                displayMessage("NAO responde", "(TIMEOUT)");
                                delayButton();
                        }
                }

        if (digitalRead(addButton) == 0)
                {
                        nodeUART++;
                        if(nodeUART > 32) nodeUART = 1;
                        delay(100);
                }
                if (digitalRead(subButton) == 0)
                {
                        nodeUART--;
                        if(nodeUART < 1) nodeUART = 32;
                        delay(100);
                }
                if(digitalRead(DIP4) == 0)
                {
            changeControlValues();
            chooseOptions();
            lcdClear(lcd);
            break;
                }
                delay(100);
        }
}

void chooseMQTT()       // permite escolher node MQTT na linha 1 do LCD
{
    printf("Escolher ESP MQTT:\n");
        while(1)
        {
                if(nodeMQTT == 0)
                {
                        nodeMQTT = oldnodeMQTT; // devolve valor coerente para ESP
                        break;  // PARA VOLTAR AO MENU ANTERIOR
                }
                displayMessageIntegerPointer("Escolher ESP:", "node MQTT= %d", &nodeMQTT);
                if (digitalRead(selectButton) == 0)
                {
                        soltarBotao();
                        displayMessage(lcdMainOptions[3], "pedir status");
                        delay(1000);
                        intAUX = nodeCodes[nodeMQTT];
                        char test_int_str[100];
                        printf("test =%stest\n", test_int_str);
                        intTOstring(test_int_str, intAUX);
                        printf("test =%stest\n", test_int_str);
                        printf("intAUX =%dtest\n", intAUX);
                        //publish(client, PROTOCOLCODEST, intTOstring(int_str, intAUX));
                        publish(client, PROTOCOLCODEST, test_int_str);
                        delayESP(); // PARA AGUARDAR ESP RESPONDER
                        if(espOK)
                        {
                                printf("STATUS OK !\n");
                                displayMessage(lcdAnswersL0[5], lcdAnswersL1[5]);
                                delay(1000);
                                espOK = false;
                                chooseSensorMQTT();
                        }
                        else
                        {
                                displayMessage("NAO responde", "(TIMEOUT)");
                                delayButton();
                        }
                }

        if (digitalRead(addButton) == 0)
                {
                        nodeMQTT++;
                        if(nodeMQTT > 32) nodeMQTT = 1;
                        delay(100);
                }
                if (digitalRead(subButton) == 0)
                {
                        nodeMQTT--;
                        if(nodeMQTT < 1) nodeMQTT = 32;
                        delay(100);
                }
                if(digitalRead(DIP4) == 0)
                {
            changeControlValues();
            chooseOptions();
            lcdClear(lcd);
            break;
                }
                delay(100);
        }
}

void chooseOptions()    // ESCOLHER NODE OU ENVIAR MENSAGEM PARA TODAS AS NODES
{
    printf("Escolher Opcao:\n");
        while(1)
        {
                displayMessage("Escolher Opcao:", lcdMainOptions[mainMenu]);
                if (digitalRead(selectButton) == 0)
                {
                        soltarBotao();
                        if(mainMenu == 0)       // ESCOLHER NODE UART
                        {
                            chooseUART();
                        }
                        if(mainMenu == 1)       // ESCOLHER NODE MQTT
                        {
                            chooseMQTT();
                        }
                        if(mainMenu == 2)       // BROADCAST
                        {
                                enviarParaTodos = true;
                                displayMessage(lcdMainOptions[mainMenu], "enviar...");
                                // serialPutchar(fd, allNodesMCU);
                                serialPutchar(fd, nodeCodes[0]); //     '0b00000000'    CÓDIGO PARA BROADCAST 
                                serialResponse();
                                intAUX = nodeCodes[0];
                                publish(client, PROTOCOLCODEST, intTOstring(int_str, intAUX));
                                // displayMessage("TOTAL", intTOstring(int_str, broadcastCount));
                                delayButton();
                                enviarParaTodos = false;
                                broadcastIndex = 0;
                                broadcastCount = 0;
                        }
                }

        if (digitalRead(addButton) == 0)
                {
                        mainMenu++;
                        if(mainMenu > 2) mainMenu = 0;
                        delay(100);
                }
                if (digitalRead(subButton) == 0)
                {
                        mainMenu--;
                        if(mainMenu < 0) mainMenu = 2;
                        delay(100);
                }
                if(digitalRead(DIP4) == 0)
                {
            changeControlValues();
            break;
                }
                delay(100);
        }
}

int main(int argc, char *argv[])
{
        if (wiringPiSetup() < 0) return 1;
        if((fd = serialOpen("/dev/ttyS3",115200)) < 0) return 1;
        pinMode(subButton,INPUT);// Sets the pin as input.
        pinMode(selectButton,INPUT);// Sets the pin as input.
        pinMode(addButton,INPUT);// Sets the pin as input.
        pullUpDnControl(subButton, PUD_UP); // Sets the Pull-up mode for the pin.
        pullUpDnControl(selectButton, PUD_UP); // Sets the Pull-up mode for the pin.
        pullUpDnControl(addButton, PUD_UP); // Sets the Pull-up mode for the pin.
        pinMode(DIP1,INPUT);// Sets the pin as input.
        pinMode(DIP2,INPUT);// Sets the pin as input.
        pinMode(DIP3,INPUT);// Sets the pin as input.
        pinMode(DIP4,INPUT);// Sets the pin as input.
        lcd = lcdInit (2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0); // (ROWS, COLUMNS, BIT MODE, LCD_RS, LCD_E, LCD_D0, LCD_D1, LCD_D2, LCD_D3, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
        printf("começou\n");
        int rc;

        // MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

        MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
        // conn_opts.keepAliveInterval = 20;
        conn_opts.keepAliveInterval = 999;
        conn_opts.cleansession = 1;
        conn_opts.username = "aluno";
        conn_opts.password = "@luno*123";
    // int rc;
    printf("dois\n");


        /* Inicializacao do MQTT (conexao & subscribe) */
        MQTTClient_create(&client, MQTT_ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
        MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);
        printf("3\n");

        rc = MQTTClient_connect(client, &conn_opts);

        if (rc != MQTTCLIENT_SUCCESS)
        {
                printf("\n\rFalha na conexao ao broker MQTT. Erro: %d\n", rc);
                exit(-1);
        }

        printf("quatro\n");
        // publicacao
    // char* payload = "UM TESTE\n";
    // int payloadlen = strlen(payload);
    int qos = 1;
    // int retained = 0;
    MQTTClient_deliveryToken dt;
    // rc = MQTTClient_publish(client, MQTT_PUBLISH_TOPIC, payloadlen, payload, qos, retained, &dt);

    printf("cinco\n");

    // MQTTClient_message msg = MQTTClient_message_initializer;
    // msg.payload = "UM TESTE\n";
    // msg.payloadlen = strlen(payload);

    // msg.retained = 0;
    // MQTTClient_deliveryToken dt;
    // rc = MQTTClient_publishMessage(client, topicName, msg, &dt);

    //   subscribe

    //const char* topic = "mytopic";
    printf("66\n");
    // rc = MQTTClient_subscribe(client, MQTT_SUBSCRIBE_TOPIC, qos);
        rc = MQTTClient_subscribe(client, OKSTATUSMQTT, qos);
        rc = MQTTClient_subscribe(client, ANALT, qos);
        rc = MQTTClient_subscribe(client, SD1GT, qos);
        rc = MQTTClient_subscribe(client, SD2GT, qos);
        rc = MQTTClient_subscribe(client, LEDT, qos);
        rc = MQTTClient_subscribe(client, ALLBROADCASTT, qos);

    //   RECEBER mensagens

        printf("set\n");
        // MQTTClient_subscribe(client, MQTT_SUBSCRIBE_TOPIC, 1);
    printf("oito\n");

        //intAUX = nodeCodes[33]; // converte char to int
        //sprintf(okStatusMQTT, "%d", intAUX);  // converte int to str

        while(1)
        {       /* o exemplo opera por "interrupcao" no callback de recepcao de mensagens MQTT. Portanto, neste laco principal, com relação ao MQTT, não é preciso fazer nada.*/
                if(digitalRead(DIP4) == 1)
                {
                        printf("ligado\n");
                        printf("\n");
                        chooseOptions(); // ESCOLHER NODE OU ENVIAR MENSAGEM PARA TODAS AS NODES
                }
                else
                {
                        changeControlValues();
                        lcdClear(lcd);
                }
        }
        serialClose(fd);
        return 0;
}

// char* getPayloadResponse(char plr[])
// {
//      // sprintf(int_str, "%d", v);
//      // return int_str;
// }


// (sensor
// [sensor
// sensor =
// sensor++
// sensor--