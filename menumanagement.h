/*
 * La Box Project
 * MenuManagement Class 
 * The purpose of this class is to organize the operation menu. 
 * This is where the actions or choices selected by the user are executed.
 * 
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#ifndef MENUMANAGEMENT
#define MENUMANAGEMENT

#ifdef USE_HMI
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "hmiConfig.h"

class hmi;
class menuObject;

class MenuManagement
{
  public:
    //----- Members

    
    //----- functions  
    MenuManagement(hmi*  screen);
    void begin();
    void update();
    void BtnUpPressed();
    void BtnDownPressed();
    void BtnSelectPressed();
    void resetMenu();

    //----- menu for prog mode
    menuObject* trainAddrRead;
    menuObject* trainCVRead;
    menuObject* trainCVWrite;
    menuObject* trainIdent;
    menuObject* shuttle;

  protected:
    //----- Members
    menuObject* listMenu[50];
    menuObject* baseMenu;
    menuObject* onOffLine;
    menuObject* onOffLineOn;
    menuObject* onOffLineOff;
    menuObject* lstEvent;
    menuObject* info;
    menuObject* about;
    menuObject* wifiInfo;
    menuObject* exCommInfo;
    menuObject* reset;
    menuObject* resetConfirm;
    menuObject* physicalMes;
    menuObject* TrainView;
    menuObject* V1Train;
    menuObject* V2Trains;
    menuObject* V3Trains;
    int menuState;
    menuObject* activeMenu ;
    //menuObject* activeMenu_prev ;

    Adafruit_SSD1306*  display;
    byte  nbMenuItems;
    //----- functions
    //void addMenuInList(menuObject* item);

  public:
      void setMenu(menuObject* menu)  { this->menuState = MENUTRAINADDRREAD; this->activeMenu = menu; this->update(); }
};
#endif

#endif
