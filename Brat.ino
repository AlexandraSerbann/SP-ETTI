#include <Servo.h>
#include "IRremote.h"

//declarare globala variabile de tip Servo
Servo cleste; 
Servo baza;
Servo inc1;
Servo inc2;

//declarare de tip int in care vom stoca pozitita fiecarui servomotor
int pos = 150; //pozitita initiala: cleste
int pos2=90; //baza
int pos3=170; //inc1
int pos4=150; //inc2

//declarare in care vom stoca ultimul cod dat de telecomanta
unsigned long lastCode;

//setare pin receiver
IRrecv irrecv(4);  
//variabila care primeste codul dat de telecomanda  
decode_results results; 

void cursuri_la_ME(){
  pos3=0;
  pos4=180;
  inc1.write(pos3);
  inc2.write(pos4);  
  delay(500);
  pos3=30;
  pos4=100;
  pos=150;
  inc1.write(pos3);
  inc2.write(pos4);
  delay(250);
  cleste.write(pos);
}

void setup() {
  //urmatoarele doua linii ne opresc ledul integrat de pe arduino
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW); 
  //activeaza receiverul
  irrecv.enableIRIn();
  //setare pini pentru servomotoare
  cleste.attach(9);
  baza.attach(6);
  inc1.attach(10);
  inc2.attach(11);
  //setare pozitii initiale pentru servomotoare
  cleste.write(pos);
  baza.write(pos2);
  inc1.write(pos3);
  inc2.write(pos4);
  //joc initial cleste
  pos=60;
  delay(250);
  cleste.write(pos);
  pos=150;
  delay(250);
  cleste.write(pos);
  //2
  pos=60;
  delay(250);
  cleste.write(pos);
  pos=150;
  delay(250);
  cleste.write(pos);
  delay(250);
  cleste.detach();
}

void loop() {
  //daca primeste un cod se executa 
  if(irrecv.decode(&results)){
    
    //cand rezultatul este cel de repetare atunci executam ultimul cod primit
    if(results.value == 0xFFFFFFFF){
        results.value=lastCode;
      }

      if(results.value == 0x410817E){ //ch-
        lastCode=results.value;
        cleste.attach(9);
        pos+=5;
        if(pos>150) pos=150;
        cleste.write(pos);
        delay(50);
        cleste.detach();
      }

      if(results.value == 0x4107E81){ //ch+
        lastCode=results.value;
        cleste.attach(9);
        pos-=5;
        if(pos<60) pos=60;
        cleste.write(pos);
        //nu mai avem nevoie de delay si sa detasam servomotorul deoarece clestele trebuie sa mentina presiune cand se inchide
      }

      if(results.value == 0x4109E61){ //ok
        if(pos>60){
          lastCode=0x4107E81; //ch+
          cleste.attach(9);
          pos=60;
          cleste.write(pos);
        }
        else{
          lastCode=0x410817E; //ch-
          cleste.attach(9);
          pos=150;
          cleste.write(pos);
          delay(250);
          cleste.detach();
        }   
      }

      if(results.value == 0x4101EE1){ //stanga
        //daca actionezi incontinuu butonul se va repeta aceasi miscare caracteristica codului
        lastCode=results.value;
        //adaugam cate putin pentru deplasarea pozitiei
        pos2+=5;
        //ne asiguram ca nu depasim limitele servomotorului(0-180);
        if(pos2>180) pos2=180;
        //servomotorul se duce la noua pozitie
        baza.write(pos2);
      }

      if(results.value == 0x4105EA1){ //dreapta
        lastCode=results.value;
        pos2-=5;
        if(pos2<0) pos2=0;
        baza.write(pos2);
      }

      if(results.value == 0x410DE21){ //jos
        lastCode=results.value;
        pos3+=5;
        if(pos3>180) pos3=180;
        inc1.write(pos3);
      }

      if(results.value == 0x410EE11){ //sus
        lastCode=results.value;
        pos3-=5;
        if(pos3<0) pos3=0;
        inc1.write(pos3);
      }

      if(results.value == 0x4106E91){ //josjos
        lastCode=results.value;
        pos4+=5;
        if(pos4>180) pos4=180;
        inc2.write(pos4);
      }

      if(results.value == 0x410BE41){ //sussus
        lastCode=results.value;
        pos4-=10;
        if(pos4<0) pos4=0;
        inc2.write(pos4);
      }

      if(results.value == 0x41000FF){
        lastCode=results.value;
        cursuri_la_ME();
      }

      //delay pentru a ne asigura ca se primeste tot codul
      delay(30);
      //se elibereaza valoare din value apoi intra din nou in void loop 
      irrecv.resume();
    }
}
