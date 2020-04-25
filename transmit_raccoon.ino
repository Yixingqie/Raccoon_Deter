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
const int motion = 9;     // the number of the pushbutton pin
const int  mosfet =  10;      // the number of the LED pin
const int thresh = 800;
unsigned long previousMillis = 0;        // will store last time LED was updated
bool resetcount = false;
// constants won't change:
const long interval = 30;           // interval at which to blink (milliseconds)
int ledState = LOW;
int track  = 0;

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

  radio.setPALevel(RF24_PA_LOW);//low range-->power usuage:low

  // opens the pipes for connections
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);

}
void loop()
{
  radio.stopListening();                                   //stops listening
  myData.temp = analogRead(tmp);
  myData.ldr_sense = analogRead(ldr);
  myData.voltage= analogRead(volt);
  myData._micros = micros();  // Send back for timing
  unsigned long currentMillis = millis();
  if (digitalRead(motion) == 1 && myData.ldr_sense < thresh) {
   if(!resetcount){
    count++;
    resetcount = true;
   }
   myData.motion_count = count; 

    if (currentMillis - previousMillis >= interval) {
      // save the last time you blinked the LED
      track +=interval;
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
    resetcount = false;
    track = 0;
    digitalWrite(mosfet, LOW);
  }

  myData.motion_count = count;
  if (currentMillis - previousMillis >= 1000 || track >=1000) {
    if(track>=1000){
      track = 0;
      }
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    Serial.println(myData.motion_count);
    Serial.print((int)((((myData.temp * 5.0) / 1024.0) - 0.5) * 100));
    Serial.println(" Celsius");
    Serial.print((myData.voltage/1023.0)*5.0);
    Serial.println("V");
    Serial.println(myData.ldr_sense);
    if (!radio.write( &myData, sizeof(myData) )) {           //continous data sending to prevent lag
      Serial.println(F("Transmit failed "));
    }
  }

}
