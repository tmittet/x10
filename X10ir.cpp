/************************************************************************/
/* X10 IR receiver library, v1.2.                                       */
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
/* Written by Thomas Mittet code@lookout.no June 2010.                  */
/************************************************************************/

#include "X10ir.h"

const uint8_t X10ir::HOUSE_CODE[16] =
{
  B0110,B1110,B0010,B1010,B0001,B1001,B0101,B1101,
  B0111,B1111,B0011,B1011,B0000,B1000,B0100,B1100,
};
const uint8_t X10ir::UNIT_CODE[16] =
{
  B0110,B1110,B0010,B1010,B0001,B1001,B0101,B1101,
  B0111,B1111,B0011,B1011,B0000,B1000,B0100,B1100,
};

X10ir *x10irInstance = NULL;

void x10irReceive_wrapper()
{
  if(x10irInstance) x10irInstance->receive();
}

X10ir::X10ir(uint8_t receiveInt, uint8_t receivePin, char defaultHouse, irReceiveCallback_t irReceiveCallback)
{
  this->receiveInt = receiveInt;
  this->receivePin = receivePin;
  this->receivePort = digitalPinToPort(receivePin);
  this->receiveBitMask = digitalPinToBitMask(receivePin);
  this->irReceiveCallback = irReceiveCallback;
#if X10_IR_UNIT_RESET_TIME
  this->defaultHouse = defaultHouse;
#endif
  house = defaultHouse;
  command = DATA_UNKNOWN;
  x10irInstance = this;
}

//////////////////////////////
/// Public
//////////////////////////////

void X10ir::begin()
{
  if(irReceiveCallback)
  {
    pinMode(receivePin, INPUT);
    attachInterrupt(receiveInt, x10irReceive_wrapper, CHANGE);
  }
}

//////////////////////////////
/// Public (Interrupt Methods)
//////////////////////////////

void X10ir::receive()
{
  // Receive pin is Low
  if(!(*portInputRegister(receivePort) & receiveBitMask))
  {
    lowUs = micros();
  }
  // Receive pin is High
  else
  {
    if(lowUs)
    {
      lowUs = micros() - lowUs;
      if(lowUs >= X10_IR_SB_MIN && lowUs <= X10_IR_SB_MAX)
      {
#if X10_IR_UNIT_RESET_TIME
        // If more than specified unit reset time in milliseconds has passed since the last successful
        // IR command was received, set the house and unit code back to their default values
        if(receiveEnded && (millis() - receiveEnded > X10_IR_UNIT_RESET_TIME || receiveEnded > millis()))
        {
          house = defaultHouse;
          unit = 0;
        }
#endif
        // Since receiving every command repeat will waste CPU cycles, let's assume that a
        // consecutive command received within a certain threshold is the same as the last one
        if(receiveEnded && millis() > receiveEnded && millis() - receiveEnded < X10_IR_REPEAT_THRESHOLD)
        {
          receiveEnded = millis();
          triggerCallback(1);
        }
        else
        {
          receiveEnded = 0;
          receiveBuffer = 0;
          receivedCount = 1;
        }
      }
      else if(receivedCount)
      {
        // Message to long: stop receiving
        if(receivedCount > 11)
        {
          receivedCount = -1;
        }
        // Binary one received: add to buffer
        else if(lowUs >= X10_IR_BIT1_MIN && lowUs <= X10_IR_BIT1_MAX)
        {
          receiveBuffer += B1 << (16 - receivedCount);
        }
        // Invalid pulse length or end pulse: stop receiving
        else if(lowUs < X10_IR_BIT0_MIN || lowUs > X10_IR_BIT0_MAX)
        {
          // If end pulse was detected: validate and parse
          if(lowUs > X10_IR_EB_MIN)
          {
            // Unit and Command (5 bits + 5 complementary bits)
            if(receivedCount == 11 && validateData(receiveBuffer, 5))
            {
              handleCommand(receiveBuffer >> 8 & B11111000);
            }
            // House (4 bits + 4 complementary bits)
            else if(receivedCount == 9 && validateData(receiveBuffer, 4))
            {
              // Mark byte as house code by setting last bit
              handleCommand(receiveBuffer >> 8 & B11110000 | B1);
            }
          }
          receivedCount = -1;
        }
        receivedCount++;
      }
    }
  }
}

//////////////////////////////
/// Private
//////////////////////////////

void X10ir::handleCommand(uint8_t data)
{
  receiveEnded = millis();
  switch(data & B1111)
  {
    case X10_IR_TYPE_HOUSE:
      house = findCodeIndex(HOUSE_CODE, data >> 4) + 65;
      unit = 0;
      command = DATA_UNKNOWN;
      break;
    case X10_IR_TYPE_UNIT:
      unit = findCodeIndex(UNIT_CODE, data >> 4) + 1;
      command = CMD_ADDRESS;
      triggerCallback(0);
      break;
    case X10_IR_TYPE_COMMAND:
      command = data >> 4;
      triggerCallback(0);
      break;
  }
}

void X10ir::triggerCallback(bool isRepeat)
{
  if(
    command == CMD_ALL_UNITS_OFF ||
    command == CMD_ALL_LIGHTS_ON ||
    command == CMD_DIM ||
    command == CMD_BRIGHT)
  {
    irReceiveCallback(house, 0, command, isRepeat);
  }
  else if(unit > 0)
  {
    irReceiveCallback(house, unit, command, isRepeat);
  }
}

bool X10ir::validateData(uint16_t data, uint8_t bits)
{
  for(uint8_t i = 0; i < bits; i++)
  {
    if(data >> 15 - i & B1 == data >> 15 - bits - i & B1) return 0;
  }
  return 1;
}

int8_t X10ir::findCodeIndex(const uint8_t codeList[16], uint8_t code)
{
  for(uint8_t i = 0; i <= 0xF; i++)
  {
    if(codeList[i] == code) return i;
  }
  return -1;
}