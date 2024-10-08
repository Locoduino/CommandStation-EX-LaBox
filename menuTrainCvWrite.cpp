/*
 * LaBox Project
 * menuTrainCvWrite Classes 
 *
 * @Author : Thierry Paris
 * @Organization : Locoduino.org
 */
#include "DCCEX.h"

#ifdef USE_HMI
#include "menuobject.h"
#include "menuTrainCvWrite.h"
#include "hmi.h"
#include "LaboxModes.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

int writeCVValue = -1;
int writeCVAddress = 0;
bool writeDisplayInProgress;
char messageWrite[21];

enum StateWriting
{
	//																						Select					Up				Down
	Ready,	// Ready to escape from the dialog.		Back to menu		--				--
	FixingAddress, // Select CV Address						Fix Value				+1				-1
	FixingValue,	// Select CV Value							Start Writing		+1				-1
	Writing, // Waiting for end of writing 	...		--							--				--
	Written_OK, // CV written without problem ...	Quit						OtherCV		--
	Written_FAIL // CV written WITH problem  ...	Quit						Redo			--
};

StateWriting	writeState;

void cvWriteValueCallback(int16_t inValue)
{
	if (inValue > 0)
		writeState = Written_OK;
	else
		writeState = Written_FAIL;
		
	writeDisplayInProgress = false;
}

#define DIAGWRITE(FCT)		//DIAG(F("%s :  %s  CV:%d  Val:%d"), FCT, writeState==Ready?"Ready":writeState == FixingAddress?"Fixing Address": \
				//writeState==FixingValue?"Fixing value":"Writing", writeCVAddress, writeCVValue);

void menuTrainCvWrite::start()
{
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvWrite::start.. Begin"); 

  if (!LaboxModes::progMode) {
		LaboxModes::Restart(CVWRITE);
  }

	if (writeCVAddress > 0)
		writeState = FixingAddress;
	else
		writeState = Ready;
	writeDisplayInProgress = false;

	_HMIDEBUG_FCT_PRINTLN("menuTrainCvWrite::start.. End"); 
}

/*!
    @brief  menuTrainCvWrite Constructor
    @param  Screen, an Adafruit_SSD1306 object to permit to display our menu
    @param p, a parent object. All menu are chained between parent and sons
    @param title, a menu has a title, a char[HMI_MenuMessageSize].
    @param val, a integer which define the kind of menu or the returned value after selection
    @return None (void).
    @note
*/
menuTrainCvWrite::menuTrainCvWrite(Adafruit_SSD1306* screen, menuObject* p, const char* title, int value): menuObject(screen, p, title, value)
{
	resetMenu();
}

/*!
    @brief  eventUp, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuTrainCvWrite::eventUp()
{
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvWrite::eventUp.. Begin");
	DIAGWRITE("up begin");

	menuObject::eventUp();

	if (writeState == Ready || writeState == FixingAddress)
	{
		writeState = FixingAddress;
		writeCVValue = -1;
		writeCVAddress++;
		if (writeCVAddress > 255)
			writeCVAddress = 255;
		else
			writeDisplayInProgress = false;
	}

	if (writeState == FixingValue)
	{
		writeCVValue++;
		if (writeCVValue > 255)
			writeCVValue = 255;
		else
			writeDisplayInProgress = false;
	}

	if (writeState == Written_FAIL)
	{
		DIAGWRITE("up end value");
		void (*ptr)(int16_t) = &cvWriteValueCallback;
		DCC::writeCVByte(writeCVAddress, writeCVValue, ptr);
		writeState = Writing;
		writeDisplayInProgress = false;
	}
	
	if (writeState == Written_OK)
	{
		writeState = FixingAddress;
		writeDisplayInProgress = false;
	}

	DIAGWRITE("up end");
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvWrite::eventUp.. End");   
}

/*!
    @brief  eventDown, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
void menuTrainCvWrite::eventDown()
{
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvWrite::eventDown.. Begin"); 
	DIAGWRITE("down begin");
	menuObject::eventDown();

	if (writeState == Ready || writeState == FixingAddress)
	{
		writeState = FixingAddress;
		writeCVValue = -1;
		if (writeCVAddress > 0) {
			writeCVAddress--;
			writeDisplayInProgress = false;
		}

		if (writeCVAddress == 0) {
			writeState = Ready;
			writeDisplayInProgress = false;
		}
	}

	if (writeState == FixingValue)
	{
		if (writeCVValue > 0) {
			writeCVValue--;
			writeDisplayInProgress = false;
		}
	}
	
	if (writeState == Written_FAIL)
	{
		writeState = FixingAddress;
		writeDisplayInProgress = false;
	}

	DIAGWRITE("down end");
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvWrite::eventDown.. End");   
}
/*!
    @brief  eventSelect, Notification of a button event
    @param  None
    @return None (void).
    @note
*/
int menuTrainCvWrite::eventSelect()
{
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvWrite::eventSelect.. Begin"); 
	DIAGWRITE("select begin");

	if (writeState == FixingAddress) {
		DIAGWRITE("select end address");
		writeState = FixingValue;
		writeDisplayInProgress = false;
		return 0;
	}

	if (writeState == FixingValue) {
		DIAGWRITE("select end value");
		void (*ptr)(int16_t) = &cvWriteValueCallback;
		DCC::writeCVByte(writeCVAddress, writeCVValue, ptr);
		writeState = Writing;
		writeDisplayInProgress = false;
		return 0;
	}

	if (writeState == Ready || writeState == Written_OK || writeState == Written_FAIL)
	{
		LaboxModes::Restart(SILENTRETURNTOMAIN);
  }

	DIAGWRITE("select end");
	_HMIDEBUG_FCT_PRINTLN("menuTrainCvWrite::eventSelect.. End");  
	return MENUEXIT;
}

/*!
    @brief  Setup HMI class and start HMI
    @param  None
    @return None (void).
    @note
*/
void menuTrainCvWrite::begin()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainCvWrite::begin.. Begin"); 


  _HMIDEBUG_FCT_PRINTLN("menuTrainCvWrite::begin.. End"); 
}

/*!
    @brief  update, call to refresh screen
    @param  None
    @return None (void).
    @note
*/
void menuTrainCvWrite::update()
{
	_HMIDEBUG_FCT_PRINTLN(F("menuTrainCvWrite::update.. Begin")); 

	if(!writeDisplayInProgress)
	{
		display->clearDisplay();
		writeDisplayInProgress = true;
	}

	display->setTextSize(1);
	display->setCursor(5, 6);
  display->println(TXT_CVWRITE_LOGO);
	display->display(); 

	display->setCursor(20, 26);
	switch (writeState)
	{
		case Written_OK:
			display->println(TXT_CVRW_CVOK);
			break;
		case Written_FAIL:
			display->println(TXT_CVRW_CVFAIL);
			break;
		default:
			display->println(TXT_CVRW_CVVALUE);
			break;
	}

	char mess1[10];
	if (writeCVAddress > 0)
		sprintf(mess1, "%03d:", writeCVAddress);
	else
		sprintf(mess1, "---:");
	if (writeCVValue >= 0)
		sprintf(messageWrite, "%s%03d", mess1, writeCVValue);
	else
		sprintf(messageWrite, "%s---", mess1);

	display->setTextColor(WHITE);
	display->setTextSize(2);
	display->setCursor(20, 38);
	display->println(messageWrite);
	
	display->setTextSize(1);

  display->setCursor(5, 56);
	//display->fillRect(5, 56, 128-5, 64-56, 0);
	switch (writeState)
	{
		case Ready:
			display->println(TXT_CVRW_READY);
			break;
		case FixingAddress:
			display->println(TXT_CVWRITE_ADDRESS);
			break;
		case FixingValue:
			display->println(TXT_CVWRITE_VALUE);
			break;
		case Writing:
			display->println(TXT_CVWRITE_WRITING);
			break;
		case Written_OK:
			display->println(TXT_CVRW_OK);
			break;
		case Written_FAIL:
			display->println(TXT_CVRW_FAIL);
			break;
	}

display->display();   

	_HMIDEBUG_FCT_PRINTLN(F("menuTrainCvWrite::update.. End")); 
}

/*!
    @brief  resetMenu, 
    @param  None
    @return None (void).
    @note
*/
void menuTrainCvWrite::resetMenu()
{
  _HMIDEBUG_FCT_PRINTLN("menuTrainCvWrite::resetMenu.. Begin"); 

  menuObject::resetMenu();

  writeDisplayInProgress = false;

  _HMIDEBUG_FCT_PRINTLN("menuTrainCvWrite::resetMenu.. End"); 
  
}
#endif