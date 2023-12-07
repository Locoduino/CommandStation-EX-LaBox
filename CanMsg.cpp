/*

  CanMsg.cpp


*/

#include "CanMsg.h"

void CanMsg::begin()
{
  // debug.printf("[CanConfig %d] : Configure ESP32 CAN\n", __LINE__);
  ACAN_ESP32_Settings settings(CAN_BITRATE);
  //  settings.mRxPin = GPIO_NUM_4 ; // Optional, default Tx pin is GPIO_NUM_4
  //  settings.mTxPin = GPIO_NUM_5 ; // Optional, default Rx pin is GPIO_NUM_5

  uint32_t errorCode;

  // with filter
  const ACAN_ESP32_Filter filter = ACAN_ESP32_Filter::singleExtendedFilter(
      ACAN_ESP32_Filter::data, 0xF << 7, 0x1FFFF87F);
  errorCode = ACAN_ESP32::can.begin(settings, filter);

  // without filter
  // errorCode = ACAN_ESP32::can.begin(settings);
  // debug.printf("[CanConfig %d] : config without filter\n", __LINE__);

  if (errorCode == 0)
    Serial.printf("[CanConfig %d] : configuration OK !\n", __LINE__);
  else
  {
    Serial.printf("[CanConfig %d] : configuration error 0x%x\n", __LINE__, errorCode);
    return;
  }
}

/*--------------------------------------
  Reception CAN
  --------------------------------------*/

void CanMsg::loop()
{
  CANMessage frame;
  if (ACAN_ESP32::can.receive(frame))
  {

    Serial.println("Can loop()");
    const byte idSatExpediteur = (frame.id & 0x7F80000) >> 19; // ID expediteur
    const byte fonction = (frame.id & 0x7F8) >> 3;

    Serial.printf("\n[CanMsg %d]------ Expediteur %d : Fonction 0x%0X\n", __LINE__, idSatExpediteur, fonction);

    if (frame.rtr) // Remote frame
      ACAN_ESP32::can.tryToSend(frame);
    else
    {
      switch (fonction) // Fonction appelée
      {
      case 0xF0:
        DCC::setThrottle((frame.data[0] << 8) + frame.data[1], frame.data[2], frame.data[3]);
        break;
      case 0xF1:
        DCC::setFunction((frame.data[0] << 8) + frame.data[1], frame.data[2] + 128, 0x00);
      case 0xF2:
        DCC::setFunction((frame.data[0] << 8) + frame.data[1], frame.data[2] + 176, 0x00);
      case 0xF3:
        DCC::setFunction((frame.data[0] << 8) + frame.data[1], frame.data[2] + 160, 0x00);
        break;
      case 0xFF:
        DCC::emergency(); // emergency stop
        break;
      }
    }
  }
}

/*--------------------------------------
  Envoi CAN
  --------------------------------------*/

void CanMsg::sendMsg(CANMessage &frame)
{
  if (0 == ACAN_ESP32::can.tryToSend(frame))
    Serial.printf("Echec envoi message CAN\n");
  else
    Serial.printf("Envoi fonction 0x%0X\n", frame.data[0]);
}

auto parseMsg = [](CANMessage &frame, byte priorite, byte idExp, byte idDes, byte fonct) -> CANMessage
{
  frame.id |= priorite << 27; // Priorite 0, 1 ou 2
  frame.id |= idExp << 19;    // ID expediteur
  frame.id |= idDes << 11;    // Hash
  frame.id |= fonct << 3;     // Fonction appelée
  frame.ext = true;
  return frame;
};

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 0;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 1;
  frame.data[0] = data0; // Fonction
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 2;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 3;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  frame.data[2] = data2;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2, byte data3)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 4;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2, byte data3, byte data4)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 5;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  frame.data[4] = data4;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2, byte data3, byte data4, byte data5)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 6;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  frame.data[4] = data4;
  frame.data[5] = data5;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2, byte data3, byte data4, byte data5, byte data6)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 7;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  frame.data[4] = data4;
  frame.data[5] = data5;
  frame.data[6] = data6;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2, byte data3, byte data4, byte data5, byte data6, byte data7)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 8;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  frame.data[4] = data4;
  frame.data[5] = data5;
  frame.data[6] = data6;
  frame.data[7] = data7;
  CanMsg::sendMsg(frame);
}
