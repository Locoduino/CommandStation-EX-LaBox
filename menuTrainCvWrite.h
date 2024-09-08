/*
 * La Box Project
 * menuTrainCvWrite Classe
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#ifndef __MENUTRAINCVWRITE__
#define __MENUTRAINCVWRITE__

#ifdef USE_HMI
#include "menuobject.h"
#include "hmiConfig.h"

class hmi;

class menuTrainCvWrite : public menuObject
{
  public:
    //----- Members


    //----- functions
    menuTrainCvWrite(Adafruit_SSD1306* screen, menuObject* parent, const char* title, int value);
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
