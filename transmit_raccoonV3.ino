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
#include "LowPower.h"
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

const unsigned long sendtime = 36000L; //10 mins rate= 3.6S clock /60S real
const unsigned long sense_time = 120; // 2 seconds
const int thresh = 10;
unsigned long previousMillis = 0;        // will store last time LED was updated
bool resetcount = false;
// constants won't change:
const long interval = 40;           // interval at which to blink (milliseconds)
int ledState = LOW;
int track  = 0;
bool trigger = false;
bool ignore = false;
int count_send = 0;


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
  int send_count;
} myData;


void setup()
{
  myData.send_count = count_send;
  myData.motion_count = count;
  //Serial.begin(115200);  //sets up serial monitor
  pinMode(motion, INPUT);
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
  unsigned long time_Millis = millis();
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
  if (count_send >= 36) {
    count_send = 0;
    myData.send_count  = count_send;
  }

  if (count_send == 0) {
    count_send++;
    myData.send_count = count_send;
    if (!radio.write( &myData, sizeof(myData) )) {           //continous data sending to prevent lag
      //    Serial.println(F("Transmit failed "));
    }

    Serial.println();
  }

  if (!myData.light && (map(myData.ldr_sense, lower_bound, 1023, 0, 100) > thresh)) {
    count_send++;
    myData.send_count = count_send;
    for (int i = 0; i < 75; i++) { //for 10 minutes
      LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); //put to sleep
    }
    radio.write( &myData, sizeof(myData) );
    radio.write( &myData, sizeof(myData) );
    radio.write( &myData, sizeof(myData) );

  } else if (!myData.light) {
    LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_OFF); //put to sleep
  }


  unsigned long timeSense = (unsigned long)(currentMillis - previous_sense);
  if (myData.light && (timeSense >= sense_time)) {

    count_send++;
    myData.send_count = count_send;
    previous_sense = currentMillis;
    radio.write( &myData, sizeof(myData) );
    //  radio.write( &myData, sizeof(myData) );
    //   radio.write( &myData, sizeof(myData) );
  }

  // Serial.println(time_Millis);
  unsigned long timeSend = (unsigned long)(time_Millis - previous);
  if ((map(myData.ldr_sense, lower_bound, 1023, 0, 100) < thresh) && (timeSend >= sendtime)) {
    count_send++;
    myData.send_count = count_send;
    previous = time_Millis;
    radio.write( &myData, sizeof(myData) );
    radio.write( &myData, sizeof(myData) );
    radio.write( &myData, sizeof(myData) );
  }

}
