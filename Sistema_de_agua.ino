#include <LiquidCrystal.h>
#include <SoftwareSerial.h> 
SoftwareSerial mySerial(4, 5); 
//- - - - - - - Datos downlink- - - - - - - - - 
char Response_Sigfox[50];  
//(*Funcion Para envio y recepcion de mensajes)
bool Send_CMD_SF(char *AT_CMD, char *Response, unsigned long Time_Wait)  
{
  bool ok = 0;
  uint32_t Last_Time;
  uint32_t Time_Elapsed = 0;
  uint8_t Buffer = 0;
  memset(Response_Sigfox, '\0', sizeof(Response_Sigfox));
  Serial.print("\n--------> CMD to Send to Wisol : ");
  Serial.println(AT_CMD);
 // esp_task_wdt_reset();
  while (mySerial.available())
  {
    mySerial.read();
  }
  mySerial.print(AT_CMD);
  mySerial.print("\n");
  //esp_task_wdt_reset();
  Last_Time = millis();
  while (Time_Elapsed <= Time_Wait)
  {
    //esp_task_wdt_reset();
    Time_Elapsed = millis() - Last_Time;
   // Get_Data_GPS();
    if (mySerial.available())
    {
      while (mySerial.available())
      {
        //esp_task_wdt_reset();
        Response_Sigfox[Buffer] = mySerial.read();
        Buffer++;
      //  Get_Data_GPS();

      }
      if (strstr(Response_Sigfox, Response))
      {
        Time_Elapsed = 90000;
      }
    }
  }
  if (strstr(Response_Sigfox, Response) != NULL)
  {
    Serial.print("\n<--------  Response Wisol : ");
    Serial.println(Response_Sigfox);
    ok = 1;
  }
  Serial.print("\n<--------  Response Wisol : ");
  Serial.println(Response_Sigfox);

  return ok;
}
//- - - - - - - - - - - - - - - - - - - - - - -
//--------------Caudalimetro-----------------
unsigned char flowsensor = 2;
volatile int flow_frequency;  // Measures flow sensor pulses
float l_hour; //unsigned int l_hour;          // Calculated litres/hour
unsigned long currentTime;
unsigned long cloopTime;
float volumen=0; //----prueba
int volint=0; //----prueba
long dt=0; //----prueba
LiquidCrystal lcd(A0,8, A1, A2, A3, A4, A5);//(13,12, 11, 10, 9, 8, 7); //(RS, 12 RW, E, D4,D5, 8D6, 7D7)
/*Función que hace el conteo del flujo de los pulsos del sensor
   de agua esta función es un interrupción del programa principal
*/
void flow () // Interrupt function
{
  flow_frequency++;
}

//-----------------------------------------------------------------------
void WaterFlowSensorYF_S201() 
{
  currentTime = millis();
  // Every second, calculate and print litres/hour
  if (currentTime >= (cloopTime + 1000))
  {
    cloopTime = currentTime; // Updates cloopTime
    // Pulse frequency (Hz) = 7.5Q, Q is flow rate in L/min.
    l_hour = (flow_frequency*60/ 7.5); // (Pulse frequency*60 ) / 7.5Q = flowrate in L/hour
    flow_frequency = 0; // Reset Counter
    Serial.print("F=");
    Serial.println(l_hour, DEC); // Print litres/hour
  }
}
//-----------------------------------------------------------------------
void SumVolume(float dV) //----prueba
{
   volumen += dV / 60 * (millis() - currentTime) / 100.0; //----prueba
   currentTime = millis(); //---prueba
}
//-----------------------------------------------------------------------

const int boton=13;//#define boton 13
int temporizador=0;
int tempo_pago=0;
//---------------------------Añadir Payload------------------------
String bufer; //variable donde guardaremos nuestro payload
String bufer2="\n";   //agregamos un salto de linea al final de nuestro payload
//-------------------------------------------------------------------
void setup() 
{
    //--------- CONFIGURA CAUDALIMETRO----------
  pinMode(flowsensor, INPUT);
  //---------------------------------------------------
  digitalWrite(flowsensor, HIGH); // Optional Internal Pull-Up
  attachInterrupt(0, flow, RISING); // Setup Interrupt
  sei(); // Enable interrupts
  currentTime = millis();
  cloopTime = currentTime;
  //---------------------------------------------------
  Serial.begin(9600);
  mySerial.begin(9600);
  while (!Serial) {;}    
  pinMode(boton, OUTPUT);//pinMode(boton, INPUT);
  pinMode(3, OUTPUT);   //enable modulo wisol
  digitalWrite(13,HIGH); //solenoide
  //--------------LCD-------------------------
  pinMode(A0, OUTPUT); //definimos los pines analogicos como salidas digitales
  pinMode(8, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  lcd.begin(16, 2);                       //tamaño (#columnas, #filas) de la pantalla LDC que voy a usar
  lcd.clear();     
  lcd.begin(16, 2);            // Inicia el LCD 16x02 (columnas, filas)  
  lcd.setCursor(4, 0);         // Coloca el cursor en las coordenadas (0,0)   
  lcd.print("Flujo:"); // Escribe no LCD   
}

void leer_distancia()
{
  
  WaterFlowSensorYF_S201();   //funcion que envia datos del sensor de flujo
  //---------------------Calcular volumen-------------------------
  SumVolume(l_hour);
  //---------------------Calcular volumen-------------------------
  Serial.print("\t V= ");//----prueba
  Serial.println(volumen,3); // Print litres/hour//----prueba
  volint=int(volumen);
   //--------------------mostrar flujo lcd-----------
  lcd.setCursor(4, 1);         // Coloca el cursor en las coordenadas (3,1) 
  lcd.print(l_hour);
  delay(100);
  //-----------------------------------------------------
  temporizador= temporizador+2;
  
  if (temporizador>=200) 
  { 
  tempo_pago=tempo_pago+1;
  //-----------------------------------------------------
  //AT$SF= comando para mandar la informacion por sigfox
  //Maximo 12 bytes
  bufer="AT$SF="; //
  //-----------------------------------------------------
  //agregamos nuestro valor de la distancia al payload a enviar
  add_int(volint); 
  //enviamos nuestro dato por Sigfox
  send_message(bufer);
    if(tempo_pago>=5)
    {
      //bufer="AT$SF=FFFF,1";
      //send_message(bufer); 
      encenderSigfox();
      Send_CMD_SF("AT$RC", "OK",2000);
      Send_CMD_SF("AT$SF=FFFF,1", "||",45000);
      Get_Downlink();
      apagarSigfox(); 
      tempo_pago=0;
    }
  temporizador=0;
  volumen=0;
  }
  Serial.print(temporizador);
 
}

void loop() 
{
  
  //if (digitalRead(boton)==LOW)
  //{ 
     leer_distancia();
    delay(1000);
  //} 
}

//----------------funcion para agregar flotantes al payload---------------------------

void add_float(float var1) 
{
  byte* a1 = (byte*) &var1;    //convertimos el dato a bytes
  String str1;
  //agregamos al comando AT$SF= nuestra informacion a enviar
  for(int i=0;i<4;i++)
  {
    str1=String(a1[i], HEX);    //convertimos el valor hex a string 
    if(str1.length()<2)
    {
      bufer+=0+str1;    //si no, se agrega un cero
    }
    else
    {
      bufer+=str1;    //si esta completo, se copia tal cual
    }
  }
}

//----------------funcion para agregar enteros al payload (hasta 255)---------------------
void add_int(int var2)    
{
  byte* a2 = (byte*) &var2; //convertimos el dato a bytes
  String str2;
  str2=String(a2[0], HEX);  //convertimos el valor hex a string 
  //verificamos si nuestro byte esta completo
  if(str2.length()<2)
  {
    bufer+=0+str2;    //si no, se agrega un cero
  }
  else
  {
    bufer+=str2;     //si esta completo, se copia tal cual
  }
}

//------------------------Función para mandar datos---------------------
void send_message(String payload)
{
  //agregamos el salto de linea "\n"
  bufer+=bufer2;
  //*******************
  //Habilitamos el modulo Sigfox
  digitalWrite(3, HIGH);
  delay(1000);
  //Reset del canal para asegurar que manda en la frecuencia correcta
  mySerial.print("AT$RC\n"); 
  Serial.print("AT$RC\n");
  //************************
  //Enviamos la informacion por sigfox
  mySerial.print(bufer);
  Serial.print(bufer);
  delay(30000);
  //deshabilitamos el modulo Sigfox
  digitalWrite(3, LOW);
}

//- - - -- - - - - - - obtener  downlink- - - -- - - - - - - - - - - - - - - - - -
void Get_Downlink(){

  char *search;
  char HEX_Data[50];
 Serial.print("Downlin :");
 Serial.println(Response_Sigfox);
  memset(HEX_Data,'\0',sizeof(HEX_Data));
  search=strstr(Response_Sigfox,"RX="); 
  if(search!=NULL){
    if(search[0]=='R'){
      if(search[1]=='X'){
        if(search[2]=='='){
          HEX_Data[0]=search[3];  
          HEX_Data[1]=search[4];
          HEX_Data[2]=search[5];
          HEX_Data[3]=search[6];
          HEX_Data[4]=search[7];
          HEX_Data[5]=search[8];
          HEX_Data[6]=search[9];
          HEX_Data[7]=search[10];
          HEX_Data[8]=search[11];
          HEX_Data[9]=search[12];
          HEX_Data[10]=search[13];  
          HEX_Data[11]=search[14];
          HEX_Data[12]=search[15];
          HEX_Data[13]=search[16];
          HEX_Data[14]=search[17];
          HEX_Data[15]=search[18];
          HEX_Data[16]=search[19];
          HEX_Data[17]=search[20];
          HEX_Data[18]=search[21];
          HEX_Data[19]=search[22];
          HEX_Data[20]=search[23];  
          HEX_Data[21]=search[24];
          HEX_Data[22]=search[25];
          HEX_Data[23]=search[26];
         Serial.print("Dato de downlink 1: ");Serial.println(HEX_Data[0]);
         Serial.print("Dato de downlink 2: ");Serial.println(HEX_Data[1]);
         Serial.print("Dato de downlink 3: ");Serial.println(HEX_Data[2]);
         Serial.print("Dato de downlink 4: ");Serial.println(HEX_Data[3]);
         Serial.print("Dato de downlink 5: ");Serial.println(HEX_Data[4]);
         Serial.print("Dato de downlink 6: ");Serial.println(HEX_Data[5]);
         Serial.print("Dato de downlink 7: ");Serial.println(HEX_Data[6]);
         Serial.print("Dato de downlink 8: ");Serial.println(HEX_Data[8]);
         Serial.print("Dato de downlink 9: ");Serial.println(HEX_Data[9]);
         Serial.print("Dato de downlink 10: ");Serial.println(HEX_Data[10]);
         Serial.print("Dato de downlink 11: ");Serial.println(HEX_Data[11]);
         Serial.print("Dato de downlink 12: ");Serial.println(HEX_Data[12]);
         Serial.print("Dato de downlink 13: ");Serial.println(HEX_Data[13]);
         Serial.print("Dato de downlink 14: ");Serial.println(HEX_Data[14]);
         Serial.print("Dato de downlink 15: ");Serial.println(HEX_Data[15]);
         Serial.print("Dato de downlink 16: ");Serial.println(HEX_Data[16]);
         Serial.print("Dato de downlink 17: ");Serial.println(HEX_Data[17]);
         Serial.print("Dato de downlink 18: ");Serial.println(HEX_Data[18]);
         Serial.print("Dato de downlink 19: ");Serial.println(HEX_Data[19]);
         Serial.print("Dato de downlink 20: ");Serial.println(HEX_Data[20]);        
         Serial.print("Dato de downlink 21: ");Serial.println(HEX_Data[21]);
         Serial.print("Dato de downlink 22: ");Serial.println(HEX_Data[22]);
         Serial.print("Dato de downlink 23: ");Serial.println(HEX_Data[23]);
         int convertido = String(HEX_Data[22]).toInt();
         if(convertido==0)
         {
          digitalWrite(13,HIGH);
          Serial.print("no pago");
          delay(1000);
         }
         else
         {
          digitalWrite(13,LOW);
          Serial.print("pago");
          delay(1000);
         }
        }
      }
    }
  }
} 




void apagarSigfox() {
  while (!Send_CMD_SF("AT", "OK", 1000));
  digitalWrite(3,0); 
  }

void (*resetFunc)(void) = 0; //declare reset function at address 0
/* @ ENCENDER WISOL
 * SE ENCIENDE MÓDULO WISOL, SE ENVÍA UN COMANDO DUMMIE AT Y SE CONFIGURA EN ZONA 2 O 4 SEGÚN SE REQUIERA
 */
void encenderSigfox() {
  uint8_t Cont = 0;
  digitalWrite(3,1);
  delay(3000);
  while (!Send_CMD_SF("AT", "OK", 2000) && Cont < 6) {
    Cont++;
    Serial.print(F("\r\nIntento numero:\r\n"));
    Serial.println(Cont);
    if (Cont >= 6);
      resetFunc();
  }
  #ifdef ZONA4
    Serial.print(F("\r\nConfigurando WISOL en zona 4 (Freq. 920.8 MHz)\r\n"));
    SendATCommnadSF("AT$IF=920800000", "OK", 3);
    SendATCommnadSF("AT$DR=922300000", "OK", 3);
  #endif 
  #ifndef ZONA4
    Serial.print(F("\r\nConfigurando WISOL en zona 2 (Freq. 902.2 MHz)\r\n"));
  #endif 
}
