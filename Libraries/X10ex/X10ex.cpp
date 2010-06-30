/************************************************************/
/* X10 Rx/Tx library for the XM10/TW7223/TW523 interface.   */
/* This library is free software; you can redistribute it   */
/* and/or modify it under the terms of the GNU License v3.  */
/*                                                          */
/* Written by Thomas Mittet thomas@mittet.nu June 2010.     */
/************************************************************/

#include "WProgram.h"
#include "pins_arduino.h"
#include "X10ex.h"

const uint8_t X10ex::HOUSE_CODE[16] =
{
  B0110,B1110,B0010,B1010,B0001,B1001,B0101,B1101,
  B0111,B1111,B0011,B1011,B0000,B1000,B0100,B1100,
};
const uint8_t X10ex::UNIT_CODE[16] =
{
  B0110,B1110,B0010,B1010,B0001,B1001,B0101,B1101,
  B0111,B1111,B0011,B1011,B0000,B1000,B0100,B1100,
};

X10ex *x10exInstance = NULL;

void x10exZeroCross_wrapper()
{
  if(x10exInstance)
  {
    x10exInstance->zeroCross();
  }
}

// Hack to get extra interrupt on non ATmega1280 pin 4
#if not defined(__AVR_ATmega1280__)
SIGNAL(PCINT2_vect)
{
  if(x10exInstance)
  {
    x10exInstance->zeroCross();
  }
}
#endif

X10ex::X10ex(
  uint8_t zeroCrossInt, uint8_t zeroCrossPin, uint8_t transmitPin,
  uint8_t receivePin, bool receiveTransmits, plcReceiveCallback_t plcReceiveCallback,
  uint8_t phases, uint8_t sineWaveHz)
{
  this->zeroCrossInt = zeroCrossInt;
  this->zeroCrossPin = zeroCrossPin;
  this->transmitPin = transmitPin;
  this->receivePin = receivePin;
  this->receivePort = digitalPinToPort(receivePin);
  this->receiveBitMask = digitalPinToBitMask(receivePin);
  this->receiveTransmits = receiveTransmits;
  this->plcReceiveCallback = plcReceiveCallback;
  this->phases = phases;
  this->sineWaveHz = sineWaveHz;
  sendBfEnd = X10_BUFFER_SIZE - 1;
  rxHouse = DATA_UNKNOWN;
  rxUnit = DATA_UNKNOWN;
  rxExtUnit = DATA_UNKNOWN;
  rxCommand = DATA_UNKNOWN;
  x10exInstance = this;
}

void X10ex::begin()
{
  digitalWrite(zeroCrossPin, HIGH);
  pinMode(zeroCrossPin, INPUT);
  if(transmitPin)
  {
    pinMode(transmitPin, OUTPUT);
  }
  digitalWrite(receivePin, HIGH);
  pinMode(receivePin, INPUT);
  attachInterrupt(zeroCrossInt, x10exZeroCross_wrapper, CHANGE);
  // Hack to get extra interrupt on non ATmega1280 pin 4
#if not defined(__AVR_ATmega1280__)
  if(zeroCrossInt == 2 && zeroCrossPin == 4)
  {
    PCMSK2 |= digitalPinToBitMask(4);
    PCICR |= 0x01 << digitalPinToPort(4) - 2;
  }
#endif
}

bool X10ex::sendAddress(char house, uint8_t unit, uint8_t repetitions)
{
  return sendCmd(house, unit, CMD_STATUS_ON, repetitions);
}

bool X10ex::sendCmd(char house, uint8_t command, uint8_t repetitions)
{
  return sendCmd(house, 0, command, repetitions);
}

bool X10ex::sendCmd(char house, uint8_t unit, uint8_t command, uint8_t repetitions)
{
  return sendExt(house, unit, command, 0, 0, repetitions);
}

// This does not work with any of the European units I've tested.
// I have no idea if it works at all, but it't part of the X10 standard.
// If you're using a PLC interface and units that support extended code,
// use the "sendExtDim" method in stead.
bool X10ex::sendDim(char house, uint8_t unit, uint8_t percent, uint8_t repetitions)
{
  if(percent == 0)
  {
    return sendExt(house, unit, CMD_OFF, 0, 0, repetitions);
  }
  else
  {
    uint8_t brightness = percent > 100 ? 31 : round(percent / 100.0 * 31.0);
    return sendExt(
      house, unit, brightness >> 4 ? CMD_PRE_SET_DIM_1 : CMD_PRE_SET_DIM_0,
      // Reverse nibble before sending
      (((brightness * 0x0802LU & 0x22110LU) | (brightness * 0x8020LU & 0x88440LU)) * 0x10101LU >> 20) & B1111, 0,
      repetitions);
  }
}

bool X10ex::sendExtDim(char house, uint8_t unit, uint8_t percent, uint8_t repetitions)
{
  if(percent == 0)
  {
    return sendExt(house, unit, CMD_OFF, 0, 0, repetitions);
  }
  else
  {
    return sendExt(
      house, unit, CMD_EXTENDED_CODE,
      percent > 100 ? 62 : round(percent / 100.0 * 62.0), EXC_PRE_SET_DIM,
      repetitions);
  }
}

// Returns true when command was buffered successfully
bool X10ex::sendExt(char house, uint8_t unit, uint8_t command, uint8_t extData, uint8_t extCommand, uint8_t repetitions)
{
  house -= house > 96 ? 97 : 65;
  unit--;
  // Validate input
  if(house > 15 || (unit > 15 && unit != 255))
  {
    return 0;
  }
  // Add house nibble (bit 1-4)
  uint32_t message = (uint32_t)HOUSE_CODE[house] << 28;
  // No unit code X10 message
  if(unit == 255)
  {
    message |=
      (uint32_t)command << 24 | // Add command nibble (bit 5-8)
      (uint32_t)1 << 23 |       // Set message type bit (9) to 1 (command)
      X10_MSG_CMD;              // Set data type bits (30-32)
  }
  // Standard X10 message
  else if(command != CMD_EXTENDED_CODE && command != CMD_EXTENDED_DATA)
  {
    // If type is preset dim send data, if not repeat house code
    uint8_t houseData = (command & B1110) == CMD_PRE_SET_DIM_0 ? extData : HOUSE_CODE[house];
    message |=
      (uint32_t)UNIT_CODE[unit] << 24 | // Add unit nibble (bit 5-8)
      (uint16_t)houseData << 8 |        // Add house/data nibble (bit 21-24)
      (uint16_t)command << 4 |          // Add command nibble (25-28)
      (uint8_t)1 << 3 |                 // Set message type bit (29) to 1 (command)
      X10_MSG_STD;                      // Set data type bits (30-32)
  }
  // Extended X10 message
  else
  {
    message |=
      (uint32_t)command << 24 |         // Add command nibble (5-8)
      (uint32_t)1 << 23 |               // Set message type bit (9) to 1 (command)
      (uint32_t)UNIT_CODE[unit] << 19 | // Add unit nibble (10-13)
      (uint32_t)extData << 11 |         // Set extended data byte (14-21)
      (uint16_t)extCommand << 3 |       // Set extended command byte (22-29)
      X10_MSG_EXT;                      // Set data type bits (30-32)
  }
  // Current command is buffered again
  if(sendBf[sendBfStart].repetitions > 0 && sendBf[sendBfStart].message == message)
  {
    // Just reset repetitions
    sendBf[sendBfStart].repetitions = repetitions;
    sendBfLastMs = millis();
    return 1;
  }
  // If slots are available in buffer
  else if((sendBfEnd + 2) % X10_BUFFER_SIZE != sendBfStart)
  {
    // Make sure identical message is not sent within rebuffer delay
    if(sendBf[sendBfEnd].message != message || millis() > sendBfLastMs + X10_REBUFFER_DELAY || sendBfLastMs - 1 > millis())
    {
      sendBfEnd = (sendBfEnd + 1) % X10_BUFFER_SIZE;
      // When nothing is currently buffered
      if(sendBfStart == sendBfEnd)
      {
        // Give controller some extra PL cycles before message is sent
        // This is necessary when command is buffered from an interrupt
        // to let the controller catch it's breath :)
        zeroCount = 0;
      }
      // Buffer message and repetitions
      sendBf[sendBfEnd].message = message;
      sendBf[sendBfEnd].repetitions = repetitions;
      sendBfLastMs = millis();
      return 1;
    }
    else
    {
      Serial.println("S");
    }
  }
  return 0;
}

void X10ex::zeroCross()
{
  bool output = 0;
  bool input = 0;
  // Send message
  if(sendBf[sendBfStart].repetitions && (zeroCount > X10_PRE_CMD_CYCLES || sentCount))
  {
    output = getBitToSend();
  }
  // Start output
  if(output)
  {
    digitalWrite(transmitPin, HIGH);
  }
  // Get input
  delayMicroseconds(X10_SAMPLE_DELAY);
  input = receiveTransmits || !sendBf[sendBfStart].repetitions ? !(*portInputRegister(receivePort) & receiveBitMask) : 0;
  // End output
  if(output)
  {
    delayMicroseconds(X10_SIGNAL_LENGTH - X10_SAMPLE_DELAY);
    digitalWrite(transmitPin, LOW);
    // Repeat output for each additional phase
    for(int i = 1; sineWaveHz && i < phases; i++)
    {
      delayMicroseconds(round(1000000.0 / (phases * sineWaveHz * 2)) - X10_SIGNAL_LENGTH);
      digitalWrite(transmitPin, HIGH);
      delayMicroseconds(X10_SIGNAL_LENGTH);
      digitalWrite(transmitPin, LOW);
    }
  }
  // If start sequence is found, receive message
  if(receivedCount)
  {
    receiveMessage(input);
  }
  // Search for start sequence and keep track of silence
  else
  {
    // If we receive a one; increment bit count
    if(input)
    {
      zeroCount = 0;
      receivedBits++;
    }
    else
    {
      // 3 consecutive ones is the startcode
      if(receivedBits == 3 && plcReceiveCallback)
      {
        // We have reached zero crossing 4 after startcode; set it to start receiving message
        receivedCount = 4;
      }
      receivedBits = 0;
      zeroCount += zeroCount == 255 ? 0 : 1;
    }
  }
}

bool X10ex::getBitToSend()
{
  sentCount++;
  bool output;
  // Send start bits
  if(sentCount - sendOffset < 5)
  {
    output = sentCount - sendOffset < 4;
  }
  // Send X10 message
  else
  {
    // Get bit position in buffer
    uint8_t bitPosition = ceil((sentCount - 4) / 2.0);
    // Get data type
    uint8_t type = sendBf[sendBfStart].message & B111;
    // Get bit to send from buffer and xor it with remainder
    // to make complement bit for every even zero cross count
    output = !(sendBf[sendBfStart].message >> 32 - bitPosition & B1) ^ sentCount % 2;
    // If type is standard X10 message
    if(type == X10_MSG_STD)
    {
      // Add cycles of silence after part one, make sure there are
      // 5 zero crosses of silence before part two is transmitted
      if(sentCount > 22 && sentCount <= 40)
      {
        output = 0;
        if(sentCount == 40 && zeroCount <= 2)
        {
          sentCount--;
        }
      }
      // Part one sent; restart by sending new start sequence then start at bit 21 in buffer
      if(sentCount == 40)
      {
        sendOffset = 40;
      }
    }
    // All messages end after 31 bits (the 62nd zero crossing)
    // If type is standard X10 message with no unit code, end after part one (11 bits)
    if(sentCount == 62 || (type == X10_MSG_CMD && sentCount == 22))
    {
      // If message has no unit code and command is BRIGHT or DIM, repeat without any silence
      zeroCount = type == X10_MSG_CMD && (sendBf[sendBfStart].message >> 24 & B1110) == CMD_DIM ? 7 : 0;
      sentCount = 0;
      sendOffset = 0;
      if(sendBf[sendBfStart].repetitions > 1)
      {
        sendBf[sendBfStart].repetitions--;
      }
      else
      {
        sendBf[sendBfStart].repetitions = 0;
        sendBfStart = (sendBfStart + 1) % X10_BUFFER_SIZE;
      }
    }
  }
  return output;
}

void X10ex::receiveMessage(bool input)
{
  receivedCount++;
  // Get data bit (odd)
  if(receivedCount % 2)
  {
    receivedDataBit = input;
  }
  // If data bit complement (even) is correct
  else if(receivedDataBit != input) 
  {
    receivedBits++;
    // Buffer one byte
    if(receivedBits < 9)
    {
      receiveBuffer += receivedDataBit << 8 - receivedBits;
    }
    // At zero crossing 22 standard message is complete; parse it
    if(receivedCount == 22)
    {
      receiveStandardMessage();
    }
    // Extended command received; parse extended message
    else if(rxCommand == CMD_EXTENDED_CODE || rxCommand == CMD_EXTENDED_DATA)
    {
      receiveExtendedMessage();
    }
  }
  // If data bit complement is no longer correct it means we have stopped receiving data
  else
  {
    if(rxCommand != DATA_UNKNOWN)
    {
      plcReceiveCallback(
        findCodeIndex(HOUSE_CODE, rxHouse) + 65,
        findCodeIndex(UNIT_CODE, rxExtUnit != DATA_UNKNOWN ? rxExtUnit : rxUnit) + 1,
        rxCommand, rxData, rxExtCommand, receivedBits);
    }
    rxCommand = DATA_UNKNOWN;
    rxData = 0;
    rxExtCommand = 0;
    clearReceiveBuffer();
    receivedCount = 0;
  }
}

void X10ex::receiveStandardMessage()
{
  // Clear extended message unit code
  rxExtUnit = DATA_UNKNOWN;
  // Address (House + Unit)
  if(!receivedDataBit)
  {
    rxHouse = (receiveBuffer & B11110000) >> 4;
    rxUnit = receiveBuffer & B1111;
  }
  // Pre-Set Dim (LSBs + Command + MSB)
  else if((receiveBuffer & B1110) == CMD_PRE_SET_DIM_0)
  {
    rxCommand = CMD_PRE_SET_DIM_0;
    rxData =
      ((((receiveBuffer * 0x0802LU & 0x22110LU) | (receiveBuffer * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & B1111) +
      ((receiveBuffer & B0001) << 4);
  }
  // Command (House + Command)
  else
  {
    rxHouse = (receiveBuffer & B11110000) >> 4;
    rxCommand = receiveBuffer & B1111;
  }
  clearReceiveBuffer();
}

void X10ex::receiveExtendedMessage()
{
  // Unit
  if(receivedCount == 30)
  {
    rxExtUnit = (receiveBuffer >> 4) & B1111;
    clearReceiveBuffer();
  }
  // Data
  else if(receivedCount == 46)
  {
    rxData = receiveBuffer;
    clearReceiveBuffer();
  }
  // Command
  else if(receivedCount == 62)
  {
    rxExtCommand = receiveBuffer;
    clearReceiveBuffer();
  }
}

void X10ex::clearReceiveBuffer()
{
  receivedBits = 0;
  receiveBuffer = 0;
}

int8_t X10ex::findCodeIndex(const uint8_t codeList[16], uint8_t code)
{
  for(uint8_t i = 0; i < 16; i++)
  {
    if(codeList[i] == code) return i;
  }
  return -1;
}