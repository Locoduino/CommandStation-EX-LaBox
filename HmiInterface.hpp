//-------------------------------------------------------------------
#ifndef __HmiInterface_Hpp__
#define __HmiInterface_Hpp__
//-------------------------------------------------------------------

#ifdef USE_HMI
#include "CircularBuffer.hpp"

#define HmiInterfaceEvent_WifiStartConnection		0
#define HmiInterfaceEvent_WifiConnected					1
#define HmiInterfaceEvent_WifiEndConnection			2

#define HmiInterfaceEvent_NewClient							10
#define HmiInterfaceEvent_CloseClient						11

#define HmiInterfaceEvent_ChangeSpeed						20
#define HmiInterfaceEvent_ChangeDirection				21
#define HmiInterfaceEvent_ChangeFunction				22

#define HmiInterfaceEvent_DccOn									30
#define HmiInterfaceEvent_DccOff								31
#define HmiInterfaceEvent_DccShort							32
#define HmiInterfaceEvent_DccEmergencyStop			33
#define HmiInterfaceEvent_DccGeneralStop				34

#define HmiInterfaceEvent_LocoAdd								40
#define HmiInterfaceEvent_LocoRemove						41

#define HmiInterfaceEvent_WriteCv								50
#define HmiInterfaceEvent_ReadCv								51

struct HmiClient
{
	int clientId;
	IPAddress ip;
	int port;
};

struct Dcc
{
	uint16_t addr;
	uint8_t speed;
	bool forward;
	uint8_t function;
	bool state;
	bool mainTrack;
	uint16_t cvAddr;
	uint16_t cvValue;
	bool success;
};

union HmiInterfaceData
{
	char ssid[30];
	char name[30];
	IPAddress ip;
	HmiClient client;
	Dcc dcc;

	HmiInterfaceData() { ssid[0] = 0; }
	~HmiInterfaceData() {}
};

struct HmiInterfaceMessage
{
	byte event;
	HmiInterfaceData data;

	HmiInterfaceMessage() { this->event = 0; }
//	~HmiInterfaceMessage() {}
};

/** This interface established a relation between a User Interface and the DCC++ part of the library.
* If HmiInterface::CurrentInterface is not NULL, each User action will call a HMI routine to eventually
* show something on a screen...
*/
class HmiInterface
{
public:
	static HmiInterface* CurrentInterface;
	CircularBuffer *pHmiInterfaceEventBuffer;

	HmiInterface();

	void WifiStartConnection(const char* pSsid);
	void WifiConnected(IPAddress ip);
	void WifiEndConnection(const char* pSsid);

	void NewClient(int clientId, IPAddress ip, int port);
	void CloseClient(int clientId);

	void ChangeSpeed(uint16_t addr, uint8_t speed);
	void ChangeDirection(uint16_t addr, bool forward);
	void ChangeFunction(uint16_t addr, uint8_t function, bool state);
	void DCCOn();
	void DCCOff();
	void DCCShort();
	void DCCEmergencyStop();
	void DCCGeneralStop();

	void LocoAdd(const char* name, uint16_t addr);
	void LocoRemove(uint16_t addr);

	void WriteCV(bool mainTrack, uint16_t cvAddr, byte cvValue, bool success);
	void ReadCV(bool mainTrack, uint16_t cvAddr, byte cvValue, bool success);

	virtual bool HmiInterfaceLoop() = 0;
	virtual void HmiInterfaceUpdateDrawing() = 0;
};

#endif
#endif