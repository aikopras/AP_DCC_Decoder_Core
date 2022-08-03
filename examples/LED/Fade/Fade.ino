//******************************************************************************************************
//
// Test sketch for the AP_DccLED library
// The onboard LED should slowly fade out
// 2021/06/26 AP / 2022/07/20
//
// Each FadeOut_Led needs 23 bytes of RAM.
//
//******************************************************************************************************
#include <Arduino.h>
#include <AP_DccLED.h>

FadeOut_Led myLed;                       // Instantiate the LED object

const uint8_t ledPin = LED_BUILTIN;      // Can be used for most boards

void setup() {
  myLed.attach(ledPin);

  // Set the parameters that control fading
  myLed.fadeTime = 50;                   // 50 * 100ms (all times are in 100ms steps)
  // The default values for fadeSteps and pwmFrequency are 50, but may be modified
  myLed.fadeSteps = 100;                 // Number of steps between LED is 100% and 0%
  myLed.pwmFrequency = 100;              // PWM frequency in Herz (preferably 50 or higher)

  // Initiate fading Out
  myLed.fadeOut();
}


void loop() {
  myLed.update();
}
