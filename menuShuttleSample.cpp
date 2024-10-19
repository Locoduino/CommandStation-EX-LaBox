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
#include "EXRAIL2.h"

#ifdef USE_HMI
#include "menuobject.h"
#include "menuShuttleSample.h"
#include "hmi.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

bool displayShuttleToDo;
int locoAddress;
char messageShuttle[21];
RMFT2 *shuttleRoute;
extern hmi boxHMI;		// from .ino !

enum StateShuttleSample
{
	//																							Select								Up								Down
	Ready,	// Ready to escape from the dialog.		Back to menu						--								--
	FixingAddress, // Select loco Address					Start running						+1								-1
	MenuStop, // Option stop selected							Stop shuttle						--							Select quit
	MenuQuit // Option Quit selected							Quit										Select stop				--
};

StateShuttleSample	shuttleSampleState;

#define DIAGSHUTTLE(FCT)		//DIAG(F("%s :  %s  CV:%d  Val:%d"), FCT, readState==Ready?"Ready":readState == FixingAddress?"Fixing":"Reading", CVAddress, CVValue);

void menuShuttleSample::start()
{
	_HMIDEBUG_FCT_PRINTLN("menuShuttleSample::start.. Begin"); 

	if (shuttleRoute == NULL)
	{
		if (locoAddress > 0)
			shuttleSampleState = FixingAddress;
		else
			shuttleSampleState = Ready;
	}
	else
	{
		shuttleSampleState = MenuStop;
	}
	displayShuttleToDo = true;

	DIAG("shuttle start");

	_HMIDEBUG_FCT_PRINTLN("menuShuttleSample::start.. End"); 
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
menuShuttleSample::menuShuttleSample(Adafruit_SSD1306* screen, menuObject* p, const char* title, int value): menuObject(screen, p, title, value)
{
	resetMenu();
}

/*!
    @brief  eventUp, Notification of the button up event
    @param  None
    @return None (void).
    @note
*/
void menuShuttleSample::eventUp()
{
	_HMIDEBUG_FCT_PRINTLN("menuShuttleSample::eventUp.. Begin");

	menuObject::eventUp();

	if (shuttleSampleState == Ready || shuttleSampleState == FixingAddress)
	{
		shuttleSampleState = FixingAddress;
		locoAddress++;
	  displayShuttleToDo = true;
	}

	if (shuttleSampleState == MenuQuit)
	{
		shuttleSampleState = MenuStop;
		displayShuttleToDo = true;
	}

	_HMIDEBUG_FCT_PRINTLN("menuShuttleSample::eventUp.. End");   
}

/*!
    @brief  eventDown, Notification of the button down event
    @param  None
    @return None (void).
    @note
*/
void menuShuttleSample::eventDown()
{
	_HMIDEBUG_FCT_PRINTLN("menuShuttleSample::eventDown.. Begin"); 
	menuObject::eventDown();

	if (shuttleSampleState == Ready || shuttleSampleState == FixingAddress)
	{
		shuttleSampleState = FixingAddress;
		if (locoAddress > 0) {
			locoAddress--;
			displayShuttleToDo = true;
		}

		if (locoAddress == 0) {
			shuttleSampleState = Ready;
			displayShuttleToDo = true;
		}
	}

	if (shuttleSampleState == MenuStop)
	{
		shuttleSampleState = MenuQuit;
		displayShuttleToDo = true;
	}

	_HMIDEBUG_FCT_PRINTLN("menuShuttleSample::eventDown.. End");   
}

/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuShuttleSample::eventSelect()
{
	_HMIDEBUG_FCT_PRINTLN("menuShuttleSample::eventSelect.. Begin"); 

	switch (shuttleSampleState)
	{
		case FixingAddress:
			// Run sample !
			TrackManager::setMainPower(POWERMODE::ON);
			shuttleRoute = RMFT2::createNewTask(123, locoAddress);
			shuttleSampleState = MenuStop;
			displayShuttleToDo = true;
			_HMIDEBUG_CRITICAL_PRINTLN("menuShuttleSample::eventSelect.. End");  
			return 0;
			break;

		case MenuQuit:
			displayShuttleToDo = true;
			break;

		case MenuStop:
			// because RMFT destructor will send a loco 1 speed command, updating Hmi, calling also the eventSelect 
			// of this menu and doing a stack overflow...
			boxHMI.stopStateMachine = true;
			delete shuttleRoute;
			shuttleRoute = NULL;
			boxHMI.stopStateMachine = false;
			shuttleSampleState = MenuQuit;
			displayShuttleToDo = true;
			return 0;
	}

	_HMIDEBUG_FCT_PRINTLN("menuShuttleSample::eventSelect.. End");  
	return MENUEXIT;
}

/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void menuShuttleSample::begin()
{
  _HMIDEBUG_FCT_PRINTLN("menuShuttleSample::begin.. Begin"); 

	locoAddress = 0;
	shuttleRoute = NULL;

  _HMIDEBUG_FCT_PRINTLN("menuShuttleSample::begin.. End"); 
}

/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuShuttleSample::update()
{
  _HMIDEBUG_FCT_PRINTLN(F("menuShuttleSample::update.. Begin")); 

  if(!displayShuttleToDo)
  {
		return;
	}

  display->clearDisplay();
  displayShuttleToDo = false;
  
  display->setTextSize(1);
  display->setCursor(5, 6);
  display->println(TXT_SHUTTLE_LOGO);

	if (shuttleSampleState == FixingAddress || shuttleSampleState == Ready)
  	display->setTextSize(2);

	display->setCursor(20, 26);
  char add[10];
  if(locoAddress > 0)
    sprintf(add,"%04d",locoAddress);
  else
    sprintf(add,"----");
  display->println(add);

	if (shuttleRoute != NULL)
	{
		display->setTextColor(WHITE);
		display->setTextSize(1);
		display->setCursor(5, 35);
		display->println(TXT_SHUTTLE_RUNNING);
	}

  display->setTextSize(1);
  display->setCursor(5, 56);
	switch (shuttleSampleState)
	{
		case Ready:
			display->println(TXT_CVRW_READY);
			break;
		case FixingAddress:
			display->println(TXT_IDENT_LOCOADDRESS);
			break;

		case MenuStop:
			sprintf(messageShuttle,">%s<",TXT_SHUTTLE_STOP);
			display->println(messageShuttle);
			display->setCursor(64, 55);
			sprintf(messageShuttle," %s ",TXT_MenuAddrQuit);
			display->println(messageShuttle);
			break;

		case MenuQuit:
			sprintf(messageShuttle," %s ",TXT_SHUTTLE_STOP);
			display->println(messageShuttle);
			display->setCursor(64, 55);
			sprintf(messageShuttle,">%s<",TXT_MenuAddrQuit);
			display->println(messageShuttle);
			break;
	}

  display->display();   

  _HMIDEBUG_FCT_PRINTLN(F("menuShuttleSample::update.. End")); 
}

/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuShuttleSample::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("menuShuttleSample::resetMenu.. Begin"); 

  menuObject::resetMenu();

  displayShuttleToDo = false;

  _HMIDEBUG_FCT_PRINTLN("menuShuttleSample::resetMenu.. End"); 
  
}
#endif