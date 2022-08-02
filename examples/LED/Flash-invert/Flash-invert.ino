//******************************************************************************************************
//
// Test sketch for the AP_DCC_LED library
// Some boards have an onboard LED that goes ON if the output becomes low.
// An example of such board is the AVR128DA64 curiosity nano
// For such boards attach() may include a second (boolean) parameter, that indicates whether or not
// the output should be inverted. If this second parameter is omitted, the output will not be inverted.
// 
// The onboard LED should blink 5 times, followed by 2 seconds off
// 2022/07/20 AP
//
//******************************************************************************************************
#include <Arduino.h>
#include <AP_DCC_LED.h>

Flash_Led myLed;                         // Instantiate the LED object

const uint8_t ledPin = PIN_PC6;          // AVR128DA64 Curiosity Nano. Inverted logic
const bool invert = true;                // true for AVR128DA64 Curiosity Nano. Otherwise false


void setup() {
  myLed.attach(ledPin, invert);

  myLed.flashOntime = 2;            // 200ms (all times are in 100ms steps)
  myLed.flashOfftime = 5;           // 500ms
  myLed.flashCount = 5;
  myLed.flashPause = 20;            // 2 seconds
  myLed.mode = neverStopFlashing;  
   
  myLed.flash();
}


void loop() {
myLed.update();
}
