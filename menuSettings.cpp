/*
 * La Box Project
 * menuSettings Classes 
 *
 * @Author : Thierry Paris
 * @Organization : Locoduino.org
 */
#include "defines.h"
#include "DCC.h"

#ifdef USE_HMI
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

#include "hmiIcons.h"
#include "menuobject.h"
#include "menuSettings.h"
#include "hmi.h"
#include "wifiEsp32.h"
#include "railcom.h"

bool updatedSettings;
int positionSettings;
//  line      text   
//--------------------
//	0					wifi
//	1					railcom
//	2
//	3	 
//	4	 
//	5	 				exit

#define SETTINGS_POSITION_EXIT 5
#define SETTINGS_POSITION_RAILCOM 1
#define SETTINGS_POSITION_WIFI 0

extern hmi boxHMI;

/*!
    @brief menuSettings Constructor
    @param Screen, an Adafruit_SSD1306 object to permit to display our menu
    @param p, a parent object. All menu are chained between parent and sons
    @param title, a menu has a title, a char[HMI_MenueMessageSize].
    @param value, a integer which define the kind of menu or the returned value after selection
    @return None (void).
    @note
*/
menuSettings::menuSettings(Adafruit_SSD1306* screen, menuObject* p, const char* title, int value): menuObject(screen, p, title, value)
{
}

void menuSettings::start()
{
  _HMIDEBUG_FCT_PRINTLN("menuSettings::start.. Begin"); 

  updatedSettings = false;
  positionSettings = SETTINGS_POSITION_EXIT;	// Quit line at the bottom of the screen

  _HMIDEBUG_FCT_PRINTLN("menuSettings::start.. End"); 
}

/*!
    @brief  eventUp, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuSettings::eventUp()
{
  _HMIDEBUG_FCT_PRINTLN("menuSettings::eventUp.. Begin"); 
  menuObject::eventUp();
	switch (positionSettings)
	{
		case SETTINGS_POSITION_EXIT:
			positionSettings = SETTINGS_POSITION_RAILCOM;
			break;
#ifdef ENABLE_RAILCOM
		case SETTINGS_POSITION_RAILCOM:
			positionSettings = SETTINGS_POSITION_WIFI;
			break;
#endif
	}

	updatedSettings = false;
  _HMIDEBUG_FCT_PRINTLN("menuSettings::eventUp.. End");   
}

/*!
    @brief  eventDown, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuSettings::eventDown()
{
  _HMIDEBUG_FCT_PRINTLN("menuSettings::eventDown.. Begin"); 
  menuObject::eventDown();
	switch (positionSettings)	{
#ifdef ENABLE_WIFI
		case SETTINGS_POSITION_WIFI:
			positionSettings = SETTINGS_POSITION_RAILCOM;
			break;
#endif
#ifdef ENABLE_RAILCOM
		case SETTINGS_POSITION_RAILCOM:
			positionSettings = SETTINGS_POSITION_EXIT;
			break;
#endif
	}

	updatedSettings = false;
  _HMIDEBUG_FCT_PRINTLN("menuSettings::eventDown.. End");   
}

extern enumEvent     _HMIEvent;

/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuSettings::eventSelect()
{
  _HMIDEBUG_FCT_PRINTLN("menuSettings::eventSelect.. Begin"); 
	switch (positionSettings)
	{
		case SETTINGS_POSITION_EXIT:
			DIAG("menuSettings::eventSelect.. Exit");  
			return MENUEXIT;

#ifdef ENABLE_RAILCOM
		case SETTINGS_POSITION_RAILCOM:
			DIAG("menuSettings::eventSelect.. Toggle Railcom");	
			if (isRailcomEnabled())
			{
				RailcomEnd();
			}
			else
			{
				RailcomBegin();
			}
			updatedSettings = false;
			return 0;	// Do not exit menu, just update display
#endif

#ifdef ENABLE_WIFI
		case SETTINGS_POSITION_WIFI:	
			DIAG("menuSettings::eventSelect.. Toggle Wifi");	
			if (WifiESP::isUp())
			{
				WifiESP::teardown();
			}
			else
			{
				WifiESP::setup(WIFI_SSID, WIFI_PASSWORD, WIFI_HOSTNAME, IP_PORT, WIFI_CHANNEL, WIFI_FORCE_AP);
			}
			updatedSettings = false;
			return 0;	// Do not exit menu, just update display
#endif
	}

  _HMIDEBUG_FCT_PRINTLN("menuSettings::eventSelect.. End");  
  return MENUEXIT;
}

/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void menuSettings::begin()
{
  _HMIDEBUG_FCT_PRINTLN("menuSettings::begin.. Begin"); 
  _HMIDEBUG_FCT_PRINTLN("menuSettings::begin.. End"); 
}

/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuSettings::update()
{
  _HMIDEBUG_FCT_PRINTLN("menuSettings::update.. Begin"); 

	if (updatedSettings)
	{
		return;
	}

	updatedSettings = true;

	display->clearDisplay();
  display->setTextSize(1);
	display->setTextColor(WHITE);

	char mess[10];
#ifdef ENABLE_WIFI
  display->setCursor(0, 0);
 	display->println(TXT_WIFI);
	sprintf(mess, positionSettings == SETTINGS_POSITION_WIFI ? ">%s<" : "%s", WifiESP::isUp() ? TXT_ON : TXT_OFF);
  display->setCursor(90+(positionSettings != SETTINGS_POSITION_WIFI) * 6, 0);
 	display->println(mess);
#endif

#ifdef ENABLE_RAILCOM
  display->setCursor(0, 9);
 	display->println(TXT_RAILCOM);
	sprintf(mess, positionSettings == SETTINGS_POSITION_RAILCOM ? ">%s<" : "%s", isRailcomEnabled() ? TXT_ON : TXT_OFF);
	display->setCursor(90+(positionSettings != SETTINGS_POSITION_RAILCOM) * 6, 9);
 	display->println(mess);
#endif

	/* exit option */
 	display->setCursor(70+(positionSettings != SETTINGS_POSITION_EXIT) * 6, 56);
	if (positionSettings == SETTINGS_POSITION_EXIT)
	{
		sprintf(mess, ">%s<", TXT_MenuAddrQuit);
	 	display->println(mess);
	}
	else
	{
	 	display->println(TXT_CHANGE);
	}

	display->display();	
  _HMIDEBUG_FCT_PRINTLN("menuSettings::update.. End"); 
}

/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuSettings::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("menuSettings::resetMenu.. Begin"); 
  menuObject::resetMenu();

  _HMIDEBUG_FCT_PRINTLN("menuSettings::resetMenu.. End");  
}
#endif