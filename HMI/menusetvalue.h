/*
 * La Box Project
 * menuSetValue Classes 
 * 
 * !!!!!! not yet implemented !!!!!!
 * 
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#ifndef _MENUSETVALUE
#define _MENUSETVALUE

#ifdef USE_HMI
#include "menuobject.h"
#include "hmiConfig.h"


class menuSetValue : public menuObject
{
  public:
    //----- Members
    int val_min;
    int val_max;
    int nbDigit;

    //----- functions
    menuSetValue();
    menuSetValue(Adafruit_SSD1306* screen, menuObject* parent, const char* title, int _min, int _max, int _nbDigit);
    void begin();
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