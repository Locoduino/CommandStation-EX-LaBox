/*
 * La Box Project
 * menuSettings Classes 
 *
 * @Author : Thierry Paris
 * @Organization : Locoduino.org
 */

#ifndef _MENUSETTINGS_
#define _MENUSETTINGS_

#ifdef USE_HMI
#include "menuobject.h"
#include "hmiConfig.h"

class hmi;

#define HMIInfo_MessageNumber			20 		// 20 messages max.
#define HMIInfo_MessageMaxSize		22		// 21 characters for one message. DO NOT FORGET /0 at the end !
#define HMIInfo_LinesOnScreen			5			// 5 lines max on the screen

class menuSettings : public menuObject
{
  public:
    //----- Members
		
    //----- functions
    menuSettings(Adafruit_SSD1306* screen, menuObject* parent, const char* title, int value);
    void begin() override;
    void start() override;
    void update() override;
    void eventUp() override;
    void eventDown() override;
    int  eventSelect() override;
  protected:
    //----- Members

    //----- functions
    void resetMenu() override;
};
#endif

#endif
