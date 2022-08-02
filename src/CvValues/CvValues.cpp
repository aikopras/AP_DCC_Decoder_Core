//*****************************************************************************************************
//
// File:      CvValues.cpp
// Author:    Aiko Pras
// History:   2021/06/14 AP Version 1.0
//            2021/12/30 AP Version 1.2
//            2022/07/13 AP Version 1.3 Safety decoder CV naming changed
//            2022/08/02 AP Version 1.4 Restructure of the library
//
// Purpose:   C++ file that implements the methods to read and modify CV values stored in EEPROM,  
//            as well as the default values for all CVs.
//
//*****************************************************************************************************
#include <Arduino.h>
#include <EEPROM.h>
#include "CvValues.h"


//*****************************************************************************************************
// Relation between default values and CVs
//
//                   RAM                            EEPROM
//                 defaults                           CVs
//                +--------+                      +--------+
//                |        |                      |        |
// constructor -> |        |     setDefaults()    |        | -> init()
//                |        |  ------------------> |        |
//     init() - > |        |       long push      |        | <- SM / PoM
//                |        |          CV8         |        |
//                +--------+                      +--------+
//
//*****************************************************************************************************

// Instantiate the cvValues object, which will be initialised in the main sketch
// The class CvValues is defined in CvValues.h
CvValues cvValues;

//*****************************************************************************************************
// Fill the `defaults` array with initial values, depending on the decoder type
void CvValues::init(uint8_t decoderType, uint8_t softwareVersion) {
  // Make all values of the defaults array 0, except the first
  for (uint8_t i = 1; i <= max_cvs; i++) defaults[i] = 0;
  defaults[0] = 0b01010101;
  //
  // Decoder type and Software version and Vendor IDs
  defaults[DecType] = decoderType;      // See const definitions from CvValues.h
  defaults[version] = softwareVersion;  // Any value is acceptable, 
  defaults[VID] = 0x0D;                 // Do It Yourself (DIY) decoder
  defaults[VID_2] = 0x0D;               // Used by my PoM software to detect these are my decoders
  //
  // Addresses:
  // - MyAddrL and MyAddrH will often be combined to create the Accessory Decoder address, but may not be needed
  //   for feedback decoders. Lowest address is 1 (not 0!). myAddrH == 0x80 means undefined.
  // - myRSAddr will often be equivalent to the decoder address, and will for feedback decoders be the main address
  //   myRSAddr == 0 means undefined. 128 is reserved for RS-bus feedback to PoM messages
  defaults[myAddrL] = 0x01;             // Decoder address, low order bits (1..64) 
  defaults[myAddrH] = 0x80;             // Decoder address, high order bits (0..3)
  defaults[myRSAddr] = 0;               // RS-bus address (1..128 / undefined = 0)
  //
  // Accessory Decoder configuration
  // Bit 7: ‘0’ = Multi-function (Loco) Decoder / ‘1’= Accessory Decoder
  // Bit 6: Addressing Method: ‘0’= Decoder Address; ‘1’ = Output Address
  // Bit 5: Type: ‘0’ = Basic Accessory; ‘1’ = Extended Accessory
  // Bit 4: Reserved for future use.
  // Bit 3: "0" = no RailCom support, "1" = RailCom support
  // Bit 0..2 = Reserved for future use.
  // Most DIY decoders are Basic Accessory Decoders with multiple outputs (decoder addressing)
  // For Accessory Decoders that have a single output only, it would be better to set bit 6
  defaults[Config] = 0b10000000;        // Setting fits for most DIY decoders
  //
  // Generic settings for most decoders
  defaults[RailCom] = 0;                // 0..1 - We don't support RailCom
  defaults[CmdStation] = 1;             // 1 = LENZ LZV100 with Xpressnet V3.6; the default value
  defaults[SkipUnEven] = 0;             // 0..1 - Only Decoder Addresses 2, 4, 6 .... 1024 will be used
  defaults[RSFEC] = 0;                  // 0..2 - Number of RS-Bus extra transmissions (Forward Error Correction)
  defaults[RSParity] = 1;               // 0..2 - RS-Bus error: 0=no reaction, 1=only if just transmitted, 2=always
  defaults[RSPulsCount] = 2;            // 0..2 - RS-Bus error: 0=no reaction, 1=only if just transmitted, 2=always
  defaults[DccQuality] = 0;             // Counts the number of DCC message checksum errors since last restart
  defaults[ParityErrors] = 0;           // Counts the number of RS-Bus parity errors since last restart
  defaults[PulseErrors] = 0;            // Counts the number of RS-Bus pulse count errors since last restart
  //
  // Time (in 20ms steps) for the four switch / relays outputs. 0 = continuous activation
  // For turn-out mechanisms like Roco 10030 amd 40295/40926, it seems that 300ms is a reasonable value
  // That value seems also resonable for many relays, so we'll use this as default.
  defaults[T_on_F1] = 15;               // 0..255 
  defaults[T_on_F2] = 15;               // 0..255 
  defaults[T_on_F3] = 15;               // 0..255 
  defaults[T_on_F4] = 15;               // 0..255 
  //
  // Settings that are specific per decoder type
  switch (decoderType) {
    //
    case SwitchDecoder:
    case SwitchDecoderWithEmergency:
      // Sending switch Feedback
      // If 1, decoder sends switch feedback messages via the RS-Bus 
      // Should be 0 if decoder sends ONLY PoM feedback (Address 128)
      defaults[SendFB] = 1;             // 0..1 
      // Activation
      // Activate coil, even if a switch "should already be" in the desired position
      // Some train controller software expect a feedback message after each switch command.
      // In that case AlwaysAct must be 1.  
      defaults[AlwaysAct] = 1;          // 0..1 
      // Multiple switches should not share the same feedback nibble. Therefore each switch
      // should have its own nibble, thus we need to skip decoder addresses
      // By setting SkipUneven, only Decoder Addresses 2, 4, 6 .... 1024 will be used 
      defaults[SkipUnEven] = 1;         // 0..1
      //
    break;
    //
    case Relays4Decoder:
      // The Relays4Decoder uses the same software as the switch decoder, except for some CV values
      defaults[SendFB] = 0;             // No need to send feedback information for relays 
      defaults[AlwaysAct] = 0;          // 0..1 
    break;
    //
    case Relays16Decoder:
      // The following defaults values apply to the Relays16Decoder 
      defaults[Ract] = 1;               // If relais switches with - (=0) or with + (=1)
      defaults[RRR1] = 15;              // Relays used for round-robin, relays 1-8  (Port C)
      defaults[RRR2] = 7;               // Relays used for round-robin, relays 9-16 (Port A)
      defaults[RInter] = 6;             // Relays decoder, round-robin interval (in seconds)
      // Relais decoder mode
      // 0: Default operation, switches on ACTIVATE command
      // 1: Same as 0, but DEACTIVATE command releases relais
      // 2: multiple relays may be activated at the same time
      // 3: Relays are activated in a round-robin fashion
      defaults[Mode] = 0;               // 1..3
    break;
    //
    case ServoDecoder:
      // Sending Servo Feedback
      // If 1, decoder sends switch feedback messages via the RS-Bus 
      // Should be 0 if decoder sends ONLY PoM feedback (Address 128)
      defaults[SendFB] = 1;             // 0..1 
      // Activation
      // Activate coil, even if a switch "should already be" in the desired position
      // Some train controller software expect a feedback message after each switch command.
      // In that case AlwaysAct must be 1.  
      defaults[AlwaysAct] = 1;          // 0..1 
      // Multiple switches should not share the same feedback nibble. Therefore each switch
      // should have its own nibble, thus we need to skip decoder addresses
      // By setting SkipUneven, only Decoder Addresses 2, 4, 6 .... 1024 will be used 
      defaults[SkipUnEven] = 1;         // 0..1
      //
    break;
    //
    case SafetyDecoder:
      // The safety decoder is different from normal (switch, servo and GBM) decoders, in the
      // sense that the default addresses for the decoder and RS-Bus should not be
      // "undefined", but 1005 for the accesory address and 127 for the RS-Bus address.
      defaults[myAddrL] = 60;
      defaults[myAddrH] = 3;            // 3 * 64 + 60 = 252 > 1005 .. 1009
      defaults[myRSAddr] = 127;
      // Sending Safety Feedback. If 1, decoder sends feedback messages via the RS-Bus
      defaults[SendFB] = 1;             // 0..1 
      // Which Pin on the X8 connector is for emergency stop button. 
      // 1 = Port PC4, 2 = Port PC5, 3 = Port PC6, 4 = Port PC7
      defaults[P_Emergency] = 4;        // 1 .. 4
      // 
      defaults[T_Watchdog] = 5;         // Maximum time (in seconds) between Watchdog messages (= the time the relay remain active)
      defaults[T_Emergency] = 25;       // Time (in 100ms steps) after an RS-emergency button push the PC gets to stop all trains
      defaults[T_CheckMove] = 50;       // Interval (in 100ms steps) in which we check if PC stopped all trains
      // Time (in 20ms steps) the RS-bus stays ON after a PUSH button is pushed. A value of 0 means a toggle button      
      defaults[T_RS_Push1] = 0; 
      defaults[T_RS_Push2] = 0;      
      defaults[T_RS_Push3] = 150;      
      defaults[T_RS_Push4] = 150;      
    break;
    // 
    case TrackOccupancyDecoder:
    case TrackOccupancyDecoderWithReverser:
    case TrackOccupancyDecoderWithRelays:
    case TrackOccupancyDecoderWithSpeed:
      // Delay related CVs; delay before the previous occupancy will be released
      // The CVs are DelayIn1 - DelayIn8 (Lenz: CV11-CV18) as well as Delay_off (OpenDecoder GBM: CV34)
      // CV11-CV18 allow specification per input; CV34 specifies for all inputs together.
      // By default software should support CV11-CV18, but when their values are 0, CV34 may be used instead.
      // Note that CV11-CV18 is specified in 10 msec steps, whereas CV34 is in 100 msec steps
      // Since the maximum delays achievable diifers for both approaches, for variables within the sketch
      // it is recommended to convert to 16 bit integers.
      defaults[DelayIn1] = 0;
      defaults[DelayIn2] = 0;
      defaults[DelayIn3] = 0;
      defaults[DelayIn4] = 0;
      defaults[DelayIn5] = 0;
      defaults[DelayIn6] = 0;
      defaults[DelayIn7] = 0;
      defaults[DelayIn8] = 0;
      defaults[Delay_off] = 15;
      // How many consecutive ON samples are needed with the same outcome before the result is considered to be stable?
      // Every sample takes 8 msec, so "3" gives 24 msec extra delay
      defaults[Min_Samples] = 3;    // 0..8
      // Sensitivity: to avoid oscillation some hysteresis is needed. 
      // If the previous state was OFF, values above Threshold_on will change state to ON
      // If the previous state was ON, values below Threshold_of will change state to OFF
      // Obviously the value of threshold_of should be lower than threshold_on
      // Experimental values for specific resistors and an ATMega16 are: 82K=7,  68K=8, 
      //  56K=10, 47K=12, 39K=15, 33K= 18, 27K=22, 22K=28, 18K=34, 15K=41, 12K=52, 10K= 68
      defaults[Threshold_on] = 20;
      defaults[Threshold_of] = 15;
      // Speed Measurement
      // A TrackOccupancyDecoder can also be used to measure speed on two different tracks.
      // The time a train needs to pass a track can be measured. Since the track length is known,
      // the train's speed can be calculated. 
      // Speed1 is the first speed measurement track, Speed1_Out defines the Track number 
      // Speed2 is the second speed measurement track, Speed2_Out defines the Track number 
      defaults[Speed1_Out] = 0;   // 1..8: Track number
      defaults[Speed1_LL] = 0;    // 0..255: Track length in mm (low order byte)
      defaults[Speed1_LH] = 0;    // 0..20:  Track length in mm (high order byte)
      defaults[Speed2_Out] = 0;
      defaults[Speed2_LL] = 0;
      defaults[Speed2_LH] = 0;
      // Reverser
      // A TrackOccupancyDecoder can also be used as a reverser.
      defaults[FB_A] = 0;         // 1..8: Feedback bit if track A is occupied
      defaults[FB_B] = 1;
      defaults[FB_C] = 2;
      defaults[FB_D] = 3;
      defaults[FB_S1] = 0;        // 1..8: Feedback bit if Sensor 1 is active
      defaults[FB_S2] = 1;
      defaults[FB_S3] = 1;
      defaults[FB_S4] = 2;
      defaults[Polarization] = 0; // If 0: J&K connected normal, if 1: J&K polarization changed
    break;
    //
    case LiftDecoder:
      defaults[StartHoming] = 1;  // If 1: perform a stepper motor homing cycle at program start
      defaults[IR_Detect] = 1;    // If 1: enable the IR sensors to detect if trains block movement
      defaults[LCD_Display] = 0;  // If 1: enable the LCD Display (which may interfere with RS-Bus)
      defaults[Serial_Line] = 0;  // See setup() of the lift decoder for details. 0: off, 1: on, 2: details
    break;
    //
    case FunctionDecoder:
    break;
  }
}


//*****************************************************************************************************
// Checks if the EEPROM and the decoder address have been initialised
//*****************************************************************************************************
bool CvValues::notInitialised(void) {
  return (EEPROM.read(0) != 0b01010101);
}


bool CvValues::addressNotSet(void) {
  // For GBM we check CV10 (RS-Bus address), for others CV9 (decoder address)
  if (cvValues.isGBM()) {return (EEPROM.read(myRSAddr) == 0x00);}
  else return (EEPROM.read(myAddrH) == 0x80);
}


bool CvValues::isGBM(void) {
  bool result;
  uint8_t decoderType = read(DecType);
  switch (decoderType) {
  case TrackOccupancyDecoder:
  case TrackOccupancyDecoderWithReverser:
  case TrackOccupancyDecoderWithRelays:
  case TrackOccupancyDecoderWithSpeed:
    result = true;
  break;
  default:
    result = false;
  break;
  }
  return result;
}


//*****************************************************************************************************
// Restore all EEPROM content to default
//*****************************************************************************************************
void CvValues::setDefaults(void) {
  // Note that defaults[0] contains the value that indicates the EEPROM has been initialised
  // Note also that the decoder type as well as software version will be overwritten.
  for (uint8_t i = 0; i <= max_cvs; i++) EEPROM.update(i, defaults[i]);
}


//*****************************************************************************************************
// Read and Write
//*****************************************************************************************************
uint8_t CvValues::read(uint8_t number){
  return EEPROM.read(number);
}

void CvValues::write(uint8_t number, uint8_t value){
  // We do not do any sanity check regarding the value that is entered!
  EEPROM.update(number, value) ;
}


//*****************************************************************************************************
// Retrieve the decoder address, as stored in the EEPROM
// For Accessory Decoders this is either the decoder address or the output address
// For Multi-function Decoders this is the (short or long) loco address
//*****************************************************************************************************
unsigned int CvValues::storedAddress(void) {
  // The decoder configuration, and thus the address mode, is stored in CV29 (Config)
  // Bit 7: ‘0’ = Multi-function (Loco) Decoder / ‘1’= Accessory Decoder
  // Bit 6: Accessory addressing Method: ‘0’= Decoder Address; ‘1’ = Output Address
  // Bit 5: Loco addressing mode: '0' = Use short loco address from CV1, 
  //                              '1' = Use long loco address from CV17/18

  unsigned int address;
  uint8_t cv1;
  uint8_t cv9;
  uint8_t cv17;
  uint8_t cv18;
  uint8_t cv29 = cvValues.read(Config);
  bool accDecoder = bitRead(cv29,7);
  bool outputAddr = bitRead(cv29,6);
  bool longLocoAddr = bitRead(cv29,5);
  if (accDecoder) {
    // RCN-213 and RCN-225 describe the relationship between CV1, CV9 and the address.
    // According to RCN213, for the first handheld address (switch = 1) CV1 should become 1.
    // - the valid range for CV1 is 1..63 (if CV9 == 0) or 0..63 (if CV9 !=0)
    // - the valid range for CV9 is 0..3  (or 128, if the decoder has not been initialised)
    // If the decoder address has not yet been initialised, the highest order bit 
    // of `address` will be set.
    if (outputAddr) {
      cv1 = cvValues.read(myAddrL);
      cv9 = cvValues.read(myAddrH) & 0b00000111;
      // CV1 starts from 1, thus the lowest output address becomes 1
      address = (cv9 << 8) + cv1;
    }
    else {  // decoder addressing
      cv1 = cvValues.read(myAddrL) & 0b00111111;
      cv9 = cvValues.read(myAddrH) & 0b00000111;
      // CV1 starts from 1, but the lowest decoder address should be 0
      address = (cv9 << 6) + cv1 - 1;
    }    
    // If the decoder address has not yet been initialised (the highest order bit 
    // of myAddrH is set), `address` will be set to maxint.
    if (cvValues.read(myAddrH) >= 128) {address = 65535;}
  }
  else {  // Loco (multifunction) address
  // CV1 stores the basic (short) address. The range is between 1 and 127
  // CV17 and CV18 store the extended (14 bit long) address. CV17 is the MSB.
  // The default address is 3
    if (longLocoAddr) {
      cv17 = cvValues.read(17) & 0b00111111;
      cv18 = cvValues.read(18);
      address = (cv17 << 8) + cv18;
    }
    else {  // Use short loco address from CV1
      cv1 = cvValues.read(myAddrL) & 0b01111111;
      address = cv1;
    }
    // Address 0 is invalid. In that case enter the default address
    if (address == 0) address = 3;
  }
  return address;
}
