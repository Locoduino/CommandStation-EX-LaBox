/*
 * La Box Project
 * menuProgMode Classes 
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
#include "menuProgMode.h"
#include "hmi.h"
#include "laboxModes.h"

bool updatedProgMode;
bool adaptFirstMessageProgMode;
char menuProgMode::messagesPM[HMIInfo_MessageNumber][HMIInfo_MessageMaxSize+1];	// 20 messages of 21 chars max (plus /0)
char menuProgMode::externalMessageBuffer[HMIInfo_MessageMaxSize+1];

extern hmi boxHMI;

/*!
    @brief menuProgMode Constructor
    @param Screen, an Adafruit_SSD1306 object to permit to display our menu
    @param p, a parent object. All menu are chained between parent and sons
    @param title, a menu has a title, a char[HMI_MenueMessageSize].
    @param value, a integer which define the kind	 of menu or the returned value after selection
    @return None (void).
    @note
*/
menuProgMode::menuProgMode(Adafruit_SSD1306* screen, menuObject* p, const char* title, int value): menuObject(screen, p, title, value)
{
  memset(messagesPM, 0, HMIInfo_MessageNumber * (HMIInfo_MessageMaxSize+1));
	this->firstMessage = 0;
	adaptFirstMessageProgMode = false;

	#if false
	// Test programming mode messages
		menuProgMode::addProgModeMessage("RD 001 : 128");
		menuProgMode::addProgModeMessage("RD 019 : FAIL");
		menuProgMode::addProgModeMessage("RD 002 : 128");
		menuProgMode::addProgModeMessage("RD 003 : 128");
		menuProgMode::addProgModeMessage("WR 004 / 003 : OK");
		menuProgMode::addProgModeMessage("RD 005 : 128");
		menuProgMode::addProgModeMessage("RD 006 : FAIL");
		menuProgMode::addProgModeMessage("RD 007 : 128");
		menuProgMode::addProgModeMessage("RD 008 : 128");
		menuProgMode::addProgModeMessage("WR 009 / 003 : OK");
		menuProgMode::addProgModeMessage("RD 010 : 128");
		menuProgMode::addProgModeMessage("RD 020 : FAIL");
		menuProgMode::addProgModeMessage("RD 021 : 128");
		menuProgMode::addProgModeMessage("RD 022 : 128");
		menuProgMode::addProgModeMessage("WR 023 / 003 : OK");
		menuProgMode::addProgModeMessage("RD 024 : 128");
		menuProgMode::addProgModeMessage("RD 025 : FAIL");
		menuProgMode::addProgModeMessage("RD 026 : 128");
		menuProgMode::addProgModeMessage("RD 027 : 128");
		menuProgMode::addProgModeMessage("WR 028 / 003 : OK");
		menuProgMode::addProgModeMessage("RD 029 : 128");
		menuProgMode::addProgModeMessage("RD 030 : FAIL");
		menuProgMode::addProgModeMessage("RD 031 : 128");
		menuProgMode::addProgModeMessage("RD 032 : 128");
		menuProgMode::addProgModeMessage("WR 033 / 003 : OK");
		menuProgMode::addProgModeMessage("Use UP/DOWN to scroll");
		menuProgMode::addProgModeMessage("Select to exit");
	#endif
}

void menuProgMode::addProgModeMessage(const char* msg)
{
	// Try to find the first empty string.
	for (int i = 0; i < HMIInfo_MessageNumber; i++)
	{
		if (messagesPM[i][0] == 0)
		{
			strncpy(messagesPM[i], msg, HMIInfo_MessageMaxSize);

			updatedProgMode = false;
			adaptFirstMessageProgMode = true;
			return;
		}
	}

	// No place found, so shift messages
	for(int i=1; i < HMIInfo_MessageNumber; i++)
	{
		strncpy(messagesPM[i-1], messagesPM[i], HMIInfo_MessageMaxSize);
	}

	// Add new message
	strncpy(messagesPM[HMIInfo_MessageNumber-1], msg, HMIInfo_MessageMaxSize);
	updatedProgMode = false;
	adaptFirstMessageProgMode = true;
}

void menuProgMode::start()
{
  _HMIDEBUG_FCT_PRINTLN("menuProgMode::start.. Begin"); 

  updatedProgMode = false;
	adaptFirstMessageProgMode = true;
	boxHMI.stopTimeOutRefresh = true;
	boxHMI.stopTrainsViewRefresh = true;

	#if false
	for(int i=0; i < HMIInfo_MessageNumber && messagesPM[i][0] != 0; i++)
	{
		Serial.print(i);
		Serial.print(" : ");
		Serial.println(messagesPM[i]);
	}
	#endif

	LaboxModes::ChangeMode(true, PROGMODE);

  _HMIDEBUG_FCT_PRINTLN("menuProgMode::start.. End"); 
}

/*!
    @brief  eventUp, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuProgMode::eventUp()
{
  _HMIDEBUG_FCT_PRINTLN("menuProgMode::eventUp.. Begin"); 
  menuObject::eventUp();

	if (this->firstMessage > 0) 
	{
		this->firstMessage--;
		updatedProgMode = false;
	}

  _HMIDEBUG_FCT_PRINTLN("menuProgMode::eventUp.. End");   
}

/*!
    @brief  eventDown, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuProgMode::eventDown()
{
  _HMIDEBUG_FCT_PRINTLN("menuProgMode::eventDown.. Begin"); 
  menuObject::eventDown();

	if (this->firstMessage < HMIInfo_MessageNumber-1) 
	{
		this->firstMessage++;
		updatedProgMode = false;
	}

  _HMIDEBUG_FCT_PRINTLN("menuProgMode::eventDown.. End");   
}

/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuProgMode::eventSelect()
{
  _HMIDEBUG_FCT_PRINTLN("menuProgMode::eventSelect.. Begin"); 
	boxHMI.stopTimeOutRefresh = false;
	boxHMI.stopTrainsViewRefresh = false;
	LaboxModes::ChangeMode(false, SILENTRETURNTOMAIN);

  _HMIDEBUG_FCT_PRINTLN("menuProgMode::eventSelect.. End");  
  return MENUEXIT;
}

/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void menuProgMode::begin()
{
  _HMIDEBUG_FCT_PRINTLN("menuProgMode::begin.. Begin"); 
  _HMIDEBUG_FCT_PRINTLN("menuProgMode::begin.. End"); 
}

/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuProgMode::update()
{
  _HMIDEBUG_FCT_PRINTLN("menuProgMode::update.. Begin"); 

	if (updatedProgMode)
	{
		return;
	}

	updatedProgMode = true;

	// Count number of messages
	int count = 0;
	for(; count < HMIInfo_MessageNumber && messagesPM[count][0] != 0; count++)
	{
		/*
		Serial.print(count);
		Serial.print(" : ");
		Serial.println(messagesPM[count]);
		*/
	}

	if (adaptFirstMessageProgMode)
	{
		// If we are beyond the first screen, adjust firstMessage to show the last messages
		this->firstMessage = count - HMIInfo_LinesOnScreen;
		if (count < HMIInfo_LinesOnScreen)
			this->firstMessage = 0;
		adaptFirstMessageProgMode = false;
	}

  display->clearDisplay();
  display->setTextSize(1);
	display->setTextColor(WHITE);

  display->setCursor(0, 0);
 	display->println(TXT_ProgMode);

	char messageProgMode[30];

	for (int i = this->firstMessage ; i < this->firstMessage + HMIInfo_LinesOnScreen && i < HMIInfo_MessageNumber; i++)
	{
		display->setCursor(0, 9 + (i -this->firstMessage) * 9);
		memset(messageProgMode, 0, HMIInfo_MessageMaxSize + 1);
		memcpy(messageProgMode, messagesPM[i], HMIInfo_MessageMaxSize);
		display->println(messageProgMode);
	}

 	display->setCursor(0, 56);
	display->println(TXT_MenuProgButtons);

	display->display();	

  _HMIDEBUG_FCT_PRINTLN("menuProgMode::update.. End"); 
}

/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuProgMode::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("menuProgMode::resetMenu.. Begin"); 
  menuObject::resetMenu();

  _HMIDEBUG_FCT_PRINTLN("menuProgMode::resetMenu.. End");  
}
#endif