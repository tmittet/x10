/************************************************************/
/* X10 RF receiver library.                                 */
/* This library is free software; you can redistribute it   */
/* and/or modify it under the terms of the GNU License v3.  */
/*                                                          */
/* Written by Thomas Mittet thomas@mittet.nu June 2010.     */
/************************************************************/

#ifndef X10rf_h
#define X10rf_h

#include <inttypes.h>

#define X10_RF_SB_MIN            4020
#define X10_RF_SB_MAX            9900
#define X10_RF_BIT0_MIN           400
#define X10_RF_BIT0_MAX          1200
#define X10_RF_BIT1_MIN          1500
#define X10_RF_BIT1_MAX          2500
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
  uint8_t receiveInt, receivePin, receivePort, receiveBitMask;
  rfReceiveCallback_t rfReceiveCallback;
  // Used by interrupt triggered methods
  uint32_t lowUs, receiveStarted;
  char house;
  uint8_t unit, command;
  // Private methods
  bool getBit();
  uint8_t getByte();
  bool verifyByte(uint8_t data);
  char parseHouseCode(uint8_t data);
};

#endif
