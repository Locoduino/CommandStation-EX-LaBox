
/*
  CanMsg.h

  Christophe Bobille - Locoduino

  The ESP32 requires to be connected to a CAN transceiver
*/

#ifndef __CAN_MARKLIN__
#define __CAN_MARKLIN__

#include <Arduino.h>
#include "DCC.h"

#ifdef ENABLE_CANMARKLIN

#include <ACAN_ESP32.h>
#include "TrackManager.h"
#include "MotorDriver.h"
#include "EXComm.h"

#define CAN_SYSTEM_CONTROL 0x00
#define CAN_LOCO_SPEED 0x04
#define CAN_LOCO_DIREC 0x05
#define CAN_LOCO_FUNCT 0x06
#define CAN_LOCO_WRITECV_MAIN 0x08

#define CAN_FIRST_NOTLOCO_COMMAND 0x09

#define VERSION_LABOX_CAN "0.7.1"
// 0.7.1  - 05/09/24 : Modifications importantes pour les codes de commandes Respect prÈcis du protocole Marklin
// 0.7.0	- 26/08/24 : Passage ‡ EXComm
// 0.6.4  - 29/03/24 : Reformattage lÈger avec remplacement des commandes hexa par des defines.
//                     utilisation du define CAN pour exclure la compilation de ces sources
//                     utilisation de DIAG pour les messages √† la console.
// 0.6.3  - 21/02/24 : Modification des identifiants de messages CAN
// 0.6.2  - 12/12/23 : Ajout d'une m√©thode pour acquisition de hash (supprim√©e ensuite)
//                     La declaration des broches du CAN et de la vitesse ont ete deplac√©es lignes 184 √† 186 de config.h
// 0.6.1  - 11/12/23 : Ajout du retour d'informations
//                     Ajout de commandes dont la POM  case 0xF7:
//                       WRITE CV on MAIN <w CAB CV VALUE>
// 0.6.0  - 11/12/23 : Adoption d'un nouveau format de messages totalement incompatible avec les anciens
// 0.5.5  - 11/12/23 : Correction inversion :
//                     case 0xFE:
//                     TrackManager::setMainPower(frameIn.data[0] ? POWERMODE::OFF : POWERMODE::ON);
// 0.5.4  - 10/12/23 : Add POWERMODE::OVERLOAD
// 0.5.3  - 10/12/23
// 0.5.2  - 09/12/23
// 0.5.1  - 09/12/23 : Fix oubli break  case 0xFE:
//                     TrackManager::setMainPower(frameIn.data[0] ? POWERMODE::ON : POWERMODE::OFF);
//                     break;
// 0.5    - 09/12/23 : Ajout du retour d'information au programme de test. C'est la mesure de courant qui a ete choisie
//                     pour cela lignes 83 a 97 du programme.
// 0.4    - 08/12/23

class CanMarklinLoco
{
private:
  uint32_t address;
  uint16_t speed;
  bool direction;

public:
  CanMarklinLoco(uint32_t addr) : address(addr), direction(1) {}
  void sAddress(uint32_t address) { this->address = address; }
  void sSpeed(uint16_t speed) { this->speed = speed; }
  void sDirection(uint8_t direction) { this->direction = direction; }
  uint32_t gAddress() const { return address; }
  uint16_t gSpeed() const { return speed; }
  uint8_t gDirection() const { return direction; }

  static std::vector<CanMarklinLoco> locos;
  static CanMarklinLoco *findLoco(uint32_t address, std::vector<CanMarklinLoco> &locos);
};

class CanMarklin : public EXCommItem
{
private:
  static gpio_num_t RxPin;
  static gpio_num_t TxPin;
  static uint32_t DESIRED_BIT_RATE;
  static uint16_t thisId;
  static bool SendCommands;

public:
  // EXCommItem part
  CanMarklin(uint16_t id, gpio_num_t rxPin, gpio_num_t txPin, uint32_t bitRate, bool sendCommands) : EXCommItem("CAN")
  {
    thisId = id;
    RxPin = rxPin;
    TxPin = txPin;
    DESIRED_BIT_RATE = bitRate;
    SendCommands = sendCommands;
    this->MainTrackEnabled = true;
    this->ProgTrackEnabled = true;
    this->AlwaysLoop = false;
  }

  bool beginItem()
  {
    begin();
    return true;
  }
  bool loopItem()
  {
    loop();
    return true;
  }

  void sendPower(bool iSOn)
  {
    if (SendCommands)
      setPower(iSOn);
  }
  void sendThrottle(uint16_t cab, uint8_t tSpeed, bool tDirection)
  {
    if (SendCommands)
      setThrottle(cab, tSpeed, tDirection);
  }

  void sendFunction(int cab, int16_t functionNumber, bool on)
  {
    if (SendCommands)
      setFunction(cab, functionNumber, on);
  }
  void sendEmergency()
  {
    if (SendCommands)
      emergency();
  }
  // End EXCommItem

  static void begin();
  static void loop();
  static uint16_t getId();

  static void setPower(bool iSOn);
  static void setThrottle(uint16_t cab, uint8_t tSpeed, bool tDirection);
  
  static void setSpeed(uint16_t cab, uint8_t tSpeed);
  static void setDirection(uint16_t cab, bool tDirection);
  static void setFunction(int cab, int16_t functionNumber, bool on);
  static void emergency();

  static void sendMsg(CANMessage &);
};

#endif
#endif