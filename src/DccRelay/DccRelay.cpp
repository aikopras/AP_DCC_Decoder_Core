//******************************************************************************************************
//
// file:      AP_DccRelay.cpp
// author:    Aiko Pras
// history:   2022-05-25 V1.0 ap: Initial version
//            2022-07-17 V1.1 ap: changed to replace the timer library
//
// purpose:   Functions for a bi-stable relay (or a switch)
//
// Despite the fact that they are slow, we use standard Arduino digitalWrite() functions.
// MegaCoreX and DxCore also provide a digitalWriteFast() variant, but such variant requires the pin
// to be constant and known at compile time. Although that could be done for various decoders, having
// constants for the pin numbers is not feasable in case of a generic library.
//
//******************************************************************************************************
#include <Arduino.h>
#include "AP_DccRelay.h"

void relayClass::init(uint8_t pinPos1, uint8_t pinPos2, uint8_t holdTime) {
  _pinPos1 = pinPos1;
  _pinPos2 = pinPos2;
  pinMode(_pinPos1, OUTPUT);
  pinMode(_pinPos2, OUTPUT);
  relayTimer.runTime = holdTime * 20;         // holdTime is in 20ms steps
  state = UNKNOWN;
}


void relayClass::activate(rState_t newState) {
  switch (newState) {
    case POS1:
      if (state != POS1) {
        if (relayTimer.running() == false) {
          relayTimer.start();                 // Start the timer, to allow later deactivation
          digitalWrite(_pinPos1, HIGH);
          state = POS1;
        }
      };
    break;
    case POS2:
      if (state != POS2) {
        if (relayTimer.running() == false) {
          relayTimer.start();                 // Start the timer, to allow later deactivation
          digitalWrite(_pinPos2, HIGH);
          state = POS2;
        }
      };
    break;
    case UNKNOWN:
    break;
  }
}


void relayClass::update() {
  if (relayTimer.expired()) {
    if (state == POS1) {digitalWrite(_pinPos1, LOW);}
    if (state == POS2) {digitalWrite(_pinPos2, LOW);}
  }
}

