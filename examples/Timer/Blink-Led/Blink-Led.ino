#include <Arduino.h>
#include <AP_DCC_Timer.h>

DccTimer myTimer;
// Arduino UNO / Mega: LED_BUILTIN
// AVR Curiosty Nano: PIN_PC6 is the on-board LED and negative logic
uint8_t LedPin = PIN_PC6;

void setup() {
  pinMode(LedPin, OUTPUT);
  myTimer.setTime(1000);
}

void loop() {
  if (myTimer.expired()) {
    digitalWrite(LedPin, !digitalRead(LedPin));
    myTimer.start();
  }
}
