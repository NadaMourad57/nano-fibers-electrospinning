#include <AccelStepper.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Wire.h>
#include "avr/wdt.h"

// Define your motor pins (DIR, STEP, and ENABLE)
#define DIR_PIN 2
#define STEP_PIN 3
#define ENABLE_PIN 4
#define HOME_SWITCH 8
#define LIMIT 9
#define HOME 7
#define START 5
#define STOP 6
#define POTENTIOMETER A0
int STEPS_PER_REV = 6400;
bool isEndOfStroke =false ;
bool distanceSet = false;
bool speedSet = false;
bool isHome = false;

AccelStepper stepper(1, STEP_PIN, DIR_PIN);

int speed = 1600;
int distance = 6400;
String inputBuffer = "";
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 32, 34, 36, 38 };
byte colPins[COLS] = { 40, 42, 44, 46 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
//setup buttons 
  pinMode(HOME, INPUT_PULLUP);
  pinMode(STOP, INPUT_PULLUP);
  pinMode(START, INPUT_PULLUP);
//setup switches 

  pinMode(HOME_SWITCH, INPUT_PULLUP);
  pinMode(LIMIT,INPUT_PULLUP);

  //setup stepper motor 
  pinMode(ENABLE_PIN, OUTPUT);
  digitalWrite(ENABLE_PIN, HIGH);
  stepper.setMaxSpeed(1600);
  stepper.setAcceleration(8000);
   stepper.setCurrentPosition(0);


  //setup serial monitor 

  Serial.begin(115200);
  
  //setup Lcd
  lcd.init();
  lcd.backlight();
   

  lcd.setCursor(0, 0);
  lcd.print("Press a Key:");
  lcd.setCursor(0, 1);
  lcd.print("A)SET SPEED");
  lcd.setCursor(0, 2);
  lcd.print("B)SET DISTANCE");
  delay(1000);
}

void loop() {

  //set speed and distance 
  if (!distanceSet || !speedSet) {
    char key = keypad.getKey();
    switch (key) {
      case 'A':
        Serial.println("pressed A");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Speed= ");
        char enter;
        while ((enter = keypad.getKey()) != '*') {
         int speedCM = map(analogRead(POTENTIOMETER), 0, 1023, 20, 200);
          speed=speedCM*16;
          lcd.print("Speed= ");
          lcd.setCursor(7, 0);
          lcd.print(speedCM);
          delay(1000);
          lcd.clear();
         
        }
        lcd.clear();
        lcd.print("Speed is set");
        Serial.println(speed);
        stepper.setSpeed(speed);
        speedSet = true;
        break;

      case 'B':
        Serial.println("pressed B");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Distance= ");
        char num = keypad.getKey();
        inputBuffer = "";
        while (num != '*') {
          if (num >= '0' && num <= '9') {
            inputBuffer += num;
          }
          num = keypad.getKey();
          lcd.setCursor(9, 0);
          lcd.print(inputBuffer);
        }
       int distanceCM = inputBuffer.toInt();
        distance=distanceCM*142.2;
        Serial.println(distanceCM);
        if (distance > 0) {
          distanceSet = true;
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Distance is set");
        break;
    }
  }

//check if home button is pressed 
  if (digitalRead(HOME) == LOW && !isHome && speedSet && distanceSet) {
    stepper.setSpeed(3200);
    Serial.println("entered Home ");

    digitalWrite(ENABLE_PIN, HIGH);
    while (digitalRead(HOME_SWITCH) != LOW&& digitalRead(STOP) != LOW) {
      stepper.runSpeed();
    }
    isHome = true;
    stepper.stop();
    stepper.runToPosition();
    stepper.setCurrentPosition(0);
  }



  if (digitalRead(START) == LOW && distanceSet && speedSet) {
isHome=false;
digitalWrite(ENABLE_PIN,HIGH);
Serial.println("entered start");
int distancetoendofstroke=-6400*3.8;
 stepper.move(distancetoendofstroke);
 stepper.setSpeed(3200);

  while(!isEndOfStroke){
    
    stepper.runSpeedToPosition();

    if(stepper.distanceToGo()==0){
    isEndOfStroke=true;
    Serial.println("centeralized");
    stepper.stop();
    stepper.runToPosition();
    }


    }



    // Move forward
    stepper.move(distance); 
    stepper.setSpeed(speed);

    while(stepper.distanceToGo()!=0){
    stepper.runSpeedToPosition();
    }
    Serial.println(stepper.currentPosition());

  
    // Move backward
    stepper.move(-distance);
    stepper.setSpeed(speed);
   
  while(stepper.distanceToGo()!=0){
    stepper.runSpeedToPosition();
  }
 
  }

  if (digitalRead(STOP) == LOW) {
    Serial.println("Entered Stop ");
    digitalWrite(ENABLE_PIN, LOW);
   
  }

  //restart 

  if (char key = keypad.getKey() == 'C') {
    Serial.println("pressed C");
    wdt_enable(WDTO_15MS);
    while (1) {}
  }
  //reached end 
  if (digitalRead(LIMIT) == LOW) {
    digitalWrite(ENABLE_PIN, LOW);
  }
}
