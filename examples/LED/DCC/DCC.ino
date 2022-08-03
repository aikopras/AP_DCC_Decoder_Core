//******************************************************************************************************
//
// Test sketch for the AP_DccLED library
// 2019/02/24 / 2021/06/15 / 2022/07/20 AP
//
//******************************************************************************************************
#include <Arduino.h>
#include <AP_DccLED.h>

DCC_Led myLed;                           // Instantiate the LED object

const uint8_t ledPin = LED_BUILTIN;      // Can be used for most boards (13)


void setup() {
  delay(2000);
  myLed.attach(ledPin);
 
  myLed.start_up();
//  myLed.activity();
//  myLed.feedback();
//  myLed.flashSlow();
//  myLed.flashFast();
}


void loop() {
myLed.update();
}
