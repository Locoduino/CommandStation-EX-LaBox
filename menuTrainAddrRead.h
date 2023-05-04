/*
 * La Box Project
 * menuInformation Classes 
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#ifndef __MENUTRAINADDRREAD__
#define __MENUTRAINADDRREAD__

#ifdef USE_HMI
#include "menuobject.h"
#include "hmiConfig.h"

class hmi;

class menuTrainAddrRead : public menuObject
{
  public:
    //----- Members


    //----- functions
    menuTrainAddrRead(Adafruit_SSD1306* screen, menuObject* parent, const char* title, int value);
    void begin();
    void update();
    void eventUp();
    void eventDown();
    int  eventSelect();
    void start();

  protected:
    //----- Members
    //----- functions
    void resetMenu();
};
#endif

#endif
