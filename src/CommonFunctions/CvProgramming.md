# The CvProgramming Class #
This class has two functions, of which only one is relevant for the main sketch: `processMessage()`.

#### void processMessage(Dcc::CmdType_t cmdType) ####
To ensure the decoder will react after reception of a PoM or SM message, the main loop should call `processMessage()` after such programming message is received. The parameters may be `Dcc::MyPomCmd` or `Dcc::SmCmd`.

## Example ##
````
switch (dcc.cmdType) {
  case Dcc::MyAccessoryCmd :
    // do something
  break;      
  case Dcc::MyPomCmd :
    cvProgramming.processMessage(Dcc::MyPomCmd);
  break;
  case Dcc::SmCmd :
    cvProgramming.processMessage(Dcc::SmCmd);
  default:
    // ...
    break;
}
````
