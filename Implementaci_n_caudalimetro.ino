#include <SoftwareSerial.h> 
SoftwareSerial mySerial(4, 5); 
//--------------Caudalimetro-----------------
unsigned char flowsensor = 2;
volatile int flow_frequency;  // Measures flow sensor pulses
float l_hour; //unsigned int l_hour;          // Calculated litres/hour
unsigned long currentTime;
unsigned long cloopTime;
float volumen=0; //----prueba
int volint=0; //----prueba
long dt=0; //----prueba

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

#define boton 7//const int boton=2;
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
  //-----------------------------------------------------
  temporizador= temporizador+2;
  
  if (temporizador>=1200) 
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
    if(tempo_pago>=8)
    {
      bufer="AT$SF=";
      int cmdpago= 65535;
      String cmdpagar;
      cmdpagar=String(cmdpago, HEX);
      bufer+=cmdpagar; 
      send_message(bufer);
      tempo_pago=0;
    }
  temporizador=0;
  volumen=0;
  }
  Serial.print(temporizador);
 
}

void loop() 
{
  
  if (digitalRead(boton)==LOW)
  { 
    leer_distancia();
    delay(1000);
  } 
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
