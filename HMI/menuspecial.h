/*
 * La Box Project
 * menuInformation Classes 
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#ifndef __MENUSPECIAL__
#define __MENUSPECIAL__

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
  protected:
    //----- Members
    unsigned long millisDisplay;
    bool          displayInProgress;
    int           locoID;
    char          message[21];
    //----- functions
    void resetMenu();
};
#endif

#endif
