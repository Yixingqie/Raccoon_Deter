/*Connections for the NRF24L01
   1 - GND
   2 - VCC 3.3V
   3 - CE to Arduino pin 7
   4 - CSN to Arduino pin 8
   5 - SCK to Arduino pin 13
   6 - MOSI to Arduino pin 11
   7 - MISO to Arduino pin 12
   8 - UNUSED
*/

//include the following libraries
#include <SPI.h>
#include "RF24.h"
#include "printf.h"

#define  CE_PIN  7   // The pins to be used for CE and SN
#define  CSN_PIN 8

#define tmp   A7  // input from thermistor
#define ldr   A6
#define volt  A5
#define moisture A4

const int motion = 9;     // motion sensor
const int  mosfet =  10;      // light
const int lower_bound = 21;

const unsigned long sendtime = 60000L;
//const int sends = 1200;
const unsigned long sense_time = 5000;
const int thresh = 40;
unsigned long previousMillis = 0;        // will store last time LED was updated
bool resetcount = false;
// constants won't change:
const long interval = 45;           // interval at which to blink (milliseconds)
int ledState = LOW;
int track  = 0;
bool trigger = false;
bool ignore = false;
//int end_of_day_counter  = 0;

unsigned long previous = 0;        // will store last time LED was updated
unsigned long previous_sense = 0;


//hardware configuring
RF24 radio(CE_PIN, CSN_PIN);
int count = 0;
//set up connection pipes
byte addresses[][6] = {"raccoon", "raccoon2"};

struct myData {
  unsigned long _micros;  // to save response times
  int temp;          //temp data to be transmitted
  int motion_count;
  int ldr_sense;
  int voltage;
  int moisture_level;
  bool light;
  bool trigger;
} myData;


void setup()
{
  Serial.begin(115200);  //sets up serial monitor
  pinMode(motion, INPUT);
  // initialize the pushbutton pin as an input:
  pinMode(mosfet, OUTPUT);
  radio.begin();          // Initialize the nRF24L01 Radio
  radio.setChannel(108);
  radio.setDataRate(RF24_250KBPS); // speed

  radio.setPALevel(RF24_PA_MAX);//HIGH range-->power usuage:HIGH

  // opens the pipes for connections
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);
  digitalWrite(mosfet, LOW);

}
void loop()
{
  radio.stopListening();                                   //stops listening
  myData.temp = analogRead(tmp);
  myData.ldr_sense = analogRead(ldr);
  myData.voltage = analogRead(volt);
  myData._micros = micros();  // Send back for timing
  myData.moisture_level = analogRead(moisture);
  unsigned long currentMillis = millis();
  trigger = digitalRead(motion);
  if ((trigger && (map(myData.ldr_sense, lower_bound, 1023, 0, 100) < thresh)) || (ignore && trigger)) {
    ignore = true;
    if ((unsigned long)(currentMillis - previousMillis) >= interval) {
      // save the last time you blinked the LED
      previousMillis = currentMillis;

      // if the LED is off turn it on and vice-versa:
      if (ledState == LOW) {
        ledState = HIGH;
      } else {
        ledState = LOW;
      }

      // set the LED with the ledState of the variable:
      digitalWrite(mosfet, ledState);
    }

  } else {
    digitalWrite(mosfet, LOW);
    ignore = false;
  }


  if (trigger) {
    //   Serial.println("Triggered");
    myData.trigger = true;
  } else {
    //   Serial.println("Not Triggered");
    myData.trigger = false;
  }
  if ((trigger && (map(myData.ldr_sense, lower_bound, 1023, 0, 100) < thresh)) || (ignore && trigger) ) {
    //   Serial.println("Light ON");
    myData.light = true;
    if (!resetcount) {
      resetcount = true;
      count++;
    }
  } else {
    resetcount = false;
    //   Serial.println("Light OFF");
    myData.light = false;
  }
  if (count >= 15) {
    count = 0;
  }

  myData.motion_count = count;

//  if (trigger && ((unsigned long)(currentMillis - previous_sense) >= sense_time)) {
//
//    previous_sense = currentMillis;
//    if (!radio.write( &myData, sizeof(myData) )) {           //continous data sending to prevent lag
//      //    Serial.println(F("Transmit failed "));
//    }
//    Serial.println();
//  }

  if ((unsigned long)(currentMillis - previous) >= sendtime || (trigger && ((unsigned long)(currentMillis - previous_sense) >= sense_time))) {
//    //  end_of_day_counter++;
//    // save the last time you blinked the LED
//    if (trigger) {
//      //   Serial.println("Triggered");
//      myData.trigger = true;
//    } else {
//      //   Serial.println("Not Triggered");
//      myData.trigger = false;
//    }
//    if ((trigger && (map(myData.ldr_sense, lower_bound, 1023, 0, 100) < thresh)) || (ignore && trigger) ) {
//      //   Serial.println("Light ON");
//      myData.light = true;
//      if (!resetcount) {
//        resetcount = true;
//        count++;
//      }
//    } else {
//      resetcount = false;
//      //   Serial.println("Light OFF");
//      myData.light = false;
//    }
    previous_sense = currentMillis;
    previous = currentMillis;
//    if (count >= 15) {
//      count = 0;
//    }
//
//    myData.motion_count = count;
//
//    //    if (end_of_day_counter >= sends) {
//    //      count  = 0;
//    //      end_of_day_counter = 0;
//    //    }
//    //  Serial.print("Number of Encounters: ");
//    //   Serial.println(myData.motion_count);
//    /*
//      Serial.print("Current Temperature: ");
//      Serial.print((int)((((myData.temp * 5.0) / 1024.0) - 0.5) * 100));
//      Serial.println(" Celsius");
//      Serial.print("Battery Voltage: ");
//      Serial.print((myData.voltage / 1023.0) * 5.0);
//      Serial.println("V");
//      Serial.print("Daylight Level: ");
//      Serial.print(map(myData.ldr_sense, lower_bound, 1023, 0, 100));
//      Serial.println("%");
//      Serial.print("Moisture Level: ");
//      Serial.print(map(myData.moisture_level, 0, 1023, 0, 100));
//      Serial.println("%");
//    */
    if (!radio.write( &myData, sizeof(myData) )) {           //continous data sending to prevent lag
      //    Serial.println(F("Transmit failed "));
    }
    Serial.println();
  }

}
