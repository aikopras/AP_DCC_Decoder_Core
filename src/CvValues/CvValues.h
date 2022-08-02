//*****************************************************************************************************
//
// File:      CvValues.h
// Author:    Aiko Pras
// History:   2021/06/14 AP Version 1.0
//            2021/12/30 AP Version 1.2
//            2022/07/13 AP Version 1.3 Safety decoder CV naming changed
//            2022/08/02 AP Version 1.4 Restructure of the library
//
// Purpose:   Header file that defines the methods to read and modify CV values stored in EEPROM,  
//            as well as the default values for all
//
//******************************************************************************************************
#pragma once
#include <Arduino.h>


//*****************************************************************************************************
// CV values are stored in EEPROM and can be accessed via methods like read(), write(), and verify().
// CV values can be modified via PoM or SM messages. A restart is generally needed to take these 
// new values into effect.
//
// EEPROM default values 
// The CV default values are written to EEPROM by the setDefaults() method.
// setDefaults() simply copies to EEPROM the contents of the `defaults` array, which holds
// all default values for all CVs. These values may vary for different types of decoders. 
// The `defaults` array will initially be filled by the class constructor, but after instatiation
// the main sketch may override values whenever needed.
//
// Depending on the further decoder software, there are three ways to call setDefaults(): 
// 1) Long (>5 sec.) press of the programming button on the board.
// 2) Sending a PoM or SM message to CV 8 (the  Vendor ID) with the value 0x0D.
// 3) In case the EEPROM has not yet been initialised, the setup() method in main will call setDefaults().
//
// CommonDecHwFunctions() should call notInitialised() to check if the EEPROM is already filled.
// notInitialised() reads the first EEPROM element (EEPROM.read(0)), which is for uninitialised
// EEPROMs generally 00 or FF.
// If the EEPROM is uninitialised, begin() writes the value 0b01010101 to the first EEPROM element
// and copies immediately after the contents of the `defaults` array.  
// Note that the first EEPROM element will not be used by any CV, since the first CV has
// number 1, and (for simplicity) is stored at EEPROM location 1 (EEPROM.write(1, "value CV1")).
//
// Depending on the board that is being used and the specific Arduino IDE settings, EEPROM values
// may not be erased when a new sketch is being uploaded. For example, the standard Arduino AVR Uno
// board does not allow the EEPROM to be erased during sketch upload. The MiniCore board for the 
// ATmega328, however, like all other boards from MCUdude (MegaCore, MightyCore, MiniCore...) has an 
// option (`EEPROM not retained`) to erase all EEPROM contents (see for details MCUdude's github pages). 
//
// The `defaults` array
// As opposed to the original OpenDecoder software, the default CV values will not be stored in
// PROGMEM but in RAM, since RAM sizes have increased over the years and storing them into RAM allows
// the main sketch to modify some of these values. In this way it is possible to have most
// parts of the CV handling in a library, but still allow the main sketch to make modifications to
// default values in case that is considered necessary.
//
// A description of CV values can be found in RCN-225. 
// For CV1-CV30 we follow that description, but with a number of exceptions:
// - CV2: not implemented here
// - CV10-16: These CVs are reserved by the NMRA, but are used here for RSbus address and GBM delay
// - CV17-18: Should be "mirrored address", but used here for GBM delay 
// - CV19-27: These CVs are reserved by the NMRA, but are used here for different purposes.
// - CV30:    This CV is reserved by the NMRA, but is used here for the second vendor ID.
// - CV31-32: Should be "pointers to extended area", but are used here for different purposes.
//
// There are two reasons why we deviate from RCN-225. First, for CV11-Cv18 we decided to follow  
// the Lenz LR101 feedback decoder CVs. Second, we decided to use several of the other NMRA reserved  
// CVs, since these have low CV numbers and are therefore stored at low EEPROM addresses.
// The alternative would have been to use the vendor specific part (such as CV33-CV81), but such
// approach would have cost more EEPROM space.
//
//*****************************************************************************************************
// Predefined values for the Decoder Type CV (CV27).
const uint8_t SwitchDecoder                     = 0b00010000;   // Switch decoder
const uint8_t SwitchDecoderWithEmergency        = 0b00010001;   // Switch decoder with Emergency board
const uint8_t ServoDecoder                      = 0b00010100;   // Servo Decoder
const uint8_t LiftDecoder                       = 0b00011000;   // Lift Decoder
const uint8_t Relays4Decoder                    = 0b00100000;   // Relays decoder for 4 relays
const uint8_t Relays16Decoder                   = 0b00100001;   // Relays decoder for 16 relays
const uint8_t TrackOccupancyDecoder             = 0b00110000;   // Track Occupancy decoder
const uint8_t TrackOccupancyDecoderWithReverser = 0b00110001;   // Track Occupancy decoder with reverser
const uint8_t TrackOccupancyDecoderWithRelays   = 0b00110010;   // Track Occupancy decoder with relays
const uint8_t TrackOccupancyDecoderWithSpeed    = 0b00110100;   // Track Occupancy decoder with speed measurement
const uint8_t FunctionDecoder                   = 0b01000000;   // Function Decoder
const uint8_t SafetyDecoder                     = 0b10000000;   // Watchdog and safety decoder


//*****************************************************************************************************
const uint8_t max_cvs = 64;        // Maximum number of CVs

// CV Names - Generic CVs implemented by all decoders
const uint8_t myAddrL      = 1;    // 0..63 / 0..255 - Decoder Address low. First address = 1.
const uint8_t T_on_F1      = 3;    // 0..255 - Time on F1 (in 20ms steps). 0 = continuous activation
const uint8_t T_on_F2      = 4;    // 0..255 - Time on F2 (in 20ms steps). 0 = continuous activation
const uint8_t T_on_F3      = 5;    // 0..255 - Time on F3 (in 20ms steps). 0 = continuous activation
const uint8_t T_on_F4      = 6;    // 0..255 - Time on F4 (in 20ms steps). 0 = continuous activation
const uint8_t version      = 7;    // 8..255 - Version should be 8 or higher
const uint8_t VID          = 8;    // 0x0D   - Vendor ID (0x0D = DIY Decoder) / If set to 0x0D, the decoder resets to default settings
const uint8_t myAddrH      = 9;    // 0..3   - Decoder Address high (3 bits)
const uint8_t myRSAddr     = 10;   // 1..128 - RS-bus address (Main address of the Feedback decoder). 0 = undefined
const uint8_t DelayIn1     = 11;   // 0..255 - Delay in 10ms steps before sending OFF signal (conform LENZ LR101)
const uint8_t DelayIn2     = 12;   // same
const uint8_t DelayIn3     = 13;   // same
const uint8_t DelayIn4     = 14;   // same
const uint8_t DelayIn5     = 15;   // same
const uint8_t DelayIn6     = 16;   // same
const uint8_t DelayIn7     = 17;   // same
const uint8_t DelayIn8     = 18;   // same
const uint8_t CmdStation   = 19;   // 0..2   - Master Station: 0 = Roco/Multimouse, 1 = Lenz, 2 = OpenDCC Z1 (Xpressnet V3.6)                    
const uint8_t RSFEC        = 20;   // 0..2   - Number of RS-Bus extra transmissions (Forward Error Correction)
const uint8_t SkipUnEven   = 21;   // 0..1   - Only Decoder Addresses 2, 4, 6 .... 1024 will be used
const uint8_t RSParity     = 22;   // 0..2   - RS-Bus: 0 = ignore parity errors, 1 = retransmit previous, 2 = retransmit always
const uint8_t Search       = 23;   // 0..1   - If 1: decoder LED blinks
const uint8_t RSPulsCount  = 24;   // 0..2   - RS-Bus: 0 = ignore pulse count errors, 1 = retransmit previous, 2 = retransmit always
const uint8_t Restart      = 25;   // 0..1   - To restart (as opposed to reset) the decoder
const uint8_t DccQuality   = 26;   // 0..255 - DCC Signal Quality
const uint8_t DecType      = 27;   // ...    - Decoder Type / See below for acceptable values. Parameter to the class constructor
const uint8_t RailCom      = 28;   // 0      - Bi-Directional (RailCom) Communication Config
const uint8_t Config       = 29;   // ...    - Accessory Decoder configuration
const uint8_t VID_2        = 30;   // 0x0D   - Second Vendor ID (Used by my PoM software to detect these are my decoders)
const uint8_t ParityErrors = 31;   // 0..255 - RS-bus Signal Quality: number of parity errors
const uint8_t PulseErrors  = 32;   // 0..255 - RS-bus Signal Quality: number of pulse count errors

// CV Names - Specific for a Track Occupancy Decoder 
const uint8_t Min_Samples  = 33;   // 0..8   - Number of ON samples before the state is considered stable
const uint8_t Delay_off    = 34;   // 0..255 - Delay (in 100 ms steps) before previous occupancy will be released
const uint8_t Threshold_on = 35;   //10..255 - Above this value a previous OFF sample will be regarded as ON
const uint8_t Threshold_of = 36;   // 5..255 - Below this value a previous ON sample will be regarded as OFF
const uint8_t Speed1_Out   = 37;   // 0..8   - Track number (1..8) for the first speed measurement track (0=none)
const uint8_t Speed1_LL    = 38;   // 0..255 - Length in mm of the first speed measurement track (LSB)
const uint8_t Speed1_LH    = 39;   // 0..20  - Length in mm of the first speed measurement track (MSB)
const uint8_t Speed2_Out   = 40;   // 0..8   - Track number (1..8) for the second speed measurement track (0=none) 
const uint8_t Speed2_LL    = 41;   // 0..255 - Length in mm of the second speed measurement track (LSB)
const uint8_t Speed2_LH    = 42;   // 0..20  - Length in mm of the second speed measurement track (MSB)
const uint8_t FB_A         = 43;   // 0..8   - Feedback bit if track A is occupied
const uint8_t FB_B         = 44;   // 0..8   - Feedback bit if track B is occupied
const uint8_t FB_C         = 45;   // 0..8   - Feedback bit if track C is occupied
const uint8_t FB_D         = 46;   // 0..8   - Feedback bit if track D is occupied
const uint8_t FB_S1        = 47;   // 0..8   - Feedback bit if Sensor 1 is active
const uint8_t FB_S2        = 48;   // 0..8   - Feedback bit if Sensor 2 is active
const uint8_t FB_S3        = 49;   // 0..8   - Feedback bit if Sensor 3 is active
const uint8_t FB_S4        = 50;   // 0..8   - Feedback bit if Sensor 4 is active
const uint8_t Polarization = 51;   // 0..1   - If 0: J&K connected normal / if 1: J&K polarization changed

// CV Names - Specific for Switch and Relays-4  Decoders
const uint8_t SendFB       = 33;   // 0..1   - Decoder will send switch feedback messages via the RS-Bus 
const uint8_t AlwaysAct    = 34;   // 0..1   - If set, decoder will activate coil / relays for each DCC command received

// CV Names - Specific for a Relays-16 Decoder 
const uint8_t Ract         = 33;   // 0..1   - If relays switches with - (=0) or with + (=1)
const uint8_t RRR1         = 34;   // Relays used for round-robin, relays 1-8  (Port C)
const uint8_t RRR2         = 35;   // Relays used for round-robin, relays 9-16 (Port A)
const uint8_t RInter       = 36;   // Relays decoder, round-robin interval (in seconds)
const uint8_t Mode         = 37;   // 1..3   - Relais decoder mode

// CV Names - Specific for a Safety Decoder 
const uint8_t SendButtonFB = 33;  // 0..1   - Decoder sends feedback via the RS-Bus if the emergency button is pushed
const uint8_t P_Emergency  = 34;  // 1/4    - Which Pin on the X8 connector is for emergency stop. Possible values: 1 .. 4
const uint8_t T_Watchdog   = 35;  // Number of seconds watchdog relay will remain active
const uint8_t T_Emergency  = 36;  // Time after an RS-emergency button push for PC to stop all trains
const uint8_t T_CheckMove  = 37;  // Interval in which we check if PC stopped all trains
const uint8_t T_RS_Push1   = 38;  // Time RS-bus stays ON after a PUSH button is pushed
const uint8_t T_RS_Push2   = 39;  // 
const uint8_t T_RS_Push3   = 40;  // 
const uint8_t T_RS_Push4   = 41;  // 

// CV Names - Specific for a Servo Decoder 
const uint8_t LastState    = 33;   // 0..1  - Save last servo position
 
// CV Names - Specific for a LiftDecoder
const uint8_t StartHoming  = 33;  // 0..1   - During initialisation decoder starts with a homing cycle
const uint8_t IR_Detect    = 34;  // 0..1   - Disable / Enable the IR detectors to block lift movement
const uint8_t LCD_Display  = 35;  // 0..1   - Disable / Enable the LCD display 
const uint8_t Serial_Line  = 36;  // 0,1,2  - Disable / Enable the Serial interface. Enable for GRBL config changes


//*****************************************************************************************************
class CvValues {
  public:
    uint8_t defaults[max_cvs + 1];                 // Default values for CVs
    
    // Fills the defaults array
    void init(uint8_t decoderType, uint8_t softwareVersion = 10);        

    // Functions to ensure the EEPROM is being filled
    bool notInitialised(void);                     // Checks if the EEPROM has been initialised
    void setDefaults(void);                        // Fills the decoder with the default values


    // Generic CV functions
    uint8_t read(uint8_t number);
    void write(uint8_t number, uint8_t value);

    
    unsigned int storedAddress(void);              // From CV1 and CV9 we get the decoder address
    bool addressNotSet(void);                      // Check if the decoder / RS-Bus address has been set
    bool isGBM(void);                              // Check if this is a GBM decoder (variant)

  private:
  
};
