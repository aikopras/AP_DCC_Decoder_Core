# <a name="RelaysXDecoder"></a>RelaysXDecoder #

## What this example shows ##
A complete DCC accessory decoder for a configurable number of mono-stable relays, with optional RS-Bus feedback. Next to the basics shown in the [BasicDecoder](../BasicDecoder/BasicDecoder.md) and [DecoderWithoutRsBus](../DecoderWithoutRsBus/DecoderWithoutRsBus.md) examples, it demonstrates:
1. a decoder that listens to **multiple decoder addresses**, and uses **multiple RS-Bus addresses**, both derived from a single constant (`NumberOfRelays`);
2. RS-Bus feedback per **nibble** (`send4bits()`), with a full update (`send8bits()`) only when the master asks for it;
3. a **read-only CV** that tells configuration software how many relays the decoder supports;
4. a sketch that is split into a DCC / RS-Bus part (the .ino file), a hardware part (hardware.h) and an application part (relays.h / relays.cpp).

The decoder type (CV27) is `RelaysXDecoder`.
___

## Files ##
| File | Contents |
| --- | --- |
| RelaysXDecoder.ino | Everything related to DCC and RS-Bus |
| hardware.h | `NumberOfRelays`, `LedBrightness`, and all pins. Adapt this file to your own board |
| relays.h / relays.cpp | Switches the relays and holds their state. Knows nothing about DCC or RS-Bus |

hardware.h holds pin definitions for the AVR32DA48 / AVR128DA48, the AVR32DA32 and the Arduino UNO / NANO (ATmega328P). For the 48 pin DxCore processors and the ATmega328P, the decoder core pins are taken from [boards.h](../../src/boards.h). The AVR32DA32 is not in boards.h, so its decoder core pins are defined in hardware.h itself. For any other processor, compilation stops with an `#error`; add a block for that processor to hardware.h.

Since boards.h defines global constants named `dccPin`, `ackPin`, `ledPin` and so on, the names in hardware.h start with a capital: `DccPin`, `AckPin`, `LedPin`, ...
___

## Decoder addresses ##
Each relay has its own accessory (switch) address: `+` switches the relay on, `-` switches it off. A decoder address holds 4 accessory addresses, so the decoder listens to `NumberOfRelays / 4` consecutive decoder addresses. The first one is stored in CV1 / CV9:

| Decoder address | Turnout 1..4 |
| --- | --- |
| first | relays 1..4 |
| first + 1 | relays 5..8 |
| ... | ... |

```cpp
  firstDecoderAddress = cvValues.storedAddress();
  lastDecoderAddress = firstDecoderAddress + NumberOfDecoderAddresses - 1;
  accCmd.setMyAddress(firstDecoderAddress, lastDecoderAddress);
```

As long as the decoder address has not been set, `storedAddress()` returns 65535. The range then starts above the highest possible decoder address (511), so no accessory command will match.

The decoder uses decoder addressing (CV29 bit 6 = 0), and only reacts on activate commands. After startup all relays are off.
___

## RS-Bus feedback ##
If CV33 (`SendFB`) is 1, the state of the relays is sent via the RS-Bus. An RS-Bus address carries 8 bits, sent as two nibbles of 4 bits. Each nibble holds the 4 relays of one decoder address, so the decoder uses `NumberOfRelays / 8` (rounded up) consecutive RS-Bus addresses, starting at CV10 (`myRSAddr`):

| RS-Bus address | Low nibble | High nibble |
| --- | --- | --- |
| first | relays 1..4 | relays 5..8 |
| first + 1 | relays 9..12 | relays 13..16 |
| ... | ... | ... |

Within a nibble, the first relay is bit 0.

- After a relay has changed, only the nibble that holds this relay is sent, using `send4bits()`.
- After a decoder restart, or after an RS-Bus error, the master asks for all feedback information (`feedbackRequested`). In that case both nibbles are sent, using `send8bits()`.
- RS-Bus address 128 is reserved for PoM feedback. If the last RS-Bus address would become 128 or higher, no feedback is sent at all.

The RS-Bus messages use the default type `Switch`.

#### A board without RS-Bus ####
Set `RsBusRX` and `RsBusUsart` in hardware.h to `PIN_UNDEFINED`. The sketch itself needs no changes: the RSbus library then simply sends nothing, whatever the value of CV33. Note, however, that the CVs can then no longer be read via PoM, since PoM feedback uses the RS-Bus as well. Writing via PoM, and reading and writing via Service Mode (programming track) remain possible.
___

## CV64: number of relays (read only) ##
Configuration software needs to know how many relays a decoder supports. The sketch therefore writes `NumberOfRelays` to CV64 at every startup:

```cpp
const uint8_t RelayCount = 64;        // CV64 (RO): number of relays this decoder supports

  cvValues.write(RelayCount, NumberOfRelays);
```

`cvValues.write()` uses `EEPROM.update()`, so the EEPROM is only written if the value differs. CV64 is outside the range CV1..CV63 that `setDefaults()` writes, so a decoder reset leaves it alone. Writing another value via PoM or Service Mode is possible, but has no effect after the next restart.
___

## LEDs ##
| LED | Meaning |
| --- | --- |
| `LedPin` | Handled by the core: address not set (slow flashing) / programming mode |
| `DccLedPin` | A DCC message is received |
| `AccLedPin` | An accessory (switch) command for one of the relays is received |
| `RsLedPin` | An RS-Bus feedback message is sent |

The last three LEDs stay on for `LedOnTime` ms, and are controlled via `analogWrite()` with `LedBrightness`. On a pin without PWM, `analogWrite()` switches the pin LOW for values below 128, so a dimmed LED would never light up. With an `LedBrightness` below 255 the LED pins must therefore be PWM pins, on a timer that is not used by other software. See hardware.h for the details per processor.
___

## Monitor ##
If `DEBUG` is defined at the top of the sketch, information is printed on the `Monitor` output (see hardware.h). On the ATmega328P there is only one USART, which is also used for RS-Bus transmission; DEBUG and RS-Bus can therefore not be used together on an UNO or NANO.
