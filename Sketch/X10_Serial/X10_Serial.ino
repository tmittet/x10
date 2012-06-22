/************************************************************************/
/* X10 PLC, RF, IR library test sketch with Serial support, v1.3        */
/*                                                                      */
/* This library is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* This library is distributed in the hope that it will be useful, but  */
/* WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     */
/* General Public License for more details.                             */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with this library. If not, see <http://www.gnu.org/licenses/>. */
/*                                                                      */
/* Written by Thomas Mittet thomas@mittet.nu October 2010.              */
/************************************************************************/

#include <X10ex.h>
#include <X10rf.h>
#include <X10ir.h>

#define DEBUG 0

#define POWER_LINE_MSG "PL:"
#define POWER_LINE_BUFFER_ERROR "PL:_ExBuffer"
#define RADIO_FREQ_MSG "RF:"
#define INFRARED_MSG "IR:"
#define SERIAL_DATA_MSG "SD:"
#define SERIAL_DATA_THRESHOLD 1000
#define SERIAL_DATA_TIMEOUT "SD:_ExTimOut"
#define MODULE_STATE_MSG "MS:"
#define MSG_DATA_ERROR "_ExSyntax"

// Fields used for serial and byte message reception
unsigned long sdReceived;
char bmHouse;
byte bmUnit;
byte bmCommand;
byte bmExtCommand;

// zeroCrossInt = 2 (pin change interrupt), zeroCrossPin = 4, transmitPin = 5, receivePin = 6, receiveTransmits = true, phases = 1, sineWaveHz = 50
X10ex x10ex = X10ex(2, 4, 5, 6, true, processPlMessage, 1, 50);
// receiveInt = 0 (external interrupt), receivePin = 2
X10rf x10rf = X10rf(0, 2, processRfCommand);
// receiveInt = 1 (external interrupt), receivePin = 3, defaultHouse = 'A'
X10ir x10ir = X10ir(1, 3, 'A', processIrCommand);

void setup()
{
  Serial.begin(115200);
  x10ex.begin();
  x10rf.begin();
  x10ir.begin();
  Serial.println("X10");
}

void loop()
{
  processSdMessage();
}

// Process messages received from X10 modules over the power line
void processPlMessage(char house, byte unit, byte command, byte extData, byte extCommand, byte remainingBits)
{
  printX10Message(POWER_LINE_MSG, house, unit, command, extData, extCommand, remainingBits);
}

// Process commands received from X10 compatible RF remote
void processRfCommand(char house, byte unit, byte command, bool isRepeat)
{
  if(!isRepeat) printX10Message(RADIO_FREQ_MSG, house, unit, command, 0, 0, 0);
  // Check if command is handled by scenario; if not continue
  if(!handleUnitScenario(house, unit, command, isRepeat, false))
  {
    // Make sure that two or more repetitions are used for bright and dim,
    // to avoid that commands are beeing buffered seperately when repeated
    if(command == CMD_BRIGHT || command == CMD_DIM)
    {
      x10ex.sendCmd(house, unit, command, 2);
    }
    // Other commands map directly: just forward to PL interface
    else if(!isRepeat)
    {
      x10ex.sendCmd(house, unit, command, 1);
    }
  }
}

// Process commands received from X10 compatible IR remote
void processIrCommand(char house, byte unit, byte command, bool isRepeat)
{
  if(!isRepeat) printX10Message(INFRARED_MSG, house, unit, command, 0, 0, 0);
  // Check if command is handled by scenario; if not continue
  if(!handleUnitScenario(house, unit, command, isRepeat, false))
  {
    // Make sure that two or more repetitions are used for bright and dim,
    // to avoid that commands are beeing buffered seperately when repeated
    if(command == CMD_BRIGHT || command == CMD_DIM)
    {
      x10ex.sendCmd(house, unit, command, 2);
    }
    // Only repeat bright and dim commands
    else if(!isRepeat)
    {
      // Handle Address Command (House + Unit)
      if(command == CMD_ADDRESS)
      {
        x10ex.sendAddress(house, unit, 1);
      }
      // Other commands map directly: just forward to PL interface
      else
      {
        x10ex.sendCmd(house, unit, command, 1);
      }
    }
  }
}

// Process serial data messages received from computer over USB, Bluetooth, e.g.
//
// Serial messages are 3 or 9 bytes long. Use hex 0-F to address units and send commands.
// Bytes must be sent within one second (defined threshold) from the first to the last
// Below are some examples:
//
// Standard Messages examples:
// A12 (House=A, Unit=2, Command=On)
// AB3 (House=A, Unit=12, Command=Off)
// A_5 (House=A, Unit=N/A, Command=Bright)
// |||
// ||+-- Command 0-F or _  Example: 2 = On, 7 = ExtendedCode and _ = No Command
// |+--- Unit 0-F or _     Example: 0 = Unit 1, F = Unit 16 and _ = No unit
// +---- House code A-P    Example: A = House A and P = House P :)
//
// Extended Message examples:
// A37x31x21 (House=A, Unit=4, Command=ExtendedCode, Extended Command=PreSetDim, Extended Data=33)
// B87x01x0D (House=B, Unit=9, Command=ExtendedCode, Extended Command=ShutterOpen, Extended Data=13)
//     |/ |/
//     |  +-- Extended Data byte in hex     Example: 01 = 1%, 1F = 50% and 3E = 100% brightness (range is decimal 0-62)
//     +----- Extended Command byte in hex  Example: 31 = PreSetDim, for more examples check the X10 ExtendedCode spec.
//
// Scenario Execute examples:
// S03 (Execute scenario 3)
// S14 (Execute scenario 20)
// ||/
// |+--- Scenario byte in hex (Hex: 00-FF, Dec: 0-255)
// +---- Scenario Execute Character
//
// Request Module State examples:
// R** (Request buffered state of all modules)
// RG* (Request buffered state of modules using house code G)
// RA2 (Request buffered state of module A3)
// |||
// ||+-- Unit 0-F or *        Example: 0 = Unit 1, A = Unit 10 and * = All units
// |+--- House code A-P or *  Example: A = House A, P = House P and * = All house codes
// +---- Request Module State Character
//
// Wipe Module State examples:
// RW* (Wipe state data for all modules)
// RWB (Wipe state data for all modules using house code B)
// |||
// ||+-- House code A-P or *  Example: A = House A, P = House P and * = All house codes
// |+--- Wipe Module State Character
// +---- Request Module State Character
//
void processSdMessage()
{
  if(Serial.available() >= 3)
  {
    byte byte1 = toupper(Serial.read());
    byte byte2 = toupper(Serial.read());
    byte byte3 = toupper(Serial.read());
    if(process3BMessage(SERIAL_DATA_MSG, byte1, byte2, byte3))
    {
      // Return error message if message sent to X10ex was not buffered successfully
      Serial.println(POWER_LINE_BUFFER_ERROR);
    }
    sdReceived = 0;
  }
  // If partial message was received
  if(Serial.available() && Serial.available() < 3)
  {
    // Store received time
    if(!sdReceived)
    {
      sdReceived = millis();
    }
    // Clear message if all bytes were not received within threshold
    else if(sdReceived > millis() || millis() - sdReceived > SERIAL_DATA_THRESHOLD)
    {
      bmHouse = 0;
      bmExtCommand = 0;
      sdReceived = 0;
      Serial.println(SERIAL_DATA_TIMEOUT);
      Serial.flush();
    }
  }
}

// Processes and executes 3 byte serial and ethernet messages.
bool process3BMessage(const char type[], byte byte1, byte byte2, byte byte3)
{
  bool x10exBufferError = 0;
  // Convert byte2 from hex to decimal unless command is request module state
  if(byte1 != 'R') byte2 = charHexToDecimal(byte2);
  // Convert byte3 from hex to decimal unless command is wipe module state
  if(byte1 != 'R' || byte2 != 'W') byte3 = charHexToDecimal(byte3);
  // Check if standard message was received (byte1 = House, byte2 = Unit, byte3 = Command)
  if(byte1 >= 'A' && byte1 <= 'P' && (byte2 <= 0xF || byte2 == '_') && (byte3 <= 0xF || byte3 == '_') && (byte2 != '_' || byte3 != '_'))
  {
    // Store first 3 bytes of message to make it possible to process 9 byte serial messages
    bmHouse = byte1;
    bmUnit = byte2 == '_' ? 0 : byte2 + 1;
    bmCommand = byte3 == '_' ? CMD_STATUS_REQUEST : byte3;
    bmExtCommand = 0;
    // Send standard message if command type is not extended
    if(bmCommand != CMD_EXTENDED_CODE && bmCommand != CMD_EXTENDED_DATA)
    {
      printX10Message(type, bmHouse, bmUnit, byte3, 0, 0, 8 * Serial.available());
      // Check if command is handled by scenario; if not continue
      if(!handleUnitScenario(bmHouse, bmUnit, bmCommand, false, true))
      {
        x10exBufferError = x10ex.sendCmd(bmHouse, bmUnit, bmCommand, bmCommand == CMD_BRIGHT || bmCommand == CMD_DIM ? 2 : 1);
      }        
      bmHouse = 0;
    }
  }
  // Check if extended message was received (byte1 = Hex Seperator, byte2 = Hex 1, byte3 = Hex 2)
  else if(byte1 == 'X' && bmHouse)
  {
    byte data = byte2 * 16 + byte3;
    // No extended command set, assume that we are receiving command
    if(!bmExtCommand)
    {
      bmExtCommand = data;
    }
    // Extended command set, we must be receiving extended data
    else
    {
      printX10Message(type, bmHouse, bmUnit, bmCommand, data, bmExtCommand, 8 * Serial.available());
      x10exBufferError = x10ex.sendExt(bmHouse, bmUnit, bmCommand, data, bmExtCommand, 1);
      bmHouse = 0;
    }
  }
  // Check if scenario execute command was received (byte1 = Scenario Character, byte2 = Hex 1, byte3 = Hex 2)
  else if(byte1 == 'S')
  {
    byte scenario = byte2 * 16 + byte3;
    Serial.print(type);
    Serial.print("S");
    if(scenario <= 0xF) { Serial.print("0"); }
    Serial.println(scenario, HEX);
    handleSdScenario(scenario);
  }
  // Check if request module state command was received (byte1 = Request State Character, byte2 = House, byte3 = Unit)
  else if(byte1 == 'R' && ((byte2 >= 'A' && byte2 <= 'P') || byte2 == '*'))
  {
    Serial.print(type);
    Serial.print("R");
    Serial.print(byte2);
    // All modules
    if(byte2 == '*')
    {
      Serial.println('*');
      for(short i = 0; i < 256; i++)
      {
        sdPrintModuleState((i >> 4) + 0x41, i & 0xF);
      }
    }
    else
    {
      if(byte3 <= 0xF)
      {
        Serial.println(byte3, HEX);
        sdPrintModuleState(byte2, byte3);
      }
      // All units using specified house code
      else
      {
        Serial.println('*');
        for(byte i = 0; i < 16; i++)
        {
          sdPrintModuleState(byte2, i);
        }
      }
    }
  }
  // Check if request wipe module state command was received (byte1 = Request State Character, byte2 = Wipe Character, byte3 = House)
  else if(byte1 == 'R' && byte2 == 'W' && ((byte3 >= 'A' && byte3 <= 'P') || byte3 == '*'))
  {
    Serial.print(type);
    Serial.print("RW");
    Serial.println((char)byte3);
    x10ex.wipeModuleState(byte3);
    Serial.print(MODULE_STATE_MSG);
    Serial.print(byte3 >= 'A' && byte3 <= 'P' ? (char)byte3 : '*');
    Serial.println("__");
  }
  // Future enhancements:
  // QT = Query Type, QN = Query Name
  // UT = Update Type, UN = Update Name
  // Unknown command/data
  else
  {
    Serial.print(type);
    Serial.println(MSG_DATA_ERROR);
  }
  return x10exBufferError;
}

void printX10Message(const char type[], char house, byte unit, byte command, byte extData, byte extCommand, int remainingBits)
{
  printX10TypeHouseUnit(type, house, unit, command);
  // Ignore non X10 commands like the CMD_ADDRESS command used by the IR library
  if(command <= 0xF)
  {
    Serial.print(command, HEX);
    if(extCommand || (extData && (command == CMD_STATUS_ON || command == CMD_STATUS_OFF)))
    {
      printX10ByteAsHex(extCommand);
      printX10ByteAsHex(extCommand == EXC_PRE_SET_DIM ? extData & B111111 : extData);
    }
  }
  else
  {
    Serial.print("_");
  }
  Serial.println();
#if DEBUG
  printDebugX10Message(type, house, unit, command, extData, extCommand, remainingBits);
#endif
}

void printX10TypeHouseUnit(const char type[], char house, byte unit, byte command)
{
  Serial.print(type);
  Serial.print(house);
  if(
    unit &&
    unit != DATA_UNKNOWN/* &&
    command != CMD_ALL_UNITS_OFF &&
    command != CMD_ALL_LIGHTS_ON &&
    command != CMD_ALL_LIGHTS_OFF &&
    command != CMD_HAIL_REQUEST*/)
  {
    Serial.print(unit - 1, HEX);
  }
  else
  {
    Serial.print("_");
  }
}

void sdPrintModuleState(char house, byte unit)
{
  unit++;
  X10state state = x10ex.getModuleState(house, unit);
  if(state.isSeen)
  {
    printX10Message(
      MODULE_STATE_MSG, house, unit,
      state.isKnown ? state.isOn ? CMD_STATUS_ON : CMD_STATUS_OFF : DATA_UNKNOWN,
      state.data,
      0, 0);
  }
}

void printX10ByteAsHex(byte data)
{
  Serial.print("x");
  if(data <= 0xF) { Serial.print("0"); }
  Serial.print(data, HEX);
}

byte charHexToDecimal(byte input)
{
  // 0123456789  =>  0-15
  if(input >= 0x30 && input <= 0x39) input -= 0x30;
  // ABCDEF  =>  10-15
  else if(input >= 0x41 && input <= 0x46) input -= 0x37;
  // Return converted byte
  return input;
}

// Handles scenario execute commands received as serial data message
void handleSdScenario(byte scenario)
{
  switch(scenario)
  {
    //////////////////////////////
    // Sample Code
    // Replace with your own setup
    //////////////////////////////
    case 0x01: sendAllLightsOn(); break;
    case 0x02: sendHallAndKitchenOn(); break;
    case 0x03: sendLivingRoomOn(); break;
    case 0x04: sendLivingRoomTvScenario(); break;
    case 0x05: sendLivingRoomMovieScenario(); break;
    case 0x11: sendAllLightsOff(); break;
    case 0x12: sendHallAndKitchenOff(); break;
    case 0x13: sendLivingRoomOff(); break;
  }
}

// Handles scenarios triggered when receiving unit on/off commands
// Use this method to trigger scenarios from simple RF/IR remotes
bool handleUnitScenario(char house, byte unit, byte command, bool isRepeat, bool failOnBufferError)
{
  //////////////////////////////
  // Sample Code
  // Replace with your own setup
  //////////////////////////////

  bool bufferError = 0;

  // Ignore all house codes except A
  if(house != 'A') return 0;

  // Unit 4 is an old X10 lamp module that doesn't remember state on its own. Use the
  // following method to revert to buffered state when these modules are turned on
  if(unit == 4)
  {
    bufferError = handleOldLampModuleState(house, unit, command, isRepeat);
  }  
  // Unit 5 is a placeholder unit code used by RF and IR remotes that triggers HallAndKitchen scenario
  else if(unit == 5)
  {
    if(!isRepeat)
    {
      if(command == CMD_ON) bufferError = sendHallAndKitchenOn();
      else if(command == CMD_OFF) bufferError = sendHallAndKitchenOff();
    }
  }
  // Unit 10 is a placeholder unit code used by RF and IR remotes that triggers AllLights scenario
  else if(unit == 10)
  {
    if(!isRepeat)
    {
      if(command == CMD_ON) bufferError = sendAllLightsOn();
      else if(command == CMD_OFF) bufferError = sendAllLightsOff();
    }
  }
  // Unit 11 is a placeholder unit code used by RF and IR remotes that triggers LivingRoom scenario
  else if(unit == 11)
  {
    if(!isRepeat)
    {
      if(command == CMD_ON) bufferError = sendLivingRoomOn();
      else if(command == CMD_OFF) bufferError = sendLivingRoomOff();
    }
  }
  // Unit 12 is a placeholder unit code used by RF and IR remotes that triggers LivingRoomTv scenario
  else if(unit == 12)
  {
    if(!isRepeat)
    {
      if(command == CMD_ON) bufferError = sendLivingRoomTvScenario();
      else if(command == CMD_OFF) bufferError = sendLivingRoomOn();
    }
  }
  // Unit 13 is a placeholder unit code used by RF and IR remotes that triggers LivingRoomMovie scenario
  else if(unit == 13)
  {
    if(!isRepeat)
    {
      if(command == CMD_ON) bufferError = sendLivingRoomMovieScenario();
      else if(command == CMD_OFF) bufferError = sendLivingRoomOn();
    }
  }
  else
  {
    return 0;
  }
  return !failOnBufferError || !bufferError;
}

// Handles old X10 lamp modules that don't remember state on their own
bool handleOldLampModuleState(char house, byte unit, byte command, bool isRepeat)
{
  if(!isRepeat)
  {
    bool bufferError = 0;
    if(command == CMD_ON)
    {
      // Get state from buffer, and set to last brightness when turning on
      X10state state = x10ex.getModuleState(house, unit);
      if(state.isKnown && !state.isOn && state.data > 0)
      {
        bufferError = x10ex.sendExt(house, unit, CMD_EXTENDED_CODE, state.data, EXC_PRE_SET_DIM, 1);
      }
    }
    return bufferError || x10ex.sendCmd(house, unit, command, 1);
  }
  return 0;
}

//////////////////////////////
// Scenarios
// Replace with your own setup
//////////////////////////////

bool sendAllLightsOn()
{
  return
    // Bedroom
    x10ex.sendExtDim('A', 7, 80, EXC_DIM_TIME_4, 1) ||
    // Livingroom table
    x10ex.sendExtDim('A', 2, 70, EXC_DIM_TIME_4, 1) ||
    // Hall
    x10ex.sendExtDim('A', 8, 75, EXC_DIM_TIME_4, 1) ||
    // Livingroom couch
    x10ex.sendExtDim('A', 3, 90, EXC_DIM_TIME_4, 1) ||
    // Kitchen
    x10ex.sendExtDim('A', 9, 100, EXC_DIM_TIME_4, 1) ||
    // Livingroom shelves
    x10ex.sendExtDim('A', 4, 40, EXC_DIM_TIME_4, 1);
}

bool sendAllLightsOff()
{
  return
    x10ex.sendCmd('A', 7, CMD_OFF, 1) ||
    x10ex.sendCmd('A', 2, CMD_OFF, 1) ||
    x10ex.sendCmd('A', 8, CMD_OFF, 1) ||
    x10ex.sendCmd('A', 3, CMD_OFF, 1) ||
    x10ex.sendCmd('A', 9, CMD_OFF, 1) ||
    x10ex.sendCmd('A', 4, CMD_OFF, 1);
}

bool sendHallAndKitchenOn()
{
  return
    x10ex.sendExtDim('A', 8, 75, EXC_DIM_TIME_4, 1) ||
    x10ex.sendExtDim('A', 9, 100, EXC_DIM_TIME_4, 1);
}

bool sendHallAndKitchenOff()
{
  return
    x10ex.sendCmd('A', 8, CMD_OFF, 1) ||
    x10ex.sendCmd('A', 9, CMD_OFF, 1);
}

bool sendLivingRoomOn()
{
  return
    x10ex.sendExtDim('A', 2, 70, EXC_DIM_TIME_4, 1) ||
    x10ex.sendExtDim('A', 3, 90, EXC_DIM_TIME_4, 1) ||
    x10ex.sendExtDim('A', 4, 40, EXC_DIM_TIME_4, 1);
}

bool sendLivingRoomOff()
{
  return
    x10ex.sendCmd('A', 2, CMD_OFF, 1) ||
    x10ex.sendCmd('A', 3, CMD_OFF, 1) ||
    x10ex.sendCmd('A', 4, CMD_OFF, 1);
}

bool sendLivingRoomTvScenario()
{
  return
    x10ex.sendExtDim('A', 2, 40, EXC_DIM_TIME_4, 1) ||
    x10ex.sendExtDim('A', 3, 30, EXC_DIM_TIME_4, 1) ||
    x10ex.sendExtDim('A', 4, 25, EXC_DIM_TIME_4, 1);
}

bool sendLivingRoomMovieScenario()
{
  return
    x10ex.sendCmd('A', 2, CMD_OFF, 1) ||
    x10ex.sendCmd('A', 3, CMD_OFF, 1) ||
    x10ex.sendExtDim('A', 4, 25, EXC_DIM_TIME_4, 1);
}

#if DEBUG

void printDebugX10Message(const char type[], char house, byte unit, byte command, byte extData, byte extCommand, int remainingBits)
{
  Serial.print("DEBUG=");
  printX10TypeHouseUnit(type, house, unit, command);
  switch(command)
  {
    // This is not a real X10 command, it's a special command used by the IR
    // library to signal that an address and a house code has been received
    case CMD_ADDRESS:
      break;
    case CMD_ALL_UNITS_OFF:
      Serial.println("_AllUnitsOff");
      break;
    case CMD_ALL_LIGHTS_ON:
      Serial.println("_AllLightsOn");
      break;
    case CMD_ON:
      Serial.print("_On");
      printDebugX10Brightness("_Brightness", extData);
      break;
    case CMD_OFF:
      Serial.print("_Off");
      printDebugX10Brightness("_Brightness", extData);
      break;
    case CMD_DIM:
      Serial.println("_Dim");
      break;
    case CMD_BRIGHT:
      Serial.println("_Bright");
      break;
    case CMD_ALL_LIGHTS_OFF:
      Serial.println("_AllLightsOff");
      break;
    case CMD_EXTENDED_CODE:
      Serial.print("_ExtendedCode");
      break;
    case CMD_HAIL_REQUEST:
      Serial.println("_HailReq");
      break;
    case CMD_HAIL_ACKNOWLEDGE:
      Serial.println("_HailAck");
      break;
    // Enable X10_USE_PRE_SET_DIM in X10ex header file
    // to use X10 standard message PRE_SET_DIM commands
    case CMD_PRE_SET_DIM_0:
    case CMD_PRE_SET_DIM_1:
      printDebugX10Brightness("_PreSetDim", extData);
      break;
    case CMD_EXTENDED_DATA:
      Serial.print("_ExtendedData");
      break;
    case CMD_STATUS_ON:
      Serial.println("_StatusOn");
      break;
    case CMD_STATUS_OFF:
      Serial.println("_StatusOff");
      break;
    case CMD_STATUS_REQUEST:
      Serial.println("_StatusReq");
      break;
    case DATA_UNKNOWN:
      Serial.println("_Unknown");
      break;
    default:
      Serial.println();
  }
  if(extCommand)
  {
    switch(extCommand)
    {
      case EXC_PRE_SET_DIM:
        printDebugX10Brightness("_PreSetDim", extData);
        break;
      default:
        Serial.print("_");
        Serial.print(extCommand, HEX);
        Serial.print("_");
        Serial.println(extData, HEX);
    }
  }
  if(remainingBits)
  {
    printX10TypeHouseUnit(type, house, unit, command);
    Serial.print("_ErrorBitCount=");
    Serial.println(remainingBits, DEC);
  }
}

void printDebugX10Brightness(const char source[], byte extData)
{
  if(extData > 0)
  {
    Serial.print(source);
    Serial.print("_");
    Serial.println(x10ex.x10BrightnessToPercent(extData), DEC);
  }
  else
  {
    Serial.println();
  }
}

#endif
