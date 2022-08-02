
# The DCC decoder core #

Implements the core functions and objects needed by every DCC decoder that supports Configuration Variables (CVs) and feedback via RS-Bus messages. It builds upon the [AP_DCC_Library](https://github.com/aikopras/AP_DCC_library) and the [RSBus](https://github.com/aikopras/RSbus) library.

The DCC decoder core initialises the DCC and RS-Bus hardware, by using the pin and USART settings as defined in [boards.h](src/boards.h).

The core functions handle most of the Programming on the Main (PoM) and Service Mode (Programming track) messages and support Configuration Variables (CVs) for different kind of decoders; these CVs can be remotely accessed via SM and PoM. A speciality is that PoM feedback messages will not be send via RailCom, but via the RS-Bus using address 128. Dedicated MAC programs are available that use this setup to configure these CVs; these programs can be downloaded from:
- https://github.com/aikopras/Programmer-Decoder-POM
- https://github.com/aikopras/Programmer-GBM-POM

If the user pushes the programming button, the core functions respond by initialising the decoder's DCC and/or RS-Bus addresses based on the next DCC address that is received. The core functions provide an onboard LED object, to signal events to the user, and an onboard button object, to allow the user to configure a new address.

The library has been tested on traditional ATMega processors, such as the ATMega 16A, ATMega 328 (UNO, Nano) and ATMega2560 (Mega), but also on the newer MegaAVR processors, such as 4808 (Thinary), 4809 (Nano Every) and the AVR128DA. For the design of new boards the use of these newer processors is recommended. Note that dedicated "boards" may need to be installed in the Arduino IDE to support these processors, such as [MightyCore](https://github.com/MCUdude/MightyCore), [MegaCore](https://github.com/MCUdude/MegaCore), [MegaCoreX](https://github.com/MCUdude/MegaCoreX) and [DxCore](https://github.com/SpenceKonde/DxCore). For each board the mapping between external (DCC and RS-Bus) signals and Arduino pin numbers is defined in the [boards.h](src/boards.h) file.

----
## DCC decoder objects ##
The only library file that needs to be included by the main sketch is `AP_Accessory_Common.h`; this header file includes the following header files and libraries: `CvValues.h`, `AP_DCC_Library`, `RSbus`, `AP_DCC_LED` and `AP_DCC_Button`. As a result, the following objects become available to the user sketch:

- `decoderHardware (class: CommonDecHwFunctions)`. Initialises the following decoder hardware: DCC interface, RS-Bus interface, onboard LED and the programming button. Provides two functions: `init()` and `update()`. See [Common Hardware Functions](src/CommonFunctions/CommonFunctions.md).

- `dcc (class: Dcc)`: the main loop of the user sketch should call *`dcc.input()`* to check if a new DCC message has been received. If a new DCC message was received, the `dcc.cmdType` should be inspected to determine the kind of DCC command. Three main command types are possible: *accessory command*, *loco command* and *CV programming command*. The Dcc class is defined as part of the [AP_DCC_library](https://github.com/aikopras/AP_DCC_library)
  - `accCmd (class: Accessory)`: if `dcc.cmdType` is of type `MyAccessoryCmd`, additional information, such as the `turnout` and `position`, is provided by the `accCmd` object.
  - `locoCmd (class: Loco)`: if `dcc.cmdType` returns any of the loco types (such as `MyLocoSpeedCmd` or `MyLocoF0F4Cmd`), additional information is provided by the `locoCmd` object.
  - `cvCmd (class: CvAccess)`: if `dcc.cmdType` returns `MyPomCmd`, the number and value of the received CV can be obtained via the `cvCmd` object.
  - `cvProgramming (class CvProgramming)`: if `dcc.cmdType` returns `MyPomCmd` or `SmCmd`, a call should be made to `cvProgramming.processMessage()` to process the message. See [CvProgramming](src/CommonFunctions/CvProgramming.md).

- `cvValues (class CvValues)`: the `init()` method of `cvValues` should be called in `setup()` of the main sketch to select the correct set of CVs for this decoder. The `read()` method should be used to retrieve individual CV values. For details, see [here](src/CvValues/CvValues.md).

- `onBoardLed (class DCC_Led)`: the onboard LED may be used to inform the user of certain events. For details, see [here](src/LED/LED.md).

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
