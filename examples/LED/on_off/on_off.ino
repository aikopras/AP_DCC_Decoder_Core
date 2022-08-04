//******************************************************************************************************
//
// Test sketch for the AP_DccLED library
// The onboard LED should stay on 500 ms, followed by 1500 ms silence
// 22022/07/20 AP
//
//******************************************************************************************************
#include <Arduino.h>
#include <AP_DccLED.h>

BasicLed myLed;                         // Instantiate the LED object

const uint8_t ledPin = LED_BUILTIN;     // Can be used for most boards (13)


void setup() {
  myLed.attach(ledPin);
}


void loop() {
  myLed.turn_on();
  delay(500);
  myLed.turn_off();
  delay(1500);
}
