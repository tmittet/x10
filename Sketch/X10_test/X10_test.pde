/************************************************************************/
/* X10 X10 PLC, RF, IR library test sketch, v1.2.                       */
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
/* Written by Thomas Mittet thomas@mittet.nu September 2010.            */
/************************************************************************/

#include <X10ex.h>
#include <X10rf.h>
#include <X10ir.h>

#define SERIAL_DATA_MSG "SD_"
#define SERIAL_DATA_TIMEOUT "SD_X_TIMEOUT"
#define SERIAL_DATA_ERROR "SD_X_ERROR"
#define MODULE_STATE_MSG "MS_"
#define POWER_LINE_MSG "PL_"
#define POWER_LINE_ERROR_BITS "_ERROR_UNEXPECTEDBITS_"
#define RADIO_FREQ_MSG "RF_"
#define INFRARED_MSG "IR_"

// Fields used for serial message reception
unsigned long scReceived;
char scHouse;
byte scUnit;
byte scCommand;

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
}

void loop()
{
  processSdMessage();
}

// Process serial data messages received from computer over USB, Bluetooth, e.g.
void processSdMessage()
{
  // Serial messages must be 3 or 6 bytes long
  // Bytes must be sent within one second from the first to the last
  // To understand the format, looking up an ASCII table is a good start
  // Below are some examples:
  // Standard message: A12 (House=A, Unit=1, Command=On)
  // Standard message: AB3 (House=A, Unit=11, Command=Off)
  // Standard message: A05 (House=A, Unit=0, Command=Bright)
  // Extended message: A47X!1 (House=A, Unit=4, Command=Extended Code, Extended Seperator=X, Extended Data=33, Extended Code=Pre Set Dim)
  // Scenario execute: S03 (Execute scenario 3)
  // Scenario execute: S14 (Execute scenario 14)
  // Request modstate: RA2 (Request buffered state of module A2)
  // Wipe modulestate: RWX (Wipe state data for all modules)
  if(Serial.available() >= 3)
  {
    byte byte1 = Serial.read();
    byte byte2 = Serial.read();
    byte byte3 = Serial.read();
    byte extData;
    byte extCommand;
    // Convert lower case letters to upper case
    byte1 = charToUpper(byte1);
    // If not extended message convert byte 2 and 3 ASCII 0-9 and A-F to decimal 0-15
    if(byte1 != 'X')
    {
      // No byte 2 hex to decimal conversion for status requests, since byte 2 is used as house code
      byte2 = byte1 == 'R' ? charToUpper(byte2) : charHexToDecimal(byte2);
      byte3 = charHexToDecimal(byte3);
    }
    // Check if standard message was received (byte1 = House, byte2 = Unit, byte3 = Command)
    if(byte1 >= 'A' && byte1 <= 'P' && byte2 <= 16 && byte3 <= 0xF)
    {
      // Store first 3 bytes of message to make it possible to process 6 byte serial messages
      scHouse = byte1;
      scUnit = byte2;
      scCommand = byte3;
      // Send standard message if command type is not extended
      if(scCommand != CMD_EXTENDED_CODE && scCommand != CMD_EXTENDED_DATA)
      {
        printX10Message(SERIAL_DATA_MSG, scHouse, scUnit, scCommand, 0, 0, 8 * Serial.available());
        // Check if command is handled by scenario, if not continue
        if(!handleUnitScenario(scHouse, scUnit, scCommand, 0))
        {
          x10ex.sendCmd(scHouse, scUnit, scCommand, 2);
        }        
        scHouse = 0;
        Serial.flush();
      }
    }
    // Check if extended message was received (byte1 = Extended Seperator, byte2 = Extended Data, byte3 = Extended Command)
    else if(byte1 == 'X' && byte3 && scHouse && (scCommand == CMD_EXTENDED_CODE || scCommand == CMD_EXTENDED_DATA))
    {
      printX10Message(SERIAL_DATA_MSG, scHouse, scUnit, scCommand, byte2, byte3, 8 * Serial.available());
      x10ex.sendExt(scHouse, scUnit, scCommand, byte2, byte3, 2);
      scHouse = 0;
      Serial.flush();
    }
    // Check if scenario execute was received (byte1 = Scenario Seperator, byte2 = Decimal * 10, byte3 = Decimal)
    else if(byte1 == 'S' && byte2 <= 9 && byte3 <= 9)
    {
      byte scenario = byte2 * 10 + byte3;
      Serial.print(SERIAL_DATA_MSG);
      Serial.print("X_SCENARIO_EXEC_");
      Serial.println(scenario, DEC);
      handleSdScenario(scenario);
      Serial.flush();
    }
    // Check if module status request was received (byte1 = Status Request Seperator, byte2 = House, byte3 = Unit)
    else if(byte1 == 'R' && byte2 >= 'A' && byte2 <= 'P' && byte3 > 0 && byte3 <= 16)
    {
      printX10TypeHouseUnit(SERIAL_DATA_MSG, byte2, byte3, DATA_UNKNOWN);
      Serial.println("_RSTATE");
      X10state state = x10ex.getModuleState(byte2, byte3);
      byte command = state.isKnown ? state.isOn ? CMD_ON : CMD_OFF : DATA_UNKNOWN;
      printX10Message(MODULE_STATE_MSG, byte2, byte3, command, state.data, 0, 0);
      Serial.flush();
    }
    else if(byte1 == 'R' && byte2 == 'W' && byte3 == 'X')
    {
      Serial.print(SERIAL_DATA_MSG);
      Serial.println("X_WIPESTATE");
      x10ex.wipeModuleState();
    }
    // Unknown data
    else
    {
      Serial.println(SERIAL_DATA_ERROR);
    }
    scReceived = 0;
  }
  // If partial message was received
  if(Serial.available() && Serial.available() < 3)
  {
    // Store received time
    if(!scReceived)
    {
      scReceived = millis();
    }
    // Clear message if all bytes were not received within one second
    else if(scReceived > millis() || millis() - scReceived > 1000)
    {
      scHouse = 0;
      scReceived = 0;
      Serial.println(SERIAL_DATA_TIMEOUT);
      Serial.flush();
    }
  }
}

// Process messages received from X10 modules over the power line
void processPlMessage(char house, byte unit, byte command, byte extData, byte extCommand, byte remainingBits)
{
  printX10Message(POWER_LINE_MSG, house, unit, command, extData, extCommand, remainingBits);
}

// Process commands received from X10 compatible RF remote
void processRfCommand(char house, byte unit, byte command, bool isRepeat)
{
  if(!isRepeat)
  {
    printX10Message(RADIO_FREQ_MSG, house, unit, command, 0, 0, 0);
  }
  // Check if command is handled by scenario, if not continue
  if(!handleUnitScenario(house, unit, command, isRepeat))
  {
    // Other commands map directly, just forward to PL interface
    // Make sure that two repetitions or more are used for bright and dim,
    // to avoid that commands are beeing sent seperately when repeated
    x10ex.sendCmd(house, unit, command, 2);
  }
}

// Process commands received from X10 compatible IR remote
void processIrCommand(char house, byte unit, byte command, bool isRepeat)
{
  if(!isRepeat)
  {
    printX10Message(INFRARED_MSG, house, unit, command, 0, 0, 0);
  }
  // Check if command is handled by scenario, if not continue
  if(!handleUnitScenario(house, unit, command, isRepeat))
  {
    // Handle Address Command (House + Unit)
    if(command == CMD_ADDRESS)
    {
      x10ex.sendAddress(house, unit, 1);
    }
    // Other commands map directly, just forward to PL interface
    else
    {
      // Make sure that two repetitions or more are used for bright and dim,
      // to avoid that commands are beeing sent seperately when repeated
      x10ex.sendCmd(house, unit, command, 2);
    }
  }
}

void printX10Message(const char type[], char house, byte unit, byte command, byte extData, byte extCommand, int remainingBits)
{
  printX10TypeHouseUnit(type, house, unit, command);
  switch(command)
  {
    // This is not a real X10 command, it's a special command used by the IR
    // library to signal that an address and a house code has been received
    case CMD_ADDRESS:
      Serial.println();
      break;
    case CMD_ALL_UNITS_OFF:
      Serial.println("_UOFF");
      break;
    case CMD_ALL_LIGHTS_ON:
      Serial.println("_LON");
      break;
    case CMD_ON:
      Serial.print("_ON");
      printX10Brightness("_BRI", extData);
      break;
    case CMD_OFF:
      Serial.println("_OFF");
      break;
    case CMD_DIM:
      Serial.println("_DIM");
      break;
    case CMD_BRIGHT:
      Serial.println("_BRI");
      break;
    case CMD_ALL_LIGHTS_OFF:
      Serial.println("_LOFF");
      break;
    case CMD_EXTENDED_CODE:
      Serial.print("_EXC");
      break;
    case CMD_HAIL_REQUEST:
      Serial.println("_HRQ");
      break;
    case CMD_HAIL_ACKNOWLEDGE:
      Serial.println("_HAC");
      break;
    case CMD_PRE_SET_DIM_0:
    case CMD_PRE_SET_DIM_1:
      printX10Brightness("_PSD", extData);
      break;
    case CMD_EXTENDED_DATA:
      Serial.print("_EXD");
      break;
    case CMD_STATUS_ON:
      Serial.println("_SON");
      break;
    case CMD_STATUS_OFF:
      Serial.println("_SOFF");
      break;
    case CMD_STATUS_REQUEST:
      Serial.println("_SRQ");
      break;
    case DATA_UNKNOWN:
      Serial.println("_UNKNOWN");
      break;
  }
  if(extCommand)
  {
    switch(extCommand)
    {
      case EXC_PRE_SET_DIM:
        printX10Brightness("_PSD", extData);
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
    Serial.println(POWER_LINE_ERROR_BITS);
    Serial.println(remainingBits, DEC);
  }
}

void printX10TypeHouseUnit(const char type[], char house, byte unit, byte command)
{
  Serial.print(type);
  Serial.print(house);
  if(
    unit &&
    unit != DATA_UNKNOWN &&
    command != CMD_ALL_UNITS_OFF &&
    command != CMD_ALL_LIGHTS_ON &&
    command != CMD_ALL_LIGHTS_OFF &&
    command != CMD_HAIL_REQUEST)
  {
    Serial.print(unit, DEC);
  }
}

void printX10Brightness(const char source[], byte extData)
{
  if(extData > 0)
  {
    Serial.print(source);
    Serial.print("_");
    Serial.println(round((extData & B111111) * 100 / 62.0), DEC);
  }
  else
  {
    Serial.println("");
  }
}

byte charToUpper(byte input)
{
  if(input >= 0x61) input -= 0x20;
  // Return converted byte
  return input;  
}

byte charHexToDecimal(byte input)
{
  // Make sure all characters are upper case
  input = charToUpper(input);
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
    case 01: sendAllLightsOn(); break;
    case 02: sendHallAndKitchenOn(); break;
    case 03: sendLivingRoomOn(); break;
    case 04: sendLivingRoomTvScenario(); break;
    case 05: sendLivingRoomMovieScenario(); break;
    case 11: sendAllLightsOff(); break;
    case 12: sendHallAndKitchenOff(); break;
    case 13: sendLivingRoomOff(); break;
  }
}

// Handles scenarios triggered when receiving unit on/off commands
// Use this method to trigger scenarios from simple RF/IR remotes
bool handleUnitScenario(char house, byte unit, byte command, bool isRepeat)
{
  // Ignore all house codes except A
  if(house != 'A')
  {
    return 0;
  }
  
  // Unit 4 is an old X10 lamp module that doesn't remember state on its own, use the
  // following method to revert to buffered state when these modules are turned on
  if(unit == 4)
  {
    handleOldLampModuleState(house, unit, command, isRepeat);
  }
  
  // Unit 5 is a placeholder unit code used by RF and IR remotes that triggers HallAndKitchen scenario
  if(unit == 5)
  {
    if(!isRepeat)
    {
      if(command == CMD_ON)
      {
        sendHallAndKitchenOn();
      }
      else if(command == CMD_OFF)
      {
        sendHallAndKitchenOff();
      }
    }
  }
  // Unit 10 is a placeholder unit code used by RF and IR remotes that triggers AllLights scenario
  else if(unit == 10)
  {
    if(!isRepeat)
    {
      if(command == CMD_ON)
      {
        sendAllLightsOn();
      }
      else if(command == CMD_OFF)
      {
        sendAllLightsOff();
      }
    }
  }
  // Unit 11 is a placeholder unit code used by RF and IR remotes that triggers LivingRoom scenario
  else if(unit == 11)
  {
    if(!isRepeat)
    {
      if(command == CMD_ON)
      {
        sendLivingRoomOn();
      }
      else if(command == CMD_OFF)
      {
        sendLivingRoomOff();
      }
    }
  }
  // Unit 12 is a placeholder unit code used by RF and IR remotes that triggers LivingRoomTv scenario
  else if(unit == 12)
  {
    if(!isRepeat)
    {
      if(command == CMD_ON)
      {
        sendLivingRoomTvScenario();
      }
      else if(command == CMD_OFF)
      {
        sendLivingRoomOn();
      }
    }
  }
  // Unit 13 is a placeholder unit code used by RF and IR remotes that triggers LivingRoomMovie scenario
  else if(unit == 13)
  {
    if(!isRepeat)
    {
      if(command == CMD_ON)
      {
        sendLivingRoomMovieScenario();
      }
      else if(command == CMD_OFF)
      {
        sendLivingRoomOn();
      }
    }
  }
  else
  {
    return 0;
  }
  return 1;
}

// Handles old X10 lamp modules that don't remember state on their own
void handleOldLampModuleState(char house, byte unit, byte command, bool isRepeat)
{
  if(command == CMD_ON && !isRepeat)
  {
    // Get state from buffer and set it to last brightness when turning on
    X10state state = x10ex.getModuleState(house, unit);
    if(state.isKnown && !state.isOn && state.data > 0)
    {
      x10ex.sendExt(house, unit, CMD_EXTENDED_CODE, state.data, EXC_PRE_SET_DIM, 1);
    }
  }
}

////////////////
// Scenarios
////////////////

void sendAllLightsOn()
{
  // Bedroom
  x10ex.sendExtDim('A', 7, 80, EXC_DIM_TIME_4, 1);
  // Livingroom table
  x10ex.sendExtDim('A', 2, 70, EXC_DIM_TIME_4, 1);
  // Hall
  x10ex.sendExtDim('A', 8, 75, EXC_DIM_TIME_4, 1);
  // Livingroom couch
  x10ex.sendExtDim('A', 3, 90, EXC_DIM_TIME_4, 1);
  // Kitchen
  x10ex.sendExtDim('A', 9, 100, EXC_DIM_TIME_4, 1);
  // Livingroom shelves
  x10ex.sendExtDim('A', 4, 40, EXC_DIM_TIME_4, 1);
}

void sendAllLightsOff()
{
  x10ex.sendCmd('A', 7, CMD_OFF, 1);
  x10ex.sendCmd('A', 2, CMD_OFF, 1);
  x10ex.sendCmd('A', 8, CMD_OFF, 1);
  x10ex.sendCmd('A', 3, CMD_OFF, 1);
  x10ex.sendCmd('A', 9, CMD_OFF, 1);
  x10ex.sendCmd('A', 4, CMD_OFF, 1);
}

void sendHallAndKitchenOn()
{
  x10ex.sendExtDim('A', 8, 75, EXC_DIM_TIME_4, 1);
  x10ex.sendExtDim('A', 9, 100, EXC_DIM_TIME_4, 1);
}

void sendHallAndKitchenOff()
{
  x10ex.sendCmd('A', 8, CMD_OFF, 1);
  x10ex.sendCmd('A', 9, CMD_OFF, 1);
}

void sendLivingRoomOn()
{
  x10ex.sendExtDim('A', 2, 70, EXC_DIM_TIME_4, 1);
  x10ex.sendExtDim('A', 3, 90, EXC_DIM_TIME_4, 1);
  x10ex.sendExtDim('A', 4, 40, EXC_DIM_TIME_4, 1); 
}

void sendLivingRoomOff()
{
  x10ex.sendCmd('A', 2, CMD_OFF, 1);
  x10ex.sendCmd('A', 3, CMD_OFF, 1);
  x10ex.sendCmd('A', 4, CMD_OFF, 1);
}

void sendLivingRoomTvScenario()
{
  x10ex.sendExtDim('A', 2, 40, EXC_DIM_TIME_4, 1);
  x10ex.sendExtDim('A', 3, 30, EXC_DIM_TIME_4, 1);
  x10ex.sendExtDim('A', 4, 25, EXC_DIM_TIME_4, 1); 
}

void sendLivingRoomMovieScenario()
{
  x10ex.sendCmd('A', 2, CMD_OFF, 1);
  x10ex.sendCmd('A', 3, CMD_OFF, 1);
  x10ex.sendExtDim('A', 4, 25, EXC_DIM_TIME_4, 1); 
}
