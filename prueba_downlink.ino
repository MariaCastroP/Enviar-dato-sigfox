#include <SoftwareSerial.h> 
SoftwareSerial mySerial(4, 5); 
char Response_Sigfox[50];  // Agregamos Variable para recibir respuestas del modulo
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


void setup() 
{
  
  sei(); // Enable interrupts
  Serial.begin(9600);
  mySerial.begin(9600);
  while (!Serial) {;}    
  pinMode(3, OUTPUT);   //enable modulo wisol
  digitalWrite(3,0);         // LINEA QUE MANTIENE EL WISOL APAGADO (0-APAGADO, 1-ENCENDIDO)
}


int prueba = 0;
void loop() 
{
  encenderSigfox();
  Send_CMD_SF("AT", "OK", 1000);
  Send_CMD_SF("AT$I=10", " ", 1000);
  Send_CMD_SF("AT$I=11", "OK", 1000);
  Send_CMD_SF("AT$T?", "OK", 1000);
  Send_CMD_SF("AT$V?", "OK", 1000);
  while(prueba==0){
  Send_CMD_SF("AT$RC", "OK",2000);
  Send_CMD_SF("AT$SF=FFFF,1", "||",39000);
  Get_Downlink();
  apagarSigfox(); 
  prueba= 3;
}
Serial.println("Sale de funcion reiniciar para proximo downlink");
delay(5000);


}

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
