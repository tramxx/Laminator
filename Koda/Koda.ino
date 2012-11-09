#include <EEPROM.h>
#include <LiquidCrystal.h>

#define ENCA    9
#define ENCB    10
#define ENCBTN  1
#define VDIODE  1
#define LED1    1
#define BTN1    1
#define BTN2    1
#define BTN3    1
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
#define ADDRTEMP   0
#define ADDRNAKLON 1
#define ADDRZACVR  2
#define ADDRMEM    9 //(naslov 3 * trijeBajtiNaRazdelek)

#define REFRESH 1000


LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

//spremenljivke za menu
unsigned char tempSet = 0;
unsigned int curTemp = 0;
unsigned long time = 0;
unsigned char menu = 0;
unsigned int T1;
unsigned int T2;
unsigned int cal1;
unsigned int cal2;
double naklon;
double zac_vrednost;
byte grelecState = 0;



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
  tempSet = read_EEPROM(ADDRTEMP);
  lcd.begin(16, 2);
  izpisiEkran();
  
  
  
}

void loop()
{
  static int readNum  = 0;
  char encoder = encoderChange();
  curTemp = analogRead(VDIODE);
  char buttons = btnChange();
  
  if(millis() - time > REFRESH || readNum)
  {
    if (readNum == 0) curTemp = 0;
    curTemp = (analogRead(VDIODE) + curTemp);
    readNum++;
    if (readNum >= 10) {
      curTemp /= 10;
      curTemp = racT(curTemp);
      readNum = 0;      
      if(curTemp > tempSet) grelecState = 0;
      else if(curTemp < tempSet - 3) grelecState = 1;
      digitalWrite(GRELEC, grelecState);
      digitalWrite(LED1, grelecState);
      izpisiEkran();
    }
  }
  if(buttons)
  {
    switch(menu)
    {
      case 0:
        switch(buttons)
        {
          case 0x1:
            menu = 1;
            izpisiEkran();
            break;
          case 0x2:
            EEPROM.read(ADDRMEM);
            break;
          case 0x4:
            EEPROM.read(ADDRMEM + 1);
            break;
          case 0x8: 
            EEPROM.read(ADDRMEM + 2);
            break;
        }
        
        break;
      case 1: 
        if(buttons & 0x1) 
        {
          menu = 11;
          izpisiEkran();
        }
        break;
      case 11: 
        if(buttons & 0x1) 
        {
          menu = 12;
          cal1 = analogRead(VDIODE);
          izpisiEkran();
        }
        if(buttons & 0x2) grelecState = !grelecState;
        digitalWrite(GRELEC, grelecState);
        digitalWrite(LED1, grelecState);
        break;
      case 12: 
        if(buttons & 0x1) 
        {
          menu = 0;
          izpisiEkran();
          cal2 = analogRead(VDIODE);
          racCal();
        }
        break;
      case 2: 
        if(buttons & 0x1) 
        {
          menu = 0;
          izpisiEkran();
        }
        break;
    }
  }
  
  if(encoder)
  {
    switch(menu)
    {
      case 0: 
        curTemp += encoder; //spremenimo nastavljeno temperaturo
        izpisiEkran(); 
        break;
      case 1:
        menu = 2;
        izpisiEkran();
        break;
      case 2:
        menu = 1;
        izpisiEkran();
        break;
      case 11:
        T1 += encoder;
        izpisiEkran();
        break;
      case 12:
        T2 += encoder;
        izpisiEkran();
        break;
    }
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

char btnChange()
{
  static byte btnTemp = 0;
  static long timer = 0;
  byte mask;
  byte btnState = digitalRead(BTN3) << 3 | digitalRead(BTN2) << 2 | digitalRead(BTN1) << 1 | digitalRead(ENCBTN);
  if(btnState == btnTemp) {
    if (timer+2000 < millis()) {
      if (menu == 0) setToMemory(btnState); 
    }
    return 0;
  }
  timer = millis();
  mask = (btnTemp^btnState)&btnTemp;
  btnTemp = btnState;
  delay(20); // Debounce (samo, ko se spremeni stanje)
  return mask;
}


void izpisiEkran()
{
  static char stariMenu = 0;
  switch(menu)
  {
    case 0:
      if(stariMenu != menu)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("T=    C  Z:    C");
        lcd.setCursor(0,1);
        lcd.print("cas=  :  ");
      }
      lcd.setCursor(2, 0);
      lcd.print(curTemp);
      lcd.setCursor(10, 0);
      lcd.print(tempSet);
      lcd.setCursor(4, 0);
      lcd.print(millis()/1000/60);
      lcd.setCursor(7, 0);
      lcd.print((millis()/1000) % 60);
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Kalibracija");
      break;
    case 11:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Izberi T1:    C");
      lcd.setCursor(10, 0);
      lcd.print(T1);
      lcd.setCursor(5, 1);
      lcd.print("OK");
      break;
    case 12:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Izberi T2:    C");
      lcd.setCursor(10, 0);
      lcd.print(T2);
      lcd.setCursor(5, 1);
      lcd.print("OK");
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Izhod");
      break;
    default:
      menu = 0;
  }
}

void write_EEPROM(char addr, double data)  //omejeno na 24 bitov
{
  data *= 1000;
  EEPROM.write(addr * 3, (long)data >> 16);     // high
  EEPROM.write(addr * 3 +1, ((long)data >>8) & 0xff);// middle
  EEPROM.write(addr * 3 +2, (long)data & 0xff);// low
}

double read_EEPROM(byte addr)  //omejeno na 24 bitov
{
  long data = 0;
  data = EEPROM.read(addr * 3) << 16; //high
  data |= EEPROM.read(addr *3 + 1) << 8;   //middle
  data |= EEPROM.read(addr *3 + 2);   //low
  return (double)data / 1000.0;
}

int racT(int adc)
{
  return adc * naklon + zac_vrednost;
}

void racCal()
{
  naklon = (T2-T1) / (cal2 - cal1);
  zac_vrednost = T1 - cal1 * naklon;
}

void setToMemory(byte mask)
{
  if (menu == 0) 
  {
    switch(mask)
    {
      case 0x2:
        EEPROM.write(ADDRMEM, curTemp);
        break;
      case 0x4:
        EEPROM.write(ADDRMEM + 1, curTemp);
        break;
      case 0x8: 
        EEPROM.write(ADDRMEM + 2, curTemp);
        break;
    }
    lcd.setCursor(0, 1);
    lcd.print("Dodano v spomin");
    menu = 21;
  }
}       
