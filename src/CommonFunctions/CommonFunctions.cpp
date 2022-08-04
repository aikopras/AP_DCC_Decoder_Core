//*****************************************************************************************************
//
// File:      CommonFunctions.cpp
// Author:    Aiko Pras
// History:   2021/06/14 AP Version 1.0
//            2021/12/30 AP Version 1.2
//            2022/08/02 AP Version 1.3
//
// Purpose:   C++ file that implements the methods to act on DCC CV-messages and pushes on the 
//            programming button. It can send RS-Bus messages and make changes to the LED.
//           
//*****************************************************************************************************
#include "AP_DCC_Decoder_Core.h"      // Header file for this C++ file

class ProgButton {
  public:
    void attach(uint8_t pin);                     // Attach the onboard programming button
    void checkForNewDecoderAddress(void);         // Is the onboard programming button pushed?
    void addressProgramming(void);                // Store the new decoder / RS-Bus address in EEPROM
};


//*****************************************************************************************************
// Objects instatiated / used in this library
//*****************************************************************************************************
// For the RS-Bus feedback channel two objects are used by the core functions.
// The first object manages the layer 1 (hardware) specific parts of the RS-Bus interface.
// This object, rsbusHardware, is defined and instatiated in the RS-bus library (rs_bus.cpp).
extern RSbusHardware rsbusHardware;   // Instantiated in rs_bus.cpp
// The second object manages POM feedback messages, using RS-Bus address 128.
// The RSbusConnection class is defined in the RSbus library
RSbusConnection rsbusPom;             // Used for feedback on PoM messages (RS-bus address 128)

DccLed onBoardLed;                    // The DccLed class is defined in the AP_DccLED Library
DccButton onBoardButton;              // The DccButton class is defined in the AP_DccButton Library

// Classes defined in here
ProgButton progButton;                // The onBoardButton is used as programming button
CvProgramming cvProgramming;          // For actions after a CV-access commandis received
Processor processor;                  // Instantiate the Processor's object, which is used for reboot
CommonDecHwFunctions decoderHardware; // Common inits(), and update of the LED and RS-Bus interfaces


//*****************************************************************************************************
// Reset the software / Reboot
//*****************************************************************************************************
void Processor::reboot(void) {
  // Restarts the program from the beginning
  // We first need to disable the DCC and RS-Bus interrupts. If we wouldn't disable them, an external
  // event will cause a jump to the ISR. After an ISR is ready, it takes the return address and returns
  // to the calling routine. It is probably this bahavior that causes problems with interrupts if we
  // don't detach them before we jump to the reset vector at address 0.
  // Note that various Internet fora propose using the Watchdog Timer (see AVR data sheets) to do a
  // software reset. Using WDT has as advantage that all IO Registers will be set to their initial
  // value, but requires the use of (optiboot) bootloaders or special code during initialization.
  // A JMP to zero seems much simpler.
  noInterrupts();
  dcc.detach();
  rsbusHardware.detach();
  asm volatile("  jmp 0");
  interrupts();
}


//*****************************************************************************************************
// Programming button 
//*****************************************************************************************************
// The ProgButton class is basically a small wrapper around the DccButton class (see AP_DccButton)
void ProgButton::attach(uint8_t pin) {
  onBoardButton.attach(pin);
  delay(500);
  onBoardButton.read();
}


void ProgButton::checkForNewDecoderAddress() {
  // Check if the decoder programming button is pushed
  // If the button is pushed for 5 seconds, restore all EEPROM data with default values.
  // If it is just pushed shortly, enter address programming
  onBoardButton.read();
  if (onBoardButton.isPressed()) onBoardLed.turn_on();
  if (onBoardButton.pressedFor(5000)) {
      onBoardLed.turn_off();
      cvValues.setDefaults();
      delay(500);
      processor.reboot();
  }    
  if (onBoardButton.wasReleased()) addressProgramming();
}


void ProgButton::addressProgramming() {
  // Set the decoder addresses:
  // CV1/CV9 (myAddrL/myAddrH): We store the output or decoder address, except for GBMs 
  // CV10 (myRSAddr): For GBM decoders we store the output address, for others the decoder address.
  onBoardLed.flashFast();
  do {
    onBoardLed.update();
    if (dcc.input()) {
      uint8_t cv29 = cvValues.read(Config);
      bool accDecoder = bitRead(cv29,7);       // Are we an accessory decoder?
      bool outputAddr = bitRead(cv29,6);       // Do we want output (or decoder) addressing?
      // Only act if we are an accessory decoder and receive an accessory commands 
      if (accDecoder && ((dcc.cmdType == Dcc::MyAccessoryCmd) || (dcc.cmdType == Dcc::AnyAccessoryCmd))) {  
        // Step 1: Store the Output address or the Decoder address, unless we have a GBM decoder
        if (!cvValues.isGBM()) {      
          // According to RCN213, for the first handheld address (switch = 1) CV1 should become 1.
          // - the valid range for CV1 is 1..63 (if CV9 == 0) or 0..63 (if CV9 !=0)
          // - the valid range for CV9 is 0..3  (or 128, if the decoder has not been initialised)
          if (outputAddr) {
            // Store the output address:
            // The range of the received output address is 1..1024 (LZV100) / 1..2048 (NMRA)
            uint8_t my_cv1 =  (accCmd.outputAddress & 0b11111111);
            uint8_t my_cv9 = ((accCmd.outputAddress >> 8) & 0b00000111);
            cvValues.write(myAddrL, my_cv1);
            cvValues.write(myAddrH, my_cv9);
          }      
          else {
            // Store the decoder address:
            // The range of the received decoder address is 0..255 (LZV100) / 511 (NMRA)
            // We therefore have to add 1 
            uint16_t tempAddress = accCmd.decoderAddress + 1;
            uint8_t my_cv1 =  (tempAddress & 0b00111111);
            uint8_t my_cv9 = ((tempAddress >> 6) & 0b00000111);
            cvValues.write(myAddrL, my_cv1);
            cvValues.write(myAddrH, my_cv9);
          }
        }
        // Step 2: Set the RS-bus address (1..127, 0 = undefined, 128 = PoM feedback)
        if (cvValues.isGBM()) { 
          if (accCmd.outputAddress < 128) cvValues.write(myRSAddr, accCmd.outputAddress);
          else cvValues.write(myRSAddr, 0);
        }
        else {
          // We store the decoder address, even if we use output addressing!
          if (accCmd.decoderAddress < 128) {
            cvValues.write(myRSAddr, accCmd.decoderAddress + 1);

          }
          else cvValues.write(myRSAddr, 0);         
        }
        delay(100);
        processor.reboot(); // we got reprogrammed -> forget everthing running and restart decoder!
      }
    }
    onBoardButton.read();
  } while (!onBoardButton.isPressed());
  // Button was pushed again, but no DCC accessory decoder message was received.
  // No need to reboot().
  onBoardLed.turn_off();
  delay(500);
  onBoardButton.read();;
}


//*****************************************************************************************************
// CvProgramming 
//*****************************************************************************************************
void CvProgramming::initPoM() {
  // For CV PoM (as well as F1..F4) messages we also listen to a loco address.   
  // This loco address is equal to the RS-bus address plus an offset.
  // For GBM decoders this offset is 6000, for other decoders 7000.
  // If the decoder address is not programmed, the decoder listens for PoM messages at 'pomAddress'.
  const int offset_gbm = 6000;
  const int offset_other = 7000;
  uint16_t pomAddress;
  if (cvValues.isGBM()) {  
    if (cvValues.addressNotSet()) {pomAddress = offset_gbm;}
    else {pomAddress = offset_gbm + cvValues.read(myRSAddr);}
    }
  else {
    if (cvValues.addressNotSet()) {pomAddress = offset_other - 1;}
    else {pomAddress = offset_other + cvValues.storedAddress();} 
  }
  locoCmd.setMyAddress(pomAddress);
  // Feedback for CV PoM (verify byte) messages will be send via RS-bus address 128
  rsbusPom.address = 128;        // 1.. 128
}       


//*****************************************************************************************************
// PoM Feedback
//*****************************************************************************************************
void CvProgramming::pom_feedback(void) {
  // According to the NMRA standards the verify byte command checks if there is a match between the 
  // value in the PoM command and the value stored in the decoder. Such behavior is useful for Service 
  // Mode Programming, but not for PoM. However, since we can send information back via the RS-bus, 
  // we modify this behavior and send the byte value stored in the decoder back.
  // Note that all CV values can be retrieved from EEPROM, except:
  // - CV23 (Search function which blinks led)
  // - CV26 (DccQuality)
  // - CV31 (ParityErrors)
  // - CV32 (PulseErrors)
  unsigned int RecCvNumber = cvCmd.number;
  uint8_t CurrentEEPROMValue = cvValues.read(RecCvNumber);
  if      (RecCvNumber == Search) rsbusPom.send8bits(LedShouldFlash);
  else if (RecCvNumber == DccQuality) rsbusPom.send8bits(dcc.errorXOR);
  else if (RecCvNumber == ParityErrors) rsbusPom.send8bits(rsbusHardware.parityErrors);
  else if (RecCvNumber == PulseErrors) rsbusPom.send8bits(rsbusHardware.pulseCountErrors);
  else rsbusPom.send8bits(CurrentEEPROMValue);
}


//*****************************************************************************************************
// CvProgramming::processMessage
//*****************************************************************************************************
void CvProgramming::processMessage(Dcc::CmdType_t cmdType) {
  // Create some local variables 
  unsigned int RecCvNumber = cvCmd.number;
  uint8_t RecCvData = cvCmd.value;
  uint8_t CurrentEEPROMValue = cvValues.read(RecCvNumber);
  bool SM  = (cmdType == Dcc::SmCmd);
  bool PoM = (cmdType == Dcc::MyPomCmd);
  // Ensure we stay within the range supported by this decoder
  if (RecCvNumber < max_cvs) {
    switch(cvCmd.operation) {
      case CvAccess::verifyByte :
        if (SM) {
          // In SM we send back a DCC-ACK signal
          // if the value of the received byte matches the CV value in EEPROM
          if (CurrentEEPROMValue == RecCvData) {dcc.sendAck();}
        }
        if (PoM) {
          // In PoM mode a railcom reply should be returned, but since we don't support Railcom
          // we send back a RS-Bus reply instead
          pom_feedback();
        }
      break;
      case CvAccess::writeByte :
        // A number of CVs have a special meaning, and can not directly be written
        switch (RecCvNumber) {
          case version:
            // CV7 (version): should not be writeable
          break;
          case VID: 
            //CV8 (VID): Reset decoder data to initial values if we'll write to CV8 the value 0x0D
            if (RecCvData == 0x0D) {
              cvValues.setDefaults();
              if (SM) dcc.sendAck();
              processor.reboot();
            }
          break;
          case Restart:
            // CV25: Restart the decoder if we write a value of 1 or higher, but do not reset the EEPROM data (cvValues)
            // Use this function after PoM has changed CV values and new values should take effect now
            if (RecCvData) processor.reboot();
          break;
          case Search:
            // Search function: blink the decoder's LED if CV23 is set to 1. 
            // Continue blinking until CV23 is set to 0
            if (RecCvData) {
              LedShouldFlash = true; 
              onBoardLed.flashFast();
            } 
            else {
              LedShouldFlash = false; 
              onBoardLed.turn_off();
            }
          break;
          default:
            cvValues.write(RecCvNumber, RecCvData);
            if (SM) dcc.sendAck();
          break;
        }
      break;
      case CvAccess::bitManipulation :
      // Note: CV Bit Operation is only implemented for Service Mode (not for PoM)
        if (cvCmd.writecmd) {
          uint8_t NewEEPROMValue = cvCmd.writeBit(CurrentEEPROMValue);
          cvValues.write(RecCvNumber, NewEEPROMValue);
          if (SM) dcc.sendAck();
        }
        else { // verify if bits are equal
          if (cvCmd.verifyBit(CurrentEEPROMValue)) {
            if (SM) dcc.sendAck();
          }
        }
      break;
      default:
      break;
    }
  }
}


//*****************************************************************************************************
// Common functions for the Decoder hardware (DCC, RS-Bus, LED, Button)
//*****************************************************************************************************
void CommonDecHwFunctions::init(void) {
  // Should be called from setup() in the main sketch.
  // Initialise the EEPROM (cvValues) if it has been erased. 
  if (cvValues.notInitialised()) cvValues.setDefaults();
  // attach input pins to the objects below 
  dcc.attach(dccPin, ackPin);
  rsbusHardware.swapUsartPin = swapUsartPin;
  rsbusHardware.attach(rsBusUsart, rsBusRX);
  onBoardLed.attach(ledPin);
  progButton.attach(buttonPin);
  // Set the Loco address for PoM messages and the RSBus PoM Feedback address 
  cvProgramming.initPoM();
  // Light the LED to indicate the decoder has started and if the address is set
  onBoardLed.start_up();
  if (cvValues.addressNotSet()) onBoardLed.flashSlow();
  else onBoardLed.start_up();
  // Set the Accessory address and the type of Command Station
  accCmd.setMyAddress(cvValues.storedAddress());
  accCmd.myMaster = cvValues.read(CmdStation);
  // initializes the 20ms timer that reduces the CPU load of update()
  TLast = millis();
}  


void CommonDecHwFunctions::update(void) {
  // Should be called from main as often as possible.
  // checkPolling() is called as often as possible. The  others only every 20ms
  rsbusHardware.checkPolling();             // Control and maintain the RS-bus polling cycles  
  unsigned long TNow = millis();            // millis() is expensive, so call it only once
  if ((TNow - TLast) >= 20) {               // 20ms passed?
    TLast = TNow;
    rsbusPom.checkConnection();             // Check RS-Bus buffer for address 128 (PoM messages)
    progButton.checkForNewDecoderAddress(); // Is the decoder programming button pushed?
    onBoardLed.update();                    // Control LED flashing
  }
}                            
