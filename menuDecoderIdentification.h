/*
 * LaBox Project
 * menuDecoderIdentification Classe 
 *
 * @Author : Thierry Paris
 * @Organization : Locoduino.org
 */

#ifndef MENUDECODERINDETIFICATION
#define MENUDECODERINDETIFICATION

#ifdef USE_HMI
#include "menuobject.h"
#include "hmiConfig.h"

class hmi;

#define HMIInfo_MessageNumber			20 		// 20 messages max.
#define HMIInfo_MessageMaxSize		22		// 21 characters for one message. DO NOT FORGET /0 at the end !
#define HMIInfo_LinesOnScreen			5			// 5 lines max on the screen

class menuDecoderIdentification : public menuObject
{
  public:
    //----- Members

    //----- functions
    menuDecoderIdentification(Adafruit_SSD1306* screen, menuObject* parent, const char* title, int value);
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
