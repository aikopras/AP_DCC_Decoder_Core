# <a name="DccRelay"></a>DccRelay #

Arduino library for a single, bi-stable, relays or switch.
Not intended for mono-stable relays!

The idea is to concentrate all relay specific code into this library, and extend this library later whenever needed.

Uses AP_DccTimer

### init() ###
Should be called during sketch startup. Includes the following parameters:
- `pinPos1`: Arduino pin number connected to coil 1
- `pinPos2`: Arduino pin number connected to coil 2
- `holdTime`: time in steps of 20ms the coil should be activated (uint8_t)
The choice for 20ms steps is made to allow easy interfacing with common DCC-CV values.
After initialisation, the relay's position will be UNKNOWN.

### activate() ###
To set the relays to a specific position. Will not activate the coil if the relay is already in the requested position, or if the relay has just been activated and the holdTime has not been passed yet.
Includes the following parameter:
- `newState`: the requested new position. The parameter is of the rState_t type, and should be either POS1 or POS2

### update() ###
Should be called as frequent as possible from the loop of the main sketch. Will deactivate the coil once the holdTime has passed.

### state ###
state is a parameter that holds the state the relay is currently in (or is moving to). The parameter is of type rState_t, which is an enumeration that can take one of the following values: POS1, POS2, UNKNOWN.

# Example #
````
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
````
