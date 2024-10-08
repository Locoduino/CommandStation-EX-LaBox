/*
 * La Box Project
 * menuInformation Classes 
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */
#include "DCCEX.h"

#ifdef USE_HMI
#include <WiFi.h>
#include "esp_wifi.h"
#include "WifiESP32.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"

#include "icons.h"
#include "menuobject.h"
#include "menuinformation.h"
#include "hmi.h"
#include "version.h"
#include "version_labox.h"
#include "excomm.h"

bool updatedInfo;
char messageInfo[30];

/*!
    @brief  menuInformation Constructor
    @param  Screen, an Adafruit_SSD1306 object to permit to display our menu
    @param p, a parent object. All menu are chained between parent and sons
    @param title, a menu has a title, a char[HMI_MenueMessageSize].
    @param val, a integer which define the king of menu or the returned value after selection
    @return None (void).
    @note
*/
menuInformation::menuInformation(Adafruit_SSD1306* screen, menuObject* p, const char* title, int value): menuObject(screen, p, title, value)
{


}

void menuInformation::start()
{
  _HMIDEBUG_FCT_PRINTLN("menuInformation::start.. Begin"); 

  updatedInfo = false;
  memset(this->messages, 0, HMIInfo_MessageNumber * (HMIInfo_MessageMaxSize+1));
	this->firstMessage = 0;
	byte mess = 0;

	switch(this->value)
	{
		case MENUINFORMATION_WIFI:
			if (WiFi.status() != WL_CONNECTED) {
				strncpy(messages[mess++], TXT_noWifi, HMIInfo_MessageMaxSize);
				break;
			}

			if (WIFI_FORCE_AP == false)
			{
				strncpy(messages[mess++], "WIfi STA Mode", HMIInfo_MessageMaxSize);
				sprintf(messageInfo, "IP %s", WiFi.localIP().toString().c_str());
				strncpy(messages[mess++], messageInfo, HMIInfo_MessageMaxSize);
				sprintf(messageInfo, "Port %d", IP_PORT);
				strncpy(messages[mess++], messageInfo, HMIInfo_MessageMaxSize);
			}
			else
			{
				strncpy(messages[mess++], "WIfi AP Mode", HMIInfo_MessageMaxSize);
				sprintf(messageInfo, "IP %s", WiFi.softAPIP().toString().c_str());
				strncpy(messages[mess++], messageInfo, HMIInfo_MessageMaxSize);
				sprintf(messageInfo, "Port %d", IP_PORT);
				strncpy(messages[mess++], messageInfo, HMIInfo_MessageMaxSize);
			}
			break;

#ifdef ENABLE_EXCOMM
		case MENUINFORMATION_EXCOMM:
			int count = EXComm::getAllInfo(HMIInfo_MessageMaxSize); 
			for (int i = 0; i < count && i < HMIInfo_MessageNumber; i++)
			{
				strncpy(messages[i], EXComm::pInfos[i].c_str(), HMIInfo_MessageMaxSize);
			}
			break;
#endif			
	}

  _HMIDEBUG_FCT_PRINTLN("menuInformation::start.. End"); 
}

/*!
    @brief  eventUp, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuInformation::eventUp()
{
  _HMIDEBUG_FCT_PRINTLN("menuInformation::eventUp.. Begin"); 
  menuObject::eventUp();

	if (this->firstMessage > 0) 
	{
		this->firstMessage--;
		updatedInfo = false;
	}

  _HMIDEBUG_FCT_PRINTLN("menuInformation::eventUp.. End");   
}

/*!
    @brief  eventDown, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuInformation::eventDown()
{
  _HMIDEBUG_FCT_PRINTLN("menuInformation::eventDown.. Begin"); 
  menuObject::eventDown();

	if (this->firstMessage < HMIInfo_MessageNumber-1) 
	{
		this->firstMessage++;
		updatedInfo = false;
	}

  _HMIDEBUG_FCT_PRINTLN("menuInformation::eventDown.. End");   
}

/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuInformation::eventSelect()
{
  _HMIDEBUG_FCT_PRINTLN("menuInformation::eventSelect.. Begin"); 
	//DIAG(F("selectedMenu : %d"), selectedMenu);
  //menuObject::eventSelect();

  _HMIDEBUG_FCT_PRINTLN("menuInformation::eventSelect.. End");  
  return MENUEXIT;
}

/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void menuInformation::begin()
{
  _HMIDEBUG_FCT_PRINTLN("menuInformation::begin.. Begin"); 


  _HMIDEBUG_FCT_PRINTLN("menuInformation::begin.. End"); 
}

/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuInformation::update()
{
  _HMIDEBUG_FCT_PRINTLN("menuInformation::update.. Begin"); 

	if (updatedInfo)
		return;

	updatedInfo = true;

  display->clearDisplay();
  display->setTextSize(1);
	display->setTextColor(WHITE);

	if (this->value == MENUINFORMATION_ABOUT)
	{
	    display->drawBitmap(0, 0, locoduino_Splash128x44, 128, 44, WHITE);
  	  display->setCursor(0, 48);
	    sprintf(messageInfo,"LaBox %s",VERSION_LABOX);
    	display->println(messageInfo);
  	  display->setCursor(0, 56);
	    sprintf(messageInfo, TXT_MenuAboutCSEX, VERSION);
    	display->println(messageInfo);
	}
	else
	{
		for (int i = this->firstMessage ; i < this->firstMessage + HMIInfo_LinesOnScreen && i < HMIInfo_MessageNumber; i++)
		{
			display->setCursor(0, 0 + (i -this->firstMessage) * 9);
			memset(messageInfo, 0, HMIInfo_MessageMaxSize + 1);
			memcpy(messageInfo, messages[i], HMIInfo_MessageMaxSize);
			display->println(messageInfo);
		}

  	display->setCursor(0, 56);
 		display->println(TXT_MenuInfoButtons);
	}

	display->display();
	
  _HMIDEBUG_FCT_PRINTLN("menuInformation::update.. End"); 
}
/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuInformation::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("menuInformation::resetMenu.. Begin"); 
  menuObject::resetMenu();



  _HMIDEBUG_FCT_PRINTLN("menuInformation::resetMenu.. End"); 
  
}
#endif