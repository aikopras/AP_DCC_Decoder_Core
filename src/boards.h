//******************************************************************************************************
//
// file:      boards.h
// Author:    Aiko Pras
// purpose:   Defines which USART and which Arduino pins are used for DCC input, DCC-Ack,
//            RS-Bus output and the programming button.
//
// History:   2021/08/01 AP Version 1.1
//            2021/12/30 AP Version 1.2
//
// The code is designed to run on traditional Arduino boards, such as the Nano, Uno and Mega,
// as well as MiniCore, MightyCore, MegaCore, MegaCoreX and DxCore boards. It has been tested with
// traditonal ATMega processors, such as the 16A, 328 and 2560, as well as more modern ATMegaX
// processors, such as the 4808, 4809 and AVR128DA48.
//
// Defines the Arduino pins and the USART number for the following decoder hardware
// - dccPin           - Any Arduino pin that supports hardware interrupts
// - rsBusRX          - Any Arduino pin that supports hardware interrupts
// - rsBusUsart       - The number of the USART being used (0, 1, ...)
// - ledPin           - Any Arduino digital pin
// - buttonPin        - Any Arduino digital pin
// - ackPin           - Any Arduino digital pin
// - swapUsartPin     - MegaX processors allow the USART to be connected to alternative pins
//
// Note that the AP_DCC_library uses Timer2 (8bit) or one of the TCBs
// Make sure the right clock-speed (F_CPU) is selected in the Arduino "Tools->Clock" menu
//
//******************************************************************************************************
#pragma once
#include <Arduino.h>

#if defined(MIGHTYCORE) && defined(__AVR_ATmega16__)
// This is the ATMega 16 board, as used by the following Mightycore boards:
// https://easyeda.com/aikopras/gbm-smd
// https://easyeda.com/aikopras/gbm-eagle
// https://easyeda.com/aikopras/vitrine-decoder
// https://easyeda.com/aikopras/servo-decoder-tht
// https://easyeda.com/aikopras/switch-decoder
// https://easyeda.com/aikopras/relays-4-decoder
// https://easyeda.com/aikopras/relays-16-decoder
// https://easyeda.com/aikopras/watchdog-decoder
const uint8_t rsBusUsart = 0;         // PIN_PD1 (8) Must be TXD0
const bool swapUsartPin = false;      // Use the standard USART pin
const uint8_t rsBusRX = PIN_PD2;      // PIN_PD2 (10) Must be on INT0
const uint8_t dccPin = PIN_PD3;       // PIN_PD3 (11) Must be on INT1
const uint8_t ledPin = PIN_PD0;       // PIN_PD0 (8)
const uint8_t buttonPin = PIN_PD6;    // PIN_PD6 (14)
const uint8_t ackPin = PIN_PD7;       // PIN_PD7 (15)

#elif defined(MEGACORE)
// This is the Megacore Lift decoder board
//https://easyeda.com/aikopras/support-lift-controller
const uint8_t rsBusUsart = 1;         // PIN_PD3 (46) is TXD1 and Digital Pin 18 on the ATMega2560
const bool swapUsartPin = false;      // Use the standard USART pin
const uint8_t rsBusRX = PIN_PD2;      // PIN_PD2 (45) is INT2 and Digital Pin 19 on the ATMega2560
const uint8_t dccPin = PIN_PD1;       // PIN_PD1 (44) is INT1 and Digital Pin 20 on the ATMega2560
const uint8_t ledPin = PIN_PB7;       // PIN_PB7 (26) is Digital Pin 13 on the ATMega2560 (Led Red)
const uint8_t buttonPin = PIN_PD7;    // PIN_PD7 (50) is Digital Pin 38 on the ATMega2560
const uint8_t ackPin = PIN_PH5;       // PIN_PH5 (17) is Digital Pin 8 on the ATMega2560

#elif defined(NANO_4808_PINOUT)
// This is the Nano Thinary, using the MegaCoreX board layout
// The onboard USB connector is Serial
const uint8_t rsBusUsart = 0;         // PIN_PA4 (6)
const bool swapUsartPin = true;       // Use the alternative USART pin
const uint8_t rsBusRX = PIN_PA0;      // PIN_PA0 (2) We use the RTC
const uint8_t dccPin = PIN_PA1;       // PIN_PA1 (3)
const uint8_t ackPin = PIN_PA2;       // PIN_PA2 (4)
const uint8_t buttonPin = PIN_PA3;    // PIN_PA3 (5)
const uint8_t ledPin = PIN_PC2;       // PIN_PC2 (13)

#elif defined(NONA4809_PINOUT)
// This is the Nano Every, using the MegaCoreX board layout
// MegaCoreX remaps Serialx to different USARTS.
// Therefore use the description of Ports, instead of Serialx
// The onboard USB connector is Serial
const uint8_t rsBusUsart = 2;         // PIN_PF4 (6)
const bool swapUsartPin = true;       // Use the alternative USART pin
const uint8_t rsBusRX = PIN_PA0;      // PIN_PA0 (2)
const uint8_t dccPin = PIN_PA1;       // PIN_PA1 (7)
const uint8_t ackPin = PIN_PE0;       // PIN_PE0 (11)
const uint8_t buttonPin = PIN_PE1;    // PIN_PE1 (12)
const uint8_t ledPin = PIN_PE2;       // PIN_PE2 (13)

#elif defined(DX_28_PINS)
// This is the servo decoder, using the Dx board layout
// The onboard USB connector is Serial1
const uint8_t rsBusUsart = 0;         // PIN_PA4 / Alternative Pin
const bool swapUsartPin = true;       // Use the alternative USART pin
const uint8_t rsBusRX = PIN_PA0;      // PIN_PA0
const uint8_t dccPin = PIN_PD0;       // PIN_PD0
const uint8_t ackPin = PIN_PD1;       // PIN_PD1
const uint8_t buttonPin = PIN_PD2;    // PIN_PD2
const uint8_t ledPin = PIN_PA7;       // PIN_PA7 / on-board LED

#elif defined(DA_48_PINS)
// This is the AVR128DA48 Curiosity Nano, using the Dx board layout
// The onboard USB connector is Serial1
const uint8_t rsBusUsart = 0;         // PIN_PA4 / Alternative Pin
const bool swapUsartPin = true;       // Use the alternative USART pin
const uint8_t rsBusRX = PIN_PA0;      // PIN_PA0
const uint8_t dccPin = PIN_PA1;       // PIN_PA1
const uint8_t ackPin = PIN_PA2;       // PIN_PA2
const uint8_t buttonPin = PIN_PA3;    // PIN_PA3
const uint8_t ledPin = PIN_PC6;       // PIN_PC6 / on-board LED, negative logic

#elif defined(DA_64_PINS)
// This is the AVR64DA64 24 Channel IO Board for the TMC
// The onboard USB connector is Serial
const uint8_t rsBusUsart = 2;         // PIN_PF0
const bool swapUsartPin = false;      // Use the default USART pin
const uint8_t rsBusRX = PIN_PF2;      // PIN_PF2
const uint8_t dccPin = PIN_PG6;       // PIN_PG6
const uint8_t ackPin = PIN_PG7;       // PIN_PG7
const uint8_t buttonPin = PIN_PG2;    // PIN_PG2
const uint8_t ledPin = PIN_PA7;       // PIN_PA7 / LED Builtin

#elif defined(__AVR_ATmega2560__)
// This is for standard Arduino Mega boards
const uint8_t rsBusUsart = 1;         // PIN_PD3 (46) is TXD1 and Digital Pin 18 on the ATMega2560
const bool swapUsartPin = false;      // Use the standard USART pin
const uint8_t rsBusRX = 19;           // PIN_PD2 (45) is INT2 and Digital Pin 19 on the ATMega2560
const uint8_t dccPin = 20;            // PIN_PD1 (44) is INT1 and Digital Pin 20 on the ATMega2560
const uint8_t ledPin = 13;            // PIN_PB7 (26) is Digital Pin 13 on the ATMega2560 (Led Red)
const uint8_t buttonPin = 38;         // PIN_PD7 (50) is Digital Pin 38 on the ATMega2560
const uint8_t ackPin = 8;             // PIN_PH5 (17) is Digital Pin 8 on the ATMega2560

#elif defined(__AVR_ATmega328P__)
// This is for standard Arduino UNO boards, with the DCC extension shield
// https://easyeda.com/aikopras/arduino-uno-dcc-shield
const uint8_t rsBusUsart = 0;         // Must be TXD
const bool swapUsartPin = false;      // Use the standard USART pin
const uint8_t rsBusRX = 2;            // Must be INT0
const uint8_t dccPin = 3;             // Must be INT1
const uint8_t ledPin = 4;             // Pin 4 is on the DCC Board; PIN 13 is the internal UNO LED
const uint8_t buttonPin = 6;          // Digital Pin 6 on the Arduino UNO
const uint8_t ackPin = 7;             // Digital Pin 7 on the Arduino UNO

#elif defined(AVR_XMEGA) || !defined(MEGACOREX)
// This is likely the Nano Every, using the "standard" Arduino megaAVR board
// The onboard USB connector is Serial
// Make sure to choose Register Emulation = None
  #if defined(AVR_NANO_4809_328MODE)
    #error "In the IDE under the Tools dropdown menu, turn of Register emulation: ATMEGA328"
  #endif
const uint8_t rsBusUsart = 2;         // PIN_PF4 (6)
const bool swapUsartPin = true;       // Use the alternative USART pin
const uint8_t rsBusRX = 2;            // PIN_PA0 (2)
const uint8_t dccPin = 7;             // PIN_PA1 (7)
const uint8_t ackPin = 11;            // PIN_PE0 (11)
const uint8_t buttonPin = 12;         // PIN_PE1 (12)
const uint8_t ledPin = 13;            // PIN_PE2 (13)

#else
  #error Code will not run on this board
  //
  // THE PIN NUMBERS BELOW ARE EQUIVALENT TO THE UNO, AND HAVE BEEN INCLUDED TO AVOID COMPILER ERRORS
const uint8_t rsBusUsart = 0;         //
const bool swapUsartPin = false;      //
const uint8_t rsBusRX = 2;            //
const uint8_t dccPin = 3;             //
const uint8_t ledPin = 4;             //
const uint8_t buttonPin = 6;          //
const uint8_t ackPin = 7;             //

#endif
