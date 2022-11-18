#include "DCCEX.h"
#include "hmi.h"
#include "hmiConfig.h"
#include "HmiInterface.hpp"

#ifdef USE_HMI
bool hmi::HmiInterfaceLoop()
{
  _HMIDEBUG_FCT_PRINTLN("hmi::HmiInterfaceLoop().. Begin");

/*#ifdef ARDUINO_ARCH_ESP32
  int core = xPortGetCoreID();
  if (core != this->executionCore || this->pHmiInterfaceEventBuffer == NULL)
    return false;
#endif*/

  HmiInterfaceMessage msg;
  char mess[LineCarNbMax];
  uint8_t speed;
  bool dir;
  
  if (!this->pHmiInterfaceEventBuffer->GetBytes((byte*)&msg, sizeof(msg)))
    return false;

  switch (msg.event)
  {
  case HmiInterfaceEvent_WifiStartConnection:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: WifiStartConnection");
    this->addNotification(0, HMI_WifiWaiting, 0);
    break;

  case HmiInterfaceEvent_WifiConnected:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: WifiConnected");

    addNotification(HMI_WifiOk);
    sprintf(mess, "%d.%d.%d.%d", msg.data.ip[0], msg.data.ip[1], msg.data.ip[2], msg.data.ip[3]);
    addNotification(mess);
    addNotification(TXT_BoxAddr);
    break;

  case HmiInterfaceEvent_WifiEndConnection:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: WifiEndConnection");
    this->addNotification(HMI_noWifi);
    break;

  case HmiInterfaceEvent_NewClient:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: NewClient");

    sprintf(mess, "#%d: %d.%d.%d.%d", msg.data.client.clientId, msg.data.client.ip[0], msg.data.client.ip[1], msg.data.client.ip[2], msg.data.client.ip[3]);
    this->addNotification(mess);
    break;

  case HmiInterfaceEvent_CloseClient:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: CloseClient");

    sprintf(mess, "#%d closed", msg.data.client.clientId);
    this->addNotification(mess);
    break;

  case HmiInterfaceEvent_ChangeSpeed:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: ChangeSpeed");
    speed = DCC::getThrottleSpeed(msg.data.dcc.addr);
    dir = DCC::getThrottleDirection(msg.data.dcc.addr);
    addNotification(msg.data.dcc.addr, dir ? HMI_OrderForward : HMI_OrderBack, speed);
    break;

  case HmiInterfaceEvent_ChangeDirection:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: ChangeDirection");
    dir = DCC::getThrottleDirection(msg.data.dcc.addr);
    addNotification(msg.data.dcc.addr, msg.data.dcc.forward ? HMI_OrderForward : HMI_OrderBack, dir);
    break;

  case HmiInterfaceEvent_ChangeFunction:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: ChangeFunction");
    addNotification(msg.data.dcc.addr, HMI_OrderFunction, msg.data.dcc.function, msg.data.dcc.state);
    break;

  case HmiInterfaceEvent_DccOn:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: DccOn");
    addNotification(HMI_StartDCC);
    break;

  case HmiInterfaceEvent_DccOff:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: DccOff");
    addNotification(HMI_OrderStopAll);
    break;

  case HmiInterfaceEvent_DccShort:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: Short");
    addNotification(HMI_ShortCurcuit);
    break;

  case HmiInterfaceEvent_DccEmergencyStop:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: EmergencyStop");
    // Todo : missing order emergency stop !
    addNotification(HMI_OrderStopAll);
    break;

  case HmiInterfaceEvent_DccGeneralStop:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: GeneralStop");
    addNotification(HMI_OrderStopAll);
    break;

  case HmiInterfaceEvent_LocoAdd:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: LocoAdd");
    speed = DCC::getThrottleSpeed(msg.data.dcc.addr);
    dir = DCC::getThrottleDirection(msg.data.dcc.addr);
    addNotification(msg.data.dcc.addr,
            dir ? HMI_OrderForward : HMI_OrderBack, 
            speed);
    break;

  case HmiInterfaceEvent_LocoRemove:
    _HMIDEBUG_LEVEL1_PRINTLN("HmiInterface :: LocoRemove");
    addNotification(msg.data.dcc.addr, HMI_OrderStop, 0);
    break;
  }
  _HMIDEBUG_FCT_PRINTLN("hmi::HmiInterfaceLoop().. End");

  return true;
}

void hmi::HmiInterfaceUpdateDrawing()
{
  this->update();
}
#endif