#include "Wire.h"
#include <OneWire.h>
OneWire oneWire(2); // pin čidel

// ----------- Display ------------
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F,20,4);

// -------- Nastavení čidel -------------
#include <DallasTemperature.h>
DallasTemperature sensors(&oneWire);
DeviceAddress PANEL = { 0x28, 0xFF, 0x99, 0x70, 0x01, 0x17, 0x03, 0x6F }; // adresa čidla pro solární panel
DeviceAddress BAZEN = { 0x28, 0xFF, 0xF1, 0x88, 0x01, 0x17, 0x03, 0x2D }; // adresa čidla pro bazén
/* Zjištění adres pro čidla:
 * https://codebender.cc/sketch:75043#DS18B20%20Address%20Finder.ino
 */

// ------- Nastavení vstupů a výstupů--------
int releFiltr = A2; // hlavní čerpadlo
int releChlor = A0; // čerpadlo chloru
int releVentil = A1; // ventil ohřevu

int buttonOn = 4; // zapnutí automatu
int buttonOff = 3; // vypnutí všeho
int buttonFiltr = 6; // zapnutí filtrace
int buttonChlor = 5; // Dávka chloru

// nastavení proměnných pro teplotu a funkce
float Tbazen, Tpanel;
boolean filtraceAuto, cisteni, chlor;
int vodaTemp = 25;

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

  pinMode(buttonOn, INPUT_PULLUP); 
  pinMode(buttonOff, INPUT_PULLUP); 
  pinMode(buttonFiltr, INPUT_PULLUP); 
  pinMode(buttonChlor, INPUT_PULLUP);

  digitalWrite(releFiltr, HIGH);
  digitalWrite(releChlor, HIGH);
  digitalWrite(releVentil, HIGH);
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3,1);
  lcd.print("BAZENBOT 2018");
  lcd.setCursor(3,2);
  lcd.print("(R) Dusan Vala");
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Vypnuto");
  lcd.setCursor(0,0);
  lcd.print("                ");
   
  // ---------- POUZE PRO NASTAVENÍ ČASU! ---------------------
  // sekundy, minuty, hodiny, den v týdnu (1- neděle), datum, měsíc, rok
  //setDS3231time(00,00,9,1,8,4,18);
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

void readDS3231time(byte *second, byte *minute, byte *hour, byte *dayOfWeek, byte *dayOfMonth, byte *month, byte *year){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}
  // ------- Hodiny ---------
void displayTime(){
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year); 
  lcd.setCursor(0,0);
  if (hour<10){
      lcd.print(" ");
    }
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
  lcd.print(" ");
  lcd.print(dayOfMonth, DEC);
  lcd.print("/");
  lcd.print(month, DEC);
  lcd.print(" 20");
  lcd.print(year, DEC); 
    
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
              lcd.setCursor(0,1);
              lcd.print("                    ");
              lcd.setCursor(0,1);
              lcd.print("Automaticky provoz");
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
              lcd.setCursor(0,1);
              lcd.print("                    ");
              lcd.setCursor(0,1);
              lcd.print("Cisteni / Filtrace");
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
               digitalWrite(releChlor, HIGH);
               lcd.setCursor(0,1);
               lcd.print("                    ");
               lcd.setCursor(14,2);
               lcd.print("                    ");
               lcd.setCursor(14,3);
               lcd.print("                    ");
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
  panelTemp(); // je zobrazena teplota soláního panelu
}
//-------------------- LOOP END ----------------

// ---------- VYPNUTÍ VŠECH FUNKCÍ --------------
void off(){
      digitalWrite(releFiltr, HIGH);
      digitalWrite(releChlor, HIGH);
      digitalWrite(releVentil, HIGH);
      cisteni = false;
      filtraceAuto = false;
      chlor = false;
      lcd.setCursor(0,1);
      lcd.print("                    ");
      lcd.setCursor(0,1);
      lcd.print("Vypnuto");
      lcd.setCursor(0,2);
      lcd.print("                    ");
      lcd.setCursor(0,3);
      lcd.print("                    ");
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
      digitalWrite(releFiltr, LOW); //zapni čerpadlo
      digitalWrite(releVentil, HIGH); //a vypni ventil pokud by byl otevřený
      if(chlor) { // pokud je aktuvována extra dávka chloru
         byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
         readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
         if((second >= 5 & (second < 45))) { // nastavení 40s dávky chloru (nutno zapnout mezi 0-5 sec)
        digitalWrite(releChlor, LOW);
         } else {
              if(second == 59) { // v 59 sekundě přejde systém zpět do automatického režimu
                lcd.setCursor(0,1);
                lcd.print("                ");
                lcd.setCursor(0,1);
                lcd.print("Automaticky provoz");
                digitalWrite(releFiltr, HIGH);
                filtraceAuto = true;
                chlor = false;
              }
              digitalWrite(releChlor, HIGH);
         }
      }
  } else {
    if(!filtraceAuto){
    digitalWrite(releFiltr, HIGH);
    }
  }
}

/* ------------ AUTOMATICKÁ FILTRACE ----------
 *  Ráno a večer zapne filtraci a nadávkuje chlor
 *  Mimo časovou filtraci zapíná denní program ohřevu bazénu
 */
void filtraceChlor(){
  byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
  readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
  if(filtraceAuto & (!cisteni & (!chlor))) {

  // --------------- VEČER / RÁNO------------
    if((hour >= 21 & (hour < 22) || (hour >= 5 & (hour< 6)))) { // filtrace pojede mezi 21-22hod večer a mezi 5-6hod ráno
      digitalWrite(releFiltr, LOW); //zapni čerpadlo 
      digitalWrite(releVentil, HIGH);
      if((minute == 1) & (second >=5) & (second <=45)) { // u každé filtrace bude přidán chlor po dobu 40s
          digitalWrite(releChlor, LOW);   
        } else {
            digitalWrite(releChlor, HIGH);
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
 
      if(Tbazen == -127 || (Tpanel == -127)){ // pokud není čidlo připojeno hlásí teplotu -127
              lcd.setCursor(14, 2);
              lcd.print("Ohrev");
              lcd.setCursor(14, 3);
              lcd.print("OFF");
              } else {
                   if(Tbazen <= 25) {//pokud je teplota bazénu menší
                      if((Tpanel - Tbazen) >= 10 ) { //a teplota panelu větší
                            digitalWrite(releFiltr, LOW); // zapni čerpadlo
                            digitalWrite(releVentil, LOW); // a uzavři ventil
                            lcd.setCursor(14, 2);
                            lcd.print("       ");
                            lcd.setCursor(14, 2);
                            lcd.print("Ohrev");
                            lcd.setCursor(14, 3);
                            lcd.print("       ");
                            lcd.setCursor(14, 3);
                            lcd.print("ON");
                      } else {
                        if((Tpanel - Tbazen) >= 3) { // pokud teplota panelu klesne na teplotu bazénu
                          digitalWrite(releFiltr, HIGH); // vypni čerpadlo
                          lcd.setCursor(14, 2);
                          lcd.print("      ");
                          lcd.setCursor(14, 2);
                          lcd.print("Ohrev");
                          lcd.setCursor(14, 3);
                          lcd.print("       ");
                          lcd.setCursor(14, 3);
                          lcd.print("OFF");
                        }
                      }
                  } else { 
                    // pokud je bazén na požadované teplotě
                    bazenOk();
                    }
                    
         // Chlazení vody
        /* if((Tbazen >= 27) && (Tpanel < (vodaTemp-3))) { //pokud je teplota bazénu větší, zapne chlazení
                          digitalWrite(releFiltr, LOW); // zapni čerpadlo
                          digitalWrite(releVentil, LOW); // a uzavři ventil
                          lcd.setCursor(12, 2);
                          lcd.print("Chlazeni");
                          lcd.setCursor(14, 3);
                          lcd.print("vody");
                      } else {
                         // pokud je bazén na požadované teplotě
                          bazenOk();
                        }*/
                 } // konec else špatných čidel
  } // konec časování čtení čidel                     
}

void bazenOk(){
  digitalWrite(releFiltr, HIGH); // vypni čerpadlo
  digitalWrite(releVentil, HIGH); // otevři ventil
  lcd.setCursor(14, 2);
  lcd.print("      ");
  lcd.setCursor(14, 2);
  lcd.print("Voda");
  lcd.setCursor(14, 3);
  lcd.print("       ");
  lcd.setCursor(14, 3);
  lcd.print("OK");
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
              lcd.setCursor(0, 2);
              lcd.print("Error00");
          } else {
               lcd.setCursor(0, 2);
               lcd.print("Voda:");
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
              lcd.setCursor(0, 3);
              lcd.print("Error01");
          } else {
               lcd.setCursor(0, 3);
               lcd.print("Solar:");
               lcd.print(Tpanel);
               lcd.print((char)223);
               lcd.print("C");
            }
  }
}
