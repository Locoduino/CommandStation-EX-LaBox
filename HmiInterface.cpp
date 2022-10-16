//#include "Labox.h"
#include "HmiInterface.hpp"

#ifdef USE_HMI
HmiInterface* HmiInterface::CurrentInterface = NULL;

HmiInterface::HmiInterface()
{
  this->pHmiInterfaceEventBuffer = new CircularBuffer(sizeof(HmiInterfaceMessage) * 10);
  this->pHmiInterfaceEventBuffer->begin(true);
}

void HmiInterface::WifiStartConnection(const char* pSsid)
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_WifiStartConnection;
  strcpy(msg.data.ssid, pSsid);

  this->pHmiInterfaceEventBuffer->PushBytes((byte *) &msg, sizeof(msg));
}

void HmiInterface::WifiConnected(IPAddress ip)
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_WifiConnected;
  msg.data.ip = ip;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::WifiEndConnection(const char* pSsid)
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_WifiEndConnection;
  strcpy(msg.data.ssid, pSsid);

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::NewClient(int clientId, IPAddress ip, int port)
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_NewClient;
  msg.data.client.clientId = clientId;
  msg.data.client.ip = ip;
  msg.data.client.port = port;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::CloseClient(int clientId)
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_CloseClient;
  msg.data.client.clientId = clientId;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::ChangeSpeed(uint16_t addr, uint8_t speed)
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_ChangeSpeed;
  msg.data.dcc.addr = addr;
  msg.data.dcc.speed = speed;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::ChangeDirection(uint16_t addr, bool forward)
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_ChangeDirection;
  msg.data.dcc.addr = addr;
  msg.data.dcc.forward = forward;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::ChangeFunction(uint16_t addr, uint8_t function, bool state)
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_ChangeFunction;
  msg.data.dcc.addr = addr;
  msg.data.dcc.function = function;
  msg.data.dcc.state = state;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::DCCOn()
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_DccOn;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::DCCOff()
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_DccOff;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::DCCShort()
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_DccShort;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::DCCEmergencyStop()
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_DccEmergencyStop;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::DCCGeneralStop()
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_DccGeneralStop;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::LocoAdd(const char* name, uint16_t addr)
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_LocoAdd;
  strcpy(msg.data.name, name);
  msg.data.dcc.addr = addr;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::LocoRemove(uint16_t addr)
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_LocoRemove;
  msg.data.dcc.addr = addr;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::WriteCV(bool mainTrack, uint16_t cvAddr, byte cvValue, bool success)
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_WriteCv;
  msg.data.dcc.mainTrack = mainTrack;
  msg.data.dcc.cvAddr = cvAddr;
  msg.data.dcc.cvValue = cvValue;
  msg.data.dcc.success = success;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}

void HmiInterface::ReadCV(bool mainTrack, uint16_t cvAddr, byte cvValue, bool success)
{
  HmiInterfaceMessage msg;
  msg.event = HmiInterfaceEvent_ReadCv;
  msg.data.dcc.mainTrack = mainTrack;
  msg.data.dcc.cvAddr = cvAddr;
  msg.data.dcc.cvValue = cvValue;
  msg.data.dcc.success = success;

  this->pHmiInterfaceEventBuffer->PushBytes((byte*)&msg, sizeof(msg));
}
#endif