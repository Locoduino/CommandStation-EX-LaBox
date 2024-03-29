
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

#define CAN_LOCO_THROTTLE           0xF0
#define CAN_LOCO_FUNCTION           0xF1
#define CAN_LOCO_WRITECV_MAIN       0xF7

#define CAN_FIRST_NOTLOCO_COMMAND   0xFA

#define CAN_POWERON                 0xFE
#define CAN_EMERGENCY_STOP          0xFF

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
