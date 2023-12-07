/*

  CanMsg.h

  The ESP32 requires to be connected to a CAN transceiver

*/

#ifndef __CAN_MSG__
#define __CAN_MSG__

#include <ACAN_ESP32.h>
#include "DCC.h"

/* ----- CAN ----------------------*/
// #define CAN_RX GPIO_NUM_4
// #define CAN_TX GPIO_NUM_5
#define CAN_BITRATE 1000UL * 1000UL // 1 Mb/s

class CanMsg
{
public:
  CanMsg() = delete;
  static void begin();
  static void loop();
  static void sendMsg(CANMessage &);
  static void sendMsg(byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte, byte, byte, byte);
};

#endif
