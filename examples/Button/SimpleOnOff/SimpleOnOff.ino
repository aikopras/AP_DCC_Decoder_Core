//******************************************************************************************************
//
// Test sketch for the AP_DCC_Button library
// Example sketch to turn an LED on and off with a tactile button switch.
// Wire the switch from the Arduino pin to ground.
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
const uint8_t buttonPin = 7;         // connect a button switch from this pin to ground
const uint8_t ledPin = 13;           // the standard Arduino "pin 13" LED

DCC_Button myButton;                 // define the button


void setup() {
  myButton.attach(buttonPin);        // attach and initialize the button object
  pinMode(ledPin, OUTPUT);           // set the LED pin as an output
}


void loop() {
  static bool ledState;              // a variable that keeps the current LED status
  myButton.read();                   // read the button
  if (myButton.wasReleased()) {      // if the button was released, change the LED state
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
}
