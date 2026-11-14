//*****************************************************************************************************
//
// File:      relays.h
// Author:    Aiko Pras
// History:   2026/11/14 AP Version 1.0
//
// Purpose:   Header file for the mono-stable relays of the RelaysXDecoder example.
//            The number of relays and their pins are defined in hardware.h.
//            After init() all relays are off.
//
// The state of the relays is stored in nibbles (4 bits): one nibble per 4 relays.
// - nibble 0 holds relays 1..4  (relay 1 = bit 0, relay 4 = bit 3)
// - nibble 1 holds relays 5..8
// - ...
// Since each decoder address controls 4 relays, and each RS-Bus message carries one nibble,
// a nibble belongs to a single decoder address and is sent in a single RS-Bus message.
//
// This class has no knowledge of DCC or RS-Bus: the main sketch determines which relay should be
// switched, and which nibble should be sent.
//
//*****************************************************************************************************
#pragma once
#include <Arduino.h>
#include "hardware.h"                 // For NumberOfRelays


const uint8_t NumberOfNibbles = NumberOfRelays / 4;


class Relays {
  public:
    void init(void);                  // Switches all relays off. Should be called from setup()

    bool set(                         // Returns true if the state of the relay has changed
      uint8_t number,                 // 1..NumberOfRelays - The relay to switch
      uint8_t value                   // 0..1 - 0 = off, 1 = on
    );

    uint8_t nibble(                   // Returns the state of 4 relays (0..15)
      uint8_t index                   // 0..NumberOfNibbles-1. Returns 0 for higher values
    );

  private:
    uint8_t state[NumberOfNibbles];   // One nibble per byte (the 4 high order bits are 0)
};


extern Relays relays;                 // Instantiated in relays.cpp
