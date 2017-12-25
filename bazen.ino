#include "Wire.h"
#include <OneWire.h>
OneWire oneWire(11); // pin čidel

// ----------- Display ------------
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F,16,2);

// -------- Nastavení čidel -------------
#include <DallasTemperature.h>
DallasTemperature sensors(&oneWire);
DeviceAddress BAZEN = { 0x28, 0xFF, 0x99, 0x70, 0x01, 0x17, 0x03, 0x6F };
DeviceAddress PANEL = { 0x28, 0xFF, 0xF1, 0x88, 0x01, 0x17, 0x03, 0x2D };

// ------- Nastavení vstupů a výstupů--------
int releFiltr = A0; // hlavní čerpadlo
int releChlor = A1; // čerpadlo chloru
int releVentil = A2; // ventil ohřevu
int ledOn = 2; // Zelená (dvoubarevná LED)
int ledOff = 10; // červená (dvoubarevná LED)
int ledFiltr = 9; // zelená LED
int ledChlor = 8; // žlutá LED
int ledVentil = 7; // modrá LED

int buttonOn = 5; // zapnutí automatu
int buttonOff = 6; // vypnutí všeho
int buttonFiltr = 4; // zapnutí filtrace
int buttonChlor = 3; // Dávka chloru

// nastavení proměnných pro teplotu a funkce
float Tbazen;
float Tpanel;
boolean filtraceAuto, cisteni, chlor;

// nastavení časování
long previousMillis = 0;
long previousMillisTemp = 0;
long previousMillisTempP = 0;
long interval = 1000; // čas snímání teploty (ms)
long previousButton = 0;
long intervalButton = 50; // čas omezení prokliku tlačítka

// ------------ Nastavení hodin ------------
#define DS3231_I2C_ADDRESS 0x68
byte decToBcd(byte val){
  return( (val/10*16) + (val%10) );
}
byte bcdToDec(byte val){
  return( (val/16*10) + (val%16) );
}
// ------------ Nastavení hodin konec------------

void setup(){
  Wire.begin();
  Serial.begin(9600);

  // -------- nastavení pinů ------------
  pinMode(releFiltr, OUTPUT); 
  pinMode(releChlor, OUTPUT); 
  pinMode(releVentil, OUTPUT);
  pinMode(ledOn, OUTPUT);
  pinMode(ledOff, OUTPUT);
  pinMode(ledFiltr, OUTPUT); 
  pinMode(ledChlor, OUTPUT); 
  pinMode(ledVentil, OUTPUT);

  pinMode(buttonOn, INPUT_PULLUP); 
  pinMode(buttonOff, INPUT_PULLUP); 
  pinMode(buttonFiltr, INPUT_PULLUP); 
  pinMode(buttonChlor, INPUT_PULLUP);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("BAZENBOT 2018");
  lcd.setCursor(0,1);
  lcd.print("(R) Dusan Vala");
  delay(2000);
  lcd.setCursor(0,1);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("Vypnuto");
  digitalWrite(ledOff, HIGH);
   
  // ---------- POUZE PRO NASTAVENÍ ČASU! ---------------------
  // sekundy, minuty, hodiny, den v týdnu (1- neděle), datum, měsíc, rok
  //setDS3231time(0,8,19,7,16,12,17);
  // ---------- POUZE PRO NASTAVENÍ ČASU! KONEC ---------------

  
//nastavení rozlišení snímání teplotních čidel (9 - 0,5°C 10 - 0,25°C..)
sensors.begin();
sensors.setResolution(9);

}
// ---------- POUZE PRO NASTAVENÍ ČASU! ---------------------
/*void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
dayOfMonth, byte month, byte year){
  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}*/

void readDS3231time(byte *second, byte *minute, byte *hour){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
}
  // ------- Hodiny ---------
void displayTime(){
  byte second, minute, hour;
  readDS3231time(&second, &minute, &hour); 
  lcd.setCursor(0,0);
  lcd.print(hour, DEC);
  lcd.print(":");
    if (minute<10){
      lcd.print("0");
    }
  lcd.print(minute, DEC);
  lcd.print(":");
    if (second<10){
      lcd.print("0");
    }
  lcd.print(second, DEC);  
}

//-------------------- LOOP --------------
void loop(){
  // ------------- ZAPNUTÍ AUTOMATIKY --------
  if (digitalRead(buttonOn) == LOW) {
      unsigned long currentMillis = millis();
        if(currentMillis - previousButton > intervalButton) {
              previousButton = currentMillis; 
              sensors.requestTemperatures();
              off();
              digitalWrite(ledOn, HIGH);
              digitalWrite(ledOff, LOW);
              lcd.setCursor(0,1);
              lcd.print("                ");
              lcd.setCursor(0,1);
              lcd.print("Automat");
              filtraceAuto = true;
        }
  }
  
  // ----------- POUZE FILTRACE / ČÍŠTĚNÍ  ------------
  if (digitalRead(buttonFiltr) == LOW) {
      unsigned long currentMillis = millis(); 
        if(currentMillis - previousButton > intervalButton) {
              previousButton = currentMillis; 
              sensors.requestTemperatures();    
              off();
              digitalWrite(ledOn, HIGH);
              digitalWrite(ledOff, LOW);
              lcd.setCursor(0,1);
              lcd.print("                ");
              lcd.setCursor(0,1);
              lcd.print("Cisteni/Filtrace");
              cisteni = true;
        }
   }
  
   // -------- DÁVKOVÁNÍ CHLORU --------
   if (digitalRead(buttonChlor) == LOW) {
       unsigned long currentMillis = millis();
         if(currentMillis - previousButton > intervalButton) {
               previousButton = currentMillis; 
               sensors.requestTemperatures();     
               off();
               digitalWrite(ledOn, HIGH);
               digitalWrite(ledOff, LOW);
               digitalWrite(releChlor, HIGH);
               digitalWrite(ledChlor, HIGH);
               lcd.setCursor(0,1);
               lcd.print("                ");
               lcd.setCursor(0,1);
               lcd.print("Davka chloru");
               chlor = true;
        }
   }

// ------------- VYPNUTO -----------
    if (digitalRead(buttonOff) == LOW)  {
        unsigned long currentMillis = millis();  
          if(currentMillis - previousButton > intervalButton) {
                previousButton = currentMillis; 
                sensors.requestTemperatures();
                off();
          }
    }
  
  filtraceChlor(); // načtení funkce denního programu
  filtrace(); // funkce pro filtraci nebo čištění
  displayTime(); // zobrazení hodin
  bazenTemp(); // zobrazení teploty bazénu
    if(filtraceAuto) { // pokud je aktivní automatický režim,
        panelTemp(); // je zobrazena teplota soláního panelu
    }
}
//-------------------- LOOP END ----------------

// ---------- VYPNUTÍ VŠECH FUNKCÍ --------------
void off(){
      digitalWrite(releFiltr, LOW);
      digitalWrite(releChlor, LOW);
      digitalWrite(releVentil, LOW);
      digitalWrite(ledFiltr, LOW);
      digitalWrite(ledChlor, LOW);
      digitalWrite(ledVentil, LOW);
      digitalWrite(ledOn, LOW);
      digitalWrite(ledOff, HIGH);
      cisteni = false;
      filtraceAuto = false;
      chlor = false;
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Vypnuto");
}

/* zapnutí pouze filtrace nebo dávky chloru
 *  Následující funkce řeší samotné filtrování (čištění) bazénu
 *  a dávkování chloru. Pokud se zvolí filtrace, tak vypne automatický režim a
 *  spustí se pouze filtrační čerpadlo. Pokud se zvolí dávka chloru, tak se navíc 
 *  na nastavený čas zapne i čerpadlo chloru. Funkci "chlorování" je nutné používat
 *  v rozmezí 0-5 sekund (v jakékoliv minutě). Pokud se zvolí později tak
 *  to nevadí, jen bude dávka o to menší. Chlor čerpadlo vypne vždy v nastavenou sekundu.
 *  Po cca 15 sekundách po vypnutí chlor čerpadla, přejde systém na automat.
 */
void filtrace(){
  if(cisteni || (chlor)){// pokud je aktivována filtrace, nebo extra dávka chloru
      digitalWrite(releFiltr, HIGH); //zapni čerpadlo
      digitalWrite(ledFiltr, HIGH); //zapni led
      digitalWrite(releVentil, LOW); //a vypni ventil pokud by byl otevřený
      if(chlor) { // pokud je aktuvována extra dávka chloru
         byte sec, minuty, hod;
         readDS3231time(&sec, &minuty, &hod);
         if((sec >= 5 & (sec < 45))) { // nastavení 40s dávky chloru (nutno zapnout mezi 0-5 sec)
        digitalWrite(releChlor, HIGH);
        digitalWrite(ledChlor, HIGH);
         } else {
              if(sec == 59) { // v 59 sekundě přejde systém zpět do automatického režimu
                lcd.setCursor(0,1);
                lcd.print("                ");
                lcd.setCursor(0,1);
                lcd.print("Automat");
                digitalWrite(releFiltr, LOW);
                digitalWrite(ledFiltr, LOW);
                filtraceAuto = true;
                chlor = false;
              }
              digitalWrite(releChlor, LOW);
              digitalWrite(ledChlor, LOW);
         }
      }
  } else {
    if(!filtraceAuto){
    digitalWrite(releFiltr, LOW);
    digitalWrite(ledFiltr, LOW);
    }
  }
}

/* ------------ AUTOMATICKÁ FILTRACE ----------
 *  Ráno a večer zapne filtraci a nadávkuje chlor
 *  Mimo časovou filtraci zapíná denní program ohřevu bazénu
 */
void filtraceChlor(){
 byte sec, minuty, hod;
 readDS3231time(&sec, &minuty, &hod);
  if(filtraceAuto & (!cisteni & (!chlor))) {

  // --------------- VEČER / RÁNO------------
    if((hod >= 21 & (hod < 22) || (hod >= 5 & (hod < 6)))) { // filtrace pojede mezi 21-22hod večer a mezi 5-6hod ráno
      digitalWrite(releFiltr, HIGH); //zapni čerpadlo 
      digitalWrite(ledFiltr, HIGH); //zapni LED
      digitalWrite(releVentil, LOW);
      digitalWrite(ledVentil, LOW);   
      if((minuty == 0) & (sec >=5) & (sec <=45)) { // u každé filtrace bude přidán chlor po dobu 40s
          digitalWrite(releChlor, HIGH);   
          digitalWrite(ledChlor, HIGH);    
        } else {
            digitalWrite(releChlor, LOW);
            digitalWrite(ledChlor, LOW);
            }
      
      } else {
        teplota(); // mimo daný čas zapne denní režim ohřevu bazénu
    }
  }
}

// ------------- DENNÍ PROGRAM OHŘEVU -------------
void teplota(){ 
  unsigned long currentMillis = millis();
  
  if(currentMillis - previousMillis > interval) {
      previousMillis = currentMillis; 
      sensors.requestTemperatures();
      Tbazen = sensors.getTempC(BAZEN); //teplota bazenu
      Tpanel = sensors.getTempC(PANEL); //teplota panelu
      float hystereze = 3; //rozdíl teplot pro zapnutí a vypnutí

      if(Tbazen == -127 || (Tpanel == -127)){ // pokud není čidlo připojeno hlásí teplotu -127
              lcd.setCursor(9, 0);
              lcd.print("Error01");
              } else {
                   if(Tbazen <= 24) {//pokud je teplota bazénu menší
                      if((Tpanel - 28) > hystereze) { //a teplota panelu větší
                            digitalWrite(releFiltr, HIGH); // zapni čerpadlo
                            digitalWrite(ledFiltr, HIGH); //zapni led 
                            digitalWrite(releVentil, HIGH); // a uzavři ventil
                            digitalWrite(ledVentil, HIGH); // zapni led
                      } else {
                        if((28 - Tpanel) > hystereze) { // pokud teplota panelu klesne
                          digitalWrite(releFiltr, LOW); // vypni čerpadlo
                          digitalWrite(ledFiltr, LOW); // vypni led
                        }
                      }
              
                  } else { // pokud je bazén na požadované teplotě
                    digitalWrite(releFiltr, LOW); // vypni čerpadlo
                    digitalWrite(ledFiltr, LOW); // vypni led
                    digitalWrite(releVentil, LOW); // otevři ventil
                    digitalWrite(ledVentil, LOW); // zhasni led
                  }
                }
  }
}

// ---------- TEPLOTA BAZÉNU ----------- 
void bazenTemp(){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillisTemp > interval) {
          previousMillisTemp = currentMillis; 
          //načtení teploty z čidla
          sensors.requestTemperatures();
          Tbazen = sensors.getTempC(BAZEN);
          if(Tbazen == -127){
              lcd.setCursor(9, 0);
              lcd.print("Error");
          } else {
               lcd.setCursor(9, 0);
               lcd.print(Tbazen);
               lcd.print((char)223);
               lcd.print("C");
            }
  }
}

// ---------- TEPLOTA SOLÁRNÍHO PANELU ----------- 
void panelTemp(){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillisTempP > interval) {
          previousMillisTempP = currentMillis; 
          //načtení teploty z čidla
          sensors.requestTemperatures();
          Tpanel = sensors.getTempC(PANEL);
          if(Tpanel == -127){
              lcd.setCursor(9, 1);
              lcd.print("Error");
          } else {
               lcd.setCursor(9, 1);
               lcd.print(Tpanel);
               lcd.print((char)223);
               lcd.print("C");
            }
  }
}
