/*
 * La Box Project
 * menuTrainCvRead Classe
 *
 * @Author : Thierry Paris
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
    void begin() override;
    void update() override;
    void eventUp() override;
    void eventDown() override;
    int  eventSelect() override;
    void start() override;

  protected:
    //----- Members
    //----- functions
    void resetMenu() override;
};
#endif

#endif
