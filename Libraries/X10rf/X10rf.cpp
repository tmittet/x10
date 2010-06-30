/************************************************************************/
/* X10 RF receiver library, v1.0.                                       */
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

#include "WProgram.h"
#include "pins_arduino.h"
#include "X10rf.h"

const uint8_t X10rf::HOUSE_CODE[16] =
{
  B0110,B1110,B0010,B1010,B0001,B1001,B0101,B1101,
  B0111,B1111,B0011,B1011,B0000,B1000,B0100,B1100,
};

X10rf *x10rfInstance = NULL;

void x10rfReceive_wrapper()
{
  if(x10rfInstance)
  {
    x10rfInstance->receive();
  }
}

X10rf::X10rf(uint8_t receiveInt, uint8_t receivePin, rfReceiveCallback_t rfReceiveCallback)
{
  this->receiveInt = receiveInt;
  this->receivePin = receivePin;
  this->receivePort = digitalPinToPort(receivePin);
  this->receiveBitMask = digitalPinToBitMask(receivePin);
  this->rfReceiveCallback = rfReceiveCallback;
  x10rfInstance = this;
}

void X10rf::begin()
{
  if(rfReceiveCallback)
  {
    pinMode(receivePin, INPUT);
    attachInterrupt(receiveInt, x10rfReceive_wrapper, CHANGE);
  }
}

void X10rf::receive()
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
      if(lowUs > X10_RF_SB_MIN && lowUs < X10_RF_SB_MAX)
      {
        // Since receiving every command repeat will waste CPU cycles, let's assume that a
        // consecutive command received within a certain threshold is the same as the last one
        if(millis() > receiveStarted && millis() - receiveStarted < X10_RF_REPEAT_THRESHOLD)
        {
          receiveStarted = millis();
          rfReceiveCallback(house, unit, command, 1);
        }
        else
        {
          receiveStarted = millis();
          uint8_t byte1 = getByte();
          // Check that unused bits are not set and verify bitwize complement
          if(receiveStarted && (byte1 & B11010000) == 0 && verifyByte(byte1))
          {
            uint8_t byte2 = getByte();
            // Check that unused bits are not set and verify bitwize complement
            if(receiveStarted && (byte2 & B11100000) == 0 && verifyByte(byte2))
            {
              house = parseHouseCode(byte1 & B1111);
              // Bright or Dim
              if(byte2 & B1)
              {
                unit = 0;
                // Bit magic to create X10 CMD_DIM (B0100) or CMD_BRIGHT (B0101) nibble
                command = byte2 >> 3 & B1 ^ B101;
              }
              // On or Off
              else
              {
                // Swap some bits to create unit integer from binary data
                unit = (byte2 >> 3 | byte2 << 1 & B100 | byte1 >> 2 & B1000) + 1;
                // Bit magic to create X10 CMD_ON (B0010) or CMD_OFF (B0011) nibble
                command = byte2 >> 2 & B1 | B10;
              }
              rfReceiveCallback(house, unit, command, 0);
            }
          }
        }
      }
      lowUs = 0;
    }
  }
}

bool X10rf::getBit()
{
  uint16_t pulse = pulseIn(receivePin, LOW, X10_RF_BIT1_MAX);
  if(pulse > X10_RF_BIT1_MIN && pulse < X10_RF_BIT1_MAX)
  {
    return 1;
  }
  else if(pulse < X10_RF_BIT0_MIN || pulse > X10_RF_BIT0_MAX)
  {
    receiveStarted = 0;
  }
  return 0;
}

uint8_t X10rf::getByte()
{
  uint8_t data = 0;
  for(uint8_t i = 0; i < 8 && receiveStarted; i++)
  {
    bool bit = getBit();
    data += bit << i;
  }
  return data;
}

bool X10rf::verifyByte(uint8_t data)
{
  for(uint8_t i = 0; i < 8 && receiveStarted; i++)
  {
    if(data >> i & B1 == getBit())
    {
      return 0;
    }
  }
  return 1 && receiveStarted;
}

char X10rf::parseHouseCode(uint8_t data)
{
  for(uint8_t i = 0; i < 16; i++)
  {
    if(HOUSE_CODE[i] == data) return i + 65;
  }
}