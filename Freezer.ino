/*
This is an example of how to use the OLED 128x64 I2C with SSD1306 driver using the Adafruit library.
It also applies to the 128x32 version, but not all components would fit the smaller screen.

Pins:
 * GND = GND
 * VCC = 5V
 * SCL = A5
 * SDA = A4

You can connect VCC to 3.3V to reduce the amount of high pitched noise that the display produces.

It's a good idea to put a resistor between A4-5V and A5-5V to help stabilize the connection.
What that does is pull-up the I2C pins to make it more reliable and prevents lock-ups.

*/


#include <Adafruit_GFX.h>  // Include core graphics library for the display
#include <Adafruit_SSD1306.h>  // Include Adafruit_SSD1306 library to drive the display


Adafruit_SSD1306 display(128, 64);  // Create display


#include <Fonts/FreeMonoBold12pt7b.h>  // Add a custom font
#include <Fonts/FreeMono9pt7b.h>  // Add a custom font


#include <OneWire.h>
#include <DallasTemperature.h>
 
// Data wire is plugged into pin D5 on the Arduino
#define ONE_WIRE_BUS 5
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Addresses of 3 DS18B20s
// Freezer Sensor
uint8_t sensor1[8] = { 0x28, 0xE3, 0xE2, 0x56, 0x05, 0x00, 0x00, 0xD3 };
// Outsite Sensor
uint8_t sensor2[8] = { 0x28, 0x72, 0xF0, 0x55, 0x05, 0x00, 0x00, 0x22 };
// Insite Sensor
// uint8_t sensor3[8] = {  };

float TemperatureFreez;
float TemperatureOut;
float TemperatureIn;
float MaxTempFreez = 0;
float MinTempFreez = 0;
float MaxTempOut = 0;
float MinTempOut = 0;
float MaxTempIn = 0;
float MinTempIn = 0;
char Text[100];
const int TargetTemp = 7;
const float Hysteresis = 0.5;

// Buzzer to pin D4
const int Buzzer = 4;

// Pin for Relays
const int Relay1 = 2;
const int RelaySolarIn = 3;

// Pin for Solar Power
const int SolarPin = 2;
// Solar Power Indicator
bool SolarPower = false;
int SolarValue;
unsigned long SolarTime = 0;
unsigned long SolarOld;
unsigned long SolarNew;

void setup()  // Start of setup
{                

  delay(100);  // This delay is needed to let the display to initialize

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize display with the I2C address of 0x3C
 
  display.clearDisplay();  // Clear the buffer

  display.setTextColor(WHITE);  // Set color of the text

  display.setRotation(0);  // Set orientation. Goes from 0, 1, 2 or 3

  display.setTextWrap(false);  // By default, long lines of text are set to automatically “wrap” back to the leftmost column.
                               // To override this behavior (so text will run off the right side of the display - useful for
                               // scrolling marquee effects), use setTextWrap(false). The normal wrapping behavior is restored
                               // with setTextWrap(true).

  display.dim(0);  //Set brightness (0 is maximun and 1 is a little dim)

  // Start up the Dallas Temperature IC Control Library
  sensors.begin();

  // initialize digital pins as an output.
  pinMode(Relay1, OUTPUT);
  pinMode(RelaySolarIn, OUTPUT);
  // turn the pin off by making the voltage HIGH
  digitalWrite(Relay1, HIGH);   
  // the relay will be switches on when the system is running
  // wen the system is not running the relay will prevent power at the input pin
  // turn the pin on by making the voltage LOW
  digitalWrite(RelaySolarIn, LOW);  

  // Set buzzer - pin as output
  pinMode(Buzzer, OUTPUT); 
  // Relay switches OFF !!!
  digitalWrite(Relay1, HIGH);

}  // End of setup

void DisplayTemperature(){
  int Seconds;
  int Minutes;
  int Hours;
  unsigned long solarDuration;
  char tText[100];
  
  display.clearDisplay();  // Clear the display so we can refresh
  // display.setFont(&FreeMonoBold12pt7b);  // Set a custom font
  display.setFont(&FreeMono9pt7b);  // Set a custom font
  display.setTextSize(0);  // Set text size. We are using a custom font so you should always use the text size of 0

  // Print text 1:
  // workaround to display float numbers
  dtostrf(TemperatureFreez, 3, 1, tText);  // (<variable>,<amount of digits we are going to use>,<amount of decimal digits>,<string name>)
  snprintf(Text,sizeof(Text),"TempF: %s", tText);
  display.setCursor(5, 15);  // (x,y)
  display.println(Text);  // Text or value to print

  // Print text 2:
  dtostrf(TemperatureOut, 3, 1, tText);  // (<variable>,<amount of digits we are going to use>,<amount of decimal digits>,<string name>)
  snprintf(Text,sizeof(Text),"TempO: %s", tText);
  // snprintf(Text, sizeof(Text), "TempO: %d.1", TemperatureOut);
  display.setCursor(5, 35);  // (x,y)
  display.println(Text);  // Text or value to print

  // Print text 3:
  display.setCursor(5, 55);  // (x,y)
  if (SolarPower == true) {
    solarDuration = SolarTime;
    // Convert milliseconds to seconds
    solarDuration = solarDuration / 1000;
    // Calculate hours
    Hours = solarDuration / 3600;
    // Get rest
    solarDuration = solarDuration % 3600;
    // Calculate Minutes
    Minutes = solarDuration / 60;
    // Get rest
    solarDuration = solarDuration % 60;
    Seconds = solarDuration;
    if (Minutes < 10){
      snprintf(Text, sizeof(Text), "Solar: %i:0%i", Hours, Minutes);
    }else {
     snprintf(Text, sizeof(Text), "Solar: %i:%i", Hours, Minutes); 
    }
    display.println(Text);
  } else {
    display.println("Solar: off");
  }
  
  display.display();  // Print everything we set previously
}

void DisplayError(){
  display.clearDisplay();  // Clear the display so we can refresh
  display.setFont(&FreeMonoBold12pt7b);  // Set a custom font
  display.setTextSize(0);  // Set text size. We are using a custom font so you should always use the text size of 0

  // Print text:
  display.setCursor(5, 15);  // (x,y)
  display.println("Kein");  // Text or value to print
  display.setCursor(5, 40);  // (x,y)
  display.println("Sensor");  // Text or value to print
  display.display();  // Print everything we set previously
}

float ReadTemperature(DeviceAddress deviceAddress){
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures
  return sensors.getTempC(deviceAddress);
}

void TestSolarPower(){
  SolarValue= analogRead(SolarPin);
  if (SolarValue > 200){
    // Solar Power On
    SolarPower = true;
    SolarNew = millis();
    SolarTime = SolarTime + SolarNew - SolarOld;
    SolarOld = SolarNew;
  } else {
    // Solar Power Off
    SolarPower = false;
    SolarOld = millis();
  }
}


void loop()  // Start of loop
{
  TemperatureFreez = ReadTemperature(sensor1);

  while (TemperatureFreez < -100){
    // No temperature sensor
    DisplayError();
    // Buzzer on
    digitalWrite(Buzzer, HIGH);
    delay(500);  
    // Buzzer off
    digitalWrite(Buzzer, LOW);
    delay(500); 
    TemperatureFreez = ReadTemperature(sensor1);
  }
  if (TemperatureFreez < TargetTemp){
    // turn the pin on by making the voltage HIGH
    // Relay switches OFF !!!
    digitalWrite(Relay1, HIGH); 
  } else {
    if (TemperatureFreez > (TargetTemp + Hysteresis)){
      // turn the pin off by making the voltage LOW
      // Relay switches ON !!!
      digitalWrite(Relay1, LOW); 
    }
  }

  if (TemperatureFreez > MaxTempFreez){
    // New max. temperature
    MaxTempFreez = TemperatureFreez;
  } else if (TemperatureFreez < MinTempFreez){
    // New min. temperature
    MinTempFreez = TemperatureFreez;
  }

  TemperatureOut = ReadTemperature(sensor2);
  if (TemperatureOut > MaxTempOut){
    // New max. temperature
    MaxTempOut = TemperatureOut;
  } else if (TemperatureOut < MinTempOut){
    // New min. temperature
    MinTempOut = TemperatureOut;
  }
  /* TemperatureIn = ReadTemperature(sensor3);
    if (TemperatureIn > MaxTempIn){
    // New max. temperature
    MaxTempIn = TemperatureIn;
  } else if (TemperatureIn < MinTempIn){
    // New min. temperature
    MinTempIn = TemperatureIn;
   */

  TestSolarPower();
  DisplayTemperature();

  delay(1000);

}  // End of loop
