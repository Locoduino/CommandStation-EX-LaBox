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

class menuInformation : public menuObject
{
  public:
    //----- Members


    //----- functions
    menuInformation(Adafruit_SSD1306* screen, menuObject* parent, const char* title, int value);
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
