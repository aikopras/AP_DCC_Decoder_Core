//******************************************************************************************************
//
// Test sketch for the AP_DccRelay library
// 2022/05/25 AP
//
// Tested on the mega 2560 Lift decoder board
//
//******************************************************************************************************
#include <Arduino.h>
#include <AP_DccRelay.h>

relayClass relay;
long tLast;

#define RELAY_POS1    63  // PIN_PK1 - Number on the Lift decoder PCB: OUT 10
#define RELAY_POS2    64  // PIN_PK2 - Number on the Lift decoder PCB: OUT 11 
#define HOLD_TIME      3  // 3 * 20ms = 60ms

void setup() {
  relay.init(RELAY_POS1, RELAY_POS2, HOLD_TIME);
}


void loop() {
  if ((millis() - tLast) > 1000) {
    tLast = millis();
    if (relay.state == relayClass::POS1) relay.activate(relayClass::POS2);
    else relay.activate(relayClass::POS1);         // If UNKNOWN or POS2 
  }
  relay.update();
}
