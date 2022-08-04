
# The DCC decoder core #

## Direct links: ##
- [AP_DCC_Library](https://github.com/aikopras/AP_DCC_library#AP_DCC_library):
  dcc.input()
- [RSBus library:](https://github.com/aikopras/RSbus#RSbus)
  RSbusConnection
- [Common Hardware Functions](src/CommonFunctions/CommonFunctions.md#CommonDecHwFunctions):
  decoderHardware.init() and decoderHardware.update()
- [Configuration Variables](src/CvValues/CvValues.md#CvValues):
  List of CVs, cvValues.init(), cvValues.read() and cvValues.write()
- [CvProgramming](src/CommonFunctions/CvProgramming.md#CvProgramming):
   cvProgramming.processMessage(...)
- [LEDs](src/DccLED/DccLED.md#AP_DccLED): BasicLed, FlashLed, DCCLed
- [Buttons](src/DccButton/DccButton.md#DccButton): DccButton, ToggleButton
- [Timers](src/DccTimer/DccTimer.md#DccTimer): DccTimer
- [Pin assignments](src/boards.h): boards.h

## Purpose ##

Implements the core functions and objects needed by every DCC decoder that supports Configuration Variables (CVs) and feedback via RS-Bus messages. It builds upon the [AP_DCC_library](https://github.com/aikopras/AP_DCC_library#AP_DCC_library) and the [RSBus](https://github.com/aikopras/RSbus) library.

The DCC decoder core initialises the DCC and RS-Bus hardware, by using the pin and USART settings as defined in [boards.h](src/boards.h).

The core functions handle most of the Programming on the Main (PoM) and Service Mode (Programming track) messages and support Configuration Variables (CVs) for different kind of decoders; these CVs can be remotely accessed via SM and PoM. A speciality is that PoM feedback messages will not be send via RailCom, but via the RS-Bus using address 128. Dedicated MAC programs are available that use this setup to configure these CVs; these programs can be downloaded from:
- https://github.com/aikopras/Programmer-Decoder-POM
- https://github.com/aikopras/Programmer-GBM-POM

If the user pushes the programming button, the core functions respond by initialising the decoder's DCC and/or RS-Bus addresses based on the next DCC address that is received. The core functions provide an onboard LED object, to signal events to the user, and an onboard button object, to allow the user to configure a new address.

The library has been tested on traditional ATMega processors, such as the ATMega 16A, ATMega 328 (UNO, Nano) and ATMega2560 (Mega), but also on the newer MegaAVR processors, such as 4808 (Thinary), 4809 (Nano Every) and the AVR128DA. For the design of new boards the use of these newer processors is recommended. Note that dedicated "boards" may need to be installed in the Arduino IDE to support these processors, such as [MightyCore](https://github.com/MCUdude/MightyCore), [MegaCore](https://github.com/MCUdude/MegaCore), [MegaCoreX](https://github.com/MCUdude/MegaCoreX) and [DxCore](https://github.com/SpenceKonde/DxCore). For each board the mapping between external (DCC and RS-Bus) signals and Arduino pin numbers is defined in the [boards.h](src/boards.h) file.

____

## Using the DCC Decoder Core ##
The only library file that needs to be included by the main sketch is `AP_Accessory_Common.h`. This header file includes the following header files and libraries: `CvValues.h`, `AP_DCC_library`, `RSbus`, `AP_DccLED` and `AP_DccButton`. Instead of `AP_Accessory_Common.h`, it is also possible to include individual header files if limited functionality is needed only.

## DCC decoder objects and classes ##
The following objects and classes become available to the user sketch:

- **decoderHardware** ([class: CommonDecHwFunctions](src/CommonFunctions/CommonFunctions.md#CommonDecHwFunctions)): initialises the following decoder hardware: DCC interface, RS-Bus interface, onboard LED and the programming button. Provides two functions: `init()` and `update()`.

- **dcc** ([class:Dcc](https://github.com/aikopras/AP_DCC_library#Dcc)): the Dcc class informs the main sketch which kind of DCC command has been received. The main loop of the user sketch should call *`dcc.input()`* to check if a new DCC message is received. If a new DCC message is received, the `dcc.cmdType` should be inspected to determine the kind of DCC command. Three main command types are possible: *accessory command*, *loco (multi-function) command* or *CV access (POM, SM) command*. The Dcc class is defined as part of the [AP_DCC_library](https://github.com/aikopras/AP_DCC_library#AP_DCC_library).
  - **accCmd** ([class: Accessory](https://github.com/aikopras/AP_DCC_library#Accessory)): if `dcc.cmdType` is of type `MyAccessoryCmd`, additional information, such as the `turnout` and `position`, is provided by the `accCmd` object.
  - **locoCmd** ([class: Loco](https://github.com/aikopras/AP_DCC_library#Loco)): if `dcc.cmdType` returns any of the loco types (such as `MyLocoSpeedCmd` or `MyLocoF0F4Cmd`), additional information is provided by the `locoCmd` object.
  - **cvCmd** ([class: CvAccess](https://github.com/aikopras/AP_DCC_library#CvAccess)): if `dcc.cmdType` returns `MyPomCmd`, the number and value of the received CV can be obtained via the `cvCmd` object.


- **CvProgramming** ([class CvProgramming](src/CommonFunctions/CvProgramming.md#CvProgramming)): processes a received PoM or SM command. Reads or modifies the CV that is targetted by this command. If `dcc.cmdType` returns `MyPomCmd` or `SmCmd`, the main sketch should call `cvProgramming.processMessage()` to ensure that the targetted CV is indeed being read or modified.

- **cvValues** ([class CvValues](src/CvValues/CvValues.md#CvValues)): allows the main sketch to `read()` or `write()` individual CV values. To select the matching set of CV default values for this type of decoder, `setup()` of the main sketch should call `cvValues.init()`.

- **[RS-Bus](https://github.com/aikopras/RSbus#RSbus)**: To send feedback messages via the RS-Bus, the user sketch may instantiated one or more RS-Bus objects of class [RSbusConnection](https://github.com/aikopras/RSbus#RSbusConnection). Note that the user sketch does not need to instantiate the RSbusHardware class itself or create RS-Bus objects for PoM feedback, since the decoderHardware object already takes care of that.

- **onBoardLed** ([defined in AP_DccLED.h](src/DccLED/DccLED.md#AP_DccLED)): the onboard LED may be used to inform the user of specific events. To accommodate different LED behaviour, the following classes are defined:
  - [BasicLed](src/DccLED/DccLED.md#BasicLed): to turn on, off or toggle LEDs.
  - [FlashLed](src/DccLED/DccLED.md#FlashLed): allows LEDs to flash slow, fast, or user specified.
  - [DCCLed](src/DccLED/DccLED.md#DCCLed): the typical class for onboard LEDs.


- **Buttons**: the user sketch may need to read the status of (debounced) buttons. Two different classes are provided: the [DccButton](src/DccButton/DccButton.md#DccButton) class for normal buttons and the [ToggleButton](src/DccButton/DccButton.md#ToggleButton) class for toggle buttons. The onboardButton is of class DccButton, but this button should not be used by the user sketch.

- **Timers**: the [DccTimer](src/DccTimer/DccTimer.md#DccTimer) class allows the user sketch to include (non-blocking) timers based on Arduino's `millis()`.

___
## Example ##
A skeleton that shows the basic usage of the core functions (without RS-Bus feedback) is shown below.  
Each decoder should call in `setup()` `cvValues.init()` as well as `decoderHardware.init()`. The main loop should call `dcc.input()` to check if a new DCC message has been received, and `decoderHardware.update()` to update the onboard LED and check if the programming button was pushed.

````
#include <AP_DCC_Decoder_Core.h>

void setup() {
  cvValues.init(SwitchDecoder, 20);
  decoderHardware.init();
}

void loop() {
  // Do something with the DCC message received ...
  if (dcc.input()) {};
  // Check if the programming button is pushed; handle CV PoM and SM messages
  decoderHardware.update();
}
````
A more elaborate example, which includes feedback via the RS_Bus, is shown [here](examples/BasicDecoder/BasicDecoder.md) in BasicDecoder.md
