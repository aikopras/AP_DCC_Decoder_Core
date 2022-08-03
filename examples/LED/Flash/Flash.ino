//******************************************************************************************************
//
// Test sketch for the AP_DccLED library
// The onboard LED should blink 5 times, followed by 2 seconds off
// 2019/02/24 / 2021/06/15 / 2022/07/20 AP
//
//******************************************************************************************************
#include <Arduino.h>
#include <AP_DccLED.h>

Flash_Led myLed;                      // Instantiate the LED object

const uint8_t ledPin = LED_BUILTIN;   // Can be used for most boards (13)


void setup() {
  myLed.attach(ledPin);

  myLed.flashOntime = 2;              // 200ms (all times are in 100ms steps)
  myLed.flashOfftime = 5;             // 500ms
  myLed.flashCount = 5;
  myLed.flashPause = 20;              // 2 seconds
  myLed.mode = neverStopFlashing;  
   
  myLed.flash();
}


void loop() {
myLed.update();
}
