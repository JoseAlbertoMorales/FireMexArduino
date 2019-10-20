#include <MQ2.h>
#include <SoftwareSerial.h>
SoftwareSerial SIM900(7,8);

//change this with the pin that you use
int pin = A0;
int smq2[3];
unsigned long count,s;
const int ledPin = 13;
int sensoract = 3;
int buttonState = 0;
String i;
String f;
unsigned long l,x;
bool truebom;

MQ2 mq2(pin);

void setup() {
    Serial.begin(9600);
    SIM900.begin(9600);
    mq2.begin();
    power_on();
}

void loop() {
  SensorCo();
  enviarDatos();
  
}


void SensorCo(){
    float* values= mq2.read(true); //set it false if you don't want to print the values in the Serial
  
    //lpg = values[0];
    smq2[0] = mq2.readLPG();
    //co = values[1];
    smq2[1] = mq2.readCO();
    //smoke = values[2];
    smq2[2] = mq2.readSmoke();
  }

void enviarDatos(){
  if(enviarAT("AT+CREG?", "+CREG: 0,1", 1000) == 1)
  {
    delay(500);
    SIM900.println("AT+CCLK?");
    delay(2000);
    while(SIM900.available()!=0)
      {
      i = SIM900.readStringUntil(' ');
      }
    i= i.substring(1,18);
    Serial.println(i);
    delay(500); 
    enviarAT("AT+HTTPINIT", "OK", 2000); //init the HTTP request
    enviarAT("AT+HTTPPARA=\"CID\",1", "OK",2000);
    enviarAT("AT+HTTPPARA=\"URL\",\"agromatic.online/fp/e.php?id=\'BM001\'&e=\'ENCENDIDO\'\"", "OK",2000);//estado de bomba 
    delay(1000);
    enviarAT("AT+HTTPACTION=0","+HTTPACTION:0,200,",10000);
    ShowSerialData();
    enviarAT("AT+HTTPTERM", "ok",2000);
    truebom = true;
    return;
  }
  }

int enviarAT(String ATcommand, char* resp_correcta, unsigned int tiempo)
{

  int x = 0;
  bool correcto = 0;
  char respuesta[100];
  unsigned long anterior;

  memset(respuesta, '\0', 100); // Inicializa el string
  delay(100);
  while ( SIM900.available() > 0) SIM900.read(); // Limpia el buffer de entrada
  SIM900.println(ATcommand); // Envia el comando AT
  x = 0;
  anterior = millis();
  // Espera una respuesta
  do {
    // si hay datos el buffer de entrada del UART lee y comprueba la respuesta
    if (SIM900.available() != 0)
    {
        respuesta[x] = SIM900.read();
        x++;
      // Comprueba si la respuesta es correcta
      if (strstr(respuesta, resp_correcta) != NULL)
      {
        correcto = 1;
      }
    }
  }
  // Espera hasta tener una respuesta
  while ((correcto == 0) && ((millis() - anterior) < tiempo));
  Serial.println(respuesta);

  return correcto;
}

void power_on()
{
  int respuesta = 0;

  // Comprueba que el modulo SIM900 esta arrancado
  if (enviarAT("AT", "OK", 2000) == 0)
  {
    Serial.println("Encendiendo el GPRS...");

    pinMode(9, OUTPUT);
    digitalWrite(9, HIGH);
    delay(1000);
    digitalWrite(9, LOW);
    delay(1000);

    // Espera la respuesta del modulo SIM900
    while (respuesta == 0) {
      // Envia un comando AT cada 2 segundos y espera la respuesta
      respuesta = enviarAT("AT", "OK", 2000);
      SIM900.println(respuesta);
    }
  }
}

void power_off()
{
  digitalWrite(9, HIGH);
  delay(1000);
  digitalWrite(9, LOW);
  delay(1000);
}

void reiniciar()
{
  Serial.println("Reiniciando...");
  power_off();
  delay (5000);
  power_on();
}

void iniciar()
{
  SIM900.println("AT+CSQ");
  delay(1000);
  ShowSerialData();
  SIM900.println("AT+CPIN?");
  ShowSerialData();
  delay(1000);
  Serial.println("Conectando a la red...");
  delay (5000);

  //Espera hasta estar conectado a la red movil
  while ( enviarAT("AT+CREG?", "+CREG: 0,1", 1000) == 0 )
  {
  }

  Serial.println("Conectado a la red.");
  
  enviarAT("AT+CGATT=1\r", "OK", 1000);
  
  SIM900.println("AT+CGATT?"); //Attach or Detach from GPRS Support
  delay(1000);
  ShowSerialData();

  enviarAT("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"", "OK",1000);//setting the SAPBR, the connection type is using gprs
  enviarAT("AT+SAPBR=3,1,\"APN\",\"internet.itelcel.com\"", "OK",1000);//setting the APN, Access point name string
  enviarAT("AT+SAPBR=3,1,\"USER\",\"webgprs\"", "OK",1000);//setting the APN, Access point name string
  enviarAT("AT+SAPBR=3,1,\"PWD\",\"webgprs2002\"", "OK",1000);//setting the APN, Access point name string
  enviarAT("AT+SAPBR=1,1", "OK",1000);//setting the SAPBR
  SIM900.println("AT+SAPBR=2,1");
  delay(1000);
  ShowSerialData();
}


void ShowSerialData()
{
  while(SIM900.available()!=0)
    Serial.print(char (SIM900.read()));
 } 
