# <a name="DecoderWithoutRsBus"></a>DecoderWithoutRsBus #

## What this example shows ##
The [BasicDecoder](../BasicDecoder/BasicDecoder.md) example shows a decoder for a predefined board: the pins come from [boards.h](../../src/boards.h), and feedback is sent over the RS-Bus. This example shows the opposite case — a decoder on a board that the library does not know, without RS-Bus, and with its own Configuration Variables.

Four things are demonstrated:
1. telling the core which Arduino pins are used, from the sketch instead of from boards.h;
2. leaving the RS-Bus out;
3. defining a decoder type of your own;
4. defining Configuration Variables of your own.

Nothing in the library has to be modified for any of this.
___

## 1. Setting the pins from the sketch ##
The core holds the pin numbers in a set of [attributes](../../src/CommonFunctions/CommonFunctions.md#Attributes). The constructor fills them with the values from boards.h, so a sketch that says nothing keeps the old behaviour. A sketch that needs something else simply overwrites the attributes it cares about:

```cpp
  decoderHardware.dccPin    = DccPin;
  decoderHardware.ackPin    = AckPin;
  decoderHardware.ledPin    = LedPin;
  decoderHardware.buttonPin = ButtonPin;
  decoderHardware.init();                  // always last
```

The assignments must come **before** `init()`, since that is where the pin numbers are handed to the hardware objects.

Note how the sketch names its constants: `DccPin`, `LedPin` and so on, with a capital. boards.h defines global constants named `dccPin`, `ackPin`, `ledPin` and `buttonPin`, and since boards.h is included by the core header, those names are in scope in every sketch. Writing `const uint8_t dccPin = PIN_PA5;` would therefore be a redefinition error.
___

## 2. Leaving the RS-Bus out ##
```cpp
  decoderHardware.rsBusRX = PIN_UNDEFINED;   // this decoder never uses the RS-Bus
```

`PIN_UNDEFINED` (255, defined in boards.h) tells the core that a piece of hardware is not present. The RS-Bus is then not attached at all: no timer is claimed and no interrupt is installed.

This line is needed even though the sketch never sends any feedback. Without it, the RS-Bus attributes keep their boards.h values, and on a board that *is* listed in boards.h the RS-Bus hardware would be initialised anyway.

The same mechanism works for `ledPin` and `buttonPin`. Leaving out the button is worth a thought: the decoder address can then no longer be set by pushing the programming button, so Service Mode (programming track) is the only way left. `dccPin` and `ackPin` are not checked — a DCC decoder without a DCC input serves no purpose.
___

## 3. A decoder type of your own ##
CV27 holds the decoder type. The values used by the library are listed in [CvValues.h](../../src/CvValues/CvValues.h); the range `0b1110xxxx` (0xE0..0xEF) is reserved for types that a sketch defines itself, and will never be used by the library:

```cpp
const uint8_t FormsignalDecoder = 0b11100000;   // Semaphore signal
```

An unknown decoder type is handled gracefully. `cvValues.init()` fills CV1..CV32 with the values that are the same for every decoder, and then looks up the decoder type to fill the decoder specific CVs. A type that it does not recognise matches nothing, so CV33..CV63 are simply left at 0 — which is exactly what is wanted here, since the sketch fills them itself.
___

## 4. Configuration Variables of your own ##
CV33 up to and including CV63 are free for the sketch. Give them a name, and assign a default value:

```cpp
const uint8_t EnableExtLed = 33;   // 0..1: the external LED is enabled (1) or disabled (0)

  cvValues.init(FormsignalDecoder, SoftwareVersion);
  cvValues.defaults[EnableExtLed] = 1;
```

The order is what makes this work:

| | |
| --- | --- |
| `cvValues.init(...)` | fills the `defaults[]` array in RAM; does **not** touch the EEPROM |
| `cvValues.defaults[...] = ...` | changes that array, so the sketch's own values are in it |
| `decoderHardware.init()` | copies `defaults[]` to EEPROM, but only if the EEPROM is still empty |

CV63 is the last one that can be given a default this way; the `defaults[]` array holds 64 entries (index 0 is the marker that says the EEPROM has been initialised). Higher CV numbers can be read and written, up to the size of the processor's EEPROM, but they cannot be given a default value here.

#### When do the defaults reach the EEPROM? ####
Only when `setDefaults()` is called. `decoderHardware.init()` does that automatically, but only for a decoder whose EEPROM is still empty. On a decoder that is already in use the stored values are kept — which is the whole point of storing them.

This matters while developing: uploading new software does not necessarily clear the EEPROM. Whether it does depends on the EESAVE fuse, and fuses are not rewritten on every upload. So after changing a default value in the sketch, the new value will not appear by itself. Force it, either by a long push of the onboard programming button, or by writing 0x0D to CV8 (VID) with a PoM or Service Mode message.
___

## The complete sketch ##
```cpp
//*****************************************************************************************************
//
// File:      DecoderWithoutRsBus.ino
// Author:    Aiko Pras
// History:   2026/07/29 AP Version 1.0
//
// Purpose:   DCC Accessory decoder without RS-Bus feedback
//            Demonstrates how to set pins and Configuration Variables (CVs) for a new decoder board
//            Glues together all common objects, including DCC message processing,
//            Configuration Variables, DCC-Ack signal, Programming button and LED.
//
// This basic example prints the contents of new DCC Accessory and PoM commands.
//
//******************************************************************************************************
#include <Arduino.h>                  // For general definitions
#include <AP_DCC_Decoder_Core.h>      // To include all objects, such as dcc, accCmd, etc.

// Define the pins that we need.
// Use capitals, to avoid potential name conflicts with the library's boards.h
const uint8_t ButtonPin = PIN_PA2;    // used by decoderHardware.init
const uint8_t LedPin = PIN_PA3;       // used by decoderHardware.init
const uint8_t DccPin = PIN_PA5;       // used by decoderHardware.init
const uint8_t AckPin = PIN_PA6;       // used by decoderHardware.init
const uint8_t ExtLedPin = PIN_PB1;    // specific for this sketch

// Define a new decoder type. Types starting with 1110 are user defined
const uint8_t FormsignalDecoder = 0b11100000;   // Semaphore signal
const uint8_t SoftwareVersion = 20;

// Define new CVs (33..63)
const uint8_t EnableExtLed = 33;   // 0..1: The external LED is enabled (1) or disabled (0)

//******************************************************************************************************
void setup() {
  // Serial monitor is used for debugging
  delay(100);
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("Start");
  delay(100);
  //
  // Step 1: Set CV default values (see CvValues.h. for details)
  // Decoder type (DecType) and software version (version) are set using cvValues.init().
  // After cvValues.init() is called, CV default values may be modified using cvValues.defaults[...]
  // Updated values are copied to EEPROM when setDefaults() is called. This happens
  // automatically by decoderHardware.init() below, but only if the EEPROM is still empty.
  // Note that flashing new software does not necessarily clear the EEPROM: that depends on
  // the EESAVE fuse, which is not rewritten on every upload. If the EEPROM already holds
  // values, they are kept, and setDefaults() must be triggered explicitly: by a long push of
  // the onboard programming button, or by a CV-POM or CV-SM message writing 0x0D to CV8 (VID).
  cvValues.init(FormsignalDecoder, SoftwareVersion);
  // Define the CVs specific for this decoder
  cvValues.defaults[EnableExtLed] = 1;
  //
  // Step 2: Configure the pins and initialise the dcc object
  decoderHardware.dccPin = DccPin;
  decoderHardware.ackPin = AckPin;
  decoderHardware.ledPin = LedPin;
  decoderHardware.buttonPin = ButtonPin;
  decoderHardware.rsBusRX = PIN_UNDEFINED;   // enforce that this decoder never uses RS-Bus
  decoderHardware.init();
  //
  // Print for testing purposes
  for (uint8_t i = 0; i <= 34; i++) {
    Serial.print("CV");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(cvValues.read(i));
  }
  if (cvValues.addressNotSet()) Serial.println("Address not set");
  Serial.print("Decoder Address: ");
  Serial.println(cvValues.storedAddress());
  //
  if (cvValues.read(EnableExtLed)) {
    pinMode(ExtLedPin, OUTPUT);
  }
}


//******************************************************************************************************
void loop() {
  // Step 1: Print the contents of new DCC Accessory and PoM commands
  if (dcc.input()) {
    switch (dcc.cmdType) {
      case Dcc::MyAccessoryCmd :
        if (accCmd.command == Accessory::basic)
          Serial.print(" Basic accessory command for my decoder address: ");
        else
          Serial.print(" Extended accessory command for my decoder address: ");
        Serial.println(accCmd.decoderAddress);
        Serial.print(" - Turnout: ");
        Serial.println(accCmd.turnout);
        Serial.print(" - Switch number: ");
        Serial.print(accCmd.outputAddress);
        if (accCmd.position == 1) Serial.println(" +");
        else Serial.println(" -");
        if (accCmd.activate) Serial.println(" - Activate");
        Serial.println();
        break;
      case Dcc::AnyAccessoryCmd :
        Serial.print(accCmd.decoderAddress);
        Serial.print(" (any)");
        Serial.println();
        break;
      case Dcc::MyPomCmd :
        cvProgramming.processMessage(Dcc::MyPomCmd);
        Serial.print("PoM Command. ");
        Serial.print("Received CV Number: ");
        Serial.print(cvCmd.number);
        Serial.print(" - Received CV Value: ");
        Serial.println(cvCmd.value);
        break;
      case Dcc::SmCmd :
        cvProgramming.processMessage(Dcc::SmCmd);
        break;
      default:
        // Nothing
        break;
    }
  }
  // Step 2: as frequent as possible we should check if the programming button is pushed,
  // and if the status of the onboard LED should be changed.
  decoderHardware.update();
}
```
___

## Notes ##
The loop over the CV values, and the printing of the decoder address, are there for testing only; a real decoder does not need them.

`ExtLedPin` is configured as an output if `EnableExtLed` says so, but this example does not drive it any further. Adding that is left to the decoder that this skeleton grows into.

Since this decoder has no RS-Bus, PoM messages can be received but nothing can be returned. Writing CV values with PoM therefore works, but reading them back requires the programming track.
