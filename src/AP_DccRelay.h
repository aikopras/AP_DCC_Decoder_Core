//******************************************************************************************************
//
// file:      AP_DccRelay.h
// author:    Aiko Pras
// history:   2022-05-25 V1.0 ap: Initial version
//            2022-07-17 V1.1 ap: changed to replace the timer library
//
// purpose:   Relay object. Relays are bi-stable, and have two coils.
//            Relay can be switched to POS1 or POS2.
//            At startup, the relay may be in an UNKNOWN state
//            The time an activation command will hold, should be specified in 20ms steps, to be
//            consistent with existing CV values.
//
//******************************************************************************************************
#pragma once
#include <AP_DccTimer.h>      // For the timers


class relayClass {
  public:
    // A relay may be in / switch to one of the following states
    typedef enum {POS1, POS2, UNKNOWN} rState_t;
    rState_t state;                   // Current state

    void init(uint8_t pinPos1, uint8_t pinPos2, uint8_t holdTime);
    void activate(rState_t newState); // to switch to a new state
    void update();                    // should be called as frequent as possible from main

  private:
    DccTimer relayTimer;              // Timer object to switch off (any of the two) coils
    uint8_t _pinPos1;                 // Arduino pin to activate Coil 1
    uint8_t _pinPos2;                 // Arduino pin to activate Coil 2
    uint8_t _holdTime;                // in 20ms steps
};

