/*
 *  © 2023 Thierry Paris / Locoduino
 *  All rights reserved.
 *  
 *  This file is part of CommandStation-EX-Labox
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  It is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with CommandStation.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <Arduino.h>
#include "defines.h"

#ifdef ENABLE_Z21

#include "Z21Throttle.h"
#include "DCC.h"
#include "WifiESP32.h"
#include "DCCWaveform.h"
#include "StringFormatter.h"
#include "Turnouts.h"
#include "DIAG.h"
#include "GITHUB_SHA.h"
#include "version.h"
#include "EXRAIL2.h"
#include "CommandDistributor.h"
#include "TrackManager.h"
#include "DCCTimer.h"
#ifdef USE_HMI
	#include "hmi.h"
#endif

static std::vector<NetworkClientUDP> clientsUDP; // a list to hold all UDP clients

bool Z21Throttle::DIAGBASE=false;
bool Z21Throttle::DIAGBROADCAST=false;
bool Z21Throttle::DIAGDATA=false;
bool Z21Throttle::DIAGVERBOSE=false;

#define DIAG_Z21	 				if (Z21Throttle::DIAGBASE) DIAG
#define DIAG_Z21BROADCAST	if (Z21Throttle::DIAGBROADCAST) DIAG
#define DIAG_Z21DATA		 	if (Z21Throttle::DIAGDATA) DIAG
#define DIAG_Z21VERBOSE	 	if (Z21Throttle::DIAGVERBOSE) DIAG

#define LOOPLOCOS(CAB)  for (int loco=0;loco<MAX_MY_LOCO;loco++)
#define LOOPTHROTTLES  Z21Throttle *pLoopThrottle = Z21Throttle::getFirstThrottle(); for (; pLoopThrottle != NULL; pLoopThrottle = pLoopThrottle->getNextThrottle())
#define LOOPCLIENTS  for (int clientId = 0; clientId < clientsUDP.size(); clientId++)

Z21Throttle *Z21Throttle::firstThrottle=NULL;
byte Z21Throttle::commBuffer[100];
byte Z21Throttle::replyBuffer[20];

Z21Throttle* Z21Throttle::readWriteThrottle = NULL;
int Z21Throttle::cvAddress = -1;
int Z21Throttle::cvValue = -1;
int Z21EXCommItem::UDPport = Z21_UDPPORT;

void printClientsUDP();

WiFiUDP NetworkClientUDP::client;

bool Z21EXCommItem::begin() {
#ifdef DCCPP_DEBUG_MODE
	CircularBuffer::Test();
#endif
	bool retf = true;

	IPAddress ip;

	if (WiFi.getMode() == wifi_mode_t::WIFI_MODE_STA)
		ip = WiFi.localIP();
	if (WiFi.getMode() == wifi_mode_t::WIFI_MODE_AP)
		ip = WiFi.softAPIP();	

	uint8_t ret = NetworkClientUDP::client.begin(ip, UDPport);
	if (ret == 1) {
		retf = true;
		NetworkClientUDP::client.flush();
	}

	if (retf)
    DIAG(F("[Z21] UDP Connection started port %d. Z21 apps are available."), UDPport);
  else {
		DIAG(F("[Z21] UDP Connection failed. Z21 apps not available."));
		UDPport = -1;
	}

	return retf;
}

void Z21EXCommItem::getInfos(String *pMess1, String *pMess2, String *pMess3, byte maxSize) 
{
	char mess[maxSize*2];

	if (UDPport == -1)
		sprintf(mess, "[Z21] Not connected");
	else
		sprintf(mess, "[Z21] Port %d", UDPport);

	*pMess1 = mess;
}

void Z21EXCommItem::broadcastLoco(int16_t cab) {
	DIAG_Z21BROADCAST(F("[Z21] start Broadcasting loco %d ----"), cab);
	LOOPTHROTTLES {
		if (pLoopThrottle->isBroadcastFlag()) {
			int DB2 = cab & 0xFF;          // Extraction de inDB2
			int DB1 = (cab >> 8) & 0x3F;   // Extraction de inDB1
			Z21Throttle::notifyLocoInfo(pLoopThrottle->getClientId(), DB1, DB2);
		}
	}
	DIAG_Z21BROADCAST(F("[Z21] end Broadcasting locos   ----"));
}

void Z21EXCommItem::broadcastSensor(int16_t id, bool value) {

}

void Z21EXCommItem::broadcastTurnout(int16_t id, bool isClosed) {
	DIAG_Z21BROADCAST(F("[Z21] start Broadcasting turnouts ----"));
	LOOPTHROTTLES {
		if (pLoopThrottle->isBroadcastFlag()) {
			int DB2 = id & 0xFF;
			int DB1 = (id >> 8);
			Z21Throttle::notifyTurnoutInfo(pLoopThrottle->getClientId(), DB1, DB2);
		}
	}
	DIAG_Z21BROADCAST(F("[Z21] end Broadcasting turnouts   ----"));
}

void Z21EXCommItem::broadcastPower() {
	DIAG_Z21BROADCAST(F("[Z21] start Broadcasting power ----"));
	LOOPTHROTTLES {
			Z21Throttle::notifyTrPw(pLoopThrottle->getClientId(), TrackManager::getMainPower() == POWERMODE::ON ? 1 : 0);
	}
	DIAG_Z21BROADCAST(F("[Z21] end Broadcasting power   ----"));
}

int readUdpPacket() {
	byte udp[UDPBYTE_SIZE];
	memset(udp, 0, UDPBYTE_SIZE);
	int len = NetworkClientUDP::client.read(udp, UDPBYTE_SIZE);
	if (len > 0) {
		for (int clientId = 0; clientId < clientsUDP.size(); clientId++) {
			if (clientsUDP[clientId].inUse) {
				if (clientsUDP[clientId].remoteIP == NetworkClientUDP::client.remoteIP() && clientsUDP[clientId].remotePort == NetworkClientUDP::client.remotePort()) {
					DIAG_Z21DATA(F("[Z21] %d: <- udp_len:%d"), clientId, len);

					// Black Z21 Android app send a bunch of 1000 bytes of value 0 ! 
					// Dont know why, but do not add this to the circular buffer !
					if (len < Z21_MAXIMAL_UDP_MSG_SIZE) {
						clientsUDP[clientId].pudpBuffer->PushBytes(udp, len);

						//if (Z21Throttle::Z21THROTTLEDATA) clientsUDP[clientId].pudpBuffer->printStatus();
					}
					else {
						DIAG_Z21(F("[Z21] %d: <- long message ignored : udp_len:%d   first byte : %d"), clientId, len, udp[0]);

						if (Z21Throttle::DIAGDATA) {
							for(int i = 0; i < 10; i++) {
								if (len > i*10) {
									DIAG_Z21DATA(F("   UDP %d-%d : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x"),
															i*10, (i*10)+9,
															(len > i*10)      ?udp[i*10]:0,
															(len > (i*10) + 1)?udp[(i*10)+1]:0,
															(len > (i*10) + 2)?udp[(i*10)+2]:0,
															(len > (i*10) + 3)?udp[(i*10)+3]:0,
															(len > (i*10) + 4)?udp[(i*10)+4]:0,
															(len > (i*10) + 5)?udp[(i*10)+5]:0,
															(len > (i*10) + 6)?udp[(i*10)+6]:0,
															(len > (i*10) + 7)?udp[(i*10)+7]:0,
															(len > (i*10) + 8)?udp[(i*10)+8]:0,
															(len > (i*10) + 9)?udp[(i*10)+9]:0);
								}
							}
						}
					}

					return clientId;
				}
			}
		}
	}

	return -1;
}

void Z21Throttle::loop() {
  if (Z21EXCommItem::UDPport < 0) {
		return;
	}

	int clientId = 0;

	// loop over all clients and remove inactive
	for (clientId = 0; clientId < clientsUDP.size(); clientId++) {
		if (!clientsUDP[clientId].inUse) {
			continue;
		}

		Z21Throttle* pThrottle = getOrAddThrottle(clientId); 

		if (pThrottle->lastHeartBeatDate != 0 && millis() - pThrottle->lastHeartBeatDate > Z21_TIMEOUT) {
			clientsUDP[clientId].connected = false;
		}

		// check if client is there and alive
		if (clientsUDP[clientId].inUse && !clientsUDP[clientId].connected) {
			DIAG_Z21(F("[Z21] Remove disconnected UDP client %d"), clientId);
			clientsUDP[clientId].inUse = false;
			printClientsUDP();
		}
	}

	int len = NetworkClientUDP::client.parsePacket();
	if (len > 0) {
		// Check if this IP is already in the client list.
		int clientId = 0;
		for (; clientId < clientsUDP.size(); clientId++) {
			if (clientsUDP[clientId].inUse) {
				if (clientsUDP[clientId].remoteIP == NetworkClientUDP::client.remoteIP() && clientsUDP[clientId].remotePort == NetworkClientUDP::client.remotePort()) {
					//DIAG_Z21VERBOSE(F("UDP client %d : %s Already connected"), clientId, clientsUDP[clientId].remoteIP.toString().c_str());
					break;
				}
			}
		}

		// If this IP is not already present, try tyo find the first slot available

		if (clientId >= clientsUDP.size()) {
			for (clientId = 0; clientId < clientsUDP.size(); clientId++) {
				if (!clientsUDP[clientId].inUse) {
					break;
				}
			}

			// If no empty slot found, add a new one.
			if (clientId >= clientsUDP.size()) {
				NetworkClientUDP nc;
				clientsUDP.push_back(nc);
			}

			clientsUDP[clientId].remoteIP = NetworkClientUDP::client.remoteIP();
			clientsUDP[clientId].remotePort = NetworkClientUDP::client.remotePort();
			clientsUDP[clientId].connected = true;
			clientsUDP[clientId].inUse = true;

			DIAG_Z21(F("[Z21] New UDP client %d, %s"), clientId, clientsUDP[clientId].remoteIP.toString().c_str());
			printClientsUDP();
			#ifdef USE_HMI
			if (hmi::CurrentInterface != NULL) hmi::CurrentInterface->NewClient(clientId, clientsUDP[clientId].remoteIP, 0);
			#endif
			// Fleischmann/Roco Android app starts with Power on !
			TrackManager::setMainPower(POWERMODE::ON);
      CommandDistributor::broadcastPower();
			Z21Throttle* pThrottle = getOrAddThrottle(clientId); 
			if (pThrottle != NULL) {
		  	pThrottle->lastHeartBeatDate = millis();
//      	pThrottle->notifyTrPw(clientId, 1);
			}
		}

		clientId = readUdpPacket();

		if (clientId >= 0) {
			if (clientsUDP[clientId].ok()) {        
				Z21Throttle* pThrottle = getOrAddThrottle(clientId); 
			  pThrottle->lastHeartBeatDate = millis();

				if (pThrottle != NULL) {
					while(clientsUDP[clientId].pudpBuffer->GetCount() > 2)
						pThrottle->parse();
				}
			}
		}
	}
}

/** Print the list of assigned locomotives. */
void Z21Throttle::printLocomotives(bool addTab) {
	if (!Z21Throttle::DIAGBASE)
		return;

	DIAG(F("[Z21]       Locomotives ------------------"));
	for (int loco = 0; loco < MAX_MY_LOCO; loco++)
			DIAG(F("[Z21] %s         %d : cab %d on throttle %c"), addTab ? "   ":"", loco, this->cabs[loco], this->clientid);
}

/** Print the list of assigned locomotives. */
void printClientsUDP() {
	if (!Z21Throttle::DIAGBASE) return;

	DIAG(F("[Z21]       UDP Clients ------------------"));
	for (int clientId = 0; clientId < clientsUDP.size(); clientId++)
		if (clientsUDP[clientId].ok())
			DIAG(F("[Z21]          %d %s: %s:%d"), clientId, clientsUDP[clientId].connected?"Connected":"Not connected", clientsUDP[clientId].remoteIP.toString().c_str(), clientsUDP[clientId].remotePort);
		else
			DIAG(F("[Z21]          %d unused"), clientId);
}

/** Print the list of assigned locomotives. */
void Z21Throttle::printThrottles(bool inPrintLocomotives) {
	if (!Z21Throttle::DIAGBASE)	return;

	DIAG(F("[Z21]       Throttles ---------------"));
	for (Z21Throttle* wt = firstThrottle; wt != NULL; wt = wt->nextThrottle) {
		if (wt->clientid == -1)
			DIAG(F("[Z21]          unused"));
		else {
			DIAG(F("[Z21]          %d : %d.%d.%d.%d:%d"), wt->clientid, 
			clientsUDP[wt->clientid].remoteIP[0], 
			clientsUDP[wt->clientid].remoteIP[1], 
			clientsUDP[wt->clientid].remoteIP[2], 
			clientsUDP[wt->clientid].remoteIP[3], 
			clientsUDP[wt->clientid].remotePort);

			if (inPrintLocomotives)
				wt->printLocomotives(true);
		}
	}
}

Z21Throttle* Z21Throttle::getOrAddThrottle(int clientId) {
	for (Z21Throttle* wt = firstThrottle; wt != NULL ; wt = wt->nextThrottle)  {
		if (wt->clientid == clientId)
			return wt; 
	}

	Z21Throttle *p = new Z21Throttle(clientId);
	printThrottles(false);
	return p;
}

void Z21Throttle::forget( byte clientId) {
	for (Z21Throttle* wt=firstThrottle; wt!=NULL ; wt=wt->nextThrottle)  
    	if (wt->clientid==clientId) {
			delete wt;
			break; 
		}
}

bool Z21Throttle::isThrottleInUse(int cab) {
	for (Z21Throttle* wt=firstThrottle; wt!=NULL ; wt=wt->nextThrottle)  
		if (wt->isCabInUse(cab)) return true;

	return false;
}

// One instance of Z21Throttle per connected client, so we know what the locos are 
 
Z21Throttle::Z21Throttle(int inClientId) {
	this->nextThrottle = firstThrottle;
	this->firstThrottle = this;
	this->clientid = inClientId;
	this->turnoutListHash = -1;  // make sure turnout list is sent once
	this->exRailSent = false;
	this->lastHeartBeatDate = 0;
	for (int loco=0;loco<MAX_MY_LOCO; loco++)
		this->cabs[loco]=0;
}

Z21Throttle::~Z21Throttle() {
	DIAG_Z21(F("[Z21] Deleting Throttle client UDP %d"),this->clientid);
	if (firstThrottle== this) {
		firstThrottle=this->nextThrottle;
		return;
	}

	for (Z21Throttle* wt=firstThrottle; wt!=NULL ; wt=wt->nextThrottle) {
		if (wt->nextThrottle==this) {
			wt->nextThrottle=this->nextThrottle;
			return;  
		}
	}
}

void Z21Throttle::write(int inClientID, byte* inpData, int inLengthData) {
	size_t size = 0;

//	if (this->dontReply)
//		return;
	
	NetworkClientUDP::client.beginPacket(clientsUDP[inClientID].remoteIP, clientsUDP[inClientID].remotePort);
	size = NetworkClientUDP::client.write(inpData, inLengthData);
	NetworkClientUDP::client.endPacket();

	DIAG_Z21DATA(F("[Z21] %d: %s SENT 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x"), inClientID,
                          size == 0 ? "BINARY NOT" :"",
                          (inLengthData > 0)?inpData[0]:0,
                          (inLengthData > 1)?inpData[1]:0,
                          (inLengthData > 2)?inpData[2]:0,
                          (inLengthData > 3)?inpData[3]:0,
                          (inLengthData > 4)?inpData[4]:0,
                          (inLengthData > 5)?inpData[5]:0,
                          (inLengthData > 6)?inpData[6]:0,
                          (inLengthData > 7)?inpData[7]:0,
                          (inLengthData > 8)?inpData[8]:0,
                          (inLengthData > 9)?inpData[9]:0);
}

// sizes : [       2        ][       2        ][inLengthData]
// bytes : [length1, length2][Header1, Header2][Data........]
bool Z21Throttle::notify(int inClientID, unsigned int inHeader, byte* inpData, unsigned int inLengthData, bool inXorInData) {
	int realLength = (inLengthData + 4 + (inXorInData == false ? 1 : 0));

	Z21Throttle::commBuffer[0] = realLength % 256;
	Z21Throttle::commBuffer[1] = realLength / 256;
	Z21Throttle::commBuffer[2] = inHeader % 256;
	Z21Throttle::commBuffer[3] = inHeader / 256;
	memcpy(Z21Throttle::commBuffer + 4, inpData, inLengthData);

	if (!inXorInData) {    // if xor byte not included in data, compute and write it !
		byte xxor = 0;

		for (unsigned int i = 0; i < inLengthData; i++)
			xxor ^= inpData[i];

		Z21Throttle::commBuffer[inLengthData+4] = xxor;
	}

	write(inClientID, Z21Throttle::commBuffer, realLength);

	return true;
}

// sizes : [       2        ][       2        ][   1   ][inLengthData]
// bytes : [length1, length2][Header1, Header2][XHeader][Data........]
bool Z21Throttle::notify(int inClientID, unsigned int inHeader, unsigned int inXHeader, byte* inpData, unsigned int inLengthData, bool inXorInData) {
	int realLength = (inLengthData + 5 + (inXorInData == false ? 1 : 0));

	Z21Throttle::commBuffer[0] = realLength % 256;
	Z21Throttle::commBuffer[1] = realLength / 256;
	Z21Throttle::commBuffer[2] = inHeader % 256;
	Z21Throttle::commBuffer[3] = inHeader / 256;
	Z21Throttle::commBuffer[4] = inXHeader;
	memcpy(Z21Throttle::commBuffer + 5, inpData, inLengthData);

	if (!inXorInData) {    // if xor byte not included in data, compute and write it !
		byte xxor = inXHeader;

		for (unsigned int i = 0; i < inLengthData; i++)
			xxor ^= inpData[i];

		Z21Throttle::commBuffer[inLengthData + 5] = xxor;
	}

	write(inClientID, Z21Throttle::commBuffer, realLength);

	return true;
}

// sizes : [       2        ][       2        ][   1   ][ 1 ][inLengthData]
// bytes : [length1, length2][Header1, Header2][XHeader][DB0][Data........]
bool Z21Throttle::notify(int inClientID, unsigned int inHeader, unsigned int inXHeader, byte inDB0, byte* inpData, unsigned int inLengthData, bool inXorInData) {
	int realLength = (inLengthData + 6 + (inXorInData == false ? 1 : 0));

	Z21Throttle::commBuffer[0] = realLength % 256;
	Z21Throttle::commBuffer[1] = realLength / 256;
	Z21Throttle::commBuffer[2] = inHeader % 256;
	Z21Throttle::commBuffer[3] = inHeader / 256;
	Z21Throttle::commBuffer[4] = inXHeader;
	Z21Throttle::commBuffer[5] = inDB0;
	memcpy(Z21Throttle::commBuffer + 6, inpData, inLengthData);

	if (!inXorInData) {   // if xor byte not included in data, compute and write it !
		byte xxor = inXHeader^inDB0;

		for (unsigned int i = 0; i < inLengthData; i++)
			xxor ^= inpData[i];

		Z21Throttle::commBuffer[inLengthData + 6] = xxor;
	}

	write(inClientID, Z21Throttle::commBuffer, realLength);

	return true;
}

void Z21Throttle::notifyStatus() {
	Z21Throttle::replyBuffer[0] = 0; // main current 1
	Z21Throttle::replyBuffer[1] = 0; // main current 2
	Z21Throttle::replyBuffer[2] = 0; // prog current 1
	Z21Throttle::replyBuffer[3] = 0; // prog current 2
	Z21Throttle::replyBuffer[4] = 0; // filtered main current 1
	Z21Throttle::replyBuffer[5] = 0; // filtered main current 2
	Z21Throttle::replyBuffer[6] = 0; // Temperature 1
	Z21Throttle::replyBuffer[7] = 0; // Temperature 2
	Z21Throttle::replyBuffer[8] = 5; // Supply voltage 1
	Z21Throttle::replyBuffer[9] = 0; // supply voltage 2
	Z21Throttle::replyBuffer[10] = 16; // VCC voltage 1 
	Z21Throttle::replyBuffer[11] = 0; // VCC voltage 2
	Z21Throttle::replyBuffer[12] = 0b00000000;	// CentralState 
	Z21Throttle::replyBuffer[13] = 0b00000000; // CentralStateEx
	Z21Throttle::replyBuffer[14] = 0;
	Z21Throttle::replyBuffer[15] = 0;
	notify(this->clientid, HEADER_LAN_SYSTEMSTATE, Z21Throttle::replyBuffer, 16, true);
}

int Z21Throttle::getOrAddLoco(int cab) {
	int loco = 0;

	// try to find the cab
	for (; loco < MAX_MY_LOCO; loco++) {
		if (this->cabs[loco] == cab) 
			return loco;
	}

	int foundLoco = -1;

	// not found ! Add the cab.
	for (loco = 0; loco < MAX_MY_LOCO; loco++) {
		if (this->cabs[loco] == 0) {
			this->cabs[loco] = cab;
			foundLoco = loco;
			break;
		}
	}

	// if no place available...
	if (foundLoco == -1) {
		printLocomotives();
		return -1;
	}

	return loco;
}

int Z21Throttle::countLocos() {
	int count = 0;

	for (int loco = 0; loco < MAX_MY_LOCO; loco++) {
		if (this->cabs[loco] != 0)
			count++;
	}

	return count;
}

bool Z21Throttle::isCabInUse(int cab) { 
	// try to find the cab
	for (int loco = 0; loco < MAX_MY_LOCO; loco++) {
		if (this->cabs[loco] == cab) 
			return true;
	}

	return false;
}

// Simplified version of DCC::getFn()
int getFn( int reg, int16_t functionNumber) {
  unsigned long funcmask = (1UL<<functionNumber);
  return  (DCC::speedTable[reg].functions & funcmask)? 1 : 0;
}

void Z21Throttle::notifyLocoInfo(int inClientID, byte inMSB, byte inLSB) {
	int locoAddress = ((inMSB & 0x3F) << 8) + inLSB;

	Z21Throttle::replyBuffer[0] = inMSB;	// loco address msb
	Z21Throttle::replyBuffer[1] = inLSB; // loco address lsb
	Z21Throttle::replyBuffer[2] = B00000100; // 0000CKKK	 C = already controlled    KKK = speed steps 000:14, 010:28, 100:128
	Z21Throttle::replyBuffer[3] = DCC::getThrottleSpeed(locoAddress); // RVVVVVVV  R = forward    VVVVVVV = speed
	if (DCC::getThrottleDirection(locoAddress)) bitSet(Z21Throttle::replyBuffer[3], 7);

  int reg = DCC::lookupSpeedTable(locoAddress);

	Z21Throttle::replyBuffer[4] = B00000000; // 0DSLFGHJ  D = double traction    S = Smartsearch   L = F0   F = F4   G = F3   H = F2   J = F1
	if (getFn(reg, 0)) bitSet(Z21Throttle::replyBuffer[4], 4);
	if (getFn(reg, 1)) bitSet(Z21Throttle::replyBuffer[4], 0);
	if (getFn(reg, 2)) bitSet(Z21Throttle::replyBuffer[4], 1);
	if (getFn(reg, 3)) bitSet(Z21Throttle::replyBuffer[4], 2);
	if (getFn(reg, 4)) bitSet(Z21Throttle::replyBuffer[4], 3);

	Z21Throttle::replyBuffer[5] = B00000000;	// function F5 to F12    F5 is bit0
	if (getFn(reg, 5)) bitSet(Z21Throttle::replyBuffer[5], 0);
	if (getFn(reg, 6)) bitSet(Z21Throttle::replyBuffer[5], 1);
	if (getFn(reg, 7)) bitSet(Z21Throttle::replyBuffer[5], 2);
	if (getFn(reg, 8)) bitSet(Z21Throttle::replyBuffer[5], 3);
	if (getFn(reg, 9)) bitSet(Z21Throttle::replyBuffer[5], 4);
	if (getFn(reg, 10)) bitSet(Z21Throttle::replyBuffer[5],5);
	if (getFn(reg, 11)) bitSet(Z21Throttle::replyBuffer[5],6);
	if (getFn(reg, 12)) bitSet(Z21Throttle::replyBuffer[5],7);

	Z21Throttle::replyBuffer[6] = B00000000;	// function F13 to F20   F13 is bit0
	if (getFn(reg, 13)) bitSet(Z21Throttle::replyBuffer[6], 0);
	if (getFn(reg, 14)) bitSet(Z21Throttle::replyBuffer[6], 1);
	if (getFn(reg, 15)) bitSet(Z21Throttle::replyBuffer[6], 2);
	if (getFn(reg, 16)) bitSet(Z21Throttle::replyBuffer[6], 3);
	if (getFn(reg, 17)) bitSet(Z21Throttle::replyBuffer[6], 4);
	if (getFn(reg, 18)) bitSet(Z21Throttle::replyBuffer[6], 5);
	if (getFn(reg, 19)) bitSet(Z21Throttle::replyBuffer[6], 6);
	if (getFn(reg, 20)) bitSet(Z21Throttle::replyBuffer[6], 7);

	Z21Throttle::replyBuffer[7] = B00000000;	// function F21 to F28   F21 is bit0
	if (getFn(reg, 21)) bitSet(Z21Throttle::replyBuffer[7], 0);
	if (getFn(reg, 22)) bitSet(Z21Throttle::replyBuffer[7], 1);
	if (getFn(reg, 23)) bitSet(Z21Throttle::replyBuffer[7], 2);
	if (getFn(reg, 24)) bitSet(Z21Throttle::replyBuffer[7], 3);
	if (getFn(reg, 25)) bitSet(Z21Throttle::replyBuffer[7], 4);
	if (getFn(reg, 26)) bitSet(Z21Throttle::replyBuffer[7], 5);
	if (getFn(reg, 27)) bitSet(Z21Throttle::replyBuffer[7], 6);
	if (getFn(reg, 28)) bitSet(Z21Throttle::replyBuffer[7], 7);
  
	notify(inClientID, HEADER_LAN_XPRESS_NET, LAN_X_HEADER_LOCO_INFO, Z21Throttle::replyBuffer, 8, false);
  
	DIAG_Z21BROADCAST(F("[Z21] %d: Notify loco %d"), inClientID, locoAddress);

  if (TrackManager::getMainPower()==POWERMODE::OFF) 
    notifyTrPw(inClientID, 0);
  else 
    notifyTrPw(inClientID, 1);
}

void Z21Throttle::notifyTurnoutInfo(int inClientID, byte inMSB, byte inLSB) {
	Z21Throttle::replyBuffer[0] = inMSB;	// turnout address msb
	Z21Throttle::replyBuffer[1] = inLSB; // turnout address lsb
	int id = (inMSB << 8) + inLSB;

	if (Turnout::isClosed(id))
		Z21Throttle::replyBuffer[2] = B00000001; // 000000ZZ	 ZZ : 00 not switched   01 pos1  10 pos2  11 invalid
	else
		Z21Throttle::replyBuffer[2] = B00000010; // 000000ZZ	 ZZ : 00 not switched   01 pos1  10 pos2  11 invalid
	notify(inClientID, HEADER_LAN_XPRESS_NET, LAN_X_HEADER_TURNOUT_INFO, Z21Throttle::replyBuffer, 3, true);

	DIAG_Z21BROADCAST(F("[Z21] %d: Notify turnout %d"), inClientID, id);
}

void Z21Throttle::notifyLocoMode(int inClientID, byte inMSB, byte inLSB) {
	Z21Throttle::replyBuffer[0] = inMSB;	// loco address msb
	Z21Throttle::replyBuffer[1] = inLSB; // loco address lsb
	Z21Throttle::replyBuffer[2] = B00000000; // 00000000	DCC   00000001 MM
	notify(inClientID, HEADER_LAN_GET_LOCOMODE, Z21Throttle::replyBuffer, 3, true);
}

void Z21Throttle::notifyTrPw(int inClientID, byte TrPw) {
  Z21Throttle::replyBuffer[0] = 0x22; 
  Z21Throttle::replyBuffer[1] = 0x2-(TrPw*2) ; // power off or on
  notify(inClientID, HEADER_LAN_XPRESS_NET, LAN_X_STATUS_CHANGED, Z21Throttle::replyBuffer, 2, false);

	DIAG_Z21BROADCAST(F("[Z21] %d: Notify power %d "), inClientID, TrPw);
}

void Z21Throttle::notifyFirmwareVersion() {
	Z21Throttle::replyBuffer[0] = 0x01;	// Version major in BCD
	Z21Throttle::replyBuffer[1] = 0x23;	// Version minor in BCD
	notify(this->clientid, HEADER_LAN_XPRESS_NET, LAN_X_HEADER_FIRMWARE_VERSION, 0x0A, Z21Throttle::replyBuffer, 2, false);
}

void Z21Throttle::notifySerialNumber() {
	Z21Throttle::replyBuffer[0] = 0x02;	// Serial number little endian
	Z21Throttle::replyBuffer[1] = 0x05;	// 
	notify(this->clientid, HEADER_LAN_GET_SERIAL_NUMBER, Z21Throttle::replyBuffer, 2, false);
}

void Z21Throttle::notifyHWInfo() {
	Z21Throttle::replyBuffer[0] = 0x00;	// Hardware type in BCD on int32
	Z21Throttle::replyBuffer[1] = 0x02;	// Hardware type in BCD on int32
	Z21Throttle::replyBuffer[2] = 0x00;	// Hardware type in BCD on int32
	Z21Throttle::replyBuffer[3] = 0x00;	// Hardware type in BCD on int32
	Z21Throttle::replyBuffer[4] = 0x23;	// Firmware version in BCD on int32
	Z21Throttle::replyBuffer[5] = 0x01;	// Firmware version in BCD on int32
	Z21Throttle::replyBuffer[6] = 0x00;	// Firmware version in BCD on int32
	Z21Throttle::replyBuffer[7] = 0x00;	// Firmware version in BCD on int32
	notify(this->clientid, HEADER_LAN_GET_HWINFO, Z21Throttle::replyBuffer, 8, true);
}

void Z21Throttle::notifyCvNACK(int inCvAddress) {
	Z21Throttle::replyBuffer[0] = highByte(inCvAddress); // cv address msb
	Z21Throttle::replyBuffer[1] = lowByte(inCvAddress); // cv address lsb
	notify(this->clientid, HEADER_LAN_XPRESS_NET, LAN_X_HEADER_CV_NACK, LAN_X_DB0_CV_NACK, Z21Throttle::replyBuffer, 0, false);
}

void Z21Throttle::notifyCvRead(int inCvAddress, int inValue) {
	Z21Throttle::replyBuffer[0] = highByte(inCvAddress); // cv address msb
	Z21Throttle::replyBuffer[1] = lowByte(inCvAddress); // cv address lsb
	Z21Throttle::replyBuffer[2] = inValue; // cv value
	notify(this->clientid, HEADER_LAN_XPRESS_NET, LAN_X_HEADER_CV_RESULT, 0x14, Z21Throttle::replyBuffer, 3, false);
}

void Z21Throttle::setSpeed(byte inNbSteps, byte inDB1, byte inDB2, byte inDB3) {
	bool isForward = bitRead(inDB3, 7);
	byte speed = inDB3;
	bitClear(speed, 7);

	DIAG_Z21(F("[Z21] %d: speed %d"), clientid, speed * (isForward ? 1:-1));

	int locoAddress = ((inDB1 & 0x3F) << 8) + inDB2;

	if (getOrAddLoco(locoAddress) == -1)
		DIAG_Z21(F("[Z21] %d: loco %d cannot be added"), clientid, locoAddress);

	DCC::setThrottle(locoAddress, speed, isForward);
}

//
// TODO Pass through a text message to avoid multi thread locks...
//

void Z21Throttle::setFunction(byte inDB1, byte inDB2, byte inDB3) {
	// inDB3 :  TTNN NNNN		TT:00 off, TT:01 on; TT:10 toggle   NNNNNN function number

	byte action = bitRead(inDB3, 6) + 2 * bitRead(inDB3, 7);
	byte function = inDB3;
	bitClear(function, 6);
	bitClear(function, 7);
	bool activeFlag = action == 0b01;

	DIAG_Z21(F("[Z21] %d: function %d %s"), clientid, function, activeFlag?"ON":"OFF");

	int locoAddress = ((inDB1 & 0x3F) << 8) + inDB2;
	if (getOrAddLoco(locoAddress) == -1)
		DIAG_Z21(F("[Z21] %d: loco %d cannot be added"), clientid, locoAddress);

	if (action == 0b10)	{	// toggle
		bool isActivated = DCC::getFn(locoAddress, function);
		activeFlag = !isActivated;
	}

	DCC::setFn(locoAddress, function, activeFlag);
}

//
// TODO Pass through a text message to avoid multi thread locks...
//

void Z21CvValueCallback(int16_t inValue)
{
	Z21Throttle::cvValue = inValue;

	if (inValue == -1)
		Z21Throttle::readWriteThrottle->notifyCvNACK(Z21Throttle::cvAddress);
	else
		Z21Throttle::readWriteThrottle->notifyCvRead(Z21Throttle::cvAddress, inValue);

	Z21Throttle::readWriteThrottle = NULL;
}

void Z21Throttle::cvReadProg(byte inDB1, byte inDB2) {
	if (Z21Throttle::readWriteThrottle != NULL)
		return;

	int cvAddress = ((inDB1 & 0x3F) << 8) + inDB2 + 1;

	DIAG_Z21(F("[Z21] %d: cvRead Prog %d"), clientid, cvAddress);

	Z21Throttle::readWriteThrottle = this;
	Z21Throttle::cvAddress = cvAddress - 1;

	void (*ptr)(int16_t) = &Z21CvValueCallback;
	DCC::readCV(cvAddress, ptr);
}

// Working as cvReadProg for the moment...
void Z21Throttle::cvReadMain(byte inDB1, byte inDB2) {
	if (Z21Throttle::readWriteThrottle != NULL)
		return;

	int cvAddress = ((inDB1 & 0x3F) << 8) + inDB2 + 1;

	DIAG_Z21(F("[Z21] %d: cvRead Main cv %d"), clientid, cvAddress);

	Z21Throttle::readWriteThrottle = this;
	Z21Throttle::cvAddress = cvAddress - 1;

	void (*ptr)(int16_t) = &Z21CvValueCallback;
	DCC::readCV(cvAddress, ptr);
}

//
// TODO Pass through a text message to avoid multi thread locks...
//

void Z21Throttle::cvWriteProg(byte inDB1, byte inDB2, byte inDB3) {
	if (Z21Throttle::readWriteThrottle != NULL)
		return;

	int cvAddress = ((inDB1 & 0x3F) << 8) + inDB2 + 1;

	DIAG_Z21(F("[Z21] %d: cvWrite Prog cv %d value %d"), clientid, cvAddress, inDB3);

	Z21Throttle::readWriteThrottle = this;
	Z21Throttle::cvAddress = cvAddress - 1;

	void (*ptr)(int16_t) = &Z21CvValueCallback;
	DCC::writeCVByte(cvAddress, inDB3, ptr);
}

// Working as cvReadProg for the moment...
void Z21Throttle::cvReadPom(byte inDB1, byte inDB2, byte inDB3, byte inDB4) {
	if (Z21Throttle::readWriteThrottle != NULL)
		return;

	int locoAddress = ((inDB1 & 0x3F) << 8) + inDB2;
	int cvAddress = ((inDB3 & B00000011) << 8) + inDB4 + 1;

	DIAG_Z21(F("[Z21] %d: cvRead Pom Loco %d cv %d"), clientid, locoAddress, cvAddress);

	Z21Throttle::readWriteThrottle = this;
	Z21Throttle::cvAddress = cvAddress - 1;

	void (*ptr)(int16_t) = &Z21CvValueCallback;
	DCC::readCV(cvAddress, ptr);
}

bool Z21Throttle::parse() {
	bool done = false;
	byte DB[100];
	CircularBuffer* pBuffer = clientsUDP[this->clientid].pudpBuffer;

	if (pBuffer == NULL)
		return false;
	if (pBuffer->isEmpty())
		return false;

	int lengthData = pBuffer->GetInt16() - 4;	// length of the data = total length - length of length (!) - length of header
	int header = pBuffer->GetInt16();
	byte Xheader = 0;
	byte DB0 = 0;
	int nbLocos = this->countLocos();

	if (lengthData > 0)	{
		pBuffer->GetBytes(DB, lengthData);
		DIAG_Z21DATA(F("[Z21] %d: <- len:%d  header:0x%02x  : 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x"),
					this->clientid, lengthData + 4, header,
					(lengthData > 0)?DB[0]:0,
					(lengthData > 1)?DB[1]:0,
					(lengthData > 2)?DB[2]:0,
					(lengthData > 3)?DB[3]:0,
					(lengthData > 4)?DB[4]:0,
					(lengthData > 5)?DB[5]:0,
					(lengthData > 6)?DB[6]:0,
					(lengthData > 7)?DB[7]:0,
					(lengthData > 8)?DB[8]:0,
					(lengthData > 9)?DB[9]:0);
	}
	else {
		if (header == 0) {
			DIAG_Z21DATA(F("[Z21] %d: <- len_raw:%d"), this->clientid, lengthData + 4);
			return true;
		}
	}

	switch (header)	{
		case HEADER_LAN_XPRESS_NET:
			Xheader = DB[0];
			switch (Xheader) {
				case LAN_X_HEADER_GENERAL:
					DB0 = DB[1];
					switch (DB0) {
					case LAN_X_DB0_GET_VERSION:
						DIAG_Z21VERBOSE(F("[Z21] %d: GET_VERSION"), this->clientid);
						break;
					case LAN_X_DB0_GET_STATUS:
						DIAG_Z21VERBOSE(F("[Z21] %d: GET_STATUS  "), this->clientid);
						notifyStatus();
						done = true;
						break;
					case LAN_X_DB0_SET_TRACK_POWER_OFF:
						DIAG_Z21(F("[Z21] %d: POWER_OFF"), this->clientid);
						TrackManager::setMainPower(POWERMODE::OFF);
		        CommandDistributor::broadcastPower();
						done = true;
						break;
					case LAN_X_DB0_SET_TRACK_POWER_ON:
						DIAG_Z21(F("[Z21] %d: POWER_ON"), this->clientid);
						TrackManager::setMainPower(POWERMODE::ON);
		        CommandDistributor::broadcastPower();
						done = true;
						break;
					}
					break;
				case LAN_X_HEADER_SET_STOP:
					DIAG_Z21(F("[Z21] %d: EMERGENCY_STOP"), this->clientid);
					//Emergency Stop  (speed code 1)
					// setThrottle will cause a broadcast so notification will be sent
					LOOPLOCOS() { 
						if (this->cabs[loco] != 0)
							DCC::setThrottle(this->cabs[loco], 1, DCC::getThrottleDirection(this->cabs[loco])); 
					}
					done = true;
					break;
				case LAN_X_HEADER_SET_LOCO:
					DB0 = DB[1];
					switch (DB0) {
						case LAN_X_DB0_LOCO_DCC14:
							DIAG_Z21VERBOSE(F("[Z21] %d: LOCO DCC 14 SPEED"), this->clientid);
							setSpeed(14, DB[2], DB[3], DB[4]);
							done = true;
							break;
						case LAN_X_DB0_LOCO_DCC28:
							DIAG_Z21VERBOSE(F("[Z21] %d: LOCO DCC 28 SPEED"), this->clientid);
							setSpeed(28, DB[2], DB[3], DB[4]);
							done = true;
							break;
						case LAN_X_DB0_LOCO_DCC128:
							DIAG_Z21VERBOSE(F("[Z21] %d: LOCO DCC 128 SPEED"), this->clientid);
							setSpeed(128, DB[2], DB[3], DB[4]);
							done = true;
							break;
						case LAN_X_DB0_SET_LOCO_FUNCTION:
							DIAG_Z21VERBOSE(F("[Z21] %d: LOCO DCC FUNCTION"), this->clientid);
							setFunction(DB[2], DB[3], DB[4]);							
							if (Z21Throttle::DIAGBASE) {
								// Debug capacity to print data...
								byte function = DB[4];
								bitClear(function, 6);
								bitClear(function, 7);
								if (function == 12) { // why not ?
									printClientsUDP();
									printThrottles(true);
								}
							}
							done = true;
							break;
					}
					break;
				case LAN_X_HEADER_GET_LOCO_INFO:
					DIAG_Z21VERBOSE(F("[Z21] %d: LOCO %d INFO: "), this->clientid, ((DB[2] & 0x3F) << 8) + DB[3]);
					notifyLocoInfo(this->clientid, DB[2], DB[3]);
					done = true;
					break;

				case LAN_X_HEADER_GET_TURNOUT_INFO:
				{
					int id = (DB[1] << 8) + DB[2];
					DIAG_Z21VERBOSE(F("[Z21] %d: TURNOUT %d INFO"), this->clientid, id);
					if (!Turnout::exists(id)) {
						// If turnout does not exist, create it
						int addr = (id / 4) + 1;
						int subaddr = id % 4;
						DCCTurnout::create(id,addr,subaddr);
						DIAG_Z21VERBOSE(F("[Z21] %d: TURNOUT %d created"), this->clientid, id);
						//Turnout::printAll(&USB_SERIAL);
					}

					notifyTurnoutInfo(this->clientid, DB[1], DB[2]);
					done = true;
				}
					break;

				case LAN_X_HEADER_GET_FIRMWARE_VERSION:
					DIAG_Z21VERBOSE(F("[Z21] %d: FIRMWARE VERSION  "), this->clientid);
					notifyFirmwareVersion();
					done = true;
					break;
				case LAN_X_HEADER_CV_READ:
					if (TrackManager::getProgDriver() != NULL) {
						DIAG_Z21VERBOSE(F("[Z21] %d: CV READ PROG "), this->clientid);
						// DB0 should be 0x11
						cvReadProg(DB[2], DB[3]);
					}
					else {
						//
						// TODO Dont work today...
						//

						// If no prog track, read on the main track !
						DIAG_Z21VERBOSE(F("[Z21] %d: CV READ MAIN "), this->clientid);
						// DB0 should be 0x11
						cvReadMain(DB[2], DB[3]);
					}
					done = true;
					break;
				case LAN_X_HEADER_CV_POM:
					DIAG_Z21VERBOSE(F("[Z21] %d: CV READ POM"), this->clientid);
					// DB0 should be 0x11
					cvReadPom(DB[2], DB[3], DB[4], DB[5]);
					done = true;
					break;
				case LAN_X_HEADER_CV_WRITE:
					DIAG_Z21VERBOSE(F("[Z21] %d: CV WRITE "), this->clientid);
					cvWriteProg(DB[2], DB[3], DB[4]);
					done = true;
					break;
				case LAN_X_HEADER_SET_TURNOUT:
					{
					int id = (DB[1] << 8) + DB[2];
					bool activate = DB[3] & 0b00001000;
					bool IsOutput1 = DB[3] & 0b00000001;
					if (activate) {
						DIAG_Z21VERBOSE(F("[Z21] %d: TURNOUT %d %s output %s"), this->clientid, id + 1, activate?"active":"inactive", IsOutput1?"1":"2");
					}
					else {
						DIAG_Z21VERBOSE(F("[Z21] %d: TURNOUT %d %s"), this->clientid, id + 1, activate?"active":"inactive");
					}

					if (!Turnout::exists(id)) {
						// If turnout does not exist, create it
						int addr = (id / 4) + 1;
						int subaddr = id % 4;
						DCCTurnout::create(id,addr,subaddr);
						DIAG_Z21VERBOSE(F("[Z21] %d: TURNOUT %d created"), this->clientid, id);
						Turnout::printAll(&USB_SERIAL);
					}

					if (activate) {
				  	Turnout::setClosed(id, !IsOutput1);
					}

					/*
					switch (DB[2] & 0b0001000) {
						// T and C according to RCN-213 where 0 is Stop, Red, Thrown, Diverging.
					case 'T': 
						Turnout::setClosed(id,false);
						break;
					case 'C': 
						Turnout::setClosed(id,true);
						break;
					case '2': 
						Turnout::setClosed(id,!Turnout::isClosed(id));
						break;
					default :
						Turnout::setClosed(id,true);
						break;
					}*/
					}
					done = true;
					break;
				case 0x22:
					break;
			}
			break;

		case HEADER_LAN_SET_BROADCASTFLAGS:
			this->broadcastFlags = CircularBuffer::GetInt32(DB, 0);
			DIAG_Z21DATA(F("[Z21] %d: BROADCAST FLAGS : %s %s %s %s %s %s %s %s %s %s %s"), this->clientid,
							(this->broadcastFlags & BROADCAST_BASE)	? "BASE " : "" ,
							(this->broadcastFlags & BROADCAST_RBUS)	? "RBUS " : "" ,
							(this->broadcastFlags & BROADCAST_RAILCOM)	? "RAILCOM " : "" ,
							(this->broadcastFlags & BROADCAST_SYSTEM)	? "SYSTEM " : "" ,
							(this->broadcastFlags & BROADCAST_BASE_LOCOINFO)	? "LOCOINFO " : "" ,
							(this->broadcastFlags & BROADCAST_LOCONET)	? "LOCONET " : "" ,
							(this->broadcastFlags & BROADCAST_LOCONET_LOCO)	? "LOCONET_LOCO " : "" ,
							(this->broadcastFlags & BROADCAST_LOCONET_SWITCH)	? "LOCONET_SWITCH " : "" ,
							(this->broadcastFlags & BROADCAST_LOCONET_DETECTOR)	? "LOCONET_DETECTOR " : "" ,
							(this->broadcastFlags & BROADCAST_RAILCOM_AUTO)	? "RAILCOM_AUTO " : "" ,
							(this->broadcastFlags & BROADCAST_CAN)	? "CAN" : "" );
			done = true;
			break;
		case HEADER_LAN_GET_LOCOMODE:
			DIAG_Z21VERBOSE(F("[Z21] %d: GET LOCOMODE"), this->clientid);
			notifyLocoMode(this->clientid, DB[0], DB[1]);	// big endian here, but resend the same as received, so no problem.
			done = true;
			break;

		case HEADER_LAN_SET_LOCOMODE:
			DIAG_Z21VERBOSE(F("[Z21] %d: SET LOCOMODE"), this->clientid);
			done = true;
			break;
		case HEADER_LAN_GET_HWINFO:
			DIAG_Z21VERBOSE(F("[Z21] %d: GET HWINFO"), this->clientid);
			notifyHWInfo();	// big endian here, but resend the same as received, so no problem.
			done = true;
			break;
		case HEADER_LAN_LOGOFF:
			DIAG_Z21VERBOSE(F("[Z21] %d: LOGOFF"), this->clientid);
			this->clientid = -1;
			done = true;
			break;
		case HEADER_LAN_SYSTEMSTATE_GETDATA:
			DIAG_Z21VERBOSE(F("[Z21] %d: SYSTEMSTATE GETDATA"), this->clientid);
			notifyStatus();	// big endian here, but resend the same as received, so no problem.
			done = true;
			break;
		case HEADER_LAN_GET_SERIAL_NUMBER:
			DIAG_Z21VERBOSE(F("[Z21] %d: GET_SERIAL_NUMBER"), this->clientid);
			notifyFirmwareVersion();
			done = true;
			break;
		case HEADER_LAN_GET_BROADCASTFLAGS:
		case HEADER_LAN_GET_TURNOUTMODE:
		case HEADER_LAN_SET_TURNOUTMODE:
		case HEADER_LAN_RMBUS_DATACHANGED:
		case HEADER_LAN_RMBUS_GETDATA:
		case HEADER_LAN_RMBUS_PROGRAMMODULE:
		case HEADER_LAN_RAILCOM_DATACHANGED:
		case HEADER_LAN_RAILCOM_GETDATA:
		case HEADER_LAN_LOCONET_DISPATCH_ADDR:
		case HEADER_LAN_LOCONET_DETECTOR:
			break;
	}

	if (!done) {
    	DIAG_Z21(F("[Z21] %d: not treated :  header:%x   Xheader:%x   DB0:%x"), this->clientid, header, Xheader, DB0);
	}
	else {
		int newNbLocos = this->countLocos();
		if (nbLocos != newNbLocos)
			printLocomotives();
	}
	return true;
}
#endif