# <a name="CommonDecHwFunctions"></a>CommonDecHwFunctions #

## Purpose ##
Every DCC decoder needs the same small set of hardware: a DCC input, an acknowledgement pin for Service Mode programming, usually an onboard LED and a programming button, and — for feedback decoders — the RS-Bus. `CommonDecHwFunctions` ties these together, so that the main sketch only has to call two functions: `init()` and `update()`.

The class also decides *which* Arduino pins and which USART are used. That is what the attributes below are for.
___

## <a name="Attributes"></a>Pin attributes ##

#### Why these attributes exist ####
The pin and USART assignments for the decoder boards are held in [boards.h](../boards.h). The right set is selected at compile time, based on the board / processor that was chosen in the Arduino IDE. That works well as long as the board being used is one of the boards listed there.

It does not work in three cases:
- a **new board**, or a processor that is already known but with the signals on different pins;
- a **processor that has no entry** in boards.h at all, such as the ATtiny 0/1/2-series;
- a decoder that **deliberately lacks some of this hardware** — no RS-Bus, no LED, or no programming button.

Until now the only answer was to modify boards.h. That is unattractive: boards.h is part of a library, so the modification is lost whenever the library is updated, and it applies to *every* sketch that is compiled afterwards, even if only a single sketch needs it.

The attributes below solve this. The constructor fills them with the values from boards.h, so nothing changes for existing sketches — they keep working unmodified. A sketch that needs something different simply overwrites the attributes it cares about, before calling `init()`.

#### The attributes ####
| Attribute | Type | Purpose |
| --- | --- | --- |
| `dccPin` | uint8_t | DCC input signal |
| `ackPin` | uint8_t | DCC acknowledgement, used during Service Mode programming |
| `ledPin` | uint8_t | Onboard LED |
| `buttonPin` | uint8_t | Onboard programming button |
| `rsBusRX` | uint8_t | RS-Bus polling input |
| `rsBusUsart` | uint8_t | Number of the USART used to transmit RS-Bus data (0, 1, ...) |
| `swapUsartPin` | bool | Use the alternative USART pin (MegaCoreX and DxCore processors) |

#### How to use them ####
```cpp
void setup() {
  // Only needed if this board differs from the boards in boards.h
  decoderHardware.dccPin    = PIN_PA2;
  decoderHardware.ackPin    = PIN_PA3;
  decoderHardware.ledPin    = PIN_PB0;
  decoderHardware.buttonPin = PIN_PA4;
  decoderHardware.rsBusRX   = PIN_UNDEFINED;   // this decoder has no RS-Bus
  decoderHardware.init();                      // always last
}
```

The order matters. `decoderHardware` is a global object, so its constructor has already run by the time `setup()` starts. Assignments made in `setup()` therefore replace the boards.h defaults. They must be made **before** `init()`, since `init()` is what actually hands the pin numbers to the hardware objects.
___

## <a name="PIN_UNDEFINED"></a>Leaving hardware out: PIN_UNDEFINED ##

#### Why a special value ####
A decoder without an RS-Bus, or without an onboard LED, is a normal thing to build. The core therefore needs a way to be told "this decoder does not have that", and it must be able to distinguish that from a valid pin number. `PIN_UNDEFINED` (255, defined in boards.h) is used for this, because 255 is a pin number that no Arduino core regards as valid.

Assigning `PIN_UNDEFINED` is safe on every supported processor: the LED, button and RS-Bus objects test the pin number before they use it, and simply do nothing if it is out of range. Nothing is written to a pin that does not exist.

#### What happens per attribute ####
- **`ledPin`** — no LED is attached, and every LED call becomes a no-op. The decoder can then no longer signal start-up, "address not yet set", or the search function (CV23).
- **`buttonPin`** — no button is attached. The decoder address can then no longer be set by pushing the programming button; use Service Mode (programming track) instead. Note that this also disables restoring the CVs to their default values by a long push; that remains possible by writing 0x0D to CV8.
- **`rsBusRX`** — the RS-Bus is not used at all.
- **`rsBusUsart`** — the decoder does not transmit on the RS-Bus. If `rsBusRX` is a valid pin number, it monitors activity on the RS-Bus but never sends.
- **`dccPin` and `ackPin`** — these are **not** checked. A DCC decoder without a DCC input serves no purpose, so no attempt is made to support that case.

A board that has no entry in boards.h falls through to the final `#else` branch, where all seven values are `PIN_UNDEFINED`. Sketches for such a board must therefore at least set `dccPin` and `ackPin` themselves.
___

## <a name="init"></a>void init(void) ##
Should be called once from `setup()` in the main sketch, after any attributes have been given a value.

`init()` performs the following steps:
1. If the EEPROM that holds the CV values has not been initialised yet (a new decoder, or one whose EEPROM was erased), it is filled with the default CV values.
2. The `dcc`, `rsbusHardware`, `onBoardLed` and `progButton` objects are attached to the pins and the USART held in the attributes.
3. The loco address for PoM messages is set. Since these PoM messages are addressed to a loco, the decoder listens to a loco address derived from its own address: for GBM decoders the RS-Bus address plus 6000, for all other decoders the decoder address plus 7000. As long as the address has not been set, a GBM listens at 6000 and other decoders at 7000. The RS-Bus address used to return PoM feedback (128) is set as well
4. The LED shows the decoder has started: two short flashes, or a slow continuous flash if the decoder address has not been set yet.
5. The decoder address and the command station type (CV27) are passed to `accCmd`, so that received accessory commands are interpreted correctly.
___

## <a name="update"></a>void update(void) ##
Should be called from the main loop, as often as possible.

#### Why "as often as possible" ####
`update()` runs at two different speeds, for a reason. `rsbusHardware.checkPolling()` is called on *every* invocation, because the decoder has to follow the RS-Bus polling cycle closely enough to know which address is being polled at any moment; delaying that check costs feedback messages. Everything else is throttled to once per 20 ms, which is more than fast enough for a button and a LED, and keeps the remaining CPU time available to the sketch.

On every call:
- `rsbusHardware.checkPolling()` — follows and maintains the RS-Bus polling cycle.

Once per 20 ms:
- `rsbusPom.checkConnection()` — transmits any waiting PoM feedback messages over the RS-Bus, using address 128.
- `progButton.checkForNewDecoderAddress()` — checks whether the programming button is pushed.
- `onBoardLed.update()` — maintains the LED flashing pattern.

All three are harmless if the corresponding hardware is not present.
___

## <a name="Processor"></a>The Processor class ##
This class has one function: `reboot()`.

#### void reboot(void) ####
Restarts the decoder using the CV values that are currently in EEPROM. In normal cases the main sketch does not need to call `reboot()` itself: it is called automatically after a new decoder address has been set with the programming button, after the CVs have been restored to their default values, and after the `Restart` CV (CV25) has been written.
