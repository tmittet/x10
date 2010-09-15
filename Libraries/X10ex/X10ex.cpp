/************************************************************************/
/* X10 Rx/Tx library for the XM10/TW7223/TW523 interface, v1.2.         */
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

#include "WProgram.h"
#include "pins_arduino.h"
#include <avr/eeprom.h>
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

void x10exIoTimer_wrapper()
{
  if(x10exInstance)
  {
    x10exInstance->ioTimer();
  }
}

ISR(TIMER1_OVF_vect)
{
  x10exIoTimer_wrapper();
}

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
  // Setup IO fields
  ioStopState = phases * 2;
  inputAtCycles = round(.5 * F_CPU * X10_SAMPLE_DELAY / 1000000);
  // Sine wave half cycle devided by number of phases
  outputStartCycles = round(.5 * F_CPU / phases / sineWaveHz / 2);
  outputStopCycles = round(.5 * F_CPU * X10_SIGNAL_LENGTH / 1000000);
  // Init. misc fields
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
  // Setup IO timer
  TCCR1A = 0;
  TIMSK1 = _BV(TOIE1);
  TCCR1B = _BV(WGM13) & ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
  ICR1 = inputAtCycles;
  sei();
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
  // Using CMD_STATUS_REQUEST to address modules (prepare for BRIGHT, DIM, etc.).
  // I've not seen any modules that reply to the request and even if they did
  // it would not cause any harm.
  return sendCmd(house, unit, CMD_STATUS_REQUEST, repetitions);
}

bool X10ex::sendCmd(char house, uint8_t command, uint8_t repetitions)
{
  return sendCmd(house, 0, command, repetitions);
}

bool X10ex::sendCmd(char house, uint8_t unit, uint8_t command, uint8_t repetitions)
{
  return sendExt(house, unit, command, 0, 0, repetitions);
}

// This does not work with any of the European modules I've tested.
// I have no idea if it works at all, but it't part of the X10 standard.
// If you're using a PLC interface and modules that support extended
// code, use the "sendExtDim" method in stead.
bool X10ex::sendDim(char house, uint8_t unit, uint8_t percent, uint8_t repetitions)
{
  if(percent == 0)
  {
    return sendExt(house, unit, CMD_OFF, 0, 0, repetitions);
  }
  else
  {
    uint8_t brightness = percent >= 100 ? 31 : round(percent / 100.0 * 31.0);
    return sendExt(
      house, unit, brightness >> 4 ? CMD_PRE_SET_DIM_1 : CMD_PRE_SET_DIM_0,
      // Reverse nibble before sending
      (((brightness * 0x0802LU & 0x22110LU) | (brightness * 0x8020LU & 0x88440LU)) * 0x10101LU >> 20) & B1111, 0,
      repetitions);
  }
}

// This method works with all X10 modules that support extended code. The only
// exception is the time attribute, that may result in unexpected behaviour
// when set to anything but the default value 0, on some X10 modules.
bool X10ex::sendExtDim(char house, uint8_t unit, uint8_t percent, uint8_t time, uint8_t repetitions)
{
  if(percent == 0)
  {
    return sendExt(house, unit, CMD_OFF, 0, 0, repetitions);
  }
  else
  {
    uint8_t data = percent >= 100 ? 62 : round(percent / 100.0 * 62.0);
    data |= time >= B11 ? B11000000 : time << 6;
    return sendExt(
      house, unit, CMD_EXTENDED_CODE,
      data, EXC_PRE_SET_DIM,
      repetitions);
  }
}

// Returns true when command was buffered successfully
bool X10ex::sendExt(char house, uint8_t unit, uint8_t command, uint8_t extData, uint8_t extCommand, uint8_t repetitions)
{
  house -= house > 96 ? 97 : 65;
  unit--;
  // Validate input
  if(house > 0xF || (unit > 0xF && unit != 0xFF))
  {
    return 0;
  }
  // Add house nibble (bit 32-29)
  uint32_t message = (uint32_t)HOUSE_CODE[house] << 28;
  // No unit code X10 message
  if(unit == 0xFF)
  {
    message |=
      (uint32_t)command << 24 | // Add command nibble (bit 28-25)
      (uint32_t)1 << 23 |       // Set message type (bit 24) to 1 (command)
      X10_MSG_CMD;              // Set data type (bit 3-1)
  }
  // Standard X10 message
  else if(command != CMD_EXTENDED_CODE && command != CMD_EXTENDED_DATA)
  {
    // If type is preset dim send data, if not repeat house code
    uint8_t houseData = (command & B1110) == CMD_PRE_SET_DIM_0 ? extData : HOUSE_CODE[house];
    message |=
      (uint32_t)UNIT_CODE[unit] << 24 | // Add unit nibble (bit 28-25)
      (uint16_t)houseData << 8 |        // Add house/data nibble (bit 12-9)
      (uint16_t)command << 4 |          // Add command nibble (bit 8-5)
      (uint8_t)1 << 3 |                 // Set message type (bit 4) to 1 (command)
      X10_MSG_STD;                      // Set data type (bit 3-1)
  }
  // Extended X10 message
  else
  {
    message |=
      (uint32_t)command << 24 |         // Add command nibble (bit 28-25)
      (uint32_t)1 << 23 |               // Set message type (bit 24) to 1 (command)
      (uint32_t)UNIT_CODE[unit] << 19 | // Add unit nibble (bit 23-20)
      (uint32_t)extData << 11 |         // Set extended data byte (bit 19-12)
      (uint16_t)extCommand << 3 |       // Set extended command byte (bit 11-4)
      X10_MSG_EXT;                      // Set data type (bit 3-1)
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
  }
  return 0;
}

X10state X10ex::getModuleState(char house, uint8_t unit)
{
  house -= house > 96 ? 97 : 65;
  unit--;
  bool isKnown = 0;
  bool isOn = 0;
  uint8_t data;
  uint8_t stateIx = house << 4 | unit;
  // Validate input
#if X10_PERSIST_STATE
  uint8_t state = eeprom_read_byte((unsigned char *)stateIx) + 1;
#else
  uint8_t state = moduleState[stateIx];
#endif
  if(house <= 0xF && unit <= 0xF && state >> 6 & B1)
  {
    isKnown = 1;
    isOn = state >> 7;
    data = state & B111111;
  }
  return (X10state) { isKnown, isOn, data };
}

void X10ex::wipeModuleState()
{
  for(int i = 0; i < 256; i++)
  {
#if X10_PERSIST_STATE
    eeprom_write_byte((unsigned char *)i, 255);
#else
    moduleState[i] = 0;
#endif
  }
}

void X10ex::zeroCross()
{
  zcOutput = 0;
  zcInput = 0;
  // Start IO timer
  TCCR1B |= _BV(CS10);
  TCNT1 = 1;
  ioState = 1;
  // Get bit to output from buffer
  if(sendBf[sendBfStart].repetitions && (zeroCount > X10_PRE_CMD_CYCLES || sentCount))
  {
    zcOutput = getBitToSend();
    if(zcOutput)
    {
      digitalWrite(transmitPin, HIGH);
    }
  }
}

void X10ex::ioTimer()
{
  if(ioState)
  {
    // Input
    if(ioState == 1)
    {
      ICR1 = outputStopCycles - inputAtCycles;
      zcInput = receiveTransmits || !sendBf[sendBfStart].repetitions ? !(*portInputRegister(receivePort) & receiveBitMask) : 0;
    }
    // Stop timer, output low and check receive
    else if((ioState == 2 && !zcOutput) || ioState == ioStopState)
    {
      ICR1 = inputAtCycles;
      digitalWrite(transmitPin, LOW);
      TCCR1B &= ~_BV(CS10);
      ioState = -1;
      // If start sequence is found, receive message
      if(receivedCount)
      {
        receiveMessage();
      }
      // Search for start sequence and keep track of silence
      else
      {
        // If we receive a one; increment bit count
        if(zcInput)
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
    // Output set High
    else if(ioState % 2)
    {
      ICR1 = outputStopCycles;
      digitalWrite(transmitPin, HIGH);
    }
    // Output set Low
    else
    {
      ICR1 = outputStartCycles - outputStopCycles;
      digitalWrite(transmitPin, LOW);
    }
    ioState++;
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

void X10ex::receiveMessage()
{
  receivedCount++;
  // Get data bit (odd)
  if(receivedCount % 2)
  {
    receivedDataBit = zcInput;
  }
  // If data bit complement (even) is correct
  else if(receivedDataBit != zcInput) 
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
      uint8_t houseIx = findCodeIndex(HOUSE_CODE, rxHouse);
      uint8_t unitIx = findCodeIndex(UNIT_CODE, rxExtUnit != DATA_UNKNOWN ? rxExtUnit : rxUnit);
      if(unitIx != 0xFF)
      {
        updateModuleState(houseIx << 4 | unitIx);
      }
      // Trigger receive callback
      plcReceiveCallback(houseIx + 65, unitIx + 1, rxCommand, rxData, rxExtCommand, receivedBits);
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
      (
        // Get the four least significant bits in reverse (bit 8-5 => 1-4)
        ((((receiveBuffer * 0x0802LU & 0x22110LU) | (receiveBuffer * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & B1111) +
        // Add most significant bit (bit 1 => 5)
        ((receiveBuffer & B0001) << 4)
      ) * 2;
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

void X10ex::updateModuleState(uint8_t index)
{
#if X10_PERSIST_STATE
  uint8_t state = eeprom_read_byte((unsigned char *)index) + 1;
#else
  uint8_t state = moduleState[index];
#endif
  uint8_t stateData = state & B111111;
  // Dim or bright; update brightness
  if(rxCommand == CMD_DIM)
  {
    if(state >> 6 == B1)
    {
      stateData = 62;
    }
    else
    {
      stateData = stateData > 9 ? stateData - 9 : 1;
    }
  }
  if(rxCommand == CMD_BRIGHT)
  {
    if(state >> 6 == B1)
    {
      stateData = 11;
    }
    else
    {
      stateData = stateData <= 53 ? stateData + 9 : 62;
    }
  }
  // Off; update known and on bits
  if(rxCommand == CMD_OFF || rxCommand == CMD_STATUS_OFF)
  {
    state |= B1000000;
    state &= B1111111;
  }
  // On; update known and on bits and get brightness from buffer
  if(rxCommand == CMD_DIM || rxCommand == CMD_BRIGHT || rxCommand == CMD_ON || rxCommand == CMD_STATUS_ON)
  {
    state = stateData | B11000000;
    rxData = stateData;
  }
  // X10 standard or extended code message pre set dim commands
  if((rxCommand & B1110) == CMD_PRE_SET_DIM_0 || (rxCommand == CMD_EXTENDED_CODE && rxExtCommand == EXC_PRE_SET_DIM))
  {
    if(rxData > 0)
    {
      state = rxData | B1000000 | ((rxData > 0) << 7);
    }
    // Dim level 0; set known and off bits only
    else
    {
      state |= B1000000;
      state &= B1111111;
    }
  }
#if X10_PERSIST_STATE
  eeprom_write_byte((unsigned char *)index, state - 1);
#else
  moduleState[index] = state;
#endif
}

void X10ex::clearReceiveBuffer()
{
  receivedBits = 0;
  receiveBuffer = 0;
}

int8_t X10ex::findCodeIndex(const uint8_t codeList[16], uint8_t code)
{
  for(uint8_t i = 0; i <= 0xF; i++)
  {
    if(codeList[i] == code) return i;
  }
  return -1;
}