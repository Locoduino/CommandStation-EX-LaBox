
/*
  CanMsg.h

  Christophe Bobille - Locoduino

  The ESP32 requires to be connected to a CAN transceiver
*/

#ifndef __CAN_MSG__
#define __CAN_MSG__

#include <Arduino.h>
#include <ACAN_ESP32.h>
#include "DCC.h"

#ifdef CAN

#include "TrackManager.h"
#include "MotorDriver.h"

class CanMsg
{
private:
  static gpio_num_t RxPin;              
  static gpio_num_t TxPin;             
  static uint32_t DESIRED_BIT_RATE; 
  static uint8_t thisId;    

public:
  CanMsg() = delete;
  static void begin();
  static void loop();
  static uint8_t getId();
  static void sendMsg(CANMessage &);
  static void sendMsg(byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte, byte, byte);
};

#endif
#endif
