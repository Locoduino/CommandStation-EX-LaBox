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
#include "WifiESP32.h"
#include <EEPROM.h>
#include "DCC.h"
#include "TrackManager.h"
#include "MotorDriver.h"
#include "LaboxModes.h"
#include "hmi.h"
#include "EXComm.h"
#include "menuobject.h"
#include "menumanagement.h"

#include "soc/rtc_wdt.h"
#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"

bool LaboxModes::progMode = false;
ProgType LaboxModes::progModeType = ProgType::MAIN;
ProgBehavior LaboxModes::progBehavior = ProgBehaviorNone;
bool LaboxModes::silentBootMode = false;
int LaboxModes::EEPROMModeProgAddress = 511;
POWERMODE LaboxModes::memoPowerBeforeJoining = POWERMODE::OFF; // Power before joining the main track

bool LaboxModes::DIAGLABOXMODES=false;

#define DIAG_LMODES				if (LaboxModes::DIAGLABOXMODES) DIAG

#ifdef USE_HMI
extern enumHMIState _HMIState;
extern hmi boxHMI;
#endif

void LaboxModes::begin()
{
	DIAG_LMODES(F("LaboxModes : begin()"));

	progBehavior = ProgBehaviorNormal;

	if (progMode == true || TrackManager::getProgDriver() == NULL) {
		progBehavior = ProgBehaviorReboot;
		DIAG_LMODES(F("progBehavior = Reboot"));
	}
	else {
		std::vector<MotorDriver *> mains = TrackManager::getMainDrivers();
		if (mains.size() == 0) {
			progBehavior = ProgBehaviorJoining;
			DIAG_LMODES(F("progBehavior = Joining"));
		}
		else {
			DIAG_LMODES(F("progBehavior = Normal"));
		}
	}

	memoPowerBeforeJoining = POWERMODE::OFF;

	SetNextMode(MAIN);

  if (progMode) {
    // must be done after all other setups.
    StartProgMode(true);
  }
	else {
		StartMainMode(true);
	}

	DIAG_LMODES(F("LaboxModes : begin() done"));
}

void LaboxModes::SetNextMode(ProgType inType)
{
	if (progBehavior == ProgBehaviorReboot) {
		DIAG_LMODES(F("LaboxModes : SetNextMode %c"), (char) inType);
    // Reset to Main mode for next reboot.
    EEPROM.writeByte(EEPROMModeProgAddress, (char) MAIN);
    EEPROM.commit();
	}
}

void LaboxModes::GetStartingMode()
{
	EEPROM.begin(512);
	byte mode = EEPROM.read(EEPROMModeProgAddress);

	DIAG_LMODES(F("LaboxModes : GetCurrentMode %c"), (char) mode);

	// If the EEPROM is not initialized, we assume MAIN mode
	if (mode == 0xFF || mode == 0)
		mode = (char) ProgType::MAIN;

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
void LaboxModes::StartProgMode(bool inForce)
{
#ifdef USE_HMI
  DIAG_LMODES("LaboxModes::setProgMode().. Begin");

	if (LaboxModes::progMode == true && !inForce)
		return;

	LaboxModes::progMode = true;

	if (progBehavior == ProgBehaviorJoining) {
		memoPowerBeforeJoining = TrackManager::getPower(1);
		TrackManager::setJoin(false);
	}

	if (progBehavior == ProgBehaviorReboot) {
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
	}

  DIAG_LMODES("LaboxModes::setProgMode().. End");
	#endif
}

/*!
    @brief  Set main Mode 
    @param  None
    @return None (void).
    @note
*/
void LaboxModes::StartMainMode(bool inForce)
{
#ifdef USE_HMI
  DIAG_LMODES("LaboxModes::setMainMode().. Begin");

	if (!inForce && LaboxModes::progMode == false)
		return;
	
  LaboxModes::progMode = false;

	if (progBehavior == ProgBehaviorJoining) {
		TrackManager::setJoin(true);
		TrackManager::setPower(memoPowerBeforeJoining);
	}

  DIAG_LMODES("LaboxModes::setMainMode().. End");
	#endif
}

void LaboxModes::ChangeMode(bool inProgMode, ProgType inType)
{
	DIAG_LMODES(F("LaboxModes : ChangeMode %s"), inProgMode?"Prog":"Main");

	if (LaboxModes::progMode == inProgMode)
			return;

	if (progBehavior == ProgBehaviorReboot) {
		DIAG("1");
		SetNextMode(inType);
		#ifdef ENABLE_RAILCOM
		if (inType == ProgType::MAIN) {
		DIAG("2");
			DONOTRESTART();
			while(true) {
				delay(100);
			}
			return;
		}
		#endif
		DIAG("3");
		LaboxModes::Restart(inType);
		return;
	}

	if (inProgMode) {
		LaboxModes::StartProgMode();
	}
	else {
		LaboxModes::StartMainMode();
	}
}

void LaboxModes::Restart(ProgType inType)
{
	DIAG_LMODES(F("LaboxModes : Restart"));

	std::vector<MotorDriver *> mains = TrackManager::getMainDrivers();
	if (mains.size() > 0 && TrackManager::getProgDriver() != NULL)
		return;

	if (EEPROM.readByte(EEPROMModeProgAddress) != (char) inType) {
		EEPROM.writeByte(EEPROMModeProgAddress, (char) inType);
		EEPROM.commit();
		EEPROM.end();
	}

	/*DCC::end();
	WifiESP::end();
	EXComm::end();
	USB_SERIAL.end();

	delay(6000);*/

	ESP.restart();
}

void centerIdentMessage(const char *Text, char *inBuffer, int inSize)
{
	int len = strlen(Text);
	memset(inBuffer, ' ', inSize);
	memcpy(inBuffer + (inSize/2 - len/2), Text, len);
}

/*!
    @brief  Show a screen to tell the user must restart the LaBox himself.
    @param  None
    @return None (void).
    @note
*/
void LaboxModes::DONOTRESTART()
{
  DIAG_LMODES(F("LaboxModes::DONOTRESTART.. Begin"));
	char buffer[21];

	Adafruit_SSD1306* display = (Adafruit_SSD1306*) &boxHMI;

  display->clearDisplay();

  display->setTextSize(1);
  display->setCursor(5, 6);
  display->println(TXT_IDENT_LOGO);

  display->setTextColor(WHITE);
  display->setTextSize(1);
  display->setCursor(5, 20);

 	centerIdentMessage(TXT_DONOTRESTART, buffer, 20);
	display->println(buffer);

  display->setTextSize(1);
  display->setCursor(5, 55);

  display->display();   

  DIAG_LMODES(F("LaboxModes::DONOTRESTART.. End"));
}

