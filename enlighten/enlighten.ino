/*
 * Notes:
 * Relay shall be wired in parallel with physical switch for OR-logic.
 * i.e. Automation system can control the lighting OR the user can force 
 * the light on. To achieve this, we write LOW to the NO relay. 
 * The alternative is a safety functionality via series connection for AND-logic.
 * i.e. When the Automation system fails (blows up/ sets on fire), the light can 
 * still be switch on or off regardless. We then write HIGH to the NC relay.
 */
// Timer Interrupt Library
#include <TimerOne.h>
 
// Date and time functions using a DS3231 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"

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

//Declare Flags for ISR
boolean setTimerFlag = false;


//Set RTC type
RTC_DS3231 rtc;

//Set delay in minutes
const int DelayMins = 2;
const int minuteOverflow = 60 - DelayMins;
boolean minuteOverflowFlag = false;
boolean timeOutFlag = false;

DateTime stopTime;
DateTime now;

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
//   beepFor(2);
   
   //Initialise Relay Output Pin
   pinMode(relayPin, OUTPUT);
   digitalWrite(relayPin, HIGH);
   delay(3000);

   //Initialise PIR Sensor Input Pin
   pinMode(interruptPin, INPUT);

   stopTime = rtc.now();

   //Attach Timer Interrupt
   Timer1.initialize(2500000); // handle is Period in microseconds
   Timer1.attachInterrupt(TISR); //Timer Interrupt Service Routine
   //Attach External Interrupt Pin to Interrupt Service Routine
   attachInterrupt(digitalPinToInterrupt(interruptPin), ISR0, RISING);
}



void loop () {
    /*
     * RTClib functions can not be directly called from ISR
     * Need to use flags
     * GG, no power saving
     */
    
//    Serial.println(digitalRead(interruptPin));

    //Set time stamp for reference
    now = rtc.now();  

    if (setTimerFlag){
      state = HIGH;
      //Set time-stamp for when to stop
      stopTime = rtc.now() +  TimeSpan(0,0, DelayMins ,0);
      //Check for timer overflow scenario
        if ( stopTime.minute() >= minuteOverflow ){
          minuteOverflowFlag = true;
        } 
        else{
          minuteOverflowFlag = false;
        }
      setTimerFlag = false;
    }
    else{

      state = LOW;
      
    }
    
    delay(1000);
}

void TISR () {

  if (minuteOverflowFlag && (now.minute() < stopTime.minute())){
    timeOutFlag = true;
  }
  else if (!(minuteOverflowFlag) && (now.minute() > stopTime.minute())){
    timeOutFlag = true;
  }
  else{
    timeOutFlag = false;
  }
  
  if ( timeOutFlag && (state == LOW) ){        
    //Latch relay open i.e. lights off
    digitalWrite(relayPin, HIGH);
  }
  else{}   
  
} // End of TISR

void ISR0 () {
  //Interrupt Service Routine, as triggered by external interrupt pin
  // If-statement here is for software debouncing
  if((long)( micros() - lastMicros) >= debounceTime*1000){
         
      blinkFor(3); //blink LED to notify ISR triggered
      
//      Serial.println(F("ISR triggered"));

      /*
       * We then want to know if the Light is already on.
       * i.e. if they are already on, then we don't need to 
       * digitalWrite to it; just reset the stop-time (flag).
       */
        if ( state == LOW ){         
          //note: state = LOW means lights are off
          //latch relay closed i.e. lights on
          digitalWrite(relayPin, LOW);            
        }
        else{/*cbf*/}
        //Let's reset the stop-time so the light never dies
        setTimerFlag = true;  
      
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
  n = 2*n; //A single-blink has both rise and fall edges
  for(i = 0; i < n; i++){
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    delay(500);
  }
  digitalWrite(ledPin, LOW); //idle
}
