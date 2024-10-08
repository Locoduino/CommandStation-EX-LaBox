/*
 *  © 2024 Thierry Paris
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
#include "DCCEXParser.h"
#include "hmi.h"

#ifdef ENABLE_EXCOMM

#include "LaboxModes.h"
#include "EXComm.h"
#include "EXCommItems.h"

bool EXComm::DIAGBASE = false;

#define DIAG_EXCOMM		if (EXComm::DIAGBASE) DIAG

byte EXComm::lastItem = 0;
byte EXComm::nextCycleItem = MAX_COMMITEMS;
EXCommItem* EXComm::commItems[MAX_COMMITEMS];
String *EXComm::pInfos = NULL;
int EXComm::infosCount = 0;

void EXComm::Setup() {
	lastItem = 0;
	SetupPrivate(LABOX_EXCOMMS);
	nextCycleItem = MAX_COMMITEMS;
}

// The setup call is done this way so that the tracks can be in a list 
// from the config... the tracks default to NULL in the declaration                 
void EXComm::SetupPrivate(
        EXCommItem * comm0, EXCommItem * comm1, EXCommItem * comm2,
        EXCommItem * comm3, EXCommItem * comm4, EXCommItem * comm5,
        EXCommItem * comm6, EXCommItem * comm7 ) {       
    addItem(0, comm0);
    addItem(1, comm1);
    addItem(2, comm2);
    addItem(3, comm3);
    addItem(4, comm4);
    addItem(5, comm5);
    addItem(6, comm6);
    addItem(7, comm7);
}

bool EXComm::addItem(byte t, EXCommItem* apComm) {
	if (t < MAX_COMMITEMS) {
		if (apComm) {
			if ((LaboxModes::progMode && apComm->ProgTrackEnabled) ||
			   (!LaboxModes::progMode && apComm->MainTrackEnabled)) {
			 	commItems[t] = apComm;
   			lastItem = t;
				return true;
  		}
			else {
				DIAG(F("[%s] item not applicable in this mode."), apComm->getName());
			}
		}

		commItems[t] = NULL;
	}

	return false;
}

void EXComm::begin() {
	//printItems();
	for (int i = 0; i <= lastItem; i++) {
    if (commItems[i] != NULL) {
			//DIAG(F("%d %s"), i, commItems[i]->name);
	    commItems[i]->begin();
		}
	}
}

void EXComm::loop() {
	// Execute 'AlwaysLoop' items
	for (int i = 0; i <= lastItem; i++) {
    if (commItems[i] != NULL && commItems[i]->AlwaysLoop) {
	    commItems[i]->loop();
		}
	}

	// Execute this cycle item
  nextCycleItem++;
  if (nextCycleItem > lastItem) 
		nextCycleItem = 0;
  if (commItems[nextCycleItem] == NULL)
		return;
	// If not already done in the previous for !
  if (!commItems[nextCycleItem]->AlwaysLoop)
	  commItems[nextCycleItem]->loop();
}

void EXComm::broadcast(byte *com)
{
	DIAG_EXCOMM(F("[EXCOMM] broadcast : %s"), com);
	int16_t p[DCCEXParser::MAX_COMMAND_PARAMS];
	while (com[0] == '<' || com[0] == ' ')
			com++; // strip off any number of < or spaces
	byte opcode = com[0];
	byte params = DCCEXParser::splitValues(p, com, false);
	
	switch (opcode)
	{
	case 'l':   // LOCO <l CAB SPEED DIRECTION>
			DIAG_EXCOMM(F("[EXCOMM] broadcast loco"));
			if (params >= 0) {
				for (int i = 0; i <= lastItem; i++) {
					if (commItems[i] != NULL) {
						commItems[i]->broadcastLoco(p[0]);
					}
				}
			}
			return;

	case 'H': // TURNOUT <H ADDRESS ACTIVATE>
			DIAG_EXCOMM(F("[EXCOMM] broadcast turnout"));
			if (params >= 1) {
				for (int i = 0; i <= lastItem; i++) {
					if (commItems[i] != NULL) {
						commItems[i]->broadcastTurnout(p[0], p[1]);
					}
				}
			}
			return;
		
	case 'j': // CLOACKTIME <jC TIME RATE>
			DIAG_EXCOMM(F("[EXCOMM] broadcast clock time"));
			if (params >= 1) {
				for (int i = 0; i <= lastItem; i++) {
					if (commItems[i] != NULL) {
						commItems[i]->broadcastClockTime(p[0], p[1]);
					}
				}
			}
			return;
		
	case 'p': // POWER  <px name> where x is 0 or 1
			DIAG_EXCOMM(F("[EXCOMM] broadcast power"));
			for (int i = 0; i <= lastItem; i++) {
				if (commItems[i] != NULL) {
					commItems[i]->broadcastPower();
				}
			}
			return;

	case 'Q':	// SENSOR ON
			DIAG_EXCOMM(F("[EXCOMM] broadcast sensor ON"));
			if (params >= 0) {
				for (int i = 0; i <= lastItem; i++) {
					if (commItems[i] != NULL) {
						commItems[i]->broadcastSensor(p[0], 1);
					}
				}
			}
			return;

	case 'q':	// SENSOR OFF
			DIAG_EXCOMM(F("[EXCOMM] broadcast sensor OFF"));
			if (params >= 0) {
				for (int i = 0; i <= lastItem; i++) {
					if (commItems[i] != NULL) {
						commItems[i]->broadcastSensor(p[0], 0);
					}
				}
			}
			return;
	}
}

void EXComm::sendPower(bool iSOn) {
	for (int i = 0; i <= lastItem; i++) {
    if (commItems[i] != NULL)
	    commItems[i]->sendPower(iSOn);
	}
}

void EXComm::sendThrottle(uint16_t cab, uint8_t tSpeed, bool tDirection) {
	for (int i = 0; i <= lastItem; i++) {
    if (commItems[i] != NULL)
	    commItems[i]->sendThrottle(cab, tSpeed, tDirection);
	}
}

void EXComm::sendFunction(uint16_t cab, int16_t functionNumber, bool on) {
	for (int i = 0; i <= lastItem; i++) {
    if (commItems[i] != NULL)
	    commItems[i]->sendFunction(cab, functionNumber, on);
	}
}

void EXComm::sendEmergency() {
	for (int i = 0; i <= lastItem; i++) {
    if (commItems[i] != NULL)
	    commItems[i]->sendEmergency();
	}
}

void EXComm::print() {
	DIAG(F("EXComm items:"));
	int i = 0;
	for (; i < MAX_COMMITEMS;) {
    if (commItems[i] == NULL) {
			DIAG(F("  %d : NULL"), i);
		}
		else {
			DIAG(F("  %d : %s"), i, commItems[i]->name);
		}
		i++;
	}
	DIAG(F("Lastitem: %d"), lastItem);
}

int EXComm::getAllInfo(byte maxSize)
{
	if (pInfos == NULL)
	{
		pInfos = new String[(lastItem + 1) * 2];
		String mess1, mess2, mess3;
		for(int i = 0; i <= lastItem; i++)
		{
			if (commItems[i] == NULL)
				continue;

			mess1.clear();
			mess2.clear();
			mess3.clear();
			commItems[i]->getInfos(&mess1, &mess2, &mess3, maxSize);
			if (mess1.length() <= maxSize && mess1.length() > 0)
				pInfos[infosCount++] = mess1;
			else
			{
				if (mess1.length() > 0)
				{
					Serial.print("[");
					Serial.print(commItems[i]->getName());
					Serial.println("] info1 too long");
				}
			}

			if (mess2.length() <= maxSize && mess2.length() > 0)
				pInfos[infosCount++] = mess2;
			else
			{
				if (mess2.length() > 0)
				{
					Serial.print("[");
					Serial.print(commItems[i]->getName());
					Serial.println("] info2 too long");
				}
			}

			if (mess3.length() <= maxSize && mess3.length() > 0)
				pInfos[infosCount++] = mess3;
			else
			{
				if (mess3.length() > 0)
				{
					Serial.print("[");
					Serial.print(commItems[i]->getName());
					Serial.println("] info3 too long");
				}
			}
		}
	}

	return infosCount;
}
#endif