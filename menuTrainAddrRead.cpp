/*
 * La Box Project
 * menuTrainAddrRead Classes 
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */
#include "DCCEX.h"

#ifdef USE_HMI
#include "menuobject.h"
#include "menuTrainAddrRead.h"
#include "globals.h"
#include "hmi.h"
#include "LaboxModes.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

int locoID = 0;
bool longAddress = false;
bool updatedDisplay;
char message[21];

enum StateReading
{
	//																						Select				Up				Down
	Reading,			// Waiting for Read result...			--					--				--
	AddressRead,	// Reading finished and ok.				--					--				--
	ReadingError,	// Reading finished and failed.		--					--				--
	MenuRetry, 		// Select retry/quit						retry 				--		option quit
	MenuQuit, 		// Select retry/quit						quit			option retry	--
};

StateReading	state;

void locoIdCallback(int16_t inId)
{
	if (inId >= LONG_ADDR_MARKER)
	{
		longAddress = true;
		inId -= LONG_ADDR_MARKER;
	}

  locoID = inId;
	if (locoID > 0)
	{
		state = StateReading::MenuQuit;
	}
	if (locoID <= 0)
	{
		state = StateReading::MenuRetry;
	}
  DIAG(F("locoIdCallback called %d !"), locoID);
  updatedDisplay = false;
}

void menuTrainAddrRead::start()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::start.. Begin"); 

  if (!LaboxModes::progMode) {
		LaboxModes::Restart(CV1ADDR);
  }

  state = StateReading::Reading;
  locoID = 0;
	longAddress = false;
  void (*ptr)(int16_t) = &locoIdCallback;
  DCC::getLocoId(ptr);
  updatedDisplay = false;
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::start.. End"); 
}

/*!
    @brief  menuTrainAddrRead Constructor
    @param  Screen, an Adafruit_SSD1306 object to permit to display our menu
    @param p, a parent object. All menu are chained between parent and sons
    @param title, a menu has a title, a char[HMI_MenueMessageSize].
    @param val, a integer which define the king of menu or the returned value after selection
    @return None (void).
    @note
*/
menuTrainAddrRead::menuTrainAddrRead(Adafruit_SSD1306* screen, menuObject* p, const char* title, int value): menuObject(screen, p, title, value)
{
  resetMenu();
}

/*!
    @brief  eventUp, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuTrainAddrRead::eventUp()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::eventUp.. Begin"); 
  menuObject::eventUp();

	if (state == StateReading::MenuQuit)
	{
		state = StateReading::MenuRetry;
	  updatedDisplay = false;
	}

  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::eventUp.. End");   
}
/*!
    @brief  eventDown, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuTrainAddrRead::eventDown()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::eventDown.. Begin"); 
  menuObject::eventDown();

	if (state == StateReading::MenuRetry)
	{
		state = StateReading::MenuQuit;
    updatedDisplay = false;
	}
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::eventDown.. End");   
}
/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuTrainAddrRead::eventSelect()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::eventSelect.. Begin"); 
  //menuObject::eventSelect();	// Do not come back to Labox main menu... We have to reboot before !

	if (state == StateReading::MenuQuit)
	{
		LaboxModes::Restart(SILENTRETURNTOMAIN);
  }

	if (state == StateReading::MenuRetry)
	{
		menuTrainAddrRead::start();
	}

  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::eventSelect.. End");  
  return MENUEXIT;
}

/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void menuTrainAddrRead::begin()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::begin.. Begin"); 


  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::begin.. End"); 
}

/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuTrainAddrRead::update()
{
  _HMIDEBUG_FCT_PRINTLN(F("menuTrainAddrRead::update.. Begin")); 

  if(!updatedDisplay)
  {
    display->clearDisplay();
    updatedDisplay = true;
  }

  display->setTextSize(1);
	display->setTextColor(WHITE);

  display->setCursor(5, 9);
  display->println(TXT_MenuAddrRead);

  display->setCursor(5, 53);
	if (state == StateReading::MenuRetry)
	{
/*    display->fillRect(5, 53, 60, 9 , WHITE);
		display->setTextColor(BLACK);
	  display->println(TXT_MenuAddrRetry);
	  display->setCursor(64, 53);
		display->setTextColor(WHITE);
	  display->println(TXT_MenuAddrQuit);*/
    sprintf(message,">%s<",TXT_MenuAddrRetry);
	  display->println(message);
	  display->setCursor(64, 53);
    sprintf(message," %s ",TXT_MenuAddrQuit);
	  display->println(message);
	}
	else
	if (state == StateReading::MenuQuit)
	{
/*		display->setTextColor(WHITE);
	  display->println(TXT_MenuAddrRetry);
	  display->setCursor(64, 53);
    display->fillRect(64, 53, 110, 9 , WHITE);
		display->setTextColor(BLACK);
	  display->println(TXT_MenuAddrQuit);*/
    sprintf(message," %s ",TXT_MenuAddrRetry);
	  display->println(message);
	  display->setCursor(64, 53);
    sprintf(message,">%s<",TXT_MenuAddrQuit);
	  display->println(message);
	}
  display->display(); 

  if(locoID > 0)
  {
		if (longAddress)
    sprintf(message,"%04d",locoID);
		else
    	sprintf(message,"%03d", locoID);
  }
  else
  if(locoID < 0)
  {
    sprintf(message," ERR");
  }
  else
    sprintf(message,"----");

  display->setTextColor(WHITE);
  display->setTextSize(3);
  display->setCursor(20, 25);
  display->println(message);  

  display->display();   
  _HMIDEBUG_FCT_PRINTLN(F("menuTrainAddrRead::update.. End")); 
}
/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuTrainAddrRead::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::resetMenu.. Begin"); 
  menuObject::resetMenu();

  updatedDisplay = false;

  _HMIDEBUG_FCT_PRINTLN("menuTrainAddrRead::resetMenu.. End"); 
  
}
#endif