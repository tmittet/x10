/************************************************************************/
/* X10 RF receiver library, v1.3.                                       */
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
/* Written by Thomas Mittet (code@lookout.no) October 2010.             */
/************************************************************************/

#include "X10rf.h"

const uint8_t X10rf::HOUSE_CODE[16] =
{
  B0110,B1110,B0010,B1010,B0001,B1001,B0101,B1101,
  B0111,B1111,B0011,B1011,B0000,B1000,B0100,B1100,
};

X10rf *x10rfInstance = NULL;

void x10rfReceive_wrapper()
{
  if(x10rfInstance) x10rfInstance->receive();
}

X10rf::X10rf(uint8_t receiveInt, uint8_t receivePin, rfReceiveCallback_t rfReceiveCallback)
{
  this->receiveInt = receiveInt;
  this->receivePin = receivePin;
  this->rfReceiveCallback = rfReceiveCallback;
  x10rfInstance = this;
}

//////////////////////////////
/// Public
//////////////////////////////

void X10rf::begin()
{
  if(rfReceiveCallback)
  {
    pinMode(receivePin, INPUT);
    attachInterrupt(receiveInt, x10rfReceive_wrapper, RISING);
  }
}

//////////////////////////////
/// Public (Interrupt Methods)
//////////////////////////////

void X10rf::receive()
{
  uint16_t lengthUs = micros() - riseUs;
  riseUs = micros();
  if(lengthUs >= X10_RF_RSB_MIN && lengthUs <= X10_RF_SB_MAX)
  {
    // Since receiving every command repeat will waste CPU cycles, let's assume that a repeated
    // start burst received within a certain threshold means that the same command is sent again
    if(receiveEnded && millis() > receiveEnded && millis() - receiveEnded < X10_RF_REPEAT_THRESHOLD)
    {
      receiveEnded = millis();
      rfReceiveCallback(house, unit, command, 1);
    }
    else
    {
      receiveEnded = 0;
      receiveBuffer = 0;
      if(lengthUs >= X10_RF_SB_MIN) receivedCount = 1;
    }
  }
  else if(receivedCount)
  {
    // Binary one received: add to buffer
    if(lengthUs >= X10_RF_BIT1_MIN && lengthUs <= X10_RF_BIT1_MAX)
    {
      receiveBuffer += 1LU << receivedCount - 1;
    }
    // Invalid pulse length: stop receiving
    else if(lengthUs < X10_RF_BIT0_MIN || lengthUs > X10_RF_BIT0_MAX)
    {
      receivedCount = -1;
    }
    // Check that unused bits are not set
    if(
      (receivedCount == 8 && (receiveBuffer & B11010000) != 0) ||
      (receivedCount == 24 && ((receiveBuffer >> 16) & B11100000) != 0))
    {
      receivedCount = -1;
    }
    // Receiving bitwize complement bits (9-16 and 25-32): verify and stop if invalid
    else if(
      ((receivedCount > 8 && receivedCount <= 16) || (receivedCount > 24 && receivedCount <= 32)) &&
      (receiveBuffer >> receivedCount - 9) & B1 == (receiveBuffer >> receivedCount - 1) & B1)
    {
      receivedCount = -1;
    }
    // Receive complete: parse message
    if(receivedCount == 32)
    {
      receivedCount = -1;
      handleCommand(receiveBuffer & 0xFF, (receiveBuffer >> 16) & 0xFF);
    }
    receivedCount++;
  }
}

//////////////////////////////
/// Private
//////////////////////////////

void X10rf::handleCommand(uint8_t byte1, uint8_t byte2)
{
  receiveEnded = millis();
  // Get house code
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

char X10rf::parseHouseCode(uint8_t data)
{
  for(uint8_t i = 0; i <= 0xF; i++)
  {
    if(HOUSE_CODE[i] == data) return i + 65;
  }
}