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
#include "LowPower.h"

//include the below libraries
#include <SPI.h>
#include "RF24.h"
//#include "printf.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define  CE_PIN  7   // The pins to be used for CE and SN
#define  CSN_PIN 8
const int low_bound = 20;
const int x = 200;
const int displayPin = 4;

//sets up reciever
RF24 radio(CE_PIN, CSN_PIN);

//sets up connection pipes
byte addresses[][6] = {"rac", "rac2"};

//struct package
struct dataStruct {
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
  //display intializations
  display.clearDisplay();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  display.clearDisplay();
  Serial.begin(115200);
  //  printf_begin();

  //nRFL01 Radio intilization
  radio.begin();
  radio.setChannel(0);
  radio.setDataRate(RF24_250KBPS); // set speed
  radio.setPALevel(RF24_PA_MAX);//max for most range--> high power usuage

  //opens pipes
  //radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);

  // Start the radio listening for data
  radio.startListening();
  pinMode(displayPin, OUTPUT);
  digitalWrite(displayPin, HIGH);
  //digitalWrite(displayPin, INPUT_PULLUP);
  //sets the text color
  display.setTextSize(1); //8.5
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

}

void loop()
{


  //while the radio is available -- > keep listening
  if ( radio.available())
  {
    while (radio.available())
    {
      radio.read( &myData, sizeof(myData) );             // Get the data
    }

    radio.stopListening();
    radio.write( &myData, sizeof(myData) );              // Sends confirmation
    radio.startListening();

    if (map(myData.ldr_sense, low_bound, 1023, 0, 100) >= 50) {
      digitalWrite(displayPin, HIGH);
      //sets the text color
      display.setTextSize(1); //8.5
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
    } else {
      digitalWrite(displayPin, LOW);
      // LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_OFF); //put to sleep

    }
    if (myData.trigger) {
      Serial.println("Triggered");
    } else {
      Serial.println("Not Triggered");
    }
    if (myData.light) {
      Serial.println("Light ON");
    } else {
      Serial.println("Light OFF");
    }
    //prints out the temp data to serial monitor
    Serial.print("Number of Encounters: ");
    Serial.println(myData.motion_count);
    Serial.print("Current Temperature: ");
    Serial.print((int)((((myData.temp * 5.0) / 1024.0) - 0.5) * 100));
    Serial.println(" Celsius");
    Serial.print("Battery Voltage: ");
    Serial.print((myData.voltage / 1023.0) * 5.0);

    if (((myData.voltage / 1023.0) * 5.0) >= 4.0) {
      Serial.print("V");
      Serial.println("--FULL");
    } else {
      Serial.println("V");
    }
    Serial.print("Daylight Level: ");
    Serial.print(map(myData.ldr_sense, low_bound, 1023, 0, 100));
    Serial.println("%");
    Serial.print("Moisture Level: ");
    Serial.print(map(myData.moisture_level, 0, 1023, 0, 100));
    Serial.println("%");

    if (myData.trigger) {
      display.print("ON");
    } else {
      display.print("OFF");
    }
    //    if(myData.light){
    //        display.println("Light ON");
    //      }else{
    //        display.println("Light OFF");
    //      }
    display.print("    ");
    //display.print(abs(((int)myData._micros *100.0)/10000000));
    //display.print("s");
    display.print(myData.send_count);
    display.print("   #: ");
    display.println(myData.motion_count);
    display.println();
    display.print("T: ");
    display.print((int)((((myData.temp * 5.0) / 1024.0) - 0.5) * 100));
    display.print("C");
    display.print("   VCC: ");
    display.print((myData.voltage / 1023.0) * 5.0);
    if (((myData.voltage / 1023.0) * 5.0) >= 3.90) {
      display.print("V");
      display.println(" F");
    } else {
      display.print("V");
      display.println(" D");
    }
    display.println();
    display.print("Daylight: ");
    display.print(map(myData.ldr_sense, low_bound, 1023, 0, 100));
    display.print("%");
    if (map(myData.ldr_sense, low_bound, 1023, 0, 100) >= 50) {
      display.println("  Day");
    } else {
      display.println("  Night");
    }
    display.println();
    display.print("Moisture: ");
    display.print(map(myData.moisture_level, 0, 1023, 0, 100));
    display.print("%");
    if (map(myData.moisture_level, 0, 1023, 0, 100) >= 50) {
      display.println("  Good");
    } else {
      display.println("  Dry");
    }
    display.display();
    display.clearDisplay(); //refreshes display
    //  LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF); //put to sleep

  } else {
    //     LowPower.powerDown(SLEEP_15MS, ADC_OFF, BOD_OFF); //put to sleep

  }

  display.clearDisplay();
}
