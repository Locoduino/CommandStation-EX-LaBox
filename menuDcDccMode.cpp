/*
 * LaBox Project
 * menuDcDccMode Classes 
 *
 * @Author : Thierry Paris
 * @Organization : Locoduino.org
 */
#include "defines.h"
#include "DCC.h"

#ifdef USE_HMI
#include "menuobject.h"
#include "menuDcDccMode.h"
#include "hmi.h"
#include "LaboxModes.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

extern hmi boxHMI;

bool displayDcDccToDo;
char messageDcDcc[21];

//#define MENUDIAG(x)		_HMIDEBUG_CRITICAL_PRINTLN(x)
#define MENUDIAG(x)		_HMIDEBUG_FCT_PRINTLN(x)
#define DIAGSTATE		//Serial.println(dcDccState == DcMode?"DcMode":	(dcDccState == DccMode ?"DccMode":(dcDccState == ConfirmChange?"ConfirmChange":"AbortChange")))

enum StateDcDcc
{					//    Dc    		>Dcc<
					//	Confirm			Abort
	//																									Select								Up								Down
	DcMode = 0,	// Ready to change to DC mode					Select DC								--						Move to Dcc
	DccMode = 1, // Ready to change to DCC mode				Select Dcc					Move to Dc				
};

StateDcDcc	dcDccState;

void menuDcDccMode::start()
{
	MENUDIAG("menuDcDccMode::start.. Begin"); 

	dcDccState = LaboxModes::mainMode == MainMode::DC ? DcMode : DccMode;
	displayDcDccToDo = true;
	this->modeChoosen = false;
	boxHMI.stopTimeOutRefresh = true;

	MENUDIAG("menuDcDccMode::start.. End"); 
}

/*!
    @brief  menuShuttleSample Constructor
    @param  Screen, an Adafruit_SSD1306 object to permit to display our menu
    @param p, a parent object. All menu are chained between parent and sons
    @param title, a menu has a title, a char[HMI_MenueMessageSize].
    @param val, a integer which define the king of menu or the returned value after selection
    @return None (void).
    @note
*/
menuDcDccMode::menuDcDccMode(Adafruit_SSD1306* screen, menuObject* p, const char* title, int value): menuObject(screen, p, title, value)
{
	resetMenu();
}

/*!
    @brief  eventUp, Notification of the button up event
    @param  None
    @return None (void).
    @note
*/
void menuDcDccMode::eventUp()
{
	MENUDIAG("menuDcDccMode::eventUp.. Begin");

	//menuObject::eventUp();

	if (dcDccState == DcMode)
	{
		dcDccState = DccMode;
	  displayDcDccToDo = true;
		DIAGSTATE;
	}

	MENUDIAG("menuDcDccMode::eventUp.. End");   
}

/*!
    @brief  eventDown, Notification of the button down event
    @param  None
    @return None (void).
    @note
*/
void menuDcDccMode::eventDown()
{
	MENUDIAG("menuDcDccMode::eventDown.. Begin"); 
	//menuObject::eventDown();

	if (dcDccState == DccMode)
	{
		dcDccState = DcMode;
	  displayDcDccToDo = true;
		DIAGSTATE;
	}

	MENUDIAG("menuDcDccMode::eventDown.. End");   
}

/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuDcDccMode::eventSelect()
{
	MENUDIAG("menuDcDccMode::eventSelect.. Begin"); 

	int ret = 0;
	switch (dcDccState)
	{
		case DcMode:
			LaboxModes::mainMode = MainMode::DC;
			ret = MENUEXIT;
			break;

		case DccMode:
			LaboxModes::mainMode = MainMode::DCC;
			ret = MENUEXIT;
			break;
	}

	while(digitalRead(PIN_BTN_SEL) == LOW)	// wait for button release to avoid immediate selection of the current mode
	{
		delay(10);
	}

	this->modeChoosen = true;
	boxHMI.stopTimeOutRefresh = false;
	DIAGSTATE;
	MENUDIAG("menuDcDccMode::eventSelect.. End");  
	return ret;
}

/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void menuDcDccMode::begin()
{
  MENUDIAG("menuDcDccMode::begin.. Begin"); 

  MENUDIAG("menuDcDccMode::begin.. End"); 
}

/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuDcDccMode::update()
{
  if(!displayDcDccToDo)
  {
		return;
	}

  MENUDIAG(F("menuDcDccMode::update.. Begin")); 

  display->clearDisplay();
  displayDcDccToDo = false;
  
  display->setTextSize(1);
  display->setTextColor(WHITE);
  display->setCursor(0, 0);
  display->println("LaBox   Locoduino.org");
	display->drawFastHLine(0,10,128, WHITE);

  display->setTextSize(2);
  display->setCursor(40, 18);
	if (dcDccState == DcMode)
	{
		sprintf(messageDcDcc," %s ", TXT_DCDCC_DCCRUNNING);
	}
	if (dcDccState == DccMode)
	{
		sprintf(messageDcDcc,">%s<", TXT_DCDCC_DCCRUNNING);
	}
  display->println(messageDcDcc);

  display->setCursor(40, 40);
	if (dcDccState == DcMode)
	{
		sprintf(messageDcDcc,">%s< ", TXT_DCDCC_DCRUNNING);
	}
	if (dcDccState == DccMode)
	{
		sprintf(messageDcDcc," %s", TXT_DCDCC_DCRUNNING);
	}
  display->println(messageDcDcc);

  display->display();   

  MENUDIAG(F("menuDcDccMode::update.. End")); 
}

/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuDcDccMode::resetMenu()
{
  MENUDIAG("menuDcDccMode::resetMenu.. Begin"); 

  menuObject::resetMenu();

  displayDcDccToDo = false;

  MENUDIAG("menuDcDccMode::resetMenu.. End"); 
  
}
#endif