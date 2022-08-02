## Example: ##
```
#include <Arduino.h>
#include <AP_DCC_Decoder_Basic.h>

// RS-Bus: Initialisation of the RS-bus object(s) that send feedback messages.    
RSbusConnection rsbus;               // Per RS-Bus address we need a dedicated object
uint8_t feedbackData = 0;            // The (initial) value we will send over the RS-Bus


//******************************************************************************************************
void setup() {
  // Set CV default values.
  // After cvValues.init() is called, CV default values may be modified using cvValues.defaults[...]
  cvValues.init(SwitchDecoder, 20);
  cvValues.defaults[CmdStation] = Roco;
  decoderHardware.init();
  rsbus.address = cvValues.read(myRSAddr);    // 1.. 127
}


//******************************************************************************************************
unsigned long TLast;                // For testing: we send 1 RS-Bus message per second

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
  // Step 2: Send periodic transmission of RS-Bus messages
  // For testing purposes we send every second 8 bits (2 messages)
  unsigned long TNow = millis();
  if ((TNow - TLast) > 1000) {
    TLast = TNow;
    rsbus.send8bits(feedbackData);   // Tell the library to buffer these 8 bits for later sending
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
  if (rsbus.feedbackRequested) rsbus.send8bits(feedbackData);
  rsbus.checkConnection();
  //
  // Step 4: as frequent as possible we should update the RS-Bus hardware, check if the programming
  // button is pushed, and if the status of the onboard LED should be changed.
  decoderHardware.update();
}
```
