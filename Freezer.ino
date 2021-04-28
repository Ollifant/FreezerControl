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
 
// Data wire is plugged into pin 7 on the Arduino
#define ONE_WIRE_BUS 7
 
// Setup a oneWire instance to communicate with any OneWire devices 
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
 
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

float Temperature;
char Text[20];
char Temp[10];
const int TargetTemp = 24;
const int Hysteresis = 1;

// Pin for LEDs
const int LEDred = 8;
const int LEDgreen = 9;
const int LEDblue = 10;

// Buzzer to pin 6
const int Buzzer = 6;

// Pin for Relays
const int Relay1 = 2;

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

  // initialize digital pin as an output.
  pinMode(Relay1, OUTPUT);
  // turn the pin off by making the voltage LOW
  digitalWrite(Relay1, LOW);    

  // Set buzzer - pin as output
  pinMode(Buzzer, OUTPUT); 

  // Set LED - pins as output
  pinMode(LEDred, OUTPUT); 
  pinMode(LEDgreen, OUTPUT); 
  pinMode(LEDblue, OUTPUT); 

}  // End of setup

void DisplayTemperature(float Temp1){
  // Convert Temperature into a string, so we can change the text alignment to the right:
  // It can be also used to add or remove decimal numbers.
  // Convert float to a string:
  dtostrf(Temp1, 3, 1, Temp);  // (<variable>,<amount of digits we are going to use>,<amount of decimal digits>,<string name>)

  display.clearDisplay();  // Clear the display so we can refresh
  // display.setFont(&FreeMonoBold12pt7b);  // Set a custom font
  display.setFont(&FreeMono9pt7b);  // Set a custom font
  display.setTextSize(0);  // Set text size. We are using a custom font so you should always use the text size of 0

  // Print text:
  display.setCursor(5, 15);  // (x,y)
  display.println("Freezer");  // Text or value to print
  strcpy(Text, "Temp: ");
  strcat(Text, Temp);
  display.setCursor(5, 35);  // (x,y)
  display.println(Text);  // Text or value to print
  
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

float ReadTemperature(){
  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus
  sensors.requestTemperatures(); // Send the command to get temperatures
  return sensors.getTempCByIndex(0); // Why "byIndex"? 
    // You can have more than one IC on the same bus. 
    // 0 refers to the first IC on the wire
}


void loop()  // Start of loop
{
  Temperature = ReadTemperature();

  while (Temperature < -100){
    // No temperature sensor
    DisplayError();
    digitalWrite(LEDred, LOW);
    digitalWrite(LEDgreen, LOW);
    // Buzzer on
    digitalWrite(Buzzer, HIGH); 
    digitalWrite(LEDblue, HIGH); 
    delay(500);  
    // Buzzer off
    digitalWrite(Buzzer, LOW);
    digitalWrite(LEDblue, LOW);
    delay(500); 
    Temperature = ReadTemperature();
  }
  if (Temperature < TargetTemp){
    // turn the pin off by making the voltage LOW
    digitalWrite(Relay1, LOW); 
    digitalWrite(LEDgreen, HIGH);
    digitalWrite(LEDred, LOW);
  } else {
    if (Temperature > (TargetTemp + Hysteresis)){
      // turn the pin on by making the voltage HIGH
      digitalWrite(Relay1, HIGH); 
      digitalWrite(LEDred, HIGH);
      digitalWrite(LEDgreen, LOW);
    }
  }
  
  DisplayTemperature(Temperature);

  delay(1000);

}  // End of loop
