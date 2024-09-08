/*
 * La Box Project
 * menuTrainCvRead Classe
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#ifndef __MENUTRAINCVREAD__
#define __MENUTRAINCVREAD__

#ifdef USE_HMI
#include "menuobject.h"
#include "hmiConfig.h"

class hmi;

class menuTrainCvRead : public menuObject
{
  public:
    //----- Members


    //----- functions
    menuTrainCvRead(Adafruit_SSD1306* screen, menuObject* parent, const char* title, int value);
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
