/*

  CanMsg.cpp

 */

#include "CanMsg.h"

#ifdef CAN

// - Config, voir lignes 184 à 186 de config.h
gpio_num_t CanMsg::RxPin = CAN_RX;
gpio_num_t CanMsg::TxPin = CAN_TX;
uint32_t CanMsg::DESIRED_BIT_RATE = CAN_BITRATE;
uint8_t CanMsg::thisId = 253;

uint8_t CanMsg::getId() { return thisId; }

QueueHandle_t xQueue; // Queue pour stocker les messages retour

/************************************************************************
Retour de LaBox pour les commandes de traction,la fonction emergency stop
la fonction power on / power off et plus generalement par la suite, toutes
les fonctions qui emettent une confirmation dans DCC-Ex
*************************************************************************/

void ackTask(void *pvParameters)
{
  BaseType_t status;
  CANMessage frame;
  //frame.ext = true;

  for (;;)
  {
    status = xQueueReceive(xQueue, &frame, pdMS_TO_TICKS(100));
    if (status == pdPASS)
    {
      frame.id &= ~ 0x18000000;
      frame.id |= 3 << 27;               // Priorite 0, 1, 2 ou 3
      frame.id &= ~ 0x7F800;
      frame.id |= CanMsg::getId() << 11; // ID expediteur
      frame.id &= ~ 0x04;
      frame.id |= 1 << 2;                // Réponse
      CanMsg::sendMsg(frame);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void CanMsg::begin()
{
  DIAG(F("[CanMsg %d] : Configure ESP32 CAN"), __LINE__);
  ACAN_ESP32_Settings settings(DESIRED_BIT_RATE);
  settings.mRxPin = RxPin;
  settings.mTxPin = TxPin;

  uint32_t errorCode;

  // with filter
  const ACAN_ESP32_Filter filter = ACAN_ESP32_Filter::singleExtendedFilter(
      ACAN_ESP32_Filter::data, 0xF << 23, 0x187FFFFB);
  errorCode = ACAN_ESP32::can.begin(settings, filter);

  // without filter
  // errorCode = ACAN_ESP32::can.begin(settings);
  // Serial.printf("[CanMsg %d] : config without filter\n", __LINE__);

  if (errorCode == 0)
    DIAG(F("[CanMsg %d] : configuration OK !"), __LINE__);
  else
  {
    DIAG(F("[CanMsg %d] : configuration error 0x%x"), __LINE__, errorCode);
    return;
  }
  xQueue = xQueueCreate(10, sizeof(CANMessage));
  xTaskCreate(ackTask, "ackTask", 2 * 1024, NULL, 0, NULL);
}

/*--------------------------------------
  Reception CAN
  --------------------------------------*/

void CanMsg::loop()
{
  CANMessage frameIn;
  if (ACAN_ESP32::can.receive(frameIn))
  {
    const uint8_t cmde = (frameIn.id & 0x7F80000) >> 19; // Commande
    const uint8_t exped = (frameIn.id & 0x7F800) >> 11;  // Expéditeur
    const uint8_t resp = (frameIn.id & 0x04) >> 2;       // Commande = 0 / Reponse = 1

    char cmdName[20];
    switch(cmde)
    {
      case CAN_LOCO_THROTTLE:        strcpy(cmdName, "Loco Throttle");        break;
      case CAN_LOCO_FUNCTION:        strcpy(cmdName, "Loco Function");        break;
      case CAN_LOCO_WRITECV_MAIN:    strcpy(cmdName, "Loco WriteCv main");    break;
      case CAN_POWERON:              strcpy(cmdName, "Power");                break;
      case CAN_EMERGENCY_STOP:       strcpy(cmdName, "Emergency Stop");       break;
    }

    DIAG(F("[CanMsg %d]------ Expediteur %d : Commande 0x%0X %s"), __LINE__, exped, cmde, cmdName);

    if (frameIn.rtr) // Remote frame
      ACAN_ESP32::can.tryToSend(frameIn);
    else
    {
      uint16_t loco = 0;
      if (cmde < CAN_FIRST_NOTLOCO_COMMAND)
        loco = (frameIn.data[0] << 8) + frameIn.data[1];

      switch (cmde) // Fonction appelée
      {
      case CAN_LOCO_THROTTLE:
        DCC::setThrottle(loco, frameIn.data[2], frameIn.data[3]);
        xQueueSendToBack(xQueue, &frameIn, 0);
        break;
      case CAN_LOCO_FUNCTION:
        DCC::setFn(loco, frameIn.data[2], frameIn.data[3]); // frame.data[2] = fonction, frame.data[3] : 'on' ou 'off'
        break;
      case CAN_LOCO_WRITECV_MAIN:
        // WRITE CV on MAIN <w CAB CV VALUE>
        DCC::writeCVByteMain(loco, frameIn.data[2], frameIn.data[3]);
        break;
      case CAN_POWERON:
        TrackManager::setMainPower(frameIn.data[0] ? POWERMODE::ON : POWERMODE::OFF);
        xQueueSendToBack(xQueue, &frameIn, 0);
        break;
      case CAN_EMERGENCY_STOP:
        DCC::setThrottle(0, 1, 1); // emergency stop
        xQueueSendToBack(xQueue, &frameIn, 0);
        break;
      }
    }
  }

  /*--------------------------------------
    Envoi mesure de courant
    --------------------------------------*/
// #define MESURE_COURANT
#ifdef MESURE_COURANT
  static uint64_t millisRefreshData = 0;
  if (millis() - millisRefreshData > 1000)
  {
    CANMessage frameOut;
    MotorDriver *mainDriver = NULL;
    for (const auto &md : TrackManager::getMainDrivers())
      mainDriver = md;
    if (mainDriver == NULL || !mainDriver->canMeasureCurrent())
      return;

    POWERMODE mode = TrackManager::getMainPower();
    if (mode == POWERMODE::ON)
    {
      uint16_t current = mainDriver->getCurrentRaw();
      sendMsg(0, 0xFD, CanMsg::getId(), 0, 1, (current & 0xFF00) >> 8, current & 0x00FF);
    }
    else if (mode == POWERMODE::OVERLOAD)
      sendMsg(0, 0xFD, CanMsg::getId(), 0, 1, 2);
    else
      sendMsg(0, 0xFD, CanMsg::getId(), 0, 1, 0);
    millisRefreshData = millis();
  }
#endif
}

/*--------------------------------------
  Envoi CAN
  --------------------------------------*/

void CanMsg::sendMsg(CANMessage &frame)
{
  if (0 == ACAN_ESP32::can.tryToSend(frame))
    DIAG(F("Echec envoi message CAN"));
}

auto formatMsg = [](CANMessage &frame, byte prio, byte cmde, byte thisNodeId, byte destNodeId, byte resp) -> CANMessage
{
  frame.id |= prio << 27;       // Priorite 0, 1, 2 ou 3
  frame.id |= cmde << 19;       // commande appelée
  frame.id |= thisNodeId << 11; // ID expediteur
  frame.id |= destNodeId << 3;  // ID destinataire
  frame.id |= resp << 2;        // Réponse
  frame.ext = true;
  return frame;
};

void CanMsg::sendMsg(byte prio, byte cmde, byte thisNodeId, byte destNodeId, byte resp)
{
  CANMessage frame;
  frame = formatMsg(frame, prio, cmde, thisNodeId, destNodeId, resp);
  frame.len = 0;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte prio, byte cmde, byte thisNodeId, byte destNodeId, byte resp, byte data0)
{
  CANMessage frame;
  frame = formatMsg(frame, prio, cmde, thisNodeId, destNodeId, resp);
  frame.len = 1;
  frame.data[0] = data0;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte prio, byte cmde, byte thisNodeId, byte destNodeId, byte resp, byte data0, byte data1)
{
  CANMessage frame;
  frame = formatMsg(frame, prio, cmde, thisNodeId, destNodeId, resp);
  frame.len = 2;
  frame.data[0] = data0;
  frame.data[1] = data1;
  CanMsg::sendMsg(frame);
}
void CanMsg::sendMsg(byte prio, byte cmde, byte thisNodeId, byte destNodeId, byte resp, byte data0, byte data1, byte data2)
{
  CANMessage frame;
  frame = formatMsg(frame, prio, cmde, thisNodeId, destNodeId, resp);
  frame.len = 3;
  frame.data[0] = data0;
  frame.data[1] = data1;
  frame.data[2] = data2;
  CanMsg::sendMsg(frame);
}
void CanMsg::sendMsg(byte prio, byte cmde, byte thisNodeId, byte destNodeId, byte resp, byte data0, byte data1, byte data2, byte data3)
{
  CANMessage frame;
  frame = formatMsg(frame, prio, cmde, thisNodeId, destNodeId, resp);
  frame.len = 4;
  frame.data[0] = data0;
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  CanMsg::sendMsg(frame);
}
#endif