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
/* Written by Thomas Mittet thomas@mittet.nu June 2010.                 */
/************************************************************************/

#ifndef X10ir_h
#define X10ir_h

#include <inttypes.h>

// With IR remotes: house, unit and command are sent separately. A default
// house code is set when initializing the IR library; this makes it
// possible to control a device without selecting house code every time.
// The house code can be changed by sending a set house code IR command.
// However: to avoid confusion when using IR remotes, it's a good idea to
// reset the house and unit codes to their default values after some time.
// The default behavior is to reset the house and unit codes after 20
// seconds of silence. To disable this feature set the reset time to 0.
#define X10_IR_UNIT_RESET_TIME  20000

// IR initial start burst min length
#define X10_IR_SB_MIN            3800
// IR start burst max length
#define X10_IR_SB_MAX            4400
// IR end burst min length
#define X10_IR_EB_MIN           11000
// IR bit 0 min length
#define X10_IR_BIT0_MIN          1050
// IR bit 0 max length
#define X10_IR_BIT0_MAX          1300
// IR bit 1 min length
#define X10_IR_BIT1_MIN          3550
// IR bit 1 max length
#define X10_IR_BIT1_MAX          4000
// When repeated start burst is detected within this millisecond threshold
// of the last command received, it is assumed that the following command
// is the same and that it does not need to be parsed
#define X10_IR_REPEAT_THRESHOLD   250

#define X10_IR_TYPE_HOUSE   B0001
#define X10_IR_TYPE_UNIT    B0000
#define X10_IR_TYPE_COMMAND B1000

#define DATA_UNKNOWN        0xF0

#define CMD_ADDRESS         0x10

#define CMD_ALL_UNITS_OFF   B0000
#define CMD_ALL_LIGHTS_ON   B0001
#define CMD_ON              B0010
#define CMD_OFF             B0011
#define CMD_DIM             B0100
#define CMD_BRIGHT          B0101

class X10ir
{

public:
  typedef void (*irReceiveCallback_t)(char, uint8_t, uint8_t, bool);
  X10ir(uint8_t receiveInt, uint8_t receivePin, char defaultHouse, irReceiveCallback_t irReceiveCallback);
  // Public methods
  void begin();
  void receive();
  
private:
  static const uint8_t HOUSE_CODE[16];
  static const uint8_t UNIT_CODE[16];
  // Set in constructor
  uint8_t receiveInt, receivePin, receivePort, receiveBitMask;
  irReceiveCallback_t irReceiveCallback;
  // Used by interrupt triggered methods
  uint32_t lowUs, receiveEnded;
#if X10_IR_UNIT_RESET_TIME
  char defaultHouse;
#endif
  char house;
  uint8_t unit, command;
  int8_t receivedCount;
  uint16_t receiveBuffer;
  // Private methods
  void triggerCallback(bool isRepeat);
  void handleCommand(uint8_t data);
  bool validateData(uint16_t data, uint8_t bits);
  int8_t findCodeIndex(const uint8_t codeList[16], uint8_t code);
};

#endif