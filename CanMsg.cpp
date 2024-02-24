/*

  CanMsg.cpp

  v 0.4 - 08/12/23
  v 0.5 - 09/12/23 : Ajout du retour d'information au programme de test. C'est la mesure de courant qui a ete choisie
                    pour cela lignes 83 a 97 du programme.
  v 0.5.1 - 09/12/23 : Fix oubli break  case 0xFE:
                                          TrackManager::setMainPower(frameIn.data[0] ? POWERMODE::ON : POWERMODE::OFF);
                                        break;
  v 0.5.2 - 09/12/23
  v 0.5.3 - 10/12/23
  v 0.5.4 - 10/12/23 : Add POWERMODE::OVERLOAD
  v 0.5.5 - 11/12/23 : Correction inversion :
                       case 0xFE:
                       TrackManager::setMainPower(frameIn.data[0] ? POWERMODE::OFF : POWERMODE::ON);
  /*******************************************************************************************************
  v 0.6.0 - 11/12/23 : Adoption d'un nouveau format de messages totalement incompatible avec les anciens
  v 0.6.1 - 11/12/23 : Ajout du retour d'informations
                       Ajouts de commandes dont la POM  case 0xF7:
                                                        // WRITE CV on MAIN <w CAB CV VALUE>
  v 6.0.2 - 12/12/23 : Ajout d'une méthode pour acquisition de hash (supprimé ensuite)
                       La declaration des broches du CAN et de la vitesse ont ete deplcees lignes 184 à 186 de config.h
  v 6.0.3 - 21/02/24 : Modification des identifiants de messages CAN

*/

#include "CanMsg.h"

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
  Serial.printf("[CanMsg %d] : Configure ESP32 CAN\n", __LINE__);
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
    Serial.printf("[CanMsg %d] : configuration OK !\n", __LINE__);
  else
  {
    Serial.printf("[CanMsg %d] : configuration error 0x%x\n", __LINE__, errorCode);
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

    Serial.printf("\n[CanMsg %d]------ Expediteur %d : Commande 0x%0X\n\n", __LINE__, exped, cmde);

    if (frameIn.rtr) // Remote frame
      ACAN_ESP32::can.tryToSend(frameIn);
    else
    {
      uint16_t loco = 0;
      if (cmde < 0xFA)
        loco = (frameIn.data[0] << 8) + frameIn.data[1];

      switch (cmde) // Fonction appelée
      {
      case 0xF0:
        DCC::setThrottle(loco, frameIn.data[2], frameIn.data[3]);
        xQueueSendToBack(xQueue, &frameIn, 0);
        break;
      case 0xF1:
        DCC::setFn(loco, frameIn.data[2], frameIn.data[3]); // frame.data[2] = fonction, frame.data[3] : 'on' ou 'off'
        break;
      case 0xF7:
        // WRITE CV on MAIN <w CAB CV VALUE>
        DCC::writeCVByteMain(loco, frameIn.data[2], frameIn.data[3]);
        break;
      case 0xFE:
        TrackManager::setMainPower(frameIn.data[0] ? POWERMODE::ON : POWERMODE::OFF);
        xQueueSendToBack(xQueue, &frameIn, 0);
        break;
      case 0xFF:
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
    Serial.printf("Echec envoi message CAN\n");
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
