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
#include "dcc.h"
#include "LaboxModes.h"
#include <EEPROM.h>

bool LaboxModes::progMode = false;
ProgType LaboxModes::progModeType = ProgType::MAIN;
bool LaboxModes::silentBootMode = false;
int LaboxModes::EEPROMModeProgAddress = 511;

void LaboxModes::SetNextMode(ProgType inType)
{
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

void LaboxModes::Restart(ProgType inType)
{
	if (EEPROM.readByte(EEPROMModeProgAddress) != (char) inType) {
		EEPROM.writeByte(EEPROMModeProgAddress, (char) inType);
		EEPROM.commit();
	}
	delay(500);
	ESP.restart();
}
