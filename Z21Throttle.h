/*
 *  © 2023 Thierry Paris / Locoduino
 *  All rights reserved.
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
#ifndef Z21Throttle_h
#define Z21Throttle_h

#include <WiFiClient.h>
#include <WiFi.h>

#include "defines.h"
#include "CircularBuffer.hpp"
#include "EXComm.h"

#ifdef ENABLE_Z21

#define UDPBYTE_SIZE	1024
#define UDP_BUFFERSIZE	2048
#define Z21_UDPPORT		21105

// Broadcast flags

#define BROADCAST_BASE				0x00000001
#define BROADCAST_RBUS				0x00000002
#define BROADCAST_RAILCOM			0x00000004
#define BROADCAST_SYSTEM			0x00000100
#define BROADCAST_BASE_LOCOINFO		0x00010000
#define BROADCAST_LOCONET			0x01000000
#define BROADCAST_LOCONET_LOCO		0x02000000
#define BROADCAST_LOCONET_SWITCH	0x04000000
#define BROADCAST_LOCONET_DETECTOR	0x08000000
#define BROADCAST_RAILCOM_AUTO		0x00040000
#define BROADCAST_CAN				0x00080000

class NetworkClientUDP {
	public:
	NetworkClientUDP() {
		this->pudpBuffer = new CircularBuffer(UDP_BUFFERSIZE);
		this->pudpBuffer->begin(true);
	};
	bool ok() {
		return (inUse);
	};

	bool inUse = true;
	bool connected = false;
	CircularBuffer *pudpBuffer = NULL;
	IPAddress remoteIP;
	int remotePort;

	static WiFiUDP client;
};

class Z21Throttle {
	public:  
		static void loop();
		static Z21Throttle* getOrAddThrottle(int clientId); 
		static void markForBroadcast(int cab);
		static void forget(byte clientId);
		static Z21Throttle* getFirstThrottle() { return firstThrottle; }
		Z21Throttle* getNextThrottle() { return this->nextThrottle; }

		void notifyCvNACK(int inCvAddress);
		void notifyCvRead(int inCvAddress, int inValue);
		
    static void notifyTrPw(int inClientID, byte TrPw);
		static void notifyLocoInfo(int inClientID, byte inMSB, byte inLSB);
		static void notifyTurnoutInfo(int inClientID, byte inMSB, byte inLSB);
		static void notifyLocoMode(int inClientID, byte inMSB, byte inLSB);

		bool parse();
		bool isBroadcastFlag() { return (this->broadcastFlags & BROADCAST_BASE) != 0; }
		bool isCabInUse(int cab);
		int getClientId() { return this->clientid; }

		static Z21Throttle *readWriteThrottle;	// NULL if no throttle is reading or writing a CV...
		static int cvAddress;
		static int cvValue;

	  static bool DIAGBASE;
	  static bool DIAGBROADCAST;
	  static bool DIAGVERBOSE;
	  static bool DIAGDATA;

private: 
		Z21Throttle(int clientId);
		~Z21Throttle();

		static const int MAX_MY_LOCO=10;      // maximum number of locos assigned to a single client
		static Z21Throttle* firstThrottle;
		static byte commBuffer[100];
		static byte replyBuffer[20];
		//static char LorS(int cab); 
		static bool isThrottleInUse(int cab);
		static void setSendTurnoutList();
		Z21Throttle* nextThrottle;

		unsigned long lastHeartBeatDate;
		int clientid;

		int cabs[MAX_MY_LOCO];

		bool exRailSent; // valid connection established
		int turnoutListHash;  // used to check for changes to turnout list
		int32_t broadcastFlags;

		int getOrAddLoco(int cab);
		int countLocos();
		void rebuildLocoMask();
		void printLocomotives(bool addTab = false);
		static void printThrottles(bool printLocomotives);

		// sizes : [       2        ][       2        ][inLengthData]
		// bytes : [length1, length2][Header1, Header2][Data........]
		static bool notify(int inClientID, unsigned int inHeader, byte* inpData, unsigned int inLengthData, bool inXorInData);

		// sizes : [       2        ][       2        ][   1   ][inLengthData]
		// bytes : [length1, length2][Header1, Header2][XHeader][Data........]
		static bool notify(int inClientID, unsigned int inHeader, unsigned int inXHeader, byte* inpData, unsigned int inLengthData, bool inXorInData);

		// sizes : [       2        ][       2        ][   1   ][ 1 ][inLengthData]
		// bytes : [length1, length2][Header1, Header2][XHeader][DB0][Data........]
		static bool notify(int inClientID, unsigned int inHeader, unsigned int inXHeader, byte inDB0, byte* inpData, unsigned int inLengthData, bool inXorInData);

		static void write(int inClientID, byte* inpData, int inLengthData);

		void notifyStatus();
		void notifyFirmwareVersion();
		void notifySerialNumber();
		void notifyHWInfo();

		void setSpeed(byte inNbSteps, byte inDB1, byte inDB2, byte inDB3);
		void setFunction(byte inDB1, byte inDB2, byte inDB3);
		void cvReadProg(byte inDB1, byte inDB2);
		void cvWriteProg(byte inDB1, byte inDB2, byte inDB3);
		void cvReadMain(byte inDB1, byte inDB2);
		void cvReadPom(byte inDB1, byte inDB2, byte inDB3, byte inDB4);
};

class Z21EXCommItem : public EXCommItem {
	public:  
		Z21EXCommItem(int inPort = Z21_UDPPORT) : EXCommItem("Z21") { 
			UDPport = inPort; 
			this->AlwaysLoop = true;
		}

		static int UDPport;

		bool begin() override;
		bool loop() override { Z21Throttle::loop(); return true; }

  	void broadcastLoco(int16_t cab) override;
  	void broadcastSensor(int16_t id, bool value) override;
  	void broadcastTurnout(int16_t id, bool isClosed) override;
  	void broadcastPower() override;

		void getInfos(String *pMess1, String *pMess2, String *pMess3, byte maxSize) override;
};

#define Z21_TIMEOUT		20000		// if no activity during this delay, disconnect the throttle...
#define Z21_MAXIMAL_UDP_MSG_SIZE	100		// All messages longer than this size will be ignored.

#define HEADER_LAN_GET_SERIAL_NUMBER 0x10
#define HEADER_LAN_LOGOFF 0x30
#define HEADER_LAN_XPRESS_NET 0x40
#define HEADER_LAN_SET_BROADCASTFLAGS 0x50
#define HEADER_LAN_GET_BROADCASTFLAGS 0x51
#define HEADER_LAN_SYSTEMSTATE_GETDATA 0x85    //0x141 0x21 0x24 0x05
#define HEADER_LAN_GET_HWINFO 0x1A
#define HEADER_LAN_GET_LOCOMODE 0x60
#define HEADER_LAN_SET_LOCOMODE 0x61
#define HEADER_LAN_GET_TURNOUTMODE 0x70
#define HEADER_LAN_SET_TURNOUTMODE 0x71
#define HEADER_LAN_RMBUS_DATACHANGED 0x80
#define HEADER_LAN_RMBUS_GETDATA 0x81
#define HEADER_LAN_RMBUS_PROGRAMMODULE 0x82
#define HEADER_LAN_RAILCOM_DATACHANGED 0x88
#define HEADER_LAN_RAILCOM_GETDATA 0x89
#define HEADER_LAN_LOCONET_DISPATCH_ADDR 0xA3
#define HEADER_LAN_LOCONET_DETECTOR 0xA4

#define LAN_GET_CONFIG 0x12

#define LAN_X_HEADER_GENERAL 0x21
#define LAN_X_HEADER_SET_STOP 0x80
#define LAN_X_HEADER_GET_FIRMWARE_VERSION 0xF1  //0x141 0x21 0x21 0x00 
#define LAN_X_HEADER_GET_LOCO_INFO 0xE3
#define LAN_X_HEADER_SET_LOCO 0xE4
#define LAN_X_HEADER_GET_TURNOUT_INFO 0x43
#define LAN_X_HEADER_SET_TURNOUT 0x53
#define LAN_X_HEADER_CV_READ 0x23
#define LAN_X_HEADER_CV_WRITE 0x24
#define LAN_X_HEADER_CV_POM 0xE6

#define LAN_X_STATUS_CHANGED 0x062

#define LAN_X_DB0_GET_VERSION 0x21
#define LAN_X_DB0_GET_STATUS 0x24
#define LAN_X_DB0_SET_TRACK_POWER_OFF 0x80
#define LAN_X_DB0_SET_TRACK_POWER_ON 0x81
#define LAN_X_DB0_LOCO_DCC14	0x10
#define LAN_X_DB0_LOCO_DCC28	0x12
#define LAN_X_DB0_LOCO_DCC128	0x13
#define LAN_X_DB0_SET_LOCO_FUNCTION 0xF8
#define LAN_X_DB0_CV_POM_WRITE 0x30
#define LAN_X_DB0_CV_POM_ACCESSORY_WRITE 0x31

#define LAN_X_OPTION_CV_POM_WRITE_BYTE 0xEC
#define LAN_X_OPTION_CV_POM_WRITE_BIT 0xE8
#define LAN_X_OPTION_CV_POM_READ_BYTE 0xE4

// Replies to the controllers
#define HEADER_LAN_SYSTEMSTATE	0x84

#define LAN_X_HEADER_LOCO_INFO	0xEF
#define LAN_X_HEADER_TURNOUT_INFO 0x43
#define LAN_X_HEADER_FIRMWARE_VERSION 0xF3
#define LAN_X_HEADER_CV_NACK 0x61
#define LAN_X_HEADER_CV_RESULT 0x64

#define LAN_X_DB0_CV_NACK_SC 0x12
#define LAN_X_DB0_CV_NACK 0x13

#endif
#endif
