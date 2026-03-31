/*
 * La Box Project
 * menuProgMode Classes 
 *
 * @Author : Thierry Paris
 * @Organization : Locoduino.org
 */

#ifndef _MENUPROGMODE_
#define _MENUPROGMODE_

#ifdef USE_HMI
#include "menuobject.h"
#include "hmiConfig.h"

class hmi;

#define HMIInfo_MessageNumber			20 		// 20 messages max.
#define HMIInfo_MessageMaxSize		22		// 21 characters for one message. DO NOT FORGET /0 at the end !
#define HMIInfo_LinesOnScreen			5			// 5 lines max on the screen

class menuProgMode : public menuObject
{
  public:
    //----- Members
    static char messagesPM[HMIInfo_MessageNumber][HMIInfo_MessageMaxSize+1];	// 20 messages of 21 chars max (plus /0)
		static char externalMessageBuffer[HMIInfo_MessageMaxSize+1];
		byte firstMessage;

    //----- functions
    menuProgMode(Adafruit_SSD1306* screen, menuObject* parent, const char* title, int value);
    void begin() override;
    void start() override;
    void update() override;
    void eventUp() override;
    void eventDown() override;
    int  eventSelect() override;

		static void addProgModeMessage(const char* msg);
  protected:
    //----- Members

    //----- functions
    void resetMenu() override;
};
#endif

#endif
