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
#include <EEPROM.h>
#include "DCC.h"
#include "LaboxModes.h"
#include "hmi.h"
#include "menuobject.h"
#include "menumanagement.h"

bool LaboxModes::progMode = false;
ProgType LaboxModes::progModeType = ProgType::MAIN;
bool LaboxModes::silentBootMode = false;
int LaboxModes::EEPROMModeProgAddress = 511;

bool LaboxModes::DIAGLABOXMODES=false;

#define DIAG_LMODES				if (LaboxModes::DIAGLABOXMODES) DIAG

#ifdef USE_HMI
extern enumHMIState _HMIState;
extern hmi boxHMI;
#endif

void LaboxModes::SetNextMode(ProgType inType)
{
		DIAG_LMODES(F("LaboxModes : SetNextMode %c"), (char) inType);
    // Reset to Main mode for next reboot.
    EEPROM.writeByte(EEPROMModeProgAddress, (char) MAIN);
    EEPROM.commit();
}

void LaboxModes::GetCurrentMode()
{
  EEPROM.begin(512);
  byte mode = EEPROM.read(EEPROMModeProgAddress);

	// Modes :	          EEPROM char		progMode		ProgModeType	SilentMode
	// ------------------+-------------+-----------+-------------+--------
	// MAIN			          M							false				MAIN					false
	// Reboot form prog		B							false				MAIN					true
	// CV1ADDR						P							true				CV1ADDR				true
	// CVREAD							Q							true				CVREAD				true
	// CVWRITE						R							true				CVWRITE				true
	// IDENTIFY						I							true				IDENTIFY			true

	progModeType = (ProgType) mode;

  progMode = true;
  silentBootMode = true;
  if (mode == (char) ProgType::MAIN) {
		silentBootMode = false;
    progMode = false;
	}

  if (mode == (char) ProgType::SILENTRETURNTOMAIN) {
		progModeType = ProgType::MAIN;
    progMode = false;
	}
}

/*!
    @brief  Set prog Mode 
    @param  None
    @return None (void).
    @note
*/
void LaboxModes::SetProgMode()
{
#ifdef USE_HMI
  _HMIDEBUG_FCT_PRINTLN("LaboxModes::setProgMode().. Begin");

  LaboxModes::progMode = true;
  _HMIState = StateParametersMenu;
	if (LaboxModes::progModeType == ProgType::CV1ADDR)
	{
  	boxHMI.menu->setMenu(boxHMI.menu->trainAddrRead);
  	boxHMI.menu->trainAddrRead->start();
	}

	if (LaboxModes::progModeType == ProgType::CVREAD)
	{
  	boxHMI.menu->setMenu(boxHMI.menu->trainCVRead);
  	boxHMI.menu->trainCVRead->start();
	}

	if (LaboxModes::progModeType == ProgType::CVWRITE)
	{
  	boxHMI.menu->setMenu(boxHMI.menu->trainCVWrite);
  	boxHMI.menu->trainCVWrite->start();
	}

	if (LaboxModes::progModeType == ProgType::IDENTIFY)
	{
  	boxHMI.menu->setMenu(boxHMI.menu->trainIdent);
  	boxHMI.menu->trainIdent->start();
	}

  _HMIDEBUG_FCT_PRINTLN("LaboxModes::setProgMode().. End");
	#endif
}

void LaboxModes::Restart(ProgType inType)
{
	DIAG_LMODES(F("LaboxModes : Restart"));
	if (EEPROM.readByte(EEPROMModeProgAddress) != (char) inType) {
		EEPROM.writeByte(EEPROMModeProgAddress, (char) inType);
		EEPROM.commit();
	}
	delay(500);
	ESP.restart();
}
