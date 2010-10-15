/************************************************************************/
/* X10 RF receiver library, v1.2.                                       */
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

#ifndef X10rf_h
#define X10rf_h

#include <inttypes.h>

// RF initial start burst min length
#define X10_RF_SB_MIN           12000
// RF start burst max length
#define X10_RF_SB_MAX           15000
// RF repeat start burst min length
// If you get choppy dimming, lower this threshold
#define X10_RF_RSB_MIN           7000
// RF bit 0 min length
#define X10_RF_BIT0_MIN          1000
// RF bit 0 max length
#define X10_RF_BIT0_MAX          1200
// RF bit 1 min length
#define X10_RF_BIT1_MIN          2100
// RF bit 1 max length
#define X10_RF_BIT1_MAX          2300
// When repeated start burst is detected within this millisecond threshold
// of the last command received, it is assumed that the following command
// is the same and that it does not need to be parsed
#define X10_RF_REPEAT_THRESHOLD   200

#define CMD_ON      B0010
#define CMD_OFF     B0011
#define CMD_DIM     B0100
#define CMD_BRIGHT  B0101

class X10rf
{

public:
  typedef void (*rfReceiveCallback_t)(char, uint8_t, uint8_t, bool);
  X10rf(uint8_t receiveInt, uint8_t receivePin, rfReceiveCallback_t rfReceiveCallback);
  // Public methods
  void begin();
  void receive();
  
private:
  static const uint8_t HOUSE_CODE[16];
  // Set in constructor
  uint8_t receiveInt, receivePin;
  rfReceiveCallback_t rfReceiveCallback;
  // Used by interrupt triggered methods
  uint32_t riseUs, receiveEnded;
  char house;
  uint8_t unit, command;
  int8_t receivedCount;
  uint32_t receiveBuffer;
  // Private methods
  void handleCommand(uint8_t byte1, uint8_t byte2);
  char parseHouseCode(uint8_t data);
};

#endif
