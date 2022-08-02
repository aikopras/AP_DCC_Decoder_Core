//*****************************************************************************************************
//
// File:      AP_DCC_Decoder_Core.h
// Author:    Aiko Pras
// History:   2021/06/14 AP Version 1.0
//            2021/12/30 AP Version 1.2
//            2022/08/02 AP Version 1.3 Restructure of the library
// Purpose:   Header file for accessory decoders that use the RS-Bus for feedback.
//            PoM feedback messages use instead of RailCom the RS-Bus with address 128.
//
// This library takes care of many of the common accesory decoder functions, such as:
// - accessing the CV values,
// - instantiating the onboard LED and Button,
// - filling the EEPROM with initial values,
// - resetting the EEPROM with initial values,
// - rebooting the decoder,
// - reacting on general CV operations (such as reset decoder, find decoder, reboot decoder),
// - initialisation and the initial assignment of the decoder address.
//
// Main => AP_Accessory_Common +-> AP_CvValues        (To access the CV EEPROM values)
//                             +-> AP_DCC_Lib         (Needed to detach the ISR)
//                             +-> RSbus_Lib          (Needed to detach the ISR)
//                             +-> AP_LED_Lib         (the LED object is instatiated here)
//                             +-> AP_Button_Lib      (the Button object is instatiated here)
//
//*****************************************************************************************************
#pragma once

#include <Arduino.h>                  // For general definitions
#include <AP_DCC_library.h>           // Interface to DCC input and DCC Ack pin
#include <RSbus.h>                    // Interface to the Lenz RS-bus
#include <AP_DCC_Button.h>            // For the onboard Button
#include "boards.h"                   // Pins and USART being used for the various boards
#include "AP_DCC_LED.h"               // For the onboard LED
#include "CvValues/CvValues.h"      // To define and access cvValue


//*****************************************************************************************************
class Processor {
  public:
    void reboot(void);                            // Restarts the decoder, using the latest EEPROM CV values
};


class CvProgramming {
  public:
    void initPoM(void);                           // Set the Loco address for PoM messages and the RS-Pom address
    void processMessage(Dcc::CmdType_t cmdType);  // Called if we have a PoM or SM message

  private:
    void pom_feedback(void);                      // Sends a PoM feedback message
    bool LedShouldFlash;                          // Local copy of CV23 (search)
};


class CommonDecHwFunctions {
  public:
    void init(void);                              // Should be called from init() in the main sketch.
    void update(void);                            // Should be called from main as often as possible.
    unsigned long TLast;                          // Some elements of update() are called every 20ms
};


//*****************************************************************************************************
// Objects defined the DCC_Library, RS-Bus library as well as AP_Accessory_Common.cpp
//*****************************************************************************************************
// A number of hardware related objects are instantiated in their libraries, and must be made known to
// the compiler and linker by 'repeating" their instantiation again, preceeded by the extern keyword
// Since the main sketch includes this header file, main does not need to repeat these again.

// The following objects are used for decoding received DCC messages. The main object is dcc,
// which manages the layer 1 (hardware) specific parts of the DCC interface. The other objects make
// available to the main sketch the contents of respectively accessory and loco (multi-function) commands.
extern Dcc dcc;                              // Instantiated in AP_DCC_library.cpp
extern Accessory accCmd;                     // Instantiated in AP_DCC_library.cpp
extern Loco locoCmd;                         // Instantiated in AP_DCC_library.cpp

// The following objects are used for storing and retrieving CV values from EEPROM,
// and respond to DCC CV-access commands
extern CvValues cvValues;                    // Instantiated in main sketch
extern CvProgramming cvProgramming;          // Instantiated in AP_Accessory_Common.cpp
extern CvAccess cvCmd;                       // Instantiated in AP_DCC_library.cpp

// The following objects provide access to the onboard LED, as well as the
// onboard programming button
extern DCC_Led onBoardLed;                   // Instantiated in AP_DCC_Decoder_Core.cpp
extern CommonDecHwFunctions decoderHardware; // Instantiated in AP_DCC_Decoder_Core.cpp
