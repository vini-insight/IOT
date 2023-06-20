
/* ======================================================================================================================================================================== */
/*                                      INICIO DAS FUNÇÕES DE TESTE (REMOVER QUANDO TESTAR NA ORANGE PI)                                                                    */
/* ======================================================================================================================================================================== */
#define INPUT 0
#define PUD_UP 1

char espnode = 0b00000001;
char analog = 0b00100010;
char dig1 = 0b00100011;
char dig2 = 0b00100100;
char ledON = 0b00100101;
char ledOFF = 0b00100110;
char okStatus = 0b00000000;
char request = 0b00100001;
char responseCode = 0b00111111;
char allNodesMCU = 0b00111111;

char key = ' ';
int numbeOFchars = 0;

void clearKey();
void readKey();
int digitalRead(char);
void delay(int);
void uart(char msg[]);
void serialPrintf(int, char msg[]);
void serialPutchar(int, char msg[]);
// bool serialDataAvail(int);
// char serialGetchar(int);
int serialDataAvail(int);
int serialGetchar(int);
void lcdPosition(int, int, int);
void lcdPrintf(int, char msg[], int, int);
void lcdClear(int);
int wiringPiSetup();
int serialOpen();
void pinMode(char, int);
void pullUpDnControl(char, int);
int lcdInit(int, int, int, int, int, int, int, int, int, int, int, int, int);
void serialClose(int);
void displayPrintf(int, char msg[], int, float);

// char subButton = 19;
// char selectButton = 23;
// char addButton = 25;

// void displayPrintf(int lcd, char msg[], int in1, float in2)
// {
//   printf("TESTE VOLTS = %f\n", in2);
//   if(in1 != 1025)
//     printf(msg, in1);
//   else if(in2 != (float) 1025)
//     printf(msg, in1, in2);
//   else
//     printf(msg);
// }

void clearKey()
{
    key = ' ';
}

void readKey()
{
  scanf("%c", &key);
  while(key != 87 && key != 119 && key != 65 && key != 97 && key != 68 && key != 100)
  {
    scanf("%c", &key);
  }
}

int digitalRead(char b)
{
  // sleep(1);
  if(b == selectButton && (key == 87 || key == 119)) // W or w
  {
    return 0;
  }
  else if(b == addButton && (key == 68 || key == 100)) // D or d
  {
    return 0;
  }
  else if(b == subButton && (key == 65 || key == 97)) // A or a
  {
    return 0;
  }
  else
  {
    return 1;
  }
}

void delay(int t)
{
  usleep(t); // Halting the execution for  Microseconds (10 seconds)
}

void uart(char msg[])
{

  // if(numbeOFchars == 0)
  //   numbeOFchars = 10;
  if(msg == analog)
    numbeOFchars = 10;
  else
    numbeOFchars = 1;
  request = msg;
}
void serialPrintf(int fd, char msg[])
{
  printf("%c", msg);
  printf("\n");
  printf("%d", msg);
  printf("\n");
  // uart();
}

void serialPutchar(int fd, char msg[])
{
  printf("putchar \n");
  printf("code %c", msg);
  printf("\n");
  printf("code %d", msg);
  printf("\n");
  printf("putchar \n");
  uart(msg);
}

// bool serialDataAvail(int fd)
int serialDataAvail(int fd)
{
  // printf("dagaAVAIL\n");
  if(numbeOFchars == 0)
    request = 0b00100001;
  if(numbeOFchars > -1)
    return numbeOFchars;
  else
    return -1;
}
// char serialGetchar(int fd)
int serialGetchar(int fd)
{
  // numbeOFchars--;
  // srand(time(0));
  // int num = rand();
  // if(num % 2 == 0)
  //     return (char) 48;
  // else
  //     return (char) 49;

  // printf("getCHAR\n");
  numbeOFchars--;
  delay(250000);
  if(request == espnode || request == ledOFF)
    return 48;
  else if(request == ledON)
    return 49;
  else if(request == analog || request == dig1 || request == dig2)
  {
    srand(time(0));
    int num = rand();
    if(num % 2 == 0)
      return (char) 48;
    else
      return (char) 49;
  }
  else
    return -1;
}
void lcdPosition(int lcd, int c, int l)
{
  // FAZ NADA
}

void lcdPrintf(int lcd, char msg[], int in1, int in2)
{
  if(in1 != 1025)
    printf(msg, in1);
  else if(in2 != 1025)
    printf(msg, in1, in2);
  else
    printf(msg);
}

void lcdClear(int lcd)
{
  // FAZ NADA
}
int wiringPiSetup()
{
  return 2;
}
int serialOpen()
{
  return 2;
}
void pinMode(char b, int i)
{
  // FAZ NADA
}
void pullUpDnControl(char b, int i)
{
  // FAZ NADA
}
int lcdInit(int l, int c, int m, int rs, int e, int d4, int d5, int d6, int d7, int w, int x, int y, int z)
{
  // FAZ NADA
}
void serialClose(int fd)
{
  // 
}
/* ======================================================================================================================================================================== */
/*                                      FIM DAS FUNÇÕES DE TESTE (REMOVER QUANDO TESTAR NA ORANGE PI)                                                                       */
/* ======================================================================================================================================================================== */
