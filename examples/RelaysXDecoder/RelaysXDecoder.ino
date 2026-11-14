//*****************************************************************************************************
//
// File:      RelaysXDecoder.ino
// Author:    Aiko Pras
// History:   2026/11/14 AP Version 1.0
//
// Purpose:   DCC Accessory decoder for a configurable number of mono-stable relays, with optional
//            RS-Bus feedback. Example for the AP_DCC_Decoder_Core library.
//
// Files
// =====
// - RelaysXDecoder.ino: everything related to DCC and RS-Bus (this file)
// - hardware.h:         the number of relays and all pins. Adapt this file to your own board
// - relays.h/.cpp:      switches the relays and holds their state. Knows nothing about DCC or RS-Bus
//
// Decoder addresses
// =================
// Each relay has its own accessory (switch) address: + switches the relay on, - switches it off.
// Since a decoder address holds 4 accessory addresses, the decoder listens to NumberOfRelays / 4
// consecutive decoder addresses. The first decoder address is stored in CV1 / CV9:
// - first decoder address:  turnout 1..4 => relays 1..4
// - second decoder address: turnout 1..4 => relays 5..8
// - ...
// The decoder uses decoder addressing (CV29 bit 6 = 0). Like the original OpenDecoder software,
// the decoder only reacts on activate commands; deactivate commands are ignored.
// After startup all relays are off.
//
// RS-Bus feedback
// ===============
// If CV33 (SendFB) is 1, the state of the relays is sent via the RS-Bus. Each RS-Bus address carries
// 8 bits, as two nibbles of 4 bits. Each nibble holds the state of the 4 relays of a single decoder
// address. Therefore the decoder uses NumberOfRelays / 8 (rounded up) consecutive RS-Bus addresses.
// The first RS-Bus address is stored in CV10 (myRSAddr):
// - first RS-Bus address:  low nibble = relays 1..4,  high nibble = relays 5..8
// - second RS-Bus address: low nibble = relays 9..12, high nibble = relays 13..16
// - ...
// Within a nibble, the first relay is bit 0.
// After a relay has changed, only the nibble that holds this relay is sent (send4bits()).
// After a decoder restart, or after an RS-Bus error, the master asks for all feedback information;
// in that case both nibbles are sent (send8bits()).
// RS-Bus address 128 is reserved for PoM feedback; if the last RS-Bus address would become 128
// or higher, no feedback is sent.
//
// If the board has no RS-Bus, set RsBusRX and RsBusUsart in hardware.h to PIN_UNDEFINED. The RSbus
// library then sends nothing, whatever the value of CV33. Note that the decoder CVs can then not
// be read via PoM, since PoM feedback uses the RS-Bus as well. Writing via PoM, and reading and
// writing via Service Mode (programming track) remain possible.
//
// CVs
// ===
// The decoder type (CV27) is RelaysXDecoder. CV1..CV32 and CV33 (SendFB) are defined by the
// AP_DCC_Decoder_Core library (see CvValues.h). In addition, this decoder uses:
// - CV64 (RelayCount): the number of relays this decoder supports (NumberOfRelays). Read only.
//   It allows configuration software to determine the number of relays. CV64 is written at each
//   startup; writing another value via PoM or SM has no effect after the next restart.
//   Since CV64 is outside the range CV1..CV63, it is not touched by a decoder reset.
//
// LEDs
// ====
// - LedPin:    handled by the AP_DCC_Decoder_Core library (address not set / programming mode)
// - DccLedPin: a DCC message is received
// - AccLedPin: an accessory (switch) command for one of the relays is received
// - RsLedPin:  an RS-Bus feedback message is sent
// The last three LEDs stay on for LedOnTime ms, and use analogWrite() with LedBrightness.
//
// Monitor
// =======
// If DEBUG is defined, information is printed on the Monitor output (see hardware.h).
//
//*****************************************************************************************************
#define DEBUG                         // Comment out for normal operation

#include <Arduino.h>                  // For general definitions
#include <AP_DCC_Decoder_Core.h>      // To include all objects, such as dcc, accCmd, etc.
#include "hardware.h"                 // Number of relays and pins for this board
#include "relays.h"                   // The relays object


const uint8_t SoftwareVersion = 10;   // Version 1.0
const uint8_t RelayCount = 64;        // CV64 (RO): number of relays this decoder supports
const unsigned long LedOnTime = 1000; // Time (in ms) the DCC, ACC and RS LEDs stay on

// The number of decoder addresses and RS-Bus addresses this decoder uses
const uint8_t NumberOfDecoderAddresses = NumberOfRelays / 4;    // 4 relays per decoder address
const uint8_t NumberOfRSAddresses = (NumberOfRelays + 7) / 8;   // 8 relays per RS-Bus address

RSbusConnection rsbus[NumberOfRSAddresses];   // Per RS-Bus address we need a dedicated object
bool sendFeedback;                    // Copy of CV33 (SendFB)

unsigned int firstDecoderAddress;     // Derived from CV1 + CV9
unsigned int lastDecoderAddress;      // firstDecoderAddress + NumberOfDecoderAddresses - 1
uint8_t firstRSAddress;               // CV10 (myRSAddr)
uint8_t lastRSAddress;                // firstRSAddress + NumberOfRSAddresses - 1

DccTimer dccLedTimer;                 // To switch the LED off that signals there is a valid DCC signal
DccTimer accLedTimer;                 // To switch the LED off that signals the DCC command is for me
DccTimer rsLedTimer;                  // To switch the LED off that signals a RS-Bus message is sent


//******************************************************************************************************
void printInfo() {
  if (cvValues.addressNotSet()) Monitor.println("Address not set");
  Monitor.print("SW-release: "); Monitor.println(SoftwareVersion);
  Monitor.print("Number of relays: "); Monitor.println(NumberOfRelays);
  Monitor.print("Decoder addresses: ");
  Monitor.print(firstDecoderAddress); Monitor.print(" - "); Monitor.println(lastDecoderAddress);
  Monitor.print("RS-Bus addresses: ");
  Monitor.print(firstRSAddress); Monitor.print(" - "); Monitor.println(lastRSAddress);
  Monitor.print("Send feedback: "); Monitor.println(sendFeedback);
}


void printDetails(uint8_t number) {
  Monitor.print("Relay: ");
  Monitor.print(number);
  if (accCmd.position == 1) Monitor.println(" +");
    else Monitor.println(" -");
}


//******************************************************************************************************
void ledOn(uint8_t pin, DccTimer &timer) {
  analogWrite(pin, LedBrightness);
  timer.setTime(LedOnTime);
}


void initLeds() {
  // The LED for programming mode (LedPin) is initialised by decoderHardware.init()
  pinMode(DccLedPin, OUTPUT);
  pinMode(AccLedPin, OUTPUT);
  pinMode(RsLedPin, OUTPUT);
  analogWrite(DccLedPin, 0);
  analogWrite(AccLedPin, 0);
  analogWrite(RsLedPin, 0);
}


//******************************************************************************************************
void sendNibble(uint8_t index) {
  // Sends the state of the 4 relays that belong to the nibble with this index.
  // Nibble 0 is the low nibble of the first RS-Bus address, nibble 1 the high nibble, etc.
  uint8_t rsIndex = index / 2;
  if ((index % 2) == 0) rsbus[rsIndex].send4bits(LowBits, relays.nibble(index));
    else rsbus[rsIndex].send4bits(HighBits, relays.nibble(index));
  ledOn(RsLedPin, rsLedTimer);
}


void sendAllNibbles(uint8_t rsIndex) {
  // Sends both nibbles of this RS-Bus address. If NumberOfRelays is not a multiple of 8,
  // the high nibble of the last RS-Bus address does not exist; relays.nibble() then returns 0.
  uint8_t lowNibble = relays.nibble(2 * rsIndex);
  uint8_t highNibble = relays.nibble(2 * rsIndex + 1);
  rsbus[rsIndex].send8bits(lowNibble | (highNibble << 4));
  ledOn(RsLedPin, rsLedTimer);
}


//******************************************************************************************************
void setup() {
  #ifdef DEBUG
    Monitor.begin(115200);
    Monitor.println();
    Monitor.println("Start RelaysXDecoder");
  #endif
  //
  // Step 1: Set CV default values (see CvValues.h for details)
  // Decoder type (DecType) and software version (version) are set using cvValues.init().
  cvValues.init(RelaysXDecoder, SoftwareVersion);
  //
  // Step 2: Use the pins of this board (see hardware.h), and initialise the dcc and rsbus objects.
  // In case the EEPROM wasn't initialised yet, the CV values (1..63) get initialised as well.
  decoderHardware.dccPin       = DccPin;
  decoderHardware.ackPin       = AckPin;
  decoderHardware.ledPin       = LedPin;
  decoderHardware.buttonPin    = ButtonPin;
  decoderHardware.rsBusRX      = RsBusRX;
  decoderHardware.rsBusUsart   = RsBusUsart;
  decoderHardware.swapUsartPin = SwapUsartPin;
  decoderHardware.init();
  //
  // Step 3: Store the number of relays, so that configuration software can read it (RO)
  cvValues.write(RelayCount, NumberOfRelays);
  //
  // Step 4: Listen to NumberOfDecoderAddresses decoder addresses.
  // If the address has not been set, storedAddress() returns 65535. The range then starts above
  // the highest possible decoder address (511), and will never match a received address.
  firstDecoderAddress = cvValues.storedAddress();
  lastDecoderAddress = firstDecoderAddress + NumberOfDecoderAddresses - 1;
  accCmd.setMyAddress(firstDecoderAddress, lastDecoderAddress);
  //
  // Step 5: RS-Bus feedback, using NumberOfRSAddresses RS-Bus addresses.
  // If CV10 is 0 (not set), the RSbus library does not send anything for address 0.
  sendFeedback = cvValues.read(SendFB);
  firstRSAddress = cvValues.read(myRSAddr);
  lastRSAddress = firstRSAddress + NumberOfRSAddresses - 1;
  if (lastRSAddress > 127) sendFeedback = false;    // 128 is reserved for PoM feedback
  for (uint8_t i = 0; i < NumberOfRSAddresses; i++) rsbus[i].address = firstRSAddress + i;
  //
  // Step 6: Initialise the LEDs and the relays
  initLeds();
  relays.init();
  //
  #ifdef DEBUG
    printInfo();
  #endif
}


//******************************************************************************************************
void loop() {
  unsigned int relayNumber;                   // The relay this DCC command is aimed at (1..)
  //
  // Step 1: Check for new DCC messages
  if (dcc.input()) {
    switch (dcc.cmdType) {
      case Dcc::MyAccessoryCmd :
        if ((accCmd.command == Accessory::basic) && (accCmd.activate == 1)) {
          relayNumber = (accCmd.decoderAddress - firstDecoderAddress) * 4 + accCmd.turnout;
          if (relayNumber <= NumberOfRelays) {
            ledOn(AccLedPin, accLedTimer);
            if (relays.set(relayNumber, accCmd.position)) {
              if (sendFeedback) sendNibble((relayNumber - 1) / 4);
            }
            #ifdef DEBUG
              printDetails(relayNumber);
            #endif
          }
        }
        break;
      case Dcc::MyPomCmd :
        cvProgramming.processMessage(Dcc::MyPomCmd);
        break;
      case Dcc::SmCmd :
        cvProgramming.processMessage(Dcc::SmCmd);
        break;
      default:
        break;
    }
    ledOn(DccLedPin, dccLedTimer);
  }
  //
  // Step 2: Turn the LEDs off after LedOnTime
  if (dccLedTimer.expired()) analogWrite(DccLedPin, 0);
  if (accLedTimer.expired()) analogWrite(AccLedPin, 0);
  if (rsLedTimer.expired()) analogWrite(RsLedPin, 0);
  //
  // Step 3: RS-Bus feedback. After a decoder restart or RS-Bus error the master asks for our state.
  if (sendFeedback) {
    for (uint8_t i = 0; i < NumberOfRSAddresses; i++) {
      if (rsbus[i].feedbackRequested) sendAllNibbles(i);
      rsbus[i].checkConnection();
    }
  }
  //
  // Step 4: Update the RS-Bus hardware, the programming button and the onboard LED
  decoderHardware.update();
}
