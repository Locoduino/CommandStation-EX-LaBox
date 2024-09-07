/*

  CanMsg.cpp

 */

#include "CanMarklin.h"

#ifdef ENABLE_CANMARKLIN

gpio_num_t CanMarklin::RxPin = GPIO_NUM_4;
gpio_num_t CanMarklin::TxPin = GPIO_NUM_5;
uint32_t CanMarklin::DESIRED_BIT_RATE = 250ul * 1000ul;
uint16_t CanMarklin::thisId = 0x1810;
bool CanMarklin::SendCommands = false;

uint16_t CanMarklin::getId() { return thisId; }

QueueHandle_t xQueue; // Queue pour stocker les messages retour

/************************************************************************
Retour de LaBox pour les commandes de traction, la fonction emergency stop
la fonction power on / power off et plus generalement par la suite, toutes
les fonctions qui emettent une confirmation dans DCC-Ex
*************************************************************************/

/*--------------------------------------
  Envoi CAN
  --------------------------------------*/

void CanMarklin::sendMsg(CANMessage &frame)
{
  if (0 == ACAN_ESP32::can.tryToSend(frame))
    DIAG(F("[CanMarklin]------ Send error"));
}

void ackTask(void *pvParameters)
{
  BaseType_t status;
  CANMessage frame;

  for (;;)
  {
    status = xQueueReceive(xQueue, &frame, portMAX_DELAY);
    if (status == pdPASS)
    {
      frame.id &= ~0xFFFF;
      frame.id |= 1 << 17;             // Réponse
      frame.id |= CanMarklin::getId(); // ID expediteur
      CanMarklin::sendMsg(frame);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Recherche d'une locomotive par son adresse
CanMarklinLoco *CanMarklinLoco::findLoco(uint32_t address, std::vector<CanMarklinLoco> &locos)
{
  for (auto &loco : locos)
  {
    if (loco.gAddress() == address)
      return &loco;
  }
  // Ajout d'une nouvelle locomotive si n'existe pas
  CanMarklinLoco *newLoco = new CanMarklinLoco(address);
  locos.push_back(*newLoco); // Ajouter le nouvel objet à la liste
  return newLoco;
}

void CanMarklin::begin()
{
  DIAG(F("[CANMARKLIN] Configure ESP32 CAN"));

  if (DESIRED_BIT_RATE < 1000000UL)
    DIAG(F("[CANMARKLIN] CAN id = %d  Bitrate = %d Kb/s  CANH:%d  CANL:%d"), CanMarklin::thisId, DESIRED_BIT_RATE / 100UL, RxPin, TxPin);
  else
    DIAG(F("[CANMARKLIN] CAN id = %d  Bitrate = %d Mb/s  CANH:%d  CANL:%d"), CanMarklin::thisId, DESIRED_BIT_RATE / 1000000UL, RxPin, TxPin);

  ACAN_ESP32_Settings settings(DESIRED_BIT_RATE);
  settings.mRxPin = RxPin;
  settings.mTxPin = TxPin;

  uint32_t errorCode;

  // with filter
  const uint32_t inIdentifier = 0x00;
  const uint32_t inDontCareMask = 0x1e0fffff;
  const ACAN_ESP32_Filter filter = ACAN_ESP32_Filter::singleExtendedFilter(
      ACAN_ESP32_Filter::data, inIdentifier, inDontCareMask);
  errorCode = ACAN_ESP32::can.begin(DESIRED_BIT_RATE, filter);
  DIAG(F("[CANMARKLIN] : config with filter"));

  // without filter
  //  errorCode = ACAN_ESP32::can.begin(settings);
  //  DIAG(F("[CANMARKLIN] : config without filter"));

  if (errorCode == 0)
    DIAG(F("[CANMARKLIN] CAN Configuration OK !"));
  else
  {
    DIAG(F("[CANMARKLIN] CAN Configuration error 0x%x"), errorCode);
    return;
  }
  xQueue = xQueueCreate(50, sizeof(CANMessage));
  xTaskCreate(ackTask, "ackTask", 2 * 1024, NULL, 0, NULL);
}

/*--------------------------------------
  Reception CAN
  --------------------------------------*/

void CanMarklin::loop()
{
  CANMessage frameIn;
  if (ACAN_ESP32::can.receive(frameIn))
  {
    const uint8_t prio = (frameIn.id & 0x1E000000) >> 25; // Priorité
    const uint8_t cmde = (frameIn.id & 0x1FE0000) >> 17;  // Commande
    const uint8_t resp = (frameIn.id & 0x10000) >> 16;    // Reponse
    const uint16_t exped = (frameIn.id & 0xFFFF);         // Expéditeur

    char cmdName[20];
    switch (cmde)
    {
    // case CAN_LOCO_THROTTLE:        strcpy(cmdName, "Loco Throttle");        break;
    case CAN_LOCO_SPEED:
      strcpy(cmdName, "Loco Speed");
      break;
    case CAN_LOCO_DIREC:
      strcpy(cmdName, "Loco Direction");
      break;
    case CAN_LOCO_FUNCT:
      strcpy(cmdName, "Loco Function");
      break;
    case CAN_LOCO_WRITECV_MAIN:
      strcpy(cmdName, "Loco WriteCv main");
      break;
    case CAN_SYSTEM_CONTROL:
      strcpy(cmdName, "Systeme control");
      break;
    }

#ifdef CAN_DEBUG
    DIAG(F("[CANMARKLIN] ------ Sender %d : Command 0x%0X %s"), exped, cmde, cmdName);
#endif

    if (frameIn.rtr) // Remote frame
      ACAN_ESP32::can.tryToSend(frameIn);
    else
    {
      uint32_t locoAddress = 0;
      CanMarklinLoco *loco = nullptr;
      if (cmde > 0x03 && cmde < CAN_FIRST_NOTLOCO_COMMAND)
      {
        static std::vector<CanMarklinLoco> locos;
        //locoAddress = (frameIn.data[0] << 24) | (frameIn.data[1] << 16) | (frameIn.data[2] << 8) | frameIn.data[3];
        locoAddress = frameIn.data32[0];

        if (locoAddress >= 0x3FB7 && locoAddress < 0xC000)
          locoAddress = locoAddress - 0x3FB7; // Pour adresse ???
        if (locoAddress >= 0xC000 && locoAddress < 0xC0FF)
          locoAddress = locoAddress - 0xC000; // Pour adresse dcc
        else if (locoAddress >= 0xFF6E)
          locoAddress = locoAddress - 0xFF6E; // Pour adresse mfx

        loco = CanMarklinLoco::findLoco(locoAddress, locos);
        // Serial.print("loco address : ");
        // Serial.println(loco->gAddress());
        // Serial.print("loco speed : ");
        // Serial.println(loco->gSpeed());
        // Serial.print("loco direction : ");
        // Serial.println(loco->gDirection());
      }

      switch (cmde) // Commande appelée
      {
      case CAN_LOCO_SPEED:
        if (loco != nullptr)
        {
          loco->sSpeed((uint16_t)(frameIn.data[4] << 8) | frameIn.data[5]);
          if (loco->gSpeed() > 1000)
            loco->sSpeed(1000);
          uint16_t speed = map(loco->gSpeed(), 0, 1000, 0, 127);
          if (speed == 1)
            speed = 0;
          loco->sSpeed(speed);
          DCC::setThrottle(loco->gAddress(), loco->gSpeed(), loco->gDirection());
        }
        xQueueSendToBack(xQueue, &frameIn, 0);
        break;
      case CAN_LOCO_DIREC:
        // Signification du paramètre Direction :
        // 0 = sens de marche sans changement
        // 1 = sens de marche avant
        // 2 = sens de marche arrière
        // 3 = inverser le sens de marche

        if (loco != nullptr)
        {
          uint8_t direction = frameIn.data[4];
          if (direction > 0)
          {
            switch (direction)
            {
            case 1:
              loco->sDirection(1);
              loco->sSpeed(0);
              break;
            case 2:
              loco->sDirection(0);
              loco->sSpeed(0);
              break;
            case 3:
              loco->sDirection(!loco->gDirection());
              loco->sSpeed(0);
              break;
            }
            DCC::setThrottle(loco->gAddress(), loco->gSpeed(), loco->gDirection());
          }
          xQueueSendToBack(xQueue, &frameIn, 0);
        }
        break;
      case CAN_LOCO_FUNCT:
        DCC::setFn(loco->gAddress(), frameIn.data[4], frameIn.data[5]);
        break;
      case CAN_LOCO_WRITECV_MAIN:
        // WRITE CV on MAIN <w CAB CV VALUE>
        DCC::writeCVByteMain(loco->gAddress(), (frameIn.data[4] << 8) | frameIn.data[5], frameIn.data[6]);
        break;
      case CAN_SYSTEM_CONTROL:
        switch (frameIn.data[4])
        {
        case (0x00):
        case (0x01):
          TrackManager::setMainPower(frameIn.data[4] ? POWERMODE::ON : POWERMODE::OFF);
          xQueueSendToBack(xQueue, &frameIn, 0);
          break;
        case (0x02):
          DCC::setThrottle(0, 1, 1); // emergency stop
          xQueueSendToBack(xQueue, &frameIn, 0);
          break;
        case (0x03):
          DCC::setThrottle(loco->gAddress(), 1, loco->gDirection()); // emergency stop (only one loco)
          xQueueSendToBack(xQueue, &frameIn, 0);
          break;
        }
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

void CanMarklin::setPower(bool isOn)
{
  // CanMarklin::sendMsg(2, CAN_POWERON, CanMarklin::getId(), thisId, 0, isOn);
}

void CanMarklin::setThrottle(uint16_t cab, uint8_t tSpeed, bool tDirection)
{
  if (tSpeed == 1) // Emergency_stop !
    CanMarklin::emergency();
  else
  {
    // CanMarklin::sendMsg(2, CAN_LOCO_THROTTLE, CanMarklin::getId(), thisId, 0, (cab & 0xFF00) >> 8, cab & 0x00FF, tSpeed, tDirection);
  }
}

void CanMarklin::setFunction(int cab, int16_t functionNumber, bool on)
{
  // CanMarklin::sendMsg(2, CAN_LOCO_FUNCTION, CanMarklin::getId(), thisId, 0, (cab & 0xFF00) >> 8, cab & 0x00FF, functionNumber, on);
}

void CanMarklin::emergency()
{
  // CanMarklin::sendMsg(0, CAN_EMERGENCY_STOP, CanMarklin::getId(), thisId, 0);
}

#endif