/*
 * La Box Project
 * menuPhysicalMes Classes 
 *
 * @Author : Thierry Paris
 * @Organization : Locoduino.org
 */

#ifndef _MENUPHYSICALMES_
#define _MENUPHYSICALMES_

#ifdef USE_HMI
#include "menuobject.h"
#include "hmiConfig.h"

class hmi;

class menuPhysicalMes : public menuObject
{
  public:
    //----- Members
		float voltage;
		float current;

    //----- functions
    menuPhysicalMes(Adafruit_SSD1306* screen, menuObject* parent, const char* title, int value);
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
