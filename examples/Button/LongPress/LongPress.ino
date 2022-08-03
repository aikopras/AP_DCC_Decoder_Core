//******************************************************************************************************
//
// Test sketch for the AP_DCC_Button library
// Example sketch demonstrating short and long button presses.
//
// A simple state machine where a short press of the button turns the Arduino LED on or off,
// and a long press causes the LED to blink rapidly.
// Once in rapid blink mode, another long press goes back to on/off mode.
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
const uint8_t buttonPin = 7;               // connect a button switch from this pin to ground
const uint8_t ledPin = 13;                 // the standard Arduino "pin 13" LED
const unsigned long LONG_PRESS = 1000;     // we define a "long press" to be 1000 ms.
const unsigned long BLINK_INTERVAL = 100;  // in the BLINK state, switch the LED every 100 ms.

DCC_Button myButton;                       // define the button


void setup() {
  myButton.attach(buttonPin);              // attach and initialize the button object
  pinMode(ledPin, OUTPUT);                 // set the LED pin as an output
}

// The list of possible states for the state machine. This state machine has a fixed
// sequence of states, i.e. ONOFF --> TO_BLINK --> BLINK --> TO_ONOFF --> ONOFF
// Note that while the user perceives two "modes", i.e. ON/OFF mode and rapid blink mode,
// two extra states are needed in the state machine to transition between these modes.
enum states_t { ONOFF,
                TO_BLINK,
                BLINK,
                TO_ONOFF };

bool ledState;            // current LED status
unsigned long ms;         // current time from millis()
unsigned long msLast;     // last time the LED was switched

void loop() {
  static states_t STATE;  // current state machine state
  ms = millis();          // record the current time
  myButton.read();        // read the button

  switch (STATE) {
    // this state watches for short and long presses, switches the LED for
    // short presses, and moves to the TO_BLINK state for long presses.
    case ONOFF:
      if (myButton.wasReleased())
        switchLED();
      else if (myButton.pressedFor(LONG_PRESS))
        STATE = TO_BLINK;
      break;

    // this is a transition state where we start the fast blink as feedback to the user,
    // but we also need to wait for the user to release the button, i.e. end the
    // long press, before moving to the BLINK state.
    case TO_BLINK:
      if (myButton.wasReleased())
        STATE = BLINK;
      else
        fastBlink();
      break;

    // the fast-blink state. Watch for another long press which will cause us to
    // turn the LED off (as feedback to the user) and move to the TO_ONOFF state.
    case BLINK:
      if (myButton.pressedFor(LONG_PRESS)) {
        STATE = TO_ONOFF;
        digitalWrite(ledPin, LOW);
        ledState = false;
      } else
        fastBlink();
      break;

    // this is a transition state where we just wait for the user to release the button
    // before moving back to the ONOFF state.
    case TO_ONOFF:
      if (myButton.wasReleased())
        STATE = ONOFF;
      break;
  }
}

// reverse the current LED state. if it's on, turn it off. if it's off, turn it on.
void switchLED() {
  msLast = ms;  // record the last switch time
  ledState = !ledState;
  digitalWrite(ledPin, ledState);
}

// switch the LED on and off every BLINK_INTERVAL milliseconds.
void fastBlink() {
  if (ms - msLast >= BLINK_INTERVAL)
    switchLED();
}
