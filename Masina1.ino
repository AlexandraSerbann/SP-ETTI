#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <SPI.h>

#include "TimerOne.h"
 
// Definim adresele pentru canalele radio
#define CLIENT_ADDRESS 1   
#define SERVER_ADDRESS 2
 
// 1.Definim conexiunile pentru motorul drept
int enA = 9;
int in1 = 14; //pinul A0, deoarece nu mai aveam pini digitali la dispozitie
int in2 = 4;
 
// Definim conexiunile pentru motorul stang
int enB = 5;
int in3 = 7;
int in4 = 6;

// 2.Definim constante pentru pini cu functia Interrupt
//Pentru Arduino Uno numai pinii 2 si 3 au functia de Interrupter
const byte MOTOR1 = 2; //motorul drept
const byte MOTOR2 = 3; //motorul stang
 
//Definim intregi care vor stoca nr de pulsuri de pe Encoder
unsigned int counter1 = 0;
unsigned int counter2 = 0;
 
//Definim nr de goluri in Encoder
float diskslots = 20.00;
//Diametrul rotilor
float diametru= 6.61; //cm

// 3.Declaram unsigned 8-bit array pentru a ne returna vitezele
// 1 Byte pentru fiecare viteza 
uint8_t ReturnMessage[2];
 
// Definim Message Buffer-ul(comenzile de la transmitor(joystick))
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

// Cream o instanta a driverului radio
RH_NRF24 RadioDriver;
 
// Seteaza radio driverul la NRF24 si client address la 1
RHReliableDatagram RadioManager(RadioDriver, SERVER_ADDRESS);
 
// Numara pulsatiile pentru motorul drept
void ISR_count1()  
{
  counter1++;  
} 
 
// Numara pulsatiile pentru motorul stang
void ISR_count2()  
{
  counter2++; 
} 

// Calculam viteza motorului drept
float viteza_dreapta(){
  float rpm = (counter1/diskslots)*60.00;
  float circumferinta= 3.14* diametru;
  counter1=0;
  return rpm*circumferinta;
}

// Calculam viteza motorului stang
float viteza_stanga(){
  float rpm = (counter2/diskslots)*60.00;
  float circumferinta= 3.14* diametru;
  counter2=0;
  return rpm*circumferinta;
}
 
void setup()
{
  //Setup pentru serial monitor
  Serial.begin(9600);

  //Crestem count1 si count2 daca senzorii optici specifici sunt activati(deranjati)
  attachInterrupt(digitalPinToInterrupt (MOTOR1), ISR_count1, RISING);//activat cand pinul trece de la low la high
  attachInterrupt(digitalPinToInterrupt (MOTOR2), ISR_count2, RISING);  
  
  // Setam toti pini motoarelor ca output
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  
  // Initializam RadioManager inplicit - 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!RadioManager.init())
    //Afisam pe serial monitor
    Serial.println("initializare esuata!");
} 
 
void loop()
{
  //Verificam daca am primit un semnal
  if (RadioManager.available())
  {
    //Mesajul pe care il returnam
    ReturnMessage[0]=viteza_dreapta(); 
    ReturnMessage[1]=viteza_stanga();
    //Mesajul primit
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (RadioManager.recvfromAck(buf, &len, &from))
 
    { 
      // Setam directia motoarelor
      if (buf[2] == 1)
      {
    // Motoarele setate sa mearga in spate
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    }else{
    // Motoarele setate sa mearga in fata
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
     }
 
      
      // Setam viteza motoarelor
      analogWrite(enA, buf[1]);
      analogWrite(enB, buf[0]);
     
      // Trimitem inapoi informatiile si verificam posibile erori de conexiune
      if (!RadioManager.sendtoWait(ReturnMessage, sizeof(ReturnMessage), from))
        //Afisam pe serial monitor
        Serial.println("Joystick deconectat!");
    }
  }              
}
