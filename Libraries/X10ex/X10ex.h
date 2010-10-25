/************************************************************************/
/* X10 Rx/Tx library for the XM10/TW7223/TW523 interface, v1.3.         */
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
/* Written by Thomas Mittet thomas@mittet.nu October 2010.              */
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
#define X10_BUFFER_SIZE      17
// Set the min delay, in ms, between buffering of two identical messages
// This delay does not affect message repeats (when button is held)
#define X10_REBUFFER_DELAY  500
// Chooses how to save module state and info (types and names).
// Set to 0: Neither state nor info is stored and state code is ignored
// Set to 1: Module state data and module info is stored in EEPROM.
// Set to 2: State data is stored in volatile memory and cleared on
// reboot. Module types and names are not stored when state is set to 2.
#define X10_PERSIST_MOD_DATA  1
// Length of module names stored in EEPROM, do not change if you don't
// know what you are doing. 4 and 8 should be valid, but this isn't tested.
#define X10_INFO_NAME_LEN    16
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
#define EXC_DIM_TIME_4        0
#define EXC_DIM_TIME_30       1
#define EXC_DIM_TIME_60       2
#define EXC_DIM_TIME_300      3

#define MODULE_TYPE_UNKNOWN   B00 // 0
#define MODULE_TYPE_APPLIANCE B01 // 1
#define MODULE_TYPE_DIMMER    B10 // 2
#define MODULE_TYPE_SENSOR    B11 // 3

// Used when buffering messages
struct X10msg
{
  uint32_t message;
  uint8_t repetitions;
};

// Used when returning module state
struct X10state
{
  bool isSeen;  // Is true when message addressed to module has been seen/received on power line
  bool isKnown; // Is true when module state ON/OFF is known
  bool isOn;    // Is true when appliance/dimmer module is ON
  uint8_t data;
};

// Used when returning module type and name
struct X10info
{
  uint8_t type;
  char name[X10_INFO_NAME_LEN + 1];
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
    bool sendAddress(uint8_t house, uint8_t unit, uint8_t repetitions);
    bool sendCmd(uint8_t house, uint8_t command, uint8_t repetitions);
    bool sendCmd(uint8_t house, uint8_t unit, uint8_t command, uint8_t repetitions);
#if X10_USE_PRE_SET_DIM
    bool sendDim(uint8_t house, uint8_t unit, uint8_t percent, uint8_t repetitions);
#endif
    bool sendExtDim(uint8_t house, uint8_t unit, uint8_t percent, uint8_t time, uint8_t repetitions);
    bool sendExt(uint8_t house, uint8_t unit, uint8_t command, uint8_t extData, uint8_t extCommand, uint8_t repetitions);
    X10state getModuleState(uint8_t house, uint8_t unit);
    void wipeModuleState(uint8_t house = '*', uint8_t unit = 0);
#if X10_PERSIST_MOD_DATA == 1
    X10info getModuleInfo(uint8_t house, uint8_t unit);
    void setModuleType(uint8_t house, uint8_t unit, uint8_t type);
  #if not defined(__AVR_ATmega8__) && not defined(__AVR_ATmega168__)
    bool setModuleName(uint8_t house, uint8_t unit, char name[X10_INFO_NAME_LEN], uint8_t length = X10_INFO_NAME_LEN);
  #endif
    void wipeModuleInfo(uint8_t house = '*', uint8_t unit = 0);
#endif
    uint8_t percentToX10Brightness(uint8_t brightness, uint8_t time = EXC_DIM_TIME_4);
    uint8_t x10BrightnessToPercent(uint8_t brightness);
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
    bool volatile zcInput, zcOutput;
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
#if X10_PERSIST_MOD_DATA >= 2
    uint8_t moduleState[256];
#endif
    // Private methods
    bool getBitToSend();
    void receiveMessage();
    void receiveStandardMessage();
    void receiveExtendedMessage();
#if X10_PERSIST_MOD_DATA
    void updateModuleState(uint8_t house, uint8_t unit, uint8_t command);
#endif
    void wipeModuleData(uint8_t house, uint8_t unit, bool info);
#if X10_PERSIST_MOD_DATA == 1
    uint8_t eepromRead(uint16_t address);
    uint8_t eepromRead(uint8_t house, uint8_t unit, uint16_t offset = 0);
    uint8_t eepromWrite(uint16_t address, uint8_t data);
    uint8_t eepromWrite(uint8_t house, uint8_t unit, uint8_t data, uint16_t offset = 0);
#endif
    void clearReceiveBuffer();
    uint8_t parseHouseCode(uint8_t house);
    int8_t findCodeIndex(const uint8_t codeList[16], uint8_t code);
    void fastDigitalWrite(uint8_t port, uint8_t bitMask, uint8_t value);
};

#endif