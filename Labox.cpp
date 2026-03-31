/*
 *  © 2025 Thierry Paris
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
#include "DCC.h"
#include "TrackManager.h"
#include "MotorDriver.h"
#include "LaboxModes.h"
#include "hmi.h"
#include "EXComm.h"
#include "menuobject.h"
#include "Railcom.h"
#include "Labox.h"

#ifdef LABOX_CV_ADDRESS

bool Labox::DIAGLABOXCVS = true;

#define DIAG_LCVS				if (Labox::DIAGLABOXCVS) DIAG

bool Labox::ChangeCVs(int address, int value)
{
	DIAG_LCVS(F("Labox : ChangeCVs()"));
	bool memo = LaboxModes::progMode;

	switch(address - LABOX_CV_ADDRESS) {
		case LABOX_CV_WIFI_ENABLE:
			#ifdef ENABLE_WIFI
			DIAG_LCVS(F("Labox : CV WIFI ENABLE %d"), value);
			if (value == 0) {
				WifiESP::teardown();
			}
			else {
				WifiESP::setup(WIFI_SSID, WIFI_PASSWORD, WIFI_HOSTNAME, IP_PORT, WIFI_CHANNEL, WIFI_FORCE_AP);
			}
			#endif
			return true;
			
		case LABOX_CV_RAILCOM_ENABLE:
			DIAG_LCVS(F("Labox : CV RAILCOM ENABLE %d"), value);
			#ifdef ENABLE_RAILCOM
			LaboxModes::progMode = false; // to avoid Railcom disable in prog mode
			if (value == 0) {
				RailcomEnd();
			}
			else {
				RailcomBegin();
			}
			LaboxModes::progMode = memo;
			pauseRailcom = true;	// We are in prog mode, pause Railcom activity
			#endif
			return true;

		default:
			DIAG_LCVS(F("Labox : CV Address %d not handled"), address);
			break;
	}

	DIAG_LCVS(F("Labox : ChangeCVs() done"));
	return false;
}

bool Labox::ParseLB(Print *stream, int16_t params, int16_t p[])
{
	bool ret = false;
	DIAG_LCVS(F("Labox : ParseLB()"));

	if (params < 2) {
		StringFormatter::send(stream, F("<LB ERROR NO PARAM>\n"));
	}
	else {
		switch (p[0]) {
			case 'W':				
				#ifdef ENABLE_WIFI
				if (params > 1) {
					if (p[1] == 1) {
						WifiESP::setup(WIFI_SSID, WIFI_PASSWORD, WIFI_HOSTNAME, IP_PORT, WIFI_CHANNEL, WIFI_FORCE_AP);
					}
					else if (p[1] == 0) {
						WifiESP::teardown();
					}
					else {
						StringFormatter::send(stream, F("<LB ERROR INVALID PARAM>\n"));
						return true;
					}
					break;
				}
				else {
					StringFormatter::send(stream, F("<LB W %d>\n"), WifiESP::isUp() ? 1 : 0);
				}
				#else
				StringFormatter::send(stream, F("<LB WIFI DISABLED>\n"));
				#endif
				
				return true;

			case 'R':				
				#ifdef ENABLE_RAILCOM
				if (params > 1) {
					if (p[1] == 1) {
						RailcomBegin();
					}
					else if (p[1] == 0) {
						RailcomEnd();
					}
					else {
						StringFormatter::send(stream, F("<LB ERROR INVALID PARAM>\n"));
						return true;
					}
					break;
				}
				else {
					StringFormatter::send(stream, F("<LB R %d>\n"), isRailcomEnabled() ? 1 : 0);
				}
				#else
				StringFormatter::send(stream, F("<LB RAILCOM DISABLED>\n"));
				#endif
				return true;

			default:
				StringFormatter::send(stream, F("<LB ERROR UNKNOWN PARAM>\n"));
				break;
		}
	}

	DIAG_LCVS(F("Labox : ParseLB() done"));
	return ret;
}
#endif
