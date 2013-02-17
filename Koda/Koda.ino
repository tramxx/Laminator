#include <EEPROM.h>
#include <LiquidCrystal.h>

#define ENCA    9
#define ENCB    8
#define ENCBTN  A1
#define VDIODE  A0
#define LED1    A2
#define BTN1    A3
#define BTN2    A4
#define BTN3    A5
#define D4      0 
#define D5      1
#define D6      2
#define D7      3
#define RS      4
#define EN      5
#define GRELEC  6
#define MOTOR   7
#define DESNO   1
#define LEVO    -1
#define ADDRTEMP   0
#define ADDRNAKLON 1
#define ADDRZACVR  2
#define ADDRMEM    12 //(naslov 4 * stirjeBajtiNaRazdelek)

#define REFRESH 100

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

//spremenljivke za menu
unsigned char tempSet = 0;  // nastavljena temperatura 
unsigned int curTemp = 0;   // trenutna temperatura
unsigned long time = 0;
unsigned char menu = 21;    // stanje menuja (0 => izven menuja, 1 => kalibracija, 11 - 12 => podmenuja kalibracije, 2 => izhod iz menuja, 21 => prehodni menu)
unsigned int T1 = 200;      // kalibracijska temperatura reference
unsigned int T2 = 200;
unsigned int cal1;          // stanje senzorja ob referenci
unsigned int cal2;
float naklon;               // naklon aproksimacijske premice
float zac_vrednost;         // začetna vrednost
byte grelecState = 0;       // stanje grelca, ON, OFF

union eeprom{ // for writing calibration data
  float decimal;     // set data
  struct {
    unsigned char N; // First byte, Lowest
    unsigned char L; // Second byte
    unsigned char M; // Third byte
    unsigned char H; // Fourth byte, Highest
  };
};

void setup()
{
  pinMode(ENCA, INPUT_PULLUP); // encoder A has internal pull-up
  pinMode(ENCB, INPUT_PULLUP); // encoder B has internal pull-up
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
  
  tempSet = read_EEPROM(ADDRTEMP);  // prenos zeljene temperature v RAM
  lcd.begin(16, 2);
  //izpisiEkran();    // prehod v menu 0
  izpisiEkran();    // izris menuja 0
  //Serial.begin(9600);
  naklon = read_EEPROM(ADDRNAKLON);        // prenos aproksimacijske premice v RAM
  zac_vrednost = read_EEPROM(ADDRZACVR);
  
}

void loop()
{
  static int readNum  = 0;
  static long time = millis();
  char encoder = encoderChange();  // preveri spremembo encoderja
  char buttons = btnChange();      // preveri spremembo na tipkah
  //curTemp = analogRead(VDIODE);    // zdi se mi, da ta stavek pokvari tempereturo
  
  if(millis() - time > REFRESH || readNum)    // vsako sekundo se 10X izvede ta zanka
  {
    if (readNum == 0) curTemp = 0;
    curTemp = (analogRead(VDIODE) + curTemp);
    readNum++;
    if (readNum >= 10) {    // povpreči stanje na ADC
      curTemp /= 10;
      //curTemp = racT(curTemp);
      curTemp = analogRead(VDIODE);
      readNum = 0;      
      if(curTemp > tempSet) grelecState = 0;
      else if(curTemp < tempSet - 3) grelecState = 1;    // 3 stopinje je temperaturna razlika, kjer ne reagiramo
      digitalWrite(GRELEC, grelecState);      // nastavimo grelec na zeljeno vrednost
      digitalWrite(LED1, grelecState);        // nastavimo LED na zeljeno vrednost
      izpisiEkran();
      time = millis();
    }
  }
  
  if(buttons)        // reagira na spremembo stanja tipk
  {
    switch(menu)     // reakcija na tipke je odvisna od tega, v katerem menuju smo
    {
      case 0:        // izven menuja se s tipko encoderja postavimo v menu, z ostalimi tipkami pa nastavimo temperaturo iz spomina
        switch(buttons)
        {
          case 0x1:
            menu = 1;
            izpisiEkran();
            break;
          case 0x2:
            if(digitalRead(BTN1)) tempSet = EEPROM.read(ADDRMEM); // temperaturo nastavi samo, če je tipka spuščena, drugače čaka, ker bo mogoče moral temperaturo shraniti
            break;
          case 0x4:
            if(digitalRead(BTN2)) tempSet = EEPROM.read(ADDRMEM + 1);
            break;
          case 0x8: 
            if(digitalRead(BTN3)) tempSet = EEPROM.read(ADDRMEM + 2);
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
  
  if(encoder)        // reagira na spremembo na encoderju
  {
    switch(menu)     // reakcija na spremembo encoderja je odvisna od tega, v katerem menuju smo
    {
      case 0: 
        tempSet += encoder;     //spremenimo nastavljeno temperaturo
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
  if(encTemp == stateA) return 0;
  
  char stateB = digitalRead(ENCB);    // popravljeno. stateB se gleda samo, če ugotovimo, da je prišlo do spremembe
  encTemp = stateA;
  if(stateA == stateB) return DESNO;   //desno
  else return LEVO;                    //levo  --> smer vrtenja je že preverjena
}

char btnChange()
{
  static byte btnTemp = 0;
  static long timer = 0;
  byte mask;
  byte btnState = digitalRead(BTN3) << 3 | digitalRead(BTN2) << 2 | digitalRead(BTN1) << 1 | digitalRead(ENCBTN);
  if(btnState == btnTemp && !btnState) {    // preveri, ce je kaksna tipka pritisnjena dalj casa 
    if (timer+2000 < millis()) {            // ce je pritisnjena vec kot 2 sekundi, shrani temperaturo kot prednastavljeno vrednost
      if (menu == 0) setToMemory(btnState); 
    }
    return 0;      
  }
  
  timer = millis();      // resetira timer za gledanje casa
  mask = (btnTemp^btnState)&btnTemp;
  btnTemp = btnState;
  delay(20); // Debounce (samo, ko se spremeni stanje)
  return mask;
}


void izpisiEkran()
{
  static char stariMenu = 10;
  switch(menu)
  {
    case 0:                      // menu 0
      if(stariMenu != menu)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("T=    C  Z:    C");
        lcd.setCursor(0,1);
        lcd.print("cas=  :  ");
      }
      lcd.setCursor(2, 0);
      lcd.print(analogRead(VDIODE));
      lcd.setCursor(11, 0);
      lcd.print(tempSet);
      lcd.setCursor(4, 1);
      lcd.print(millis()/1000/60);
      lcd.setCursor(7, 1);
      lcd.print((millis()/1000) % 60);
      break;
    case 1:                      // menu 1
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Kalibracija");
      break;
    case 11:                      // menu 11
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Izberi T1:    C");
      lcd.setCursor(10, 0);
      lcd.print(T1);
      lcd.setCursor(5, 1);
      lcd.print("OK");
      break;
    case 12:                      // menu 12
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Izberi T2:    C");
      lcd.setCursor(10, 0);
      lcd.print(T2);
      lcd.setCursor(5, 1);
      lcd.print("OK");
      break;
    case 2:                      // menu 2
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Izhod");
      break;
    default:                     // ce je v kakem drugem stanju ( naprimer  menu = 21, kar uporabljamo za izris poljubnega teksta
      menu = 0;
  }
}

void write_EEPROM(byte addr, float value) {    // zapisi float v EEPROM
  eeprom str;
  str.decimal = value;
  EEPROM.write(addr*4, str.N);
  EEPROM.write(addr*4+1, str.L);
  EEPROM.write(addr*4+2, str.M);
  EEPROM.write(addr*4+3, str.H);
}

float read_EEPROM(byte addr) {                  // beri float iz EEPROM
  union eeprom data;
  data.N = EEPROM.read(addr*4);
  data.L = EEPROM.read(addr*4+1);
  data.M = EEPROM.read(addr*4+2);
  data.H = EEPROM.read(addr*4+3);
  return data.decimal;
}

int racT(int adc)
{
  return adc * naklon + zac_vrednost;
}

void racCal()
{
  naklon = (T2-T1) / (cal2 - cal1);
  zac_vrednost = T1 - cal1 * naklon;
  write_EEPROM(ADDRNAKLON, naklon);
  write_EEPROM(ADDRZACVR, zac_vrednost);
}

void setToMemory(byte mask)    // Shrani prednastavljeno temperaturo v EEPROM
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