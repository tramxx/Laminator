#include <EEPROM.h>

#define ENCA    9
#define ENCB    10
#define ENCBTN  
#define VDIODE  
#define LED1    
#define BTN1    
#define BTN2    
#define BTN3    
#define D4      0 
#define D5      1
#define D6      2
#define D7      3
#define RS      4
#define EN      5
#define GRELEC  6
#define MOTOR   7
#define DESNO   1
#define LEVO    2
#define EADDR   0
#define REFRESH 1000

//spremenljivke za menu
unsigned char tempSet;
unsigned int curTemp;
unsigned long time = 0;
unsigned char menu = 0;
unsigned int T1;
unsigned int T2;
unsigned int cal1;
unsigned int cal2;



void setup()
{
  pinMode(ENCA, INPUT);
  digitalWrite(ENCA, HIGH);  //enable pull-up
  pinMode(ENCB, INPUT);
  digitalWrite(ENCB, HIGH);  //enable pull-up
  pinMode(VDIODE, INPUT);
  pinMode(ENCBTN, INPUT);
  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);
  pinMode(BTN3, INPUT);
  
  pinMode(D4, OUTPUT);
  pinMode(D5, OUTPUT);
  pinMode(D6, OUTPUT);
  pinMode(D7, OUTPUT);
  pinMode(RS, OUTPUT);
  pinMode(EN, OUTPUT);
  pinMode(GRELEC, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  pinMode(LED1, OUTPUT);
  tempSet = EEPROM.read(EADDR);
  
}


void loop()
{
  static readNum  =0;
  char encoder = encoderChange();
  curTemp = analogRead(VDIODE);
  
  if(millis() - time > REFRESH || readNum)
  {
    if (readNum == 0) curTemp = 0;
    curTemp = (analogRead(VDIODE) + curTemp);
    readNum++;
    if (readNum > 10) {
      curTemp /= 10;    // preracunaj in izpisi
      readNum = 0;      
      if(curTemp > tempSet) digitalWrite(GRELEC, LOW);
      else if(curTemp < tempSet - 3) digitalWrite(GRELEC, HIGH);
    }
  }
  
  
  
  if(encoder)
  {
    //handler
  }
  
  
  
}


char encoderChange()
{
  static char encTemp = 0;
  char stateA = digitalRead(ENCA);
  char stateB = digitalRead(ENCB);
  if(encTemp == stateA) return 0;
  encTemp = stateA;
  if(stateA == stateB) return DESNO; //desno
  else return LEVO;                //levo
}

void izpisiEkran()
{
  static char stariMenu = 0;
  switch(menu)
  {
    case 0:
      if(stariMenu != menu)
      {
        //izpisi menu0
      }
      else {} //posodobi temperaturo
      break;
    case 1:
      //izpisi menu 1
      break;
    case 11:
      //izpisi menu 11
      break;
    case 12;
      //izpisi menu 12
      break;
    case 3:
      //izpisi izhod
      break;
  }
}
  
