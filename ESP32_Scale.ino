#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "HX711.h"
#include <Preferences.h>
#include <string>

using namespace std;

#define tare_pin 3
#define unit_pin 4

// Declaration to assign pins used for HX711 interface
#define HX711_SCK 1
#define HX711_SDA 2

const long LOADCELL_OFFSET = 50682624;
const long LOADCELL_DIVIDER = 5895655;
HX711 scale;


// Declaration for SSD1306 display connected using software SPI (default case):
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_MOSI  35 // map to sda
#define OLED_CLK   36
#define OLED_DC    39
#define OLED_CS    34
#define OLED_RESET 40
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

/* Comment out above, uncomment this block to use hardware SPI
#define OLED_DC     39
#define OLED_CS     34
#define OLED_RESET  40
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  &SPI, OLED_DC, OLED_RESET, OLED_CS);
*/

#define NUMFLAKES  10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };

  Preferences preferences;

const string units[3] = {"gram","ounce","pound"};
int currentUnitIndex = 0;
bool isInTareMode = false;
bool hasTareBeenSet = false;
float lastReading;
void setup() {
  
  system_initialize();
}

void loop() {

  if(digitalRead(tare_pin) == HIGH){
    
    calibrate();
  }
  if(digitalRead(unit_pin) == HIGH){
    IncrementUnitIndex();
  }
    displayScaleReading();



}
// set all system initialization here
void system_initialize(void){
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setTextSize(1);
  display.setCursor(24,16);
  display.setTextColor(SSD1306_WHITE);
  testdrawbitmap();

  scale.begin(HX711_SDA,HX711_SCK);
  scale.set_scale(LOADCELL_DIVIDER);
  scale.set_offset(LOADCELL_OFFSET); 
  
  Serial.println(F("Scale Initialized"));
  display.clearDisplay();

  scale.set_scale(preferences.getFloat("ScaleFactor",0));
}

string getCurrentUnit(void){
  return units[currentUnitIndex];
}
void IncrementUnitIndex(void)
{

  if(currentUnitIndex == 2){
    currentUnitIndex = 0;
  }
  else{
    currentUnitIndex++;
  };
}
void calibrate(void){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(24,0);
    display.println("Calibrating Scale");
    display.display();
    delay(5000);

    scale.tare();
    
    while(digitalRead(tare_pin) == HIGH){ // block execution until button is released
      display.clearDisplay();
      display.println(" Release Tare button to finish calibration");
      display.display();
    }
    delay(5000);
    display.clearDisplay();
    Serial.print("Scale calibrated");
    float scaleFactor = scale.get_units(10);
    Serial.print("Scale factor:");
    Serial.println(scaleFactor);
    preferences.putFloat("ScaleFactor",scaleFactor);
    scale.set_scale(scaleFactor);
}
bool hasValueChanged(void){
  return lastReading == scale.read();
}

void displayScaleReading(void){
  float toDisplay = (scale.get_units(20) >= 0)? scale.get_units(20): 0.00;
  outputToDisplay(toDisplay);
}

void outputToDisplay(float reading){
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(32,24);
  display.println(reading);
  Serial.println(reading);
  display.display();
}


void testdrawchar(void) {
  display.clearDisplay();

  //display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Not all the characters will fit on the display. This is normal.
  // Library will draw what it can and the rest will be clipped.
  for(int16_t i=0; i<256; i++) {
    if(i == '\n') display.write(' ');
    else          display.write(i);
  }

  display.display();
  delay(2000);
}


void testdrawbitmap(void) {
  display.clearDisplay();

  display.drawBitmap(
    (display.width()  - LOGO_WIDTH ) / 2,
    (display.height() - LOGO_HEIGHT) / 2,
    logo_bmp, LOGO_WIDTH, LOGO_HEIGHT, 1);
  display.display();
  delay(1000);
}
