# AP_DccTimer #

Arduino library to create a (non-blocking) timer that uses the Arduino `millis()` method. It should run on all AVR processors that are using Arduino software. The library was inspired and to a large extend copied from the MoBaTools library (see acknowledgements below).

The library was developed to support my own DCC decoder software, but might also be useful for other purposes.

### Object instantiation ###
`DccTimer myTimer`

### unsigned long runTime ###
The `runTime` attribute can be set directly, without calling `setTime()`.
This allows setting the runTime without automatically starting the timer.
The `runTime` should be given in milliseconds.

### void setTime(unsigned long runTime) ###
Sets the `runTime` attribute and immediately starts the timer.
The timer will run for a duration of `runTime` milliseconds, unless `stop()` is called before expiry.
If `setTime()` is called again, the `runTime` may be modified.

### void start() ###
Start the timer, using the `runTime` as specified before.

### void restart() ###
Same as `start()`.

### void stop() ###
Stops the timer before expiry. No `expired()` event will be generated.

### bool expired() ###
Returns TRUE if the timer has expired. After expiry, only the first call to `expired()` returns TRUE, any subsequent calls to `expired()` return FALSE. If the timer was stopped before expiry, `expired()` will return FALSE.

### bool running() ###
Returns TRUE if the timer was started, but the `runTime` has not expired yet. Returns FALSE after the timer is stopped.

### unsigned long getRuntime() ###
Returns `runTime` (in ms), as specified before.

### unsigned long getElapsed() ###
Returns the time (in ms) since the timer was last started. If the timer was stopped or got expired, the returned value equals `runTime`.  

### unsigned long getRemain() ###
The time (in ms) that remains till expiry. Is 0 if the timer has expired or was stopped.

# Example #
````
#include <Arduino.h>
#include <AP_DccTimer.h>

DccTimer myTimer;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);    
  myTimer.setTime(1000);
  myTimer.start();
}

void loop() {
  if (myTimer.expired()) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    myTimer.restart();
  }
}
````

# Acknowledgements #
This library is inspired and largely based on MoToTimer.h from the MoBaTools library: https://github.com/MicroBahner/MobaTools. Acknowledgements should therefore go to the developer of MoBaTools,  MicroBahner (Frans-Peter), who also gave valuable feedback during development of this library.
Development  was needed, since the MoBaTools library does not compile on DxCore boards (status: July 2022).
