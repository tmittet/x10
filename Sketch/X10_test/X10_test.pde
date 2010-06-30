/************************************************************************/
/* X10 X10 PLC, RF, IR library test sketch, v1.0.                       */
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
/* Written by Thomas Mittet thomas@mittet.nu June 2010.                 */
/************************************************************************/

#include <X10ex.h>
#include <X10rf.h>
#include <X10ir.h>

// Fields used for serial message reception
unsigned long scReceived;
char scHouse;
byte scUnit;
byte scCommand;

// zeroCrossInt = 2 (pin change interrupt), zeroCrossPin = 4, transmitPin = 5, receivePin = 6, receiveTransmits = true
X10ex x10ex = X10ex(2, 4, 5, 6, true, processPlMessage);
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
  if(Serial.available() >= 3)
  {
    byte byte1 = Serial.read();
    byte byte2 = Serial.read();
    byte byte3 = Serial.read();
    byte extData;
    byte extCommand;
    // Convert lower case letters to upper case
    if(byte1 >= 0x61) byte1 -= 0x20;
    // If not extended message convert ASCII 0-9... to decimal 0-16
    if(byte1 != 0x58)
    {
      // 0123456789  =>  0-9
      if(byte2 >= 0x30 && byte2 <= 0x39) byte2 -= 0x30;
      // ABCDEFG  =>  10-16
      else if(byte2 >= 0x41 && byte2 <= 0x47) byte2 -= 37;
      // 0123456789  =>  0-16
      if(byte3 >= 0x30 && byte3 <= 0x39) byte3 -= 0x30;
      // ABCDEFG  =>  10-16
      else if(byte3 >= 0x41 && byte3 <= 0x47) byte3 -= 37;
    }
    // Check if standard message was received
    if(byte1 >= 0x41 && byte1 <= 0x50 && byte2 <= 16 && byte3 <= 0xF)
    {
      scHouse = byte1;
      scUnit = byte2;
      scCommand = byte3;
      // Send standard message
      if(scCommand != B0111 && scCommand != B1100)
      {
        printX10Message("SM_", scHouse, scUnit, scCommand, 0, 0, 8 * Serial.available());
        x10ex.sendCmd(scHouse, scUnit, scCommand, 2);
        scHouse = 0;
        Serial.flush();
      }
    }
    // Check if extended message was received (Extended seperator X = 0x58)
    else if(byte1 == 0x58 && byte3 && scHouse && (scCommand == B0111 || scCommand == B1100))
    {
      printX10Message("SM_", scHouse, scUnit, scCommand, byte2, byte3, 8 * Serial.available());
      x10ex.sendExt(scHouse, scUnit, scCommand, byte2, byte3, 2);
      scHouse = 0;
      Serial.flush();
    }
    // Check if scenario execute was received (Scenario seperator S = 0x53)
    else if(byte1 == 0x53 && byte2 <= 9 && byte3 <= 9)
    {
      byte scenario = byte2 * 10 + byte3;
      Serial.print("SM_SCENARIO_");
      Serial.println(scenario, DEC);
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
    // Unknown data
    else
    {
      Serial.println("SM_ERROR");
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
      Serial.println("SM_TIMEOUT");
      Serial.flush();
    }
  }
}

void processPlMessage(char house, byte unit, byte command, byte extData, byte extCommand, byte remainingBits)
{
  printX10Message("PL_", house, unit, command, extData, extCommand, remainingBits);
}

void processRfCommand(char house, byte unit, byte command, bool isRepeat)
{
  if(!isRepeat)
  {
    printX10Message("RF_", house, unit, command, 0, 0, 0);
  }
  // Check if command is handled by scenario, if not continue
  if(!handleScenario(unit, command, isRepeat))
  {
    // Other commands map directly, just forward to PL interface
    // Make sure that two repetitions or more are used for bright and dim,
    // to avoid that commands are beeing sent seperately when repeated
    x10ex.sendCmd(house, unit, command, 2);
  }
}

void processIrCommand(char house, byte unit, byte command, bool isRepeat)
{
  if(!isRepeat)
  {
    printX10Message("IR_", house, unit, command, 0, 0, 0);
  }
  // Check if command is handled by scenario, if not continue
  if(!handleScenario(unit, command, isRepeat))
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
      Serial.println("_ON");
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
      Serial.print("_PRD_");
      Serial.println(round(extData * 100 / 31.0), DEC);
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
  }
  if(extCommand)
  {
    switch(extCommand)
    {
      case EXC_PRE_SET_DIM:
        Serial.print("_PRD_");
        Serial.println(round(extData * 100 / 62.0), DEC);
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
    Serial.print("DEBUG: ");
    Serial.print(remainingBits, DEC);
    Serial.println(" UNEXPECTED BITS RECEIVED.");
  }
}

bool handleScenario(byte unit, byte command, bool isRepeat)
{
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

void sendAllLightsOn()
{
  // Bedroom
  x10ex.sendExtDim('A', 7, 80, 1);
  // Livingroom table
  x10ex.sendExtDim('A', 2, 70, 1);
  // Hall
  x10ex.sendExtDim('A', 8, 75, 1);
  // Livingroom couch
  x10ex.sendExtDim('A', 3, 90, 1);
  // Kitchen
  x10ex.sendExtDim('A', 9, 100, 1);
  // Livingroom shelves
  x10ex.sendExtDim('A', 4, 40, 1);
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
  x10ex.sendExtDim('A', 8, 75, 1);
  x10ex.sendExtDim('A', 9, 100, 1);
}

void sendHallAndKitchenOff()
{
  x10ex.sendCmd('A', 8, CMD_OFF, 1);
  x10ex.sendCmd('A', 9, CMD_OFF, 1);
}

void sendLivingRoomOn()
{
  x10ex.sendExtDim('A', 2, 70, 1);
  x10ex.sendExtDim('A', 3, 90, 1);
  x10ex.sendExtDim('A', 4, 40, 1); 
}

void sendLivingRoomOff()
{
  x10ex.sendCmd('A', 2, CMD_OFF, 1);
  x10ex.sendCmd('A', 3, CMD_OFF, 1);
  x10ex.sendCmd('A', 4, CMD_OFF, 1);
}

void sendLivingRoomTvScenario()
{
  x10ex.sendExtDim('A', 2, 40, 1);
  x10ex.sendExtDim('A', 3, 30, 1);
  x10ex.sendExtDim('A', 4, 25, 1); 
}

void sendLivingRoomMovieScenario()
{
  x10ex.sendCmd('A', 2, CMD_OFF, 1);
  x10ex.sendCmd('A', 3, CMD_OFF, 1);
  x10ex.sendExtDim('A', 4, 25, 1); 
}
