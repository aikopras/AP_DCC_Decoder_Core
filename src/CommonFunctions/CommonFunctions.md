
# <a name="CommonDecHwFunctions"></a>CommonDecHwFunctions functions #
This is a core class needed by every decoder and it has two functions: `init()` and `update()`.

#### void init(void) ####
Init should be called from `setup()` in the main sketch. It initialises the `dcc`, `rsbusHardware`, `onBoardLed` and `progButton` objects with Arduino pin and USART numbers, such as defined in the [boards.h](src/boards.h) file. If needed, this [boards.h](src/boards.h) file may be modified to satisfy the needs of a specific board. If the EEPROM that stores the CV values has been erased, `init` reinitialises the EEPROM.

#### void update(void) ####
Update should be called from main loop as often as possible. It controls and maintains the RS-Bus polling process (by calling `rsbusHardware.checkPolling`) and checks (every 20ms) if the onboard programming button is pushed, if the status of the onboard LED should be changed and if there are any waiting PoM messages that should be send using the RS-Bus with address 128.
___

## The Processor Class ##
This class has one function: `reboot()`.

#### void reboot(void) ####
In normal cases the main sketch may, but doesn't need to call `reboot()`, since `reboot()` will automatically be called after the decoder's programming button was pushed, or after the `Restart` CV (CV25) is set.
