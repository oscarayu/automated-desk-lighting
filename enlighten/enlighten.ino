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

//PIR value reading
int val = 0;

//Set delay in minutes
int DelayMins = 10;

DateTime stopTime;

void setup () {

//  Serial.begin(9600);

//  delay(2000); // wait for console opening

  if (! rtc.begin()) {
//    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    
//    Serial.println("RTC lost power, lets set the time!");

    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }


     //Relay Output Pin
   pinMode(2, OUTPUT);
   digitalWrite(2, HIGH);
   delay(3000);

   //PIR Sensor Input Pin
   pinMode(5, INPUT);

   stopTime = rtc.now();

}



void loop () {
    
  //Read PIR sensor value
  val = digitalRead(5);

  DateTime now = rtc.now();  
  
  int TimeToStop = stopTime.hour()*3600 + stopTime.minute() * 60 + stopTime.second();
  int TimeNow = now.hour()*3600 + now.minute() * 60 + now.second();
  
  if ( TimeNow > TimeToStop ){
    
      if ( val == HIGH ){
        
        //If motion detected then latch relay closed
        digitalWrite(2, LOW);
        stopTime = rtc.now() +  TimeSpan(0,0, DelayMins ,0);
//        Serial.println("Set RELAY CLOSED");
        
      }
      else if ( val == LOW ) {
        //If no motion present then latch relay open
        digitalWrite(2, HIGH);  
//        Serial.println("Set RELAY OPEN");
      }
      else{}
  
  }
  else{}

  delay(1500);
}
