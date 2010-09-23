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

#ifndef X10ex_h
#define X10ex_h

#include <inttypes.h>

// Number of silent power line cycles before command is sent
#define X10_PRE_CMD_CYCLES    6
// Sample delay should be set to 500us according to spec
#define X10_SAMPLE_DELAY    500
// Signal length should be set to 1000us according to spec
#define X10_SIGNAL_LENGTH  1000
// Set buffer size to the number of individual messages you would like to
// buffer, plus one. The buffer is useful when triggering a scenario e.g.
// Each slot in the buffer uses 5 bytes of memory
#define X10_BUFFER_SIZE      16
// Set the min delay, in ms, between buffering of two identical messages
// This delay does not affect message repeats (when button is held)
#define X10_REBUFFER_DELAY  500
// When set to 1: module state data is stored in EEPROM. When set to 2:
// data is stored in volatile memory and cleared on reboot. When set to 0:
// state isn't stored at all and compiler ignores state code
#define X10_PERSIST_STATE     1
// Enable this to use X10 standard message PRE_SET_DIM commands.
// PRE_SET_DIM commands do not work with any of the European modules I've
// tested. I have no idea if it works at all, but it's part of the X10
// standard. If you're using a PLC interface and modules that support
// extended code: use the "sendExtDim" method in stead.
#define X10_USE_PRE_SET_DIM   0

// These are message buffer data types used to seperate X10 standard
// message format from extended message format, e.g.
#define X10_MSG_STD B001
#define X10_MSG_CMD B010
#define X10_MSG_EXT B011

#define DATA_UNKNOWN          0xF0

#define CMD_ALL_UNITS_OFF     B0000
#define CMD_ALL_LIGHTS_ON     B0001
#define CMD_ON                B0010
#define CMD_OFF               B0011
#define CMD_DIM               B0100
#define CMD_BRIGHT            B0101
#define CMD_ALL_LIGHTS_OFF    B0110
#define CMD_EXTENDED_CODE     B0111
#define CMD_HAIL_REQUEST      B1000
#define CMD_HAIL_ACKNOWLEDGE  B1001
#define CMD_PRE_SET_DIM_0     B1010
#define CMD_PRE_SET_DIM_1     B1011
#define CMD_EXTENDED_DATA     B1100
#define CMD_STATUS_ON         B1101
#define CMD_STATUS_OFF        B1110
#define CMD_STATUS_REQUEST    B1111

#define EXC_PRE_SET_DIM       B00110001
#define EXC_DIM_TIME_4        B00
#define EXC_DIM_TIME_30       B01
#define EXC_DIM_TIME_60       B10
#define EXC_DIM_TIME_300      B11

// Used when buffering messages
struct X10msg
{
  uint32_t message;
  uint8_t repetitions;
};

// Used when returning module state
struct X10state
{
  bool isKnown;
  bool isOn;
  uint8_t data;
};

class X10ex
{

  public:
    typedef void (*plcReceiveCallback_t)(char, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);
    // Phase retransmits not needed on European systems using the XM10 PLC interface,
    // so the phases and sineWaveHz parameters are optional and defaults to 1 and 50.
    X10ex(
      uint8_t zeroCrossInt, uint8_t zeroCrossPin, uint8_t transmitPin,
      uint8_t receivePin, bool receiveTransmits, plcReceiveCallback_t plcReceiveCallback,
      uint8_t phases = 1, uint8_t sineWaveHz = 50);
    // Public methods
    void begin();
    bool sendAddress(char house, uint8_t unit, uint8_t repetitions);
    bool sendCmd(char house, uint8_t command, uint8_t repetitions);
    bool sendCmd(char house, uint8_t unit, uint8_t command, uint8_t repetitions);
#if X10_USE_PRE_SET_DIM
    bool sendDim(char house, uint8_t unit, uint8_t percent, uint8_t repetitions);
#endif
    bool sendExtDim(char house, uint8_t unit, uint8_t percent, uint8_t time, uint8_t repetitions);
    bool sendExt(char house, uint8_t unit, uint8_t command, uint8_t extData, uint8_t extCommand, uint8_t repetitions);
    X10state getModuleState(char house, uint8_t unit);
    void wipeModuleState();
    void zeroCross();
	void ioTimer();
  
  private:
    static const uint8_t HOUSE_CODE[16];
    static const uint8_t UNIT_CODE[16];
    // Set in constructor
    uint8_t zeroCrossInt, zeroCrossPin, transmitPin, transmitPort, transmitBitMask, receivePin, receivePort, receiveBitMask, ioStopState;
	  uint16_t inputDelayCycles, outputDelayCycles, outputLengthCycles;
    bool receiveTransmits;
    plcReceiveCallback_t plcReceiveCallback;
	  // Transmit and receive fields
    int8_t ioState;
    volatile bool zcInput, zcOutput;
    // Transmit fields
    X10msg volatile sendBf[X10_BUFFER_SIZE];
    uint8_t volatile sendBfStart, sendBfEnd;
    uint32_t sendBfLastMs;
    uint8_t zeroCount, sentCount, sendOffset;
    // Receive fields
    bool receivedDataBit;
    uint8_t receivedCount, receivedBits, receiveBuffer;
    uint8_t rxHouse, rxUnit, rxExtUnit, rxCommand, rxData, rxExtCommand;
    // State stored in byte (8=On/Off, 7=State Known/Unknown, 6-1 data)
#if X10_PERSIST_STATE >= 2
    uint8_t moduleState[256];
#endif
    // Private methods
    bool getBitToSend();
    void receiveMessage();
    void receiveStandardMessage();
    void receiveExtendedMessage();
#if X10_PERSIST_STATE
    void updateModuleState(uint8_t index);
#endif
    void clearReceiveBuffer();
    int8_t findCodeIndex(const uint8_t codeList[16], uint8_t code);
    void fastDigitalWrite(uint8_t port, uint8_t bitMask, uint8_t value);
};

#endif