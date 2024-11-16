/*
 * LaBox Project
 * menuTrainCvRead Classes 
 *
 * @Author : Thierry Paris
 * @Organization : Locoduino.org
 */
#include "defines.h"
#include "DCC.h"
#include "TrackManager.h"

#ifdef USE_HMI
#include "menuobject.h"
#include "menuDcDccMode.h"
#include "hmi.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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
	DccMode = 1, // Ready to change to DCC mode				Select Dcc					Move to Dc				Move to Confirm
	ConfirmChange = 2, // Confirm the change !				Change comfirmed		Move to DCC				Move to Abort
	AbortChange = 3, // Abort the change !				    Abort comfirmed			Move to Confirm				--
};

StateDcDcc	dcDccState;
TRACK_MODE currentPowerMode;
TRACK_MODE wantedPowerMode;

void menuDcDccMode::start()
{
	MENUDIAG("menuDcDccMode::start.. Begin"); 

  MotorDriver *  mainDriver=NULL;
  for(const auto& md: TrackManager::getMainDrivers()) {
    mainDriver=md;
  }

	if (mainDriver != NULL)
	{
		currentPowerMode = mainDriver->getMode();
		wantedPowerMode = currentPowerMode;

		dcDccState = AbortChange;
		displayDcDccToDo = true;
	}

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

	menuObject::eventUp();

	if (dcDccState > 0)
	{
		dcDccState = (StateDcDcc) ((int) dcDccState - 1);
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
	menuObject::eventDown();

	if (dcDccState < (int) AbortChange)
	{
		dcDccState = (StateDcDcc) ((int) dcDccState + 1);
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
			wantedPowerMode = TRACK_MODE_DC;
			dcDccState = ConfirmChange;
			displayDcDccToDo = true;
			break;

		case DccMode:
			wantedPowerMode = TRACK_MODE_MAIN;
			dcDccState = ConfirmChange;
			displayDcDccToDo = true;
			break;

		case ConfirmChange:
			if (currentPowerMode != wantedPowerMode)
			{
				Serial.print("Move to ");
				Serial.println(wantedPowerMode == TRACK_MODE_DC ? "DcMode":"DccMode");
				//TrackManager::setMainPower(POWERMODE::OFF);
				//TrackManager::setTrackMode(0, wantedPowerMode, DC_MODE_ADDRESS);
				dcDccState = wantedPowerMode == TRACK_MODE_DC ? DcMode:DccMode;
			}
			ret = MENUEXIT;
			break;

		case AbortChange:
			ret = MENUEXIT;
			break;
	}

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
  display->setCursor(5, 6);
  display->println(TXT_SHUTTLE_LOGO);

 	display->setTextSize(2);

	display->setCursor(20, 20);
	if (currentPowerMode == TRACK_MODE_DC)
	{
		if (wantedPowerMode == TRACK_MODE_DC)
			display->println(TXT_DCDCC_DC);
		else
		{
			display->setCursor(5, 20);
			display->println(TXT_DCDCC_TODCC);
		}
	}
	if (currentPowerMode == TRACK_MODE_MAIN)
	{
		if (wantedPowerMode == TRACK_MODE_MAIN)
			display->println(TXT_DCDCC_DCC);
		else
		{
			display->setCursor(5, 20);
			display->println(TXT_DCDCC_TODC);
		}
	}

  display->setTextSize(1);
	switch (dcDccState)
	{
		case DcMode:
			displayOptionString(TXT_DCDCC_DC, true, 5, 44);
			displayOptionString(TXT_DCDCC_DCC, false, 64, 44);
			displayOptionString(TXT_DCDCC_CONFIRM, false, 5, 55);
			displayOptionString(TXT_DCDCC_ABORT, false, 64, 55);
			break;

		case DccMode:
			displayOptionString(TXT_DCDCC_DC, false, 5, 44);
			displayOptionString(TXT_DCDCC_DCC, true, 64, 44);
			displayOptionString(TXT_DCDCC_CONFIRM, false, 5, 55);
			displayOptionString(TXT_DCDCC_ABORT, false, 64, 55);
			break;

		case ConfirmChange:
			displayOptionString(TXT_DCDCC_DC, false, 5, 44);
			displayOptionString(TXT_DCDCC_DCC, false, 64, 44);
			displayOptionString(TXT_DCDCC_CONFIRM, true, 5, 55);
			displayOptionString(TXT_DCDCC_ABORT, false, 64, 55);
			break;

		case AbortChange:
			displayOptionString(TXT_DCDCC_DC, false, 5, 44);
			displayOptionString(TXT_DCDCC_DCC, false, 64, 44);
			displayOptionString(TXT_DCDCC_CONFIRM, false, 5, 55);
			displayOptionString(TXT_DCDCC_ABORT, true, 64, 55);
			break;
	}

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