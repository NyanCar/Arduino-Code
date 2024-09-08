#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <Servo.h>
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo servo;

const int HX711_dout = 13; //mcu > HX711 dout pin
const int HX711_sck = 12; //mcu > HX711 sck pin
HX711_ADC LoadCell(HX711_dout, HX711_sck);
const int calVal_eepromAdress = 0;

unsigned long t = 0;
int upButton = 3;
int downButton = 4;
int selectButton = 2;
int menu = 1;
int time = 1;
int weight = 0;
int minutes = 0;
int seconds = 0;
int totalSeconds = 0;
unsigned long previousMillis = 0; 
const long interval = 1000;

void setup() 
{
  Serial.begin(57600); delay(10);
  Serial.println();
  Serial.println("Starting...");
  lcd.init();
  lcd.clear();
	lcd.backlight();
  lcd.setCursor(0,0);
	lcd.print("Welcome!");
  lcd.setCursor(0,1);
  lcd.print("Loading...");
  delay(1000);
  servo.attach(11);
  servo.write(0);
  LoadCell.begin();
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);
  pinMode(selectButton, INPUT_PULLUP);
  updateMenu();
  float calibrationValue;
  calibrationValue = 696.0;
#if defined(ESP8266)|| defined(ESP32)
#endif
  unsigned long stabilizingtime = 2000;
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }
}


bool menuStatus = true;
bool timeStatus = false;
bool weightStatus = false;
bool startStatus = false;
void loop() {

  if (!digitalRead(selectButton)){
    if (menuStatus == true and menu == 1){
      menuStatus = false;
      timeStatus = true;
      
      //executeAction()
      timeMenu();
      delay(100);
      while (!digitalRead(selectButton));
    }
    else if (menuStatus == true and menu == 2){
      menuStatus = false;
      weightStatus = true;
      
      //executeAction()
      weightMenu();
      delay(100);
      while (!digitalRead(selectButton));
    }
    else if (menuStatus == true and menu == 3){
      menuStatus = false;
      startStatus = true;
      
      //executeAction()
      counterMenu();
      while (!digitalRead(selectButton));
    }
    else if (timeStatus == true){
      timeStatus = false;
      menuStatus = true;

      updateMenu();
      delay(100);
      while (!digitalRead(selectButton));
    }
    else if (weightStatus == true){
      weightStatus = false;
      menuStatus = true;

      updateMenu();
      delay(100);
      while (!digitalRead(selectButton));
    }
    else if (startStatus == true){
      startStatus = false;
      menuStatus = true;

      updateMenu();
      delay(100);
      while (!digitalRead(selectButton));
    }
  }
  if (!digitalRead(downButton)){
    if (menuStatus == true){
      menu++;
      updateMenu();
      delay(100);
      while (!digitalRead(downButton));
    }
    if (timeStatus == true){
      time--;
      timeMenu();
      delay(100);
      while(!digitalRead(downButton));
    }
    if (weightStatus == true){
      weight -= 5;
      weightMenu();
      delay(100);
      while(!digitalRead(downButton));
    }
    if (startStatus == true){
      while(!digitalRead(downButton));
    }    
  }
  if (!digitalRead(upButton)){
    if (menuStatus == true){
      menu--;
      updateMenu();
      delay(100);
      while(!digitalRead(upButton));
    }
    if (timeStatus == true){
      time++;
      timeMenu();
      delay(100);
      while(!digitalRead(upButton));
    }
    if (weightStatus == true){
      weight += 5;
      weightMenu();
      delay(100);
      while(!digitalRead(downButton));
    }
    if (startStatus == true){
      while(!digitalRead(downButton));
    }
  }
  
}

void updateMenu() {
  switch (menu) {
    case 0:
      menu = 1;
      break;
    case 1:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(">Set Time");
      lcd.setCursor(0,1);
      lcd.print(" Set Weight");
      break;
    case 2:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(" Set Time");
      lcd.setCursor(0,1);
      lcd.print(">Set Weight");
      break;
    case 3:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(">Start");
      break;
    case 4:
      menu = 3;
      break;
  }
}

//TIME MENU
void timeMenu() {
  switch (time) {
    case 0:
      time = 1;
      break;
    case 1:
      lcd.clear();
      lcd.print(">1:00");
      break;
    case 2:
      lcd.clear();
      lcd.print(">6:00");
      break;
    case 3:
      lcd.clear();
      lcd.print(">12:00");
      break;
    case 4:
      lcd.clear();
      lcd.print(">18:00");
      break;
    case 5:
      time = 4;
      break;
  }
}

//WEIGHT MENU
void weightMenu() {
  lcd.clear();
  //weight_str = sprintf(weight);
  lcd.print(weight);
  lcd.print(" g");
}

//countingMenu

void counterMenu() {
  lcd.clear();
  lcd.print("Loading...");
  if (time == 1){
    minutes = 0;
    seconds = 59;
    totalSeconds = minutes*60+seconds;
    delay(500);
    while (totalSeconds >= 0) {
    const int serialPrintInterval = 500;
    static boolean newDataReady = 0;
    if (LoadCell.update()) newDataReady = true;
    if (newDataReady) {
      if (millis() > t + serialPrintInterval) {
        float i = LoadCell.getData();
        Serial.print("Load_cell output val: ");
        Serial.println(i);
        newDataReady = 0;
        t = millis();
      }
    }
      if (!digitalRead(selectButton)){
        updateMenu();
        menuStatus = true;
        break;
        while(!digitalRead(downButton));
      }
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        lcd.clear();
        lcd.setCursor(0,0);
        if (minutes<10){
          lcd.print("0");
        }
        lcd.print(minutes);
        lcd.print(":");
        if(seconds<10){
          lcd.print("0");
        }
        lcd.print(seconds);
        seconds--;
        if (seconds == 0){
          minutes--;
          seconds = 60;
        }
      }
      totalSeconds = minutes*60+seconds;
      if (totalSeconds == 0){
        float i = LoadCell.getData();
        if (i < weight){
          lcd.clear();
          lcd.print("Dispensing...");
          servo.write(180);
          while (i<weight) {
          const int serialPrintInterval = 500;
          static boolean newDataReady = 0;
          if (LoadCell.update()) newDataReady = true;
          if (newDataReady) {
            if (millis() > t + serialPrintInterval) {
              float i = LoadCell.getData();
              Serial.print("Load_cell output val: ");
              Serial.println(i);
              newDataReady = 0;
              t = millis();
              }
            }
          i = LoadCell.getData();
          }
          servo.write(0);
          minutes = 0;
          seconds = 59;
        }
        else {
          lcd.clear();
          lcd.print("There's still");
          lcd.setCursor(0,1);
          lcd.print("food!");
          lcd.setCursor(0,0);
          delay(1000);
          minutes = 0;
          seconds = 59;
        }
      }
    }
  }

  else if (time == 2){
    minutes = 5;
    seconds = 59;
    totalSeconds = minutes*60+seconds;
    delay(500);
    while (totalSeconds >= 0) {
    const int serialPrintInterval = 500;
    static boolean newDataReady = 0;
    if (LoadCell.update()) newDataReady = true;
    if (newDataReady) {
      if (millis() > t + serialPrintInterval) {
        float i = LoadCell.getData();
        Serial.print("Load_cell output val: ");
        Serial.println(i);
        newDataReady = 0;
        t = millis();
      }
    }
      if (!digitalRead(selectButton)){
        updateMenu();
        menuStatus = true;
        break;
        while(!digitalRead(downButton));
      }
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        lcd.clear();
        lcd.setCursor(0,0);
        if (minutes<10){
          lcd.print("0");
        }
        lcd.print(minutes);
        lcd.print(":");
        if(seconds<10){
          lcd.print("0");
        }
        lcd.print(seconds);
        seconds--;
        if (seconds == 0){
          minutes--;
          seconds = 60;
        }
      }
      totalSeconds = minutes*60+seconds;
      if (totalSeconds == 0){
        float i = LoadCell.getData();
        if (i < weight){
          lcd.clear();
          lcd.print("Dispensing...");
          servo.write(180);
          while (i<weight) {
          const int serialPrintInterval = 500;
          static boolean newDataReady = 0;
          if (LoadCell.update()) newDataReady = true;
          if (newDataReady) {
            if (millis() > t + serialPrintInterval) {
              float i = LoadCell.getData();
              Serial.print("Load_cell output val: ");
              Serial.println(i);
              newDataReady = 0;
              t = millis();
              }
            }
          i = LoadCell.getData();
          }
          servo.write(0);
          minutes = 5;
          seconds = 59;
        }
        else {
          lcd.clear();
          lcd.print("There's still");
          lcd.setCursor(0,1);
          lcd.print("food!");
          lcd.setCursor(0,0);
          delay(1000);
          minutes = 5;
          seconds = 59;
        }
      }
    }
  }

  else if (time == 3){
    minutes = 11;
    seconds = 59;
    totalSeconds = minutes*60+seconds;
    delay(500);
    while (totalSeconds >= 0) {
    const int serialPrintInterval = 500;
    static boolean newDataReady = 0;
    if (LoadCell.update()) newDataReady = true;
    if (newDataReady) {
      if (millis() > t + serialPrintInterval) {
        float i = LoadCell.getData();
        Serial.print("Load_cell output val: ");
        Serial.println(i);
        newDataReady = 0;
        t = millis();
      }
    }
      if (!digitalRead(selectButton)){
        updateMenu();
        menuStatus = true;
        break;
        while(!digitalRead(downButton));
      }
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        lcd.clear();
        lcd.setCursor(0,0);
        if (minutes<10){
          lcd.print("0");
        }
        lcd.print(minutes);
        lcd.print(":");
        if(seconds<10){
          lcd.print("0");
        }
        lcd.print(seconds);
        seconds--;
        if (seconds == 0){
          minutes--;
          seconds = 60;
        }
      }
      totalSeconds = minutes*60+seconds;
      if (totalSeconds == 0){
        float i = LoadCell.getData();
        if (i < weight){
          lcd.clear();
          lcd.print("Dispensing...");
          servo.write(180);
          while (i<weight) {
          const int serialPrintInterval = 500;
          static boolean newDataReady = 0;
          if (LoadCell.update()) newDataReady = true;
          if (newDataReady) {
            if (millis() > t + serialPrintInterval) {
              float i = LoadCell.getData();
              Serial.print("Load_cell output val: ");
              Serial.println(i);
              newDataReady = 0;
              t = millis();
              }
            }
          i = LoadCell.getData();
          }
          servo.write(0);
          minutes = 11;
          seconds = 59;
        }
        else {
          lcd.clear();
          lcd.print("There's still");
          lcd.setCursor(0,1);
          lcd.print("food!");
          lcd.setCursor(0,0);
          delay(1000);
          minutes = 11;
          seconds = 59;
        }
      }
    }
  }
  else if (time == 4){
    minutes = 17;
    seconds = 59;
    totalSeconds = minutes*60+seconds;
    delay(500);
    while (totalSeconds >= 0) {
    const int serialPrintInterval = 500;
    static boolean newDataReady = 0;
    if (LoadCell.update()) newDataReady = true;
    if (newDataReady) {
      if (millis() > t + serialPrintInterval) {
        float i = LoadCell.getData();
        Serial.print("Load_cell output val: ");
        Serial.println(i);
        newDataReady = 0;
        t = millis();
      }
    }
      if (!digitalRead(selectButton)){
        updateMenu();
        menuStatus = true;
        break;
        while(!digitalRead(downButton));
      }
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        lcd.clear();
        lcd.setCursor(0,0);
        if (minutes<10){
          lcd.print("0");
        }
        lcd.print(minutes);
        lcd.print(":");
        if(seconds<10){
          lcd.print("0");
        }
        lcd.print(seconds);
        seconds--;
        if (seconds == 0){
          minutes--;
          seconds = 60;
        }
      }
      totalSeconds = minutes*60+seconds;
      if (totalSeconds == 0){
        float i = LoadCell.getData();
        if (i < weight){
          lcd.clear();
          lcd.print("Dispensing...");
          servo.write(180);
          while (i<weight) {
          const int serialPrintInterval = 500;
          static boolean newDataReady = 0;
          if (LoadCell.update()) newDataReady = true;
          if (newDataReady) {
            if (millis() > t + serialPrintInterval) {
              float i = LoadCell.getData();
              Serial.print("Load_cell output val: ");
              Serial.println(i);
              newDataReady = 0;
              t = millis();
              }
            }
          i = LoadCell.getData();
          }
          servo.write(0);
          minutes = 17;
          seconds = 59;
        }
        else {
          lcd.clear();
          lcd.print("There's still");
          lcd.setCursor(0,1);
          lcd.print("food!");
          lcd.setCursor(0,0);
          delay(1000);
          minutes = 17;
          seconds = 59;
        }
      }
    }
  }
}