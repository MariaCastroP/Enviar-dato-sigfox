#include <SoftwareSerial.h> //modificado by victor palma ************
SoftwareSerial mySerial(4, 5); // RX, TX  //modificado by victor palma********

//Programa de ejemplo utilizando el sensor ultrasonico HC-SR04
//En este caso el dato en flotante es transformado a su representacion en hexadecimal en formato float 32 little-endian
#define trigPin 6
#define echoPin 8
#define boton 7//const int boton=2;
int temporizador=0;
int tempo_pago=0;
//*****************************************************
String bufer; //variable donde guardaremos nuestro payload************
String bufer2="\n";   //agregamos un salto de linea al final de nuestro payload ***********+
//*****************************************************
void setup() 
{
  Serial.begin(9600);
  mySerial.begin(9600);// modificado by victor palma
  while (!Serial) {;}    // modificado by victor palma

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(boton, OUTPUT);//pinMode(boton, INPUT);
  pinMode(3, OUTPUT);   //enable modulo wisol
}

void leer_distancia()
{
  float duracion;  
  int distancia;

  digitalWrite(trigPin, LOW);        
  delayMicroseconds(2);              
  digitalWrite(trigPin, HIGH);       
  delayMicroseconds(10);            
  digitalWrite(trigPin, LOW);       
  duracion = pulseIn(echoPin, HIGH) ;
  distancia = duracion / 2 / 29.1;

  Serial.println("Nueva Lectura Sensor Ultrasonico");// modificado by victor palma
  Serial.print("Distancia: ") ;
  Serial.println(distancia) ;
  
  temporizador= temporizador+2;
  
  if (temporizador>=7200) 
  { 
    tempo_pago=tempo_pago+1;
  //-----------------------------------------------------
  //AT$SF= comando para mandar la informacion por sigfox
  //Maximo 12 bytes
  bufer="AT$SF="; //
  //-----------------------------------------------------
  //agregamos nuestro valor de la distancia al payload a enviar
  add_int(distancia); //un flotante ocupa 4 bytes
  
  //enviamos nuestro dato por Sigfox
  send_message(bufer);
    if(tempo_pago>=3)
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
  }
  Serial.print(temporizador);
 
}

void loop() 
{
  
  //digitalWrite(boton,HIGH);// borrar test 
  //delay(1000);// borrar test
  //digitalWrite(boton,LOW);// borrar test
  //delay(10000);// borrar test

  if (digitalRead(boton)==LOW)
  { 
    leer_distancia();
    delay(1000);
  } 
}
void add_float(float var1) //funcion para agregar flotantes al payload
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
void add_int(int var2)    //funcion para agregar enteros al payload (hasta 255)
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