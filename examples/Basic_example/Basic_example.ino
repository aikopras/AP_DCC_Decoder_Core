//*****************************************************************************************************
//
// File:      Basic_example.ino
// Author:    Aiko Pras
// History:   2021/06/27 AP Version 1.0
//            2021/12/30 AP Version 1.2
//
// Purpose:   DCC Accessory decoder skeleton. Includes RS-Bus feedback and Configuration Variables (CVs)
//            Glues together all common objects, including DCC message processing, RS-Bus feedback,  
//            Configuration Variables, DCC-Ack signal, Programming button and LED.
//
// This basic example prints the contents of new DCC Accessory and PoM commands, and sends a RS-Bus
// feedback message every second. Normal RS-Bus feedback messages may use any address between 1 and 127.
// RS-Bus address 128 is reserved for PoM messages that trigger Pom responses via the RS-Bus.
// Such PoM messages can be used together with dedicated PC/MAC software, such as:
// https://github.com/aikopras/Programmer-Decoder-POM
// https://github.com/aikopras/Programmer-GBM-POM
//           
//******************************************************************************************************
#include <Arduino.h>                  // For general definitions
#include <AP_DCC_Decoder_Basic.h>     // To include all objects, such as dcc, accCmd, etc.

// RS-Bus: Initialisation of the RS-bus object(s) that send feedback messages. Per RS-Bus address a 
// dedicated object is required. The first should use the "base RS-Bus address", as stored in myRSAddr
// (CV10). Subsequent feedback objects should preferably use increasing addresses.   
RSbusConnection rsbus;               // Per RS-Bus address we need a dedicated object
uint8_t feedbackData = 0;            // The (initial) value we will send over the RS-Bus 


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
  // Step 1: Set CV default values (see AP_CV_values.h. for details) 
  // Decoder type (DecType) and software version (version) are set using cvValues.init().
  // After cvValues.init() is called, CV default values may be modified using cvValues.defaults[...]
  // Updated values take effect after setDefaults() is called, either by a long push of the 
  // onboard programming button, or by a CV-POM or CV-SM message to CV8 (VID).
  cvValues.init(SwitchDecoder, 20);
  // cvValues.init(TrackOccupancyDecoder, 20);
  // cvValues.defaults[CmdStation] = OpenDCC;
  //
  // Step 2: initialise the dcc and rsbus objects with addresses and other settings
  decoderHardware.init();
  //
  // Step 3: Initialise the object(s) for RS-Bus feedback messages
  rsbus.address = cvValues.read(myRSAddr);    // 1.. 127
  //
  // Print for testing purposes
  for (uint8_t i = 0; i <= 32; i++) {
    Serial.print("CV"); 
    Serial.print(i); 
    Serial.print(": "); 
    Serial.println(cvValues.read(i)); 
  }
  if (cvValues.addressNotSet()) Serial.println("Address not set");
  Serial.print("Decoder Address: "); 
  Serial.println(cvValues.storedAddress()); 
  Serial.print("RS-Bus Feedback Address: "); 
  Serial.println(rsbus.address); 

}


//******************************************************************************************************
unsigned long TLast;                // For testing: we send 1 RS-Bus message per second

void loop() {
  // Example 1: Print the contents of new DCC Accessory and PoM commands
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
  // Step2: Send periodic transmission of RS-Bus messages
  // For testing purposes we send every second 8 bits (2 messages)
  unsigned long TNow = millis();
  if ((TNow - TLast) > 1000) {
    TLast = TNow;
 //   rsbus.send8bits(feedbackData);   // Tell the library to buffer these 8 bits for later sending
    switch (feedbackData) {
      case 0   : feedbackData = 1;   break;
      case 1   : feedbackData = 2;   break;
      case 2   : feedbackData = 4;   break;
      case 4   : feedbackData = 8;   break;
      case 8   : feedbackData = 16;  break;
      case 16  : feedbackData = 32;  break;
      case 32  : feedbackData = 64;  break;
      case 64  : feedbackData = 128; break;
      case 128 : feedbackData = 1;   break;
     }
  }
  // Step 3: RS-Bus updates
  // As frequent as possible we should check if the RS-Bus asks for the most recent feedback data.
  // This is the case after a decoder restart or after a RS-Bus error. In addition we have to 
  // check if the buffer contains feedback data, and the ISR is ready to send that data via the UART.  
  if (rsbus.feedbackRequested) rsbus.send8bits(feedbackData);
  rsbus.checkConnection();
  //
  // Step 4: as frequent as possible we should update the RS-Bus hardware, check if the programming
  // button is pushed, and if the status of the onboard LED should be changed.
  decoderHardware.update();
}
