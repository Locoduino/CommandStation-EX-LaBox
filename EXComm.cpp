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
#include "hmi.h"
#include "LaboxModes.h"

#ifdef ENABLE_EXCOMM

#include "EXComm.h"
#include "EXCommItems.h"

byte EXCommItem::lastItem = 0;
byte EXCommItem::nextCycleItem = MAX_COMMITEMS;
EXCommItem* EXCommItem::commItems[MAX_COMMITEMS];

void EXCommItem::Setup() {
	lastItem = 0;
	SetupPrivate(LABOX_EXCOMMS);
	nextCycleItem = MAX_COMMITEMS;
}

// The setup call is done this way so that the tracks can be in a list 
// from the config... the tracks default to NULL in the declaration                 
void EXCommItem::SetupPrivate(
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

bool EXCommItem::addItem(byte t, EXCommItem* apComm) {
	if (t < MAX_COMMITEMS) {
		if (apComm) {
			if ((LaboxModes::progMode && apComm->ProgTrackEnabled) ||
			   (!LaboxModes::progMode && apComm->MainTrackEnabled)) {
			 	commItems[t] = apComm;
   			lastItem = t;
				return true;
  		}
		}

		commItems[t] = NULL;
	}

	return false;
}

void EXCommItem::beginItems() {
	//printItems();
	for (int i = 0; i <= lastItem; i++) {
    if (commItems[i] != NULL) {
			//DIAG(F("%d %s"), i, commItems[i]->name);
	    commItems[i]->beginItem();
		}
	}
}

void EXCommItem::loop() {
	// Execute 'AlwaysLoop' items
	for (int i = 0; i <= lastItem; i++) {
    if (commItems[i] != NULL && commItems[i]->AlwaysLoop) {
	    commItems[i]->loopItem();
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
	  commItems[nextCycleItem]->loopItem();
}

void EXCommItem::sendPowerItems(bool iSOn) {
	for (int i = 0; i <= lastItem; i++) {
    if (commItems[i] != NULL)
	    commItems[i]->sendPower(iSOn);
	}
}

void EXCommItem::sendThrottleItems(uint16_t cab, uint8_t tSpeed, bool tDirection) {
	for (int i = 0; i <= lastItem; i++) {
    if (commItems[i] != NULL)
	    commItems[i]->sendThrottle(cab, tSpeed, tDirection);
	}
}

void EXCommItem::sendFunctionItems(int cab, int16_t functionNumber, bool on) {
	for (int i = 0; i <= lastItem; i++) {
    if (commItems[i] != NULL)
	    commItems[i]->sendFunction(cab, functionNumber, on);
	}
}

void EXCommItem::sendEmergencyItems() {
	for (int i = 0; i <= lastItem; i++) {
    if (commItems[i] != NULL)
	    commItems[i]->sendEmergency();
	}
}

void EXCommItem::printItems() {
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


#endif