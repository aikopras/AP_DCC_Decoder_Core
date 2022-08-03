//******************************************************************************************************
//
// file:      AP_DccLED.cpp
// author:    Aiko Pras
// history:   2019-02-24 V1.0.2 ap initial version, changed into Arduino library
//            2021-06-15 V1.1   ap attach/detach for pins (instead of constructor)
//            2021-06-26 V1.2   ap extended with fade out
//            2022-07-20 V1.3   ap divided into multiple objects, to save RAM if methods are not needed
//
// purpose:   Functions related to LEDs
//
// Object inheritance (between brackets the required RAM)
// Basic_Led (2) +--> Flash_Led (11) ---> DCC_Led (11)
//               +--> FadeOut_Led (23)
//
//******************************************************************************************************
#include <Arduino.h>
#include "AP_DccLED.h"


//******************************************************************************************************
// Basic_Led: Simple on/off LEDs
//******************************************************************************************************
void Basic_Led::attach(uint8_t pin, bool invert){
  _pin = pin;
  pinMode(_pin, OUTPUT);
  if (invert) _LED_ON = LOW;                // In some cases the LED goes ON if the pin goes LOW
  else _LED_ON = HIGH;
}

bool Basic_Led::ledIsOn(void){
  if (digitalRead(_pin) == _LED_ON) return true;
    else return false;
}

void Basic_Led::turn_on(void){
  digitalWrite(_pin, _LED_ON);
}

void Basic_Led::turn_off(void){
  digitalWrite(_pin, !_LED_ON);
}

void Basic_Led::toggle(void){
  if (ledIsOn()) turn_off();
  else turn_on();
}


//******************************************************************************************************
// Flash_Led
//******************************************************************************************************
// attach(), turn_on() and turn_off() must be extended by modifying mode / initialising last_flash_time
void Flash_Led::attach(uint8_t pin, bool invert){
  Basic_Led::attach(pin, invert);
  last_flash_time = millis();
}


void Flash_Led::turn_off(void){
  mode = alwaysOff;
  Basic_Led::turn_off();
}


void Flash_Led::turn_on(void){
  mode = alwaysOn;
  Basic_Led::turn_on();
}


void Flash_Led::flash(void) {
  last_flash_time = millis();
  flash_time_remain = flashOntime;               // we start with the LED on
  flash_number_now = 1;                          // This is the first flash
  Basic_Led::turn_on();
}


// make a series of flashes, then a longer pause
void Flash_Led::flashSlow(void) {
  flashOntime = 5;                // 0,5 sec
  flashOfftime = 5;               // 0,5 sec
  flashCount = 1;                 // 1 flashes
  flashPause = 5;
  mode = neverStopFlashing;
  flash();
}


void Flash_Led::flashFast(void) {
  flashOntime = 1;                // 0,1 sec
  flashOfftime = 2;               // 0,2 sec
  flashCount = 1;                 // 1 flashes
  flashPause = 2;
  mode = neverStopFlashing;
  flash();
}


void Flash_Led::update(void) {
  if (mode == alwaysOn) return;                  // No update needed
  if (mode == alwaysOff) return;                 // No update needed
  unsigned long current_time = millis();         // Storing millis() in a local variable gives shorter code
  if ((current_time - last_flash_time) >= 100) { // We only update the LED every 100 msec
    last_flash_time = current_time;
    --flash_time_remain;                         // Another 100 msec passed
    if (flash_time_remain == 0) {                // Do we need to change state?
      if (ledIsOn()) {                           // LED is ON but will, at the end of update(), be set OFF
        if (flash_number_now != flashCount)      // We did not yet had all flashes of the series
          flash_time_remain = flashOfftime;      // So we will wait a normal off time
        else {                                   // We had the complete series of flashes
          if (mode == neverStopFlashing) {       // We have to start a new series of flashes
            flash_time_remain = flashPause;      // Next we will wait a longer pause
            flash_number_now = 0;                // Restart the counter for the required number of flashes
          }
          else mode = alwaysOff;                 // We don't need to start a new series of flashes//
        }
      }
      else {                                     // LED is OFF but will, at the end of update(), be set ON
        flash_time_remain = flashOntime;         // Next will be a certain time on
        flash_number_now++;                      // Increment the number of blinks we did
      }
      toggle();                                    // Change LED ON <-> OFF
    }
  }
}


//******************************************************************************************************
// DCC_Led
//******************************************************************************************************
// Two short flashes, to indicate decoder start
void DCC_Led::start_up(void) {
  flashOntime = 2;                // 0,2 sec
  flashOfftime = 2;               // 0,2 sec
  flashCount = 2;                 // 2 flashes
  mode = singleFlashSerie;
  flash();
}

// Single very short flash, to indicate a switch command
void DCC_Led::activity(void) {
  flashOntime = 2;                // 0,1 sec
  flashCount = 1;                 // single flash
  mode = singleFlashSerie;
  flash();
}

// Single short flash, to indicate a RS-Bus feedback
void DCC_Led::feedback(void) {
  flashOntime = 5;                // 0,5 sec
  flashCount = 1;                 // single flash
  mode = singleFlashSerie;
  flash();
}


//******************************************************************************************************
// FadeOut_Led
//******************************************************************************************************
void FadeOut_Led::attach(uint8_t pin, bool invert){
  Basic_Led::attach(pin, invert);
  fadeTime = 40;                  // In 100ms steps
  fadeSteps = 50;                 // Number of steps between LED is 100% and 0%
  pwmFrequency = 50;              // PWM frequency in Herz (preferably 50 or higher)
}


void FadeOut_Led::fadeOut(void) {
  fadeLedIsOn = false;
  brightnessLevel = fadeSteps;                   // To dim the LED brightness level counts down
  fadeStepTime = 100000 / fadeSteps * fadeTime;
  pwmInterval = 1000000 / pwmFrequency;
  pwmOnTime = pwmInterval / 100 * brightnessLevel ;
  pwmOffTime = pwmInterval - pwmOnTime;
}


void FadeOut_Led::update(void) {
  // Is it time to lower the LED's brightness?
  unsigned long Fade_Interval = micros() - last_fade_time;
  if (Fade_Interval > (fadeStepTime)) {
    if (brightnessLevel >= 1) brightnessLevel--;
    pwmOnTime = pwmInterval / 100 * brightnessLevel ;
    pwmOffTime = pwmInterval - pwmOnTime;
    last_fade_time = micros();
  }
  // Below the code for PWM
  unsigned long PWM_Interval = micros() - last_pwm_time;
  if (fadeLedIsOn) {
    if (PWM_Interval > pwmOnTime) {       // pwmOnTime is over: LED has been on long enough
      last_pwm_time = micros();;
      turn_off();
      fadeLedIsOn = false;
    }
  }
  else {
    if (PWM_Interval > pwmOffTime) {      // pwmOffTime is over: LED has been off long enough
      last_pwm_time = micros();;
      turn_on();
      fadeLedIsOn = true;
    }
  }
}
