/*
 * La Box Project
 * menuInformation Classes 
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#ifndef MENUINFORMATION
#define MENUINFORMATION

#ifdef USE_HMI
#include "menuobject.h"
#include "hmiConfig.h"

class hmi;

#define HMIInfo_MessageNumber			20 		// 20 messages max.
#define HMIInfo_MessageMaxSize		22		// 21 characters for one message. DO NOT FORGET /0 at the end !
#define HMIInfo_LinesOnScreen			5			// 5 lines max on the screen

class menuInformation : public menuObject
{
  public:
    //----- Members
    char messages[HMIInfo_MessageNumber][HMIInfo_MessageMaxSize+1];	// 20 messages of 21 chars max (plus /0)
		byte firstMessage;

    //----- functions
    menuInformation(Adafruit_SSD1306* screen, menuObject* parent, const char* title, int value);
    void begin();
    void start();
    void update();
    void eventUp();
    void eventDown();
    int  eventSelect();
  protected:
    //----- Members

    //----- functions
    void resetMenu();
};
#endif

#endif
