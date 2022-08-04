# <a name="AP_DccLED"></a>AP_DccLED #

Arduino library containing all the LED specific code needed for the various DCC decoders. Although this library is part of the DCC decoder core Library, it can also be used seperately.

There are already several LED related libraries for model trains, of which the best known are MobaTools (https://github.com/MicroBahner/MobaTools) and the MobaLedLib (https://github.com/Hardi-St/MobaLedLib). This library supports flashing, fade-out and some specific methods that are shared between all my decoders.

To accommodate specific needs and to save valuable RAM space, four different classes have been defined (between brackets how much RAM is needed per object instantiation):
- BasicLed (2 bytes)
- FlashLed (11 bytes)
- DccLed (11 bytes)
- FadeOutLed (23 bytes)

Technically, the FlashLed inherits methods and attributes from the BasicLed, the DccLed inherits from the FlashLed and the FadeOutLed again inherits from the BasicLed. This means that each class can use all BasicLed methods, and the DccLed can also use the methods and (public) attributes from the FlashLed.

---

## <a name="BasicLed"></a>BasicLed ##
The `BasicLed` class is intended for simple LED actions, such as turning the LED on, off or to toggle its state.

#### - void attach (uint8_t pin, bool invert=false) ####
To connect the processor output pin to the LED. In many cases the output pin for the onboard LED is `LED_BUILTIN`.

Although most boards are designed to switch the LED `ON` once the output pin gets `HIGH`, there are also some boards where the logic is inverted and the LED switches `ON` once the output pin gets `LOW`. In such cases the optional parameter `invert` may be used to change the output logic. If `invert` is not specified, the default behaviour is not-inverted.

#### - bool ledIsOn(void) ####
Returns true if the LED is ON. Can also be used in case of inverted logic.

#### - void turn_on(void) ####
To turn the LED on.

#### - void turn_off(void) ####
To turn the LED off.

#### - void toggle(void) ####
If the LED is on, it will be turned off.
If the LED is off, it will be turned on.

---

## <a name="FlashLed"></a>FlashLed ##
The `FlashLed` class allows the flashing of LEDs. The class inherits methods and attributes from the `BasicLed` class, so all the methods described above can  be used again. In addition, the following methods and attributes exist.

#### FlashLed Attributes: ####
All Flash attributes or of the uint8_t type, which means that the possible values range from 0..255. All times are specified in 100ms steps, which means that the highest value is 25,5 seconds.

- uint8_t flashOntime <br>
Time the LED should be ON (in 100ms steps)
- uint8_t flashOfftime <br>
Time the LED should be OFF (in 100ms steps)
- uint8_t flashPause <br>
Time in between a series of flashes in which the LED should remain off  (in 100ms steps)
- uint8_t flashCount <br>
Number of flashes before a pause
- bool neverStopFlashing <br>
A boolean to indicate if we want a single series of flashes, or that we start again after a `flashPause` with a new series of pulses.

![FlashLed Attributes](extras/Flashing.png "FlashLed Attributes")

#### FlashLed Methods: ####

#### - void attach (uint8_t pin, bool invert=false) ####
Same as `BasicLed` attach(), but performs some additional initialisation actions.

#### - void flash(void) ####
Puts the LED in flashing mode, based on the settings of attributes as shown above.  

#### - void flashSlow(void) ####
Creates a continuous series of slow flashes. `flashOntime`, `flashOfftime` as well as `flashPause` are all 500ms. The associated attributes are automatically set, and not need be specified again.

#### - void flashFast(void) ####
Creates a continuous series of fast flashes. `flashOntime` is 100ms, and `flashOfftime` as well as `flashPause` are both 200ms.

#### - void update(void) ####
Should be called from main as often as possible. Update will check every 100ms if it is time to turn the LED on or off.

---

## <a name="DccLed"></a>DccLed ##
The idea of the DccLed class is to define some common methods that can be used (and are therefore the same) for each decoder.
The `DccLed` class inherits methods and attributes from the `FlashLed` class, so all the methods described there can  be used again. In addition, the following methods exist.

#### - void start_up(void) ####
To indicate that the decoder (has re)started. Two short flashes of 200ms each.

#### - void activity(void) ####
Single very short flash (200ms), to indicate an activity, such as a switch command.

#### - void feedback(void) ####
Single short flash (500ms), to indicate that an (RS-Bus) feedback message is send.

---

## <a name="FadeOutLed"></a>FadeOutLed ##
The `FadeOutLed` class was defined to allow LEDs to slowly fade out, without using the hardware PWM functions that many boards offer. Development was needed for a board with a Mega2560 processor, which connected a large number of LEDs that could not be attached to pins that supported with hardware PWM. Since the `FadeOutLed` class is relatively expensive in both RAM and CPU usage, in many cases it might be better to use other libraries that support hardware based FadeOut (and FadeIn).

#### FadeOutLed Attributes: ####
All Fade attributes or of the uint8_t type, which means that the possible values range from 0..255. All times are specified in 100ms steps, which means that the the longest time to fade out is 25,5 seconds.
The default values may be fine for most cases.

- uint8_t fadeTime <br>
The time it takes to change the brightness from 100% to 0% (in 100ms steps). The default value is 4 seconds.
- uint8_t fadeSteps <br>
Number of steps between the LED brightness moves from 100% to 0%. Higher values give smoother behaviour, but increase CPU load. The default is 50 steps.
- uint8_t pwmFrequency <br>
The brightness of the LED is set by using a (software generated) PWM (Pulse Width Modulation) signal. The default frequency is 50Hz, which is high enough for the human eye. Higher values increase CPU load.

#### - void attach (uint8_t pin, bool invert=false) ####
Same as `BasicLed` attach(), but performs some additional initialisation actions.

#### - void fadeOut(void) ####
Starts fading the LED, using the values as specified above.

#### - void update(void) ####
Should be called from main as often as possible. Update will check every 100ms if it is time to change the LED intensity.

---

# Examples #
#### Basic on/off ####
````
#include <Arduino.h>
#include <AP_DccLED.h>

BasicLed myLed;                         // Instantiate the LED object
const uint8_t ledPin = LED_BUILTIN;   // Can be used for most boards (13)

void setup() {
  myLed.attach(ledPin);
}

void loop() {
  myLed.turn_on();
  delay(500);
  myLed.turn_off();
  delay(1500);
}
````
#### flashing ####
````
#include <Arduino.h>
#include <AP_DccLED.h>

FlashLed myLed;                      // Instantiate the LED object
const uint8_t ledPin = LED_BUILTIN;   // Can be used for most boards (13)

void setup() {
  myLed.attach(ledPin);
  myLed.flashOntime = 2;              // 200ms (all times are in 100ms steps)
  myLed.flashOfftime = 5;             // 500ms
  myLed.flashCount = 5;
  myLed.flashPause = 20;              // 2 seconds
  myLed.neverStopFlashing = true;  
  myLed.flash();
}

void loop() {
  myLed.update();
}
````
