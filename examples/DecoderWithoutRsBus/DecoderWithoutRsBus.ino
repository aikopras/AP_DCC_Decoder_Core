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
          else Serial.print(" Extended accessory command for my decoder address: ");
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
