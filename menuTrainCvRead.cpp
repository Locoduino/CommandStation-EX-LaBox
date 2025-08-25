/*
 * LaBox Project
 * menuTrainCvRead Classes 
 *
 * @Author : Thierry Paris
 * @Organization : Locoduino.org
 */
#include "defines.h"
#include "DCC.h"

#ifdef USE_HMI
#include "menuobject.h"
#include "menuTrainCvRead.h"
#include "hmi.h"
#include "LaboxModes.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

int CVValue = -1;
bool displayInProgress;
char messageRead[30];
extern hmi boxHMI;		// from .ino !

enum StateReadingCv
{
	//																							Select								Up								Down
	Ready,	// Ready to escape from the dialog.		Back to menu						--								--
	FixingAddress, // Select CV Address						Start reading						+1								-1
	Reading, // Waiting for Read result ...						--									--								--
	Reading_OK, // CV read without problem ...		Quit										OtherCV						--
	Reading_FAIL // CV read WITH problem  ...			Quit										Redo							--
};

StateReadingCv	readState;

void cvValueCallback(int16_t inValue)
{
	if (inValue >= 0)
		readState = Reading_OK;
	else
		readState = Reading_FAIL;

  CVValue = inValue;
  DIAG(F("cvValueCallback called cv%d = %d !"), boxHMI.currentCVAddress, CVValue);
  displayInProgress = false;
}

#define DIAGREAD(FCT)		//DIAG(F("%s :  %s  CV:%d  Val:%d"), FCT, readState==Ready?"Ready":readState == FixingAddress?"Fixing":"Reading", boxHMI.currentCVAddress, CVValue);

void menuTrainCvRead::start()
{
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvRead::start.. Begin"); 

	LaboxModes::ChangeMode(true, CVREAD);
  
  if (boxHMI.currentCVAddress > 0)
		readState = FixingAddress;
	else
		readState = Ready;
		
	displayInProgress = false;
	boxHMI.isCVAddressEditing = true;

	DIAGREAD("start");

	_HMIDEBUG_FCT_PRINTLN("menuTrainCvRead::start.. End"); 
}

/*!
    @brief  menuTrainCvRead Constructor
    @param  Screen, an Adafruit_SSD1306 object to permit to display our menu
    @param p, a parent object. All menu are chained between parent and sons
    @param title, a menu has a title, a char[HMI_MenueMessageSize].
    @param val, a integer which define the king of menu or the returned value after selection
    @return None (void).
    @note
*/
menuTrainCvRead::menuTrainCvRead(Adafruit_SSD1306* screen, menuObject* p, const char* title, int value): menuObject(screen, p, title, value)
{
	resetMenu();
}

/*!
    @brief  eventUp, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuTrainCvRead::eventUp()
{
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvRead::eventUp.. Begin");
	DIAGREAD("up begin");

	menuObject::eventUp();

	if (readState == Ready || readState == FixingAddress)
	{
		readState = FixingAddress;
		CVValue = -1;
		boxHMI.currentCVAddress++;
		if (boxHMI.currentCVAddress > 255)
			boxHMI.currentCVAddress = 255;
		else
		  displayInProgress = false;
	}

	if (readState == Reading_FAIL)
	{
		CVValue = -1;
		void (*ptr)(int16_t) = &cvValueCallback;
		DCC::readCV(boxHMI.currentCVAddress, ptr);
		readState = Reading;
		displayInProgress = false;
	}

	if (readState == Reading_OK)
	{
		readState = FixingAddress;
		displayInProgress = false;
	}

	DIAGREAD("up end");
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvRead::eventUp.. End");   
}

/*!
    @brief  eventDown, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuTrainCvRead::eventDown()
{
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvRead::eventDown.. Begin"); 
	DIAGREAD("down begin");
	menuObject::eventDown();

	if (readState == Ready || readState == FixingAddress)
	{
		readState = FixingAddress;
		CVValue = -1;
		if (boxHMI.currentCVAddress > 0) {
			boxHMI.currentCVAddress--;
			displayInProgress = false;
		}

		if (boxHMI.currentCVAddress == 0) {
			readState = Ready;
			displayInProgress = false;
		}
	}

	if (readState == Reading_FAIL)
	{
		readState = FixingAddress;
		displayInProgress = false;
	}

	DIAGREAD("down end");
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvRead::eventDown.. End");   
}

/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuTrainCvRead::eventSelect()
{
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvRead::eventSelect.. Begin"); 

	if (readState == FixingAddress)
	{
		CVValue = -1;
		void (*ptr)(int16_t) = &cvValueCallback;
		DCC::readCV(boxHMI.currentCVAddress, ptr);
		readState = Reading;
		displayInProgress = false;
		_HMIDEBUG_FCT_PRINTLN("menuTrainCvRead::eventSelect.. End");  
		return 0;
	}

	if (readState == Ready || readState == Reading_OK || readState == Reading_FAIL)
	{
		LaboxModes::ChangeMode(false, SILENTRETURNTOMAIN);
  }

	DIAGREAD("select end");
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvRead::eventSelect.. End");  
	return MENUEXIT;
}

/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void menuTrainCvRead::begin()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainCvRead::begin.. Begin"); 


  _HMIDEBUG_FCT_PRINTLN("menuTrainCvRead::begin.. End"); 
}

/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuTrainCvRead::update()
{
  if(displayInProgress && !boxHMI.currentCVAddressMoved)
	{
			return;
  }

  if(boxHMI.currentCVAddressMoved)
	{
		if (readState == Ready || readState == FixingAddress)
		{
			readState = FixingAddress;
		}
	}

  _HMIDEBUG_FCT_PRINTLN(F("menuTrainCvRead::update.. Begin")); 

  displayInProgress = true;
	boxHMI.currentCVAddressMoved = false;
	
  display->clearDisplay();
  display->setTextSize(1);
  display->setCursor(5, 6);
  display->println(TXT_CVREAD_LOGO);

	display->setCursor(20, 26);
	switch (readState)
	{
		case Reading_OK:
			display->println(TXT_CVRW_CVOK);
			break;
		case Reading_FAIL:
			display->println(TXT_CVRW_CVFAIL);
			break;
		default:
			display->println(TXT_CVRW_CVVALUE);
			break;
	}

  char add[20];
  if(boxHMI.currentCVAddress > 0)
    sprintf(add,"%03d:", boxHMI.currentCVAddress);
  else
    sprintf(add,"---:");
  if(CVValue >= 0)
    sprintf(messageRead,"%s%03d", add, CVValue);
  else
    sprintf(messageRead,"%s---", add);

  display->setTextColor(WHITE);
  display->setTextSize(2);
  display->setCursor(20, 38);
  display->println(messageRead);

  display->setTextSize(1);

  display->setCursor(5, 56);
	//display->fillRect(5, 56, 128-5, 64-56, 0);
	switch (readState)
	{
		case Ready:
			display->println(TXT_CVRW_READY);
			break;
		case FixingAddress:
			display->println(TXT_CVREAD_ADDRESS);
			break;
		case Reading:
			display->println(TXT_CVREAD_READING);
			break;
		case Reading_OK:
			display->println(TXT_CVRW_OK);
			break;
		case Reading_FAIL:
			display->println(TXT_CVRW_FAIL);
			break;
	}

  display->display();   

  _HMIDEBUG_FCT_PRINTLN(F("menuTrainCvRead::update.. End")); 
}

/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuTrainCvRead::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainCvRead::resetMenu.. Begin"); 

  menuObject::resetMenu();

  displayInProgress = false;

  _HMIDEBUG_FCT_PRINTLN("menuTrainCvRead::resetMenu.. End"); 
  
}
#endif