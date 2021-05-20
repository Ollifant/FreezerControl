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

// --------------- 2.13 ePaper Display --------------- 
#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include "GxEPD2_display_selection_new_style.h"

const int DIGITS = 1;
// Constants for 2.13 ePaper display
// Middle of the display
int displayMiddle;
// Font hight
uint16_t fHight;
// Middle of the rows

const int middleRow1 = 35;
const int middleRow2 = 105;
const int middleRow3 = 175;
// Width of rows
const int wRows = 70;
// Left side of rows
const int xRow1 = 0;
const int xRow2 = 70;
const int xRow3 = 140;
const int xSolar = 45;

const uint16_t y1 = 22;
const uint16_t y2 = 45;
const uint16_t y3 = 68;
const uint16_t y4 = 96;
const uint16_t wTime = 65;
uint16_t xTime;

const char Temperature[] = "Temperatur:";
const char textIn[] = "In";
const char textOut[] = "Out";
const char textFreez[] = "Freez";
const char textSolar[] = "Solar: ";

class PrintString : public Print, public String
{
  public:
    size_t write(uint8_t data) override
    {
      return concat(char(data));
    };
};
// ---------------------------------------------------

float TemperatureFreez = 0;
float TemperatureOut = 0;
float TemperatureIn = 0;
float oldFreez = 0;
float oldOut = 0;
float oldIn = 0;
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
unsigned long SolarTime = 0;
unsigned long SolarOld;

int SolarMinutes = 0;

void setup()  // Start of setup
{                
  Serial.begin(9600);
  Serial.println("setup");
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

  // Display initialisieren
  delay(1000);
  initDisplay();

}  // End of setup

void initDisplay(){
  display.init();
  // Set rotation to landscape
  display.setRotation(1);
  // Locate middle of the display to center the headline
  displayMiddle = display.width() / 2;
  // Set font
  display.setFont(&FreeMonoBold9pt7b);
  // Get hight of the font
  fHight = FreeMonoBold9pt7b.yAdvance;
  // Write on display
  displayInitialState(TemperatureIn, TemperatureOut, TemperatureFreez);
}

void displayInitialState(float tempIn, float tempOut, float tempFreez)
{
  int16_t tbx, tby; 
  uint16_t tbw, tbh, xHead, xIn, xOut, xFreez;
  uint16_t xLeft, xMiddle, xRight, wLeft, wMiddle, wRight;
  
  display.setTextColor(GxEPD_BLACK);
  // Anzahl der Nachkommastellen begrenzen
  PrintString valIn, valOut, valFreez;
  valIn.print(tempIn, DIGITS);
  valOut.print(tempOut, DIGITS);
  valFreez.print(tempFreez, DIGITS);

  // Headline
  display.getTextBounds(Temperature, 0, 0, &tbx, &tby, &tbw, &tbh);
  xHead = displayMiddle - (tbw/2);
  
  // Title
  display.getTextBounds(textIn, 0, 0, &tbx, &tby, &tbw, &tbh);
  xIn = middleRow1 - (tbw/2);
  display.getTextBounds(textOut, 0, 0, &tbx, &tby, &tbw, &tbh);
  xOut = middleRow2 - (tbw/2);
  display.getTextBounds(textFreez, 0, 0, &tbx, &tby, &tbw, &tbh);
  xFreez = middleRow3 - (tbw/2);

  // Values
  display.getTextBounds(valIn, 0, 0, &tbx, &tby, &wLeft, &tbh);
  xLeft = middleRow1 - (wLeft/2);
  display.getTextBounds(valOut, 0, 0, &tbx, &tby, &wMiddle, &tbh);
  xMiddle = middleRow2 - (wMiddle/2);
  display.getTextBounds(valFreez, 0, 0, &tbx, &tby, &wRight, &tbh);
  xRight = middleRow3 - (wRight/2);

  // Solar
  display.getTextBounds(textSolar, 0, 0, &tbx, &tby, &tbw, &tbh);
  xTime = xSolar + tbw;
 
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setTextColor(GxEPD_BLACK);
    // Headline
    display.setCursor(xHead, y1);
    display.print(Temperature);
    // Title
    display.setCursor(xIn, y2);
    display.print("In");
    display.setCursor(xOut, y2);
    display.print("Out");
    // Values
    display.setCursor(xFreez, y2);
    display.print("Freez");
    display.setCursor(xLeft, y3);
    display.print(valIn);
    display.setCursor(xMiddle, y3);
    display.print(valOut);
    display.setCursor(xRight, y3);
    display.print(valFreez);
    // Solar Power
    display.setCursor(xSolar, y4);
    display.print(textSolar);
    display.setCursor(xTime, y4);
    display.print("00:00");

    display.writeLine(1,4, 210, 4, GxEPD_BLACK);
  }
  while (display.nextPage());
}

void displayTemperature(double v, int middleRow)
{
  uint16_t xMiddle;
  
  display.setTextColor(GxEPD_BLACK);
  PrintString valueString;
  valueString.print(v, DIGITS);
  int16_t tbx, tby; 
  uint16_t tbw, tbh;

  display.getTextBounds(valueString, 0, 0, &tbx, &tby, &tbw, &tbh);
  xMiddle = middleRow - (tbw/2);
    
  display.setPartialWindow(int(middleRow-(wRows/2)), int(y3-(fHight/2)-5), wRows, fHight);
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(xMiddle, y3);
    display.print(valueString);
  }
  while (display.nextPage());
}

void displaySolarTime(){
  int Seconds;
  int Minutes;
  int Hours;
  unsigned long solarDuration;
  char tText[100];
  char tMinutes[10];
  char tHours[10];

  CheckSolarPower(); 

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
    
    if (Minutes != SolarMinutes){
      // Time changed by min. 1 minutes
      if (Hours < 10){
        snprintf(tHours, sizeof(tHours), "0%i", Hours);
      }
      else{
        snprintf(tHours, sizeof(tHours), "%i", Hours);
      }
      if (Minutes < 10){
        snprintf(tMinutes, sizeof(tMinutes), "0%i", Minutes);
      }
      else{
        snprintf(tMinutes, sizeof(tMinutes), "%i", Minutes);
      }
      snprintf(tText, sizeof(tText), "%s:%s", tHours, tMinutes);
      
      display.setTextColor(GxEPD_BLACK);
      display.setPartialWindow(xTime, int(y4-(fHight/2)), wTime, fHight);
      display.firstPage();
      do
      {
        display.fillScreen(GxEPD_WHITE);
        display.setCursor(xTime, y4);
        display.print(tText);
      }
      while (display.nextPage());
      
      // Save minutes for later comparison
      SolarMinutes = Minutes;
    }
  }
}

void DisplayError(){
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_RED);
 
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(5,30);
    display.print("Fehler !!!");
    display.setCursor(5, 50);
    display.print("Temperatursensor");
    display.setCursor(5, 70);
    display.print("fehlt");
  }
  while (display.nextPage());
}

float ReadTemperature(DeviceAddress deviceAddress){
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures
  return sensors.getTempC(deviceAddress);
}

void CheckSolarPower(){
  unsigned long SolarNew;
  bool solarPowerOld;

  // Save old state
  solarPowerOld = SolarPower;
  // Get new state and save timestamps
  if (analogRead(SolarPin) > 200){
    // Solar Power On
    SolarPower = true;
    SolarNew = millis();
    // Calculate new time
    SolarTime = SolarTime + SolarNew - SolarOld;
    SolarOld = SolarNew;
  } else {
    // Solar Power Off
    SolarPower = false;
    SolarOld = millis();
  }

  if (solarPowerOld != SolarPower){
    // State of the solar system changed
    display.setPartialWindow((xSolar-20), int(y4-(fHight/2)-5), 15, fHight);
    display.firstPage();
    do
    {
      display.fillScreen(GxEPD_WHITE);
      if (SolarPower){
        display.fillCircle ((xSolar-15), (y4 - 5), 5, GxEPD_RED);
      }
      else{
        display.drawCircle ((xSolar-15), (y4 - 5), 5, GxEPD_BLACK);
      }
    }
    while (display.nextPage());
    }
}


void loop()  // Start of loop
{
  TemperatureFreez = ReadTemperature(sensor1);

  if (TemperatureFreez < -100){
    DisplayError();
    while (TemperatureFreez < -100){
      // No temperature sensor
      // Buzzer on
      digitalWrite(Buzzer, HIGH);
      delay(500);
      // Buzzer off
      digitalWrite(Buzzer, LOW);
      delay(500);
      TemperatureFreez = ReadTemperature(sensor1);
    }
    // Restore Display
    displayInitialState(TemperatureIn, TemperatureOut, TemperatureFreez);
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

  TemperatureOut = ReadTemperature(sensor2);
  // TemperatureIn = ReadTemperature(sensor3);
 
  //displayTemperature(TemperatureIn, middleRow1);

  if (abs(TemperatureOut - oldOut) > 0.1){
    // Temperature has changed significantly
    displayTemperature(TemperatureOut, middleRow2);
    // Save new value to old value
    oldOut = TemperatureOut;
  }

  if (abs(TemperatureFreez - oldFreez) > 0.1){
    // Temperature has changed significantly
    displayTemperature(TemperatureFreez, middleRow3);
    // Save new value to old value
    oldFreez = TemperatureFreez;
  }
   
  displaySolarTime();

  //oldIn = TemperatureIn;
  delay(1000);

}  // End of loop
