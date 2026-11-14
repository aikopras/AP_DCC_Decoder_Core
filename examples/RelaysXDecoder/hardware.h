//*****************************************************************************************************
//
// File:      hardware.h
// Author:    Aiko Pras
// History:   2026/11/14 AP Version 1.0
//
// Purpose:   Hardware specific settings for the RelaysXDecoder example: the number of relays,
//            and the pins for the decoder core, the relays, the LEDs and the Monitor (debug) output.
//
// Pin definitions are given for:
// - DxCore: AVR32DA48 and AVR128DA48. The decoder core pins are taken from boards.h.
// - DxCore: AVR32DA32. This processor is not in boards.h; the decoder core pins are defined here.
// - Arduino UNO / NANO (ATmega328P). The decoder core pins are taken from boards.h.
// For other processors, add a new block below.
//
// Note that boards.h (included via AP_DCC_Decoder_Core.h) defines global constants named dccPin,
// ackPin, ledPin, ... Therefore the names in this file start with a capital: DccPin, AckPin, ...
//
// Requirements for the pins
// =========================
// Decoder core (DccPin, AckPin, LedPin, ButtonPin, RsBusRX, RsBusUsart):
//   See boards.h and CommonFunctions.md of the AP_DCC_Decoder_Core library. In short:
//   - DccPin must be interrupt capable (on the ATmega328P: INT0 or INT1).
//   - RsBusRX must be interrupt capable. On 28/32 pin DxCore processors the RSbus library uses
//     the RTC, which requires PIN_PA0 (EXTCLK).
//   - RsBusUsart is the number of the USART that transmits RS-Bus data.
//   - If the decoder has no RS-Bus, set RsBusRX and RsBusUsart to PIN_UNDEFINED.
//   - If the decoder has no onboard LED or button, set LedPin or ButtonPin to PIN_UNDEFINED.
//
// Relays (RelayPins[]):
//   Any digital output pin. HIGH switches the relay on.
//
// LEDs (DccLedPin, AccLedPin, RsLedPin):
//   The LEDs are controlled via analogWrite(), using LedBrightness. On a pin without PWM,
//   analogWrite() switches the pin LOW for values below 128, and HIGH otherwise. Thus:
//   - With an LedBrightness below 255, the LED pins must be PWM pins.
//   - With an LedBrightness of 255, any digital output pin can be used.
//   The PWM pins may not depend on a timer that is already used by other software:
//   - ATmega328P: Timer2 is used by the AP_DCC_library, so pin 3 and 11 give no PWM.
//   - DxCore: TCB0 is used by the AP_DCC_library, and DxCore uses (by default) TCB2 for millis().
//     Therefore use the pins of TCA0 or TCD0. Note that DxCore connects TCA0 by default to PORTD
//     on 28/32 pin processors, but to PORTC on 48/64 pin processors.
//
// Monitor (debug) output:
//   Only used if DEBUG is defined in the main sketch. On the ATmega328P there is only one USART,
//   which is also used for RS-Bus transmission. Therefore DEBUG and RS-Bus can not be used together
//   on an UNO or NANO.
//
//*****************************************************************************************************
#pragma once
#include <Arduino.h>
#include <AP_DCC_Decoder_Core.h>      // For boards.h and PIN_UNDEFINED


//*****************************************************************************************************
// Number of relays. Must be a multiple of 4, since each decoder address controls 4 relays.
// The number of decoder addresses and RS-Bus addresses are derived from this value.
const uint8_t NumberOfRelays = 8;

// Brightness of the DCC, ACC and RS LEDs (1..255). See above for the requirements.
const uint8_t LedBrightness = 16;


//*****************************************************************************************************
#if defined(__AVR_AVR32DA48__) || defined(__AVR_AVR128DA48__)
  // Decoder core: taken from boards.h
  const uint8_t RsBusUsart = rsBusUsart;  // PIN_PA4 / Alternative Pin
  const bool SwapUsartPin = swapUsartPin; // Use the alternative USART pin
  const uint8_t RsBusRX = rsBusRX;        // PIN_PA0
  const uint8_t DccPin = dccPin;          // PIN_PA1
  const uint8_t AckPin = ackPin;          // PIN_PA2
  const uint8_t ButtonPin = buttonPin;    // PIN_PA3
  const uint8_t LedPin = ledPin;          // PIN_PB0 / onboard LED, to indicate programming mode

  // LEDs (TCA0 is connected to PORTC)
  const uint8_t DccLedPin = PIN_PC2;      // indicates a valid DCC signal
  const uint8_t AccLedPin = PIN_PC3;      // an accessory (switch) command for this decoder is received
  const uint8_t RsLedPin = PIN_PC4;       // a RS-Bus feedback message is sent

  // Relays
  const uint8_t RelayPins[] = {
    PIN_PD0, PIN_PD1, PIN_PD2, PIN_PD3,   // relays 1..4
    PIN_PD4, PIN_PD5, PIN_PD6, PIN_PD7    // relays 5..8
  };

  // Monitor: USART1, TX = PIN_PC0, RX = PIN_PC1
  #define Monitor Serial1


//*****************************************************************************************************
#elif defined(__AVR_AVR32DA32__)
  // Decoder core: not in boards.h, thus defined here
  const uint8_t RsBusUsart = 0;           // PIN_PA4 / Alternative Pin
  const bool SwapUsartPin = true;         // Use the alternative USART pin
  const uint8_t RsBusRX = PIN_PA0;        // PIN_PA0 (EXTCLK, required by the RSbus library)
  const uint8_t DccPin = PIN_PA1;         // PIN_PA1
  const uint8_t AckPin = PIN_PA2;         // PIN_PA2
  const uint8_t ButtonPin = PIN_PA3;      // PIN_PA3
  const uint8_t LedPin = PIN_PA7;         // PIN_PA7 / onboard LED, to indicate programming mode

  // LEDs (TCA0 is connected to PORTD)
  const uint8_t DccLedPin = PIN_PD0;      // indicates a valid DCC signal
  const uint8_t AccLedPin = PIN_PD1;      // an accessory (switch) command for this decoder is received
  const uint8_t RsLedPin = PIN_PD2;       // a RS-Bus feedback message is sent

  // Relays
  const uint8_t RelayPins[] = {
    PIN_PD3, PIN_PD4, PIN_PD5, PIN_PD6,   // relays 1..4
    PIN_PD7, PIN_PF2, PIN_PF3, PIN_PF4    // relays 5..8
  };

  // Monitor: USART1, TX = PIN_PC0, RX = PIN_PC1
  #define Monitor Serial1


//*****************************************************************************************************
#elif defined(__AVR_ATmega328P__)
  // Decoder core: taken from boards.h (Arduino UNO / NANO with the DCC extension shield)
  const uint8_t RsBusUsart = rsBusUsart;  // USART0 (TXD = pin 1)
  const bool SwapUsartPin = swapUsartPin; // Use the standard USART pin
  const uint8_t RsBusRX = rsBusRX;        // pin 2 (INT0)
  const uint8_t DccPin = dccPin;          // pin 3 (INT1)
  const uint8_t AckPin = ackPin;          // pin 7
  const uint8_t ButtonPin = buttonPin;    // pin 6
  const uint8_t LedPin = ledPin;          // pin 4 / onboard LED, to indicate programming mode

  // LEDs (pin 5 = Timer0, pin 9 and 10 = Timer1)
  const uint8_t DccLedPin = 5;            // indicates a valid DCC signal
  const uint8_t AccLedPin = 9;            // an accessory (switch) command for this decoder is received
  const uint8_t RsLedPin = 10;            // a RS-Bus feedback message is sent

  // Relays
  const uint8_t RelayPins[] = {
    8, 12, A0, A1,                        // relays 1..4
    A2, A3, A4, A5                        // relays 5..8
  };

  // Monitor: USART0, which is also used by the RS-Bus! See above.
  #define Monitor Serial


//*****************************************************************************************************
#else
  #error "No pins defined for this processor. Add a block for this processor to hardware.h"
#endif


//*****************************************************************************************************
static_assert((NumberOfRelays % 4) == 0, "NumberOfRelays must be a multiple of 4");
static_assert(sizeof(RelayPins) == NumberOfRelays, "RelayPins[] must hold NumberOfRelays pins");
