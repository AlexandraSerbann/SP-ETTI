
#include <RHReliableDatagram.h>
#include <RH_NRF24.h> 
#include <SPI.h>

#include <LiquidCrystal.h>

 
// Definim conexiunile joystickului la pini analogici 1 si 2
#define joyVert    A0 
#define joyHorz    A1
 
// Definim valorile neutre ale joystickului [0;1023]
int joyposVert = 512;
int joyposHorz = 512;
 
// Definim adresele pentru canalele radio
#define CLIENT_ADDRESS 1   
#define SERVER_ADDRESS 2
 
// Cream o instanta a driverului radio
RH_NRF24 RadioDriver;
 
// Seteaza radio driverul la NRF24 si client address la 1
RHReliableDatagram RadioManager(RadioDriver, CLIENT_ADDRESS);
 
// Declaram un vector de tip unsigned 8-bit pentru controlul motoarelor
// 2 Bytes pentru vitezele motoarelor si 1 byte pentru directia lor
uint8_t motorcontrol[3]; 
 
// Definim Message Buffer-ul(raspunsul de la receptor(masina))
uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];

// Definim conexiunile displayului
const int rs=2, en=3, d4=4, d5=5, d6=6, d7=7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
 
void setup()
{
  //Setam cu ajutorul pinului ~9(PWM) intensitatea displayului
  analogWrite(9,100);
  //Aprindem lcd-ul
  lcd.begin(16, 2);
 
  // Initializam RadioManager inplicit - 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!RadioManager.init()){
    //setam cursorul lcd-ului pe pozitia 0,0
    lcd.setCursor(0,0);
    //printam
    lcd.print("initializare esuata!");
  }
}
 
void loop()
{
  // Citim pozitiile x si y ale joystickului 
  joyposVert = analogRead(joyVert); 
  joyposHorz = analogRead(joyHorz);
  
  // Determinam daca ne miscam in fata sau un spate
  // Ne folosim de pozitia verticala a joystickului 
  // Si aplicam rezultatele vitezei motoarelor si directiei

  //Ne luam o marja de eroare deoarece joystickul nu este perfect calibrat
  if (joyposVert < 460)
  {
    // Motoarele setate sa mearga in fata
    motorcontrol[2] = 0;
 
    //Convertim valorile date de joystick in valori pentru motoare
    //Cu ajutorul functiei map(de la 460 la 0 in: de la o pana la 255)
    motorcontrol[0] = map(joyposVert, 460, 0, 0, 255);
    motorcontrol[1] = map(joyposVert, 460, 0, 0, 255);
 
  }
  else if (joyposVert > 564)
  {
    // Motoarele setate sa mearga in fata
    motorcontrol[2] = 1;
 
    motorcontrol[0] = map(joyposVert, 564, 1023, 0, 255);
    motorcontrol[1] = map(joyposVert, 564, 1023, 0, 255); 
 
  }
  else
  {
    //Motoare oprite
    motorcontrol[0] = 0;
    motorcontrol[1] = 0;
    motorcontrol[2] = 0; 
 
  }
  
  // Pentru viraje ne folosim de pozitia orizontala a joystickului
  if (joyposHorz < 460)
  {
    // Dreapta=Motorul din stanga accelerat si motorul din dreapta incetinit
    joyposHorz = map(joyposHorz, 460, 0, 0, 255);
 
    motorcontrol[0] = motorcontrol[0] + joyposHorz;
    motorcontrol[1] = motorcontrol[1] - joyposHorz;
 
    // Conditii sa nu depasim valorile maxime [0;255]
    if (motorcontrol[0] < 0)motorcontrol[0] = 0;
    if (motorcontrol[1] > 255)motorcontrol[1] = 255;
 
  }
  else if (joyposHorz > 564)
  {
    //Stanga=Motorul din dreapta accelerat si motorul din stanga incetinit
    joyposHorz = map(joyposHorz, 564, 1023, 0, 255);
  
    motorcontrol[0] = motorcontrol[0] - joyposHorz;
    motorcontrol[1] = motorcontrol[1] + joyposHorz;
 
    //Conditii
    if (motorcontrol[0] > 255)motorcontrol[0] = 255;
    if (motorcontrol[1] < 0)motorcontrol[1] = 0;      
 
  }
  
  //Trimitem un mesaj care contine controlul motoarelor catre receptor(masina)
  if (RadioManager.sendtoWait(motorcontrol, sizeof(motorcontrol), SERVER_ADDRESS))
  {
    // Mesajul primit
    uint8_t len = sizeof(buf);
    uint8_t from;
    if (RadioManager.recvfromAckTimeout(buf, &len, 2000, &from))
    {
      //stergem ecranul si afisam la pozitia 7,0 media vitezelor calculata de senzorii optici de pe masina, informatie transmisa inapoi catre transmitator(joystick) de receptor(masina)
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Viteza:");
      
      lcd.setCursor(9,0);
      lcd.print((buf[1]+buf[0])/2);

      lcd.setCursor(9,1);
      lcd.print("cm/min");
    }
    else
    { 
      lcd.clear();
      lcd.setCursor(5,0);
      lcd.print("Masina");
      lcd.setCursor(2,1);
      lcd.print("deconectata!");
    }
  }
  else{
    //Daca dupa timpul de asteptare al raspunsului este depasit
    lcd.clear();
    lcd.setCursor(5,0);
    lcd.print("Masina");
    lcd.setCursor(2,1);
    lcd.print("deconectata!");
  } 
  //Timpul de asteptare pana la urmatoarea trasmisiune(50ms)
  delay(50);
}
