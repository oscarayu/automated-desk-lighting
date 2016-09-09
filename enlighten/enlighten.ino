/*
 * Notes:
 * Relay shall be wired in parallel with physical switch for OR-logic.
 * i.e. Automation system can control the lighting OR the user can force 
 * the light on. To achieve this, we write LOW to the NO relay. 
 * The alternative is a safety functionality via series connection for AND-logic.
 * i.e. When the Automation system fails (blows up/ sets on fire), the light can 
 * still be switch on or off regardless. We then write HIGH to the NC relay.
 */
// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 rtc;

//Set delay in minutes
int DelayMins = 10;

//Declare variables for Relay
const byte relayPin = 5;

//Declare variables for external interrupt (PIR sensor)
const byte beeperPin = 8;
volatile byte beeperState = LOW;
const byte ledPin = 13; //PIR Status indicator
volatile byte ledState = LOW;
const byte interruptPin = 2; //PIR Sensor Pin
//Note: Arduino Uno interrupt pin is on pin 2
volatile byte state = LOW; //PIR STATE

//Declare loop counter
int i = 0;

//Declare variables for debouncing
const long debounceTime = 300; //millis
volatile unsigned long lastMicros;

DateTime stopTime;
DateTime now;
volatile long int TimeToStop = 0;
volatile long int TimeNow = 0;

void setup () {

  Serial.begin(9600);

  delay(2000); // wait for console opening

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    
    Serial.println("RTC lost power, lets set the time!");

    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

   //Initialise Beeper Output Pin
   pinMode(beeperPin, OUTPUT);
   beepFor(2);
   
   //Initialise Relay Output Pin
   pinMode(relayPin, OUTPUT);
   digitalWrite(relayPin, HIGH);
   delay(3000);

   //Initialise PIR Sensor Input Pin
   pinMode(interruptPin, INPUT);

   stopTime = rtc.now();
   
   //Attach External Interrupt Pin to Interrupt Service Routine
   attachInterrupt(digitalPinToInterrupt(interruptPin), ISR0, RISING);

}



void loop () {
    /*derp*/
    Serial.println(digitalRead(interruptPin));
    delay(1000);
}

void ISR0 () {
  //Interrupt Service Routine, as triggered by external interrupt pin
  // If-statement here is for software debouncing
  if((long)( micros() - lastMicros) >= debounceTime*1000){
      state = HIGH; //PIR detected motion and has triggered ISR
      blinkFor(3); //blink LED to notify ISR triggered
      
      Serial.println(F("Start ISR"));
      beepFor(1);
    
      //Store time stamp for reference
      now = rtc.now();  
    
      //Convert to seconds for comparison
      TimeToStop = stopTime.hour()*3600 + stopTime.minute() * 60 + stopTime.second();
      TimeNow = now.hour()*3600 + now.minute() * 60 + now.second();

      beepFor(1);
    
      if ( TimeNow > TimeToStop ){
        
          if ( state == HIGH ){
            //If motion detected then latch relay closed
            digitalWrite(relayPin, LOW);
            stopTime = rtc.now() +  TimeSpan(0,0, DelayMins ,0);
            //Serial.println("Set RELAY CLOSED");
          }
          else if ( state == LOW ) {
            //If no motion present then latch relay open
            digitalWrite(relayPin, HIGH);  
            //Serial.println("Set RELAY OPEN");
          }
          else{/*cbf*/}
      }
      else{ 
          /*
           * We haven't time-out
           * But let's check if we're close to timing-out
           * and give a warning if true.
           * 10 second pre-heat.
           */
          if ( (TimeNow + 10) > TimeToStop ){
              beepFor(2);
          }
          else{/*cbf*/}
      }    
    Serial.println(F("End of ISR"));
    lastMicros = micros();
  }
  else{/*cbf*/}
} //End of ISR0 Function

void beepFor(int n){
  beeperState = LOW; //Force Start LOW
  n = 2*n; //A single-beep has both rising and falling edge
  for(i = 0; i < n; i++){
    beeperState = !beeperState;
    digitalWrite(beeperPin, beeperState);
    delay(100);
  }
  digitalWrite(beeperPin, LOW); //ok, that's enough
}

void blinkFor(int n){
  ledState = LOW; //Force Start LOW
  for(i = 0; i < n; i++){
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    delay(500);
  }
  digitalWrite(ledPin, LOW); //idle
}
