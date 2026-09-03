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
#include "LaboxRailcom.h"
#include "Labox.h"
#include "driver/mcpwm.h"
#include "esp_log.h"

#ifdef LABOX_CV_ADDRESS

bool Labox::DIAGLABOXCVS = false;
bool Labox::DIAGLABOXDC = false;

#ifdef USE_HMI
extern hmi boxHMI;
#endif

#define DIAG_LCVS				if (Labox::DIAGLABOXCVS) DIAG
#define DIAG_LDC				if (Labox::DIAGLABOXDC) DIAG

bool Labox::ChangeCVs(int address, int value)
{
	DIAG_LCVS(F("Labox : ChangeCVs()"));
#ifdef ENABLE_LABOX_RAILCOM
	bool memo = LaboxModes::progMode;
#endif

	switch(address - LABOX_CV_ADDRESS) {
		case LABOX_CV_WIFI_ENABLE:
			#ifdef WIFI_ON
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
			#ifdef ENABLE_LABOX_RAILCOM
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
				#ifdef WIFI_ON
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
				#ifdef ENABLE_LABOX_RAILCOM
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

// LaBox specific DC control functions

static mcpwm_generator_t mainGen;
static mcpwm_generator_t invGen;

bool LaboxDC::powered = false;
bool LaboxDC::forward = true;
bool LaboxDC::started = false;
int LaboxDC::speed = -1;

LocoSlot *LaboxDC::trainSlot = NULL;

void LaboxDC::begin()
{
	DIAG_LDC(F("LaboxDC : begin()"));
	pinMode(LABOX_DC_POWER, OUTPUT);
	pinMode(LABOX_DC_IN1, OUTPUT);
	pinMode(LABOX_DC_IN2, OUTPUT);

#ifdef USE_HMI
	if (hmi::CurrentInterface != NULL)
		boxHMI.nbTrainToView = 1;	
#endif

  int ret = ADCee::init(PIN_VOLTAGE_MES);
  if (ret < -1010) { // XXX give value a name later
    DIAG(F("ADCee::init error %d, disable current pin %d"), ret, PIN_VOLTAGE_MES);
  }

  trainSlot = LocoSlot::getSlot(LABOX_DC_CAB, true);

  if (trainSlot == NULL) { // XXX give value a name later
    DIAG(F("No loco slot found !"));
  }

	// MCPWM part
	  // GPIO routing.
  ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, LABOX_DC_IN1));
  ESP_ERROR_CHECK(mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, LABOX_DC_IN2));

  // Timer/channel setup.
  mcpwm_config_t cfg = {
  	.frequency = LABOX_DC_FREQUENCY,
  	.cmpr_a = 0,
   	.cmpr_b = 0,
   	.duty_mode = MCPWM_DUTY_MODE_0,
   	.counter_mode = MCPWM_UP_DOWN_COUNTER
	};
  ESP_ERROR_CHECK(mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &cfg));

	ESP_ERROR_CHECK(mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_A, 0.f));
	ESP_ERROR_CHECK(mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_B, 0.f));

	ESP_ERROR_CHECK(mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0));

	mainGen = MCPWM_GEN_A;
	invGen = MCPWM_GEN_B;

	started = true;
}

void LaboxDC::SetSpeed(int val)    
{
	if (val == speed) {
		return;
	}
	speed = val;

	DIAG_LDC(F("LaboxDC : SetSpeed(%d)"), val);

	if (val <= 1) { // emergency stop (1) or normal stop (0) !
		Stop();
		return;
	}

	if (val > 128) {
		val = 128;
	}

  int speed = map(val, 0,127, 0,100);

	ESP_ERROR_CHECK(mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, mainGen, speed));
	ESP_ERROR_CHECK(mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, invGen, 0.f));
}

bool LaboxDC::SetDirection(bool inForward)
{
	if (forward == inForward)
	{
		return false;
	}

	DIAG_LDC(F("LaboxDC : SetDirection(%s)"), inForward ? "1:forward" : "0:reverse");

	//SetSpeed(0); // stop before changing direction
	if (inForward)
	{
		mainGen = MCPWM_GEN_A;
		invGen = MCPWM_GEN_B;
	}
	else 
	{
		mainGen = MCPWM_GEN_B;
		invGen = MCPWM_GEN_A;
	}

	forward = inForward;
	return true;
}

void LaboxDC::Stop()
{
	DIAG_LDC(F("LaboxDC : Stop()"));
	ESP_ERROR_CHECK(mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, mainGen, 0.f));
	ESP_ERROR_CHECK(mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, invGen, 0.f));
}

bool LaboxDC::Power(bool on)
{
	DIAG_LDC(F("LaboxDC : Power(%s)"), on ? "ON" : "OFF");
	powered = on;
	if (on) {
		digitalWrite(LABOX_DC_POWER, HIGH);
		forward = false; // Default direction is reverse when powering on, to avoid loco moving when powering on if direction was forward before power off
		SetDirection(true); // Default direction is forward when powering on
	}
	else {
		digitalWrite(LABOX_DC_POWER, LOW);
	}

	#ifdef USE_HMI
	if (hmi::CurrentInterface != NULL) {
		if (on)
			hmi::CurrentInterface->DCCOn();
		else
			hmi::CurrentInterface->DCCOff();
	}
	#endif

	return true;
}

#endif
