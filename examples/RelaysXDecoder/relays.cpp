//*****************************************************************************************************
//
// File:      relays.cpp
// Author:    Aiko Pras
// History:   2026/11/14 AP Version 1.0
//
// Purpose:   C++ file that implements the methods for the mono-stable relays.
//            Only standard Arduino functions are used, so this code does not depend on a
//            specific processor.
//
//*****************************************************************************************************
#include <Arduino.h>
#include "hardware.h"                 // For NumberOfRelays and RelayPins[]
#include "relays.h"                   // Header file for this C++ file

Relays relays;                        // The object used by the main sketch


//*****************************************************************************************************
void Relays::init(void) {
  // Switch all relays off. The output is set LOW before the pin becomes an output, to avoid a glitch
  for (uint8_t i = 0; i < NumberOfRelays; i++) {
    digitalWrite(RelayPins[i], LOW);
    pinMode(RelayPins[i], OUTPUT);
  }
  for (uint8_t i = 0; i < NumberOfNibbles; i++) state[i] = 0;
}


//*****************************************************************************************************
bool Relays::set(uint8_t number, uint8_t value) {
  // Returns true if the state of the relay has changed
  if ((number < 1) || (number > NumberOfRelays)) return false;
  uint8_t index = (number - 1) / 4;           // The nibble that holds this relay
  uint8_t bit = (number - 1) % 4;             // The bit within that nibble
  uint8_t newState = state[index];
  if (value) bitSet(newState, bit);
    else bitClear(newState, bit);
  if (newState == state[index]) return false;
  state[index] = newState;
  digitalWrite(RelayPins[number - 1], value ? HIGH : LOW);
  return true;
}


//*****************************************************************************************************
uint8_t Relays::nibble(uint8_t index) {
  if (index >= NumberOfNibbles) return 0;
  return state[index];
}
