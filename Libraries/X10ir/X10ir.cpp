/************************************************************/
/* X10 IR receiver library.                                 */
/* This library is free software; you can redistribute it   */
/* and/or modify it under the terms of the GNU License.     */
/*                                                          */
/* Written by Thomas Mittet thomas@mittet.nu June 2010.     */
/************************************************************/

#include "WProgram.h"
#include "pins_arduino.h"
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
  if(x10irInstance)
  {
    x10irInstance->receive();
  }
}

X10ir::X10ir(uint8_t receiveInt, uint8_t receivePin, char defaultHouse, irReceiveCallback_t irReceiveCallback)
{
  this->receiveInt = receiveInt;
  this->receivePin = receivePin;
  this->receivePort = digitalPinToPort(receivePin);
  this->receiveBitMask = digitalPinToBitMask(receivePin);
  this->irReceiveCallback = irReceiveCallback;
  house = defaultHouse;
  command = DATA_UNKNOWN;
  x10irInstance = this;
}

void X10ir::begin()
{
  if(irReceiveCallback)
  {
    pinMode(receivePin, INPUT);
    attachInterrupt(receiveInt, x10irReceive_wrapper, CHANGE);
  }
}

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
      if(lowUs > X10_IR_SB_MIN && lowUs < X10_IR_SB_MAX)
      {
        // Since receiving every command repeat will waste CPU cycles, let's assume that a
        // consecutive command received within a certain threshold is the same as the last one
        if(millis() > receiveStarted && millis() - receiveStarted < X10_IR_REPEAT_THRESHOLD)
        {
          receiveStarted = millis();
          triggerCallback(1);
        }
        else
        {
          receiveStarted = millis();
          uint8_t data = getNibble();
          switch(data & B1111)
          {
            case X10_IR_NIB_HOUSE:
              house = findCodeIndex(HOUSE_CODE, data >> 4) + 65;
              unit = 0;
              command = DATA_UNKNOWN;
              break;
            case X10_IR_NIB_UNIT:
              unit = findCodeIndex(UNIT_CODE, data >> 4) + 1;
              command = CMD_ADDRESS;
              triggerCallback(0);
              break;
            case X10_IR_NIB_COMMAND:
              command = data >> 4;
              triggerCallback(0);
              break;
            default:
              receiveStarted = 0;
          }
        }
      }
      lowUs = 0;
    }
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

uint8_t X10ir::getNibble()
{
  uint8_t pos = 0;
  uint16_t pulse = 0;
  uint16_t data = 0;
  while(pulse < X10_IR_EB_MIN && pos < 11)
  {
    pulse = pulseIn(receivePin, LOW, X10_IR_EB_MAX);
    if(pulse >= X10_IR_BIT1_MIN && pulse <= X10_IR_BIT1_MAX)
    {
      data += B1 << (15 - pos);
    }
    else if(
      (pulse < X10_IR_BIT0_MIN || pulse > X10_IR_BIT0_MAX) &&
      (pulse < X10_IR_EB_MIN || pulse > X10_IR_EB_MAX))
    {
      return X10_IR_NIB_ERROR;
    }
    pos++;
  }
  // Unit, Command or House
  if(pos == 11 && validateNibble(data, 5))
  {
    return data >> 8 & B11111000;
  }
  else if(pos == 9 && validateNibble(data, 4))
  {
    return data >> 8 & B11110000 + 1;
  }
  return X10_IR_NIB_ERROR;
}

bool X10ir::validateNibble(uint16_t data, uint8_t bits)
{
  for(uint8_t i = 0; i < bits; i++)
  {
    if(data >> 15 - i & B1 == data >> 15 - bits - i & B1)
    {
      return 0;
    }
  }
  return 1;
}

int8_t X10ir::findCodeIndex(const uint8_t codeList[16], uint8_t code)
{
  for(uint8_t i = 0; i < 16; i++)
  {
    if(codeList[i] == code) return i;
  }
  return -1;
}