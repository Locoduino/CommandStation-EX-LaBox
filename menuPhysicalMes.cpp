/*
 * La Box Project
 * menuPhysicalMes Classes 
 *
 * @Author : Thierry Paris
 * @Organization : Locoduino.org
 */
#include "defines.h"
#include "DCC.h"

#ifdef USE_HMI
#include <WiFi.h>
#include "esp_wifi.h"
#include "WifiESP32.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

#include "hmiIcons.h"
#include "menuobject.h"
#include "menuPhysicalMes.h"
#include "hmi.h"
#include "version.h"
#include "version_labox.h"
#include "EXComm.h"

bool updatedUI;
char messageUI[30];
unsigned long lastUpdateTime;
const unsigned long HMIPhysicalMes_UpdateIntervalMs = 1000;

extern hmi boxHMI;

/*!
    @brief  menuPhysicalMes Constructor
    @param  Screen, an Adafruit_SSD1306 object to permit to display our menu
    @param p, a parent object. All menu are chained between parent and sons
    @param title, a menu has a title, a char[HMI_MenueMessageSize].
    @param val, a integer which define the king of menu or the returned value after selection
    @return None (void).
    @note
*/
menuPhysicalMes::menuPhysicalMes(Adafruit_SSD1306* screen, menuObject* p, const char* title, int value): menuObject(screen, p, title, value)
{


}

void menuPhysicalMes::start()
{
  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::start.. Begin"); 

  updatedUI = false;
	
	this->voltage = 0.0;
	this->current = 0.0;
	
	boxHMI.stopTimeOutRefresh = true;
	boxHMI.stopTrainsViewRefresh = true;
	lastUpdateTime = millis();

  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::start.. End"); 
}

/*!
    @brief  eventUp, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuPhysicalMes::eventUp()
{
  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::eventUp.. Begin"); 

  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::eventUp.. End");   
}

/*!
    @brief  eventDown, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuPhysicalMes::eventDown()
{
  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::eventDown.. Begin"); 

  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::eventDown.. End");   
}

/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuPhysicalMes::eventSelect()
{
  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::eventSelect.. Begin"); 
	boxHMI.stopTimeOutRefresh = false;
	boxHMI.stopTrainsViewRefresh = false;
	lastUpdateTime = 0;

  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::eventSelect.. End");  
  return MENUEXIT;
}

/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void menuPhysicalMes::begin()
{
  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::begin.. Begin"); 


  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::begin.. End"); 
}

/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuPhysicalMes::update()
{
  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::update.. Begin"); 

	if (updatedUI)
	{
		if (millis() - lastUpdateTime < HMIPhysicalMes_UpdateIntervalMs)
			return;
	}

	this->voltage = boxHMI.voltage;
	this->current = boxHMI.current;

	lastUpdateTime = millis();

	updatedUI = true;

  display->clearDisplay();

  sprintf(messageUI,"U: %.0fV", this->voltage);
  display->setTextColor(WHITE);
  display->setTextSize(2);
  display->setCursor(5, 5);
  display->println(messageUI);

	if (current < 1000)
		sprintf(messageUI, "I: %.0fmA", this->current);
	else
		sprintf(messageUI, "I: %.1fA", this->current/1000.f);
  display->setCursor(5, 32);
  display->println(messageUI);

  display->setTextSize(1);
	display->setCursor(0, 56);
 	display->println(TXT_MenuInfoButtons);

	display->display();
	
  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::update.. End"); 
}
/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuPhysicalMes::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::resetMenu.. Begin"); 
  menuObject::resetMenu();



  _HMIDEBUG_FCT_PRINTLN("menuPhysicalMes::resetMenu.. End"); 
  
}
#endif