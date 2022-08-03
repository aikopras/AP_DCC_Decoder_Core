//******************************************************************************************************
//
// Test sketch for the AP_DCC_Button library
// Example sketch to demonstrate toggle buttons.
//
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
// https://github.com/JChristensen/JC_Button
//
// 2021/06/27 AP
//
//******************************************************************************************************
#include <Arduino.h>
#include <AP_DCC_Button.h>

// pin assignments
const uint8_t buttonPin = 7;               // connect a button switch from this pin to ground
const uint8_t ledPin = 13;                 // the standard Arduino "pin 13" LED

ToggleButton myButton;                     // define the toggle button

void setup() {
  myButton.attach(buttonPin);              // attach and initialize the button object
  pinMode(ledPin, OUTPUT);                 // set the LED pin as an output
  // show the initial states
  digitalWrite(ledPin, myButton.toggleState());
}

void loop() {
  // read the buttons
  myButton.read();
  // if button state changed, update the LEDs
  if (myButton.changed()) digitalWrite(ledPin, myButton.toggleState());
}
