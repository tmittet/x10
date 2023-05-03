/************************************************************************/
/* X10 Rx/Tx library for the XM10/TW7223/TW523 interface, v1.6.         */
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
  if(x10exInstance) x10exInstance->zeroCross();
}

// Hack to get extra interrupt on non ATmega8, 168 and 328 pin 4 to 7
#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
SIGNAL(PCINT2_vect)
{
  if(x10exInstance) x10exInstance->zeroCross();
}
#endif

void x10exIoTimer_wrapper()
{
  if(x10exInstance) x10exInstance->ioTimer();
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
  transmitPort = digitalPinToPort(transmitPin);
  transmitBitMask = digitalPinToBitMask(transmitPin);
  this->receivePin = receivePin;
  receivePort = digitalPinToPort(receivePin);
  receiveBitMask = digitalPinToBitMask(receivePin);
  this->receiveTransmits = receiveTransmits;
  this->plcReceiveCallback = plcReceiveCallback;
  // Setup IO fields
  ioStopState = phases * 2;
  inputDelayCycles = round(.5 * F_CPU * X10_SAMPLE_DELAY / 1000000);
  // Sine wave half cycle devided by number of phases
  outputDelayCycles = round(.5 * F_CPU / phases / sineWaveHz / 2);
  outputLengthCycles = round(.5 * F_CPU * X10_SIGNAL_LENGTH / 1000000);
  // Init. misc fields
  sendBfEnd = X10_BUFFER_SIZE - 1;
  rxHouse = DATA_UNKNOWN;
  rxUnit = DATA_UNKNOWN;
  rxExtUnit = DATA_UNKNOWN;
  rxCommand = DATA_UNKNOWN;
  x10exInstance = this;
}

//////////////////////////////
/// Public
//////////////////////////////

void X10ex::begin()
{
  // Using arduino digitalWrite here ensures that pins are
  // set up correctly (pwm timers are turned off, etc).
#if defined(ARDUINO) && ARDUINO >= 101
  pinMode(zeroCrossPin, INPUT_PULLUP);
  pinMode(receivePin, INPUT_PULLUP);
#else
  digitalWrite(zeroCrossPin, HIGH);
  pinMode(zeroCrossPin, INPUT);
  digitalWrite(receivePin, HIGH);
  pinMode(receivePin, INPUT);
#endif
  pinMode(transmitPin, OUTPUT);
  digitalWrite(transmitPin, LOW);
  // Setup IO timer
  TCCR1A = 0;
  TCCR1B = _BV(WGM13) & ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
  TIMSK1 = _BV(TOIE1);
  ICR1 = inputDelayCycles;
  // Attach zero cross interrupt
  attachInterrupt(zeroCrossInt, x10exZeroCross_wrapper, CHANGE);
  // Make sure interrupts are enabled
  sei();
  // Hack to get extra interrupt on non ATmega8, 168 and 328 pin 4 to 7
#if defined(__AVR_ATmega8__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)
  if(zeroCrossInt == 2 && zeroCrossPin >= 4 && zeroCrossPin <= 7)
  {
    PCMSK2 |= digitalPinToBitMask(zeroCrossPin);
    PCICR |= 0x01 << digitalPinToPort(zeroCrossPin) - 2;
  }
#endif
}

bool X10ex::sendAddress(uint8_t house, uint8_t unit, uint8_t repetitions)
{
  // Using CMD_STATUS_REQUEST to address modules (prepare for BRIGHT, DIM, etc.).
  // I've not seen any modules that reply to the request, and even if they did
  // it would not cause any harm.
  return sendCmd(house, unit, CMD_STATUS_REQUEST, repetitions);
}

bool X10ex::sendCmd(uint8_t house, uint8_t command, uint8_t repetitions)
{
  return sendCmd(house, 0, command, repetitions);
}

bool X10ex::sendCmd(uint8_t house, uint8_t unit, uint8_t command, uint8_t repetitions)
{
  return sendExt(house, unit, command, 0, 0, repetitions);
}

// You can enable this by changing defined value in header file. If you're using a
// PLC interface and modules that support extended code, use the "sendExtDim" method.
#if X10_USE_PRE_SET_DIM
bool X10ex::sendDim(uint8_t house, uint8_t unit, uint8_t percent, uint8_t repetitions)
{
  if(percent == 0)
  {
    return sendExt(house, unit, CMD_OFF, 0, 0, repetitions);
  }
  else
  {
    uint8_t brightness = percentToX10Brightness(percent * 2);
    return sendExt(
      house, unit, brightness >> 4 ? CMD_PRE_SET_DIM_1 : CMD_PRE_SET_DIM_0,
      // Reverse nibble before sending
      (((brightness * 0x0802LU & 0x22110LU) | (brightness * 0x8020LU & 0x88440LU)) * 0x10101LU >> 20) & B1111, 0,
      repetitions);
  }
}
#endif

// This method works with all X10 modules that support extended code. The only
// exception is the time attribute, that may result in unexpected behaviour
// when set to anything but the default value 0, on some X10 modules.
bool X10ex::sendExtDim(uint8_t house, uint8_t unit, uint8_t percent, uint8_t time, uint8_t repetitions)
{
  if(percent == 0)
  {
    return sendExt(house, unit, CMD_OFF, 0, 0, repetitions);
  }
  else
  {
    uint8_t data = percentToX10Brightness(percent, time);
    return sendExt(
      house, unit, CMD_EXTENDED_CODE,
      data, EXC_PRE_SET_DIM,
      repetitions);
  }
}

// Returns true when command was buffered successfully
bool X10ex::sendExt(uint8_t house, uint8_t unit, uint8_t command, uint8_t extData, uint8_t extCommand, uint8_t repetitions)
{
  house = parseHouseCode(house);
  unit--;
  // Validate input
  if(house > 0xF || (unit > 0xF && unit != 0xFF))
  {
    return 1;
  }
  // Add house nibble (bit 32-29)
  uint32_t message = (uint32_t)HOUSE_CODE[house] << 28;
  // No unit code X10 message
  if(unit == 0xFF)
  {
    message |=
      (uint32_t)command << 24 | // Add command nibble (bit 28-25)
      1LU << 23 |               // Set message type (bit 24) to 1 (command)
      X10_MSG_CMD;              // Set data type (bit 3-1)
  }
  // Standard X10 message
  else if(command != CMD_EXTENDED_CODE && command != CMD_EXTENDED_DATA)
  {
    // If type is preset dim, send data; if not, repeat house code
    uint8_t houseData = (command & B1110) == CMD_PRE_SET_DIM_0 ? extData : HOUSE_CODE[house];
    message |=
      (uint32_t)UNIT_CODE[unit] << 24 | // Add unit nibble (bit 28-25)
      (uint16_t)houseData << 8 |        // Add house/data nibble (bit 12-9)
      (uint16_t)command << 4 |          // Add command nibble (bit 8-5)
      1 << 3 |                          // Set message type (bit 4) to 1 (command)
      X10_MSG_STD;                      // Set data type (bit 3-1)
  }
  // Extended X10 message
  else
  {
    message |=
      (uint32_t)command << 24 |         // Add command nibble (bit 28-25)
      1LU << 23 |                       // Set message type (bit 24) to 1 (command)
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
    return 0;
  }
  // If slots are available in buffer
  else if((sendBfEnd + 2) % X10_BUFFER_SIZE != sendBfStart)
  {
    // Make sure identical message is not sent within rebuffer delay
    if(sendBf[sendBfEnd].message != message || millis() > sendBfLastMs + X10_REBUFFER_DELAY || sendBfLastMs - 1 > millis())
    {
      sendBfEnd = (sendBfEnd + 1) % X10_BUFFER_SIZE;
      // Buffer message and repetitions
      sendBf[sendBfEnd].message = message;
      sendBf[sendBfEnd].repetitions = repetitions;
      sendBfLastMs = millis();
    }
    // Return success even if message was not rebuffered because of rebuffer delay
    // There is really no point in buffering two identical commands in quick succession
    // If commands must be repeated several times, use the repetitions attribute
    return 0;
  }
  return 1;
}

X10state X10ex::getModuleState(uint8_t house, uint8_t unit)
{
  bool isSeen = 0;
  bool isKnown = 0;
  bool isOn = 0;
  uint8_t data = 0;
#if X10_PERSIST_MOD_DATA
  // Validate input
  #if X10_PERSIST_MOD_DATA == 1
  uint8_t state = eepromRead(house, unit);
  #else
  house = parseHouseCode(house);
  unit--;
  uint8_t state = 0;
  if(house <= 0xF && unit <= 0xF) state = moduleState[house << 4 | unit];
  #endif
  // Bit 1 and 2 in state byte has the state, last 6 bits is brightness
  // 00 = Not seen, Not known, Not On
  // 01 = Seen, Not known, Not On
  // 10 = Seen, Known, Not On
  // 11 = Seen, Known, On
  if(state & B11000000)
  {
    isSeen = 1;
    isKnown = state & B10000000;
    isOn = state >= B11000000;
    if(isKnown) data = state & B111111;
  }
#endif
  return (X10state) { isSeen, isKnown, isOn, data };
}

// WARNING:
// - If house is outside range A-P, state of all modules is wiped
// - If unit is outside range 1-16, state of all modules in house is wiped
void X10ex::wipeModuleState(uint8_t house, uint8_t unit)
{
#if X10_PERSIST_MOD_DATA
  wipeModuleData(house, unit, 0);
#endif
}

#if X10_PERSIST_MOD_DATA == 1
X10info X10ex::getModuleInfo(uint8_t house, uint8_t unit)
{
  uint8_t infoData = eepromRead(house, unit, 256);
  X10info info;
  info.type = infoData >> 6;
  uint8_t ix = 0;
  #if not defined(__AVR_ATmega8__) && not defined(__AVR_ATmega168__)
  if(infoData & B100000)
  {
    uint16_t nameAddr = (infoData & B11111) * X10_INFO_NAME_LEN + 512;
    while(ix < X10_INFO_NAME_LEN)
    {
      info.name[ix] = eepromRead(nameAddr + ix);
      if(info.name[ix] == '\0') break;
      ix++;
    }
  }
  #endif
  info.name[ix] = '\0';
  return info;
}

void X10ex::setModuleType(uint8_t house, uint8_t unit, uint8_t type)
{
  if(type <= B11)
  {
    // Make sure module is marked as seen
    updateModuleState(house, unit, DATA_UNKNOWN);
    // Update module type
    eepromWrite(house, unit, (eepromRead(house, unit, 256) | B11000000) & (type << 6 | B111111), 256);
  }
}

  #if not defined(__AVR_ATmega8__) && not defined(__AVR_ATmega168__)
bool X10ex::setModuleName(uint8_t house, uint8_t unit, char name[X10_INFO_NAME_LEN], uint8_t length)
{
  uint16_t nameAddr;
  uint8_t infoData = eepromRead(house, unit, 256);
  if(infoData & B100000)
  {
    nameAddr = (infoData & B11111) * X10_INFO_NAME_LEN + 512;
  }
  else
  {
    // Find empty slot in EEPROM
    nameAddr = 512;
    while(nameAddr < 1024)
    {
      if(eepromRead(nameAddr) == '\0') break;
      nameAddr += X10_INFO_NAME_LEN;
    }
    // If we have run out of space: abort by returning error code
    if(nameAddr > 1024 - X10_INFO_NAME_LEN) return 1;
    // Make sure module is marked as seen
    updateModuleState(house, unit, DATA_UNKNOWN);
    // Update module pointer to name address
    eepromWrite(
      house, unit,
      infoData & B11000000 | B100000 | (nameAddr / X10_INFO_NAME_LEN - 512),
      256);
  }
  for(uint8_t ix = 0; ix < X10_INFO_NAME_LEN; ix++)
  {
    if(name[ix] && ix < length)
    {
      eepromWrite(nameAddr + ix, name[ix]);
    }
    else
    {
      eepromWrite(nameAddr + ix, '\0');
      // Clear module pointer to name address if name is cleared
      if(!ix) eepromWrite(house, unit, infoData & B11000000, 256);
      break;
    }
  }
  return 0;
}
  #endif

// WARNING:
// - If house is outside range A-P, all module info is wiped
// - If unit is outside range 1-16, all module info in house is wiped
void X10ex::wipeModuleInfo(uint8_t house, uint8_t unit)
{
  wipeModuleData(house, unit, 1);
}
#endif

uint8_t X10ex::percentToX10Brightness(uint8_t brightness, uint8_t time)
{
  brightness = brightness >= 100 ? 62 : round(brightness / 100.0 * 62.0);
  brightness |= time >= B11 ? B11000000 : time << 6;
  return brightness;
}

uint8_t X10ex::x10BrightnessToPercent(uint8_t brightness)
{
  return round(100.0 * (brightness & B111111) / 62.0);
}

//////////////////////////////
/// Public (Interrupt Methods)
//////////////////////////////

void X10ex::zeroCross()
{
  zcInput = 0;
  // Start IO timer
  TCNT1 = 1;
  TCCR1B |= _BV(CS10);
  // Get bit to output from buffer
  if(sendBf[sendBfStart].repetitions && (sentCount || zeroCount > X10_PRE_CMD_CYCLES - 1))
  {
    // Start output as soon as possible after zero crossing
    if(zcOutput) fastDigitalWrite(transmitPort, transmitBitMask, HIGH);
    zcOutput = getBitToSend();
  }
  else
  {
    zcOutput = 0;
  }
}

void X10ex::ioTimer()
{
  // Read input
  if(ioState == 1)
  {
    ICR1 = outputLengthCycles - inputDelayCycles;
    zcInput = receiveTransmits || !sendBf[sendBfStart].repetitions ? !(*portInputRegister(receivePort) & receiveBitMask) : 0;
  }
  // Set output low, stop timer, and check receive
  else if((!zcOutput && ioState == 2) || ioState == ioStopState)
  {
    fastDigitalWrite(transmitPort, transmitBitMask, LOW);
    // Stop IO timer
    TCCR1B &= ~_BV(CS10);
    // Reset timer (ready for next zero cross)
    ICR1 = inputDelayCycles;
    ioState = 0;
    // If start sequence is found: receive message
    if(receivedCount)
    {
      receiveMessage();
    }
    // Search for start sequence and keep track of silence
    else
    {
      // If we received a one: increment bit count
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
          // We have reached zero crossing 4 after startcode: set it to start receiving message
          receivedCount = 4;
        }
        receivedBits = 0;
        zeroCount += zeroCount == 255 ? 0 : 1;
      }
    }
  }
  // Set output High
  else if(ioState % 2)
  {
    ICR1 = outputLengthCycles;
    fastDigitalWrite(transmitPort, transmitBitMask, HIGH);
  }
  // Set output Low
  else if(ioState)
  {
    ICR1 = outputDelayCycles - outputLengthCycles;
    fastDigitalWrite(transmitPort, transmitBitMask, LOW);
  }
  ioState++;
}

//////////////////////////////
/// Private
//////////////////////////////

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
    bool isOdd = sentCount % 2;
    // Get bit position in buffer
    uint8_t bitPosition = (sentCount - (isOdd ? 3 : 4)) / 2;
    // Get data type
    uint8_t type = sendBf[sendBfStart].message & B111;
    // Get bit to send from buffer, and xor it with odd field
    // to make complement bit for every even zero cross count
    output = !(sendBf[sendBfStart].message >> 32 - bitPosition & B1) ^ isOdd;
    // If type is standard X10 message
    if(type == X10_MSG_STD)
    {
      // Add cycles of silence after part one; make sure there are
      // 5 zero crosses of silence before part two is transmitted
      if(sentCount > 22 && sentCount <= 40)
      {
        output = 0;
        if(sentCount == 40 && zeroCount <= 2) sentCount--;
      }
      // Part one sent: restart by sending new start sequence then start at bit 21 in buffer
      if(sentCount == 40) sendOffset = 40;
    }
    // All messages end after 31 bits (the 62nd zero crossing)
    // If type is standard X10 message with no unit code: end after part one (11 bits)
    if(sentCount == 62 || (type == X10_MSG_CMD && sentCount == 22))
    {
      // If message has no unit code and command is BRIGHT or DIM: repeat without any silence
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
  // If data bit complement is correct
  else if(receivedDataBit != zcInput) 
  {
    receivedBits++;
    // Buffer one byte
    if(receivedBits < 9) receiveBuffer += receivedDataBit << 8 - receivedBits;
    // At zero crossing 22 standard message is complete: parse it
    if(receivedCount == 22)
    {
      receiveStandardMessage();
    }
    // Extended command received: parse extended message
    else if(rxCommand == CMD_EXTENDED_CODE || rxCommand == CMD_EXTENDED_DATA)
    {
      receiveExtendedMessage();
    }
  }
  // If data bit complement is no longer correct, it means we have stopped receiving data
  else
  {
    if(rxCommand != DATA_UNKNOWN)
    {
      uint8_t house = findCodeIndex(HOUSE_CODE, rxHouse) + 65;
      uint8_t unit = findCodeIndex(UNIT_CODE, rxExtUnit != DATA_UNKNOWN ? rxExtUnit : rxUnit) + 1;
#if X10_PERSIST_MOD_DATA
      if(unit) updateModuleState(house, unit, rxCommand);
#endif
      // Trigger receive callback
      plcReceiveCallback(house, unit, rxCommand, rxData, rxExtCommand, receivedBits);
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
#if X10_USE_PRE_SET_DIM
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
#endif
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

#if X10_PERSIST_MOD_DATA
void X10ex::updateModuleState(uint8_t house, uint8_t unit, uint8_t command)
{
  #if X10_PERSIST_MOD_DATA == 1
  uint8_t state = eepromRead(house, unit);
  #else
  uint8_t state = moduleState[parseHouseCode(house) << 4 | (unit - 1)];
  #endif
  // Bit 1 and 2 in state byte has the state, last 6 bits is brightness
  // 00 = Not seen, Not known, Not On
  // 01 = Seen, Not known, Not On
  // 10 = Seen, Known, Not On
  // 11 = Seen, Known, On
  uint8_t brightness = state & B111111;
  // If not seen, set seen
  if(!(state & B11000000)) state |= B1000000;
  // Dim: estimate brightness
  if(command == CMD_DIM)
  {
    // Module is off: set full brightness
    if(state >> 6 == B1)
    {
      brightness = 62;
    }
    // Module is on: decrease until limit
    else
    {
      brightness = brightness > 9 ? brightness - 9 : 1;
    }
  }
  // Bright: estimate brightness
  else if(command == CMD_BRIGHT)
  {
    // Module is off: set low brightness
    if(state >> 6 == B1)
    {
      brightness = 11;
    }
    // Module is on: increase until limit
    else
    {
      brightness = brightness <= 53 ? brightness + 9 : 62;
    }
  }
  // Off: set state and get brightness from buffer
  if(command == CMD_OFF || command == CMD_STATUS_OFF)
  {
    state = brightness | B10000000;
    rxData = brightness;
  }
  // On: set state and get brightness from buffer
  else if(command == CMD_DIM || command == CMD_BRIGHT || command == CMD_ON || command == CMD_STATUS_ON)
  {
    state = brightness | B11000000;
    rxData = brightness;
  }
  // X10 standard or extended code message pre set dim commands
  else if((command & B1110) == CMD_PRE_SET_DIM_0 || (command == CMD_EXTENDED_CODE && rxExtCommand == EXC_PRE_SET_DIM))
  {
    // Brightness > 0: update state and set brightness
    if(rxData > 0)
    {
      state = rxData | B11000000;
    }
    // Brightness 0: set state off
    else
    {
      state = brightness | B10000000;
    }
  }
  #if X10_PERSIST_MOD_DATA == 1
  eepromWrite(house, unit, state);
  #else
  moduleState[parseHouseCode(house) << 4 | (unit - 1)] = state;
  #endif
}
#endif

void X10ex::wipeModuleData(uint8_t house, uint8_t unit, bool info)
{
#if X10_PERSIST_MOD_DATA
  house = parseHouseCode(house);
  unit--;
  uint16_t ix = 0;
  uint8_t endIx = 255;
  if(house <= 0xF)
  {
    ix = house << 4;
    if(unit <= 0xF)
    {
      ix += unit;
      endIx = ix;
    }
    else
    {
      endIx = ix + 0xF;
    }
  }
  while(ix <= endIx)
  {
    if(info)
    {
  #if X10_PERSIST_MOD_DATA == 1
    #if not defined(__AVR_ATmega8__) && not defined(__AVR_ATmega168__)
      uint8_t infoData = eepromRead(ix + 256);
      if(infoData & B100000)
      {
        eepromWrite((infoData & B11111) * X10_INFO_NAME_LEN + 512, '\0');
      }
    #endif
      eepromWrite(ix + 256, 0);
  #endif
    }
    else
    {
  #if X10_PERSIST_MOD_DATA == 1
      eepromWrite(ix, 0);
  #else
      moduleState[ix] = 0;
  #endif
    }
    ix++;
  }
#endif
}

#if X10_PERSIST_MOD_DATA == 1
uint8_t X10ex::eepromRead(uint16_t address)
{
  // Return byte read from EEPROM, add 1 because of initial EEPROM value 255
  return eeprom_read_byte((unsigned char *)address) + 1;
}

uint8_t X10ex::eepromRead(uint8_t house, uint8_t unit, uint16_t offset)
{
  house = parseHouseCode(house);
  unit--;
  // If house or unit code is out of range: return 0
  if(house > 0xF || unit > 0xF) return 0;
  return eepromRead((house << 4 | unit) + offset);
}

uint8_t X10ex::eepromWrite(uint16_t address, uint8_t data)
{
  // Subtract 1 because of initial EEPROM value 255
  eeprom_write_byte((unsigned char *)address, data - 1);
}

uint8_t X10ex::eepromWrite(uint8_t house, uint8_t unit, uint8_t data, uint16_t offset)
{
  house = parseHouseCode(house);
  unit--;
  // If house and unit code is within valid range
  if(house <= 0xF && unit <= 0xF)
  {
    eepromWrite((house << 4 | unit) + offset, data);
  }
}
#endif

void X10ex::clearReceiveBuffer()
{
  receivedBits = 0;
  receiveBuffer = 0;
}

uint8_t X10ex::parseHouseCode(uint8_t house)
{
  return house - (house <= 0xF ? 0 : house >= 0x61 ? 0x61 : 0x41);
}

int8_t X10ex::findCodeIndex(const uint8_t codeList[16], uint8_t code)
{
  for(uint8_t i = 0; i <= 0xF; i++)
  {
    if(codeList[i] == code) return i;
  }
  return -1;
}

void X10ex::fastDigitalWrite(uint8_t port, uint8_t bitMask, uint8_t value)
{
  if(port == NOT_A_PIN) return;
  volatile uint8_t *out = portOutputRegister(port);
  uint8_t sreg = SREG;
  cli();
  if(value == LOW)
  {
    *out &= ~bitMask;
  }
  else
  {
    *out |= bitMask;
  }
  SREG = sreg;
}
